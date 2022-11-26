// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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
#include "ThemeHelper.h"

#include <Contrib/FontIcons/CustomFont.h>

#include <Gui/ImWidgets.h>
#include <ctools/cTools.h>
#include <imgui/imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui/imgui_internal.h>

//// STATIC ///////////////////////////////////////////////////////////////////////////////////

#ifdef USE_SHADOW
float ThemeHelper::puShadowStrength = 0.5f; // low value is darker than higt (0.0f - 2.0f)
bool ThemeHelper::puUseShadow = false;
bool ThemeHelper::puUseTextureForShadow = false;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////

ThemeHelper::ThemeHelper()
{
	prFileTypeInfos[".ttf"] = IGFD::FileStyle(ImVec4(0.1f, 0.9f, 0.5f, 1.0f));
	prFileTypeInfos[".otf"] = IGFD::FileStyle(ImVec4(0.1f, 0.1f, 0.5f, 1.0f));
	prFileTypeInfos[".cpp"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.7f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".h"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".ifs"] = IGFD::FileStyle(ImVec4(0.1f, 0.5f, 0.1f, 1.0f), ICON_IGFS_FILE_TYPE_PROJECT);

	ImGui::CustomStyle::Instance()->Instance();
	ApplyStyleColorsDefault();
}

ThemeHelper::~ThemeHelper() = default;

void ThemeHelper::Draw()
{
	if (puShowImGuiStyleEdtor)
		ShowCustomImGuiStyleEditor(&puShowImGuiStyleEdtor);
}

void ThemeHelper::DrawMenu()
{
	if (ImGui::BeginMenu("General UI"))
	{
		if (ImGui::MenuItem("Orange/Blue (Default)")) ApplyStyleColorsOrangeBlue();
		if (ImGui::MenuItem("Green/Blue")) ApplyStyleColorsGreenBlue();
		if (ImGui::MenuItem("Classic")) ApplyStyleColorsClassic();
		if (ImGui::MenuItem("Dark"))	ApplyStyleColorsDark();
		if (ImGui::MenuItem("Darcula"))	ApplyStyleColorsDarcula();
		if (ImGui::MenuItem("RedDark"))	ApplyStyleColorsRedDark();
		if (ImGui::MenuItem("Light"))	ApplyStyleColorsLight();

		ImGui::Separator();

		ImGui::MenuItem("Customize", "", &puShowImGuiStyleEdtor);
		
		ImGui::Separator();
		
		if (ImGui::BeginMenu("Contrast"))
		{
			ImGui::DrawContrastWidgets();

			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("File Type Colors"))
	{
		bool fileColorUpdate = false;

		for (auto &it : prFileTypeInfos)
		{
			fileColorUpdate |= ImGui::ColorEdit4(it.first.c_str(), &prFileTypeInfos[it.first].color.x);
		}

		if (fileColorUpdate)
		{
			ApplyFileTypeColors();
		}

		ImGui::EndMenu();
	}
}

/* default theme */
void ThemeHelper::ApplyStyleColorsDefault()
{
	ApplyStyleColorsOrangeBlue();
}

void ThemeHelper::ApplyStyleColorsOrangeBlue()
{
	const auto colors = prImGuiStyle.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.16f, 0.17f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.26f, 0.28f, 0.29f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.21f, 0.29f, 0.36f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.71f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.93f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.20f, 0.21f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.23f, 0.25f, 0.26f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.30f, 0.33f, 0.35f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.21f, 0.29f, 0.36f, 0.89f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.13f, 0.52f, 0.94f, 0.45f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.13f, 0.71f, 1.00f, 0.89f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.24f, 0.78f, 0.78f, 0.31f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(1.00f, 0.60f, 0.00f, 0.80f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.48f, 0.00f, 0.80f);
	colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.40f, 0.00f, 0.80f);
	colors[ImGuiCol_Header] = ImVec4(0.13f, 0.52f, 0.94f, 0.66f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.52f, 0.94f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.52f, 0.94f, 0.59f);
	colors[ImGuiCol_Separator] = ImVec4(0.18f, 0.35f, 0.58f, 0.59f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.41f, 0.68f, 0.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.41f, 0.68f, 0.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.13f, 0.52f, 0.94f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.13f, 0.52f, 0.94f, 0.95f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	ImGui::CustomStyle::Instance()->Instance()->GoodColor = ImVec4(0.00f, 0.77f, 0.00f, 1.00f);
	ImGui::CustomStyle::Instance()->BadColor = ImVec4(0.92f, 0.28f, 0.10f, 1.00f);
	ImGui::CustomStyle::Instance()->GlyphButtonColor = ImVec4(0.09f, 0.31f, 0.47f, 1.00f);
	ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ImVec4(1.00f, 0.60f, 0.00f, 0.80f);

	// Main
	prImGuiStyle.WindowPadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.FramePadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemInnerSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	prImGuiStyle.IndentSpacing = 4.00f;
	prImGuiStyle.ScrollbarSize = 10.00f;
	prImGuiStyle.GrabMinSize = 8.00f;

	// Borders
	prImGuiStyle.WindowBorderSize = 0.00f;
	prImGuiStyle.ChildBorderSize = 0.00f;
	prImGuiStyle.PopupBorderSize = 1.00f;
	prImGuiStyle.FrameBorderSize = 0.00f;
	prImGuiStyle.TabBorderSize = 0.00f;

	// Rounding
	prImGuiStyle.WindowRounding = 2.00f;
	prImGuiStyle.ChildRounding = 2.00f;
	prImGuiStyle.FrameRounding = 2.00f;
	prImGuiStyle.PopupRounding = 2.00f;
	prImGuiStyle.ScrollbarRounding = 2.00f;
	prImGuiStyle.GrabRounding = 2.00f;
	prImGuiStyle.TabRounding = 2.00f;

	// Alignment
	prImGuiStyle.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.WindowMenuButtonPosition = ImGuiDir_Left;
	prImGuiStyle.ColorButtonPosition = ImGuiDir_Right;
	prImGuiStyle.ButtonTextAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.SelectableTextAlign = ImVec2(0.00f, 0.50f);

	// Safe Area Padding
	prImGuiStyle.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

	prFileTypeInfos[".ttf"] = IGFD::FileStyle(ImVec4(0.1f, 0.9f, 0.5f, 1.0f));
	prFileTypeInfos[".otf"] = IGFD::FileStyle(ImVec4(0.1f, 0.1f, 0.5f, 1.0f));
	prFileTypeInfos[".cpp"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.7f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".h"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".ifs"] = IGFD::FileStyle(ImVec4(0.1f, 0.5f, 0.1f, 1.0f), ICON_IGFS_FILE_TYPE_PROJECT);

	ApplyFileTypeColors();
}

void ThemeHelper::ApplyStyleColorsGreenBlue()
{
	const auto colors = prImGuiStyle.Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.44f, 0.44f, 0.60f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.70f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.80f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_Button] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_Header] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.36f, 0.36f, 0.36f, 0.54f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.00f, 0.57f, 0.38f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.11f, 0.90f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	ImGui::CustomStyle::Instance()->GoodColor = ImVec4(0.20f, 0.80f, 0.20f, 1.00f);
	ImGui::CustomStyle::Instance()->BadColor = ImVec4(0.80f, 0.20f, 0.20f, 1.00f);
	ImGui::CustomStyle::Instance()->GlyphButtonColor = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);


	// Main
	prImGuiStyle.WindowPadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.FramePadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemInnerSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	prImGuiStyle.IndentSpacing = 10.00f;
	prImGuiStyle.ScrollbarSize = 20.00f;
	prImGuiStyle.GrabMinSize = 4.00f;

	// Borders
	prImGuiStyle.WindowBorderSize = 0.00f;
	prImGuiStyle.ChildBorderSize = 0.00f;
	prImGuiStyle.PopupBorderSize = 0.00f;
	prImGuiStyle.FrameBorderSize = 1.00f;
	prImGuiStyle.TabBorderSize = 0.00f;

	// Rounding
	prImGuiStyle.WindowRounding = 0.00f;
	prImGuiStyle.ChildRounding = 0.00f;
	prImGuiStyle.FrameRounding = 0.00f;
	prImGuiStyle.PopupRounding = 0.00f;
	prImGuiStyle.ScrollbarRounding = 0.00f;
	prImGuiStyle.GrabRounding = 0.00f;
	prImGuiStyle.TabRounding = 0.00f;

	// Alignment
	prImGuiStyle.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.WindowMenuButtonPosition = ImGuiDir_Left;
	prImGuiStyle.ColorButtonPosition = ImGuiDir_Right;
	prImGuiStyle.ButtonTextAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.SelectableTextAlign = ImVec2(0.00f, 0.50f);

	// Safe Area Padding
	prImGuiStyle.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

	prFileTypeInfos[".ttf"].color = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	prFileTypeInfos[".otf"].color = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	prFileTypeInfos[".cpp"].color = ImVec4(0.5f, 0.9f, 0.1f, 1.0f); // yellow high
	prFileTypeInfos[".h"].color = ImVec4(0.25f, 0.9f, 0.1f, 1.0f); // yellow high
	prFileTypeInfos[".ifs"].color = ImVec4(0.9f, 0.1f, 0.9f, 1.0f); // purple high

	ApplyFileTypeColors();
}

void ThemeHelper::ApplyStyleColorsClassic()
{
	const auto colors = prImGuiStyle.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
	colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
	colors[ImGuiCol_Button] = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
	colors[ImGuiCol_Tab] = ImVec4(0.34f, 0.34f, 0.68f, 0.79f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.40f, 0.40f, 0.73f, 0.84f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.28f, 0.28f, 0.57f, 0.82f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.35f, 0.35f, 0.65f, 0.84f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.29f, 0.00f, 1.00f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.45f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	ImGui::CustomStyle::Instance()->GoodColor = ImVec4(0.20f, 0.80f, 0.20f, 1.00f);
	ImGui::CustomStyle::Instance()->BadColor = ImVec4(1.00f, 0.27f, 0.27f, 1.00f);
	ImGui::CustomStyle::Instance()->GlyphButtonColor = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
	ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);

	// Main
	prImGuiStyle.WindowPadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.FramePadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemInnerSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	prImGuiStyle.IndentSpacing = 10.00f;
	prImGuiStyle.ScrollbarSize = 20.00f;
	prImGuiStyle.GrabMinSize = 4.00f;

	// Borders
	prImGuiStyle.WindowBorderSize = 0.00f;
	prImGuiStyle.ChildBorderSize = 0.00f;
	prImGuiStyle.PopupBorderSize = 0.00f;
	prImGuiStyle.FrameBorderSize = 1.00f;
	prImGuiStyle.TabBorderSize = 0.00f;

	// Rounding
	prImGuiStyle.WindowRounding = 0.00f;
	prImGuiStyle.ChildRounding = 0.00f;
	prImGuiStyle.FrameRounding = 0.00f;
	prImGuiStyle.PopupRounding = 0.00f;
	prImGuiStyle.ScrollbarRounding = 0.00f;
	prImGuiStyle.GrabRounding = 0.00f;
	prImGuiStyle.TabRounding = 0.00f;

	// Alignment
	prImGuiStyle.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.WindowMenuButtonPosition = ImGuiDir_Left;
	prImGuiStyle.ColorButtonPosition = ImGuiDir_Right;
	prImGuiStyle.ButtonTextAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.SelectableTextAlign = ImVec2(0.00f, 0.50f);

	// Safe Area Padding
	prImGuiStyle.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

	prFileTypeInfos[".ttf"] = IGFD::FileStyle(ImVec4(0.1f, 0.9f, 0.5f, 1.0f));
	prFileTypeInfos[".otf"] = IGFD::FileStyle(ImVec4(0.1f, 0.1f, 0.5f, 1.0f));
	prFileTypeInfos[".cpp"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.7f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".h"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".ifs"] = IGFD::FileStyle(ImVec4(0.1f, 0.5f, 0.1f, 1.0f), ICON_IGFS_FILE_TYPE_PROJECT);

	ApplyFileTypeColors();
}

void ThemeHelper::ApplyStyleColorsDark()
{
	const auto colors = prImGuiStyle.Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.02f, 0.00f, 1.00f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	ImGui::CustomStyle::Instance()->GoodColor = ImVec4(0.20f, 0.80f, 0.20f, 1.00f);
	ImGui::CustomStyle::Instance()->BadColor = ImVec4(0.80f, 0.20f, 0.20f, 1.00f);
	ImGui::CustomStyle::Instance()->GlyphButtonColor = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

	// Main
	prImGuiStyle.WindowPadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.FramePadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemInnerSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	prImGuiStyle.IndentSpacing = 10.00f;
	prImGuiStyle.ScrollbarSize = 20.00f;
	prImGuiStyle.GrabMinSize = 4.00f;

	// Borders
	prImGuiStyle.WindowBorderSize = 0.00f;
	prImGuiStyle.ChildBorderSize = 0.00f;
	prImGuiStyle.PopupBorderSize = 0.00f;
	prImGuiStyle.FrameBorderSize = 1.00f;
	prImGuiStyle.TabBorderSize = 0.00f;

	// Rounding
	prImGuiStyle.WindowRounding = 0.00f;
	prImGuiStyle.ChildRounding = 0.00f;
	prImGuiStyle.FrameRounding = 0.00f;
	prImGuiStyle.PopupRounding = 0.00f;
	prImGuiStyle.ScrollbarRounding = 0.00f;
	prImGuiStyle.GrabRounding = 0.00f;
	prImGuiStyle.TabRounding = 0.00f;

	// Alignment
	prImGuiStyle.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.WindowMenuButtonPosition = ImGuiDir_Left;
	prImGuiStyle.ColorButtonPosition = ImGuiDir_Right;
	prImGuiStyle.ButtonTextAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.SelectableTextAlign = ImVec2(0.00f, 0.50f);

	// Safe Area Padding
	prImGuiStyle.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

	prFileTypeInfos[".ttf"] = IGFD::FileStyle(ImVec4(0.1f, 0.9f, 0.5f, 1.0f));
	prFileTypeInfos[".otf"] = IGFD::FileStyle(ImVec4(0.1f, 0.1f, 0.5f, 1.0f));
	prFileTypeInfos[".cpp"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.7f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".h"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".ifs"] = IGFD::FileStyle(ImVec4(0.1f, 0.5f, 0.1f, 1.0f), ICON_IGFS_FILE_TYPE_PROJECT);

	ApplyFileTypeColors();
}

void ThemeHelper::ApplyStyleColorsLight()
{
	const auto colors = prImGuiStyle.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.76f, 0.80f, 0.84f, 0.93f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.60f, 0.73f, 0.88f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.92f, 0.93f, 0.94f, 0.99f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.74f, 0.82f, 0.91f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.00f, 0.01f, 1.00f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.02f, 0.00f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	ImGui::CustomStyle::Instance()->GoodColor = ImVec4(0.20f, 0.50f, 0.20f, 1.00f);
	ImGui::CustomStyle::Instance()->BadColor = ImVec4(0.91f, 0.00f, 0.00f, 1.00f);
	ImGui::CustomStyle::Instance()->GlyphButtonColor = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

	// Main
	prImGuiStyle.WindowPadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.FramePadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemInnerSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	prImGuiStyle.IndentSpacing = 10.00f;
	prImGuiStyle.ScrollbarSize = 20.00f;
	prImGuiStyle.GrabMinSize = 4.00f;

	// Borders
	prImGuiStyle.WindowBorderSize = 0.00f;
	prImGuiStyle.ChildBorderSize = 0.00f;
	prImGuiStyle.PopupBorderSize = 0.00f;
	prImGuiStyle.FrameBorderSize = 1.00f;
	prImGuiStyle.TabBorderSize = 0.00f;

	// Rounding
	prImGuiStyle.WindowRounding = 0.00f;
	prImGuiStyle.ChildRounding = 0.00f;
	prImGuiStyle.FrameRounding = 0.00f;
	prImGuiStyle.PopupRounding = 0.00f;
	prImGuiStyle.ScrollbarRounding = 0.00f;
	prImGuiStyle.GrabRounding = 0.00f;
	prImGuiStyle.TabRounding = 0.00f;

	// Alignment
	prImGuiStyle.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.WindowMenuButtonPosition = ImGuiDir_Left;
	prImGuiStyle.ColorButtonPosition = ImGuiDir_Right;
	prImGuiStyle.ButtonTextAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.SelectableTextAlign = ImVec2(0.00f, 0.50f);

	// Safe Area Padding
	prImGuiStyle.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

	prFileTypeInfos[".ttf"] = IGFD::FileStyle(ImVec4(0.1f, 0.9f, 0.5f, 1.0f));
	prFileTypeInfos[".otf"] = IGFD::FileStyle(ImVec4(0.1f, 0.1f, 0.5f, 1.0f));
	prFileTypeInfos[".cpp"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.7f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".h"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".ifs"] = IGFD::FileStyle(ImVec4(0.1f, 0.5f, 0.1f, 1.0f), ICON_IGFS_FILE_TYPE_PROJECT);

	ApplyFileTypeColors();
}

void ThemeHelper::ApplyStyleColorsDarcula()
{
	// https://github.com/ocornut/imgui/issues/707
	// by ice1000

	const auto colors = prImGuiStyle.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.73f, 0.73f, 0.73f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.24f, 0.25f, 0.25f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.24f, 0.25f, 0.25f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.24f, 0.25f, 0.25f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.33f, 0.33f, 0.33f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.16f, 0.16f, 0.16f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.17f, 0.17f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.45f, 0.68f, 1.00f, 0.67f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.27f, 0.29f, 0.29f, 0.80f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.27f, 0.29f, 0.29f, 0.60f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.31f, 0.42f, 0.51f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.22f, 0.31f, 0.42f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.14f, 0.19f, 0.26f, 0.91f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
	colors[ImGuiCol_Button] = ImVec4(0.33f, 0.35f, 0.36f, 0.49f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.31f, 0.42f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.14f, 0.19f, 0.26f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.33f, 0.35f, 0.36f, 0.53f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.68f, 1.00f, 0.67f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
	colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.68f, 1.00f, 0.67f);
	colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.19f, 0.19f, 0.57f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.74f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	ImGui::CustomStyle::Instance()->GoodColor = ImVec4(0.20f, 0.80f, 0.20f, 1.00f);
	ImGui::CustomStyle::Instance()->BadColor = ImVec4(0.95f, 0.30f, 0.30f, 1.00f);
	ImGui::CustomStyle::Instance()->GlyphButtonColor = ImVec4(0.33f, 0.35f, 0.36f, 0.49f);
	ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ImVec4(0.14f, 0.19f, 0.26f, 1.00f);

	// Main
	prImGuiStyle.WindowPadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.FramePadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemInnerSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	prImGuiStyle.IndentSpacing = 10.00f;
	prImGuiStyle.ScrollbarSize = 20.00f;
	prImGuiStyle.GrabMinSize = 4.00f;

	// Borders
	prImGuiStyle.WindowBorderSize = 0.00f;
	prImGuiStyle.ChildBorderSize = 0.00f;
	prImGuiStyle.PopupBorderSize = 0.00f;
	prImGuiStyle.FrameBorderSize = 1.00f;
	prImGuiStyle.TabBorderSize = 0.00f;

	// Rounding
	prImGuiStyle.WindowRounding = 0.00f;
	prImGuiStyle.ChildRounding = 0.00f;
	prImGuiStyle.FrameRounding = 0.00f;
	prImGuiStyle.PopupRounding = 0.00f;
	prImGuiStyle.ScrollbarRounding = 0.00f;
	prImGuiStyle.GrabRounding = 0.00f;
	prImGuiStyle.TabRounding = 0.00f;

	// Alignment
	prImGuiStyle.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.WindowMenuButtonPosition = ImGuiDir_Left;
	prImGuiStyle.ColorButtonPosition = ImGuiDir_Right;
	prImGuiStyle.ButtonTextAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.SelectableTextAlign = ImVec2(0.00f, 0.50f);

	// Safe Area Padding
	prImGuiStyle.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

	prFileTypeInfos[".ttf"] = IGFD::FileStyle(ImVec4(0.1f, 0.9f, 0.5f, 1.0f));
	prFileTypeInfos[".otf"] = IGFD::FileStyle(ImVec4(0.1f, 0.1f, 0.5f, 1.0f));
	prFileTypeInfos[".cpp"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.7f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".h"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".ifs"] = IGFD::FileStyle(ImVec4(0.1f, 0.5f, 0.1f, 1.0f), ICON_IGFS_FILE_TYPE_PROJECT);

	ApplyFileTypeColors();
}

void ThemeHelper::ApplyStyleColorsRedDark()
{
	const auto colors = prImGuiStyle.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.14f, 0.14f, 0.67f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.20f, 0.20f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.17f, 0.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.33f, 0.35f, 0.36f, 0.53f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.76f, 0.28f, 0.44f, 0.67f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
	colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.86f, 0.23f, 0.43f, 0.67f);
	colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.19f, 0.19f, 0.57f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.74f);
	colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.79f, 0.00f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.98f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	ImGui::CustomStyle::Instance()->GoodColor = ImVec4(0.20f, 0.80f, 0.20f, 1.00f);
	ImGui::CustomStyle::Instance()->BadColor = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	ImGui::CustomStyle::Instance()->GlyphButtonColor = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);

	// Main
	prImGuiStyle.WindowPadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.FramePadding = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.ItemInnerSpacing = ImVec2(4.00f, 4.00f);
	prImGuiStyle.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	prImGuiStyle.IndentSpacing = 10.00f;
	prImGuiStyle.ScrollbarSize = 20.00f;
	prImGuiStyle.GrabMinSize = 4.00f;

	// Borders
	prImGuiStyle.WindowBorderSize = 0.00f;
	prImGuiStyle.ChildBorderSize = 0.00f;
	prImGuiStyle.PopupBorderSize = 0.00f;
	prImGuiStyle.FrameBorderSize = 1.00f;
	prImGuiStyle.TabBorderSize = 0.00f;

	// Rounding
	prImGuiStyle.WindowRounding = 0.00f;
	prImGuiStyle.ChildRounding = 0.00f;
	prImGuiStyle.FrameRounding = 0.00f;
	prImGuiStyle.PopupRounding = 0.00f;
	prImGuiStyle.ScrollbarRounding = 0.00f;
	prImGuiStyle.GrabRounding = 0.00f;
	prImGuiStyle.TabRounding = 0.00f;

	// Alignment
	prImGuiStyle.WindowTitleAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.WindowMenuButtonPosition = ImGuiDir_Left;
	prImGuiStyle.ColorButtonPosition = ImGuiDir_Right;
	prImGuiStyle.ButtonTextAlign = ImVec2(0.50f, 0.50f);
	prImGuiStyle.SelectableTextAlign = ImVec2(0.00f, 0.50f);

	// Safe Area Padding
	prImGuiStyle.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

	prFileTypeInfos[".ttf"] = IGFD::FileStyle(ImVec4(0.1f, 0.9f, 0.5f, 1.0f));
	prFileTypeInfos[".otf"] = IGFD::FileStyle(ImVec4(0.1f, 0.1f, 0.5f, 1.0f));
	prFileTypeInfos[".cpp"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.7f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".h"] = IGFD::FileStyle(ImVec4(0.5f, 0.1f, 0.5f, 1.0f), ICON_IGFS_FILE_TYPE_TEXT);
	prFileTypeInfos[".ifs"] = IGFD::FileStyle(ImVec4(0.1f, 0.5f, 0.1f, 1.0f), ICON_IGFS_FILE_TYPE_PROJECT);

	ApplyFileTypeColors();
}

void ThemeHelper::ApplyFileTypeColors()
{
	for (auto &it : prFileTypeInfos)
	{
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, it.first.c_str(), it.second.color, it.second.icon);
	}

	memcpy(&ImGui::GetStyle(), &prImGuiStyle, sizeof(ImGuiStyle));
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string ThemeHelper::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	{
		prImGuiStyle = prImGuiStyle;
		auto colors = prImGuiStyle.Colors;
		
		str += vOffset + "<ImGui_Styles>\n";
		for (auto i = 0; i < ImGuiCol_COUNT; i++)
		{
			str += vOffset + "\t<" + GetStyleColorName(i) + " value=\"" + ct::fvec4(colors[i]).string() + "\"/>\n";
		}

		str += vOffset + "\t<GoodColor value=\"" + ct::fvec4(ImGui::CustomStyle::Instance()->GoodColor).string() + "\"/>\n";
		str += vOffset + "\t<BadColor value=\"" + ct::fvec4(ImGui::CustomStyle::Instance()->BadColor).string() + "\"/>\n";
		str += vOffset + "\t<GlyphButtonColor value=\"" + ct::fvec4(ImGui::CustomStyle::Instance()->GlyphButtonColor).string() + "\"/>\n";
		str += vOffset + "\t<GlyphButtonColorActive value=\"" + ct::fvec4(ImGui::CustomStyle::Instance()->GlyphButtonColorActive).string() + "\"/>\n";

		str += vOffset + "\t<WindowPadding value=\"" + ct::fvec2(prImGuiStyle.WindowPadding).string() + "\"/>\n";
		str += vOffset + "\t<FramePadding value=\"" + ct::fvec2(prImGuiStyle.FramePadding).string() + "\"/>\n";
		str += vOffset + "\t<ItemSpacing value=\"" + ct::fvec2(prImGuiStyle.ItemSpacing).string() + "\"/>\n";
		str += vOffset + "\t<ItemInnerSpacing value=\"" + ct::fvec2(prImGuiStyle.ItemInnerSpacing).string() + "\"/>\n";
		str += vOffset + "\t<IndentSpacing value=\"" + ct::toStr(prImGuiStyle.IndentSpacing) + "\"/>\n";
		str += vOffset + "\t<ScrollbarSize value=\"" + ct::toStr(prImGuiStyle.ScrollbarSize) + "\"/>\n";
		str += vOffset + "\t<GrabMinSize value=\"" + ct::toStr(prImGuiStyle.GrabMinSize) + "\"/>\n";
		str += vOffset + "\t<WindowRounding value=\"" + ct::toStr(prImGuiStyle.WindowRounding) + "\"/>\n";
		str += vOffset + "\t<ChildRounding value=\"" + ct::toStr(prImGuiStyle.ChildRounding) + "\"/>\n";
		str += vOffset + "\t<FrameRounding value=\"" + ct::toStr(prImGuiStyle.FrameRounding) + "\"/>\n";
		str += vOffset + "\t<PopupRounding value=\"" + ct::toStr(prImGuiStyle.PopupRounding) + "\"/>\n";
		str += vOffset + "\t<ScrollbarRounding value=\"" + ct::toStr(prImGuiStyle.ScrollbarRounding) + "\"/>\n";
		str += vOffset + "\t<GrabRounding value=\"" + ct::toStr(prImGuiStyle.GrabRounding) + "\"/>\n";
		str += vOffset + "\t<TabRounding value=\"" + ct::toStr(prImGuiStyle.TabRounding) + "\"/>\n";
		str += vOffset + "\t<WindowBorderSize value=\"" + ct::toStr(prImGuiStyle.WindowBorderSize) + "\"/>\n";
		str += vOffset + "\t<ChildBorderSize value=\"" + ct::toStr(prImGuiStyle.ChildBorderSize) + "\"/>\n";
		str += vOffset + "\t<PopupBorderSize value=\"" + ct::toStr(prImGuiStyle.PopupBorderSize) + "\"/>\n";
		str += vOffset + "\t<FrameBorderSize value=\"" + ct::toStr(prImGuiStyle.FrameBorderSize) + "\"/>\n";
		str += vOffset + "\t<TabBorderSize value=\"" + ct::toStr(prImGuiStyle.TabBorderSize) + "\"/>\n";
#ifdef USE_SHADOW
		str += vOffset + "\t<useshadow value=\"" + (puUseShadow ? "true" : "false") + "\"/>\n";
		str += vOffset + "\t<shadowstrength value=\"" + ct::toStr(puShadowStrength) + "\"/>\n";
		str += vOffset + "\t<useshadowtexture value=\"" + (puUseTextureForShadow ? "true" : "false") + "\"/>\n";
#endif
		str += vOffset + "</ImGui_Styles>\n";
	}

	{
		str += vOffset + "<FileTypes>\n";
		for (auto& it : prFileTypeInfos)
		{
			str += vOffset + "\t<filetype value=\"" + it.first + "\" color=\"" +
				ct::fvec4(it.second.color).string() + "\"/>\n";
		}
		str += vOffset + "</FileTypes>\n";
	}

	return str;
}

bool ThemeHelper::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "FileTypes")
	{
		std::string fileType;
		std::string color;
		
		for (auto attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			const std::string attValue = attr->Value();

			if (attName == "value") fileType = attValue;
			if (attName == "color") color = attValue;
		}

		prFileTypeInfos[fileType] = IGFD::FileStyle(ct::toImVec4(ct::fvariant(color).GetV4()));
		ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, fileType.c_str(), prFileTypeInfos[fileType]);
	}

	if (strParentName == "ImGui_Styles")
	{
		const auto att = vElem->FirstAttribute();
		if (att && std::string(att->Name()) == "value")
		{
			strValue = att->Value();
			const auto colors = prImGuiStyle.Colors;

			if (strName.find("ImGuiCol") != std::string::npos)
			{
				const auto id = GetImGuiColFromName(strName);
				if (id >= 0)
				{
					colors[id] = ct::toImVec4(ct::fvariant(strValue).GetV4());
					return false;
				}
			}

			if (strName == "GoodColor") ImGui::CustomStyle::Instance()->GoodColor = ct::toImVec4(ct::fvariant(strValue).GetV4());
			else if (strName == "BadColor") ImGui::CustomStyle::Instance()->BadColor = ct::toImVec4(ct::fvariant(strValue).GetV4());
			else if (strName == "GlyphButtonColor") ImGui::CustomStyle::Instance()->GlyphButtonColor = ct::toImVec4(ct::fvariant(strValue).GetV4());
			else if (strName == "GlyphButtonColorActive") ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ct::toImVec4(ct::fvariant(strValue).GetV4());
			else if (strName == "WindowPadding") prImGuiStyle.WindowPadding = ct::toImVec2(ct::fvariant(strValue).GetV2());
			else if (strName == "FramePadding") prImGuiStyle.FramePadding = ct::toImVec2(ct::fvariant(strValue).GetV2());
			else if (strName == "ItemSpacing") prImGuiStyle.ItemSpacing = ct::toImVec2(ct::fvariant(strValue).GetV2());
			else if (strName == "ItemInnerSpacing") prImGuiStyle.ItemInnerSpacing = ct::toImVec2(ct::fvariant(strValue).GetV2());
			else if (strName == "IndentSpacing") prImGuiStyle.IndentSpacing = ct::fvariant(strValue).GetF();
			else if (strName == "ScrollbarSize") prImGuiStyle.ScrollbarSize = ct::fvariant(strValue).GetF();
			else if (strName == "GrabMinSize") prImGuiStyle.GrabMinSize = ct::fvariant(strValue).GetF();
			else if (strName == "WindowRounding") prImGuiStyle.WindowRounding = ct::fvariant(strValue).GetF();
			else if (strName == "ChildRounding") prImGuiStyle.ChildRounding = ct::fvariant(strValue).GetF();
			else if (strName == "FrameRounding") prImGuiStyle.FrameRounding = ct::fvariant(strValue).GetF();
			else if (strName == "PopupRounding") prImGuiStyle.PopupRounding = ct::fvariant(strValue).GetF();
			else if (strName == "ScrollbarRounding") prImGuiStyle.ScrollbarRounding = ct::fvariant(strValue).GetF();
			else if (strName == "GrabRounding") prImGuiStyle.GrabRounding = ct::fvariant(strValue).GetF();
			else if (strName == "TabRounding") prImGuiStyle.TabRounding = ct::fvariant(strValue).GetF();
			else if (strName == "WindowBorderSize") prImGuiStyle.WindowBorderSize = ct::fvariant(strValue).GetF();
			else if (strName == "ChildBorderSize") prImGuiStyle.ChildBorderSize = ct::fvariant(strValue).GetF();
			else if (strName == "PopupBorderSize") prImGuiStyle.PopupBorderSize = ct::fvariant(strValue).GetF();
			else if (strName == "FrameBorderSize") prImGuiStyle.FrameBorderSize = ct::fvariant(strValue).GetF();
			else if (strName == "TabBorderSize") prImGuiStyle.TabBorderSize = ct::fvariant(strValue).GetF();
#ifdef USE_SHADOW
			else if (strName == "useshadow") puUseShadow = ct::ivariant(strValue).GetB();
			else if (strName == "shadowstrength") puShadowStrength = ct::fvariant(strValue).GetF();
			else if (strName == "useshadowtexture") puUseTextureForShadow = ct::ivariant(strValue).GetB();
#endif
		}
	}

	return true;
}

void ThemeHelper::ApplyStyle()
{
	ApplyFileTypeColors();
}

///////////////////////////////////////////////////////
//// PRIVVATE /////////////////////////////////////////
///////////////////////////////////////////////////////

std::string ThemeHelper::GetStyleColorName(ImGuiCol idx)
{
	switch (idx)
	{
	case ImGuiCol_Text: return "ImGuiCol_Text";
	case ImGuiCol_TextDisabled: return "ImGuiCol_TextDisabled";
	case ImGuiCol_WindowBg: return "ImGuiCol_WindowBg";
	case ImGuiCol_ChildBg: return "ImGuiCol_ChildBg";
	case ImGuiCol_PopupBg: return "ImGuiCol_PopupBg";
	case ImGuiCol_Border: return "ImGuiCol_Border";
	case ImGuiCol_BorderShadow: return "ImGuiCol_BorderShadow";
	case ImGuiCol_FrameBg: return "ImGuiCol_FrameBg";
	case ImGuiCol_FrameBgHovered: return "ImGuiCol_FrameBgHovered";
	case ImGuiCol_FrameBgActive: return "ImGuiCol_FrameBgActive";
	case ImGuiCol_TitleBg: return "ImGuiCol_TitleBg";
	case ImGuiCol_TitleBgActive: return "ImGuiCol_TitleBgActive";
	case ImGuiCol_TitleBgCollapsed: return "ImGuiCol_TitleBgCollapsed";
	case ImGuiCol_MenuBarBg: return "ImGuiCol_MenuBarBg";
	case ImGuiCol_ScrollbarBg: return "ImGuiCol_ScrollbarBg";
	case ImGuiCol_ScrollbarGrab: return "ImGuiCol_ScrollbarGrab";
	case ImGuiCol_ScrollbarGrabHovered: return "ImGuiCol_ScrollbarGrabHovered";
	case ImGuiCol_ScrollbarGrabActive: return "ImGuiCol_ScrollbarGrabActive";
	case ImGuiCol_CheckMark: return "ImGuiCol_CheckMark";
	case ImGuiCol_SliderGrab: return "ImGuiCol_SliderGrab";
	case ImGuiCol_SliderGrabActive: return "ImGuiCol_SliderGrabActive";
	case ImGuiCol_Button: return "ImGuiCol_Button";
	case ImGuiCol_ButtonHovered: return "ImGuiCol_ButtonHovered";
	case ImGuiCol_ButtonActive: return "ImGuiCol_ButtonActive";
	case ImGuiCol_Header: return "ImGuiCol_Header";
	case ImGuiCol_HeaderHovered: return "ImGuiCol_HeaderHovered";
	case ImGuiCol_HeaderActive: return "ImGuiCol_HeaderActive";
	case ImGuiCol_Separator: return "ImGuiCol_Separator";
	case ImGuiCol_SeparatorHovered: return "ImGuiCol_SeparatorHovered";
	case ImGuiCol_SeparatorActive: return "ImGuiCol_SeparatorActive";
	case ImGuiCol_ResizeGrip: return "ImGuiCol_ResizeGrip";
	case ImGuiCol_ResizeGripHovered: return "ImGuiCol_ResizeGripHovered";
	case ImGuiCol_ResizeGripActive: return "ImGuiCol_ResizeGripActive";
	case ImGuiCol_Tab: return "ImGuiCol_Tab";
	case ImGuiCol_TabHovered: return "ImGuiCol_TabHovered";
	case ImGuiCol_TabActive: return "ImGuiCol_TabActive";
	case ImGuiCol_TabUnfocused: return "ImGuiCol_TabUnfocused";
	case ImGuiCol_TabUnfocusedActive: return "ImGuiCol_TabUnfocusedActive";
	case ImGuiCol_DockingPreview: return "ImGuiCol_DockingPreview";
	case ImGuiCol_DockingEmptyBg: return "ImGuiCol_DockingEmptyBg";
	case ImGuiCol_PlotLines: return "ImGuiCol_PlotLines";
	case ImGuiCol_PlotLinesHovered: return "ImGuiCol_PlotLinesHovered";
	case ImGuiCol_PlotHistogram: return "ImGuiCol_PlotHistogram";
	case ImGuiCol_PlotHistogramHovered: return "ImGuiCol_PlotHistogramHovered";
	case ImGuiCol_TableHeaderBg: return "ImGuiCol_TableHeaderBg";
	case ImGuiCol_TableBorderStrong: return "ImGuiCol_TableBorderStrong";
	case ImGuiCol_TableBorderLight: return "ImGuiCol_TableBorderLight";
	case ImGuiCol_TableRowBg: return "ImGuiCol_TableRowBg";
	case ImGuiCol_TableRowBgAlt: return "ImGuiCol_TableRowBgAlt";
	case ImGuiCol_TextSelectedBg: return "ImGuiCol_TextSelectedBg";
	case ImGuiCol_DragDropTarget: return "ImGuiCol_DragDropTarget";
	case ImGuiCol_NavHighlight: return "ImGuiCol_NavHighlight";
	case ImGuiCol_NavWindowingHighlight: return "ImGuiCol_NavWindowingHighlight";
	case ImGuiCol_NavWindowingDimBg: return "ImGuiCol_NavWindowingDimBg";
	case ImGuiCol_ModalWindowDimBg: return "ImGuiCol_ModalWindowDimBg";
	default:;
	}
	return "ImGuiCol_Unknown";
}

int ThemeHelper::GetImGuiColFromName(const std::string& vName)
{
	if (vName == "ImGuiCol_Text") return ImGuiCol_Text;
	else if (vName == "ImGuiCol_TextDisabled") return ImGuiCol_TextDisabled;
	else if (vName == "ImGuiCol_WindowBg") return ImGuiCol_WindowBg;
	else if (vName == "ImGuiCol_ChildBg") return ImGuiCol_ChildBg;
	else if (vName == "ImGuiCol_PopupBg") return ImGuiCol_PopupBg;
	else if (vName == "ImGuiCol_Border") return ImGuiCol_Border;
	else if (vName == "ImGuiCol_BorderShadow") return ImGuiCol_BorderShadow;
	else if (vName == "ImGuiCol_FrameBg") return ImGuiCol_FrameBg;
	else if (vName == "ImGuiCol_FrameBgHovered") return ImGuiCol_FrameBgHovered;
	else if (vName == "ImGuiCol_FrameBgActive") return ImGuiCol_FrameBgActive;
	else if (vName == "ImGuiCol_TitleBg") return ImGuiCol_TitleBg;
	else if (vName == "ImGuiCol_TitleBgActive") return ImGuiCol_TitleBgActive;
	else if (vName == "ImGuiCol_TitleBgCollapsed") return ImGuiCol_TitleBgCollapsed;
	else if (vName == "ImGuiCol_MenuBarBg") return ImGuiCol_MenuBarBg;
	else if (vName == "ImGuiCol_ScrollbarBg") return ImGuiCol_ScrollbarBg;
	else if (vName == "ImGuiCol_ScrollbarGrab") return ImGuiCol_ScrollbarGrab;
	else if (vName == "ImGuiCol_ScrollbarGrabHovered") return ImGuiCol_ScrollbarGrabHovered;
	else if (vName == "ImGuiCol_ScrollbarGrabActive") return ImGuiCol_ScrollbarGrabActive;
	else if (vName == "ImGuiCol_CheckMark") return ImGuiCol_CheckMark;
	else if (vName == "ImGuiCol_SliderGrab") return ImGuiCol_SliderGrab;
	else if (vName == "ImGuiCol_SliderGrabActive") return ImGuiCol_SliderGrabActive;
	else if (vName == "ImGuiCol_Button") return ImGuiCol_Button;
	else if (vName == "ImGuiCol_ButtonHovered") return ImGuiCol_ButtonHovered;
	else if (vName == "ImGuiCol_ButtonActive") return ImGuiCol_ButtonActive;
	else if (vName == "ImGuiCol_Header") return ImGuiCol_Header;
	else if (vName == "ImGuiCol_HeaderHovered") return ImGuiCol_HeaderHovered;
	else if (vName == "ImGuiCol_HeaderActive") return ImGuiCol_HeaderActive;
	else if (vName == "ImGuiCol_Separator") return ImGuiCol_Separator;
	else if (vName == "ImGuiCol_SeparatorHovered") return ImGuiCol_SeparatorHovered;
	else if (vName == "ImGuiCol_SeparatorActive") return ImGuiCol_SeparatorActive;
	else if (vName == "ImGuiCol_ResizeGrip") return ImGuiCol_ResizeGrip;
	else if (vName == "ImGuiCol_ResizeGripHovered") return ImGuiCol_ResizeGripHovered;
	else if (vName == "ImGuiCol_ResizeGripActive") return ImGuiCol_ResizeGripActive;
	else if (vName == "ImGuiCol_Tab") return ImGuiCol_Tab;
	else if (vName == "ImGuiCol_TabHovered") return ImGuiCol_TabHovered;
	else if (vName == "ImGuiCol_TabActive") return ImGuiCol_TabActive;
	else if (vName == "ImGuiCol_TabUnfocused") return ImGuiCol_TabUnfocused;
	else if (vName == "ImGuiCol_TabUnfocusedActive") return ImGuiCol_TabUnfocusedActive;
	else if (vName == "ImGuiCol_DockingPreview") return ImGuiCol_DockingPreview;
	else if (vName == "ImGuiCol_DockingEmptyBg") return ImGuiCol_DockingEmptyBg;
	else if (vName == "ImGuiCol_PlotLines") return ImGuiCol_PlotLines;
	else if (vName == "ImGuiCol_PlotLinesHovered") return ImGuiCol_PlotLinesHovered;
	else if (vName == "ImGuiCol_PlotHistogram") return ImGuiCol_PlotHistogram;
	else if (vName == "ImGuiCol_PlotHistogramHovered") return ImGuiCol_PlotHistogramHovered;
	else if (vName == "ImGuiCol_TableHeaderBg") return ImGuiCol_TableHeaderBg;
	else if (vName == "ImGuiCol_TableBorderStrong") return ImGuiCol_TableBorderStrong;
	else if (vName == "ImGuiCol_TableBorderLight") return ImGuiCol_TableBorderLight;
	else if (vName == "ImGuiCol_TableRowBg") return ImGuiCol_TableRowBg;
	else if (vName == "ImGuiCol_TableRowBgAlt") return ImGuiCol_TableRowBgAlt;
	else if (vName == "ImGuiCol_TextSelectedBg") return ImGuiCol_TextSelectedBg;
	else if (vName == "ImGuiCol_DragDropTarget") return ImGuiCol_DragDropTarget;
	else if (vName == "ImGuiCol_NavHighlight") return ImGuiCol_NavHighlight;
	else if (vName == "ImGuiCol_NavWindowingHighlight") return ImGuiCol_NavWindowingHighlight;
	else if (vName == "ImGuiCol_NavWindowingDimBg") return ImGuiCol_NavWindowingDimBg;
	else if (vName == "ImGuiCol_ModalWindowDimBg") return ImGuiCol_ModalWindowDimBg;
	return -1;
}

inline void DrawItem(int vIdx, const ImGuiTextFilter& vFilter, const char* vName, ImVec4& vStyleColor, ImVec4& vRefColor, ImGuiColorEditFlags vFlags)
{
	if (!vFilter.PassFilter(vName))
		return;

	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushID(vIdx);
	ImGui::ColorEdit4("##color", (float*)&vStyleColor, ImGuiColorEditFlags_AlphaBar | vFlags);
	if (memcmp(&vStyleColor, &vRefColor, sizeof(ImVec4)) != 0)
	{
		// Tips: in a real user application, you may want to merge and use an icon font into the main font, so instead of "Save"/"Revert" you'd use icons.
		// Read the FAQ and docs/FONTS.txt about using icon fonts. It's really easy and super convenient!
		ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); 
		if (ImGui::ContrastedButton("Save")) 
			vRefColor = vStyleColor;
		ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); 
		if (ImGui::ContrastedButton("Revert")) 
			vStyleColor = vRefColor;
	}
	ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
	ImGui::TextUnformatted(vName);
	ImGui::PopID();
}

inline void ExportColors(ImGuiStyle& style_to_export, ImGuiStyle& ref_style, bool export_only_modified)
{
	ImGui::LogText("const auto colors = prImGuiStyle.Colors;" IM_NEWLINE);

	for (auto i = 0; i < ImGuiCol_COUNT; i++)
	{
		const auto& col = style_to_export.Colors[i];
		const auto name = ImGui::GetStyleColorName(i);
		if (!export_only_modified || memcmp(&col, &ref_style.Colors[i], sizeof(ImVec4)) != 0)
			ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
	}

	ImGui::LogText(IM_NEWLINE);

	ImGui::LogText("ImGui::CustomStyle::Instance()->GoodColor%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, 32 - (int)strlen("ImGui::CustomStyle::Instance()->GoodColor"),
		"", ImGui::CustomStyle::Instance()->GoodColor.x, ImGui::CustomStyle::Instance()->GoodColor.y, ImGui::CustomStyle::Instance()->GoodColor.z, ImGui::CustomStyle::Instance()->GoodColor.w);
	ImGui::LogText("ImGui::CustomStyle::Instance()->BadColor%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, 32 - (int)strlen("ImGui::CustomStyle::Instance()->BadColor"),
		"", ImGui::CustomStyle::Instance()->BadColor.x, ImGui::CustomStyle::Instance()->BadColor.y, ImGui::CustomStyle::Instance()->BadColor.z, ImGui::CustomStyle::Instance()->BadColor.w);

	ImGui::LogText("ImGui::CustomStyle::Instance()->GlyphButtonColor%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, 32 - (int)strlen("ImGui::CustomStyle::Instance()->GlyphButtonColor"),
		"", ImGui::CustomStyle::Instance()->GlyphButtonColor.x, ImGui::CustomStyle::Instance()->GlyphButtonColor.y, ImGui::CustomStyle::Instance()->GlyphButtonColor.z, ImGui::CustomStyle::Instance()->GlyphButtonColor.w);
	ImGui::LogText("ImGui::CustomStyle::Instance()->GlyphButtonColorActive%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, 32 - (int)strlen("ImGui::CustomStyle::Instance()->GlyphButtonColorActive"),
		"", ImGui::CustomStyle::Instance()->GlyphButtonColorActive.x, ImGui::CustomStyle::Instance()->GlyphButtonColorActive.y, ImGui::CustomStyle::Instance()->GlyphButtonColorActive.z, ImGui::CustomStyle::Instance()->GlyphButtonColorActive.w);
}

inline void ExportSize_Float(const char* name, float& size_to_export, float& ref_size, bool export_only_modified)
{
	if (!export_only_modified || memcmp(&size_to_export, &ref_size, sizeof(float)) != 0)
		ImGui::LogText("prImGuiStyle.%s%*s= %.2ff;" IM_NEWLINE, name, 25 - (int)strlen(name), "", size_to_export);
}

inline void ExportSize_ImVec2(const char* name, ImVec2& size_to_export, ImVec2& ref_size, bool export_only_modified)
{
	if (!export_only_modified || memcmp(&size_to_export, &ref_size, sizeof(ImVec2)) != 0)
		ImGui::LogText("prImGuiStyle.%s%*s= ImVec2(%.2ff, %.2ff);" IM_NEWLINE, name, 25 - (int)strlen(name), "", size_to_export.x, size_to_export.y);
}

inline void ExportSizes(ImGuiStyle& style_to_export, ImGuiStyle& ref_style, bool export_only_modified)
{
	//ImGui::LogText("ImGuiStyle& style = prImGuiStyle;" IM_NEWLINE);

	{
		ImGui::LogText(IM_NEWLINE "// Main" IM_NEWLINE);

		ExportSize_ImVec2("WindowPadding", style_to_export.WindowPadding, ref_style.WindowPadding, export_only_modified);
		ExportSize_ImVec2("FramePadding", style_to_export.FramePadding, ref_style.FramePadding, export_only_modified);
		ExportSize_ImVec2("ItemSpacing", style_to_export.ItemSpacing, ref_style.ItemSpacing, export_only_modified);
		ExportSize_ImVec2("ItemInnerSpacing", style_to_export.ItemInnerSpacing, ref_style.ItemInnerSpacing, export_only_modified);
		ExportSize_ImVec2("TouchExtraPadding", style_to_export.TouchExtraPadding, ref_style.TouchExtraPadding, export_only_modified);
		ExportSize_Float("IndentSpacing", style_to_export.IndentSpacing, ref_style.IndentSpacing, export_only_modified);
		ExportSize_Float("ScrollbarSize", style_to_export.ScrollbarSize, ref_style.ScrollbarSize, export_only_modified);
		ExportSize_Float("GrabMinSize", style_to_export.GrabMinSize, ref_style.GrabMinSize, export_only_modified);
	}

	{
		ImGui::LogText(IM_NEWLINE "// Borders" IM_NEWLINE);

		ExportSize_Float("WindowBorderSize", style_to_export.WindowBorderSize, ref_style.WindowBorderSize, export_only_modified);
		ExportSize_Float("ChildBorderSize", style_to_export.ChildBorderSize, ref_style.ChildBorderSize, export_only_modified);
		ExportSize_Float("PopupBorderSize", style_to_export.PopupBorderSize, ref_style.PopupBorderSize, export_only_modified);
		ExportSize_Float("FrameBorderSize", style_to_export.FrameBorderSize, ref_style.FrameBorderSize, export_only_modified);
		ExportSize_Float("TabBorderSize", style_to_export.TabBorderSize, ref_style.TabBorderSize, export_only_modified);
	}
	
	{
		ImGui::LogText(IM_NEWLINE "// Rounding" IM_NEWLINE);

		ExportSize_Float("WindowRounding", style_to_export.WindowRounding, ref_style.WindowRounding, export_only_modified);
		ExportSize_Float("ChildRounding", style_to_export.ChildRounding, ref_style.ChildRounding, export_only_modified);
		ExportSize_Float("FrameRounding", style_to_export.FrameRounding, ref_style.FrameRounding, export_only_modified);
		ExportSize_Float("PopupRounding", style_to_export.PopupRounding, ref_style.PopupRounding, export_only_modified);
		ExportSize_Float("ScrollbarRounding", style_to_export.ScrollbarRounding, ref_style.ScrollbarRounding, export_only_modified);
		ExportSize_Float("GrabRounding", style_to_export.GrabRounding, ref_style.GrabRounding, export_only_modified);
		ExportSize_Float("TabRounding", style_to_export.TabRounding, ref_style.TabRounding, export_only_modified);
	}

	{
		ImGui::LogText(IM_NEWLINE "// Alignment" IM_NEWLINE);

		ExportSize_ImVec2("WindowTitleAlign", style_to_export.WindowTitleAlign, ref_style.WindowTitleAlign, export_only_modified);

		// for this one we could just save ImGuiDir number, but its more redable to have ImGuiDir_ name
		if (!export_only_modified || memcmp(&style_to_export.WindowMenuButtonPosition, &ref_style.WindowMenuButtonPosition, sizeof(ImGuiDir)) != 0)
		{
			const char* dirName = 0;
			switch (style_to_export.WindowMenuButtonPosition)
			{
			case ImGuiDir_None: dirName = "ImGuiDir_None"; break;
			case ImGuiDir_Left: dirName = "ImGuiDir_Left"; break;
			case ImGuiDir_Right: dirName = "ImGuiDir_Right"; break;
			};

			ImGui::LogText("prImGuiStyle.%s%*s= %s;" IM_NEWLINE, "WindowMenuButtonPosition", 25 - (int)strlen("WindowMenuButtonPosition"), "", dirName);
		}

		// for this one we could just save ImGuiDir number, but its more redable to have ImGuiDir_ name
		if (!export_only_modified || memcmp(&style_to_export.ColorButtonPosition, &ref_style.ColorButtonPosition, sizeof(ImGuiDir)) != 0)
		{
			const char* dirName = 0;
			switch (style_to_export.ColorButtonPosition)
			{
			case ImGuiDir_Left: dirName = "ImGuiDir_Left"; break;
			case ImGuiDir_Right: dirName = "ImGuiDir_Right"; break;
			};

			ImGui::LogText("prImGuiStyle.%s%*s= %s;" IM_NEWLINE, "ColorButtonPosition", 25 - (int)strlen("ColorButtonPosition"), "", dirName);
		}

		ExportSize_ImVec2("ButtonTextAlign", style_to_export.ButtonTextAlign, ref_style.ButtonTextAlign, export_only_modified);
		ExportSize_ImVec2("SelectableTextAlign", style_to_export.SelectableTextAlign, ref_style.SelectableTextAlign, export_only_modified);
	}
	
	{
		ImGui::LogText(IM_NEWLINE "// Safe Area Padding" IM_NEWLINE);

		ExportSize_ImVec2("DisplaySafeAreaPadding", style_to_export.DisplaySafeAreaPadding, ref_style.DisplaySafeAreaPadding, export_only_modified);
	}
}

void ThemeHelper::ShowCustomImGuiStyleEditor(bool* vOpen, ImGuiStyle* ref)
{
	if (ImGui::Begin("Styles Editor", vOpen))
	{
		// You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
		auto& style = prImGuiStyle;
		static ImGuiStyle ref_saved_style;
		static ImVec4 ref_Good_Color;
		static ImVec4 ref_Bad_Color;
		static ImVec4 ref_Glyph_button;
		static ImVec4 ref_Glyph_button_Hovered;
		static ImVec4 ref_Glyph_button_Active;

		// Default to using internal storage as reference
		static auto init = true;
		if (init && ref == nullptr)
			ref_saved_style = style;
		init = false;
		if (ref == nullptr)
			ref = &ref_saved_style;

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

		auto window_border = (style.WindowBorderSize > 0.0f);
		if (ImGui::Checkbox("WindowBorder", &window_border))
			style.WindowBorderSize = window_border ? 1.0f : 0.0f;

		ImGui::SameLine();

		auto frame_border = (style.FrameBorderSize > 0.0f);
		if (ImGui::Checkbox("FrameBorder", &frame_border))
			style.FrameBorderSize = frame_border ? 1.0f : 0.0f;

		ImGui::SameLine();

		auto popup_border = (style.PopupBorderSize > 0.0f);
		if (ImGui::Checkbox("PopupBorder", &popup_border))
			style.PopupBorderSize = popup_border ? 1.0f : 0.0f;

#ifdef USE_SHADOW
		// Custom Shadow
		ImGui::Checkbox("Use Shadow", &puUseShadow);
		if (puUseShadow)
		{
			ImGui::SliderFloatDefaultCompact(300.0f, "Inner Shadow", &puShadowStrength, 2.0f, 0.0f, 0.5f);
			ImGui::Checkbox("Use Texture for Shadow", &puUseTextureForShadow);
		}
#endif

		// Save/Revert button
		if (ImGui::ContrastedButton("Save Ref"))
		{
			*ref = ref_saved_style = style;
			ref_Good_Color = ImGui::CustomStyle::Instance()->GoodColor;
			ref_Bad_Color = ImGui::CustomStyle::Instance()->BadColor;
			ref_Glyph_button = ImGui::CustomStyle::Instance()->GlyphButtonColor;
			ref_Glyph_button_Active = ImGui::CustomStyle::Instance()->GlyphButtonColorActive;
		}
		ImGui::SameLine();
		if (ImGui::ContrastedButton("Revert Ref"))
		{
			style = *ref;
			ImGui::CustomStyle::Instance()->GoodColor = ref_Good_Color;
			ImGui::CustomStyle::Instance()->BadColor = ref_Bad_Color;
			ImGui::CustomStyle::Instance()->GlyphButtonColor = ref_Glyph_button;
			ImGui::CustomStyle::Instance()->GlyphButtonColorActive = ref_Glyph_button_Active;
		}
		ImGui::SameLine();
		ImGui::HelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. Use \"Export\" below to save them somewhere.");

		ImGui::Separator();
		
		static auto output_dest = 0;
		static auto output_only_modified = true;
		if (ImGui::ContrastedButton("Export Sizes and Colors"))
		{
			if (output_dest == 0)
				ImGui::LogToClipboard();
			else
				ImGui::LogToTTY();

			ExportColors(style, *ref, output_only_modified);

			ImGui::LogText(IM_NEWLINE);

			ExportSizes(style, *ref, output_only_modified);

			ImGui::LogFinish();
		}
		ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type_size_and_colors", &output_dest, "To Clipboard\0To TTY\0");
		ImGui::SameLine(); ImGui::Checkbox("Only Modified##size_and_colors", &output_only_modified);

		ImGui::Separator();

		if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Sizes"))
			{
				static auto output_dest_sizes = 0;
				static auto output_only_modified_sizes = true;
				if (ImGui::ContrastedButton("Export"))
				{
					if (output_dest_sizes == 0)
						ImGui::LogToClipboard();
					else
						ImGui::LogToTTY();

					ExportSizes(style, *ref, output_only_modified_sizes);

					ImGui::LogFinish();
				}
				ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest_sizes, "To Clipboard\0To TTY\0");
				ImGui::SameLine(); ImGui::Checkbox("Only Modified", &output_only_modified_sizes);

				ImGui::Separator();

				ImGui::BeginChild("##sizes", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
				ImGui::PushItemWidth(-160);

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
				auto window_menu_button_position = style.WindowMenuButtonPosition + 1;
				if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
					style.WindowMenuButtonPosition = window_menu_button_position - 1;
				ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
				ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a button is larger than its text content.");
				ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a selectable is larger than its text content.");
				
				ImGui::Text("Safe Area Padding"); ImGui::SameLine(); ImGui::HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
				ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
				
				ImGui::PopItemWidth();
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Colors"))
			{
				static auto output_dest_colors = 0;
				static auto output_only_modified_colors = true;
				if (ImGui::ContrastedButton("Export"))
				{
					if (output_dest_colors == 0)
						ImGui::LogToClipboard();
					else
						ImGui::LogToTTY();

					ExportColors(style, *ref, output_only_modified_colors);

					ImGui::LogFinish();
				}
				ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest_colors, "To Clipboard\0To TTY\0");
				ImGui::SameLine(); ImGui::Checkbox("Only Modified", &output_only_modified_colors);

				ImGui::Separator();

				static ImGuiTextFilter filter;
				filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

				static auto alpha_flags = 0;
				if (ImGui::RadioButton("Opaque", alpha_flags == 0)) { alpha_flags = 0; } ImGui::SameLine();
				if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
				if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
				ImGui::HelpMarker("In the color list:\nLeft-click on colored square to open color picker,\nRight-click to open edit options menu.");

				ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
				ImGui::PushItemWidth(-200);

				int id = 0;
				DrawItem(id++, filter, "Good Color", ImGui::CustomStyle::Instance()->GoodColor, ref_Good_Color, alpha_flags);
				DrawItem(id++, filter, "Bad Color", ImGui::CustomStyle::Instance()->BadColor, ref_Bad_Color, alpha_flags);
				DrawItem(id++, filter, "GlyphButton", ImGui::CustomStyle::Instance()->GlyphButtonColor, ref_Glyph_button, alpha_flags);
				DrawItem(id++, filter, "GlyphButtonActive", ImGui::CustomStyle::Instance()->GlyphButtonColorActive, ref_Glyph_button_Active, alpha_flags);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					DrawItem(i + id, filter, ImGui::GetStyleColorName(i), style.Colors[i], ref->Colors[i], alpha_flags);
				}
				ImGui::PopItemWidth();
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();

	memcpy(&ImGui::GetStyle(), &prImGuiStyle, sizeof(ImGuiStyle));
}
