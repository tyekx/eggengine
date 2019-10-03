#pragma once

#include "EggMpl.hpp"
#include "Components.h"
#include <iostream>
#include "GameObjectObserver.h"
#include "EggSceneConf.h"



namespace Egg {
	struct IGameObjectObserver;

	class GameObject {
		constexpr static unsigned int DataSize = TupleSizeofSum<COMPONENTS_T>::value;
		unsigned int Id;
		GameObject * Owner;
		SignatureType Signature;
		uint8_t Data[DataSize];
		std::string Name;

		EGG_DEBUG_ENGINE_DEFINE_OBSERVER(Egg::IGameObjectObserver)

	public:
		GameObject(unsigned int id) : Id{ id }, Owner { nullptr }, Signature{ 0ULL }, Data{} EGG_DEBUG_ENGINE_CONSTRUCT_OBSERVER {
			memset(Data, 0, DataSize);
		}

		unsigned int GetId() const {
			return Id;
		}

		SignatureType GetSignature() const {
			return Signature;
		}

		const std::string & GetName() const {
			return Name;
		}

		void SetName(const std::string & name);

		~GameObject();

		void SetOwner(GameObject * o);

		GameObject * GetOwner() const {
			return Owner;
		}

		template<typename T>
		bool HasComponent() {
			return (Signature & (1ULL << TupleIndexOf<T, COMPONENTS_T>::value)) > 0ULL;
		}

		template<typename T>
		T * GetComponent() {
			if(HasComponent<T>()) {
				return reinterpret_cast<T *>(Data + TupleOffsetOf<T, COMPONENTS_T>::value);
			}
			return nullptr;
		}

		template<typename T, typename ... U>
		T * AddComponent(U && ... args) {
			if(!HasComponent<T>()) {
				T * ptr = reinterpret_cast<T *>(Data + TupleOffsetOf<T, COMPONENTS_T>::value);
				new (ptr) T(std::forward<U>(args)...);
				Signature |= (1ULL << TupleIndexOf<T, COMPONENTS_T>::value);
				EGG_DEBUG_ENGINE_NOTIFY_OBSERVER(GameObjectSignatureChanged, this);
				return ptr;
			}
			return nullptr;
		}

		template<typename T>
		void RemoveComponent() {
			if(HasComponent<T>()) {
				T * ptr = reinterpret_cast<T *>(Data + TupleOffsetOf<T, COMPONENTS_T>::value);
				ptr->~T();
				Signature &= (~(1ULL << TupleIndexOf<T, COMPONENTS_T>::value));
				EGG_DEBUG_ENGINE_NOTIFY_OBSERVER(GameObjectSignatureChanged, this);
			}
		}

	};
}