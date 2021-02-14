#include "pch.h"
#include "CustomBindingPlugin.h"

BAKKESMOD_PLUGIN(CustomBindingPlugin, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void CustomBindingPlugin::onLoad()
{
	_globalCvarManager = cvarManager;

	gameWrapper->HookEvent("Function TAGame.GameViewportClient_TA.HandleKeyPress", std::bind(&CustomBindingPlugin::OnTick, this, std::placeholders::_1));

	//Read in binds set in previous sessions
	//might need to add checks just incase anyone decides to be bold and edit the file directly
	std::ifstream bindFile(bindFilePath);
	if (bindFile.is_open()) {
		std::string line;
		while (std::getline(bindFile, line)) {
			std::string keysString = line.substr(0, line.find(" "));
			std::string command = line.substr(line.find(" ") + 1, std::string::npos);
			CustomBinding binding;
			std::stringstream ss(keysString);
			std::getline(ss, binding.key1, '+');
			std::getline(ss, binding.key2, '+');
			std::getline(ss, binding.key3, '+');
			binding.command = command;
			bindings.push_back(binding);
		}
	}
	bindFile.close();
}

void CustomBindingPlugin::onUnload()
{

}

//Check all the bindings when a key is pressed to see if we should do the set command
//AND IT ACTUALLY WORKS
void CustomBindingPlugin::OnTick(std::string eventName)
{
	for (auto& binding : bindings) {
		if (gameWrapper->IsKeyPressed(keys[binding.key1]) && gameWrapper->IsKeyPressed(keys[binding.key2])
			&& gameWrapper->IsKeyPressed(keys[binding.key3]) && !binding.allKeysPressed) {
			binding.allKeysPressed = true;
			cvarManager->executeCommand(binding.command);
		}
		else if ((!gameWrapper->IsKeyPressed(keys[binding.key1]) || !gameWrapper->IsKeyPressed(keys[binding.key2])
			|| !gameWrapper->IsKeyPressed(keys[binding.key3])) && binding.allKeysPressed) {
			binding.allKeysPressed = false;
		}
	}
}

void CustomBindingPlugin::AddBinding(std::string key1, std::string key2, std::string key3, std::string command)
{
	auto binding = CustomBinding(key1, key2, key3, command);
	bindings.push_back(binding);
}

void CustomBindingPlugin::WriteBindings()
{
	std::ofstream bindFile(bindFilePath);
	if (bindFile.is_open()) {
		for (auto& binding : bindings) {
			bindFile << binding.GetKeyString() << " " << binding.command;
		}
	}
	bindFile.close();
}

/*
* Imagine using discord to talk to people
* cpp comments on a shared github is where it's at
* 
* Here's how I'm thinking we store the data in data file or whatever (slightly older Blaku here, I have assumed this structure)
* keyName1+keyName2+keyName3 bakkescommand
* I'm gonna write some functions that will interact with the data (eh kinda)
* the key in the file to find things will be by the key combinations (sounds like the start of a map to me)
* also in the gui we should link to the key name website so people can see what keys map to what if they need to
* 
* Sleep time now
* TODO
* Some indexing for the binds that's not a vector since deletion at any point can happen
* maybe a linked list for that? I feel like I just want to throw maps at everything lol
* Stupid gui shit
*/

