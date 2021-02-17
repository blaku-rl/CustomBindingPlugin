#pragma once
#include "Key.h"
#include <vector>

class CustomBinding {
	//If we make bindings be of custom size, we can change these to a vector/map of keys
	//These members can stay public until the potentinal vector/map change
public:
	std::vector<std::string> keyList;
	std::string command;
	bool allKeysPressed;

	CustomBinding();
	CustomBinding(CustomBinding& binding);
	CustomBinding(std::vector<std::string> newKeyList, std::string newCommand);

	void SetKeys(std::string key1New, std::string key2New, std::string key3New);
	std::string GetKeyString();
};