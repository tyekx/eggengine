﻿#include "pch.h"
#include "CollidersPage.h"
#if __has_include("CollidersPage.g.cpp")
#include "CollidersPage.g.cpp"
#endif
#include "XamlGlobal.h"
#include "XamlHelpers.h"
#include "DC_Bone.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggAssetEditor::implementation
{
    CollidersPage::CollidersPage()
    {
        InitializeComponent();

        colliders = winrt::single_threaded_observable_vector<EggAssetEditor::DC_Collider>();
        bones = winrt::single_threaded_observable_vector<EggAssetEditor::DC_Bone>();
        tokens = winrt::single_threaded_vector<winrt::event_token>();

        colliders.VectorChanged(Windows::Foundation::Collections::VectorChangedEventHandler<EggAssetEditor::DC_Collider>(this, &CollidersPage::OnCollidersCollectionChanged));
    }


    winrt::event_token CollidersPage::AddCallbackFor(uint32_t idx) {
        auto collectionView = colliders.GetView();
        auto dcColliderOuter = collectionView.GetAt(idx);

        return dcColliderOuter.PropertyChanged([idx](Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Data::PropertyChangedEventArgs const & e) -> void {
            auto & collider = Global::Model->colliders.at(idx);

            auto dcCollider = sender.as<EggAssetEditor::DC_Collider>();

            if(e.PropertyName() == L"LocalPosition" || e.PropertyName() == L"LocalRotation") {
                auto lPos = dcCollider.LocalPosition();
                auto lRot = dcCollider.LocalRotation();

                DirectX::XMVECTOR localRotationQuat = DirectX::XMQuaternionRotationRollPitchYaw(lRot.z, lRot.y, lRot.x);

                collider.localPosition = DirectX::XMFLOAT3{ lPos.x, lPos.y, lPos.z };
                DirectX::XMStoreFloat4(&collider.localRotation, localRotationQuat);

                Global::EditorApp->UpdateColliderData(Global::Model->colliders);
                Global::EditorApp->Run();
            } else {
                ::ColliderType type = static_cast<::ColliderType>(dcCollider.Type());
                collider.type = type;
                collider.boneReference = dcCollider.BoneReference();

                switch(type) {
                    case ColliderType::BOX:
                    {
                        auto boxArgs = dcCollider.BoxArgs();
                        collider.boxArgs = DirectX::XMFLOAT3{ boxArgs.x, boxArgs.y, boxArgs.z };
                    }
                    break;
                    case ColliderType::CAPSULE:
                    {
                        auto capsuleArgs = dcCollider.CapsuleArgs();
                        collider.capsuleArgs = DirectX::XMFLOAT2{ capsuleArgs.x, capsuleArgs.y };
                    }
                    break;
                    case ColliderType::SPHERE:
                        collider.sphereArgs = dcCollider.SphereArg();
                        break;
                    case ColliderType::MESH:
                        return;
                }

                Global::EditorApp->SetColliders(Global::Model->colliders);
                Global::EditorApp->Run();
            }
        });
    }


    EggAssetEditor::DC_Collider CollidersPage::ConvertCollider(const Collider & collider) {
        auto dcCollider = winrt::make<EggAssetEditor::implementation::DC_Collider>();

        DirectX::XMVECTOR localRotationQuat = DirectX::XMLoadFloat4(&collider.localRotation);
        DirectX::XMVECTOR axis;
        float angle;
        DirectX::XMQuaternionToAxisAngle(&axis, &angle, localRotationQuat);

        axis = DirectX::XMVector3Normalize(axis);
        DirectX::XMFLOAT3 a;
        DirectX::XMStoreFloat3(&a, axis);
        float roll = asinf(a.x * a.y * (1.0f - cosf(angle)) + a.z * sinf(angle));
        float pitch = atan2f(a.y * sinf(angle) - a.x * a.z * (1.0f - cosf(angle)), 1.0f - (a.y * a.y + a.z * a.z) * (1.0f - cosf(angle)));
        float yaw = atan2f(a.x * sinf(angle) - a.y * a.z * (1.0f - cosf(angle)), 1.0f - (a.x * a.x + a.z * a.z) * (1.0f - cosf(angle)));

        dcCollider.LocalPosition(Windows::Foundation::Numerics::float3(collider.localPosition.x, collider.localPosition.y, collider.localPosition.z));
        dcCollider.LocalRotation(Windows::Foundation::Numerics::float3(roll, pitch, yaw));
        dcCollider.BoneReference(collider.boneReference);

        switch(collider.type) {
            case ColliderType::BOX:
                dcCollider.BoxArgs(Windows::Foundation::Numerics::float3(collider.boxArgs.x, collider.boxArgs.y, collider.boxArgs.z));
                break;
            case ColliderType::CAPSULE:
                dcCollider.CapsuleArgs(Windows::Foundation::Numerics::float2(collider.capsuleArgs.x, collider.capsuleArgs.y));
                break;
            case ColliderType::SPHERE:
                dcCollider.SphereArg(collider.sphereArgs);
                break;
            case ColliderType::MESH:
                dcCollider.Type(static_cast<uint32_t>(ColliderType::MESH));
                break;
        }

        return dcCollider;
    }

    void CollidersPage::OnCollidersCollectionChanged(
        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Collider> const & sender,
        Windows::Foundation::Collections::IVectorChangedEventArgs const & evt) {

        auto changeEvent = evt.CollectionChange();

        if(changeEvent == Windows::Foundation::Collections::CollectionChange::ItemInserted) {
            uint32_t idx = evt.Index();
            tokens.InsertAt(idx, AddCallbackFor(idx));
        }

        // on removal the indices must be updated to reflect the proper reference
        if(changeEvent == Windows::Foundation::Collections::CollectionChange::ItemRemoved) {
            uint32_t idx = evt.Index();
            tokens.RemoveAt(idx);

            auto tokensView = tokens.GetView();
            auto collidersView = colliders.GetView();

            for(uint32_t i = 0; i < tokensView.Size(); ++i) {
                collidersView.GetAt(i).PropertyChanged(tokens.GetAt(i));
            }

            tokens.Clear();

            for(uint32_t i = 0; i < collidersView.Size(); ++i) {
                tokens.Append(AddCallbackFor(i));
            }
        }
    }

    Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Collider> CollidersPage::Colliders() {
        return colliders;
    }

    Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Bone> CollidersPage::Bones() {
        return bones;
    }

    void CollidersPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        colliders.Clear();
        bones.Clear();

        auto obones = XamlHelpers::ConvertBones();

        auto obonesView = obones.GetView();
        auto noneBone = make<EggAssetEditor::implementation::DC_Bone>();
        noneBone.Name(L"None");
        noneBone.Depth(0);

        bones.Append(noneBone);
        for(uint32_t i = 0; i < obonesView.Size(); ++i) {
            bones.Append(obonesView.GetAt(i));
        }

        if(Global::Model != nullptr && !Global::Model->colliders.empty()) {
            for(const auto & collider : Global::Model->colliders) {
                colliders.Append(ConvertCollider(collider));
            }

            Global::EditorApp->SetColliders(Global::Model->colliders);
            Global::EditorApp->Run();
        }

        __super::OnNavigatedTo(e);
    }

    void CollidersPage::ClearBoneReference_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        int32_t i = collidersList().SelectedIndex();
        auto collidersView = colliders.GetView();

        if(i < 0 || i >= static_cast<int32_t>(collidersView.Size())) {
            return;
        }

        uint32_t idx = static_cast<uint32_t>(i);

        collidersView.GetAt(idx).BoneReference(0xFF);
        boneComboBox().SelectedIndex(0);
    }

    void CollidersPage::AddCollider_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        if(Global::Model != nullptr) {
            Collider dCollider;
            dCollider.boneReference = 0xFF;
            dCollider.boxArgs = DirectX::XMFLOAT3{ 10.0f, 10.0f, 10.0f };
            dCollider.type = ColliderType::BOX;
            dCollider.localPosition = DirectX::XMFLOAT3{};
            dCollider.localRotation = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f };
            
            Global::Model->colliders.push_back(dCollider);
            colliders.Append(ConvertCollider(dCollider));

            Global::EditorApp->SetColliders(Global::Model->colliders);
            Global::EditorApp->Run();
        }
    }

    void CollidersPage::collidersList_SelectionChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e)
    {
        int32_t idx = collidersList().SelectedIndex();

        auto collidersView = Colliders().GetView();
        auto bonesView = Bones().GetView();

        if(idx < 0 || idx >= static_cast<int32_t>(collidersView.Size())) {
            boneComboBox().SelectedIndex(-1);
            return;
        }

        int32_t boneIdx = collidersView.GetAt(idx).BoneReference();

        if(boneIdx == 255 || boneIdx < 0) {
            boneIdx = 0;
        } else {
            boneIdx += 1;
        }

        try {
            boneComboBox().SelectedIndex(boneIdx);
        } catch(hresult_error & ni) {
            OutputDebugStringW(L"No clue why this throws...\r\n");
        }
    }

    void CollidersPage::boneComboBox_SelectionChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e)
    {
        auto collidersView = Colliders().GetView();
        auto bonesView = Bones().GetView();

        int32_t boneIdx = boneComboBox().SelectedIndex();
        int32_t colliderIdx = collidersList().SelectedIndex();

        if(colliderIdx == -1 || colliderIdx >= collidersView.Size()) {
            return;
        }

        int32_t currentBoneIdx = collidersView.GetAt(colliderIdx).BoneReference();

        if(currentBoneIdx != (boneIdx - 1)) {
            collidersView.GetAt(colliderIdx).BoneReference(boneIdx - 1);
        }
    }

}






