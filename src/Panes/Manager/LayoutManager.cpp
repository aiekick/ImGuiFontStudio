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

#include "LayoutManager.h"

#include <ctools/FileHelper.h>
#include <MainFrame.h>
#include <Res/CustomFont.h>
#include <Gui/ImGuiWidgets.h>
#include <Project/ProjectFile.h>

#ifdef _DEBUG
#include <Panes/DebugPane.h>
#endif
#include <Panes/FinalFontPane.h>
#include <Panes/FontStructurePane.h>
#include <Panes/GeneratorPane.h>
#include <Panes/GlyphPane.h>
#include <Panes/FontPreviewPane.h>
#include <Panes/SourceFontPane.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/Logger.h>

PaneFlags LayoutManager::m_Pane_Shown = PaneFlags::PANE_OPENED_DEFAULT;
PaneFlags LayoutManager::m_Pane_Focused = PaneFlags::PANE_FOCUS_DEFAULT;

LayoutManager::LayoutManager() = default;
LayoutManager::~LayoutManager() = default;

void LayoutManager::Init()
{
	if (!FileHelper::Instance()->IsFileExist("imgui.ini"))
	{
		m_FirstLayout = true; // need default layout
		LogStr("We will apply default layout :)");
	}
#ifdef _DEBUG
	DebugPane::Instance()->Init();
#endif
	FinalFontPane::Instance()->Init();
	FontStructurePane::Instance()->Init();
	GeneratorPane::Instance()->Init();
	GlyphPane::Instance()->Init();
	FontPreviewPane::Instance()->Init();
	SourceFontPane::Instance()->Init();
}

void LayoutManager::Unit()
{
#ifdef _DEBUG
	DebugPane::Instance()->Unit();
#endif
	FinalFontPane::Instance()->Unit();
	FontStructurePane::Instance()->Unit();
	GeneratorPane::Instance()->Unit();
	GlyphPane::Instance()->Unit();
	FontPreviewPane::Instance()->Unit();
	SourceFontPane::Instance()->Unit();
}

void LayoutManager::InitAfterFirstDisplay(ImVec2 vSize)
{
	if (m_FirstLayout)
	{
		ApplyInitialDockingLayout(vSize);
		m_FirstLayout = false;
	}

	if (m_FirstStart)
	{
		// focus after start of panes
		SetFocusedPanes(m_Pane_Focused);
		m_FirstStart = false;
	}
}

void LayoutManager::StartDockPane(ImGuiDockNodeFlags vFlags)
{
	m_DockSpaceID = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(m_DockSpaceID, ImVec2(0, 0), vFlags);
}

void LayoutManager::ApplyInitialDockingLayout(ImVec2 vSize)
{
	ImGui::DockBuilderRemoveNode(m_DockSpaceID); // Clear out existing layout
	ImGui::DockBuilderAddNode(m_DockSpaceID, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImGui::DockBuilderSetNodeSize(m_DockSpaceID, vSize);

	float leftWidth = 250.0f;
	float rightWidth = 310.0f;
	float bottomWidth = 200.0f;

	ImGuiID dockMainID = m_DockSpaceID; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
	ImGuiID dockLeftID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, leftWidth / vSize.x, nullptr, &dockMainID);
	ImGuiID dockRightID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, rightWidth / vSize.x, nullptr, &dockMainID);
	ImGuiID dockBottomID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, bottomWidth / vSize.y, nullptr, &dockMainID);
	//ImGuiID dockSelectionID = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Up, 0.50f, NULL, &dockRight);
	//ImGuiID dockGeneratorID = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.50f, NULL, &dockRight);

	ImGui::DockBuilderDockWindow(PARAM_PANE, dockLeftID);
	ImGui::DockBuilderDockWindow(SOURCE_PANE, dockMainID);
	ImGui::DockBuilderDockWindow(FINAL_PANE, dockMainID);
	ImGui::DockBuilderDockWindow(GENERATOR_PANE, dockRightID); // dockGeneratorID
	ImGui::DockBuilderDockWindow(SELECTED_FONT_PANE, dockRightID); // dockSelectionID
	ImGui::DockBuilderDockWindow(GLYPH_PANE, dockMainID);
	ImGui::DockBuilderDockWindow(FONT_STRUCTURE_PANE, dockMainID);
	ImGui::DockBuilderDockWindow(FONT_PREVIEW_PANE, dockBottomID);
#ifdef _DEBUG
	ImGui::DockBuilderDockWindow(DEBUG_PANE, dockLeftID);
#endif
	ImGui::DockBuilderFinish(m_DockSpaceID);

	m_Pane_Shown = PaneFlags::PANE_OPENED_DEFAULT; // will show when pane will be passed

	m_Pane_Focused = PaneFlags::PANE_FOCUS_DEFAULT;
	SetFocusedPanes(m_Pane_Focused);
}

void LayoutManager::DisplayMenu(ImVec2 vSize)
{
	if (ImGui::BeginMenu(ICON_IGFS_LAYOUT " Layout"))
	{
		if (ImGui::MenuItem("Default Layout"))
		{
			ApplyInitialDockingLayout(vSize);
		}

		ImGui::Separator();

		ImGui::MenuItem<PaneFlags>("Params Pane", "", &m_Pane_Shown, PaneFlags::PANE_PARAM);
		ImGui::MenuItem<PaneFlags>("Generator Pane", "", &m_Pane_Shown, PaneFlags::PANE_GENERATOR);
		ImGui::MenuItem<PaneFlags>("Source Pane", "", &m_Pane_Shown, PaneFlags::PANE_SOURCE);
		ImGui::MenuItem<PaneFlags>("Final Pane", "", &m_Pane_Shown, PaneFlags::PANE_FINAL);
		ImGui::MenuItem<PaneFlags>("Font Preview Pane", "", &m_Pane_Shown, PaneFlags::PANE_FONT_PREVIEW);
		ImGui::MenuItem<PaneFlags>("Selected Font Pane", "", &m_Pane_Shown, PaneFlags::PANE_SELECTED_FONT);
		ImGui::MenuItem<PaneFlags>("Glyph Pane", "", &m_Pane_Shown, PaneFlags::PANE_GLYPH);
		ImGui::MenuItem<PaneFlags>("Font Structure Pane", "", &m_Pane_Shown, PaneFlags::PANE_FONT_STRUCTURE);
#ifdef _DEBUG
		ImGui::Separator();
		ImGui::MenuItem<PaneFlags>("Debug Pane", "", &m_Pane_Shown, PaneFlags::PANE_DEBUG);
#endif
		ImGui::EndMenu();
	}
}

int LayoutManager::DisplayPanes(ProjectFile *vProjectFile, int vWidgetId)
{
	vWidgetId = SourceFontPane::Instance()->DrawPanes(vProjectFile, vWidgetId);
	vWidgetId = FinalFontPane::Instance()->DrawPanes(vProjectFile, vWidgetId);
	vWidgetId = GeneratorPane::Instance()->DrawPanes(vProjectFile, vWidgetId);
	vWidgetId = GlyphPane::Instance()->DrawPanes(vProjectFile, vWidgetId);
    vWidgetId = FontStructurePane::Instance()->DrawPanes(vProjectFile, vWidgetId);
	vWidgetId = FontPreviewPane::Instance()->DrawPanes(vProjectFile, vWidgetId);
#ifdef _DEBUG
	vWidgetId = DebugPane::Instance()->DrawPanes(vProjectFile, vWidgetId);
#endif

	return vWidgetId;
}

void LayoutManager::DrawDialogsAndPopups(ProjectFile* vProjectFile)
{
	SourceFontPane::Instance()->DrawDialogsAndPopups(vProjectFile);
	FinalFontPane::Instance()->DrawDialogsAndPopups(vProjectFile);
	GeneratorPane::Instance()->DrawDialogsAndPopups(vProjectFile);
	GlyphPane::Instance()->DrawDialogsAndPopups(vProjectFile);
	FontStructurePane::Instance()->DrawDialogsAndPopups(vProjectFile);
	FontPreviewPane::Instance()->DrawDialogsAndPopups(vProjectFile);
#ifdef _DEBUG
	DebugPane::Instance()->DrawDialogsAndPopups(vProjectFile);
#endif
}

void LayoutManager::ShowSpecificPane(PaneFlags vPane)
{
	m_Pane_Shown = (PaneFlags)((int32_t)m_Pane_Shown | (int32_t)vPane);
}

void LayoutManager::FocusSpecificPane(PaneFlags vPane)
{
	m_Pane_Shown = (PaneFlags)((int32_t)m_Pane_Shown | (int32_t)vPane);

	if (vPane == PaneFlags::PANE_FINAL)					FocusSpecificPane(FINAL_PANE);
	else if (vPane == PaneFlags::PANE_SELECTED_FONT)	FocusSpecificPane(SELECTED_FONT_PANE);
	else if (vPane == PaneFlags::PANE_SOURCE)			FocusSpecificPane(SOURCE_PANE);
	else if (vPane == PaneFlags::PANE_PARAM)			FocusSpecificPane(PARAM_PANE);
	else if (vPane == PaneFlags::PANE_GENERATOR)		FocusSpecificPane(GENERATOR_PANE);
	else if (vPane == PaneFlags::PANE_GLYPH)			FocusSpecificPane(GLYPH_PANE);
	else if (vPane == PaneFlags::PANE_FONT_STRUCTURE)	FocusSpecificPane(FONT_STRUCTURE_PANE);
	else if (vPane == PaneFlags::PANE_FONT_PREVIEW)		FocusSpecificPane(FONT_PREVIEW_PANE);
#ifdef _DEBUG
	else if (vPane == PaneFlags::PANE_DEBUG)			FocusSpecificPane(DEBUG_PANE);
#endif
}

void LayoutManager::ShowAndFocusSpecificPane(PaneFlags vPane)
{
	ShowSpecificPane(vPane);
	FocusSpecificPane(vPane);
}

bool LayoutManager::IsSpecificPaneFocused(PaneFlags vPane)
{
	if (vPane == PaneFlags::PANE_FINAL)					return IsSpecificPaneFocused(FINAL_PANE);
	else if (vPane == PaneFlags::PANE_SELECTED_FONT)	return IsSpecificPaneFocused(SELECTED_FONT_PANE);
	else if (vPane == PaneFlags::PANE_SOURCE)			return IsSpecificPaneFocused(SOURCE_PANE);
	else if (vPane == PaneFlags::PANE_PARAM)			return IsSpecificPaneFocused(PARAM_PANE);
	else if (vPane == PaneFlags::PANE_GENERATOR)		return IsSpecificPaneFocused(GENERATOR_PANE);
	else if (vPane == PaneFlags::PANE_GLYPH)			return IsSpecificPaneFocused(GLYPH_PANE);
	else if (vPane == PaneFlags::PANE_FONT_STRUCTURE)	return IsSpecificPaneFocused(FONT_STRUCTURE_PANE);
	else if (vPane == PaneFlags::PANE_FONT_PREVIEW)		return IsSpecificPaneFocused(FONT_PREVIEW_PANE);
#ifdef _DEBUG
	else if (vPane == PaneFlags::PANE_DEBUG)			return IsSpecificPaneFocused(DEBUG_PANE);
#endif
	return false;
}

void LayoutManager::AddSpecificPaneToExisting(const char* vNewPane, const char* vExistingPane)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vExistingPane);
	if (window)
	{
		auto dockid = window->DockId;
		ImGui::DockBuilderDockWindow(vNewPane, dockid);
	}
}

///////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////
///////////////////////////////////////////////////////

bool LayoutManager::IsSpecificPaneFocused(const char *vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel);
	if (window)
	{
		return window->DockTabIsVisible;
	}
	return false;
}

void LayoutManager::FocusSpecificPane(const char *vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel);
	if (window)
	{
		if(!window->DockTabIsVisible)
			ImGui::FocusWindow(window);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION PRVIATE ////////////////////////////
///////////////////////////////////////////////////////

PaneFlags LayoutManager::GetFocusedPanes()
{
	PaneFlags flag = PaneFlags::PANE_NONE;

	if (IsSpecificPaneFocused(FINAL_PANE))			flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_FINAL);
	if (IsSpecificPaneFocused(SELECTED_FONT_PANE))	flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_SELECTED_FONT);
	if (IsSpecificPaneFocused(SOURCE_PANE))			flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_SOURCE);
	if (IsSpecificPaneFocused(PARAM_PANE))			flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_PARAM);
	if (IsSpecificPaneFocused(GENERATOR_PANE))		flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_GENERATOR);
	if (IsSpecificPaneFocused(GLYPH_PANE))			flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_GLYPH);
	if (IsSpecificPaneFocused(FONT_STRUCTURE_PANE))	flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_FONT_STRUCTURE);
	if (IsSpecificPaneFocused(FONT_PREVIEW_PANE))	flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_FONT_PREVIEW);
#ifdef _DEBUG
	if (IsSpecificPaneFocused(DEBUG_PANE))			flag = (PaneFlags)((int32_t)flag | (int32_t)PaneFlags::PANE_DEBUG);
#endif

	return flag;
}

void LayoutManager::SetFocusedPanes(PaneFlags vActivePanes)
{
	if (vActivePanes & PaneFlags::PANE_FINAL)			FocusSpecificPane(FINAL_PANE);
	if (vActivePanes & PaneFlags::PANE_SELECTED_FONT)	FocusSpecificPane(SELECTED_FONT_PANE);
	if (vActivePanes & PaneFlags::PANE_SOURCE)			FocusSpecificPane(SOURCE_PANE);
	if (vActivePanes & PaneFlags::PANE_PARAM)			FocusSpecificPane(PARAM_PANE);
	if (vActivePanes & PaneFlags::PANE_GENERATOR)		FocusSpecificPane(GENERATOR_PANE);
	if (vActivePanes & PaneFlags::PANE_GLYPH)			FocusSpecificPane(GLYPH_PANE);
	if (vActivePanes & PaneFlags::PANE_FONT_STRUCTURE)	FocusSpecificPane(FONT_STRUCTURE_PANE);
	if (vActivePanes & PaneFlags::PANE_FONT_PREVIEW)	FocusSpecificPane(FONT_PREVIEW_PANE);
#ifdef _DEBUG
	if (vActivePanes & PaneFlags::PANE_DEBUG)			FocusSpecificPane(DEBUG_PANE);
#endif
}

///////////////////////////////////////////////////////
//// CONFIGURATION PUBLIC /////////////////////////////
///////////////////////////////////////////////////////

std::string LayoutManager::getXml(const std::string& vOffset)
{
	std::string str;

	str += vOffset + "<layout>\n";

	m_Pane_Focused = GetFocusedPanes();
	str += vOffset + "\t<panes opened=\"" + ct::ivariant((int32_t)m_Pane_Shown).GetS() + "\" active=\"" + ct::ivariant((int32_t)m_Pane_Focused).GetS() + "\"/>\n";

	str += vOffset + "</layout>\n";

	return str;
}

bool LayoutManager::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
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

	if (strParentName == "layout")
	{
		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "opened")
				m_Pane_Shown = (PaneFlags)ct::ivariant(attValue).GetI();
			if (attName == "active")
				m_Pane_Focused = (PaneFlags)ct::ivariant(attValue).GetI();
		}
	}

	return true;
}