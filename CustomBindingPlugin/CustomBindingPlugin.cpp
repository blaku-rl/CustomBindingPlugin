#include "pch.h"
#include "CustomBindingsPlugin.h"

BAKKESMOD_PLUGIN(CustomBindingsPlugin, "A plugin made for settting custom bindings with 3 keys", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void CustomBindingsPlugin::onLoad()
{
	_globalCvarManager = cvarManager;
	
	gameWrapper->HookEvent("Function TAGame.GameViewportClient_TA.HandleKeyPress", std::bind(&CustomBindingsPlugin::OnKeyPressed, this, std::placeholders::_1));

	GenerateSettingsFile();
	SetUpKeysMap();
	ReadInBindings();
	UpdateSelectedBinding(0);
}

void CustomBindingsPlugin::onUnload()
{
	WriteBindings();
}

void CustomBindingsPlugin::GenerateSettingsFile()
{
	std::ofstream bindFile(setFilePath);
	if (bindFile.is_open()) {
		bindFile << "Custom Bindings Plugin\n";
		bindFile << "8 |\n";
		bindFile << "9 | If you want to open the plugin menu from a command run: togglemenu custombindingsplugin;\n";
		bindFile << "8 |\n";
		bindFile << "0 | Open Custom Bindings Plugin GUI | plugin load custombindingsplugin; writeplugins; togglemenu custombindingsplugin\n";
	}
	bindFile.close();
}

//RL can change these values with any update, just read them in each time and get the proper int codes
void CustomBindingsPlugin::SetUpKeysMap()
{
	for (auto& keyName : keyNames) {
		keys.insert(std::pair<std::string, int>(keyName, gameWrapper->GetFNameIndexByString(keyName)));
	}
}

void CustomBindingsPlugin::OnKeyPressed(std::string eventName)
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

void CustomBindingsPlugin::ReadInBindings()
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
			if (AreKeysValid(binding)) {
				bindings.push_back(std::make_shared<CustomBinding>(binding));
			}
		}
	}
	bindFile.close();
}

void CustomBindingsPlugin::WriteBindings()
{
	std::ofstream bindFile(bindFilePath);
	if (bindFile.is_open()) {
		for (auto& binding : bindings) {
			if (AreKeysValid(*binding)) {
				bindFile << binding->GetKeyString() << " " << binding->command << "\n";
			}
		}
	}
	bindFile.close();
}

void CustomBindingsPlugin::AddBinding()
{
	auto binding = CustomBinding("None", "None", "None", "");
	bindings.push_back(std::make_shared<CustomBinding>(binding));

	UpdateSelectedBinding(bindings.size() - 1);
}

void CustomBindingsPlugin::RemoveSelectedBinding()
{
	std::list<std::shared_ptr<CustomBinding>>::iterator begin = bindings.begin();
	std::advance(begin, guiBindingSelectedPos);
	bindings.erase(begin);

	WriteBindings();
	UpdateSelectedBinding(0);
}

void CustomBindingsPlugin::UpdateSelectedBinding(int pos)
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
	bindingChangeDesired = 0;

	*commandBuffer = {};
	for (int i = 0; i < sizeof(guiBindingSelected.command); i++) {
		commandBuffer[i] = guiBindingSelected.command[i];
	}
}

void CustomBindingsPlugin::SaveSelectedBinding()
{
	if (!AreKeysValid(guiBindingSelected)) { return; }

	std::list<std::shared_ptr<CustomBinding>>::iterator begin = bindings.begin();
	std::advance(begin, guiBindingSelectedPos);

	auto& curBinding = *begin;
	curBinding->key1 = guiBindingSelected.key1;
	curBinding->key2 = guiBindingSelected.key2;
	curBinding->key3 = guiBindingSelected.key3;
	curBinding->command = std::string(commandBuffer);

	WriteBindings();
}

bool CustomBindingsPlugin::AreKeysValid(CustomBinding binding)
{
	return keys.find(binding.key1) != keys.end() && keys.find(binding.key2) != keys.end() && keys.find(binding.key3) != keys.end();
}