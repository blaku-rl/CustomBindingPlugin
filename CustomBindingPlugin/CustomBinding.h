#pragma once
#include "Key.h"

class CustomBinding {
	//If we make bindings be of custom size, we can change these to a vector/map of keys
	//These members can stay public until the potentinal vector/map change
public:
	std::string key1;
	std::string key2;
	std::string key3;
	std::string command;
	bool allKeysPressed;

	CustomBinding();
	CustomBinding(CustomBinding& binding);
	CustomBinding(std::string key1New, std::string key2New, std::string key3New, std::string newCommand);

	void SetAllKeys(std::string key1New, std::string key2New, std::string key3New);
	std::string GetKeyString();
	bool AreKeysValid();
};