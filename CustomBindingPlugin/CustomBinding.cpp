#include "pch.h"
#include "CustomBinding.h"

CustomBinding::CustomBinding()
{
	keyList = {};
	command = "";
	allKeysPressed = false;
}

CustomBinding::CustomBinding(CustomBinding& binding)
{
	keyList = binding.keyList;
	command = binding.command;
	allKeysPressed = binding.allKeysPressed;
}

CustomBinding::CustomBinding(std::vector<std::string> newKeyList, std::string newCommand)
{
	keyList = newKeyList;
	command = newCommand;
	allKeysPressed = false;
}

std::string CustomBinding::GetKeyString()
{
	if (keyList.size() == 0) { return ""; }

	std::string keyString = "";
	for (int i = 0; i < keyList.size() - 1; i++) {
		keyString += keyList[i] + ",";
	}

	keyString += keyList[keyList.size() - 1];
	return keyString;
}
