#include "WinapiWindowModule.h"
#include "Input.h"
#include <io.h>
#include <fcntl.h>
#include <iostream>

#include <Netcode/Input/Key.h>
#include <Netcode/Config.h>

using Netcode::Graphics::DisplayMode;

namespace Netcode::Module {

	BOOL MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM userData) {
		 auto * p = reinterpret_cast<std::vector<HMONITOR> *>(userData);

		 p->push_back(hMonitor);

		 return TRUE;
	}

	void SetupConsole() {

		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		int consoleHandleR, consoleHandleW;
		HANDLE stdioHandle;
		FILE * fptr;

		AllocConsole();
		std::string str = "Dev Console";
		SetConsoleTitle(str.c_str());

		EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_GRAYED);
		DrawMenuBar(GetConsoleWindow());

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleInfo);

		stdioHandle = GetStdHandle(STD_INPUT_HANDLE);
		consoleHandleR = _open_osfhandle(reinterpret_cast<intptr_t>(stdioHandle), _O_TEXT);
		fptr = _fdopen(consoleHandleR, "r");
		*stdin = *fptr;
		setvbuf(stdin, NULL, _IONBF, 0);

		stdioHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		consoleHandleW = _open_osfhandle(reinterpret_cast<intptr_t>(stdioHandle), _O_TEXT);
		fptr = _fdopen(consoleHandleW, "w");
		*stdout = *fptr;
		setvbuf(stdout, NULL, _IONBF, 0);

		stdioHandle = GetStdHandle(STD_ERROR_HANDLE);
		*stderr = *fptr;
		setvbuf(stderr, NULL, _IONBF, 0);

		std::ios::sync_with_stdio();

		HWND cWnd = GetConsoleWindow();

		DWORD consoleMode;
		GetConsoleMode(cWnd, &consoleMode);

		SetConsoleMode(cWnd, consoleMode | ENABLE_WINDOW_INPUT);

		const bool consoleMaximized = Config::GetOptional<bool>(L"window.debugConsole.maximized:bool", false);
		
		if(consoleMaximized) {
			size_t monitorIndex = Config::GetOptional<uint8_t>(L"window.debugConsole.displayIndex:u8", 0);

			std::vector<HMONITOR> hMons;
			hMons.reserve(4);
			EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, reinterpret_cast<LPARAM>(&hMons));

			if(!hMons.empty()) {
				monitorIndex = std::min(monitorIndex, hMons.size() - 1);

				MONITORINFOEX monitorInfoEx = {};
				monitorInfoEx.cbSize = sizeof(MONITORINFOEX);

				GetMonitorInfoA(hMons[monitorIndex], &monitorInfoEx);

				SetWindowPos(cWnd, nullptr, monitorInfoEx.rcMonitor.left, monitorInfoEx.rcMonitor.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

				ShowWindow(cWnd, SW_MAXIMIZE);
			} else {
				Log::Debug("Failed to set fullscreen console position");

				ShowWindow(cWnd, SW_SHOWDEFAULT);
			}
		} else {
			Int2 position = Config::GetOptional(L"window.debugConsole.position:Int2", Int2::Zero);
			Int2 size = Config::GetOptional(L"window.debugConsole.size:Int2", Int2{ 800,600 });
			SetWindowPos(cWnd, nullptr, position.x, position.y, size.x, size.y, SWP_NOZORDER);

			ShowWindow(cWnd, SW_SHOWDEFAULT);
		}
	}

	void SizingTimerProc(_In_ HWND hwnd, _In_ UINT wmTimer, _In_ UINT_PTR timerPtr, _In_ DWORD timeSinceEpochMs) {
		Log::Debug("SizingTimerProc()");
		PostMessage(hwnd, UM_RENDER, 0, 0);
	}

	static KeyCode TranslateVirtualKey(LPARAM lParam) {
		uint32_t scanCode = LOBYTE(HIWORD(lParam)) & (0x7f);
		uint32_t keyCode = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);

		return static_cast<KeyCode>(keyCode);
	}

	LRESULT CALLBACK WindowProcess(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
		WinapiWindowModule * pThis = reinterpret_cast<WinapiWindowModule *>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));

		// Netcode::Utility::DebugEvent(message, { WM_NCHITTEST, WM_MOUSEFIRST, WM_SETCURSOR, WM_MOUSEMOVE, WM_INPUT });

		switch(message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_MOUSEMOVE:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				Input::ProcessMouseMoveEvent(Netcode::Int2{ x, y });
			}
			break;

		case WM_SYSKEYDOWN:
			if((wParam == VK_RETURN) && (lParam & (1 << 29))) {
				if(pThis->displayMode == DisplayMode::BORDERLESS) {
					AppEvent evt;
					evt.type = EAppEventType::MODE_CHANGED;
					evt.displayMode = DisplayMode::WINDOWED;
					pThis->eventSystem->PostEvent(evt);
				} else if(pThis->displayMode == DisplayMode::WINDOWED) {
					AppEvent evt;
					evt.type = EAppEventType::MODE_CHANGED;
					evt.displayMode = DisplayMode::BORDERLESS;
					pThis->eventSystem->PostEvent(evt);
				}
			} else {
				if(wParam == VK_MENU || wParam == VK_CONTROL) {
					Input::ProcessKeyEvent(Key{ TranslateVirtualKey(lParam), KeyState::PRESSED });
				}
			}
			break;

		case WM_SYSCOMMAND:
			// ALT should no longer access system menu
			if(wParam == SC_KEYMENU) {
				return LRESULT{};
			}
			break;

		case WM_SYSKEYUP: 
			if(wParam == VK_MENU || wParam == VK_CONTROL) {
				Input::ProcessKeyEvent(Key{ TranslateVirtualKey(lParam), KeyState::RELEASED });
			}
			break;

		case WM_ENTERSIZEMOVE: 
			Log::Debug("WM_ENTERSIZEMOVE");
			pThis->EnterSizeMove();
			break;

		case WM_EXITSIZEMOVE:
			Log::Debug("WM_EXITSIZEMOVE");
			pThis->ExitSizeMove();
			break;

		case UM_RENDER:
			Log::Debug("UM_RENDER");
			break;

		case WM_SIZE:
			if(pThis) {
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);
				AppEvent evt;
				evt.type = EAppEventType::RESIZED;
				evt.resizeArgs.x = width;
				evt.resizeArgs.y = height;
				pThis->eventSystem->PostEvent(evt);
			}
			break;

		case WM_ACTIVATE:
		case WM_SETFOCUS:
			Input::ProcessFocusedEvent();
			if(pThis) {
				pThis->eventSystem->PostEvent(AppEvent{ EAppEventType::FOCUSED });
			}
			break;

		case WM_KILLFOCUS:
			Input::ProcessBlurredEvent();
			if(pThis) {
				pThis->eventSystem->PostEvent(AppEvent{ EAppEventType::BLURRED });
			}
			break;

		case WM_INPUT:
			Input::ProcessPlatformEvent(wParam, lParam);
			return 0;

		case WM_KEYDOWN: {
				Input::ProcessKeyEvent(Key{ TranslateVirtualKey(lParam), KeyState::PRESSED });
			}
			break;

		case WM_KEYUP: {
				Input::ProcessKeyEvent(Key{ TranslateVirtualKey(lParam), KeyState::RELEASED });
			}
			break;
		case WM_CHAR: {
				Input::ProcessCharEvent(static_cast<wchar_t>(wParam));
			}
			break;
		}

		
		return DefWindowProc(windowHandle, message, wParam, lParam);
	}

	void WinapiWindowModule::CompleteFrame() {
		Netcode::Input::CompleteFrame();
	}

	bool WinapiWindowModule::KeepRunning() {
		return isRunning;
	}

	void WinapiWindowModule::OnFocus() {

	}

	void WinapiWindowModule::OnBlur() {

	}

	void WinapiWindowModule::OnModeChanged(DisplayMode mode)
	{
		if(mode == displayMode) {
			return;
		}

		if(mode == DisplayMode::BORDERLESS) {
			RECT clientRect;
			RECT windowRect;
			GetClientRect(windowHandle, &clientRect);
			GetWindowRect(windowHandle, &windowRect);
			lastWindowedPos = windowRect;
			lastWindowedClientRect = clientRect;

			SetWindowLongW(windowHandle, GWL_STYLE, windowedStyle & (~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME)));

			Rect displayRect = graphics->GetDisplayRect();

			SetWindowPos(windowHandle, HWND_TOP,
				displayRect.left,
				displayRect.top,
				displayRect.right - displayRect.left,
				displayRect.bottom - displayRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(windowHandle, SW_MAXIMIZE);

		} else if(mode == DisplayMode::WINDOWED) {
			SetWindowLongW(windowHandle, GWL_STYLE, windowedStyle);

			SetWindowPos(windowHandle, HWND_TOP,
				lastWindowedPos.left,
				lastWindowedPos.top, 
				lastWindowedClientRect.right - lastWindowedClientRect.left,
				lastWindowedClientRect.bottom - lastWindowedClientRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(windowHandle, SW_NORMAL);
		}

		displayMode = mode;
	}

	void WinapiWindowModule::EnterSizeMove()
	{
		timerHandle = SetTimer(windowHandle, 1, 500, SizingTimerProc);
	}

	void WinapiWindowModule::ExitSizeMove()
	{
		KillTimer(windowHandle, 1);
	}

	void WinapiWindowModule::Start(AApp * app) {
		displayMode = DisplayMode::WINDOWED;
		const char * windowClassName = "NetcodeWndClass";
		windowedStyle = WS_OVERLAPPEDWINDOW;

		graphics = app->graphics.get();

#if defined(_DEBUG)
		constexpr bool showDebugConsoleDefault = true;

		const bool debugConsoleEnabled =
			Config::GetOptional<bool>(L"window.debugConsole.enabled:bool", showDebugConsoleDefault);

		if(debugConsoleEnabled) {
			ShowDebugWindow();
		}
		
		Log::Setup(true);
#else 
		Log::Setup(false);
#endif
		
		const Int2 windowPosition = Config::GetOptional<Int2>(L"window.position:Int2", Int2{ CW_USEDEFAULT, CW_USEDEFAULT });
		const Int2 windowSize = Config::GetOptional<Int2>(L"window.size:Int2", Int2{ CW_USEDEFAULT, CW_USEDEFAULT });

		eventSystem = app->events.get();

		WNDCLASSEX windowClass = { 0 };

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.lpfnWndProc = WindowProcess;
		windowClass.lpszClassName = windowClassName;
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;

		RegisterClassEx(&windowClass);

		HWND wnd = CreateWindow(windowClassName,
								"Netcode3D",
								windowedStyle,
								windowPosition.x,
								windowPosition.y,
								windowSize.x,
								windowSize.y,
								NULL,
								NULL,
								windowClass.hInstance,
								0);

		SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));


		ASSERT(wnd != NULL, "Failed to create window");

		windowHandle = wnd;

		POINT p;
		if(GetCursorPos(&p)) {
			if(ScreenToClient(wnd, &p)) {
				Input::SetInitialMousePosition(Int2{ p.x, p.y });
			}
		}
		
		isRunning = true;
	}

	void WinapiWindowModule::Shutdown() {
		PostQuitMessage(0);
	}

	void * WinapiWindowModule::GetUnderlyingPointer() {
		return reinterpret_cast<void *>(windowHandle);
	}

	void WinapiWindowModule::ProcessMessages() {
		MSG winMessage = { 0 };

		while(PeekMessage(&winMessage, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&winMessage);
			DispatchMessage(&winMessage);

			if(winMessage.message == WM_QUIT) {
				isRunning = false;
			}
		}
	}

	void WinapiWindowModule::ShowWindow() {
		::ShowWindow(windowHandle, SW_SHOWDEFAULT);
	}

	void WinapiWindowModule::ShowDebugWindow() {
		SetupConsole();
	}

}

