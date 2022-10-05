#include "CustomBinding.h"

CustomBinding::CustomBinding()
{
	keyListFnameIndex = {};
	name = "";
	command = "";
	allKeysPressed = false;
	enabled = true;
}

CustomBinding::CustomBinding(const CustomBinding& binding)
{
	keyListFnameIndex = binding.keyListFnameIndex;
	name = binding.name;
	command = binding.command;
	allKeysPressed = binding.allKeysPressed;
	enabled = binding.enabled;
}

CustomBinding::CustomBinding(std::vector<int> newKeyListFnameIndex, std::string newName, std::string newCommand, bool isEnabled)
{
	keyListFnameIndex = newKeyListFnameIndex;
	name = newName;
	command = newCommand;
	allKeysPressed = false;
	enabled = isEnabled;
}