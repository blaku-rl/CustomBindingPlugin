#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

#include "Key.h"
#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class CustomBindingPlugin: public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginWindow*/
{
	virtual void onLoad();
	virtual void onUnload();

	void onTick(std::string eventName);

	int ctrlKeyValue;
	int shiftKeyValue;
	int mKeyValue;
	bool keysArePressed;
};

