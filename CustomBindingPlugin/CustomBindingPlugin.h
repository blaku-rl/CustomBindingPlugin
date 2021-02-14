#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

#include <fstream>
#include <sstream>

#include "CustomBinding.h"
#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class CustomBindingPlugin: public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginWindow*/
{
	virtual void onLoad();
	virtual void onUnload();

	void OnTick(std::string eventName);
	void AddBinding(std::string key1, std::string key2, std::string key3, std::string command);
	void WriteBindings();

	std::filesystem::path bindFilePath = gameWrapper->GetBakkesModPath() / "data" / "CustomBinding.data";
	std::vector<CustomBinding> bindings;
};

