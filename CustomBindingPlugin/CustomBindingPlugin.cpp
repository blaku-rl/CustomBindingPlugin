#include "pch.h"
#include "CustomBindingsPlugin.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <sstream>

BAKKESMOD_PLUGIN(CustomBindingsPlugin, "Custom Binding Plugin", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

std::map<std::string, int> keyNameToFnameIndex = {};
std::map<int, KeyInfo> keysPressed = {};

void to_json(nlohmann::json& j, const CustomBinding& binding) {
	std::vector<std::string> curKeyNames = {};
	for (const auto& fNameIndex : binding.keyListFnameIndex) {
		curKeyNames.push_back(keysPressed[fNameIndex].keyName);
	}

	j = nlohmann::json{ {"name", binding.name}, {"command", binding.command}, {"enabled", binding.enabled}, {"keys", curKeyNames} };
}

void from_json(const nlohmann::json& j, CustomBinding& binding) {
	std::vector<std::string> curKeyNames = {};
	j.at("name").get_to(binding.name);
	j.at("command").get_to(binding.command);
	j.at("enabled").get_to(binding.enabled);
	j.at("keys").get_to(curKeyNames);

	std::vector<int> indicies = {};
	for (const auto& keyName : curKeyNames) {
		indicies.push_back(keyNameToFnameIndex[keyName]);
	}
	binding.keyListFnameIndex = indicies;
}

void CustomBindingsPlugin::onLoad()
{
	using namespace std::placeholders;
	_globalCvarManager = cvarManager;
	
	setupThread = std::thread([this]() {
		SetUpKeysMap();
		if (std::filesystem::exists(oldBindFilePath)) {
			ReadInOldBindingData();
			WriteBindings();
			std::filesystem::remove(oldBindFilePath);
		}
		else {
			ReadInBindings();
		}
		UpdateSelectedBinding(0);
		isSetupComplete = true;
		});

	useKeyPress = std::make_shared<bool>(false);
	cvarManager->registerCvar("custombindings_usekeypress", "0", "True/False value for if using a key press is desired for registering a key", true, true, 0, true, 1, true)
		.bindTo(useKeyPress);
	autoSaveOnChanges = std::make_shared<bool>(false);
	cvarManager->registerCvar("custombindings_autosavechanges", "0", "True/False value for turning on auto saving for changes made to binds", true, true, 0, true, 1, true)
		.bindTo(autoSaveOnChanges);
	showEnabledStatus = std::make_shared<bool>(false);
	cvarManager->registerCvar("custombindings_showenabledstatus", "0", "True/False value for displaying enabled status on bind list", true, true, 0, true, 1, true)
		.bindTo(showEnabledStatus);
	enabledTag = std::make_shared<std::string>("");
	cvarManager->registerCvar("custombindings_enabledtag", "[*] ", "String to put in front of enabled bindings")
		.bindTo(enabledTag);
	disabledTag = std::make_shared<std::string>("");
	cvarManager->registerCvar("custombindings_disabledtag", "    ", "String to put in front of disabled bindings")
		.bindTo(disabledTag);

	gameWrapper->HookEventWithCaller<ActorWrapper>(KeyPressedEvent,
		std::bind(&CustomBindingsPlugin::OnKeyPressed, this, _1, _2, _3));

	gameWrapper->HookEventPost(TickFunction,
		[this](std::string eventName) {
			if (isSetupComplete && setupThread.joinable()) {
				setupThread.join();
				gameWrapper->UnhookEventPost("Function Engine.GameViewportClient.Tick");
				cvarManager->log("Custom Bindings Plugin setup complete");
			}
		});
}

void CustomBindingsPlugin::onUnload()
{
	if (setupThread.joinable()) {
		setupThread.join();
	}
	WriteBindings();
}

//RL can change these values with any update, just read them in each time and get the proper int codes
void CustomBindingsPlugin::SetUpKeysMap()
{
	for (int i = 1; i < keyNames.size(); i++) {
		const int& fnameIndex = gameWrapper->GetFNameIndexByString(keyNames[i]);
		keyNameIndexToFnameIndex[i] = fnameIndex;
		fnameIndexToKeyNameIndex[fnameIndex] = i;
		keyNameToFnameIndex[keyNames[i]] = fnameIndex;
		keysPressed[fnameIndex] = { keyNames[i], false };
	}
}

void CustomBindingsPlugin::OnKeyPressed(ActorWrapper aw, void* params, std::string eventName)
{
	if (!isSetupComplete) return;

	KeyPressParams* keyPressData = (KeyPressParams*)params;
	if (keysPressed.find(keyPressData->Key.Index) == keysPressed.end()) 
		return;
	keysPressed[keyPressData->Key.Index].isPressed = keyPressData->EventType != EInputEvent::Released;

	if (BindingSelected.IsKeyChangeDesired()) {
		std::string& keyName = keysPressed[keyPressData->Key.Index].keyName;
		for (int i = 1; i < keyNames.size(); ++i) {
			if (keyNames[i] == keyName) {
				BindingSelected.UpdateSelectedKey(i);
				return;
			}
		}
		return;
	}

	CheckForSatisfiedBinds();
}

void CustomBindingsPlugin::CheckForSatisfiedBinds()
{
	for (auto& binding : bindings) {
		if (!binding.enabled) continue;
		bool keysPressedForBinding = CheckForAllKeysPressed(binding);
		if (keysPressedForBinding && !binding.allKeysPressed) {
			binding.allKeysPressed = true;
			cvarManager->executeCommand(binding.command);
		}
		else if (!keysPressedForBinding && binding.allKeysPressed) {
			binding.allKeysPressed = false;
		}
	}
}

bool CustomBindingsPlugin::CheckForAllKeysPressed(const CustomBinding& binding)
{
	if (binding.keyListFnameIndex.size() == 0) { return false; }
	for (const auto& fnameIndex : binding.keyListFnameIndex) {
		if (!keysPressed[fnameIndex].isPressed)
			return false;
	}
	return true;
}

void CustomBindingsPlugin::AddBinding()
{
	bindings.push_back(CustomBinding());
	UpdateSelectedBinding(bindings.size() - 1);
}

void CustomBindingsPlugin::SwapBindings(const int& originalIndex, const int& newIndex)
{
	if (originalIndex < 0 || originalIndex >= bindings.size() || newIndex < 0 || newIndex >= bindings.size())
		return;

	CustomBinding tempBind = bindings[originalIndex];
	bindings[originalIndex] = bindings[newIndex];
	bindings[newIndex] = tempBind;
	ImGui::ResetMouseDragDelta();

	isMovingBinding = true;
	if (originalIndex == BindingSelected.selectedBindingIndex)
		BindingSelected.selectedBindingIndex = newIndex;
	else if (newIndex == BindingSelected.selectedBindingIndex)
		BindingSelected.selectedBindingIndex = originalIndex;
}

void CustomBindingsPlugin::RemoveSelectedBinding()
{
	if (BindingSelected.selectedBindingIndex < 0 || BindingSelected.selectedBindingIndex >= bindings.size())
		return;
	bindings.erase(bindings.begin() + BindingSelected.selectedBindingIndex);

	WriteBindings();
	UpdateSelectedBinding(0);
}

void CustomBindingsPlugin::UpdateSelectedBinding(const int& pos)
{
	if (bindings.size() == 0) {
		BindingSelected.ClearSelectedBinding();
		return;
	}
	if (pos < 0 || pos >= bindings.size()) { return; }

	const auto& curBinding = bindings[pos];
	std::vector<int> keyListIndex = {};
	for (const auto& key : curBinding.keyListFnameIndex) {
		keyListIndex.push_back(fnameIndexToKeyNameIndex[key]);
	}

	BindingSelected.SetNewSelectedBinding(curBinding, pos, keyListIndex);
}

void CustomBindingsPlugin::SaveSelectedBinding()
{
	std::vector<int> fnameIndexList = {};
	for (const auto& index : BindingSelected.keyListIndex) {
		fnameIndexList.push_back(keyNameIndexToFnameIndex[index]);
	}

	auto& curBinding = bindings[BindingSelected.selectedBindingIndex];
	curBinding.keyListFnameIndex = fnameIndexList;
	curBinding.command = BindingSelected.command;
	curBinding.name = BindingSelected.name;
	curBinding.enabled = BindingSelected.enabled;

	WriteBindings();
}

bool CustomBindingsPlugin::ChangesHaveBeenMade()
{
	if (BindingSelected.selectedBindingIndex < 0 || BindingSelected.selectedBindingIndex >= bindings.size())
		return false;
	const CustomBinding& bind = bindings[BindingSelected.selectedBindingIndex];
	bool changesMade = bind.keyListFnameIndex.size() != BindingSelected.keyListIndex.size() || bind.name != BindingSelected.name 
		|| bind.command != BindingSelected.command || bind.enabled != BindingSelected.enabled;
	if (!changesMade) {
		for (int i = 0; i < bind.keyListFnameIndex.size(); i++) {
			if (bind.keyListFnameIndex[i] != keyNameIndexToFnameIndex[BindingSelected.keyListIndex[i]]) {
				changesMade = true;
				break;
			}
		}
	}

	return changesMade;
}

void CustomBindingsPlugin::ReadInOldBindingData()
{
	std::ifstream bindFile(oldBindFilePath);
	if (bindFile.is_open()) {
		std::string line;
		while (std::getline(bindFile, line)) {
			if (line != "{") { break; }
			CustomBinding binding;

			std::getline(bindFile, line);
			if (line.find(keyTag) != std::string::npos) {
				std::string keysString = line.substr(line.find(keyTag) + keyTag.size(), std::string::npos);
				std::string key;
				std::stringstream ss(keysString);
				while (std::getline(ss, key, ','))
					binding.keyListFnameIndex.push_back(gameWrapper->GetFNameIndexByString(key));
			}

			std::getline(bindFile, line);
			if (line.find(nameTag) != std::string::npos) {
				binding.name = line.substr(line.find(nameTag) + nameTag.size(), std::string::npos);
			}

			std::getline(bindFile, line);
			if (line.find(commandTag) != std::string::npos) {
				binding.command = line.substr(line.find(commandTag) + commandTag.size(), std::string::npos);
			}

			std::getline(bindFile, line);
			if (line == "}") {
				bindings.push_back(binding);
			}
		}
	}
	bindFile.close();
}

void CustomBindingsPlugin::ReadInBindings()
{
	if (!std::filesystem::exists(bindFilePath)) {
		bindings = {};
		return;
	}
	try {
		auto dataIn = std::ifstream(bindFilePath);
		nlohmann::json jsonBindings;
		if (dataIn.is_open()) {
			dataIn >> jsonBindings;
		}
		dataIn.close();
		bindings = jsonBindings.get<std::vector<CustomBinding>>();
	}
	catch (std::exception& e) {
		LOG("{}", e.what());
		bindings = {};
	}
}

void CustomBindingsPlugin::WriteBindings()
{
	nlohmann::json jsonBindings = bindings;
	auto dataOut = std::ofstream(bindFilePath);
	if (dataOut.is_open()) {
		dataOut << jsonBindings.dump();
	}
	dataOut.close();
}
