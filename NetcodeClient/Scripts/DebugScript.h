#pragma once

#include "../GameObject.h"
#include <Netcode/Input/Key.h>
#include <Netcode/Input.h>

class TransformDebugScript : public ScriptBase {
	Transform * tr;
	float rotY;
public:
	virtual void BeginPlay(GameObject* gameObject) override {
		tr = gameObject->GetComponent<Transform>();
		rotY = 0.0f;
	}

	virtual void Update(Netcode::GameClock * gameClock) override {
		const float dt = gameClock->FGetDeltaTime();
		
		rotY = std::fmodf(rotY + dt, Netcode::C_2PI);

		tr->rotation = Netcode::Quaternion{ 0.0f, rotY, 0.0f };
	}
};

class DebugScript : public ScriptBase {
	float * refs[4];
	float * curr;
	Netcode::EventToken token;
public:
	DebugScript(float * x = nullptr, float * y = nullptr, float * z = nullptr, float * w = nullptr) : refs{}, curr{ nullptr }, token{} {
		refs[0] = x;
		refs[1] = y;
		refs[2] = z;
		refs[3] = w;
	}

	virtual void BeginPlay(GameObject * owner) override {
		token = Netcode::Input::OnKeyPressed->Subscribe([this](Netcode::Key key, Netcode::KeyModifiers modifiers) -> void {
			int32_t idx = static_cast<int32_t>(key.GetCode()) - static_cast<int32_t>(Netcode::KeyCode::_0);
			if(idx >= 0 && idx < 4) {
				curr = refs[idx];
				Log::Debug("[DebugScript] selected value of index: {0}", idx);
			}

			if(curr != nullptr) {
				float scale = 1.0f;

				if(modifiers.IsSet(Netcode::KeyModifier::CTRL)) {
					scale *= 10.0f;
				} else if(modifiers.IsSet(Netcode::KeyModifier::ALT)) {
					scale /= 10.0f;
				}

				if(key.GetCode() == Netcode::KeyCode::ADD) {
					*curr += scale;
					Log::Debug("[DebugScript] value increased to {0}", std::to_string(*curr));
				}

				if(key.GetCode() == Netcode::KeyCode::SUBTRACT) {
					*curr -= scale;
					Log::Debug("[DebugScript] value decreased to {0}", std::to_string(*curr));
				}
			}

		});
	}

	virtual void Update(Netcode::GameClock*) override {
		
	}
};

