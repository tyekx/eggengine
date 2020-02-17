#pragma once

#include "AnimationState.h"
#include "Transition.h"
#include <EggAssetLib/Model.h>
#include <vector>
#include "LinearClassifier.h"
#include "AnimationBlender.h"

namespace Egg::Animation {



	class Blackboard {
		Memory::LinearClassifier classifier;

		AnimationState * prevState;
		AnimationState * currentState;

		AnimationState * states;
		unsigned int statesLength;

		AnimationBlender * blender;


		AnimationState * FindReferenceByName(const std::string & name);

		void CheckCurrentTransitions(MovementController * movCtrl);

	public:

		Blackboard();

		void CreateResources(Asset::Model * model,
							 const std::initializer_list<AnimationState> & states,
							 const std::initializer_list<TransitionInit> & transitions);

		void Update(float dt, MovementController * movCtrl);

		void CopyBoneDataInto(void * dest);
		void CopyToRootDataInto(void * dest);
	};

}
