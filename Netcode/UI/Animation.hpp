#pragma once

#include <Netcode/HandleDecl.h>
#include <NetcodeFoundation/Math.h>
#include <vector>
#include <memory>

namespace Netcode::UI {

    using AnimationTimeFunc = float(*)(float);

    class Control;
	
    class Animation {
    protected:
        AnimationTimeFunc timeFunc;
        float duration;
        float time;

    public:
        Animation(AnimationTimeFunc timeFunc, float duration) : timeFunc{ timeFunc }, duration{ duration }, time{ 0.0f } { }

        void SetTime(float animTime) {
            time = animTime;
        }

        virtual ~Animation() = default;
        virtual void Run(Control * ctrl, float dt) = 0;
        virtual bool IsDone() const = 0;
    };

    class AnimationContainer {
        std::vector<std::unique_ptr<Animation>> animations;
        uint32_t numAnimations;
    public:

        AnimationContainer() : animations{}, numAnimations{ 0 } { }

        void Splice(uint32_t keepFirstN) {
        	if(animations.size() <= keepFirstN) {
                return;
        	}
            animations.erase(std::begin(animations) + keepFirstN, std::end(animations));
            numAnimations = keepFirstN;
        }
    	
        void Update(Control * ctrl, float dt) {
            for(uint32_t i = 0; i < numAnimations;) {
                Animation * anim = animations[i].get();

                anim->Run(ctrl, dt);

                if(anim->IsDone()) {
                    for(uint32_t j = i; j < numAnimations - 1; ++j) {
                        std::swap(animations[j], animations[j + 1]);
                    }
                    numAnimations -= 1;
                } else {
                    ++i;
                }
            }
        }

        void RestartAnimation(uint32_t idx) {
            if(idx < numAnimations) {
                animations[idx]->SetTime(0.0f);
            }
        }

        void Add(std::unique_ptr<Animation> anim) {
            if(static_cast<uint32_t>(animations.size()) > numAnimations) {
                animations[numAnimations++] = std::move(anim);
            } else {
                numAnimations++;
                animations.emplace_back(std::move(anim));
            }
        }

    	uint32_t Count() const {
            return numAnimations;
        }
    	
        void Clear() {
            numAnimations = 0;
            animations.clear();
        }

        bool Empty() const {
            return numAnimations == 0;
        }

    };

    template<typename VectorType>
    class Interpolator {
        VectorType startValue;
        VectorType endValue;
    public:
        Interpolator(VectorType startValue, VectorType endValue) : startValue{ startValue }, endValue{ endValue } {

        }

        VectorType operator()(VectorType vec, float value) const {
            return VectorType::Lerp(startValue, endValue, value);
        }
    };

    template<>
    class Interpolator<float> {
        float startValue;
        float endValue;
    public:
        Interpolator(float startValue, float endValue) : startValue{ startValue }, endValue{ endValue } {

        }

        float operator()(float currentValue, float t) const {
            return startValue * (1.0f - t) + t * endValue;
        }
    };

    class RepeatBehaviour {
        uint64_t times;
        uint64_t counter;
    public:
        RepeatBehaviour(uint64_t nTimes = std::numeric_limits<uint64_t>::max()) : times{ nTimes }, counter{ 0 } { }

        float operator()(float currentValue, float duration) {
            if(currentValue >= duration) {
                counter++;
                return ::fmodf(currentValue, duration);
            }

            return currentValue;
        }

        bool IsDone() const {
            return times <= counter;
        }
    };

    class PlayOnceBehaviour {
        float delay;
        bool isDone;
    public:

        PlayOnceBehaviour(float delay = 0.0f) : delay{ delay }, isDone { false } { }

        float operator()(float currentValue, float duration) {
            if(isDone) {
                return duration;
            }

            if(delay > 0.0f) {
                delay -= currentValue;
                return 0.0f;
            }
            

            if(currentValue >= duration) {
                isDone = true;
                return duration;
            }

            return currentValue;
        }

        bool IsDone() const {
            return isDone;
        }
    };

    template<typename CtrlType, typename GetterFunc, typename SetterFunc, typename Animator, typename Behaviour>
    class PropertyAnimation : public Animation {
        GetterFunc getter;
        SetterFunc setter;
        Animator animator;
        Behaviour behaviour;
    public:

        PropertyAnimation(AnimationTimeFunc timeFunc, float duration, GetterFunc getter, SetterFunc setter, Animator animator, Behaviour behav) :
            Animation{ timeFunc, duration }, getter{ getter }, setter{ setter }, animator{ animator }, behaviour{ behav } {

        }

        virtual void Run(Control * control, float dt) override {
            Animation::time = behaviour(Animation::time + dt, Animation::duration);

            float t = Animation::timeFunc(Animation::time / Animation::duration);

            auto currentValue = ((*static_cast<const CtrlType *>(control)).*getter)();
            auto updatedValue = animator(currentValue, t);

            ((*static_cast<CtrlType*>(control)).*setter)(updatedValue);
        }

        virtual bool IsDone() const override {
            return behaviour.IsDone();
        }
    };

    template<typename CtrlType, typename PropertyType, typename Animator, typename Behaviour>
    std::unique_ptr<Animation> MakeAnimation(PropertyType(CtrlType:: * getter)() const, void (CtrlType:: * setter)(const PropertyType &), Animator animator, Behaviour behaviour, AnimationTimeFunc timeFunc, float duration) {
        return std::make_unique<PropertyAnimation<CtrlType, PropertyType(CtrlType:: *)() const, void (CtrlType:: *)(const PropertyType &), Animator, Behaviour>>(timeFunc, duration, getter, setter, animator, behaviour);
    }

    template<typename CtrlType, typename PropertyType, typename Animator, typename Behaviour>
    std::unique_ptr<Animation> MakeAnimation(PropertyType(CtrlType:: * getter)() const, void (CtrlType:: * setter)(PropertyType), Animator animator, Behaviour behaviour, AnimationTimeFunc timeFunc, float duration) {
        return std::make_unique<PropertyAnimation<CtrlType, PropertyType(CtrlType:: *)() const, void (CtrlType:: *)(PropertyType), Animator, Behaviour>>(timeFunc, duration, getter, setter, animator, behaviour);
    }

}
