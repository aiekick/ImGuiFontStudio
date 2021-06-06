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
#include "FinalFontPane.h"

#include <MainFrame.h>
#include <Generator/Generator.h>
#include <Gui/ImWidgets.h>
#include <Panes/Manager/LayoutManager.h>
#include <Res/CustomFont.h>
#include <Helper/SelectionHelper.h>
#include <Panes/GlyphPane.h>
#include <Project/GlyphInfos.h>

#include <Helper/TranslationSystem.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <cinttypes> // printf zu

static char sGlyphNameBuffer[512] = "\0";

FinalFontPane::FinalFontPane() = default;
FinalFontPane::~FinalFontPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool FinalFontPane::Init()
{
	return true;
}

void FinalFontPane::Unit()
{

}

int FinalFontPane::DrawPanes(int vWidgetId, std::string vUserDatas)
{
	m_PaneWidgetId = vWidgetId;

	DrawFinalFontPane();
	DrawSelectedFontPane();

	return m_PaneWidgetId;
}

void FinalFontPane::DrawDialogsAndPopups(std::string vUserDatas)
{

}

int FinalFontPane::DrawWidgets(int vWidgetId, std::string vUserDatas)
{
	UNUSED(vUserDatas);

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FinalFontPane::DrawFinalFontPane()
{
	if (LayoutManager::Instance()->m_Pane_Shown & m_PaneFlag)
	{
		if (ImGui::BeginFlag<PaneFlags>(m_PaneName,
			&LayoutManager::Instance()->m_Pane_Shown, m_PaneFlag,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (ProjectFile::Instance()->IsLoaded())
			{
				if (!ProjectFile::Instance()->m_Fonts.empty())
				{
					if (ImGui::BeginMenuBar())
					{
						if (ImGui::BeginMenu(TSLOC(FFP,ViewMode)))
						{
							ImGui::MenuItem<FinalFontPaneModeFlags>("by Font, no order", "",
								&m_FinalFontPaneModeFlags, 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_NO_ORDER, true);

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("by Font, ordered by CodePoint", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT, true))
							{
								PrepareSelectionByFontOrderedByCodePoint();
							}

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("by Font, ordered by Name", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES, true))
							{
								PrepareSelectionByFontOrderedByGlyphNames();
							}

							ImGui::Spacing();

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("Merged, no order", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_NO_ORDER, true))
							{
								PrepareSelectionMergedNoOrder();
							}

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("Merged, ordered by CodePoint", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_CODEPOINT, true))
							{
								PrepareSelectionMergedOrderedByCodePoint();
							}

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("Merged, ordered by Name", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_NAMES, true))
							{
								PrepareSelectionMergedOrderedByGlyphNames();
							}

							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Infos"))
						{
							if (ImGui::MenuItem("Show Tooltip", "", ProjectFile::Instance()->m_FinalPane_ShowGlyphTooltip))
							{
								ProjectFile::Instance()->SetProjectChange();
							}
							
							ImGui::EndMenu();
						}

						SelectionHelper::Instance()->DrawSelectionMenu(SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

						if (ImGui::BeginMenu("Edition"))
						{
							ImGui::MenuItem("Active Edit mode", "", &m_GlyphEdition);

							if (m_GlyphEdition)
							{
								ImGui::MenuItem("Auto Update codePoint during Edition", "",
									&m_AutoUpdateCodepoint_WhenEditWithButtons);
							}

							ImGui::EndMenu();
						}

						ImGui::EndMenuBar();
					}

					if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_NO_ORDER)
					{
						DrawSelectionsByFontNoOrder(ProjectFile::Instance()->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT)
					{
						DrawSelectionsByFontOrderedByCodePoint(ProjectFile::Instance()->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES)
					{
						DrawSelectionsByFontOrderedByGlyphNames(ProjectFile::Instance()->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_NO_ORDER)
					{
						DrawSelectionMergedNoOrder();
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_CODEPOINT)
					{
						DrawSelectionMergedOrderedByCodePoint();
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_NAMES)
					{
						DrawSelectionMergedOrderedByGlyphNames();
					}
				}
			}
		}

		ImGui::End();
	}
}

void FinalFontPane::DrawSelectedFontPane()
{
	/*if (LayoutManager::Instance()->m_Pane_Shown & m_PaneFlag)
	{
		if (ImGui::BeginFlag<PaneFlags>(m_PaneName,
			&LayoutManager::Instance()->m_Pane_Shown, m_PaneFlag,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
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
								PrepareSelectionByFontOrderedByCodePoint();
							}

							if (ImGui::MenuItem<SelectedFontPaneModeFlags>("by Name", "",
								&m_SelectedFontPaneModeFlags,
								SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES, true))
							{
								PrepareSelectionByFontOrderedByGlyphNames();
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
						DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(ProjectFile::Instance()->m_SelectedFont, false, true, false, ProjectFile::Instance()->m_CurrentPane_ShowGlyphTooltip);
					}
					else if (m_SelectedFontPaneModeFlags & SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES)
					{
						DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(ProjectFile::Instance()->m_SelectedFont, false, true, false, ProjectFile::Instance()->m_CurrentPane_ShowGlyphTooltip);
					}
				}
			}
		}

		ImGui::End();
	}*/
}

///////////////////////////////////////////////////////////////////////////////////////////
////// PUBLIC : PREPARATON ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void FinalFontPane::SetFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags)
{
	m_FinalFontPaneModeFlags = (FinalFontPaneModeFlags)(vFinalFontPaneModeFlags); /// set
}

bool FinalFontPane::IsFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags)
{
	return (m_FinalFontPaneModeFlags & vFinalFontPaneModeFlags) == vFinalFontPaneModeFlags; // check
}

bool FinalFontPane::IsSelectedFontPaneMode(SelectedFontPaneModeFlags vSelectedFontPaneModeFlags)
{
	return (m_SelectedFontPaneModeFlags & vSelectedFontPaneModeFlags) == vSelectedFontPaneModeFlags; // check
}

void FinalFontPane::PrepareSelection()
{
	if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT))
	{
		PrepareSelectionByFontOrderedByCodePoint();
	}
	else if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES))
	{
		PrepareSelectionByFontOrderedByGlyphNames();
	}

	if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_NO_ORDER))
	{
		// nothing to prepare because this is the default view => pointed on FontInfos->m_SelectedGlyphs
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT))
	{
		PrepareSelectionByFontOrderedByCodePoint();
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES))
	{
		PrepareSelectionByFontOrderedByGlyphNames();
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_NO_ORDER))
	{
		PrepareSelectionMergedNoOrder();
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_CODEPOINT))
	{
		PrepareSelectionMergedOrderedByCodePoint();
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_NAMES))
	{
		PrepareSelectionMergedOrderedByGlyphNames();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
////// PRIVATE ////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool FinalFontPane::DrawGlyph(
	std::shared_ptr<FontInfos> vFontInfos, const ImVec2& vSize,
	std::shared_ptr<GlyphInfos> vGlyph, bool vShowRect,
	ImVec4 vGlyphButtonStateColor[3],
	bool *vNameupdated, bool *vCodePointUpdated, 
	bool vForceEditMode)
{
	int res = false;
	
	if (vFontInfos.use_count() && vGlyph.use_count())
	{
		ImGui::PushID(vGlyph.get());
		ImGui::BeginGroup();
		
		bool selected = false;
		SelectionHelper::Instance()->IsGlyphIntersectedAndSelected(
			vFontInfos, vSize, vGlyph->glyph.Codepoint, &selected, 
			SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

		ct::fvec2 trans = vGlyph->m_Translation * vFontInfos->m_Point;
		ct::fvec2 scale = vGlyph->m_Scale;

		res = GlyphInfos::DrawGlyphButton(
			m_PaneWidgetId,
			vFontInfos->GetImFont(),
			&selected, vSize, &vGlyph->glyph, vGlyphButtonStateColor, vGlyph->m_Colored,
			ImVec2(trans.x, trans.y), ImVec2(scale.x, scale.y), 
			-1, vShowRect ? 3.0f : 0.0f);

		if (res)
		{
			// left button : check == 1
			// right button  : check == 2
			if (res == 2) // put glyph on glyph pane
			{
				GlyphPane::Instance()->LoadGlyph(vFontInfos, vGlyph);
			}

			if (vForceEditMode)
			{
				GlyphPane::Instance()->LoadGlyph(vFontInfos, vGlyph);
			}
			else
			{
				SelectionHelper::Instance()->SelectWithToolOrApplyOnGlyph(
					vFontInfos,
					vGlyph->glyph, 0, selected, true,
					SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
			}
		}

		if (m_GlyphEdition || vForceEditMode)
		{
			ImGui::SameLine();

			ImGui::BeginGroup();

			bool displayResetHeaderNameBtn = (vGlyph->newHeaderName != vGlyph->oldHeaderName);
			float ResetHeaderNameBtnWidth = (displayResetHeaderNameBtn ?
				ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f);
			if (!vGlyph->m_editingName)
			{
				vGlyph->editHeaderName = vGlyph->newHeaderName;
				ImGui::PushItemWidth(
					GLYPH_EDIT_CONTROL_WIDTH - ResetHeaderNameBtnWidth
				);
			}
			else
			{
				float x = ImGui::GetCursorScreenPos().x;
				if (ImGui::ContrastedButton(ICON_IGFS_OK "##okname", nullptr, nullptr, ImGui::GetFrameHeight()))
				{
					if (vNameupdated)
						*vNameupdated = true;
					vGlyph->newHeaderName = vGlyph->editHeaderName;
					SelectionHelper::Instance()->AnalyseSourceSelection();
					ProjectFile::Instance()->SetProjectChange();
					vGlyph->m_editingName = false;
				}
				ImGui::SameLine();
				if (ImGui::ContrastedButton(ICON_IGFS_CANCEL "##cancelname", nullptr, nullptr, ImGui::GetFrameHeight()))
				{
					vGlyph->m_editingName = false;
				}
				ImGui::SameLine();
				ImGui::PushItemWidth(
					GLYPH_EDIT_CONTROL_WIDTH - 
					ImGui::GetCursorScreenPos().x + x - ImGui::GetStyle().ItemInnerSpacing.x - 
					ResetHeaderNameBtnWidth
				);
			}
			snprintf(sGlyphNameBuffer, 511, "%s", vGlyph->editHeaderName.c_str());
			bool nameChanged = ImGui::InputText("##glyphname", sGlyphNameBuffer, 512);
			ImGui::PopItemWidth();
			if (nameChanged)
			{
				vGlyph->editHeaderName = std::string(sGlyphNameBuffer);
				vGlyph->m_editingName = true;
			}
			if (displayResetHeaderNameBtn)
			{
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetFrameHeight());
				if (ImGui::ContrastedButton("R##resetname", nullptr, nullptr, ImGui::GetFrameHeight()))
				{
					if (vNameupdated)
						*vNameupdated = true;
					vGlyph->newHeaderName = vGlyph->oldHeaderName;
					SelectionHelper::Instance()->AnalyseSourceSelection();
					ProjectFile::Instance()->SetProjectChange();
				}
				ImGui::PopItemWidth();
			}

			bool displayResetCodePointBtn = ((uint32_t)vGlyph->newCodePoint != vGlyph->glyph.Codepoint);
			float ResetCodePointBtnWidth = (displayResetCodePointBtn ? 
				ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f);
			if (!vGlyph->m_editingCodePoint)
			{
				vGlyph->editCodePoint = vGlyph->newCodePoint;
				ImGui::PushItemWidth(
					GLYPH_EDIT_CONTROL_WIDTH - ImGui::GetFrameHeight() * 2.0f - 
					ImGui::GetStyle().ItemInnerSpacing.x * 2.0f - ResetCodePointBtnWidth
				);
			}
			else 
			{
				float x = ImGui::GetCursorScreenPos().x;
				bool btn = m_AutoUpdateCodepoint_WhenEditWithButtons;
				if (!btn)
					btn = ImGui::ContrastedButton(ICON_IGFS_OK "##okcodepoint", nullptr, nullptr, ImGui::GetFrameHeight());
				if (btn)
				{
					if (vCodePointUpdated)
						*vCodePointUpdated = true;
					vGlyph->newCodePoint = (uint32_t)vGlyph->editCodePoint;// range 0 => 2^16;
					SelectionHelper::Instance()->AnalyseSourceSelection();
					ProjectFile::Instance()->SetProjectChange();
					vGlyph->m_editingCodePoint = false;
				}
				if (!m_AutoUpdateCodepoint_WhenEditWithButtons)
				{
					ImGui::SameLine();
					if (ImGui::ContrastedButton(ICON_IGFS_CANCEL "##cancelcodepoint", nullptr, nullptr, ImGui::GetFrameHeight()))
					{
						vGlyph->m_editingCodePoint = false;
					}
					ImGui::SameLine();
					ImGui::PushItemWidth(
						GLYPH_EDIT_CONTROL_WIDTH - 
						ImGui::GetCursorScreenPos().x + x - ImGui::GetFrameHeight() * 2.0f - 
						ImGui::GetStyle().ItemInnerSpacing.x * 2.0f - ResetCodePointBtnWidth
					);
				}
				else
				{
					ImGui::PushItemWidth(
						GLYPH_EDIT_CONTROL_WIDTH - ImGui::GetFrameHeight() * 2.0f - 
						ImGui::GetStyle().ItemInnerSpacing.x * 2.0f - ResetCodePointBtnWidth
					);
				}
			}

			bool codePointChanged = ImGui::InputInt("##glyphnewcodepoint", &vGlyph->editCodePoint, 0, 0);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::ContrastedButton(ICON_IGFS_REMOVE"##glyphdeccodepoint", nullptr, nullptr, ImGui::GetFrameHeight()))
			{
				codePointChanged = true;
				vGlyph->editCodePoint--;
			}
			ImGui::SameLine();
			if (ImGui::ContrastedButton(ICON_IGFS_ADD"##glyphinccodepoint", nullptr, nullptr, ImGui::GetFrameHeight()))
			{
				codePointChanged = true;
				vGlyph->editCodePoint++;
			}
			if (codePointChanged)
			{
				vGlyph->editCodePoint = ct::clamp<int>(vGlyph->editCodePoint, 0, 65535);
				vGlyph->m_editingCodePoint = true;
			}
			if (displayResetCodePointBtn)
			{
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetFrameHeight());
				if (ImGui::ContrastedButton("R##resetcodepoint", nullptr, nullptr, ImGui::GetFrameHeight()))
				{
					if (vCodePointUpdated)
						*vCodePointUpdated = true;
					vGlyph->newCodePoint = vGlyph->glyph.Codepoint;
					SelectionHelper::Instance()->AnalyseSourceSelection();
					ProjectFile::Instance()->SetProjectChange();
				}
				ImGui::PopItemWidth();
			}
			ImGui::EndGroup();
		}

		ImGui::EndGroup();
		ImGui::PopID();
	}
	
	return (res > 0);
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontNoOrder( bool vShowTooltipInfos)
{
		for (auto it : ProjectFile::Instance()->m_Fonts)
		{
			DrawSelectionsByFontNoOrder_OneFontOnly(it.second, true, false, false, vShowTooltipInfos);
		}
		
		SelectionHelper::Instance()->SelectWithToolOrApply(
			SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
}

static inline void DrawGlyphInfosToolTip(std::shared_ptr<FontInfos> vFontInfos, std::shared_ptr<GlyphInfos> vGlyphInfos)
{
	if (ImGui::IsItemHovered() && vFontInfos.use_count() && vGlyphInfos.use_count())
	{
		ImGui::SetTooltip("glyph index : %i\nnew name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
			vGlyphInfos->glyphIndex,
			vGlyphInfos->newHeaderName.c_str(),
			(int)vGlyphInfos->newCodePoint,
			vGlyphInfos->oldHeaderName.c_str(),
			(int)vGlyphInfos->glyph.Codepoint,
			vFontInfos->m_FontFileName.c_str());
	}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontNoOrder_OneFontOnly(
	std::shared_ptr<FontInfos> vFontInfos,
	bool vWithFramedGroup,
	bool vForceEditMode,
	bool vForceEditModeOneColumn,
	bool vShowTooltipInfos)
{
	if (vFontInfos.use_count())
	{
		if (vFontInfos->m_SelectedGlyphs.empty())
			return;

		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				if (vFontInfos->m_SelectedGlyphs.begin()->second)
				{
					uint32_t startCodePoint = vFontInfos->m_SelectedGlyphs.begin()->second->glyph.Codepoint;

					char buffer[1024] = "\0";
					snprintf(buffer, 1023, "Font %s / Start CodePoint %u / Count %u",
						vFontInfos->m_FontFileName.c_str(), startCodePoint,
						(uint32_t)vFontInfos->m_SelectedGlyphs.size());
					bool frm = true;
					if (vWithFramedGroup)
						frm = ProjectFile::Instance()->CollapsingHeader_Centered(buffer, -1.0f, vFontInfos->m_CollapseFontInFinalPane);
					if (frm)
					{
						vFontInfos->m_CollapseFontInFinalPane = false;
						ImVec2 cell_size, glyph_size;
						uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(&cell_size, &glyph_size, m_GlyphEdition, vForceEditMode, vForceEditModeOneColumn);
						if (glyphCountX)
						{
							uint32_t idx = 0;
							uint32_t lastGlyphCodePoint = 0;
							m_GlyphButtonStateColor[0] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
							bool showRangeColoring = ProjectFile::Instance()->IsRangeColoringShown();

							bool nameUpdated = false;
							bool codepointUpdated = false;

							for (auto& it : vFontInfos->m_SelectedGlyphs)
							{
								uint32_t x = idx % glyphCountX;

								uint32_t codePoint = it.first;
								auto glyphInfo = it.second;

								if (x) ImGui::SameLine();

								GlyphInfos::GetGlyphButtonColorsForCodePoint(showRangeColoring, 
									codePoint, lastGlyphCodePoint, m_GlyphButtonStateColor);

								DrawGlyph(vFontInfos,
									glyph_size, glyphInfo, false, m_GlyphButtonStateColor,
									&nameUpdated, &codepointUpdated, vForceEditMode);

								if (vShowTooltipInfos)
								{
									DrawGlyphInfosToolTip(vFontInfos, glyphInfo);
								}

								lastGlyphCodePoint = codePoint;

								idx++;
							}

							// update for other views
							if (nameUpdated)
							{
								PrepareSelectionByFontOrderedByGlyphNames();
								PrepareSelectionMergedOrderedByGlyphNames();
							}

							// update for other views
							if (codepointUpdated)
							{
								PrepareSelectionByFontOrderedByCodePoint();
								PrepareSelectionMergedOrderedByCodePoint();
							}
						}
					}
				}
			}
		}
	}
}

void FinalFontPane::PrepareSelectionByFontOrderedByCodePoint()
{

		for (auto itFont : ProjectFile::Instance()->m_Fonts)
		{
			itFont.second->m_GlyphsOrderedByCodePoints.clear();

			for (auto& itGlyph : itFont.second->m_SelectedGlyphs)
			{
				if (itGlyph.second)
				{
					itGlyph.second->SetFontInfos(itFont.second);
					itFont.second->m_GlyphsOrderedByCodePoints[itGlyph.second->newCodePoint].push_back(itGlyph.second);
				}
			}
		}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByCodePoint( bool vShowTooltipInfos)
{

		for (auto it : ProjectFile::Instance()->m_Fonts)
		{
			DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(it.second, true, false, false, vShowTooltipInfos);
		}

		SelectionHelper::Instance()->SelectWithToolOrApply(
			SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(
	std::shared_ptr<FontInfos> vFontInfos,
	bool vWithFramedGroup, 
	bool vForceEditMode, 
	bool vForceEditModeOneColumn,
	bool vShowTooltipInfos)
{
	if (vFontInfos.use_count())
	{
		if (vFontInfos->m_GlyphsOrderedByCodePoints.empty())
			return;

		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				if (vFontInfos->m_GlyphsOrderedByCodePoints.empty())
				{
					PrepareSelectionByFontOrderedByCodePoint();
				}
				else
				{
                    uint32_t startCodePoint = vFontInfos->m_GlyphsOrderedByCodePoints.begin()->second[0]->newCodePoint;

					char buffer[1024] = "\0";
					snprintf(buffer, 1023, "Font %s / Start CodePoint %u / Count %u",
						vFontInfos->m_FontFileName.c_str(), startCodePoint,
						(uint32_t)vFontInfos->m_GlyphsOrderedByCodePoints.size());
					bool frm = true;
					if (vWithFramedGroup)
						frm = ProjectFile::Instance()->CollapsingHeader_Centered(buffer, -1.0f, vFontInfos->m_CollapseFontInFinalPane);
					if (frm)
					{
						ImVec2 cell_size, glyph_size;
						uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(&cell_size, &glyph_size, m_GlyphEdition, vForceEditMode, vForceEditModeOneColumn);
						if (glyphCountX)
						{
                            uint32_t idx = 0;
                            uint32_t lastGlyphCodePoint = 0;
							m_GlyphButtonStateColor[0] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
							bool showRangeColoring = ProjectFile::Instance()->IsRangeColoringShown();

							bool nameUpdated = false;
							bool codepointUpdated = false;

							for (auto &itGlyph : vFontInfos->m_GlyphsOrderedByCodePoints)
							{
								uint32_t codePoint = itGlyph.first;
								auto glyphVector = itGlyph.second;

								// si plus d'un glyph ici, alors deux glyph partagent le meme codepoint
								// et il va falloir le montrer
								// un rerange sera necesaire
								for (auto & glyphInfo : glyphVector)
								{
                                    uint32_t x = idx++ % glyphCountX;

									if (x) ImGui::SameLine();

									GlyphInfos::GetGlyphButtonColorsForCodePoint(showRangeColoring,
										codePoint, lastGlyphCodePoint, m_GlyphButtonStateColor);
									
									DrawGlyph(vFontInfos,
										glyph_size, glyphInfo, glyphVector.size() > 1, m_GlyphButtonStateColor,
										&nameUpdated, &codepointUpdated, vForceEditMode);

									if (vShowTooltipInfos)
									{
										DrawGlyphInfosToolTip(vFontInfos, glyphInfo);

										/*if (ImGui::IsItemHovered())
										{
											ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
												glyphInfo->newHeaderName.c_str(),
												(int)glyphInfo->newCodePoint,
												glyphInfo->oldHeaderName.c_str(),
												(int)glyphInfo->glyph.Codepoint,
												vFontInfos->m_FontFileName.c_str());
										}*/
									}

									lastGlyphCodePoint = codePoint;
								}
							}

							if (nameUpdated)
							{
								PrepareSelectionByFontOrderedByGlyphNames();
								PrepareSelectionMergedOrderedByGlyphNames();
							}

							if (codepointUpdated)
							{
								PrepareSelectionByFontOrderedByCodePoint();
								PrepareSelectionMergedOrderedByCodePoint();
							}
						}
					}
				}
			}
		}
	}
}

void FinalFontPane::PrepareSelectionByFontOrderedByGlyphNames()
{
		for (auto itFont : ProjectFile::Instance()->m_Fonts)
		{
			itFont.second->m_GlyphsOrderedByGlyphName.clear();

			for (auto& itGlyph : itFont.second->m_SelectedGlyphs)
			{
				if (itGlyph.second)
				{
					itGlyph.second->SetFontInfos(itFont.second);
					itFont.second->m_GlyphsOrderedByGlyphName[itGlyph.second->newHeaderName].push_back(itGlyph.second);
				}
			}
		}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByGlyphNames( bool vShowTooltipInfos)
{
		for (auto it : ProjectFile::Instance()->m_Fonts)
		{
			DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(it.second, true, false, false, vShowTooltipInfos);
		}

		SelectionHelper::Instance()->SelectWithToolOrApply(
			SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(
	std::shared_ptr<FontInfos> vFontInfos,
	bool vWithFramedGroup, 
	bool vForceEditMode, 
	bool vForceEditModeOneColumn,
	bool vShowTooltipInfos)
{
	if (vFontInfos.use_count())
	{
		if (vFontInfos->m_GlyphCodePointToName.empty())
			return;

		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				if (vFontInfos->m_GlyphsOrderedByGlyphName.empty())
				{
					PrepareSelectionByFontOrderedByGlyphNames();
				}
				else
				{
					std::string name = vFontInfos->m_GlyphsOrderedByGlyphName.begin()->second[0]->newHeaderName;

					char buffer[1024] = "\0";
					snprintf(buffer, 1023, "Font %s / Start Name %s / Count Names %u",
						vFontInfos->m_FontFileName.c_str(), name.c_str(),
						(uint32_t)vFontInfos->m_GlyphsOrderedByGlyphName.size());
					bool frm = true;
					if (vWithFramedGroup)
						frm = ProjectFile::Instance()->CollapsingHeader_Centered(buffer, -1.0f, vFontInfos->m_CollapseFontInFinalPane);
					if (frm)
					{
						ImVec2 cell_size2, glyph_size;
						uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(&cell_size2, &glyph_size, m_GlyphEdition, vForceEditMode, vForceEditModeOneColumn);
						if (glyphCountX)
						{
							int idx = 0;
                            uint32_t lastGlyphCodePoint = 0;
							m_GlyphButtonStateColor[0] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
							bool showRangeColoring = ProjectFile::Instance()->IsRangeColoringShown();

							bool nameUpdated = false;
							bool codepointUpdated = false;

							for (auto &itGlyph : vFontInfos->m_GlyphsOrderedByGlyphName)
							{
								auto glyphVector = itGlyph.second;

								// si plus d'un glyph ici, alors deux glyph partagent le meme codepoint
								// et il va falloir le montrer
								// un rerange sera necesaire
								for (auto & glyphInfo : glyphVector)
								{
                                    uint32_t x = idx++ % glyphCountX;

									if (x) ImGui::SameLine();

									GlyphInfos::GetGlyphButtonColorsForCodePoint(showRangeColoring,
										glyphInfo->newCodePoint, lastGlyphCodePoint, m_GlyphButtonStateColor);

									DrawGlyph(vFontInfos,
										glyph_size, glyphInfo, glyphVector.size() > 1, m_GlyphButtonStateColor,
										&nameUpdated, &codepointUpdated, vForceEditMode);

									if (vShowTooltipInfos)
									{
										DrawGlyphInfosToolTip(vFontInfos, glyphInfo);
										
										/*if (ImGui::IsItemHovered())
										{
											ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
												glyphInfo->newHeaderName.c_str(),
												(int)glyphInfo->newCodePoint,
												glyphInfo->oldHeaderName.c_str(),
												(int)glyphInfo->glyph.Codepoint,
												vFontInfos->m_FontFileName.c_str());
										}*/
									}

									lastGlyphCodePoint = glyphInfo->newCodePoint;
								}
							}

							if (nameUpdated)
							{
								PrepareSelectionByFontOrderedByGlyphNames();
								PrepareSelectionMergedOrderedByGlyphNames();
							}

							if (codepointUpdated)
							{
								PrepareSelectionByFontOrderedByCodePoint();
								PrepareSelectionMergedOrderedByCodePoint();
							}
						}
					}
				}
			}
		}
	}
}

void FinalFontPane::PrepareSelectionMergedNoOrder()
{
	m_GlyphsMergedNoOrder.clear();

		for (auto itFont : ProjectFile::Instance()->m_Fonts)
		{
			for (auto& itGlyph : itFont.second->m_SelectedGlyphs)
			{
				if (itGlyph.second)
				{
					itGlyph.second->SetFontInfos(itFont.second);
					m_GlyphsMergedNoOrder.push_back(itGlyph.second);
				}
			}
		}
}

void FinalFontPane::DrawSelectionMergedNoOrder()
{
	if (m_GlyphsMergedNoOrder.empty())
		return;

        ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(&cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphCodePoint = 0;
			m_GlyphButtonStateColor[0] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			bool showRangeColoring = ProjectFile::Instance()->IsRangeColoringShown();

			bool nameUpdated = false;
			bool codepointUpdated = false;

			for (auto& glyphInfo : m_GlyphsMergedNoOrder)
			{
				auto fontInfos = glyphInfo->GetFontInfos();
				if (!fontInfos.expired() && glyphCountX)
				{
					auto fontInfosPtr = fontInfos.lock();
					if (fontInfosPtr.use_count())
					{
						if (fontInfosPtr->m_ImFontAtlas.IsBuilt())
						{
							if (fontInfosPtr->m_ImFontAtlas.TexID)
							{
								uint32_t x = idx++ % glyphCountX;

								if (x) ImGui::SameLine();

								GlyphInfos::GetGlyphButtonColorsForCodePoint(showRangeColoring,
									glyphInfo->newCodePoint, lastGlyphCodePoint, m_GlyphButtonStateColor);

								DrawGlyph(fontInfosPtr,
									glyph_size, glyphInfo, false, m_GlyphButtonStateColor,
									&nameUpdated, &codepointUpdated);

								if (ProjectFile::Instance()->m_FinalPane_ShowGlyphTooltip)
								{
									DrawGlyphInfosToolTip(fontInfosPtr, glyphInfo);

									/*if (ImGui::IsItemHovered())
									{
										ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
											glyphInfo->newHeaderName.c_str(),
											(int)glyphInfo->newCodePoint,
											glyphInfo->oldHeaderName.c_str(),
											(int)glyphInfo->glyph.Codepoint,
											vFontInfos->m_FontFileName.c_str());
									}*/
								}

								lastGlyphCodePoint = glyphInfo->newCodePoint;
							}
						}
					}
				}
			}

			SelectionHelper::Instance()->SelectWithToolOrApply(
				SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

			if (nameUpdated)
			{
				PrepareSelectionByFontOrderedByGlyphNames();
				PrepareSelectionMergedOrderedByGlyphNames();
			}

			if (codepointUpdated)
			{
				PrepareSelectionByFontOrderedByCodePoint();
				PrepareSelectionMergedOrderedByCodePoint();
			}
		}
}

void FinalFontPane::PrepareSelectionMergedOrderedByCodePoint()
{
	m_GlyphsMergedOrderedByCodePoints.clear();

		for (auto itFont : ProjectFile::Instance()->m_Fonts)
		{
			for (auto& itGlyph : itFont.second->m_SelectedGlyphs)
			{
				if (itGlyph.second)
				{
					itGlyph.second->SetFontInfos(itFont.second);
					m_GlyphsMergedOrderedByCodePoints[itGlyph.second->newCodePoint].push_back(itGlyph.second);
				}
			}
		}
}

void FinalFontPane::DrawSelectionMergedOrderedByCodePoint()
{
	if (m_GlyphsMergedOrderedByCodePoints.empty())
		return;

		ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(&cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphCodePoint = 0;
			m_GlyphButtonStateColor[0] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			bool showRangeColoring = ProjectFile::Instance()->IsRangeColoringShown();

			bool nameUpdated = false;
			bool codepointUpdated = false;

			for (auto& itGlyph : m_GlyphsMergedOrderedByCodePoints)
			{
				uint32_t codePoint = itGlyph.first;
				auto glyphVector = itGlyph.second;

				// si plus d'un glyph ici, alors deux glyph partagent le meme codepoint
				// et il va falloir le montrer
				// un rerange sera necesaire
				for (auto& glyphInfo : glyphVector)
				{
					auto fontInfos = glyphInfo->GetFontInfos();
					if (!fontInfos.expired() && glyphCountX)
					{
						auto fontInfosPtr = fontInfos.lock();
						if (fontInfosPtr.use_count())
						{
							if (fontInfosPtr->m_ImFontAtlas.IsBuilt())
							{
								if (fontInfosPtr->m_ImFontAtlas.TexID)
								{
									uint32_t x = idx++ % glyphCountX;

									if (x) ImGui::SameLine();

									GlyphInfos::GetGlyphButtonColorsForCodePoint(showRangeColoring,
										codePoint, lastGlyphCodePoint, m_GlyphButtonStateColor);

									DrawGlyph(fontInfosPtr,
										glyph_size, glyphInfo, glyphVector.size() > 1, m_GlyphButtonStateColor,
										&nameUpdated, &codepointUpdated);

									if (ProjectFile::Instance()->m_FinalPane_ShowGlyphTooltip)
									{
										DrawGlyphInfosToolTip(fontInfosPtr, glyphInfo);

										/*if (ImGui::IsItemHovered())
										{
											ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
												glyphInfo->newHeaderName.c_str(),
												(int)glyphInfo->newCodePoint,
												glyphInfo->oldHeaderName.c_str(),
												(int)glyphInfo->glyph.Codepoint,
												vFontInfos->m_FontFileName.c_str());
										}*/
									}

									lastGlyphCodePoint = codePoint;
								}
							}
						}
					}
				}
			}

			SelectionHelper::Instance()->SelectWithToolOrApply(
				SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

			if (nameUpdated)
			{
				PrepareSelectionByFontOrderedByGlyphNames();
				PrepareSelectionMergedOrderedByGlyphNames();
			}

			if (codepointUpdated)
			{
				PrepareSelectionByFontOrderedByCodePoint();
				PrepareSelectionMergedOrderedByCodePoint();
			}
		}
}

void FinalFontPane::PrepareSelectionMergedOrderedByGlyphNames()
{
	m_GlyphsMergedOrderedByGlyphName.clear();

		for (auto itFont : ProjectFile::Instance()->m_Fonts)
		{
			for (auto& itGlyph : itFont.second->m_SelectedGlyphs)
			{
				if (itGlyph.second)
				{
					itGlyph.second->SetFontInfos(itFont.second);
					m_GlyphsMergedOrderedByGlyphName[itGlyph.second->newHeaderName].push_back(itGlyph.second);
				}
			}
		}
}

void FinalFontPane::DrawSelectionMergedOrderedByGlyphNames()
{
	if (m_GlyphsMergedOrderedByGlyphName.empty())
		return;

        ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(&cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphCodePoint = 0;
			m_GlyphButtonStateColor[0] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			bool showRangeColoring = ProjectFile::Instance()->IsRangeColoringShown();

			bool nameUpdated = false;
			bool codepointUpdated = false;

			for (auto& itGlyph : m_GlyphsMergedOrderedByGlyphName)
			{
				auto glyphVector = itGlyph.second;

				// si plus d'un glyph ici, alors deux glyph partagent le meme codepoint
				// et il va falloir le montrer
				// un rerange sera necesaire
				for (auto& glyphInfo : glyphVector)
				{
					auto fontInfos = glyphInfo->GetFontInfos();
					if (!fontInfos.expired() && glyphCountX)
					{
						auto fontInfosPtr = fontInfos.lock();
						if (fontInfosPtr.use_count())
						{
							if (fontInfosPtr->m_ImFontAtlas.IsBuilt())
							{
								if (fontInfosPtr->m_ImFontAtlas.TexID)
								{
									uint32_t x = idx++ % glyphCountX;

									if (x) ImGui::SameLine();

									GlyphInfos::GetGlyphButtonColorsForCodePoint(showRangeColoring,
										glyphInfo->newCodePoint, lastGlyphCodePoint, m_GlyphButtonStateColor);

									DrawGlyph(fontInfosPtr,
										glyph_size, glyphInfo, glyphVector.size() > 1, m_GlyphButtonStateColor,
										&nameUpdated, &codepointUpdated);

									if (ProjectFile::Instance()->m_FinalPane_ShowGlyphTooltip)
									{
										DrawGlyphInfosToolTip(fontInfosPtr, glyphInfo);

										/*if (ImGui::IsItemHovered())
										{
											ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
												glyphInfo->newHeaderName.c_str(),
												(int)glyphInfo->newCodePoint,
												glyphInfo->oldHeaderName.c_str(),
												(int)glyphInfo->glyph.Codepoint,
												vFontInfos->m_FontFileName.c_str());
										}*/
									}

									lastGlyphCodePoint = glyphInfo->newCodePoint;
								}
							}
						}
					}
				}
			}

			SelectionHelper::Instance()->SelectWithToolOrApply(
				SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

			if (nameUpdated)
			{
				PrepareSelectionByFontOrderedByGlyphNames();
				PrepareSelectionMergedOrderedByGlyphNames();
			}

			if (codepointUpdated)
			{
				PrepareSelectionByFontOrderedByCodePoint();
				PrepareSelectionMergedOrderedByCodePoint();
			}
		}
}

std::string FinalFontPane::getXml(const std::string& vOffset, const std::string& vUserDatas = "")
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

bool FinalFontPane::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "")
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

	if (strParentName == "finalfontpane")
	{
		/*if (strName == "glyphsizepolicy_count")
			m_Final_GlyphSize_Policy_Count = ct::ivariant(strValue).GetI();
		else if (strName == "glyphsizepolicy_width")
			m_Final_GlyphSize_Policy_Width = ct::fvariant(strValue).GetF();*/
	}

	if (strParentName == "selectedfontpane")
	{
		/*if (strName == "glyphsizepolicy_count")
			m_Selected_GlyphSize_Policy_Count = ct::ivariant(strValue).GetI();
		else if (strName == "glyphsizepolicy_width")
			m_Selected_GlyphSize_Policy_Width = ct::fvariant(strValue).GetF();*/
	}

	return true;
}