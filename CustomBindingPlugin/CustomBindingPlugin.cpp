#include "pch.h"
#include "CustomBindingPlugin.h"

BAKKESMOD_PLUGIN(CustomBindingPlugin, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void CustomBindingPlugin::onLoad()
{
	_globalCvarManager = cvarManager;

	gameWrapper->HookEvent("Function TAGame.GameViewportClient_TA.HandleKeyPress", std::bind(&CustomBindingPlugin::onTick, this, std::placeholders::_1));

	ctrlKeyValue = gameWrapper->GetFNameIndexByString("LeftControl");
	shiftKeyValue = gameWrapper->GetFNameIndexByString("LeftShift");
	mKeyValue = gameWrapper->GetFNameIndexByString("M");
	keysArePressed = false;
}

void CustomBindingPlugin::onUnload()
{
}

void CustomBindingPlugin::onTick(std::string eventName)
{
	if (gameWrapper->IsKeyPressed(ctrlKeyValue) && gameWrapper->IsKeyPressed(shiftKeyValue) && gameWrapper->IsKeyPressed(mKeyValue) && !keysArePressed) {
		keysArePressed = true;
		cvarManager->log("All keys are pressed for the first time");
		cvarManager->executeCommand("redirect_shoot");
	}
	
	if ((!gameWrapper->IsKeyPressed(ctrlKeyValue) || !gameWrapper->IsKeyPressed(shiftKeyValue) || !gameWrapper->IsKeyPressed(mKeyValue)) && keysArePressed) {
		keysArePressed = false;
		cvarManager->log("One key is unpressed");
	}
}
