#pragma once

#include <DirectXColors.h>

#include <Netcode/Input.h>
#include <Netcode/BasicGeometry.h>
#include <Netcode/DebugPhysx.h>
#include <Netcode/NetcodeMath.h>
#include <Netcode/Modules.h>
#include <Netcode/Stopwatch.h>
#include <Netcode/Service.hpp>

#include "Asset.h"
#include "GameObject.h"
#include "Systems.h"
#include "Scene.h"
#include "DevCameraScript.h"
#include "PlayerBehavior.h"
#include "Snippets.h"
#include "PhysxHelpers.h"
#include "Services.h"
#include "RemoteAvatarScript.h"
#include "Layer.h"

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;

class GameApp : public Netcode::Module::AApp, Netcode::Module::TAppEventHandler {
	Netcode::Stopwatch stopwatch;
	Netcode::Physics::PhysX px;
	physx::PxMaterial * defaultPhysxMaterial;
	
	Netcode::MovementController movCtrl;

	TransformSystem transformSystem;
	ScriptSystem scriptSystem;
	RenderSystem renderSystem;
	AnimationSystem animSystem;
	PhysXSystem pxSystem;
	UISystem uiSystem;
	UISpriteSystem uiSpriteSystem;
	UITransformSystem uiTransformSystem;
	UITextSystem uiTextSystem;
	UIAnimSystem uiAnimSystem;

	GameScene * gameScene;
	UIScene * uiScene;

	std::unique_ptr<Layer> menuLayer;

	float totalTime;

	void LoadSystems() {
		uiSystem.CreateResources(uiScene->GetPhysXScene(), &uiScene->perFrameData);
		uiTextSystem.gEngine = &renderSystem.renderer;
		uiSpriteSystem.gEngine = &renderSystem.renderer;
	}

	void Render() {
		graphics->frame->Prepare();
		
		renderSystem.renderer.perFrameData = &gameScene->perFrameData;
		renderSystem.renderer.ssaoData = &gameScene->ssaoData;

		gameScene->Foreach([this](GameObject * gameObject) -> void {
			if(gameObject->IsActive()) {
				transformSystem.Run(gameObject);
				pxSystem.Run(gameObject);
				renderSystem.Run(gameObject);
			}
		});

		gameScene->UpdatePerFrameCb();

		uiScene->Update();
		uiSystem.Raycast();

		uiScene->Foreach([this](UIObject * uiObject) -> void {
			uiTransformSystem.Run(uiObject);
			uiSystem.Run(uiObject);
			uiSpriteSystem.Run(uiObject);
			uiTextSystem.Run(uiObject);
		});

		auto builder = graphics->CreateFrameGraphBuilder();
		renderSystem.renderer.CreateFrameGraph(builder);

		graphics->frame->Run(builder->Build());
		graphics->frame->Render();
		graphics->frame->Present();
		
		renderSystem.renderer.Reset();
	}

	void Simulate(float dt) {
		totalTime += dt;

		gameScene->GetPhysXScene()->simulate(dt);
		gameScene->GetPhysXScene()->fetchResults(true);

		uiScene->Foreach([this, dt](UIObject * uiObject) -> void {
			if(uiObject->IsActive()) {
				uiAnimSystem.Run(uiObject, dt);
			}
		});

		gameScene->Foreach([this, dt](GameObject * gameObject)->void {
			if(gameObject->IsActive()) {
				scriptSystem.Run(gameObject, dt);
				animSystem.Run(gameObject, dt);
			}
		});
	}

	void LoadServices() {
		Service::Init<AssetManager>();
		Service::Init<GameScene>(px);
		Service::Init<UIScene>(px);

		gameScene = Service::Get<GameScene>();
		uiScene = Service::Get<UIScene>();

		gameScene->Setup();
	}

	void ConnectServer() {
		Netcode::Network::SessionConfig config;
		config.tickIntervalMs = 500;
		config.selfAddress = boost::asio::ip::address::from_string("127.0.0.1");
		config.clientPort = 8889;

		network->CreateClient(config);
	}

	void LoadAssets() {
		AssetManager * assetManager = Service::Get<AssetManager>();

		uiSystem.gEngine = &renderSystem.renderer;

		menuLayer = std::make_unique<MainMenuLayer>(uiScene);
		menuLayer->Construct(this);
		menuLayer->Activate();

		Netcode::Input::SetAxis("Vertical", 'W', 'S');
		Netcode::Input::SetAxis("Horizontal", 'A', 'D');
		Netcode::Input::SetAxis("Jump", VK_SPACE, 0);
		Netcode::Input::SetAxis("Fire", VK_LBUTTON, 0);

		renderSystem.CreatePermanentResources(graphics.get());
		defaultPhysxMaterial = px.physics->createMaterial(0.5f, 0.5f, 0.5f);

		CreateLocalAvatar();
		CreateRemoteAvatar();

		animSystem.SetMovementController(&movCtrl);

		{
			auto planeActor = physx::PxCreatePlane(*px.physics, physx::PxPlane{ 0.0f, 1.0f, 0.0f, 200.0f }, *defaultPhysxMaterial);
			gameScene->SpawnPhysxActor(planeActor);
		}

		GameObject * testbox = gameScene->Create();
		
		LoadComponents(assetManager->Import(L"testbox.ncasset"), testbox);

		gameScene->Spawn(testbox);
	}

	void InitializeBoneData(Model* modelComponent) {
		const DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
		modelComponent->boneData = std::make_unique<BoneData>();
		for(unsigned int i = 0; i < BoneData::MAX_BONE_COUNT; ++i) {
			DirectX::XMStoreFloat4x4A(&modelComponent->boneData->BindTransform[i], identity);
			DirectX::XMStoreFloat4x4A(&modelComponent->boneData->ToRootTransform[i], identity);
		}
	}

	void CreateRemoteAvatar() {
		AssetManager * assetManager = Service::Get<AssetManager>();

		GameObject * avatarController = gameScene->Create();
		GameObject * avatarHitboxes = gameScene->Create();

		Netcode::Asset::Model * avatarModel = assetManager->Import(L"test.ncasset");

		LoadComponents(avatarModel, avatarHitboxes);
		Animation* anim = avatarHitboxes->AddComponent<Animation>();
		CreateYbotAnimationComponent(avatarModel, anim);

		physx::PxController * pxController = gameScene->CreateController();
		avatarController->AddComponent<Transform>();
		avatarHitboxes->Parent(avatarController);

		avatarController->AddComponent<Script>()->SetBehavior(std::make_unique<RemoteAvatarScript>(pxController));

		gameScene->Spawn(avatarController);
		gameScene->Spawn(avatarHitboxes);
	}

	void CreateLocalAvatar() {
		GameObject * avatarController = gameScene->Create();
		GameObject * avatarCamera = gameScene->Create();

		avatarCamera->Parent(avatarController);

		Transform* avatarCamTransform =  avatarCamera->AddComponent<Transform>();
		avatarCamTransform->position.y = 180.0f;

		Camera * fpsCam = avatarCamera->AddComponent<Camera>();
		fpsCam->ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };
		fpsCam->aspect = graphics->GetAspectRatio();
		fpsCam->farPlane = 10000.0f;
		fpsCam->nearPlane = 1.0f;
		fpsCam->up = DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f };


		physx::PxController * pxController = gameScene->CreateController();

		Transform * act = avatarController->AddComponent<Transform>();
		Script* scriptComponent = avatarController->AddComponent<Script>();
		scriptComponent->SetBehavior(std::make_unique<PlayerBehavior>(pxController, fpsCam));
		scriptComponent->Setup(avatarController);

		gameScene->Spawn(avatarController);
		gameScene->Spawn(avatarCamera);

		gameScene->SetCamera(avatarCamera);
	}

	void LoadComponents(Netcode::Asset::Model * model, GameObject * gameObject) {
		gameObject->AddComponent<Transform>();

		if(model->meshes.Size() > 0) {
			Model * modelComponent = gameObject->AddComponent<Model>();
			LoadModelComponent(model, modelComponent);

			if(model->bones.Size() > 0 && model->animations.Size() > 0) {
				InitializeBoneData(modelComponent);
			}
		}

		if(model->colliders.Size() > 0) {
			Collider * colliderComponent = gameObject->AddComponent<Collider>();
			LoadColliderComponent(model, colliderComponent);
			colliderComponent->actorRef->userData = gameObject;
		}
	}

	void LoadColliderComponent(Netcode::Asset::Model * model, Collider * colliderComponent) {
		if(model->colliders.Size() == 0) {
			return;
		}

		physx::PxPhysics * pxp = px.physics;
		physx::PxRigidDynamic * actor = pxp->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
		actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		std::vector<ColliderShape> eggShapes;
		eggShapes.reserve(model->colliders.Size());

		for(size_t i = 0; i < model->colliders.Size(); ++i) {
			Netcode::Asset::Collider * eggCollider = model->colliders.Data() + i;
			eggShapes.push_back(*eggCollider);
			physx::PxShape * shape = nullptr;
			physx::PxShapeFlags shapeFlags;
			physx::PxFilterData filterData;

			if(eggCollider->boneReference >= 0 && eggCollider->boneReference < 0x7F) {
				shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eVISUALIZATION;

				filterData.word0 = PHYSX_COLLIDER_TYPE_HITBOX;
			} else {
				shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION;
				filterData.word0 = PHYSX_COLLIDER_TYPE_WORLD;
			}

			if(eggCollider->type == Netcode::Asset::ColliderType::MESH) {
				const auto & mesh = model->meshes[0];
				physx::PxConvexMeshDesc cmd;
				cmd.points.count = mesh.lodLevels[0].vertexCount;
				cmd.points.data = mesh.vertices;
				cmd.points.stride = mesh.vertexSize;
				cmd.vertexLimit = 255;
				cmd.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eFAST_INERTIA_COMPUTATION;
				cmd.indices.data = mesh.indices;
				cmd.indices.count = mesh.lodLevels[0].indexCount;
				cmd.indices.stride = 4;

				physx::PxDefaultMemoryOutputStream buf;
				physx::PxConvexMeshCookingResult::Enum r;
				px.cooking->cookConvexMesh(cmd, buf, &r);

				physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
				physx::PxConvexMesh * convexMesh = pxp->createConvexMesh(input);

				physx::PxConvexMeshGeometry pcmg{ convexMesh };

				shape = pxp->createShape(pcmg, *defaultPhysxMaterial, true, shapeFlags);

			} else {
				shape = CreatePrimitiveShapeFromAsset(*eggCollider, pxp, defaultPhysxMaterial, shapeFlags);
			}

			shape->setQueryFilterData(filterData);
			shape->userData = &eggShapes.at(i);
			actor->attachShape(*shape);
		}

		colliderComponent->actorRef = actor;
		colliderComponent->shapes = std::move(eggShapes);
	}

	void LoadModelComponent(Netcode::Asset::Model * model, Model * modelComponent) {
		for(size_t meshIdx = 0; meshIdx < model->meshes.Size(); ++meshIdx) {
			Netcode::Asset::Material * mat = model->materials.Data() + model->meshes[meshIdx].materialId;
			Netcode::Asset::Mesh * mesh = model->meshes.Data() + meshIdx;

			DirectX::XMStoreFloat4x4A(&modelComponent->perObjectData.Model, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4A(&modelComponent->perObjectData.InvModel, DirectX::XMMatrixIdentity());

			Netcode::InputLayoutBuilderRef inputLayoutBuilder = graphics->CreateInputLayoutBuilder();

			for(uint32_t ilIdx = 0; ilIdx < mesh->inputElementsLength; ++ilIdx) {
				inputLayoutBuilder->AddInputElement(mesh->inputElements[ilIdx].semanticName,
					mesh->inputElements[ilIdx].semanticIndex,
					mesh->inputElements[ilIdx].format,
					mesh->inputElements[ilIdx].byteOffset);
			}

			Netcode::InputLayoutRef inputLayout = inputLayoutBuilder->Build();

			Netcode::Graphics::UploadBatch batch;

			auto appMesh = std::make_shared<Mesh>();

			uint8_t * const vBasePtr = reinterpret_cast<uint8_t *>(mesh->vertices);
			uint8_t * const iBasePtr = reinterpret_cast<uint8_t *>(mesh->indices);

			for(unsigned int i = 0; i < mesh->lodLevelsLength; ++i) {
				uint8_t * vData = vBasePtr + mesh->lodLevels[i].vertexBufferByteOffset;
				uint8_t * iData = nullptr;
				uint64_t vbuffer = graphics->resources->CreateVertexBuffer(mesh->lodLevels[i].vertexBufferSizeInBytes, mesh->vertexSize, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
				uint64_t vCount = mesh->lodLevels[i].vertexCount;
				uint64_t ibuffer = 0;
				uint64_t iCount = 0;

				batch.Upload(vbuffer, vData, mesh->lodLevels[i].vertexBufferSizeInBytes);
				batch.ResourceBarrier(vbuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

				if(mesh->indices != nullptr) {
					ibuffer = graphics->resources->CreateIndexBuffer(mesh->lodLevels[i].indexBufferSizeInBytes, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
					iData = iBasePtr + mesh->lodLevels[i].indexBufferByteOffset;
					iCount = mesh->lodLevels[i].indexCount;

					batch.Upload(ibuffer, iData, mesh->lodLevels[i].indexBufferSizeInBytes);
					batch.ResourceBarrier(ibuffer, ResourceState::COPY_DEST, ResourceState::INDEX_BUFFER);
				}

				GBuffer lod;
				lod.indexBuffer = ibuffer;
				lod.vertexBuffer = vbuffer;
				lod.indexCount = iCount;
				lod.vertexCount = vCount;

				appMesh->AddLOD(lod);
				appMesh->vertexSize = mesh->vertexSize;
			}

			appMesh->selectedLod = 0;
			appMesh->boundingBox = mesh->boundingBox;

			auto material = std::make_shared<TestMaterial>();

			material->data.diffuseColor = DirectX::XMFLOAT4A{ mat->diffuseColor.x, mat->diffuseColor.y,mat->diffuseColor.z, 1.0f };
			material->data.fresnelR0 = DirectX::XMFLOAT3{ 0.05f, 0.05f, 0.05f };
			material->data.shininess = mat->shininess;

			graphics->frame->SyncUpload(batch);

			modelComponent->AddShadedMesh(appMesh, material);
		}

	}

public:

	virtual void OnResized(int w, int h) override {
		float asp = graphics->GetAspectRatio();
		//scene.camera->GetComponent<Camera>()->aspect = asp;
		renderSystem.renderer.OnResize(w, h);
		uiSystem.SetScreenSize(DirectX::XMUINT2{ static_cast<uint32_t>(w), static_cast<uint32_t>(h) });
		uiScene->SetScreenSize(DirectX::XMUINT2{ static_cast<uint32_t>(w), static_cast<uint32_t>(h) });
		menuLayer->OnResized(w, h);
	}

	virtual void AddAppEventHandlers(Netcode::Module::AppEventSystem * eventSystem) override {
		Netcode::Module::AApp::AddAppEventHandlers(eventSystem);

		eventSystem->AddHandler(this);
	}

	/*
	Initialize modules
	*/
	virtual void Setup(Netcode::Module::IModuleFactory * factory) override {
		events = std::make_unique<Netcode::Module::AppEventSystem>();

		window = factory->CreateWindowModule(this, 0);
		graphics = factory->CreateGraphicsModule(this, 0);
		audio = factory->CreateAudioModule(this, 0);
		network = factory->CreateNetworkModule(this, 0);

		StartModule(window.get());
		StartModule(graphics.get());
		StartModule(audio.get());
		StartModule(network.get());

		if(window) {
			window->ShowWindow();
		}

		if(network) {
			network->SetWebserverAddress("netcode.webs", 80);
		}

		AddAppEventHandlers(events.get());

		stopwatch.Start();

		px.CreateResources();


		LoadServices();
		LoadSystems();
		LoadAssets();

	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		while(window->KeepRunning()) {
			window->ProcessMessages();
			
			events->Dispatch();

			float dt = stopwatch.Restart();
			Simulate(dt);

			Render();

			window->CompleteFrame();
		}
	}

	/*
	Properly shutdown the application
	*/
	virtual void Exit() override {
		Service::Clear();
		px.ReleaseResources();
		ShutdownModule(network.get());
		ShutdownModule(audio.get());
		ShutdownModule(graphics.get());
		ShutdownModule(window.get());
	}
};
