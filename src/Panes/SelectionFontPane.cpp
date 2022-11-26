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
#include "SelectionFontPane.h"

#include <Panes/FinalFontPane.h>

#include <MainFrame.h>
#include <Generator/Generator.h>
#include<Gui/ImWidgets.h>
#include <Panes/Manager/LayoutManager.h>
#include <Contrib/FontIcons/CustomFont.h>
#include <Helper/SelectionHelper.h>
#include <Panes/GlyphPane.h>
#include <Project/GlyphInfos.h>

#include <Helper/TranslationSystem.h>

#include <imgui/imgui_internal.h>

#include <cinttypes> // printf zu

static char sGlyphNameBuffer[512] = "\0";

SelectionFontPane::SelectionFontPane() = default;
SelectionFontPane::~SelectionFontPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool SelectionFontPane::Init()
{
	return true;
}

void SelectionFontPane::Unit()
{

}

int SelectionFontPane::DrawPanes(const uint32_t& /*vCurrentFrame*/, int vWidgetId, std::string /*vUserDatas*/, PaneFlags& vInOutPaneShown)
{
	m_PaneWidgetId = vWidgetId;

	DrawSelectedFontPane(vInOutPaneShown);

	return m_PaneWidgetId;
}

void SelectionFontPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, std::string /*vUserDatas*/)
{

}

int SelectionFontPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, int vWidgetId, std::string /*vUserDatas*/)
{
	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SelectionFontPane::DrawSelectedFontPane(PaneFlags& vInOutPaneShown)
{
	if (vInOutPaneShown & m_PaneFlag)
	{
		static ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar;
		if (ImGui::Begin<PaneFlags>(m_PaneName,
			&vInOutPaneShown, m_PaneFlag, flags))
		{
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
			auto win = ImGui::GetCurrentWindowRead();
			if (win->Viewport->Idx != 0)
				flags |= ImGuiWindowFlags_NoResize;// | ImGuiWindowFlags_NoTitleBar;
			else
				flags = ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_MenuBar;
#endif
			if (ProjectFile::Instance()->IsLoaded())
			{
				if (ProjectFile::Instance()->m_SelectedFont)
				{
					if (ImGui::BeginMenuBar())
					{
						if (ImGui::BeginMenu("Sorting"))
						{
							if (ImGui::MenuItem<SelectedFontPaneModeFlags>("by CodePoint", "",
								&m_SelectedFontPaneModeFlags,
								SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT, true))
							{
								FinalFontPane::Instance()->PrepareSelectionByFontOrderedByCodePoint();
							}

							if (ImGui::MenuItem<SelectedFontPaneModeFlags>("by Name", "",
								&m_SelectedFontPaneModeFlags,
								SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES, true))
							{
								FinalFontPane::Instance()->PrepareSelectionByFontOrderedByGlyphNames();
							}

							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Infos"))
						{
							if (ImGui::MenuItem("Show Tooltip", "", ProjectFile::Instance()->m_CurrentPane_ShowGlyphTooltip))
							{
								ProjectFile::Instance()->SetProjectChange();
							}

							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Edition"))
						{
							ImGui::MenuItem("Auto Update codePoint during Edition", "",
								&m_AutoUpdateCodepoint_WhenEditWithButtons);

							ImGui::EndMenu();
						}
						
						ImGui::EndMenuBar();
					}

					if (m_SelectedFontPaneModeFlags & SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT)
					{
						FinalFontPane::Instance()->DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(ProjectFile::Instance()->m_SelectedFont, false, true, false, ProjectFile::Instance()->m_CurrentPane_ShowGlyphTooltip);
					}
					else if (m_SelectedFontPaneModeFlags & SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES)
					{
						FinalFontPane::Instance()->DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(ProjectFile::Instance()->m_SelectedFont, false, true, false, ProjectFile::Instance()->m_CurrentPane_ShowGlyphTooltip);
					}
				}
			}
		}

		ImGui::End();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
////// PUBLIC : PREPARATON ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool SelectionFontPane::IsSelectedFontPaneMode(SelectedFontPaneModeFlags vSelectedFontPaneModeFlags)
{
	return (m_SelectedFontPaneModeFlags & vSelectedFontPaneModeFlags) == vSelectedFontPaneModeFlags; // check
}

void SelectionFontPane::PrepareSelection()
{
	if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT))
	{
		FinalFontPane::Instance()->PrepareSelectionByFontOrderedByCodePoint();
	}
	else if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES))
	{
		FinalFontPane::Instance()->PrepareSelectionByFontOrderedByGlyphNames();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
////// PRIVATE ////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

std::string SelectionFontPane::getXml(const std::string& vOffset, const std::string& vUserDatas = "")
{
	UNUSED(vOffset);
	UNUSED(vUserDatas);

	std::string str;

	//str += vOffset + "<finalfontpane>\n";
	//str += vOffset + "\t<glyphsizepolicy_count>" + ct::toStr(m_Final_GlyphSize_Policy_Count) + "</glyphsizepolicy_count>\n";
	//str += vOffset + "\t<glyphsizepolicy_width>" + ct::toStr(m_Final_GlyphSize_Policy_Width) + "</glyphsizepolicy_width>\n";
	//str += vOffset + "</finalfontpane>\n";

	//str += vOffset + "<selectedfontpane>\n";
	//str += vOffset + "\t<glyphsizepolicy_count>" + ct::toStr(m_Selected_GlyphSize_Policy_Count) + "</glyphsizepolicy_count>\n";
	//str += vOffset + "\t<glyphsizepolicy_width>" + ct::toStr(m_Selected_GlyphSize_Policy_Width) + "</glyphsizepolicy_width>\n";
	//str += vOffset + "</selectedfontpane>\n";

	return str;
}

bool SelectionFontPane::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "")
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

	if (strParentName == "selectedfontpane")
	{
		/*if (strName == "glyphsizepolicy_count")
			m_Selected_GlyphSize_Policy_Count = ct::ivariant(strValue).GetI();
		else if (strName == "glyphsizepolicy_width")
			m_Selected_GlyphSize_Policy_Width = ct::fvariant(strValue).GetF();*/
	}

	return true;
}