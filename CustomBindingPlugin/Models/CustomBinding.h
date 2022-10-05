#pragma once
#include "Key.h"
#include <vector>

class CustomBinding {
public:
	std::vector<int> keyListFnameIndex;
	std::string name;
	std::string command;
	bool allKeysPressed;
	bool enabled;

	CustomBinding();
	CustomBinding(const CustomBinding& binding);
	CustomBinding(std::vector<int> newKeyListFnameIndex, std::string newName, std::string newCommand, bool isEnabled);
};