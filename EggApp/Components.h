#pragma once

#include <functional>
#include <string>
#include "ConstantBufferTypes.h"
#include <EggAssetLib/Collider.h>

using ColliderShape = Egg::Asset::Collider;

#define COMPONENT_ALIGN __declspec(align(16))

COMPONENT_ALIGN struct Sprite {

};

COMPONENT_ALIGN struct Button {
public:
	uint32_t width;
	uint32_t height;
	std::function<void()> onClick;
	std::function<void()> onMouseEnter;
	std::function<void()> onMouseLeave;
	bool isMouseOver;
};

COMPONENT_ALIGN struct Text {
public:
	wchar_t text[128];
};

COMPONENT_ALIGN struct LongText {
public:
	std::wstring text;
};

class GameObject;

class IBehavior {
public:
	virtual ~IBehavior() = default;
	virtual void Setup(GameObject * gameObject) = 0;
	virtual void Update(float dt) = 0;
};

COMPONENT_ALIGN class Script {
protected:
	std::unique_ptr<IBehavior> behavior;
public:
	void SetBehavior(std::unique_ptr<IBehavior> behavior);
	void Setup(GameObject * owner);
	void Update(float dt);

	Script() = default;

	Script(Script &&) noexcept = default;
	Script & operator=(Script &&) noexcept = default;
};

COMPONENT_ALIGN class Transform {
public:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 worldPosition;
	DirectX::XMFLOAT4 worldRotation;

	Transform() : position{ 0.0f, 0.0f, 0.0f }, rotation{ 0.0f, 0.0f, 0.0f, 1.0f }, scale{ 1.0f, 1.0f, 1.0f }, worldPosition{ position }, worldRotation{ rotation } {

	}

	Transform(Transform &&) noexcept = default;
	Transform & operator=(const Transform &) = default;
	Transform & operator=(Transform &&) noexcept = default;
	~Transform() noexcept = default;
};

struct ShadedMesh {
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

	ShadedMesh(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat) : mesh{ std::move(m) }, material{ std::move(mat) } {

	}

	ShadedMesh(ShadedMesh &&) noexcept = default;
	~ShadedMesh() noexcept = default;
	ShadedMesh & operator=(ShadedMesh && sm) noexcept = default;
};

COMPONENT_ALIGN class Model {
public:

	PerObjectData perObjectData;
	std::unique_ptr<BoneData> boneData;
	std::vector<ShadedMesh> meshes;

	Model() : perObjectData{ }, boneData{ nullptr }, meshes{ } { }
	~Model() = default;

	Model(Model &&) noexcept = default;
	Model & operator=(Model && m) noexcept = default;

	ShadedMesh & AddShadedMesh(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat) {
		return meshes.emplace_back(std::move(m), std::move(mat));
	}
};


COMPONENT_ALIGN class Camera {
public:
	DirectX::XMFLOAT3 ahead;
	DirectX::XMFLOAT3 up;

	float fov;
	float aspect;
	float nearPlane;
	float farPlane;

	Camera() : ahead{}, up{ 0.0f, 1.0f, 0.0f }, fov{ 1.0f }, aspect{ 1.0f }, nearPlane{ 0.0f }, farPlane{ 1.0f } { }

	void SetProj(float fov, float aspect, float nearPlane, float farPlane) {
		this->fov = fov;
		this->aspect = aspect;
		this->nearPlane = nearPlane;
		this->farPlane = farPlane;
	}

	Camera & operator=(const Camera &) = default;
};

COMPONENT_ALIGN class Animation {
public:
	Egg::Animation::Blackboard blackboard;

	Animation() = default;
	Animation(Animation &&) noexcept = default;
	Animation & operator=(Animation &&) noexcept = default;
};

COMPONENT_ALIGN class Collider {
public:
	physx::PxActor * actorRef;
	std::vector<ColliderShape> shapes;

	Collider() = default;
	Collider(Collider &&) noexcept = default;

	Collider & operator=(Collider && c) noexcept = default;
};