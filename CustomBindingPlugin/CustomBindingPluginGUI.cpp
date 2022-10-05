#include "pch.h"
#include "CustomBindingsPlugin.h"
#include "imgui/imgui_internal.h"
#include "IMGUI/imgui_searchablecombo.h"
#include "IMGUI/imgui_stdlib.h"

// Do ImGui rendering heref
void CustomBindingsPlugin::Render()
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(800, 600), ImVec2(FLT_MAX, FLT_MAX));
	if (ImGui::Begin(menuTitleDisplay.c_str(), &isWindowOpen_)) {
		if (!isSetupComplete) {
			ImGui::Text("Loading Plugin data, please wait");
		}
		else {
			RenderAllBindings();
			ImGui::SameLine();
			RenderBindingDetails();
		}
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
		for (int i = 0; i < bindings.size(); i++)
		{
			std::string nameLabel = (*showEnabledStatus ? (bindings[i].enabled ? *enabledTag : *disabledTag) : "") + bindings[i].name;
			if (ImGui::Selectable(nameLabel.c_str(), i == BindingSelected.selectedBindingIndex)) {
				if (isMovingBinding)
					isMovingBinding = false;
				else if (ChangesHaveBeenMade() && !*autoSaveOnChanges) {
					bindIndexAfterSave = i;
					ImGui::OpenPopup("Save Binding Reminder");
				}
				else {
					gameWrapper->Execute([this, i](GameWrapper* gw) {
						if (*autoSaveOnChanges)
							SaveSelectedBinding();
						UpdateSelectedBinding(i);
						});
				}
			}

			HandleDraggingOnList(i);
		}
		if (ImGui::ButtonEx("Add New Binding", ImVec2(-1, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
			if (ChangesHaveBeenMade()) {
				ImGui::OpenPopup("Save Binding Reminder");
			}
			gameWrapper->Execute([this](GameWrapper* gw) {
				AddBinding();
				});
		}
		SaveBindingReminder();
	}
	ImGui::EndChild();
}

void CustomBindingsPlugin::HandleDraggingOnList(const int& originalIndex)
{
	if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
	{
		int newIndex = originalIndex + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
		gameWrapper->Execute([this, originalIndex, newIndex](GameWrapper* gw) {
			SwapBindings(originalIndex, newIndex);
			});
	}
}

void CustomBindingsPlugin::SaveBindingReminder()
{
	if (ImGui::BeginPopupModal("Save Binding Reminder", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Would you like to save your changes?");
		if (ImGui::Button("Save", ImVec2(120, 0))) {
			gameWrapper->Execute([this](GameWrapper* gw) {
				SaveSelectedBinding();
				UpdateSelectedBinding(bindIndexAfterSave);
				bindIndexAfterSave = 0;
				});
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Discard", ImVec2(120, 0))) {
			gameWrapper->Execute([this](GameWrapper* gw) {
				UpdateSelectedBinding(bindIndexAfterSave);
				bindIndexAfterSave = 0;
				});
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void CustomBindingsPlugin::RenderBindingDetails()
{
	if (ImGui::BeginChild("##BindingsView", ImVec2(0, 0), true)) {
		if (BindingSelected.selectedBindingIndex >= 0 && BindingSelected.selectedBindingIndex < bindings.size()) {
			ImGui::TextUnformatted(BindingSelected.name.c_str());
			ImGui::Separator();
			RenderEditBindOptions();
			RenderSaveOptions();
			RenderDeleteBind();
		}
	}

	ImGui::EndChild();
}

void CustomBindingsPlugin::RenderEditBindOptions()
{
	ImGui::Checkbox("Enable Binding", &BindingSelected.enabled);
	ImGui::TextUnformatted("Keys:");
	if (BindingSelected.keyListIndex.size() > 0) {
		for (int i = 0; i < BindingSelected.keyListIndex.size(); i++) {
			if (*useKeyPress) {
				std::string keyLabel = BindingSelected.keyListIndex[i] == 0 ? "Press Any Key" : keyNames[BindingSelected.keyListIndex[i]];
				if (ImGui::ButtonEx(keyLabel.c_str(), ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
					gameWrapper->Execute([this, i](GameWrapper* gw) {
						BindingSelected.SetKeyToChange(i);
						});
				}
			}
			else {
				std::string label = "Key " + std::to_string(i + 1);
				ImGui::SearchableCombo(label.c_str(), &BindingSelected.keyListIndex[i], keyNames, "", "");
			}

			ImGui::SameLine();
			std::string removeLine = "Remove Key##" + std::to_string(i + 1);
			if (ImGui::ButtonEx(removeLine.c_str(), ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
				gameWrapper->Execute([this, i](GameWrapper* gw) {
					BindingSelected.RemoveSelectedKey(i);
					});
			}
		}
	}
	if (ImGui::ButtonEx("Add New Key", ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
		gameWrapper->Execute([this](GameWrapper* gw) {
			BindingSelected.AddNewKey(*useKeyPress);
			});
	}
	ImGui::TextUnformatted("Name:");
	ImGui::SameLine();
	ImGui::InputText("##name", &BindingSelected.name, ImGuiTextFlags_None);
	ImGui::TextUnformatted("Command:");
	ImGui::SameLine();
	ImGui::InputText("##command", &BindingSelected.command, ImGuiTextFlags_None);
	if (ImGui::ButtonEx("Run Command", ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
		gameWrapper->Execute([this](GameWrapper* gw) {
			cvarManager->executeCommand(BindingSelected.command);
			});
	}
}

void CustomBindingsPlugin::RenderSaveOptions()
{
	if (ChangesHaveBeenMade()) {
		if (*autoSaveOnChanges) {
			SaveSelectedBinding();
			return;
		}
		if (ImGui::ButtonEx("Save Changes", ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
			gameWrapper->Execute([this](GameWrapper* gw) {
				SaveSelectedBinding();
				});
		}
		ImGui::SameLine();
		if (ImGui::ButtonEx("Discard Changes", ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
			gameWrapper->Execute([this](GameWrapper* gw) {
				UpdateSelectedBinding(BindingSelected.selectedBindingIndex);
				});
		}
	}
}

void CustomBindingsPlugin::RenderDeleteBind()
{
	ImGui::PushID(0);
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
	if (ImGui::Button("Delete Binding", ImVec2(0, 0))) {
		ImGui::PopStyleColor(3);
		ImGui::PopID();
		ImGui::OpenPopup("Delete Binding Confirmation");
	}
	else {
		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}

	if (ImGui::BeginPopupModal("Delete Binding Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Are you sure you want to delete this bind?");
		ImGui::PushID(0);
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
		if (ImGui::Button("Delete", ImVec2(120, 0))) {
			gameWrapper->Execute([this](GameWrapper* gw) {
				RemoveSelectedBinding();
				});
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor(3);
		ImGui::PopID();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
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
bool CustomBindingsPlugin::ShouldBlockInput()
{
	bool imguiIO = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
	bool keyChangeListen = !BindingSelected.IsKeyChangeDesired();

	return imguiIO && keyChangeListen;
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
	WriteBindings();
}

// Setting File Rendering
std::string CustomBindingsPlugin::GetPluginName()
{
	return "Custom Bindings Plugin 2.0";
}

void CustomBindingsPlugin::RenderSettings()
{
	std::string helpText = "If you want to open the plugin menu from a command run/bind: togglemenu " + GetMenuName() + ";";
	ImGui::TextUnformatted(helpText.c_str());

	RenderBoolCvar("custombindings_usekeypress", "Use Key Press For Setting Keys");
	RenderBoolCvar("custombindings_autosavechanges", "Automatically save changes made to a bind");
	RenderBoolCvar("custombindings_showenabledstatus", "Show enabled status for each bind to the left of their name");
	if (*showEnabledStatus) {
		RenderStringCvar("custombindings_enabledtag", "Enabled Indicator:", "##enabledindic");
		RenderStringCvar("custombindings_disabledtag", "Disabled Indicator:", "##disabledindic");
	}

	if (ImGui::ButtonEx("Open Bindings Menu", ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine)) {
		gameWrapper->Execute([this](GameWrapper* gw) {
			cvarManager->executeCommand("togglemenu " + GetMenuName());
			});
	}
}

void CustomBindingsPlugin::RenderBoolCvar(const char* cvarName, const char* description)
{
	CVarWrapper boolCvar = cvarManager->getCvar(cvarName);
	if (!boolCvar) return;
	bool enabled = boolCvar.getBoolValue();
	if (ImGui::Checkbox(description, &enabled)) {
		gameWrapper->Execute([this, boolCvar, enabled](...) mutable {
			boolCvar.setValue(enabled);
			});
	}
}

void CustomBindingsPlugin::RenderStringCvar(const char* cvarName, const char* description, const char* label)
{
	CVarWrapper strCvar = cvarManager->getCvar(cvarName);
	if (!strCvar) return;
	std::string str = strCvar.getStringValue();
	ImGui::TextUnformatted(description);
	ImGui::SameLine();
	if (ImGui::InputText(label, &str, ImGuiTextFlags_None)) {
		gameWrapper->Execute([this, strCvar, str](...) mutable {
			strCvar.setValue(str);
			});
	}
}
