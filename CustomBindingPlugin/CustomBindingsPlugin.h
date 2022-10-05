#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "./Models/CustomBindingGUI.h"
#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class CustomBindingsPlugin : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow, public BakkesMod::Plugin::PluginSettingsWindow
{
	//Plugin Begin/End
	virtual void onLoad();
	virtual void onUnload();

	//Initilization
	void SetUpKeysMap();

	//Key press handling
	void OnKeyPressed(ActorWrapper aw, void* params, std::string eventName);
	void CheckForSatisfiedBinds();
	bool CheckForAllKeysPressed(const CustomBinding& binding);
	
	//Maniuplating bindings
	void AddBinding();
	void SwapBindings(const int& originalIndex, const int& newIndex);
	void RemoveSelectedBinding();
	void UpdateSelectedBinding(const int& pos);
	void SaveSelectedBinding();
	bool ChangesHaveBeenMade();

	//Reading and writing binding saved data
	void ReadInOldBindingData();
	void ReadInBindings();
	void WriteBindings();

	//Setup Thread Stuff
	std::thread setupThread;
	bool isSetupComplete = false;

	//Cvar
	std::shared_ptr<bool> useKeyPress;
	std::shared_ptr<bool> autoSaveOnChanges;
	std::shared_ptr<bool> showEnabledStatus;
	std::shared_ptr<std::string> enabledTag;
	std::shared_ptr<std::string> disabledTag;

	//Plugin Data
	std::vector<CustomBinding> bindings;
	std::map<int, int> keyNameIndexToFnameIndex = {};
	std::map<int, int> fnameIndexToKeyNameIndex = {};

	//Constants
	const std::string keyTag = "keys:";
	const std::string nameTag = "name:";
	const std::string commandTag = "command:";
	const std::string KeyPressedEvent = "Function TAGame.GameViewportClient_TA.HandleKeyPress";
	const std::string TickFunction = "Function Engine.GameViewportClient.Tick";
	const std::filesystem::path oldBindFilePath = gameWrapper->GetDataFolder() / "CustomBindings.data";
	const std::filesystem::path bindFilePath = gameWrapper->GetDataFolder() / "CustomBindings.json";

	//ImGui Menu stuff
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "custombindingsplugin";
	std::string menuTitleDisplay = "Custom Bindings Plugin 2.0";
	CustomBindingGUI BindingSelected;
	bool isMovingBinding = false;
	int bindIndexAfterSave = 0;

	//ImGui Rendering
	virtual void Render() override;
	void RenderAllBindings();
	void HandleDraggingOnList(const int& index);
	void SaveBindingReminder();
	void RenderBindingDetails();
	void RenderEditBindOptions();
	void RenderSaveOptions();
	void RenderDeleteBind();

	//ImGui Management
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;

	//Setting File Rendering
	virtual std::string GetPluginName() override;
	virtual void RenderSettings() override;
	void RenderBoolCvar(const char* cvarName, const char* description);
	void RenderStringCvar(const char* cvarName, const char* description, const char* label);
};

