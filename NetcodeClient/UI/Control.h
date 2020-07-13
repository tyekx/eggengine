#pragma once

#include <memory>

#include <Netcode/HandleTypes.h>
#include <Netcode/Graphics/GraphicsContexts.h>
#include <Netcode/Functions.h>
#include <Netcode/PhysXWrapper.h>
#include <Netcode/Event.hpp>

#include "EventArgs.h"
#include "Animation.hpp"

using Netcode::SpriteBatch;

namespace UI {

    enum class Direction : uint32_t {
        VERTICAL, HORIZONTAL
    };

	enum class HorizontalAnchor : uint32_t {
		LEFT, CENTER, RIGHT
	};

	enum class VerticalAnchor : uint32_t {
		TOP, MIDDLE, BOTTOM
	};

    enum class SizingType : uint32_t {
        FIXED, DERIVED, INHERITED, WINDOW
    };

    class Control : public std::enable_shared_from_this<Control> {
    protected:
        Netcode::Float2 size;
        Netcode::Float2 position;
        Netcode::Float2 rotationOrigin;
        Netcode::Float4 margin;
        Netcode::Float4 padding;
        float rotationZ;
        float zIndex;
        SizingType zIndexSizing;
        SizingType rotationOriginSizing;
        SizingType sizing;
        HorizontalAnchor horizontalRotationOrigin;
        VerticalAnchor verticalRotationOrigin;
        HorizontalAnchor horizontalContentAlignment;
        VerticalAnchor verticalContentAlignment;
        std::shared_ptr<Control> parent;
        std::vector<std::shared_ptr<Control>> children;
        AnimationContainer animations;
        bool enabled;

        SizingType ZIndexSizing() const;

        void ZIndexSizing(SizingType sz);

        SizingType RotationOriginSizing() const;

        void RotationOriginSizing(SizingType sz);

        static Netcode::Float2 CalculateAnchorOffset(HorizontalAnchor xAnchor, VerticalAnchor yAnchor, const Netcode::Float2 & controlSize);

        void UpdateZIndices(int depth) {
            if(ZIndexSizing() == SizingType::DERIVED) {
                zIndex = static_cast<float>(depth);
            }

            for(auto & child : children) {
                child->UpdateZIndices(depth + 1);
            }
        }

    public:
        static constexpr float MAX_DEPTH = 256.0f;

        Control();

        virtual ~Control() = default;

        HorizontalAnchor HorizontalRotationOrigin() const;

        void HorizontalRotationOrigin(HorizontalAnchor xAnchor);

        VerticalAnchor VerticalRotationOrigin() const;

        void VerticalRotationOrigin(VerticalAnchor yAnchor);

        HorizontalAnchor HorizontalContentAlignment() const;

        void HorizontalContentAlignment(HorizontalAnchor horizontalAnchor);

        VerticalAnchor VerticalContentAlignment() const;

        void VerticalContentAlignment(VerticalAnchor verticalAnchor);

        void AddAnimation(std::unique_ptr<Animation> anim) {
            anim->Run(0.0f);
            animations.Add(std::move(anim));
        }

        SizingType Sizing() const;

        void Sizing(SizingType sizingType);
     
        Netcode::Float2 Size() const;

        void Size(const Netcode::Float2 & sz);

        Netcode::Float2 Position() const;

        void Position(const Netcode::Float2 & pos);

        Netcode::Float2 RotationOrigin() const;

        void RotationOrigin(const Netcode::Float2 & pos);

        void RotationOrigin(HorizontalAnchor x, VerticalAnchor y);

        Netcode::Float2 ScreenPosition() const;

        Netcode::Float4 Margin() const;

        void Margin(const Netcode::Float4 & leftTopRightBottom);

        Netcode::Float4 Padding() const;

        void Padding(const Netcode::Float4 & leftTopRightBottom);

        void Parent(std::shared_ptr<Control> newParent);

        std::shared_ptr<Control> Parent() const;

        const std::vector<std::shared_ptr<Control>> & Children() const;

        void RotationZ(float rotZ);

        float RotationZ() const;

        void ZIndex(float zValue);

        float ZIndex() const;

        void ResetZIndex();

        virtual Netcode::Float2 BoxSize() const;
        
        virtual Netcode::Float2 CalculatedSize() const;

        virtual void UpdateSize(const Netcode::Float2 & screenSize);

        virtual void UpdateLayout();

        virtual void Update(float dt);

        virtual void Destruct();

        virtual void Enable();

        virtual void Disable();

        virtual void AddChild(std::shared_ptr<Control> child);

        virtual void Render(Netcode::SpriteBatchPtr batch);

        virtual void OnInitialized();

        virtual void OnFocused(FocusChangedEventArgs & args);

        virtual void OnKeyDown(KeyboardEventArgs & args);

        virtual void OnKeyUp(KeyboardEventArgs & args);

        virtual void OnClick(MouseEventArgs & args);

        virtual void OnMouseEnter(MouseEventArgs & args);

        virtual void OnMouseMove(MouseEventArgs & args);

        virtual void OnMouseLeave(MouseEventArgs & args);

        virtual void OnEnabled();

        virtual void OnDisabled();

        virtual void OnSizeChanged();

        virtual void OnPositionChanged();

        virtual bool IsFocused();

        virtual bool IsEnabled();

        Netcode::Event<Control *, MouseEventArgs &> MouseEnterEvent;
        Netcode::Event<Control *, MouseEventArgs &> MouseLeaveEvent;
        Netcode::Event<Control *, MouseEventArgs &> MouseMoveEvent;
    };


    class Panel : public Control {
    protected:
        Netcode::BorderType borderType;
        float borderWidth;
        float borderRadius;
        Netcode::BackgroundType backgroundType;
        VerticalAnchor backgroundVerticalAlignment;
        HorizontalAnchor backgroundHorizontalAlignment;
        Netcode::ResourceViewsRef backgroundImage;
        Netcode::Float2 backgroundSize;
        Netcode::UInt2 backgroundImageSize;
        Netcode::Float4 backgroundColor;
        Netcode::Float4 borderColor;
        Netcode::Float4 renderedBackgroundColor;
        Netcode::Float4 renderedBorderColor;

        Netcode::BorderType BorderType() const {
            return borderType;
        }

        void BorderType(Netcode::BorderType type) {
            borderType = type;
        }

        Netcode::BackgroundType BackgroundType() const {
            return backgroundType;
        }

        void BackgroundType(Netcode::BackgroundType type) {
            backgroundType = type;
        }

        void BackgroundImageSize(const Netcode::UInt2 & imgSize) {
            backgroundImageSize = imgSize;
        }

        Netcode::SpriteDesc GetSpriteDesc() const {
            if(BackgroundType() == Netcode::BackgroundType::TEXTURE) {
                Netcode::Float2 bgSize = BackgroundSize();

                if(bgSize.x == 0.0f || bgSize.y == 0.0f) {
                    return Netcode::SpriteDesc{ BackgroundImage(), BackgroundImageSize(), RenderedBackgroundColor() };
                } else {
                    HorizontalAnchor ha = BackgroundHorizontalAlignment();
                    VerticalAnchor va = BackgroundVerticalAlignment();

                    Netcode::Float2 controlSize = Size();

                    Netcode::Vector2 anchorDiff = CalculateAnchorOffset(ha, va, controlSize);
                    Netcode::Vector2 anchorOffset = CalculateAnchorOffset(ha, va, bgSize);

                    Netcode::Vector2 topLeftCorner = anchorOffset - anchorDiff;
                    Netcode::Vector2 bottomRightCorner = topLeftCorner + controlSize;

                    Netcode::Float2 tl = topLeftCorner;
                    Netcode::Float2 br = bottomRightCorner;

                    Netcode::Rect sourceRectangle{
                        static_cast<int32_t>(tl.x),
                        static_cast<int32_t>(tl.y),
                        static_cast<int32_t>(br.x),
                        static_cast<int32_t>(br.y)
                    };

                    return Netcode::SpriteDesc{ BackgroundImage(), BackgroundImageSize(), sourceRectangle, RenderedBackgroundColor() };
                }
            }

            if(BackgroundType() == Netcode::BackgroundType::SOLID) {
                return Netcode::SpriteDesc{ RenderedBackgroundColor() };
            }

            return Netcode::SpriteDesc{};
        }

        Netcode::BorderDesc GetBorderDesc() const {
            if(BorderType() == Netcode::BorderType::SOLID) {
                return Netcode::BorderDesc{ BorderWidth(), BorderRadius(), RenderedBorderColor() };
            }

            return Netcode::BorderDesc{};
        }

        void RenderedBackgroundColor(const Netcode::Float4 & clr) {
            renderedBackgroundColor = clr;
        }

        void RenderedBorderColor(const Netcode::Float4 & clr) {
            renderedBorderColor = clr;
        }

    public:
        Netcode::UInt2 BackgroundImageSize() const {
            return backgroundImageSize;
        }

        Panel() : Control{}, borderType{ Netcode::BorderType::NONE }, borderWidth{ 0.0f }, borderRadius{ 0.0f }, backgroundType{ Netcode::BackgroundType::NONE }, backgroundVerticalAlignment{ VerticalAnchor::TOP }, backgroundHorizontalAlignment{ HorizontalAnchor::LEFT },
            backgroundImage{ nullptr }, backgroundSize{ Netcode::Float2::Zero }, backgroundImageSize{ Netcode::UInt2::Zero }, backgroundColor{ Netcode::Float4::Zero }, borderColor{ Netcode::Float4::Zero },
            renderedBackgroundColor{ Netcode::Float4::Zero }, renderedBorderColor{ Netcode::Float4::Zero } { }

        virtual ~Panel() = default;

        Netcode::Float4 RenderedBorderColor() const {
            return renderedBorderColor;
        }

        Netcode::Float4 RenderedBackgroundColor() const {
            return renderedBackgroundColor;
        }

        VerticalAnchor BackgroundVerticalAlignment() const {
            return backgroundVerticalAlignment;
        }

        void BackgroundVerticalAlignment(VerticalAnchor verticalAlignment) {
            backgroundVerticalAlignment = verticalAlignment;
        }

        HorizontalAnchor BackgroundHorizontalAlignment() const {
            return backgroundHorizontalAlignment;
        }

        void BackgroundHorizontalAlignment(HorizontalAnchor horizontalAlignment) {
            backgroundHorizontalAlignment = horizontalAlignment;
        }

        Netcode::Float4 BackgroundColor() const {
            return backgroundColor;
        }

        void ResetBackground() {
            BackgroundImage(nullptr);
            BackgroundColor(Netcode::Float4::Zero);
            BackgroundSize(Netcode::Float2::Zero);
            BackgroundType(Netcode::BackgroundType::NONE);
        }

        void ResetBorder() {
            BorderRadius(0.0f);
            BorderWidth(0.0f);
            BorderColor(Netcode::Float4::Zero);
            BorderType(Netcode::BorderType::NONE);
        }

        float BorderRadius() const {
            return borderRadius;
        }

        void BorderRadius(float br) {
            borderRadius = br;
            BorderType(Netcode::BorderType::SOLID);
        }

        void BorderWidth(float bw) {
            borderWidth = bw;
            BorderType(Netcode::BorderType::SOLID);
        }

        float BorderWidth() const {
            return borderWidth;
        }

        void BorderColor(const Netcode::Float4 & c) {
            borderColor = c;
            RenderedBorderColor(borderColor);
            BorderType(Netcode::BorderType::SOLID);
        }

        Netcode::Float4 BorderColor() const {
            return borderColor;
        }

        Netcode::Float2 BackgroundSize() const {
            return backgroundSize;
        }

        void BackgroundSize(const Netcode::Float2 & bgSizeInPixels) {
            backgroundSize = bgSizeInPixels;
        }

        void BackgroundColor(const Netcode::Float4 & color) {
            if(BackgroundType() == Netcode::BackgroundType::NONE) {
                BackgroundType(Netcode::BackgroundType::SOLID);
            }

            if(color.w == 0.0f) {
                BackgroundType(Netcode::BackgroundType::NONE);
            }

            backgroundColor = color;
            RenderedBackgroundColor(backgroundColor);
        }

        float Opacity() const {
            return backgroundColor.w;
        }

        void Opacity(float w) {
            backgroundColor.w = w;
        }

        Netcode::ResourceViewsRef BackgroundImage() const {
            return backgroundImage;
        }

        void BackgroundImage(std::nullptr_t) {
            BackgroundImage(nullptr, Netcode::UInt2::Zero);
            BackgroundType(Netcode::BackgroundType::SOLID);
        }
        
        void BackgroundImage(Netcode::ResourceViewsRef imageRef, const Netcode::UInt2 & imageSize) {
            backgroundImage = imageRef;
            BackgroundImageSize(imageSize);
            if(imageRef != nullptr) {
                BackgroundType(Netcode::BackgroundType::TEXTURE);
            }
        }

        virtual void Render(Netcode::SpriteBatchPtr batch) override {
            Netcode::SpriteDesc spriteDesc = GetSpriteDesc();
            Netcode::BorderDesc borderDesc = GetBorderDesc();

            if(!spriteDesc.IsEmpty() || !borderDesc.IsEmpty()) {
                batch->DrawSprite(spriteDesc, borderDesc, ScreenPosition(), Size(), RotationOrigin(), RotationZ(), ZIndex());
            }

            Control::Render(batch);
        }
    };

    class StackPanel : public Panel {
    protected:
        Direction stackDirection;

        static Netcode::Float2 ZeroDirection(const Netcode::Float2 & value, Direction dir) {
            if(dir == Direction::VERTICAL) {
                return Netcode::Float2{ value.x, 0.0f };
            } else {
                return Netcode::Float2{ 0.0f, value.y };
            }
        }

        static Direction OppositeDirection(Direction dir) {
            if(dir == Direction::VERTICAL) {
                return Direction::HORIZONTAL;
            } else {
                return Direction::VERTICAL;
            }
        }

    public:

        virtual ~StackPanel() = default;

        StackPanel() : Panel{}, stackDirection{ Direction::VERTICAL } {
            
        }

        Direction StackDirection() const {
            return stackDirection;
        }

        void StackDirection(Direction dir) {
            stackDirection = dir;
        }

        virtual void UpdateLayout() override {
            Panel::UpdateLayout();

            Netcode::Float2 ao = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), Size());
            ao = ZeroDirection(ao, StackDirection());

            const Netcode::Vector2 dirSelect = ZeroDirection(Netcode::Float2::One, OppositeDirection(StackDirection()));
            const Netcode::Vector2 anchorOffset = ao;
            Netcode::Vector2 dirSum = Netcode::Float2::Zero;

            for(auto & child : children) {
                const Netcode::Float2 bs = child->BoxSize();

                Netcode::Float2 ad = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), bs);
                ad = ZeroDirection(ad, StackDirection());

                const Netcode::Vector2 anchorDiff = ad;

                child->Position(anchorOffset - anchorDiff + dirSum);

                dirSum += dirSelect * bs;
            }
        }

        virtual void UpdateSize(const Netcode::Float2 & screenSize) override {
            Control::UpdateSize(screenSize);

            if(Sizing() == SizingType::DERIVED) {
                if(StackDirection() == Direction::VERTICAL) {
                    float maxWidth = 0.0f;
                    float heightSum = 0.0f;
                    for(auto & child : children) {
                        Netcode::Float2 childSize = child->BoxSize();

                        if(maxWidth < childSize.x) {
                            maxWidth = childSize.x;
                        }

                        heightSum += childSize.y;
                    }

                    Size(Netcode::Float2{ maxWidth, heightSum });
                } else {
                    float maxHeight = 0.0f;
                    float widthSum = 0.0f;

                    for(auto & child : children) {
                        Netcode::Float2 childSize = child->BoxSize();

                        if(maxHeight < childSize.y) {
                            maxHeight = childSize.y;
                        }

                        widthSum += childSize.x;
                    }

                    Size(Netcode::Float2{ widthSum, maxHeight });
                }
            }
        }
    };

    class InputGroup : public StackPanel {
    protected:
    public:
        virtual ~InputGroup() = default;

        InputGroup() : StackPanel{} { }
        
        /*
        handle tab key press
        */

        virtual void AddChild(std::shared_ptr<Control> child) override {
            if(!children.empty()) {
                children.clear();
            }
            StackPanel::AddChild(child);
        }
    };

    class ScrollViewer : public Panel {
    protected:
    public:
        virtual ~ScrollViewer() = default;
        ScrollViewer() : Panel{} { }
    };

    class Label : public Panel {
    protected:
        Netcode::SpriteFontRef font;
        Netcode::Float4 textColor;
        Netcode::Float4 renderedTextColor;
        std::wstring text;
        Netcode::Float2 textPosition;


        void TextPosition(const Netcode::Float2 & tp) {
            textPosition = tp;
        }

        Netcode::Float2 TextPosition() const {
            return textPosition;
        }

        void UpdateTextPosition() {
            const Netcode::Float2 textSize = font->MeasureString(text.c_str());
            const Netcode::Vector2 textAnchorOffset = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), Size());
            const Netcode::Vector2 textAnchorDiff = CalculateAnchorOffset(HorizontalContentAlignment(), VerticalContentAlignment(), textSize);
            TextPosition(textAnchorOffset - textAnchorDiff);
        }

        void UpdateTextSize() {
            if(Sizing() == SizingType::DERIVED) {
                Size(font->MeasureString(text.c_str()));
                TextPosition(Netcode::Float2::Zero);
            } else {
                if(text.empty()) {
                    TextPosition(Netcode::Float2::Zero);
                } else {
                    UpdateTextPosition();
                }
            }
        }

        void RenderedTextColor(const Netcode::Float4 & clr) {
            renderedTextColor = clr;
        }

    public:
        virtual ~Label() = default;

        Label() : Panel{}, font{ nullptr }, textColor{ Netcode::Float4::Zero }, renderedTextColor{ Netcode::Float4::Zero }, text{}, textPosition{ Netcode::Float2::Zero } {

        }

        void Font(Netcode::SpriteFontRef ref);
        Netcode::SpriteFontRef Font() const;

        Netcode::Float4 TextColor() const {
            return textColor;
        }

        void TextColor(const Netcode::Float4 & color) {
            textColor = color;
            RenderedTextColor(textColor);
        }

        Netcode::Float4 RenderedTextColor() const {
            return renderedTextColor;
        }


        Netcode::Float3 TextRGB() const {
            return Netcode::Float3{ textColor.x, textColor.y, textColor.z };
        }

        void TextRGB(const Netcode::Float3 & rgb) {
            TextColor(Netcode::Float4{ rgb.x, rgb.y, rgb.z, TextColor().w });
        }

        float TextOpacity() const {
            return textColor.w;
        }

        void TextOpacity(float w) {
            textColor.w = w;
        }

        const std::wstring & Text() const {
            return text;
        }

        void Text(const std::wstring & ws) {
            if(text != ws) {
                text = ws;
                OnTextChanged();
            }
        }

        virtual void Render(Netcode::SpriteBatchPtr batch) override {
            Panel::Render(batch);

            if(font != nullptr && !text.empty()) {
                const Netcode::Vector2 screenPos = ScreenPosition();
                const Netcode::Vector2 textPos = TextPosition();

                font->DrawString(batch, text, screenPos + textPos, RenderedTextColor(), RotationOrigin(), RotationZ(), ZIndex());
            }
        }

        virtual void OnFontChanged() {
            if(font != nullptr) {
                UpdateTextSize();
            }
        }

        virtual void OnTextChanged() {
            if(font != nullptr) {
                UpdateTextSize();
            }
        }
    };


    class Input : public Label {
    protected:
        Netcode::Float4 hoveredBackgroundColor;
        Netcode::Float4 hoveredBorderColor;
        Netcode::Float4 hoveredTextColor;
        Netcode::PxPtr<physx::PxRigidDynamic> actor;
        int32_t tabIndex;
        bool focused;
        bool hovered;

        void UpdateActorPose() {
            Netcode::Vector2 sPos = ScreenPosition();
            Netcode::Vector2 halfSize = Netcode::Vector2{ Size() } / 2.0f;

            Netcode::Float2 sp = sPos + halfSize;

            actor->setGlobalPose(physx::PxTransform{ sp.x, sp.y, ZIndex(), physx::PxQuat{ RotationZ(), physx::PxVec3{ 0.0f, 0.0f, 1.0f } } });
        }

    public:
        virtual ~Input() = default;
        Input(Netcode::PxPtr<physx::PxRigidDynamic> pxActor) : Label{}, actor{ std::move(pxActor) }, tabIndex{ 0 }, focused{ false }, hovered{ false } {
            actor->userData = this;
        }

        int32_t TabIndex() const {
            return tabIndex;
        }

        void TabIndex(int32_t tbIndex) {
            tabIndex = tbIndex;
        }

        bool Focused() const {
            return focused;
        }

        void HoveredBackgroundColor(const Netcode::Float4 & color) {
            hoveredBackgroundColor = color;
        }

        Netcode::Float4 HoveredBackgroundColor() const {
            return hoveredBackgroundColor;
        }

        void HoveredTextColor(const Netcode::Float4 & color) {
            hoveredTextColor = color;
        }

        Netcode::Float4 HoveredTextColor() const {
            return hoveredTextColor;
        }

        void HoveredBorderColor(const Netcode::Float4 & color) {
            hoveredBorderColor = color;
        }

        Netcode::Float4 HoveredBorderColor() const {
            return hoveredBorderColor;
        }

        virtual void OnMouseEnter(MouseEventArgs & evtArgs) override {
            Label::OnMouseEnter(evtArgs);

            RenderedBackgroundColor(HoveredBackgroundColor());
            RenderedBorderColor(HoveredBorderColor());
            RenderedTextColor(HoveredTextColor());

            hovered = true;
        }

        virtual void OnMouseLeave(MouseEventArgs & evtArgs) override {
            Label::OnMouseLeave(evtArgs);

            RenderedBackgroundColor(BackgroundColor());
            RenderedBorderColor(BorderColor());
            RenderedTextColor(TextColor());

            hovered = false;
        }

        void SetDefaultHoverColors() {
            HoveredBackgroundColor(BackgroundColor());
            HoveredBorderColor(BorderColor());
            HoveredTextColor(TextColor());
        }

        virtual void OnSizeChanged() override {
            Label::OnSizeChanged();

            physx::PxShape* shape{ nullptr };
            physx::PxU32 returnedShapes = actor->getShapes(&shape, 1, 0);

            if(returnedShapes == 1) {
                Netcode::Float2 halfSize = Netcode::Vector2{ Size() } / 2.0f;

                shape->setGeometry(physx::PxBoxGeometry{ halfSize.x, halfSize.y, 0.25f });

                UpdateActorPose();
            }
        }

        virtual void Destruct() override {
            physx::PxScene * scene = actor->getScene();

            if(scene != nullptr) {
                scene->removeActor(*actor);
            }

            Label::Destruct();
        }

        virtual void OnPositionChanged() override {
            UpdateActorPose();
        }

        virtual void OnFocused(FocusChangedEventArgs & evtArgs) override {
            if(evtArgs.TabIndex() == TabIndex()) {
                evtArgs.Handled(true);
                focused = true;
            }
        }
    };

    class Button : public Input {
    protected:
    public:
        virtual ~Button() = default;

        using Input::Input;

        virtual void OnClick(MouseEventArgs & evtArgs) override {
            evtArgs.Handled(true);

            Log::Debug("Button");
        }
    };

    class TextBox : public Input {
    protected:
        bool isPassword;


    public:
        virtual ~TextBox() = default;

        TextBox(Netcode::PxPtr<physx::PxRigidDynamic> pxActor) : Input{ std::move(pxActor) }, isPassword{ false } { }

        bool IsPassword() const {
            return isPassword;
        }

        void IsPassword(bool isPw) {
            isPassword = isPw;
        }

        virtual void OnClick(MouseEventArgs & evtArgs) {
            evtArgs.Handled(true);

            Log::Debug("TextBox");
        }

    };

}

/*
Login panel example:
- Panel: center, middle alignment, window sized
    - StackPanel: derived sized, vertical stack alignment
        - StackPanel: derived sized, horizontal stack alignment
            - Label: right content alignment, with text: "Username:", fix sized
            - TextBox: left content alignment, fix sized
        - StackPanel: derived sized, horizontal stack alignment
            - Label: right content alignment, text: "Password:", fix sized
            - TextBox: password switch, left content alignment, fix sized
        - StackPanel: derived sized, horizontal stack alignment
            - Button: center content alignment, text: "Exit", fix sized
            - Button: center content alignment, text: "Login", fix sized

Spinner example:
- Panel: center, middle alignment, window sized
    - StackPanel: derived sized, vertical stack alignment
        - Panel: spinner background, fixed sized, animation on the Rotation property
        - Label: center content alignment, fixed sized
            
*/
