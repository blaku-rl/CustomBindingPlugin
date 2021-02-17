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

bool CustomBindingsPlugin::CheckForAllKeysPressed(CustomBinding& binding)
{
	if (binding.keyList.size() == 0) { return false; }
	for (auto& key : binding.keyList) {
		if (!gameWrapper->IsKeyPressed(keys[key]))
			return false;
	}
	return true;
}

void CustomBindingsPlugin::OnKeyPressed(std::string eventName)
{
	for (auto& binding : bindings) {
		if (CheckForAllKeysPressed(*binding) && !binding->allKeysPressed) {
			binding->allKeysPressed = true;
			cvarManager->executeCommand(binding->command);
		}
		else if ((!CheckForAllKeysPressed(*binding)) && binding->allKeysPressed) {
			binding->allKeysPressed = false;
		}
	}

	if (bindingChangeDesired >= 0 && bindingChangeDesired < guiBindingSelected.keyList.size()) {
		std::string keyName = "";

		for (auto& key : keys) {
			if (gameWrapper->IsKeyPressed(key.second)) {
				keyName = key.first;
				break;
			}
		}

		//The key press function has no keys pressed on the first pass
		if (keyName == "") { return; }

		guiBindingSelected.keyList[bindingChangeDesired] = keyName;
		bindingChangeDesired = -1;
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
			std::string key;
			std::stringstream ss(keysString);
			while (std::getline(ss, key, ','))
				binding.keyList.push_back(key);

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
	auto binding = CustomBinding();
	bindings.push_back(std::make_shared<CustomBinding>(binding));

	UpdateSelectedBinding(bindings.size() - 1);
}

void CustomBindingsPlugin::AddKeyToSelectedBinding()
{
	guiBindingSelected.keyList.push_back("Press Any Key");
	bindingChangeDesired = guiBindingSelected.keyList.size() - 1;
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
	bindingChangeDesired = -1;

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
	curBinding->keyList = guiBindingSelected.keyList;
	curBinding->command = std::string(commandBuffer);

	WriteBindings();
}

bool CustomBindingsPlugin::AreKeysValid(CustomBinding& binding)
{
	for (auto& key : binding.keyList) {
		if (keys.find(key) == keys.end())
			return false;
	}

	return true;
}