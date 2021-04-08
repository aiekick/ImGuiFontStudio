// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * Copyright 2020 Stephane Cuillerdier (aka Aiekick)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SettingsDlg.h"

#include <MainFrame.h>
#include <Gui/ImWidgets.h>
#include <Helper/ThemeHelper.h>
#include <Res/CustomFont.h>

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <GLFW/glfw3.h>

#define CATEGORY_GENERAL "General"
#define CATEGORY_STYLES "Styles"

// cette classe ne doit rien modifier tant que le user n'a pas appuy� sur ok
// donc ca ne doit rien modifier dans l'app tant que c'est pas ok
// si cancel ca doit tout annuler
// donc on ne manipule aucune varaibles externe dans que pas ok

SettingsDlg::SettingsDlg()
{
	m_ShowDialog = false;
}

SettingsDlg::~SettingsDlg() = default;

void SettingsDlg::OpenDialog()
{
	if (m_ShowDialog)
		return;

	Init();

	Load();

	m_ShowDialog = true;
}

void SettingsDlg::CloseDialog()
{
	m_ShowDialog = false;
}

bool SettingsDlg::DrawDialog()
{
	if (m_ShowDialog)
	{
		bool res = false;

		ImGui::SetNextWindowSizeConstraints(MainFrame::Instance()->m_DisplaySize * 0.5f, MainFrame::Instance()->m_DisplaySize);

		if (ImGui::Begin("Settings"))
		{
			ImGui::Separator();

			DrawCategoryPanes();

			ImGui::SameLine();

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

			ImGui::SameLine();

			DrawContentPane();

			ImGui::Separator();

			DrawButtonsPane();
		}
		ImGui::End();

		return res;
	}

	return false;
}

void SettingsDlg::DrawCategoryPanes()
{
	ImVec2 size = ImGui::GetContentRegionMax() - ImVec2(100, 68);

	ImGui::BeginChild("Categories", ImVec2(100, size.y));
		
	for (auto it = m_Categories.begin(); it != m_Categories.end(); ++it)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			if (ImGui::Selectable(it2->first.c_str(), m_CurrentCategory == it2->first))
			{
				m_CurrentCategory = it2->first;
			}
		}
	}
	
	ImGui::EndChild();
}

void SettingsDlg::DrawContentPane()
{
	ImVec2 size = ImGui::GetContentRegionMax() - ImVec2(100, 68);

	if (!ImGui::GetCurrentWindow()->ScrollbarY)
	{
		size.x -= ImGui::GetStyle().ScrollbarSize;
	}

	ImGui::BeginChild("##Content", size);

	for (auto it = m_Categories.begin(); it != m_Categories.end(); ++it)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			if (m_CurrentCategory == it2->first)
			{
				it2->second(SettingsPaneModeEnum::SETTINGS_PANE_MODE_CONTENT);
			}
		}
	}

	ImGui::EndChild();
}

void SettingsDlg::DrawButtonsPane()
{
	if (ImGui::Button("Cancel"))
	{
		CloseDialog();
	}

	ImGui::SameLine();
	
	if (ImGui::Button("Ok"))
	{
		Save();
		CloseDialog();
	}
}

void SettingsDlg::Load()
{
	for (auto it = m_Categories.begin(); it != m_Categories.end(); ++it)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			it2->second(SettingsPaneModeEnum::SETTINGS_PANE_MODE_LOAD);
		}
	}
}

void SettingsDlg::Save()
{
	for (auto it = m_Categories.begin(); it != m_Categories.end(); ++it)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			it2->second(SettingsPaneModeEnum::SETTINGS_PANE_MODE_SAVE);
		}
	}

	MainFrame::Instance()->Unit();
}

/////////////////////////////////////////////////////////////////////////

void SettingsDlg::Init()
{
	//m_Categories[0][CATEGORY_GENERAL] = std::bind(&SettingsDlg::DrawPane_General, this, std::placeholders::_1);
	m_Categories[0][CATEGORY_STYLES] = std::bind(&SettingsDlg::DrawPane_Style, this, std::placeholders::_1);
	
	m_CurrentCategory = CATEGORY_STYLES;
}

void SettingsDlg::DrawPane_General(SettingsPaneModeEnum vMode)
{
	// 
	if (vMode == SettingsPaneModeEnum::SETTINGS_PANE_MODE_LOAD)
	{

	}
	else if (vMode == SettingsPaneModeEnum::SETTINGS_PANE_MODE_SAVE)
	{

	}
	else if(vMode == SettingsPaneModeEnum::SETTINGS_PANE_MODE_CONTENT)
	{
		
	}
}

void SettingsDlg::DrawPane_Style(SettingsPaneModeEnum vMode)
{
	if (vMode == SettingsPaneModeEnum::SETTINGS_PANE_MODE_LOAD)
	{
		
	}
	else if (vMode == SettingsPaneModeEnum::SETTINGS_PANE_MODE_SAVE)
	{
		
	}
	else if (vMode == SettingsPaneModeEnum::SETTINGS_PANE_MODE_CONTENT)
	{
		ImGuiStyle* ref = nullptr;

		// You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
		ImGuiStyle& style = ImGui::GetStyle();
		static ImGuiStyle ref_saved_style;

		// Default to using internal storage as reference
		static bool init = true;
		if (init/* && ref == nullptr*/)
			ref_saved_style = style;
		init = false;
		//if (ref == nullptr)
			ref = &ref_saved_style;

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

		// Save/Revert button
		if (ImGui::Button("Save Ref"))
			*ref = ref_saved_style = style;
		ImGui::SameLine();
		if (ImGui::Button("Revert Ref"))
			style = *ref;
		ImGui::SameLine();
		ImGui::HelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. Use \"Export\" below to save them somewhere.");

		ImGui::Separator();

		if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Sizes"))
			{
				ImGui::Text("Main");
				ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
				ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
				ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
				ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
				ImGui::Text("Borders");
				ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::Text("Rounding");
				ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
				ImGui::Text("Alignment");
				ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
				int window_menu_button_position = style.WindowMenuButtonPosition + 1;
				if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
					style.WindowMenuButtonPosition = window_menu_button_position - 1;
				ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
				ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a button is larger than its text content.");
				ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a selectable is larger than its text content.");
				ImGui::Text("Safe Area Padding"); ImGui::SameLine(); ImGui::HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
				ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Colors"))
			{
				static ImGuiTextFilter filter;
				filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

				static ImGuiColorEditFlags alpha_flags = 0;
				if (ImGui::RadioButton("Opaque", alpha_flags == 0)) { alpha_flags = 0; } ImGui::SameLine();
				if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
				if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
				ImGui::HelpMarker("In the color list:\nLeft-click on colored square to open color picker,\nRight-click to open edit options menu.");

				ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
				ImGui::PushItemWidth(400);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const char* name = ImGui::GetStyleColorName(i);
					if (!filter.PassFilter(name))
						continue;
					ImGui::PushID(i);

					ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
					if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
					{
						// Tips: in a real user application, you may want to merge and use an icon font into the main font, so instead of "Save"/"Revert" you'd use icons.
						// Read the FAQ and docs/FONTS.txt about using icon fonts. It's really easy and super convenient!
						ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) ref->Colors[i] = style.Colors[i];
						ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) style.Colors[i] = ref->Colors[i];
					}
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
					ImGui::TextUnformatted(name);
					ImGui::PopID();
				}
				ImGui::PopItemWidth();
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::PopItemWidth();
	}
}