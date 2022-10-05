#include "CustomBindingGUI.h"

CustomBindingGUI::CustomBindingGUI()
{
	ClearSelectedBinding();
}

void CustomBindingGUI::ClearSelectedBinding()
{
	keyListIndex = {};
	selectedBindingIndex = -1;
	bindingKeyChangeDesired = -1;
	name = "";
	command = "";
	enabled = false;
}

void CustomBindingGUI::SetNewSelectedBinding(const CustomBinding& binding, const int& bindingIndex, const std::vector<int>& newKeyListIndex)
{
	keyListIndex = newKeyListIndex;
	selectedBindingIndex = bindingIndex;
	name = binding.name;
	command = binding.command;
	enabled = binding.enabled;
}

bool CustomBindingGUI::IsKeyChangeDesired()
{
	return bindingKeyChangeDesired >= 0 && bindingKeyChangeDesired < keyListIndex.size();
}

void CustomBindingGUI::UpdateSelectedKey(const int& keyIndex)
{
	keyListIndex[bindingKeyChangeDesired] = keyIndex;
	bindingKeyChangeDesired = -1;
}

void CustomBindingGUI::SetKeyToChange(const int& pos)
{
	if (pos < 0 || pos >= keyListIndex.size()) { return; }

	bindingKeyChangeDesired = pos;
	keyListIndex[bindingKeyChangeDesired] = 0;
}

void CustomBindingGUI::RemoveSelectedKey(const int& pos)
{
	if (pos < 0 || pos >= keyListIndex.size()) { return; }
	keyListIndex.erase(keyListIndex.begin() + pos);
}

void CustomBindingGUI::AddNewKey(const bool& useKeyPress)
{
	keyListIndex.push_back(0);
	if (useKeyPress) {
		bindingKeyChangeDesired = keyListIndex.size() - 1;
	}
}