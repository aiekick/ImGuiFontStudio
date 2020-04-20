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
#include "ImGuiThemeHelper.h"

#include "ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

ImGuiThemeHelper::ImGuiThemeHelper()
{
	m_FileTypeColors[".ttf"] = ImVec4(0.1f, 0.1f, 0.5f, 1.0f);
	m_FileTypeColors[".otf"] = ImVec4(0.1f, 0.1f, 0.5f, 1.0f);
	m_FileTypeColors[".cpp"] = ImVec4(0.5f, 0.1f, 0.7f, 1.0f);
	m_FileTypeColors[".h"] = ImVec4(0.5f, 0.1f, 0.5f, 1.0f);
	m_FileTypeColors[".ifs"] = ImVec4(0.1f, 0.5f, 0.1f, 1.0f);
}

ImGuiThemeHelper::~ImGuiThemeHelper()
{
	
}

void ImGuiThemeHelper::DrawMenu()
{
	if (ImGui::MenuItem("Default")) ApplyStyleColorsDefault();
	if (ImGui::MenuItem("Classic")) ApplyStyleColorsClassic();
	if (ImGui::MenuItem("Dark"))	ApplyStyleColorsDark();
	if (ImGui::MenuItem("Darcula"))	ApplyStyleColorsDarcula();
	if (ImGui::MenuItem("Light"))	ApplyStyleColorsLight();
	
	ImGui::Separator();

	ImGui::Text("File Type Colors :");
	ImGui::Indent();
	ImGui::Text("Fonts :");
	if (ImGui::ColorEdit4(".ttf", &m_FileTypeColors[".ttf"].x))
	{
		ApplyFileTypeColors();
	}
	if (ImGui::ColorEdit4(".otf", &m_FileTypeColors[".otf"].x))
	{
		ApplyFileTypeColors();
	}
	ImGui::Text("Code :");
	if (ImGui::ColorEdit4(".cpp", &m_FileTypeColors[".cpp"].x))
	{
		ApplyFileTypeColors();
	}
	if (ImGui::ColorEdit4(".h", &m_FileTypeColors[".h"].x))
	{
		ApplyFileTypeColors();
	}
	ImGui::Text("Project :");
	if (ImGui::ColorEdit4(".ifs", &m_FileTypeColors[".ifs"].x))
	{
		ApplyFileTypeColors();
	}
	ImGui::Unindent();
}

/* default theme */
void ImGuiThemeHelper::ApplyStyleColorsDefault(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.44f, 0.44f, 0.6f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.7f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.8f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.6f);
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
	colors[ImGuiCol_DockingPreview] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.13f, 0.13f, 0.13f, 0.80f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

	// dont care of these settings
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	
	style->WindowPadding = ImVec2(4, 4);
	style->FramePadding = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(4, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 10;
	style->ScrollbarSize = 20;
	style->GrabMinSize = 4;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->PopupRounding = 0;
	style->ScrollbarRounding = 0;
	style->GrabRounding = 0;
	style->TabRounding = 0;

	style->WindowBorderSize = 0;
	style->ChildBorderSize = 0;
	style->PopupBorderSize = 0;
	style->FrameBorderSize = 1;
	style->TabBorderSize = 0;

	m_FileTypeColors[".ttf"] = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	m_FileTypeColors[".otf"] = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	m_FileTypeColors[".cpp"] = ImVec4(0.5f, 0.9f, 0.1f, 1.0f); // yellow high
	m_FileTypeColors[".h"] = ImVec4(0.25f, 0.9f, 0.1f, 1.0f); // yellow high
	m_FileTypeColors[".ifs"] = ImVec4(0.9f, 0.1f, 0.9f, 1.0f); // purple high

	// dark theme so high color
	goodColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
	badColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	ApplyFileTypeColors();
}

void ImGuiThemeHelper::ApplyStyleColorsClassic(ImGuiStyle* dst)
{
	ImGui::StyleColorsClassic(dst);

	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	
	style->WindowPadding = ImVec2(4, 4);
	style->FramePadding = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(4, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 10;
	style->ScrollbarSize = 20;
	style->GrabMinSize = 4;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->PopupRounding = 0;
	style->ScrollbarRounding = 0;
	style->GrabRounding = 0;
	style->TabRounding = 0;

	style->WindowBorderSize = 0;
	style->ChildBorderSize = 0;
	style->PopupBorderSize = 0;
	style->FrameBorderSize = 1;
	style->TabBorderSize = 0;

	m_FileTypeColors[".ttf"] = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	m_FileTypeColors[".otf"] = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	m_FileTypeColors[".cpp"] = ImVec4(0.5f, 0.9f, 0.1f, 1.0f); // yellow high
	m_FileTypeColors[".h"] = ImVec4(0.25f, 0.9f, 0.1f, 1.0f); // yellow high
	m_FileTypeColors[".ifs"] = ImVec4(0.9f, 0.1f, 0.9f, 1.0f); // purple high

	// dark theme so high color
	goodColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
	badColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	ApplyFileTypeColors();
}

void ImGuiThemeHelper::ApplyStyleColorsDark(ImGuiStyle* dst)
{
	ImGui::StyleColorsDark(dst);

	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	
	style->WindowPadding = ImVec2(4, 4);
	style->FramePadding = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(4, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 10;
	style->ScrollbarSize = 20;
	style->GrabMinSize = 4;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->PopupRounding = 0;
	style->ScrollbarRounding = 0;
	style->GrabRounding = 0;
	style->TabRounding = 0;

	style->WindowBorderSize = 0;
	style->ChildBorderSize = 0;
	style->PopupBorderSize = 0;
	style->FrameBorderSize = 1;
	style->TabBorderSize = 0;

	m_FileTypeColors[".ttf"] = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	m_FileTypeColors[".otf"] = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	m_FileTypeColors[".cpp"] = ImVec4(0.5f, 0.9f, 0.1f, 1.0f); // yellow high
	m_FileTypeColors[".h"] = ImVec4(0.25f, 0.9f, 0.1f, 1.0f); // yellow high
	m_FileTypeColors[".ifs"] = ImVec4(0.9f, 0.1f, 0.9f, 1.0f); // purple high

	// dark theme so high color
	goodColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
	badColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	ApplyFileTypeColors();
}

void ImGuiThemeHelper::ApplyStyleColorsLight(ImGuiStyle* dst)
{
	ImGui::StyleColorsLight(dst);
	
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	
	style->WindowPadding = ImVec2(4, 4);
	style->FramePadding = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(4, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 10;
	style->ScrollbarSize = 20;
	style->GrabMinSize = 4;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->PopupRounding = 0;
	style->ScrollbarRounding = 0;
	style->GrabRounding = 0;
	style->TabRounding = 0;

	style->WindowBorderSize = 0;
	style->ChildBorderSize = 0;
	style->PopupBorderSize = 0;
	style->FrameBorderSize = 1;
	style->TabBorderSize = 0;

	m_FileTypeColors[".ttf"] = ImVec4(0.1f, 0.5f, 0.1f, 1.0f); // green low
	m_FileTypeColors[".otf"] = ImVec4(0.1f, 0.5f, 0.1f, 1.0f); // green low
	m_FileTypeColors[".cpp"] = ImVec4(0.5f, 0.5f, 0.1f, 1.0f); // yellow low
	m_FileTypeColors[".h"] = ImVec4(0.25f, 0.5f, 0.1f, 1.0f); // yellow low
	m_FileTypeColors[".ifs"] = ImVec4(0.5f, 0.1f, 0.5f, 1.0f); // purple low

	// light theme so low color
	goodColor = ImVec4(0.2f, 0.5f, 0.2f, 1.0f);
	badColor = ImVec4(0.5f, 0.2f, 0.2f, 1.0f);

	ApplyFileTypeColors();
}

void ImGuiThemeHelper::ApplyStyleColorsDarcula(ImGuiStyle* dst)
{
	//https://github.com/ocornut/imgui/issues/707
	// bi ice1000

	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	style->Colors[ImGuiCol_Text] = ImVec4(0.73333335f, 0.73333335f, 0.73333335f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.34509805f, 0.34509805f, 0.34509805f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.23529413f, 0.24705884f, 0.25490198f, 0.94f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.23529413f, 0.24705884f, 0.25490198f, 0.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.23529413f, 0.24705884f, 0.25490198f, 0.94f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 0.50f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 0.54f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.453125f, 0.67578125f, 0.99609375f, 0.67f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.47058827f, 0.47058827f, 0.47058827f, 0.67f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.27058825f, 0.28627452f, 0.2901961f, 0.80f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.27058825f, 0.28627452f, 0.2901961f, 0.60f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21960786f, 0.30980393f, 0.41960788f, 0.51f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.21960786f, 0.30980393f, 0.41960788f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.13725491f, 0.19215688f, 0.2627451f, 0.91f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.33333334f, 0.3529412f, 0.36078432f, 0.49f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.21960786f, 0.30980393f, 0.41960788f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.13725491f, 0.19215688f, 0.2627451f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.33333334f, 0.3529412f, 0.36078432f, 0.53f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.453125f, 0.67578125f, 0.99609375f, 0.67f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.47058827f, 0.47058827f, 0.47058827f, 0.67f);
	style->Colors[ImGuiCol_Separator] = ImVec4(0.31640625f, 0.31640625f, 0.31640625f, 1.00f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.31640625f, 0.31640625f, 0.31640625f, 1.00f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.31640625f, 0.31640625f, 0.31640625f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.18431373f, 0.39607847f, 0.79215693f, 0.90f);
	style->Colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
	style->Colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
	style->Colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	style->Colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
	style->Colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
	style->Colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style->Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

	style->WindowPadding = ImVec2(4, 4);
	style->FramePadding = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(4, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 10;
	style->ScrollbarSize = 20;
	style->GrabMinSize = 4;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->PopupRounding = 0;
	style->ScrollbarRounding = 0;
	style->GrabRounding = 0;
	style->TabRounding = 0;

	style->WindowBorderSize = 0;
	style->ChildBorderSize = 0;
	style->PopupBorderSize = 0;
	style->FrameBorderSize = 1;
	style->TabBorderSize = 0;

	m_FileTypeColors[".ttf"] = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	m_FileTypeColors[".otf"] = ImVec4(0.1f, 0.9f, 0.5f, 1.0f); // green high
	m_FileTypeColors[".cpp"] = ImVec4(0.5f, 0.9f, 0.1f, 1.0f); // yellow high
	m_FileTypeColors[".h"] = ImVec4(0.25f, 0.9f, 0.1f, 1.0f); // yellow high
	m_FileTypeColors[".ifs"] = ImVec4(0.9f, 0.1f, 0.9f, 1.0f); // purple high

	// dark theme so high color
	goodColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
	badColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

	ApplyFileTypeColors();
}

void ImGuiThemeHelper::ApplyFileTypeColors()
{
	for (auto &it : m_FileTypeColors)
	{
		igfd::ImGuiFileDialog::Instance()->SetFilterColor(it.first, it.second);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string ImGuiThemeHelper::getXml(const std::string& vOffset)
{
	std::string str;

	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	
	str += vOffset + "<ImGui_Styles>\n";

	for (int i = 0; i < ImGuiCol_COUNT; i++)
	{
		std::string name = GetStyleColorName(i);
		str += vOffset + "\t<" + name + " value=\"" + ct::fvec4(colors[i]).string() + "\"/>\n";
	}

	str += vOffset + "\t<WindowPadding value=\"" + ct::fvec2(style->WindowPadding).string() + "\"/>\n";
	str += vOffset + "\t<FramePadding value=\"" + ct::fvec2(style->FramePadding).string() + "\"/>\n";
	str += vOffset + "\t<ItemSpacing value=\"" + ct::fvec2(style->ItemSpacing).string() + "\"/>\n";
	str += vOffset + "\t<ItemInnerSpacing value=\"" + ct::fvec2(style->ItemInnerSpacing).string() + "\"/>\n";
	str += vOffset + "\t<IndentSpacing value=\"" + ct::toStr(style->IndentSpacing) + "\"/>\n";
	str += vOffset + "\t<ScrollbarSize value=\"" + ct::toStr(style->ScrollbarSize) + "\"/>\n";
	str += vOffset + "\t<GrabMinSize value=\"" + ct::toStr(style->GrabMinSize) + "\"/>\n";
	str += vOffset + "\t<WindowRounding value=\"" + ct::toStr(style->WindowRounding) + "\"/>\n";
	str += vOffset + "\t<ChildRounding value=\"" + ct::toStr(style->ChildRounding) + "\"/>\n";
	str += vOffset + "\t<FrameRounding value=\"" + ct::toStr(style->FrameRounding) + "\"/>\n";
	str += vOffset + "\t<PopupRounding value=\"" + ct::toStr(style->PopupRounding) + "\"/>\n";
	str += vOffset + "\t<ScrollbarRounding value=\"" + ct::toStr(style->ScrollbarRounding) + "\"/>\n";
	str += vOffset + "\t<GrabRounding value=\"" + ct::toStr(style->GrabRounding) + "\"/>\n";
	str += vOffset + "\t<TabRounding value=\"" + ct::toStr(style->TabRounding) + "\"/>\n";
	str += vOffset + "\t<WindowBorderSize value=\"" + ct::toStr(style->WindowBorderSize) + "\"/>\n";
	str += vOffset + "\t<ChildBorderSize value=\"" + ct::toStr(style->ChildBorderSize) + "\"/>\n";
	str += vOffset + "\t<PopupBorderSize value=\"" + ct::toStr(style->PopupBorderSize) + "\"/>\n";
	str += vOffset + "\t<FrameBorderSize value=\"" + ct::toStr(style->FrameBorderSize) + "\"/>\n";
	str += vOffset + "\t<TabBorderSize value=\"" + ct::toStr(style->TabBorderSize) + "\"/>\n";

	str += vOffset + "</ImGui_Styles>\n";

	str += vOffset + "<FileTypes>\n";
	for (auto &it : m_FileTypeColors)
	{
		str += vOffset + "\t<filetype value=\"" + it.first + "\" color=\"" +
			ct::fvec4(it.second.x, it.second.y, it.second.z, it.second.w).string() + "\"/>\n";
	}
	str += vOffset + "</FileTypes>\n";

	return str;
}

void ImGuiThemeHelper::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
{
	// The value of this child identifies the name of this element
	std::string strName = "";
	std::string strValue = "";
	std::string strParentName = "";

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != 0)
		strParentName = vParent->Value();

	if (strParentName == "FileTypes")
	{
		std::string fileType;
		std::string color;
		
		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != 0; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "value") fileType = attValue;
			if (attName == "color") color = attValue;
		}

		m_FileTypeColors[fileType] = ct::toImVec4(ct::fvariant(color).getV4());
		igfd::ImGuiFileDialog::Instance()->SetFilterColor(fileType, m_FileTypeColors[fileType]);
	}

	if (strParentName == "ImGui_Styles")
	{
		auto att = vElem->FirstAttribute();
		if (att && std::string(att->Name()) == "value")
		{
			strValue = att->Value();

			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4* colors = style->Colors;

			if (strName.find("ImGuiCol") != std::string::npos)
			{
				int id = GetImGuiColFromName(strName);
				if (id >= 0)
				{
					colors[id] = ct::toImVec4(ct::fvariant(strValue).getV4());
					return;
				}
			}

			if (strName == "WindowPadding") style->WindowPadding = ct::toImVec2(ct::fvariant(strValue).getV2());
			else if (strName == "FramePadding") style->FramePadding = ct::toImVec2(ct::fvariant(strValue).getV2());
			else if (strName == "ItemSpacing") style->ItemSpacing = ct::toImVec2(ct::fvariant(strValue).getV2());
			else if (strName == "ItemInnerSpacing") style->ItemInnerSpacing = ct::toImVec2(ct::fvariant(strValue).getV2());
			else if (strName == "IndentSpacing") style->IndentSpacing = ct::fvariant(strValue).getF();
			else if (strName == "ScrollbarSize") style->ScrollbarSize = ct::fvariant(strValue).getF();
			else if (strName == "GrabMinSize") style->GrabMinSize = ct::fvariant(strValue).getF();
			else if (strName == "WindowRounding") style->WindowRounding = ct::fvariant(strValue).getF();
			else if (strName == "ChildRounding") style->ChildRounding = ct::fvariant(strValue).getF();
			else if (strName == "FrameRounding") style->FrameRounding = ct::fvariant(strValue).getF();
			else if (strName == "PopupRounding") style->PopupRounding = ct::fvariant(strValue).getF();
			else if (strName == "ScrollbarRounding") style->ScrollbarRounding = ct::fvariant(strValue).getF();
			else if (strName == "GrabRounding") style->GrabRounding = ct::fvariant(strValue).getF();
			else if (strName == "TabRounding") style->TabRounding = ct::fvariant(strValue).getF();
			else if (strName == "WindowBorderSize") style->WindowBorderSize = ct::fvariant(strValue).getF();
			else if (strName == "ChildBorderSize") style->ChildBorderSize = ct::fvariant(strValue).getF();
			else if (strName == "PopupBorderSize") style->PopupBorderSize = ct::fvariant(strValue).getF();
			else if (strName == "FrameBorderSize") style->FrameBorderSize = ct::fvariant(strValue).getF();
			else if (strName == "TabBorderSize") style->TabBorderSize = ct::fvariant(strValue).getF();
		}
	}
}


///////////////////////////////////////////////////////
//// PRIVVATE /////////////////////////////////////////
///////////////////////////////////////////////////////

std::string ImGuiThemeHelper::GetStyleColorName(ImGuiCol idx)
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
	case ImGuiCol_TextSelectedBg: return "ImGuiCol_TextSelectedBg";
	case ImGuiCol_DragDropTarget: return "ImGuiCol_DragDropTarget";
	case ImGuiCol_NavHighlight: return "ImGuiCol_NavHighlight";
	case ImGuiCol_NavWindowingHighlight: return "ImGuiCol_NavWindowingHighlight";
	case ImGuiCol_NavWindowingDimBg: return "ImGuiCol_NavWindowingDimBg";
	case ImGuiCol_ModalWindowDimBg: return "ImGuiCol_ModalWindowDimBg";
	}
	return "";
}

int ImGuiThemeHelper::GetImGuiColFromName(const std::string& vName)
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
	else if (vName == "ImGuiCol_TextSelectedBg") return ImGuiCol_TextSelectedBg;
	else if (vName == "ImGuiCol_DragDropTarget") return ImGuiCol_DragDropTarget;
	else if (vName == "ImGuiCol_NavHighlight") return ImGuiCol_NavHighlight;
	else if (vName == "ImGuiCol_NavWindowingHighlight") return ImGuiCol_NavWindowingHighlight;
	else if (vName == "ImGuiCol_NavWindowingDimBg") return ImGuiCol_NavWindowingDimBg;
	else if (vName == "ImGuiCol_ModalWindowDimBg") return ImGuiCol_ModalWindowDimBg;
	return -1;
}
