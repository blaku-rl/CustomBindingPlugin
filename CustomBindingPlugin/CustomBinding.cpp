#include "pch.h"
#include "CustomBinding.h"

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
	return key1 + "+" + key2 + "+" + key3;
}
