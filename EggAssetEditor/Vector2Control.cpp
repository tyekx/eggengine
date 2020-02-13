﻿#include "pch.h"
#include "Vector2Control.h"
#if __has_include("Vector2Control.g.cpp")
#include "Vector2Control.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggAssetEditor::implementation
{
    Vector2Control::Vector2Control()
    {
        InitializeComponent();

        auto dc = winrt::make<DC_Vector2Control>();

        wrapper().DataContext(dc);

        dc.PropertyChanged(
            Windows::UI::Xaml::Data::PropertyChangedEventHandler(this, &Vector2Control::OnValueUpdated)
        );
    }

    Windows::UI::Xaml::DependencyProperty Vector2Control::valueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"Value",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<EggAssetEditor::Vector2Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(Windows::Foundation::Numerics::float2{0.0f, 0.0f}), Windows::UI::Xaml::PropertyChangedCallback{ &Vector2Control::OnValueChanged } }
    );
}