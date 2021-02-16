#include "pch.h"
#include "CustomBindingPlugin.h"
#include "imgui/imgui_internal.h"


// Do ImGui rendering here
void CustomBindingPlugin::Render()
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(800, 600), ImVec2(FLT_MAX, FLT_MAX));
	if (ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_)) {
		RenderAllBindings();
		ImGui::SameLine();
		RenderBindingDetails();
	}
	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

void CustomBindingPlugin::RenderAllBindings()
{
	if (ImGui::BeginChild("##BindingsList", ImVec2(200, 0), true)) {
		ImGui::TextUnformatted("Bindings:");
		ImGui::Separator();
		size_t i = 0;
		for (auto& binding : bindings) {
			if (ImGui::ButtonEx(binding->GetKeyString().c_str(), ImVec2(-1, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
				UpdateSelectedBinding(i);
			}
			i++;
		}
		if (ImGui::ButtonEx("Add New Binding", ImVec2(-1, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
			AddBinding();
		}
		ImGui::EndChild();
	}
}

void CustomBindingPlugin::RenderBindingDetails()
{
	if (ImGui::BeginChild("##BindingsView", ImVec2(0, 0), true)) {
		if (guiBindingSelectedPos >= 0 && guiBindingSelectedPos < bindings.size()) {
			std::string anyKeyText = "Press Any Key";
			ImGui::TextUnformatted(guiBindingSelected.GetKeyString().c_str());
			ImGui::Separator();
			ImGui::TextUnformatted("Keys:");
			ImGui::SameLine();
			//TODO see why the Press Any Key is not displayed on the third button unless the other 2 have been chosen
			if (ImGui::ButtonEx((bindingChangeDesired == 1 ? anyKeyText : guiBindingSelected.key1).c_str(), ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
				bindingChangeDesired = 1;
			}
			ImGui::SameLine();
			if (ImGui::ButtonEx((bindingChangeDesired == 2 ? anyKeyText : guiBindingSelected.key2).c_str(), ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
				bindingChangeDesired = 2;
			}
			ImGui::SameLine();
			if (ImGui::ButtonEx((bindingChangeDesired == 3 ? anyKeyText : guiBindingSelected.key3).c_str(), ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
				bindingChangeDesired = 3;
			}
			ImGui::TextUnformatted("Command:");
			ImGui::SameLine();
			ImGui::InputTextEx("", "", commandBuffer, sizeof(commandBuffer), ImVec2(0, 0), ImGuiTextFlags_None);
			if (ImGui::ButtonEx("Save", ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
				SaveSelectedBinding();
			}
			ImGui::SameLine();
			if (ImGui::ButtonEx("Remove", ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
				RemoveSelectedBinding();
			}
		}
		ImGui::EndChild();
	}
}

// Name of the menu that is used to toggle the window.
std::string CustomBindingPlugin::GetMenuName()
{
	return "custombindingplugin";
}

// Title to give the menu
std::string CustomBindingPlugin::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void CustomBindingPlugin::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
//TODO See if there is a way that we can get the input without just setting this to false
bool CustomBindingPlugin::ShouldBlockInput()
{
	return false; //ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool CustomBindingPlugin::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void CustomBindingPlugin::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void CustomBindingPlugin::OnClose()
{
	isWindowOpen_ = false;
}

