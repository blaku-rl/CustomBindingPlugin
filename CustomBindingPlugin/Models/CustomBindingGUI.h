#pragma once
#include "CustomBinding.h"

class CustomBindingGUI {
public:
	std::vector<int> keyListIndex;
	int selectedBindingIndex;
	int bindingKeyChangeDesired;
	std::string name;
	std::string command;
	bool enabled;

	CustomBindingGUI();
	void ClearSelectedBinding();
	void SetNewSelectedBinding(const CustomBinding& binding, const int& bindingIndex, const std::vector<int>& newKeyListIndex);
	bool IsKeyChangeDesired();
	void UpdateSelectedKey(const int& keyIndex);
	void SetKeyToChange(const int& pos);
	void RemoveSelectedKey(const int& pos);
	void AddNewKey(const bool& useKeyPress);
};