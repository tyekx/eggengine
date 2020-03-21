#pragma once

#include "UIControl.h"

class UILabel : public UIControl {
	UIObject * backgroundObject;
	UIObject * textObject;
public:

	virtual UIObject * GetRoot() override {
		return backgroundObject;
	}
	
	void SetTextAlignment(HorizontalAnchor hAlign) {
		Text * textComponent = textObject->GetComponent<Text>();
		textComponent->horizontalAlignment = hAlign;
		AlignText(textObject->GetComponent<Text>(), textObject->GetComponent<Transform>(), backgroundObject->GetComponent<UIElement>());
	}

	void SetSize(const DirectX::XMFLOAT2 & size) {
		UIElement * bgElem = backgroundObject->GetComponent<UIElement>();
		bgElem->width = size.x;
		bgElem->height = size.y;
		AlignText(textObject->GetComponent<Text>(), textObject->GetComponent<Transform>(), bgElem);
	}

	void SetText(std::wstring text) {
		Text * textComponent = textObject->GetComponent<Text>();
		textComponent->text = std::move(text);
		AlignText(textObject->GetComponent<Text>(), textObject->GetComponent<Transform>(), backgroundObject->GetComponent<UIElement>());
	}

	void SetFont(Egg::SpriteFontRef spriteFont) {
		Text * textComponent = textObject->GetComponent<Text>();
		textComponent->font = std::move(spriteFont);
		AlignText(textComponent, textObject->GetComponent<Transform>(), backgroundObject->GetComponent<UIElement>());
	}

	void SetZIndex(float z) {
		Transform * bgTransform = backgroundObject->GetComponent<Transform>();
		bgTransform->position.z = z;
	}

	UILabel(UIObject * bg, UIObject * tx) : backgroundObject{ bg }, textObject{ tx } {
		textObject->Parent(backgroundObject);

		Transform * bgTransform = backgroundObject->AddComponent<Transform>();
		UIElement * bgElem = backgroundObject->AddComponent<UIElement>();
		bgElem->width = 0.0f;
		bgElem->height = 0.0f;

		Transform * textTransform = textObject->AddComponent<Transform>();
		UIElement * textElem = textObject->AddComponent<UIElement>();
		Text * txt = textObject->AddComponent<Text>();
		txt->horizontalAlignment = HorizontalAnchor::LEFT;
		txt->verticalAlignment = VerticalAnchor::TOP;
		txt->color = DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f };
	}
};
