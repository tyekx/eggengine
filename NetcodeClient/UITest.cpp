#include "UITest.h"

#include <Netcode/Graphics/ResourceDesc.h>
#include <Netcode/UI/TextBox.h>
#include <Netcode/UI/Button.h>
#include <Netcode/UI/StackPanel.h>
#include <Netcode/UI/InputGroup.h>
#include <Netcode/UI/ScrollViewer.h>
#include <Netcode/UI/Slider.h>
#include <Netcode/UI/CheckBox.h>

#include <sstream>

#include "NetcodeApp.h"

namespace ui = Netcode::UI;

Ref<ui::Button> PageBase::CreateButton(const wchar_t * text) {
	Ref<ui::Button> button = controlAllocator.MakeShared<ui::Button>(eventAllocator, CreatePhysxActor());
	button->Sizing(ui::SizingType::FIXED);
	button->Size(Netcode::Float2{ 172.0f, 48.0f });
	button->BackgroundColor(COLOR_SECONDARY);
	button->BorderColor(COLOR_HOVER);
	button->BorderRadius(BORDER_RADIUS);
	button->BorderWidth(BORDER_WIDTH);
	button->Font(textFont);
	button->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	button->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	button->Text(text);
	button->TextColor(COLOR_ACCENT);

	button->OnMouseKeyPressed.Subscribe([](ui::Control * ctrl, ui::MouseEventArgs & args) -> void {
		if(args.Key().GetCode() == Netcode::KeyCode::MOUSE_LEFT) {
			auto * pThis = static_cast<ui::Input *>(ctrl);
			pThis->ClearAnimations();
			pThis->AddAnimation(ui::MakeAnimation(
				&ui::Panel::BackgroundColor,
				&ui::Panel::BackgroundColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_ACCENT },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.15f
			));

			pThis->AddAnimation(ui::MakeAnimation(
				&ui::Label::TextColor,
				&ui::Label::TextColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_SECONDARY },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.15f
			));
		}
	});

	button->OnMouseKeyReleased.Subscribe([](ui::Control * ctrl, ui::MouseEventArgs & args) -> void {
		if(args.Key().GetCode() == Netcode::KeyCode::MOUSE_LEFT) {
			auto * pThis = static_cast<ui::Input *>(ctrl);
			pThis->ClearAnimations();
			pThis->AddAnimation(ui::MakeAnimation(
				&ui::Panel::BackgroundColor,
				&ui::Panel::BackgroundColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_HOVER },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.35f
			));

			pThis->AddAnimation(ui::MakeAnimation(
				&ui::Label::TextColor,
				&ui::Label::TextColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.35f
			));
		}
	});

	button->OnMouseEnter.Subscribe([](ui::Control * ctrl, ui::MouseEventArgs & args) -> void {
		ui::Label * pThis = static_cast<ui::Label *>(ctrl);

		ctrl->ClearAnimations();
		ctrl->AddAnimation(ui::MakeAnimation(
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>(pThis->BackgroundColor(), COLOR_HOVER),
			ui::PlayOnceBehaviour{},
			&Netcode::Function::EaseOutQuad,
			0.7f
		));

		ctrl->AddAnimation(ui::MakeAnimation(
			&ui::Label::TextColor,
			&ui::Label::TextColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));
	});

	button->OnMouseLeave.Subscribe([](ui::Control * ctrl, ui::MouseEventArgs & args) -> void {
		ui::Label * pThis = static_cast<ui::Label *>(ctrl);

		ctrl->ClearAnimations();
		ctrl->AddAnimation(ui::MakeAnimation(
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>(pThis->BackgroundColor(), COLOR_SECONDARY),
			ui::PlayOnceBehaviour{},
			&Netcode::Function::EaseOutQuad,
			0.7f
		));

		ctrl->AddAnimation(ui::MakeAnimation(
			&ui::Label::TextColor,
			&ui::Label::TextColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));
	});

	return button;
}

Ref<Netcode::UI::TextBox> PageBase::CreateTextBox() {
	Ref<ui::TextBox> textBox = controlAllocator.MakeShared<ui::TextBox>(eventAllocator, CreatePhysxActor());
	textBox->Sizing(ui::SizingType::FIXED);
	textBox->Size(Netcode::Float2{ 280.0f, 48.0f });
	textBox->BackgroundColor(COLOR_SECONDARY);
	textBox->BorderColor(COLOR_HOVER);
	textBox->BorderRadius(BORDER_RADIUS);
	textBox->BorderWidth(BORDER_WIDTH);
	textBox->Font(textFont);
	textBox->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	textBox->HorizontalContentAlignment(ui::HorizontalAnchor::LEFT);
	textBox->Padding(Netcode::Float4{ 10.0f, 0.0f, 10.0f, 0.0f });
	textBox->Text(L"");
	textBox->TextColor(COLOR_ACCENT);
	textBox->SelectionBackgroundColor(COLOR_SECONDARY);
	textBox->SelectionTextColor(COLOR_ACCENT);
	textBox->CaretColor(COLOR_SECONDARY);

	textBox->OnKeyPressed.Subscribe([](ui::Control * ctrl, ui::KeyEventArgs & args) -> void {
		ui::TextBox * pThis = static_cast<ui::TextBox *>(ctrl);

		Netcode::Key key = args.Key();

		pThis->HandleDefaultKeyStrokes(key, args.Modifier());
	});

	textBox->OnCharInput.Subscribe([](ui::Control * ctrl, ui::CharInputEventArgs & args) -> void {
		wchar_t v = args.Value();
		ui::TextBox * pThis = static_cast<ui::TextBox *>(ctrl);

		if(v > 0x20) {
			pThis->AppendChar(v);
		}
	});

	textBox->OnBlurred.Subscribe([](ui::Control * ctrl, ui::FocusChangedEventArgs & args) -> void {
		auto * pThis = static_cast<ui::Input *>(ctrl);
		Netcode::Float4 targetColor = (pThis->Hovered()) ? COLOR_HOVER : COLOR_SECONDARY;
		pThis->ClearAnimations();
		pThis->AddAnimation(ui::MakeAnimation(
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), targetColor },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));

		pThis->AddAnimation(ui::MakeAnimation(
			&ui::Label::TextColor,
			&ui::Label::TextColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));
	});

	textBox->OnFocused.Subscribe([](ui::Control * ctrl, ui::FocusChangedEventArgs & args) -> void {
		auto * pThis = static_cast<ui::Input *>(ctrl);
		pThis->ClearAnimations();
		pThis->AddAnimation(ui::MakeAnimation(
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_ACCENT },
			ui::PlayOnceBehaviour{},
			& Netcode::Function::EaseOutQuad,
			0.7f
		));

		pThis->AddAnimation(ui::MakeAnimation(
			&ui::Label::TextColor,
			&ui::Label::TextColor,
			ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_SECONDARY },
			ui::PlayOnceBehaviour{},
			&Netcode::Function::EaseOutQuad,
			0.7f
		));
	});

	textBox->OnMouseEnter.Subscribe([](ui::Control * controlPointer, ui::MouseEventArgs & args) -> void {
		auto * pThis = static_cast<ui::Input *>(controlPointer);

		if(!pThis->Focused()) {
			pThis->ClearAnimations();
			pThis->AddAnimation(ui::MakeAnimation(
				&ui::Panel::BackgroundColor,
				&ui::Panel::BackgroundColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_HOVER },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.7f
			));

			pThis->AddAnimation(ui::MakeAnimation(
				&ui::Label::TextColor,
				&ui::Label::TextColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.7f
			));
		}
	});

	textBox->OnMouseLeave.Subscribe([](ui::Control * controlPointer, ui::MouseEventArgs & args) -> void {
		auto * pThis = static_cast<ui::Input *>(controlPointer);

		if(!pThis->Focused()) {
			pThis->ClearAnimations();
			pThis->AddAnimation(ui::MakeAnimation(
				&ui::Panel::BackgroundColor,
				&ui::Panel::BackgroundColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->BackgroundColor(), COLOR_SECONDARY },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.7f
			));

			pThis->AddAnimation(ui::MakeAnimation(
				&ui::Label::TextColor,
				&ui::Label::TextColor,
				ui::Interpolator<Netcode::Vector4>{ pThis->TextColor(), COLOR_ACCENT },
				ui::PlayOnceBehaviour{},
				& Netcode::Function::EaseOutQuad,
				0.7f
			));
		}
	});

	return textBox;
}

Ref<Netcode::UI::Slider> PageBase::CreateSliderWithTextBox()
{
	Ref<ui::Slider> slider = controlAllocator.MakeShared<ui::Slider>(controlAllocator, eventAllocator, CreatePhysxActor());
	Ref<ui::TextBox> sliderValue = CreateTextBox();
	sliderValue->Padding(Netcode::Float4{ 7.0f, 0.0f, 0.0f, 0.0f });
	sliderValue->SizeX(120.0f);
	Ref<ui::Panel> sliderBar = slider->SliderBar();
	Ref<ui::Panel> sliderButton = slider->SliderButton();
	sliderBar->BorderRadius(4.0f);
	sliderBar->BackgroundColor(COLOR_SECONDARY);
	sliderBar->BorderColor(COLOR_HOVER);
	
	sliderButton->BorderRadius(4.0f);
	sliderButton->BorderWidth(2.0f);
	sliderButton->BackgroundColor(COLOR_SECONDARY);
	sliderButton->BorderColor(COLOR_HOVER);
	
	slider->BindTextBox(std::move(sliderValue));
	return slider;
}

static void LogControls(int depth, const std::vector<Ref<ui::Control>> & children) {
	std::string prefix(depth, '\t');

	for(const auto & i : children) {
		Netcode::Float2 sp = i->ScreenPosition();
		Netcode::Float2 sz = i->Size();

		std::ostringstream oss;
		oss << prefix << "WindowCoords: (" << sp.x << "; " << sp.y << ") Size: (" << sz.x << "; " << sz.y << ")";
		
		Log::Debug(oss.str().c_str());
		LogControls(depth + 1, i->Children());
	}
}

void PageBase::DebugDump()
{
	LogControls(0, Children());
}

void PageBase::InitializeComponents() {
	AssetManager * assets = Service::Get<AssetManager>();

	textFont = assets->ImportFont(L"compiled/fonts/titillium18.spritefont");
}

void LoginPage::InitializeComponents() {
	PageBase::InitializeComponents();
	
	AssetManager * assets = Service::Get<AssetManager>();
	/*
	aenami = assets->ImportTexture2D(L"compiled/textures/ui/aenami_dreamer.jpg");
	Ref<Netcode::ResourceViews> aenamiRv = assets->CreateTextureRV(aenami);
	Netcode::UInt2 aenamiSize = Netcode::UInt2{ static_cast<uint32_t>(aenami->GetDesc().width), aenami->GetDesc().height };
	*/
	Ref<ui::Panel> rootPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	rootPanel->BackgroundColor(Netcode::Float4::One);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	//rootPanel->BackgroundImage(aenamiRv, aenamiSize);
	//rootPanel->BackgroundSize(Netcode::Float2{ 1920.0f, 1080.0f });
	rootPanel->BackgroundHorizontalAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->BackgroundVerticalAlignment(ui::VerticalAnchor::MIDDLE);


	Ref<ui::InputGroup> inputGroup = controlAllocator.MakeShared<ui::InputGroup>(eventAllocator, CreatePhysxActor());
	inputGroup->Sizing(ui::SizingType::DERIVED);

	Ref<ui::StackPanel> inputWrapper = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	inputWrapper->Sizing(ui::SizingType::DERIVED);
	inputWrapper->StackDirection(ui::Direction::VERTICAL);
	inputWrapper->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);

	Ref<ui::Label> titleLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	titleLabel->Sizing(ui::SizingType::FIXED);
	titleLabel->Size(Netcode::Float2{ 400.0f, 100.0f });
	titleLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	titleLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	titleLabel->TextColor(COLOR_ACCENT);
	titleLabel->Font(assets->ImportFont(L"compiled/fonts/titillium48bold.spritefont"));
	titleLabel->Text(L"Netcode");
	titleLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 64.0f });

	Ref<ui::StackPanel> usernameField = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	usernameField->StackDirection(ui::Direction::HORIZONTAL);
	usernameField->Sizing(ui::SizingType::DERIVED);
	usernameField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	Ref<ui::Label> usernameLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	usernameLabel->Sizing(ui::SizingType::FIXED);
	usernameLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
	usernameLabel->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	usernameLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	usernameLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
	usernameLabel->TextColor(COLOR_ACCENT);
	usernameLabel->Font(assets->ImportFont(L"compiled/fonts/titillium18.spritefont"));
	usernameLabel->Text(L"Username:");

	Ref<ui::StackPanel> passwordField = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	passwordField->StackDirection(ui::Direction::HORIZONTAL);
	passwordField->Sizing(ui::SizingType::DERIVED);
	passwordField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	Ref<ui::Label> passwordLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	passwordLabel->Sizing(ui::SizingType::FIXED);
	passwordLabel->Size(Netcode::Float2{ 120.0f, 48.0f });
	passwordLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });
	passwordLabel->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	passwordLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	passwordLabel->TextColor(COLOR_ACCENT);
	passwordLabel->Font(assets->ImportFont(L"compiled/fonts/titillium18.spritefont"));
	passwordLabel->Text(L"Password:");

	Ref<ui::StackPanel> buttonField = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	buttonField->StackDirection(ui::Direction::HORIZONTAL);
	buttonField->Sizing(ui::SizingType::DERIVED);
	buttonField->Margin(Netcode::Float4{ 10.0f, 10.0f, 10.0f, 0.0f });

	loginButton = CreateButton(L"Login");
	loginButton->Margin(Netcode::Float4{ 0.0f, 0.0f, 20.0f, 0.0f });
	
	exitButton = CreateButton(L"Exit");
	exitButton->Margin(Netcode::Float4{ 0.0f, 0.0f, 10.0f, 0.0f });

	usernameTextBox = CreateTextBox();
	passwordTextBox = CreateTextBox();
	passwordTextBox->IsPassword(true);

	passwordField->AddChild(passwordLabel);
	passwordField->AddChild(passwordTextBox);

	usernameField->AddChild(usernameLabel);
	usernameField->AddChild(usernameTextBox);

	buttonField->AddChild(exitButton);
	buttonField->AddChild(loginButton);

	inputWrapper->AddChild(titleLabel);
	inputWrapper->AddChild(usernameField);
	inputWrapper->AddChild(passwordField);
	inputWrapper->AddChild(buttonField);


	inputGroup->AddChild(inputWrapper);
	rootPanel->AddChild(inputGroup);

	this->AddChild(rootPanel);

	std::unique_ptr<ui::Animation> positionAnim = ui::MakeAnimation(
		&ui::Control::Position,
		&ui::Control::Position,
		ui::Interpolator<Netcode::Vector2>{ Netcode::Float2{ -1000.0f, 0.0f }, Netcode::Float2{ 0.0f, 0.0f } },
		ui::PlayOnceBehaviour{ },
		& Netcode::Function::EaseOutQuad, 0.7f);

	inputGroup->AddAnimation(std::move(positionAnim));

	UpdateZIndices();

	loginButton->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &)->void {
		if(onLoginClick) {
			onLoginClick();
		}
	});

	exitButton->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &)->void {
		if(onExitClick) {
			onExitClick();
		}
	});
}

void LoginPage::Activate() {
	PageBase::Activate();

	passwordTextBox->Text(L"");
}

void MainMenuPage::SetUsername(const std::string & username) {
	if(currentlyDisplayedUsername == username) {
		return;
	}
	
	std::wstring ws = L"Welcome, " + Netcode::Utility::ToWideString(username);
	loggedInLabel->Text(ws);
	currentlyDisplayedUsername = username;
}

void MainMenuPage::Activate() {
	PageBase::Activate();

	GameApp* app = Service::Get<GameApp *>();
	SetUsername(app->user.name);
}

void MainMenuPage::InitializeComponents()
{
	PageBase::InitializeComponents();
	
	Ref<ui::Panel> rootPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	rootPanel->BackgroundColor(Netcode::Float4::One);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);

	Ref<ui::StackPanel> buttonField = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	buttonField->StackDirection(ui::Direction::VERTICAL);
	buttonField->Sizing(ui::SizingType::DERIVED);
	buttonField->HorizontalContentAlignment(Netcode::UI::HorizontalAnchor::CENTER);

	AssetManager * assets = Service::Get<AssetManager>();
	
	Ref<ui::Label> titleLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	titleLabel->Sizing(ui::SizingType::FIXED);
	titleLabel->Size(Netcode::Float2{ 400.0f, 100.0f });
	titleLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	titleLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	titleLabel->TextColor(COLOR_ACCENT);
	titleLabel->Font(assets->ImportFont(L"compiled/fonts/titillium48bold.spritefont"));
	titleLabel->Text(L"Netcode");
	titleLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 24.0f });

	loggedInLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	loggedInLabel->Sizing(ui::SizingType::FIXED);
	loggedInLabel->Size(Netcode::Float2{ 400.0f, 48.0f });
	loggedInLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	loggedInLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	loggedInLabel->TextColor(COLOR_ACCENT);
	loggedInLabel->Font(textFont);

	hostGameBtn = CreateButton(L"Host game");
	hostGameBtn->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 8.0f });
	
	joinGameBtn = CreateButton(L"Join game");
	joinGameBtn->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 8.0f });
	
	optionsBtn = CreateButton(L"Options");
	optionsBtn->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 8.0f });

	logoutBtn = CreateButton(L"Logout");
	logoutBtn->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 8.0f });
	
	exitBtn = CreateButton(L"Exit");

	buttonField->AddChild(titleLabel);
	buttonField->AddChild(loggedInLabel);
	buttonField->AddChild(hostGameBtn);
	buttonField->AddChild(joinGameBtn);
	buttonField->AddChild(optionsBtn);
	buttonField->AddChild(logoutBtn);
	buttonField->AddChild(exitBtn);

	rootPanel->AddChild(buttonField);
	
	AddChild(rootPanel);

	UpdateZIndices();

	hostGameBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onHostGameClick) {
			onHostGameClick();
		}
	});
	
	joinGameBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onJoinGameClick) {
			onJoinGameClick();
		}
	});
	
	logoutBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onLogoutClick) {
			onLogoutClick();
		}
	});
	
	optionsBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onOptionsClick) {
			onOptionsClick();
		}
	});
	
	exitBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onExitClick) {
			onExitClick();
		}
	});
}

class SelectablePanel : public ui::StackPanel {
public:
	using ui::StackPanel::StackPanel;

	void PropagateOnMouseMove(Netcode::UI::MouseEventArgs & args) override {
		args.Handled(true);
		ui::StackPanel::PropagateOnMouseMove(args);
	}
	virtual void PropagateOnMouseEnter(Netcode::UI::MouseEventArgs& args) override {
		args.Handled(true);
		ui::StackPanel::PropagateOnMouseEnter(args);
	}

	virtual void PropagateOnMouseLeave(Netcode::UI::MouseEventArgs& args) override {
		args.Handled(true);
		ui::StackPanel::PropagateOnMouseLeave(args);
	}
};


Ref<ui::Control> ServerBrowserPage::CreateServerRow(const GameServerData & srvData, int32_t index)
{
	auto panel = controlAllocator.MakeShared<SelectablePanel>(eventAllocator, CreatePhysxActor());
	panel->StackDirection(ui::Direction::HORIZONTAL);
	panel->HorizontalContentAlignment(ui::HorizontalAnchor::LEFT);
	panel->Sizing(ui::SizingType::FIXED);
	panel->Size(Netcode::Float2{ 960.0f, 48.0f });
	panel->BackgroundColor(COLOR_SECONDARY);
	panel->ZIndex(15.0f);
	
	Ref<ui::Label> label = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	label->Sizing(ui::SizingType::FIXED);
	label->Size(Netcode::Float2{ 540.0f, 48.0f });
	label->HorizontalContentAlignment(ui::HorizontalAnchor::LEFT);
	label->Padding(Netcode::Float4{ 16.0f , 0.0f, -16.0f, 0.0f});
	label->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	label->TextColor(COLOR_ACCENT);
	label->Font(textFont);
	if(!srvData.hostname.empty()) {
		label->Text(Netcode::Utility::ToWideString(srvData.hostname));
	} else {
		label->Text(Netcode::Utility::ToWideString(srvData.address));
	}
	label->ZIndex(9.0f);
	
	Ref<ui::Label> hostLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	hostLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	hostLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	hostLabel->Sizing(ui::SizingType::FIXED);
	hostLabel->Size(Netcode::Float2{ 240.0f, 48.0f });
	hostLabel->TextColor(COLOR_ACCENT);
	hostLabel->Font(textFont);
	hostLabel->Text(Netcode::Utility::ToWideString(srvData.host));
	hostLabel->ZIndex(9.0f);

	Ref<ui::Label> slotsLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	slotsLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	slotsLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	slotsLabel->Sizing(ui::SizingType::FIXED);
	slotsLabel->Size(Netcode::Float2{ 100.0f, 48.0f });
	slotsLabel->TextColor(COLOR_ACCENT);
	slotsLabel->Font(textFont);
	
	std::wostringstream woss;
	woss << srvData.activePlayers << L'/' << srvData.availableSlots;
	slotsLabel->Text(woss.str());
	slotsLabel->ZIndex(9.0f);
	
	Ref<ui::Label> rttLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	rttLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rttLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	rttLabel->Sizing(ui::SizingType::FIXED);
	rttLabel->Size(Netcode::Float2{ 80.0f, 48.0f });
	rttLabel->TextColor(COLOR_ACCENT);
	rttLabel->Font(textFont);

	woss.str(L"");
	woss.clear();
	woss << srvData.rtt;
	rttLabel->Text(woss.str());
	rttLabel->ZIndex(9.0f);

	panel->AddChild(label);
	panel->AddChild(hostLabel);
	panel->AddChild(slotsLabel);
	panel->AddChild(rttLabel);

	panel->OnClick.Subscribe([this, index](Control * ctrl, ui::MouseEventArgs & evtArgs) -> void {
		auto * c = dynamic_cast<ui::StackPanel *>(ctrl);
		selectionIndex = index;
		c->ClearAnimations();
		c->AddAnimation(ui::MakeAnimation(
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>(c->BackgroundColor(), COLOR_ACCENT),
			ui::PlayOnceBehaviour{ },
			&Netcode::Function::LerpIn,
			0.3f
		));
	});
	
	panel->OnMouseEnter.Subscribe([](Control * ctrl, ui::MouseEventArgs & evtArgs) -> void {
		auto * c = dynamic_cast<ui::StackPanel *>(ctrl);
		c->ClearAnimations();
		c->AddAnimation(ui::MakeAnimation(
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>(c->BackgroundColor(), COLOR_HOVER),
			ui::PlayOnceBehaviour{ },
			&Netcode::Function::LerpIn,
			0.3f
		));
	});

	panel->OnMouseLeave.Subscribe([](Control * ctrl, ui::MouseEventArgs & evtArgs) -> void {
		auto * c = dynamic_cast<ui::StackPanel *>(ctrl);
		c->ClearAnimations();
		c->AddAnimation(ui::MakeAnimation(
			&ui::Panel::BackgroundColor,
			&ui::Panel::BackgroundColor,
			ui::Interpolator<Netcode::Vector4>(c->BackgroundColor(), COLOR_SECONDARY),
			ui::PlayOnceBehaviour{ },
			&Netcode::Function::LerpIn,
			0.3f
		));
	});

	return panel;
}

void ServerBrowserPage::InitializeComponents()
{
	PageBase::InitializeComponents();
	AssetManager * assets = Service::Get<AssetManager>();

	Ref<ui::Label> titleLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	titleLabel->Sizing(ui::SizingType::FIXED);
	titleLabel->Size(Netcode::Float2{ 980.0f, 100.0f });
	titleLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	titleLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	titleLabel->TextColor(COLOR_ACCENT);
	titleLabel->Font(assets->ImportFont(L"compiled/fonts/titillium48bold.spritefont"));
	titleLabel->Text(L"Server browser");
	titleLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 24.0f });
	titleLabel->ZIndex(20.0f);

	Ref<ui::Panel> rootPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	rootPanel->BackgroundColor(Netcode::Float4::One);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	rootPanel->ZIndex(10.0f);
	
	Ref<ui::StackPanel> contentWrapper = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	contentWrapper->Sizing(ui::SizingType::DERIVED);
	contentWrapper->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	contentWrapper->StackDirection(ui::Direction::VERTICAL);
	contentWrapper->ZIndex(11.0f);

	Ref<ui::StackPanel> headerWrapper = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	headerWrapper->Sizing(ui::SizingType::FIXED);
	headerWrapper->Size(Netcode::Float2{ 980.0f, 48.0f });
	headerWrapper->HorizontalContentAlignment(ui::HorizontalAnchor::LEFT);
	headerWrapper->StackDirection(ui::Direction::HORIZONTAL);
	headerWrapper->BackgroundColor(COLOR_SECONDARY);
	headerWrapper->ZIndex(12.0f);

	Ref<ui::Panel> borderPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	borderPanel->BackgroundColor(Netcode::Float4::One);
	borderPanel->Sizing(ui::SizingType::FIXED);
	borderPanel->Size(Netcode::Float2{ 980.0f, 4.0f });
	borderPanel->BackgroundColor(COLOR_ACCENT);
	borderPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	borderPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	borderPanel->ZIndex(12.0f);

	Ref<ui::Label> endpointLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	endpointLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	endpointLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	endpointLabel->Sizing(ui::SizingType::FIXED);
	endpointLabel->Size(Netcode::Float2{ 540.0f, 48.0f });
	endpointLabel->Text(L"Endpoint");
	endpointLabel->Font(textFont);
	endpointLabel->TextColor(COLOR_ACCENT);

	Ref<ui::Label> userLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	userLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	userLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	userLabel->Sizing(ui::SizingType::FIXED);
	userLabel->Size(Netcode::Float2{ 240.0f, 48.0f });
	userLabel->Text(L"Created by");
	userLabel->Font(textFont);
	userLabel->TextColor(COLOR_ACCENT);

	Ref<ui::Label> slotsLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	slotsLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	slotsLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	slotsLabel->Sizing(ui::SizingType::FIXED);
	slotsLabel->Size(Netcode::Float2{ 100.0f, 48.0f });
	slotsLabel->Text(L"Slots");
	slotsLabel->Font(textFont);
	slotsLabel->TextColor(COLOR_ACCENT);

	Ref<ui::Label> rttLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	rttLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rttLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	rttLabel->Sizing(ui::SizingType::FIXED);
	rttLabel->Size(Netcode::Float2{ 80.0f, 48.0f });
	rttLabel->Text(L"RTT");
	rttLabel->Font(textFont);
	rttLabel->TextColor(COLOR_ACCENT);

	headerWrapper->AddChild(endpointLabel);
	headerWrapper->AddChild(userLabel);
	headerWrapper->AddChild(slotsLabel);
	headerWrapper->AddChild(rttLabel);

	Ref<ui::ScrollViewer> scrollViewer = controlAllocator.MakeShared<ui::ScrollViewer>(controlAllocator, eventAllocator, CreatePhysxActor());
	scrollViewer->VerticalContentAlignment(ui::VerticalAnchor::TOP);
	scrollViewer->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	scrollViewer->MaxSize(Netcode::Float2{ 980.0f, 360.0f });
	scrollViewer->Sizing(ui::SizingType::FIXED);
	scrollViewer->Size(Netcode::Float2{ 980.0f, 360.0f });
	scrollViewer->ScrollBarColor(COLOR_HOVER);
	scrollViewer->ScrollButtonColor(COLOR_ACCENT);
	scrollViewer->BackgroundColor(COLOR_TERTIARY);
	scrollViewer->ZIndex(12.0f);

	Ref<ui::StackPanel> buttonPanel = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	buttonPanel->Sizing(ui::SizingType::DERIVED);
	buttonPanel->HorizontalContentAlignment(ui::HorizontalAnchor::LEFT);
	buttonPanel->StackDirection(ui::Direction::HORIZONTAL);
	buttonPanel->ZIndex(12.0f);
	
	auto joinGameBtn = CreateButton(L"Join game");
	auto refreshBtn = CreateButton(L"Refresh");
	refreshBtn->Margin(Netcode::Float4{ 0.0f, 0.0f, 8.0f, 0.0f });
	auto backBtn = CreateButton(L"Back");
	backBtn->Margin(Netcode::Float4{ 0.0f, 0.0f, 8.0f, 0.0f });

	Ref<ui::StackPanel> serversListPanel = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	serversListPanel->Sizing(ui::SizingType::DERIVED);
	serversListPanel->ZIndex(11.5f);

	listControl = serversListPanel;
	
	scrollViewer->AddChild(serversListPanel);
	scrollViewer->Margin(Netcode::Float4{ 0.0f, 0.0f , 0.0f , 8.0f });

	buttonPanel->AddChild(backBtn);
	buttonPanel->AddChild(refreshBtn);
	buttonPanel->AddChild(joinGameBtn);
	
	contentWrapper->AddChild(titleLabel);
	contentWrapper->AddChild(headerWrapper);
	contentWrapper->AddChild(borderPanel);
	contentWrapper->AddChild(scrollViewer);
	contentWrapper->AddChild(buttonPanel);

	rootPanel->AddChild(contentWrapper);

	this->AddChild(rootPanel);

	UpdateZIndices();

	backBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onCancel) {
			onCancel();
		}
	});
	
	refreshBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onRefresh) {
			onRefresh();
		}
	});
	
	joinGameBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onJoinCallback) {
			if(selectionIndex == -1 || serverData.size() <= static_cast<size_t>(selectionIndex)) {
				onJoinCallback(nullptr);
			} else {
				onJoinCallback(&serverData.at(selectionIndex));
			}
		}
	});
}

void ServerBrowserPage::SetList(std::vector<GameServerData> srvData) {
	serverData = std::move(srvData);
	selectionIndex = -1;
	int32_t v = 0;

	for(auto & i : listControl->Children()) {
		i->Destruct();
	}
	
	listControl->ClearChildren();
	for(const auto & i : serverData) {
		listControl->AddChild(CreateServerRow(i, v++));
	}
	listControl->UpdateLayout();
}

void LoadingPage::CloseDialog()
{
	rootPanel->ClearAnimations();
	
	rootPanel->AddAnimation(ui::MakeAnimation(
		&ui::Control::SizeY,
		&ui::Control::SizeY,
		ui::Interpolator<float>{ rootPanel->SizeY(), 0.0f },
		ui::PlayOnceBehaviour{  },
		& Netcode::Function::EaseOutQuad,
		0.25f
	));

	switch(state) {
		case DialogState::LOADER: StartHideLoaderAnims(0.0f); break;
		case DialogState::ERROR_MSG: StartHideErrorAnims(0.0f); break;
		default: break;
	}

	rootPanel->OnAnimationsFinished.Subscribe([this](ui::Control *) -> void {
		Deactivate();
		content->ClearChildren();
		Enabled(false);
	});
}

void LoadingPage::Destruct() {
	PageBase::Destruct();
	errorRoot->Destruct();
	loaderRoot->Destruct();
}

void LoadingPage::InitializeComponents() {
	PageBase::InitializeComponents();

	HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);

	state = DialogState::CLOSED;
	
	AssetManager * assets = Service::Get<AssetManager>();

	loadingIcon = assets->ImportTexture2D(L"compiled/textures/ui/loading_icon.png");
	Ref<Netcode::ResourceViews> loadingIconRv = assets->CreateTextureRV(loadingIcon);
	Netcode::UInt2 loadingIconSize = Netcode::UInt2{ static_cast<uint32_t>(loadingIcon->GetDesc().width), loadingIcon->GetDesc().height };

	warningIcon = assets->ImportTexture2D(L"compiled/textures/ui/warning_icon.png");
	Ref<Netcode::ResourceViews> warningIconRv = assets->CreateTextureRV(warningIcon);
	Netcode::UInt2 warningIconSize = Netcode::UInt2{ static_cast<uint32_t>(warningIcon->GetDesc().width), warningIcon->GetDesc().height };

	Ref<ui::Panel> rPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	rPanel->BackgroundColor(COLOR_TERTIARY);
	rPanel->Sizing(ui::SizingType::FIXED);
	rPanel->Size(Netcode::Float2{ 1920.0f, 300.0f });
	rPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	rPanel->Overflow(ui::OverflowType::HIDDEN);
	
	Ref<ui::Control> contentPanel = controlAllocator.MakeShared<ui::Control>(eventAllocator, CreatePhysxActor());
	contentPanel->Sizing(ui::SizingType::DERIVED);
	//contentPanel->StackDirection(ui::Direction::VERTICAL);
	contentPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);

	auto errRoot = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	errRoot->Sizing(ui::SizingType::DERIVED);
	errRoot->StackDirection(ui::Direction::VERTICAL);
	errRoot->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	errorRoot = std::move(errRoot);
	
	auto loadRoot = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, CreatePhysxActor());
	loadRoot->Sizing(ui::SizingType::DERIVED);
	loadRoot->StackDirection(ui::Direction::VERTICAL);
	loadRoot->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	loaderRoot = std::move(loadRoot);

	loaderIconPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	loaderIconPanel->BackgroundImage(loadingIconRv, loadingIconSize);
	loaderIconPanel->Sizing(ui::SizingType::FIXED);
	loaderIconPanel->Size(Netcode::Float2{ 80.0f, 80.0f });
	loaderIconPanel->BackgroundColor(COLOR_ACCENT);
	loaderIconPanel->RotationOrigin(ui::HorizontalAnchor::CENTER, ui::VerticalAnchor::MIDDLE);

	loaderLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	loaderLabel->Text(L"");
	loaderLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	loaderLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	loaderLabel->TextColor(COLOR_ACCENT);
	loaderLabel->Sizing(ui::SizingType::FIXED);
	loaderLabel->Size(Netcode::Float2{ 200.0f, 40.0f });
	loaderLabel->Font(textFont);

	loaderRoot->AddChild(loaderIconPanel);
	loaderRoot->AddChild(loaderLabel);
	loaderRoot->UpdateLayout();

	errorIconPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, CreatePhysxActor());
	errorIconPanel->BackgroundImage(warningIconRv, warningIconSize);
	errorIconPanel->Sizing(ui::SizingType::FIXED);
	errorIconPanel->Size(Netcode::Float2{ 80.0f, 80.0f });
	errorIconPanel->BackgroundColor(COLOR_ACCENT);

	errorButton = CreateButton(L"Okay");

	errorLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	errorLabel->Text(L"");
	errorLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	errorLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	errorLabel->TextColor(COLOR_ACCENT);
	errorLabel->Sizing(ui::SizingType::FIXED);
	errorLabel->Size(Netcode::Float2{ 200.0f, 40.0f });
	errorLabel->Font(textFont);

	errorRoot->AddChild(errorIconPanel);
	errorRoot->AddChild(errorLabel);
	errorRoot->AddChild(errorButton);
	errorRoot->UpdateLayout();

	std::unique_ptr<ui::Animation> loadingAnim = ui::MakeAnimation(
		&ui::Control::RotationZ,
		&ui::Control::RotationZ,
		ui::Interpolator<float>{ 0.0f, Netcode::C_2PI },
		ui::RepeatBehaviour{},
		&Netcode::Function::LerpIn, 0.7f
	);

	errorButton->OnClick.Subscribe([this](ui::Control *, ui::MouseEventArgs & args) ->void {
		if(args.Key().IsFalling()) {
			CloseDialog();
		}
	});

	loaderIconPanel->AddAnimation(std::move(loadingAnim));
	
	rPanel->AddChild(contentPanel);
	this->AddChild(rPanel);

	content = contentPanel;
	rootPanel = rPanel;

	UpdateZIndices();
}

void LoadingPage::Activate() {
	PageBase::Activate();

	Enabled(true);

	static_cast<ui::Panel *>(rootPanel.get())->BackgroundColor(Netcode::Float4::One);
	rootPanel->Size(Netcode::Float2{ 0.0f, 20.0f });

	rootPanel->AddAnimation(ui::MakeAnimation(
		&ui::Control::SizeX,
		&ui::Control::SizeX,
		ui::Interpolator<float>{ 0.0f, 1920.0f },
		ui::PlayOnceBehaviour{ 0.0f },
		&Netcode::Function::EaseOutQuad,
		0.5f
	));

	rootPanel->AddAnimation(ui::MakeAnimation(
		&ui::Panel::BackgroundColor,
		&ui::Panel::BackgroundColor,
		ui::Interpolator<Netcode::Vector4>{ COLOR_ACCENT, COLOR_TERTIARY },
		ui::PlayOnceBehaviour{ 0.3f },
		& Netcode::Function::LerpIn,
		0.45f
	));
}

void LoadingPage::Deactivate() {
	PageBase::Deactivate();
	state = DialogState::CLOSED;
}

void LoadingPage::SetError(const std::wstring & msg) {
	float ySizeAnimationDelay = 0.0f;

	switch(state) {
		case DialogState::LOADER:
			StartHideLoaderAnims(0.0f);
			break;
		case DialogState::CLOSED:
			ySizeAnimationDelay = 0.4f;
			break;
		case DialogState::ERROR_MSG: return;
		default: break;
	}

	state = DialogState::ERROR_MSG;
	
	content->AddChild(errorRoot);
	errorLabel->Text(msg);
	Netcode::Vector2  p = loaderRoot->ScreenPosition();
	content->UpdateLayout();
	Netcode::Vector2 p2 = loaderRoot->ScreenPosition();
	loaderRoot->Position(p - p2);
	
	rootPanel->OnAnimationsFinished.Subscribe([this](Control * p) -> void {
		content->RemoveChild(loaderRoot);
	});
	
	if(rootPanel->Animations().Count() > 2) {
		ySizeAnimationDelay = 0.35f;
		rootPanel->Animations().Splice(2);
	}

	StartShowErrorAnims(ySizeAnimationDelay);
	
	rootPanel->AddAnimation(ui::MakeAnimation(
		&ui::Control::SizeY,
		&ui::Control::SizeY,
		ui::Interpolator<float>{ rootPanel->SizeY(), errorRoot->SizeY() + 80.0f },
		ui::PlayOnceBehaviour{ ySizeAnimationDelay },
		& Netcode::Function::EaseOutQuad,
		0.3f
	));
}

void LoadingPage::StartShowErrorAnims(float delay)
{
	errorIconPanel->Opacity(0.0f);
	errorIconPanel->AddAnimation(ui::MakeAnimation(
		&ui::Panel::Opacity,
		&ui::Panel::Opacity,
		ui::Interpolator<float>{ errorIconPanel->Opacity(), 1.0f },
		ui::PlayOnceBehaviour{ 0.25f },
		& Netcode::Function::LerpIn,
		0.35f
	));

	errorLabel->TextOpacity(0.0f);
	errorLabel->AddAnimation(ui::MakeAnimation(
		&ui::Label::TextOpacity,
		&ui::Label::TextOpacity,
		ui::Interpolator<float>{ errorLabel->TextOpacity(), 1.0f },
		ui::PlayOnceBehaviour{ 0.25f },
		& Netcode::Function::LerpIn,
		0.35f
	));

	errorButton->ZIndex(10.0f);
	errorButton->Opacity(0.0f);
	errorButton->TextOpacity(0.0f);
	errorButton->BorderOpacity(0.0f);

	errorButton->AddAnimation(ui::MakeAnimation(
		&ui::Panel::Opacity,
		&ui::Panel::Opacity,
		ui::Interpolator<float>{ errorButton->Opacity(), 1.0f },
		ui::PlayOnceBehaviour{ 0.25f },
		& Netcode::Function::LerpIn,
		0.35f
	));

	errorButton->AddAnimation(ui::MakeAnimation(
		&ui::Label::TextOpacity,
		&ui::Label::TextOpacity,
		ui::Interpolator<float>{ errorButton->TextOpacity(), 1.0f },
		ui::PlayOnceBehaviour{ 0.25f },
		& Netcode::Function::LerpIn,
		0.35f
	));

	errorButton->AddAnimation(ui::MakeAnimation(
		&ui::Label::BorderOpacity,
		&ui::Label::BorderOpacity,
		ui::Interpolator<float>{ errorButton->BorderOpacity(), 1.0f },
		ui::PlayOnceBehaviour{ 0.25f },
		& Netcode::Function::LerpIn,
		0.35f
	));
}

void LoadingPage::StartHideErrorAnims(float delay)
{
	errorIconPanel->AddAnimation(ui::MakeAnimation(
		&ui::Panel::Opacity,
		&ui::Panel::Opacity,
		ui::Interpolator<float>{ errorIconPanel->Opacity(), 0.0f },
		ui::PlayOnceBehaviour{ delay },
		& Netcode::Function::LerpIn,
		0.25f
	));

	errorLabel->AddAnimation(ui::MakeAnimation(
		&ui::Label::TextOpacity,
		&ui::Label::TextOpacity,
		ui::Interpolator<float>{ errorLabel->TextOpacity(), 0.0f },
		ui::PlayOnceBehaviour{ delay },
		& Netcode::Function::LerpIn,
		0.25f
	));

	errorButton->AddAnimation(ui::MakeAnimation(
		&ui::Panel::Opacity,
		&ui::Panel::Opacity,
		ui::Interpolator<float>{ errorButton->Opacity(), 0.0f },
		ui::PlayOnceBehaviour{ delay },
		& Netcode::Function::LerpIn,
		0.25f
	));

	errorButton->AddAnimation(ui::MakeAnimation(
		&ui::Label::TextOpacity,
		&ui::Label::TextOpacity,
		ui::Interpolator<float>{ errorButton->TextOpacity(), 0.0f },
		ui::PlayOnceBehaviour{ delay },
		& Netcode::Function::LerpIn,
		0.25f
	));

	errorButton->AddAnimation(ui::MakeAnimation(
		&ui::Label::BorderOpacity,
		&ui::Label::BorderOpacity,
		ui::Interpolator<float>{ errorButton->BorderOpacity(), 0.0f },
		ui::PlayOnceBehaviour{ delay },
		& Netcode::Function::LerpIn,
		0.25f
	));
}

void LoadingPage::StartShowLoaderAnims(float delay)
{
	loaderIconPanel->Animations().Splice(1);
	loaderLabel->ClearAnimations();
	
	loaderIconPanel->AddAnimation(ui::MakeAnimation(
		&ui::Panel::Opacity,
		&ui::Panel::Opacity,
		ui::Interpolator<float>{ loaderIconPanel->Opacity(), 1.0f },
		ui::PlayOnceBehaviour{ 0.35f },
		& Netcode::Function::LerpIn,
		0.45f
	));

	loaderLabel->AddAnimation(ui::MakeAnimation(
		&ui::Label::TextOpacity,
		&ui::Label::TextOpacity,
		ui::Interpolator<float>{ loaderLabel->TextOpacity(), 1.0f },
		ui::PlayOnceBehaviour{ delay },
		& Netcode::Function::LerpIn,
		0.45f
	));
}

void LoadingPage::StartHideLoaderAnims(float delay)
{
	loaderIconPanel->Animations().Splice(1);
	loaderLabel->ClearAnimations();

	loaderIconPanel->AddAnimation(ui::MakeAnimation(
		&ui::Panel::Opacity,
		&ui::Panel::Opacity,
		ui::Interpolator<float>{ loaderIconPanel->Opacity(), 0.0f },
		ui::PlayOnceBehaviour{ delay },
		& Netcode::Function::LerpIn,
		0.25f
	));

	loaderLabel->AddAnimation(ui::MakeAnimation(
		&ui::Label::TextOpacity,
		&ui::Label::TextOpacity,
		ui::Interpolator<float>{ loaderLabel->TextOpacity(), 0.0f },
		ui::PlayOnceBehaviour{ 0.0f },
		& Netcode::Function::LerpIn,
		0.25f
	));
}

void LoadingPage::SetLoader(const std::wstring & msg) {
	loaderRoot->Position(Netcode::Float2::Zero);
	content->AddChild(loaderRoot);
	loaderLabel->Text(msg);
	content->UpdateLayout();

	float rootPanelYSizeAnimDelay = 0.0f;

	if(state == DialogState::CLOSED) {
		rootPanelYSizeAnimDelay = 0.4f;
	}

	StartShowLoaderAnims(rootPanelYSizeAnimDelay);

	state = DialogState::LOADER;

	rootPanel->AddAnimation(ui::MakeAnimation(
		&ui::Control::SizeY,
		&ui::Control::SizeY,
		ui::Interpolator<float>{ rootPanel->SizeY(), loaderRoot->SizeY() + 60.0f },
		ui::PlayOnceBehaviour{ rootPanelYSizeAnimDelay },
		& Netcode::Function::EaseOutQuad,
		0.3f
	));
}

void OptionsPage::InitializeComponents()
{
	PageBase::InitializeComponents();

	VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);

	AssetManager * assets = Service::Get<AssetManager>();
	
	Ref<ui::Label> titleLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	titleLabel->Sizing(ui::SizingType::FIXED);
	titleLabel->Size(Netcode::Float2{ 280.0f, 100.0f });
	titleLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	titleLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	titleLabel->TextColor(COLOR_ACCENT);
	titleLabel->Font(assets->ImportFont(L"compiled/fonts/titillium48bold.spritefont"));
	titleLabel->Text(L"Options");
	titleLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 48.0f });

	Ref<ui::StackPanel> sensitivityPanel = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, nullptr);
	sensitivityPanel->Sizing(ui::SizingType::DERIVED);
	sensitivityPanel->StackDirection(ui::Direction::HORIZONTAL);

	Ref<ui::Label> sensitivityLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, nullptr);
	sensitivityLabel->Sizing(ui::SizingType::FIXED);
	sensitivityLabel->Size(Netcode::Float2{ 360.0f, 64.0f });
	sensitivityLabel->Margin(Netcode::Float4{ 280.0f, 0.0f, 0.0f, 0.0f });
	sensitivityLabel->Font(textFont);
	sensitivityLabel->TextColor(COLOR_ACCENT);
	sensitivityLabel->Text(L"Mouse sensitivity:");

	Ref<ui::Slider> slider = CreateSliderWithTextBox();
	slider->MinValue(0.1);
	slider->MaxValue(100.0);
	slider->Value(1.0);

	sensitivityPanel->AddChild(sensitivityLabel);
	sensitivityPanel->AddChild(slider);

	Ref<ui::Label> notifLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, nullptr);
	notifLabel->Sizing(ui::SizingType::FIXED);
	notifLabel->Size(Netcode::Float2{ 480.0f, 64.0f });
	notifLabel->Margin(Netcode::Float4{ 280.0f, 0.0f, 0.0f, 0.0f });
	notifLabel->Font(textFont);
	notifLabel->TextColor(COLOR_ACCENT);
	notifLabel->Text(L"Axis remapping is not supported yet.");
	
	Ref<ui::Label> controlsLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, nullptr);
	controlsLabel->Sizing(ui::SizingType::FIXED);
	controlsLabel->Size(Netcode::Float2{ 480.0f, 240.0f });
	controlsLabel->Margin(Netcode::Float4{ 280.0f, 0.0f, 0.0f, 0.0f});
	controlsLabel->Font(textFont);
	controlsLabel->TextColor(COLOR_ACCENT);
	controlsLabel->Text(LR"([W]: move forward
[S]: move backward
[A]: move left
[D]: move right
[Space]: jump
[LMB]: fire)");

	Ref<ui::Button> backBtn = CreateButton(L"Back");
	backBtn->ZIndex(64.0f);
	backBtn->Position(Netcode::Float2{ -64.0f, -44.0f });
	backBtn->OnClick.Subscribe([this](Control *, ui::MouseEventArgs &) -> void {
		if(onBack) {
			onBack();
		}
	});

	Ref<ui::StackPanel> stackPanel = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, nullptr);
	stackPanel->StackDirection(ui::Direction::VERTICAL);
	stackPanel->Sizing(ui::SizingType::DERIVED);
	stackPanel->AddChild(titleLabel);
	stackPanel->AddChild(sensitivityPanel);
	stackPanel->AddChild(notifLabel);
	stackPanel->AddChild(controlsLabel);

	Ref<ui::ScrollViewer> sv = controlAllocator.MakeShared<ui::ScrollViewer>(controlAllocator, eventAllocator, CreatePhysxActor());
	sv->ScrollBarColor(COLOR_HOVER);
	sv->ScrollButtonColor(COLOR_ACCENT);
	sv->BackgroundColor(Netcode::Float4::One);
	sv->AddChild(stackPanel);
	sv->Sizing(ui::SizingType::INHERITED);

	Ref<ui::Panel> rootPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, nullptr);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	rootPanel->AddChild(sv);
	rootPanel->AddChild(backBtn);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::BOTTOM);
	
	AddChild(rootPanel);
}

void HUD::Update(float dt) {
	uint32_t v = 0;
	for(uint32_t i = 0; i < numActiveLog; i++) {
		logBuffer[i].displayedFor += dt;

		if(logBuffer[i].displayedFor > 5.0f) {
			ui::Label * label = logBuffer[i].label;
			label->Enabled(false);
			v++;
		}
	}

	numActiveLog -= v;
}

void HUD::AddKillFeedItem(const std::wstring & text) {
	uint32_t rotateSize = std::min(numActiveLog + 1, maxLogCount);
	if(rotateSize > 1) {
		std::rotate(std::begin(logBuffer), std::begin(logBuffer) + (rotateSize - 1), std::begin(logBuffer) + rotateSize);
	}
	numActiveLog = rotateSize;
	
	for(uint32_t i = 0; i < numActiveLog; i++) {
		logBuffer[i].label->Position(Netcode::Float2{ 0.0f, static_cast<float>(i * 24) });
	}
	
	ui::Label* label = logBuffer[0].label;
	logBuffer[0].displayedFor = 0.0f;
	label->Enabled(true);
	label->Text(text);
}

void HUD::InitializeComponents()
{
	PageBase::InitializeComponents();

	numActiveLog = 0;
	maxLogCount = 4;
	
	AssetManager * assets = Service::Get<AssetManager>();
	crosshairTexture = assets->ImportTexture2D(L"compiled/textures/ui/crosshair.png");
	auto rv = assets->CreateTextureRV(crosshairTexture);
	hitpipTexture = assets->ImportTexture2D(L"compiled/textures/ui/hitpip.png");
	auto hitpipRV = assets->CreateTextureRV(hitpipTexture);
	Netcode::UInt2 hitpipSize = Netcode::UInt2{ static_cast<uint32_t>(hitpipTexture->GetDesc().width), hitpipTexture->GetDesc().height };
	Netcode::UInt2 rvSize = Netcode::UInt2{ static_cast<uint32_t>(crosshairTexture->GetDesc().width), crosshairTexture->GetDesc().height };
	
	Ref<ui::Panel> crosshair = controlAllocator.MakeShared<ui::Panel>(eventAllocator, nullptr);
	crosshair->Sizing(ui::SizingType::FIXED);
	crosshair->Size(Netcode::Float2{ 32.0f, 32.0f });
	crosshair->BackgroundColor(Netcode::Float4::One);
	crosshair->BackgroundImage(rv, rvSize);
	
	Ref<ui::Panel> hitpip = controlAllocator.MakeShared<ui::Panel>(eventAllocator, nullptr);
	hitpip->Sizing(ui::SizingType::FIXED);
	hitpip->Size(Netcode::Float2{ 28.0f, 28.0f });
	hitpip->BackgroundColor(Netcode::Float4{ 0.8f, 0.8f, 0.8f, 1.0f });
	hitpip->BackgroundImage(hitpipRV, hitpipSize);

	Ref<ui::Label> elimLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, nullptr);
	elimLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	elimLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	elimLabel->Sizing(ui::SizingType::FIXED);
	elimLabel->Size(Netcode::Float2{ 640.0f, 48.0f });
	elimLabel->Font(textFont);
	elimLabel->TextColor(Netcode::Float4::One);
	elimLabel->Text(L"You were eliminated by MrLongNamePotato");
	elimLabel->Position(Netcode::Float2{ 0.0f, 64.0f });
	
	Ref<ui::Panel> rootPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, nullptr);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);

	Ref<ui::Panel> alignmentPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, nullptr);
	alignmentPanel->Sizing(ui::SizingType::INHERITED);
	alignmentPanel->VerticalContentAlignment(ui::VerticalAnchor::TOP);
	alignmentPanel->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);

	killFeed = controlAllocator.MakeShared<ui::Panel>(eventAllocator, nullptr);
	killFeed->Sizing(ui::SizingType::FIXED);
	killFeed->Size(Netcode::Float2{ 480.0f, 160.0f });
	killFeed->HorizontalContentAlignment(ui::HorizontalAnchor::RIGHT);
	killFeed->Position(Netcode::Float2{ -20.0f, 20.0f });

	auto font = assets->ImportFont(L"compiled/fonts/titillium14.spritefont");

	for(int i = 0; i < 4; i++) {
		Ref<ui::Label> elim1 = controlAllocator.MakeShared<ui::Label>(eventAllocator, nullptr);
		elim1->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
		elim1->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
		elim1->Sizing(ui::SizingType::DERIVED);
		elim1->Font(font);
		elim1->TextColor(Netcode::Float4::One);
		elim1->Text(L"MrLongNamePotato eliminated MrLongNamePotato");
		elim1->Position(Netcode::Float2{ 0.0f, static_cast<float>(i * 24) });
		elim1->Enabled(false);
		logBuffer[i].label = elim1.get();
		logBuffer[i].displayedFor = 0.0f;
		killFeed->AddChild(elim1);
	}

	alignmentPanel->AddChild(killFeed);
	
	rootPanel->AddChild(crosshair);
	rootPanel->AddChild(alignmentPanel);
	rootPanel->AddChild(hitpip);
	rootPanel->AddChild(elimLabel);

	AddChild(rootPanel);
}

void HostServerPage::InitializeComponents()
{
	PageBase::InitializeComponents();
	
	AssetManager * assets = Service::Get<AssetManager>();

	checkboxTexture = assets->ImportTexture2D(L"textures/ui/checkmark.png");
	Netcode::UInt2 s{ static_cast<uint32_t>(checkboxTexture->GetDesc().width), checkboxTexture->GetDesc().height };
	Ref<Netcode::ResourceViews> v = assets->CreateTextureRV(checkboxTexture);
	
	Ref<ui::Label> titleLabel = controlAllocator.MakeShared<ui::Label>(eventAllocator, CreatePhysxActor());
	titleLabel->Sizing(ui::SizingType::FIXED);
	titleLabel->Size(Netcode::Float2{ 400.0f, 100.0f });
	titleLabel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	titleLabel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);
	titleLabel->TextColor(COLOR_ACCENT);
	titleLabel->Font(assets->ImportFont(L"compiled/fonts/titillium48bold.spritefont"));
	titleLabel->Text(L"Host server");
	titleLabel->Margin(Netcode::Float4{ 0.0f, 0.0f, 0.0f, 64.0f });

	Ref<ui::CheckBox> checkBox = controlAllocator.MakeShared<ui::CheckBox>(eventAllocator, CreatePhysxActor());
	checkBox->Sizing(ui::SizingType::FIXED);
	checkBox->Size(Netcode::Float2{ 40.0f, 40.0f });
	checkBox->CheckedImage(v, s);
	checkBox->BackgroundColor(Netcode::Float4{ 1.0f, 0.0f, 0.0f, 1.0f });

	Ref<ui::StackPanel> contentPanel = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, nullptr);
	contentPanel->StackDirection(ui::Direction::VERTICAL);
	contentPanel->Sizing(ui::SizingType::DERIVED);

	Ref<ui::StackPanel> buttonPanel = controlAllocator.MakeShared<ui::StackPanel>(eventAllocator, nullptr);
	buttonPanel->StackDirection(ui::Direction::HORIZONTAL);
	buttonPanel->Sizing(ui::SizingType::DERIVED);

	Ref<ui::Button> backBtn = CreateButton(L"Back");
	backBtn->Margin(Netcode::Float4{ 0.0f, 0.0f, 8.0f, 0.0f });
	Ref<ui::Button> hostBtn = CreateButton(L"Host");
	buttonPanel->AddChild(backBtn);
	buttonPanel->AddChild(hostBtn);

	contentPanel->AddChild(titleLabel);
	contentPanel->AddChild(checkBox);
	contentPanel->AddChild(buttonPanel);
	
	Ref<ui::Panel> rootPanel = controlAllocator.MakeShared<ui::Panel>(eventAllocator, nullptr);
	rootPanel->Sizing(ui::SizingType::INHERITED);
	//rootPanel->BackgroundColor(Netcode::Float4::One);
	rootPanel->HorizontalContentAlignment(ui::HorizontalAnchor::CENTER);
	rootPanel->VerticalContentAlignment(ui::VerticalAnchor::MIDDLE);

	rootPanel->AddChild(contentPanel);
	AddChild(rootPanel);
}
