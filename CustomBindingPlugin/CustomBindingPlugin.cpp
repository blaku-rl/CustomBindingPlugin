#include "pch.h"
#include "CustomBindingPlugin.h"

BAKKESMOD_PLUGIN(CustomBindingPlugin, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void CustomBindingPlugin::onLoad()
{
	_globalCvarManager = cvarManager;
	
	gameWrapper->HookEvent("Function TAGame.GameViewportClient_TA.HandleKeyPress", std::bind(&CustomBindingPlugin::OnKeyPressed, this, std::placeholders::_1));

	ReadInBindings();
	UpdateSelectedBinding(0);
}

void CustomBindingPlugin::onUnload()
{
	WriteBindings();
}

void CustomBindingPlugin::OnKeyPressed(std::string eventName)
{
	for (auto& binding : bindings) {
		if (gameWrapper->IsKeyPressed(keys[binding->key1]) && gameWrapper->IsKeyPressed(keys[binding->key2])
			&& gameWrapper->IsKeyPressed(keys[binding->key3]) && !binding->allKeysPressed) {
			binding->allKeysPressed = true;
			cvarManager->executeCommand(binding->command);
		}
		else if ((!gameWrapper->IsKeyPressed(keys[binding->key1]) || !gameWrapper->IsKeyPressed(keys[binding->key2])
			|| !gameWrapper->IsKeyPressed(keys[binding->key3])) && binding->allKeysPressed) {
			binding->allKeysPressed = false;
		}
	}

	if (bindingChangeDesired > 0 && bindingChangeDesired < 4) {
		cvarManager->log("Checking for key presses");
		std::string keyName = "";

		for (auto& key : keys) {
			if (gameWrapper->IsKeyPressed(key.second)) {
				keyName = key.first;
				break;
			}
		}

		//The key press function has no keys pressed on the first pass
		if (keyName == "") { return; }

		switch (bindingChangeDesired) {
		case 1:
			guiBindingSelected.key1 = keyName;
			break;
		case 2:
			guiBindingSelected.key2 = keyName;
			break;
		case 3:
			guiBindingSelected.key3 = keyName;
			break;
		}

		bindingChangeDesired = 0;
	}
}

//TODO add checks when reading in the file to make sure its of the proper format
void CustomBindingPlugin::ReadInBindings()
{
	std::ifstream bindFile(bindFilePath);
	if (bindFile.is_open()) {
		std::string line;
		while (std::getline(bindFile, line)) {
			std::string keysString = line.substr(0, line.find(" "));
			std::string command = line.substr(line.find(" ") + 1, std::string::npos);
			CustomBinding binding;
			std::stringstream ss(keysString);
			std::getline(ss, binding.key1, ',');
			std::getline(ss, binding.key2, ',');
			std::getline(ss, binding.key3, ',');
			binding.command = command;
			if (binding.AreKeysValid()) {
				bindings.push_back(std::make_shared<CustomBinding>(binding));
			}
		}
	}
	bindFile.close();
}

void CustomBindingPlugin::WriteBindings()
{
	std::ofstream bindFile(bindFilePath);
	if (bindFile.is_open()) {
		for (auto& binding : bindings) {
			if (binding->AreKeysValid()) {
				bindFile << binding->GetKeyString() << " " << binding->command << "\n";
			}
		}
	}
	bindFile.close();
}

void CustomBindingPlugin::AddBinding()
{
	auto binding = CustomBinding("None", "None", "None", "");
	bindings.push_back(std::make_shared<CustomBinding>(binding));

	UpdateSelectedBinding(bindings.size() - 1);
}

void CustomBindingPlugin::RemoveSelectedBinding()
{
	std::list<std::shared_ptr<CustomBinding>>::iterator begin = bindings.begin();
	std::advance(begin, guiBindingSelectedPos);
	bindings.erase(begin);

	WriteBindings();
	UpdateSelectedBinding(0);
}

void CustomBindingPlugin::UpdateSelectedBinding(int pos)
{
	if (bindings.size() == 0) {
		guiBindingSelectedPos = -1;
		return;
	}

	std::list<std::shared_ptr<CustomBinding>>::iterator begin = bindings.begin();
	std::advance(begin, pos);

	auto& curBinding = *begin;

	guiBindingSelected = CustomBinding(*curBinding);
	guiBindingSelectedPos = pos;

	*commandBuffer = {};
	for (int i = 0; i < sizeof(guiBindingSelected.command); i++) {
		commandBuffer[i] = guiBindingSelected.command[i];
	}
}

void CustomBindingPlugin::SaveSelectedBinding()
{
	if (!guiBindingSelected.AreKeysValid()) { return; }

	std::list<std::shared_ptr<CustomBinding>>::iterator begin = bindings.begin();
	std::advance(begin, guiBindingSelectedPos);

	auto& curBinding = *begin;
	curBinding->key1 = guiBindingSelected.key1;
	curBinding->key2 = guiBindingSelected.key2;
	curBinding->key3 = guiBindingSelected.key3;
	curBinding->command = std::string(commandBuffer);

	WriteBindings();
}