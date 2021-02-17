#include "pch.h"
#include "CustomBindingsPlugin.h"
#include "imgui/imgui_internal.h"


// Do ImGui rendering here
void CustomBindingsPlugin::Render()
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(800, 600), ImVec2(FLT_MAX, FLT_MAX));
	if (ImGui::Begin(menuTitleDisplay.c_str(), &isWindowOpen_)) {
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

void CustomBindingsPlugin::RenderAllBindings()
{
	if (ImGui::BeginChild("##BindingsList", ImVec2(250, 0), true)) {
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

void CustomBindingsPlugin::RenderBindingDetails()
{
	if (ImGui::BeginChild("##BindingsView", ImVec2(0, 0), true)) {
		if (guiBindingSelectedPos >= 0 && guiBindingSelectedPos < bindings.size()) {
			std::string anyKeyText = "Press Any Key";
			ImGui::TextUnformatted(guiBindingSelected.GetKeyString().c_str());
			ImGui::Separator();
			ImGui::TextUnformatted("Keys:");
			ImGui::SameLine();
			if (guiBindingSelected.keyList.size() > 0) {
				for (int i = 0; i < guiBindingSelected.keyList.size(); i++) {
					if (ImGui::ButtonEx((bindingChangeDesired == i ? anyKeyText : guiBindingSelected.keyList[i]).c_str(), ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
						bindingChangeDesired = 1;
					}
					ImGui::SameLine();
				}
			}
			if (ImGui::ButtonEx("Add New Key", ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
				AddKeyToSelectedBinding();
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
std::string CustomBindingsPlugin::GetMenuName()
{
	return "custombindingsplugin";
}

// Title to give the menu
std::string CustomBindingsPlugin::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void CustomBindingsPlugin::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
//TODO See if there is a way that we can get the input without just setting this to false
bool CustomBindingsPlugin::ShouldBlockInput()
{
	return false; //ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool CustomBindingsPlugin::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void CustomBindingsPlugin::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void CustomBindingsPlugin::OnClose()
{
	isWindowOpen_ = false;
}

