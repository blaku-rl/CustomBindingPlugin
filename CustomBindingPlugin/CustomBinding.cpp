#include "pch.h"
#include "CustomBinding.h"

CustomBinding::CustomBinding()
{
	key1 = "";
	key2 = "";
	key3 = "";
	command = "";
	allKeysPressed = false;
}

CustomBinding::CustomBinding(CustomBinding& binding)
{
	key1 = binding.key1;
	key2 = binding.key2;
	key3 = binding.key3;
	command = binding.command;
	allKeysPressed = binding.allKeysPressed;
}

CustomBinding::CustomBinding(std::string key1New, std::string key2New, std::string key3New, std::string newCommand)
{
	SetAllKeys(key1New, key2New, key3New);
	command = newCommand;
	allKeysPressed = false;
}

void CustomBinding::SetAllKeys(std::string key1New, std::string key2New, std::string key3New)
{
	key1 = key1New;
	key2 = key2New;
	key3 = key3New;
}

std::string CustomBinding::GetKeyString()
{
	return key1 + "," + key2 + "," + key3;
}

bool CustomBinding::AreKeysValid()
{
	return keys.find(key1) != keys.end() && keys.find(key2) != keys.end() && keys.find(key3) != keys.end();
}
