#pragma once

#include "HandleDecl.h"

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "Network/NetworkDecl.h"

#include "ModulesConfig.h"
#include <Netcode/Graphics/GraphicsContexts.h>

namespace Netcode::Module {

	/*
	The OO part of this project uses I,A,T prefixes.

	I: interface: only pure virtual functions
	A: abstract: has implementations and data members
	T: trait: only virtual functions with default implementations, no data members

	An I and A forces the consumer to implement functions while T gives the freedom not to,
	a T prefixed class must not force implementations
	*/

	class AApp;
	class IModule;
	class IModuleFactory;
	class IWindowModule;
	class IGraphicsModule;
	class INetworkModule;
	class IAudioModule;

	enum class EAppEventType : unsigned {
		NOOP, DEVICE_LOST, RESIZED, MODE_CHANGED, FOCUSED, BLURRED, CLOSED
	};

	/*
	An AppEvent should be a relatively rare event with special semantics
	*/
	struct AppEvent {
		EAppEventType type;
		union {
			Netcode::Int2 resizeArgs;
			Netcode::Graphics::DisplayMode displayMode;
		};
	};

	class IAppEventHandler {
	public:
		virtual ~IAppEventHandler() = default;
		virtual void HandleEvent(const AppEvent & evt) = 0;
	};

	class TAppEventHandler : public IAppEventHandler {
	protected:
		virtual void HandleEvent(const AppEvent & evt) override;
	public:
		virtual ~TAppEventHandler() = default;
		virtual void OnDeviceLost();
		virtual void OnBlur();
		virtual void OnFocus();
		virtual void OnResized(int x, int y);
		virtual void OnClosed();
		virtual void OnModeChanged(Netcode::Graphics::DisplayMode newMode);
	};

	class AppEventSystem {
	protected:
		std::vector<IAppEventHandler *> handlers;
		std::vector<AppEvent> events;
		AppEvent resizeEvent;
	public:
		void RemoveHandler(IAppEventHandler * evtHandler);

		void AddHandler(IAppEventHandler * evtHandler);

		void Dispatch();

		/*
		Posts an event to be processed later
		*/
		void PostEvent(const AppEvent & evt);

		/*
		Broadcast calls every handle immediately
		*/
		void Broadcast(const AppEvent & evt);
	};
	 
	/*
	Abstract App, provides the user convenient way to use certain subsystems, currently supplied:
	window, graphics, network, physics, audio
	*/
	class AApp {
	protected:
		void StartModule(IModule * m);
		void ShutdownModule(IModule * m);

	public:
		std::unique_ptr<AppEventSystem> events;
		std::unique_ptr<IWindowModule> window;
		std::unique_ptr<IGraphicsModule> graphics;
		std::unique_ptr<INetworkModule> network;
		std::unique_ptr<IAudioModule> audio;

		virtual ~AApp() = default;

		/*
		This is a Base dependent function, meaning you have to invoke the base classes implementation in a cascading fashion
		*/
		virtual void AddAppEventHandlers(AppEventSystem * eventSystem);

		/*
		Initialize modules
		*/
		virtual void Setup(IModuleFactory * factory) = 0;

		/*
		Advance simulation, update modules
		*/
		virtual void Run() = 0;

		/*
		Properly shutdown the application
		*/
		virtual void Exit() = 0;
	};

	class IModule : public TAppEventHandler {
	public:
		virtual ~IModule() = default;
		virtual void Start(AApp * app) = 0;
		virtual void Shutdown() = 0;
	};

	class IWindowModule : public IModule {
	public:
		virtual ~IWindowModule() = default;
		virtual void * GetUnderlyingPointer() = 0;
		virtual void ProcessMessages() = 0;
		virtual void CompleteFrame() = 0;
		virtual void ShowWindow() = 0;
		virtual bool KeepRunning() = 0;
		virtual void ShowDebugWindow() = 0;
	};

	class IGraphicsModule : public IModule {
	public:
		Graphics::IDebugContext * debug;
		Graphics::IResourceContext * resources;
		Graphics::IFrameContext * frame;

		virtual ~IGraphicsModule() = default;

		virtual float GetAspectRatio() const = 0;
		virtual Netcode::UInt2 GetBackbufferSize() const = 0;
		virtual Rect GetDisplayRect() const = 0;
		virtual void * GetSwapChain() const = 0;
		virtual DXGI_FORMAT GetBackbufferFormat() const = 0;
		virtual DXGI_FORMAT GetDepthStencilFormat() const = 0;
		virtual void ClearImportCache() = 0;

		virtual Ref<Fence> CreateFence(uint64_t initialValue) = 0;
		virtual Ref<ShaderBuilder> CreateShaderBuilder() = 0;
		virtual Ref<GPipelineStateBuilder> CreateGPipelineStateBuilder() = 0;
		virtual Ref<CPipelineStateBuilder> CreateCPipelineStateBuilder() = 0;
		virtual Ref<InputLayoutBuilder> CreateInputLayoutBuilder() = 0;
		virtual Ref<StreamOutputBuilder> CreateStreamOutputBuilder() = 0;
		virtual Ref<RootSignatureBuilder> CreateRootSignatureBuilder() = 0;
		virtual Ref<SpriteFontBuilder> CreateSpriteFontBuilder() = 0;
		virtual Ref<SpriteBatchBuilder> CreateSpriteBatchBuilder() = 0;
		virtual Ref<TextureBuilder> CreateTextureBuilder() = 0;
		virtual Ref<FrameGraphBuilder> CreateFrameGraphBuilder() = 0;
	};

	class IAudioModule : public IModule {
	public:
		virtual ~IAudioModule() = default;

	};

	class INetworkModule : public IModule {
	public:
		virtual ~INetworkModule() = default;

		virtual Network::Cookie GetCookie(const std::string & key) = 0;
		virtual void SetCookie(const Network::Cookie & cookie) = 0;
		virtual void EraseCookie(const std::string & key) = 0;

		virtual Network::CompletionToken<Network::Response> Login(const std::wstring & username, const std::wstring & password) = 0;
		virtual Network::CompletionToken<Network::Response> QueryServers() = 0;
		virtual Network::CompletionToken<Network::Response> Status() = 0;
		virtual Network::CompletionToken<Network::Response> Logout() = 0;

		virtual Ref<Network::ServerSessionBase> CreateServer() = 0;
		virtual Ref<Network::ClientSessionBase> CreateClient() = 0;
	};

	class IModuleFactory {
	public:
		virtual ~IModuleFactory() = default;
		virtual std::unique_ptr<IWindowModule> CreateWindowModule(AApp * app, int windowType) = 0;
		virtual std::unique_ptr<IGraphicsModule> CreateGraphicsModule(AApp * app, int graphicsType) = 0;
		virtual std::unique_ptr<INetworkModule> CreateNetworkModule(AApp * app, int networkType) = 0;
		virtual std::unique_ptr<IAudioModule> CreateAudioModule(AApp * app, int audioType) = 0;
	};

}
