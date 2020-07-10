#pragma once

#include <memory>

#include <Netcode/HandleTypes.h>
#include <Netcode/Graphics/GraphicsContexts.h>

#include "EventArgs.h"

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
        Netcode::Float2 anchorOffset;
        Netcode::Float4 margin;
        Netcode::Float4 padding;
        float rotationZ;
        SizingType sizing;
        HorizontalAnchor horizontalContentAlignment;
        VerticalAnchor verticalContentAlignment;
        std::shared_ptr<Control> parent;
        std::vector<std::shared_ptr<Control>> children;
        bool enabled;

        void AnchorOffset(const Netcode::Float2 & layoutPos);

        static Netcode::Float2 CalculateAnchorOffset(HorizontalAnchor xAnchor, VerticalAnchor yAnchor, const Netcode::Float2 & controlSize);

    public:
        Control();

        virtual ~Control() = default;

        HorizontalAnchor HorizontalContentAlignment() const;

        void HorizontalContentAlignment(HorizontalAnchor horizontalAnchor);

        VerticalAnchor VerticalContentAlignment() const;

        void VerticalContentAlignment(VerticalAnchor verticalAnchor);

        SizingType Sizing() const;

        void Sizing(SizingType sizingType);
     
        Netcode::Float2 Size() const;

        void Size(const Netcode::Float2 & sz);

        Netcode::Float2 Position() const;

        void Position(const Netcode::Float2 & pos);

        Netcode::Float2 ScreenPosition() const;

        Netcode::Float2 AnchorOffset() const;

        void Parent(std::shared_ptr<Control> newParent);

        std::shared_ptr<Control> Parent() const;

        const std::vector<std::shared_ptr<Control>> & Children() const;

        void RotationZ(float rotZ);

        float RotationZ() const;

        virtual void UpdateSize();

        virtual void UpdateLayout();

        virtual void Destruct();

        virtual void Enable();

        virtual void Disable();

        virtual void AddChild(std::shared_ptr<Control> child);

        virtual void OnScreenResized(const Netcode::UInt2 & newSize);

        virtual void OnFocused(FocusChangedEventArgs & args);

        virtual void OnInitialized();

        virtual void OnLayoutChanged();

        virtual void OnKeyDown(KeyboardEventArgs & args);

        virtual void OnKeyUp(KeyboardEventArgs & args);

        virtual void OnClick(MouseEventArgs & args);

        virtual void OnMouseEnter(MouseEventArgs & args);

        virtual void OnMouseLeave(MouseEventArgs & args);

        virtual void OnRender(Netcode::SpriteBatchRef batch);

        virtual void OnEnabled();

        virtual void OnDisabled();

        virtual void OnSizeChanged();

        virtual bool IsFocused();

        virtual bool IsEnabled();
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
                    return Netcode::SpriteDesc{ BackgroundImage(), BackgroundImageSize(), BackgroundColor() };
                } else {
                    HorizontalAnchor ha = BackgroundHorizontalAlignment();
                    VerticalAnchor va = BackgroundVerticalAlignment();

                    Netcode::UInt2 imgSize = BackgroundImageSize();

                    Netcode::Float2 imgSizeAsFloat = Netcode::Float2{
                        static_cast<float>(imgSize.x),
                        static_cast<float>(imgSize.y)
                    };

                    Netcode::Vector2 anchorOffset = CalculateAnchorOffset(ha, va, imgSizeAsFloat);
                    Netcode::Vector2 anchorDiff = CalculateAnchorOffset(ha, va, bgSize);

                    Netcode::Vector2 topLeftCorner = anchorOffset - anchorDiff;
                    Netcode::Vector2 bottomRightCorner = topLeftCorner + imgSizeAsFloat;

                    Netcode::Float2 tl = topLeftCorner;
                    Netcode::Float2 br = bottomRightCorner;

                    Netcode::Rect sourceRectangle{
                        static_cast<int32_t>(tl.x),
                        static_cast<int32_t>(tl.y),
                        static_cast<int32_t>(br.x),
                        static_cast<int32_t>(br.y)
                    };

                    return Netcode::SpriteDesc{ BackgroundImage(), BackgroundImageSize(), sourceRectangle, BackgroundColor() };
                }
            }

            if(BackgroundType() == Netcode::BackgroundType::SOLID) {
                return Netcode::SpriteDesc{ BackgroundColor() };
            }

            return Netcode::SpriteDesc{};
        }

        Netcode::BorderDesc GetBorderDesc() const {
            if(BorderType() == Netcode::BorderType::SOLID) {
                return Netcode::BorderDesc{ BorderWidth(), BorderRadius(), BorderColor() };
            }

            return Netcode::BorderDesc{};
        }

    public:
        Netcode::UInt2 BackgroundImageSize() const {
            return backgroundImageSize;
        }

        Panel() : Control{}, borderType{ Netcode::BorderType::NONE }, borderWidth{ 0.0f }, borderRadius{ 0.0f }, backgroundType{ Netcode::BackgroundType::NONE }, backgroundVerticalAlignment{ VerticalAnchor::TOP }, backgroundHorizontalAlignment{ HorizontalAnchor::LEFT },
            backgroundImage{ nullptr }, backgroundSize{ Netcode::Float2::Zero }, backgroundImageSize{ Netcode::UInt2::Zero }, backgroundColor{ Netcode::Float4::Zero }, borderColor{ Netcode::Float4::Zero } { }

        virtual ~Panel() = default;

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
        }

        void BorderWidth(float bw) {
            BorderType(Netcode::BorderType::SOLID);
            borderWidth = bw;
        }

        float BorderWidth() const {
            return borderWidth;
        }

        void BorderColor(const Netcode::Float4 & c) {
            borderColor = c;
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

        virtual void OnRender(Netcode::SpriteBatchRef batch) {
            Netcode::SpriteDesc spriteDesc = GetSpriteDesc();
            Netcode::BorderDesc borderDesc = GetBorderDesc();

            if(!spriteDesc.IsEmpty() || !borderDesc.IsEmpty()) {
                batch->DrawSprite(spriteDesc, borderDesc, ScreenPosition(), Size());
            }

            Control::OnRender(batch);
        }
    };

    class StackPanel : public Panel {
    protected:
        Direction stackDirection;
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

        virtual void UpdateSize() override {
            Control::UpdateSize();

            if(Sizing() == SizingType::DERIVED) {
                if(StackDirection() == Direction::VERTICAL) {
                    float maxWidth = 0.0f;
                    float heightSum = 0.0f;
                    for(auto & child : children) {
                        Netcode::Float2 childSize = child->Size();

                        if(maxWidth < childSize.x) {
                            maxWidth = childSize.x;
                        }

                        child->Position(Netcode::Float2{ 0.0f, heightSum });

                        heightSum += childSize.y;
                    }

                    Size(Netcode::Float2{ maxWidth, heightSum });
                } else {
                    float maxHeight = 0.0f;
                    float widthSum = 0.0f;

                    for(auto & child : children) {
                        Netcode::Float2 childSize = child->Size();

                        if(maxHeight < childSize.y) {
                            maxHeight = childSize.y;
                        }

                        child->Position(Netcode::Float2{ widthSum, 0.0f });

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

        /*
        mouse enter, mouse leave, static currently focused ptr
        mouse wheel input
        override OnRender method
            use rendercontext to set ScissorRect
        */
    };

    class Label : public Panel {
    protected:
        Netcode::SpriteFontRef font;
        Netcode::Float4 textColor;
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

    public:
        virtual ~Label() = default;

        Label() : Panel{}, font{ nullptr }, textColor{ Netcode::Float4::Zero }, text{}, textPosition{ Netcode::Float2::Zero } {

        }

        void Font(Netcode::SpriteFontRef ref);
        Netcode::SpriteFontRef Font() const;

        Netcode::Float4 TextColor() const {
            return textColor;
        }

        void TextColor(const Netcode::Float4 & color) {
            textColor = color;
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

        virtual void OnRender(Netcode::SpriteBatchRef batch) override {
            if(font != nullptr && !text.empty()) {
                const Netcode::Vector2 screenPos = ScreenPosition();
                const Netcode::Vector2 textPos = TextPosition();

                font->DrawString(batch, text.c_str(), screenPos + textPos, TextColor());
            }

            Panel::OnRender(batch);
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
        int32_t tabIndex;
        bool focused;

    public:
        virtual ~Input() = default;
        Input() : Label{}, tabIndex{ 0 }, focused{ false } { }

        int32_t TabIndex() const {
            return tabIndex;
        }

        void TabIndex(int32_t tbIndex) {
            tabIndex = tbIndex;
        }

        bool Focused() const {
            return focused;
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
        Button() : Input{} { }
    };

    class TextBox : public Input {
    protected:
        bool isPassword;


    public:
        virtual ~TextBox() = default;

        TextBox() : Input{}, isPassword{ false } { }

        bool IsPassword() const {
            return isPassword;
        }

        void IsPassword(bool isPw) {
            isPassword = isPw;
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