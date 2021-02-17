#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

#include <fstream>
#include <sstream>

#include "CustomBinding.h"
#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class CustomBindingsPlugin: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
	virtual void onLoad();
	virtual void onUnload();

	void GenerateSettingsFile();
	void SetUpKeysMap();
	bool CheckForAllKeysPressed(CustomBinding& binding);
	void OnKeyPressed(std::string eventName);
	void ReadInBindings();
	void WriteBindings();
	void AddBinding();
	void AddKeyToSelectedBinding();
	void RemoveSelectedBinding();
	void UpdateSelectedBinding(int pos);
	void SaveSelectedBinding();
	bool AreKeysValid(CustomBinding& binding);

	std::map<std::string, int> keys = {};
	std::filesystem::path bindFilePath = gameWrapper->GetBakkesModPath() / "data" / "CustomBindings.data";
	std::filesystem::path setFilePath = gameWrapper->GetBakkesModPath() / "plugins" / "settings" / "custombindingsplugin.set";
	std::list<std::shared_ptr<CustomBinding>> bindings;

	//ImGui Shit
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "custombindingsplugin";
	std::string menuTitleDisplay = "Custom Bindings Plugin 1.0";
	CustomBinding guiBindingSelected;
	int guiBindingSelectedPos = -1;
	int bindingChangeDesired = -1;
	char commandBuffer[100] = {};

	virtual void Render() override;
	void RenderAllBindings();
	void RenderBindingDetails();
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
};

