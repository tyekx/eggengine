cmake_minimum_required(VERSION 3.8)

target_sources(Netcode 
PUBLIC
	"Button.h"
	"Common.h"
	"CheckBox.h"
	"Input.h"
	"InputGroup.h"
	"Label.h"
	"Page.h"
	"PageManager.h"
	"Panel.h"
	"ScrollViewer.h"
	"StackPanel.h"
	"TextBox.h"
	"Animation.hpp"
	"Slider.h"
	"Control.h"

PRIVATE
	"Button.cpp"
	"Input.cpp"
	"InputGroup.cpp"
	"CheckBox.cpp"
	"Label.cpp"
	"Page.cpp"
	"PageManager.cpp"
	"Panel.cpp"
	"ScrollViewer.cpp"
	"StackPanel.cpp"
	"TextBox.cpp"
	"Animation.cpp"
	"Slider.cpp"
	"Control.cpp"
)
