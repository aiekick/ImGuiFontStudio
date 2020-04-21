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

#include "GuiLayout.h"

#include <FileHelper.h>
#include "MainFrame.h"
#include "Res/CustomFont.h"
#include "Gui/ImGuiWidgets.h"
#include "Project/ProjectFile.h"
#include "Panes/SourceFontPane.h"
#include "Panes/FinalFontPane.h"
#include "Panes/GlyphPane.h"
#include "Panes/GeneratorPane.h"

#ifdef _DEBUG
#include "Panes/DebugPane.h"
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

PaneFlags GuiLayout::m_Pane_Shown = PANE_ALLS;

GuiLayout::GuiLayout()
{

}

GuiLayout::~GuiLayout()
{
	
}

void GuiLayout::Init()
{
	if (!FileHelper::Instance()->IsFileExist("imgui.ini"))
	{
		m_FirstLayout = true; // need default layout
	}
}

void GuiLayout::InitAfterFirstDisplay(ImVec2 vSize)
{
	if (m_FirstLayout)
	{
		ApplyInitialDockingLayout(vSize);
		m_FirstLayout = false;
	}

	if (m_FirstStart)
	{
		// focus after start on "Source Fonts" pane
		auto win = ImGui::FindWindowByName(SOURCE_PANE);
		if (win)
			ImGui::FocusWindow(win);
		m_FirstStart = false;
	}
}

void GuiLayout::StartDockPane()
{
	m_DockSpaceID = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(m_DockSpaceID, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
}

void GuiLayout::ApplyInitialDockingLayout(ImVec2 vSize)
{
	ImGui::DockBuilderRemoveNode(m_DockSpaceID); // Clear out existing layout
	ImGui::DockBuilderAddNode(m_DockSpaceID, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImGui::DockBuilderSetNodeSize(m_DockSpaceID, vSize);

	float leftWidth = 250.0f;
	float rightWidth = 310.0f;

	ImGuiID dockMainID = m_DockSpaceID; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
	ImGuiID dockParamID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, leftWidth / vSize.x, nullptr, &dockMainID);
	ImGuiID dockRightID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, rightWidth / vSize.x, nullptr, &dockMainID);
	//ImGuiID dockSelectionID = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Up, 0.50f, NULL, &dockRight);
	//ImGuiID dockGeneratorID = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.50f, NULL, &dockRight);

	ImGui::DockBuilderDockWindow(PARAM_PANE, dockParamID);
	ImGui::DockBuilderDockWindow(SOURCE_PANE, dockMainID);
	ImGui::DockBuilderDockWindow(FINAL_PANE, dockMainID);
	ImGui::DockBuilderDockWindow(GENERATOR_PANE, dockRightID); // dockGeneratorID
	ImGui::DockBuilderDockWindow(CURRENT_FONT_PANE, dockRightID); // dockSelectionID
	ImGui::DockBuilderDockWindow(GLYPH_PANE, dockMainID);
#ifdef _DEBUG
	ImGui::DockBuilderDockWindow(DEBUG_PANE, dockRightID);
#endif
	ImGui::DockBuilderFinish(m_DockSpaceID);

	ShowAndFocusPane(PaneFlags::PANE_SOURCE);
	ShowAndFocusPane(PaneFlags::PANE_GENERATOR);

	m_Pane_Shown = PaneFlags::PANE_ALLS;
}

void GuiLayout::DisplayMenu(ImVec2 vSize)
{
	if (ImGui::BeginMenu(ICON_IGFS_LAYOUT " Layout"))
	{
		if (ImGui::MenuItem("Default Layout"))
		{
			ApplyInitialDockingLayout(vSize);
		}

		ImGui::Separator();

		ImGui::MenuItem<PaneFlags>("Show/Hide Params Pane", "", &m_Pane_Shown, PaneFlags::PANE_PARAM);
		ImGui::MenuItem<PaneFlags>("Show/Hide Source Pane", "", &m_Pane_Shown, PaneFlags::PANE_SOURCE);
		ImGui::MenuItem<PaneFlags>("Show/Hide Selected Font Pane", "", &m_Pane_Shown, PaneFlags::PANE_SELECTED_FONT);
		ImGui::MenuItem<PaneFlags>("Show/Hide Final Pane", "", &m_Pane_Shown, PaneFlags::PANE_FINAL);
		ImGui::MenuItem<PaneFlags>("Show/Hide Generator Pane", "", &m_Pane_Shown, PaneFlags::PANE_GENERATOR);
		ImGui::MenuItem<PaneFlags>("Show/Hide Glyph Pane", "", &m_Pane_Shown, PaneFlags::PANE_GLYPH);
#ifdef _DEBUG
		ImGui::MenuItem<PaneFlags>("Show/Hide Debug Pane", "", &m_Pane_Shown, PaneFlags::PANE_DEBUG);
#endif
		ImGui::EndMenu();
	}
}

int GuiLayout::DisplayPanes(ProjectFile *vProjectFile, int vWidgetId)
{
	vWidgetId = SourceFontPane::Instance()->DrawParamsPane(vProjectFile, vWidgetId);
	vWidgetId = SourceFontPane::Instance()->DrawSourceFontPane(vProjectFile, vWidgetId);
	vWidgetId = FinalFontPane::Instance()->DrawFinalFontPane(vProjectFile, vWidgetId);
	vWidgetId = FinalFontPane::Instance()->DrawCurrentFontPane(vProjectFile, vWidgetId);
	vWidgetId = GeneratorPane::Instance()->DrawGeneratorPane(vProjectFile, vWidgetId);
	vWidgetId = GlyphPane::Instance()->DrawGlyphPane(vProjectFile, vWidgetId);
#ifdef _DEBUG
	vWidgetId = DebugPane::Instance()->DrawDebugPane(vProjectFile, vWidgetId);
#endif

	return vWidgetId;
}

void GuiLayout::ShowAndFocusPane(PaneFlags vPane)
{
	m_Pane_Shown = (PaneFlags)(m_Pane_Shown | vPane);

	if (vPane == PaneFlags::PANE_FINAL)	ActivePane(FINAL_PANE);
	else if (vPane == PaneFlags::PANE_SELECTED_FONT) ActivePane(CURRENT_FONT_PANE);
	else if (vPane == PaneFlags::PANE_SOURCE) ActivePane(SOURCE_PANE);
	else if (vPane == PaneFlags::PANE_PARAM) ActivePane(PARAM_PANE);
	else if (vPane == PaneFlags::PANE_GENERATOR) ActivePane(GENERATOR_PANE);
	else if (vPane == PaneFlags::PANE_GLYPH) ActivePane(GLYPH_PANE);
#ifdef _DEBUG
	else if (vPane == PaneFlags::PANE_DEBUG) ActivePane(DEBUG_PANE);
#endif
}

bool GuiLayout::IsPaneActive(PaneFlags vPane)
{
	if (vPane == PaneFlags::PANE_FINAL)	return IsPaneActive(FINAL_PANE);
	else if (vPane == PaneFlags::PANE_SELECTED_FONT) return IsPaneActive(CURRENT_FONT_PANE);
	else if (vPane == PaneFlags::PANE_SOURCE) return IsPaneActive(SOURCE_PANE);
	else if (vPane == PaneFlags::PANE_PARAM) return IsPaneActive(PARAM_PANE);
	else if (vPane == PaneFlags::PANE_GENERATOR) return IsPaneActive(GENERATOR_PANE);
	else if (vPane == PaneFlags::PANE_GLYPH) return IsPaneActive(GLYPH_PANE);
#ifdef _DEBUG
	else if (vPane == PaneFlags::PANE_DEBUG) return IsPaneActive(DEBUG_PANE);
#endif
	return false;
}

bool GuiLayout::IsPaneActive(const char *vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel);
	if (window)
	{
		return window->DockTabIsVisible;
	}
	return false;
}

void GuiLayout::ActivePane(const char *vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel);
	if (window)
	{
		if(!window->DockTabIsVisible)
			ImGui::FocusWindow(window);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string GuiLayout::getXml(const std::string& vOffset)
{
	std::string str;

	str += vOffset + "<layout>\n";

	str += vOffset + "\t<panes value=\"" + ct::ivariant(m_Pane_Shown).getS() + "\"/>\n";
	
	str += vOffset + "</layout>\n";

	return str;
}

void GuiLayout::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
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
		auto att = vElem->FirstAttribute();
		if (att && std::string(att->Name()) == "value")
		{
			strValue = att->Value();

			if (strName == "panes") m_Pane_Shown = (PaneFlags)ct::ivariant(strValue).getI();
		}
	}
}