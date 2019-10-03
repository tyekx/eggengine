﻿//
// UC_AssetBrowser.xaml.h
// Declaration of the UC_AssetBrowser class
//

#pragma once

#include "UC_AssetBrowser.g.h"
#include "EggProject.h"
#include "ProjectFolderDataContext.h"

namespace EggEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_AssetBrowser sealed
	{
		ProjectFolderDataContext ^ rootFolder;
		ProjectFolderDataContext ^ currentFolder;

	public:
		UC_AssetBrowser();

		void ImportAsset(Windows::Storage::StorageFile ^ file);

	private:

		void EventHandler_OpenedProject(EggProject ^ proj);
		void EventHandler_ClosedProject(EggProject ^ proj);

		void CtxBtnNewProjectFolder_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void CtxBtnNewMaterial_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void CtxBtnNewShadedMesh_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);


		void BreadCrumb_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs ^ e);
		void ProjectFolder_DoubleTap(Platform::Object ^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs ^ e);
		void ChangeProjectFolder(ProjectFolderDataContext ^ folder);
		void OpenAssetHandler(Platform::Object ^ obj);
		/*
		Implementation details
		*/
		void ImplDetail_RecursiveAddBreadCrumb(ProjectFolderDataContext ^ folder);
	};
}