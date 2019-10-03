﻿//
// UC_Asset.xaml.h
// Declaration of the UC_Asset class
//

#pragma once

#include "Auxiliary.h"
#include "UC_Asset.g.h"
#include "AssetDataContext.h"

namespace EggEditor
{

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_Asset sealed
	{
		static UC_Asset ^ currentlyDragging;
	public:
		UC_Asset();
		UC_Asset(Platform::String^ text, UINT atype);
		event OpenAssetCallback ^ OpenAsset;

		static property UC_Asset ^ CurrentlyDragging {
			UC_Asset ^ get() {
				return currentlyDragging;
			}
		}

		property Platform::String ^ VirtualPath {
			Platform::String ^ get() {
				return L"";
			}
		}
		
		property AssetDataContext ^ TypedDataContext {
			AssetDataContext ^ get() {
				return (AssetDataContext ^)DataContext;
			}
		}

		static unsigned int GetAssetTypeFromExtension(Platform::String ^ ext) {
			if(ext == L".jpg" || ext == L".png" || ext == L".jpeg") {
				return ASSET_TYPE_TEXTURE2D;
			} else if(ext == L".obj") {
				return ASSET_TYPE_MESH;
			} else if(ext == L".cso") {
				return ASSET_TYPE_SHADER;
			}
			return UINT_MAX;
		}

	private:
		void AssetDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args);
		void MenuFlyoutItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e);
		void CtxOpenInEditorBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
	};
}