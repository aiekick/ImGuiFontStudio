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
#include <Gui/ImGuiWidgets.h>
#include <Panes/Manager/LayoutManager.h>
#include <Res/CustomFont.h>
#include <Helper/SelectionHelper.h>
#include <Panes/GlyphPane.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <cinttypes> // printf zu

static char glyphNameBuffer[512] = "\0";

FinalFontPane::FinalFontPane() = default;
FinalFontPane::~FinalFontPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FinalFontPane::Init()
{
	
}

void FinalFontPane::Unit()
{

}

int FinalFontPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	paneWidgetId = vWidgetId;

	DrawFinalFontPane(vProjectFile);
	DrawSelectedFontPane(vProjectFile);

	return paneWidgetId;
}

void FinalFontPane::DrawDialogsAndPopups(ProjectFile * /*vProjectFile*/)
{

}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FinalFontPane::DrawFinalFontPane(ProjectFile *vProjectFile)
{
	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_FINAL)
	{
		if (ImGui::Begin<PaneFlags>(FINAL_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_FINAL,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile && vProjectFile->IsLoaded())
			{
				if (!vProjectFile->m_Fonts.empty())
				{
					if (ImGui::BeginMenuBar())
					{
						if (ImGui::BeginMenu("View Mode"))
						{
							ImGui::MenuItem<FinalFontPaneModeFlags>("by Font, no order", "",
								&m_FinalFontPaneModeFlags, 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_NO_ORDER, true);

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("by Font, ordered by CodePoint", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT, true))
							{
								PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
							}

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("by Font, ordered by Name", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES, true))
							{
								PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
							}

							ImGui::Spacing();

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("Merged, no order", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_NO_ORDER, true))
							{
								PrepareSelectionMergedNoOrder(vProjectFile);
							}

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("Merged, ordered by CodePoint", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_CODEPOINT, true))
							{
								PrepareSelectionMergedOrderedByCodePoint(vProjectFile);
							}

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("Merged, ordered by Name", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_NAMES, true))
							{
								PrepareSelectionMergedOrderedByGlyphNames(vProjectFile);
							}

							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Infos"))
						{
							if (ImGui::MenuItem("Show Tooltip", "", &vProjectFile->m_FinalPane_ShowGlyphTooltip))
							{
								vProjectFile->SetProjectChange();
							}
							
							ImGui::EndMenu();
						}

						SelectionHelper::Instance()->DrawSelectionMenu(vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

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
						DrawSelectionsByFontNoOrder(vProjectFile, vProjectFile->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT)
					{
						DrawSelectionsByFontOrderedByCodePoint(vProjectFile, vProjectFile->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES)
					{
						DrawSelectionsByFontOrderedByGlyphNames(vProjectFile, vProjectFile->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_NO_ORDER)
					{
						DrawSelectionMergedNoOrder(vProjectFile);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_CODEPOINT)
					{
						DrawSelectionMergedOrderedByCodePoint(vProjectFile);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_NAMES)
					{
						DrawSelectionMergedOrderedByGlyphNames(vProjectFile);
					}
				}
			}
		}

		ImGui::End();
	}
}

void FinalFontPane::DrawSelectedFontPane(ProjectFile *vProjectFile)
{
	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_SELECTED_FONT)
	{
		if (ImGui::Begin<PaneFlags>(SELECTED_FONT_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_SELECTED_FONT,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				if (vProjectFile->m_SelectedFont)
				{
					if (ImGui::BeginMenuBar())
					{
						if (ImGui::BeginMenu("Sorting"))
						{
							if (ImGui::MenuItem<SelectedFontPaneModeFlags>("by CodePoint", "",
								&m_SelectedFontPaneModeFlags,
								SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT, true))
							{
								PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
							}

							if (ImGui::MenuItem<SelectedFontPaneModeFlags>("by Name", "",
								&m_SelectedFontPaneModeFlags,
								SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES, true))
							{
								PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
							}

							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Infos"))
						{
							if (ImGui::MenuItem("Show Tooltip", "", &vProjectFile->m_CurrentPane_ShowGlyphTooltip))
							{
								vProjectFile->SetProjectChange();
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
						DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(vProjectFile, vProjectFile->m_SelectedFont, false, true, false, vProjectFile->m_CurrentPane_ShowGlyphTooltip);
					}
					else if (m_SelectedFontPaneModeFlags & SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES)
					{
						DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(vProjectFile, vProjectFile->m_SelectedFont, false, true, false, vProjectFile->m_CurrentPane_ShowGlyphTooltip);
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

void FinalFontPane::SetFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags)
{
	m_FinalFontPaneModeFlags = (FinalFontPaneModeFlags)(m_FinalFontPaneModeFlags | vFinalFontPaneModeFlags);
}

bool FinalFontPane::IsFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags)
{
	return (m_FinalFontPaneModeFlags & vFinalFontPaneModeFlags);
}

bool FinalFontPane::IsSelectedFontPaneMode(SelectedFontPaneModeFlags vSelectedFontPaneModeFlags)
{
	return (m_SelectedFontPaneModeFlags & vSelectedFontPaneModeFlags);
}

void FinalFontPane::PrepareSelection(ProjectFile *vProjectFile)
{
	if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT))
	{
		PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
	}
	else if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES))
	{
		PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
	}

	if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_NO_ORDER))
	{
		// nothing to prepare becasue this is the default view => pointed on FontInfos->m_SelectedGlyphs
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT))
	{
		PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES))
	{
		PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_NO_ORDER))
	{
		PrepareSelectionMergedNoOrder(vProjectFile);
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_CODEPOINT))
	{
		PrepareSelectionMergedOrderedByCodePoint(vProjectFile);
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_NAMES))
	{
		PrepareSelectionMergedOrderedByGlyphNames(vProjectFile);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
////// PRIVATE ////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool FinalFontPane::DrawGlyph(ProjectFile *vProjectFile, 
	std::shared_ptr<FontInfos> vFontInfos, const ImVec2& vSize,
	GlyphInfos *vGlyph, bool vShowRect, 
	bool *vNameupdated, bool *vCodePointUpdated, 
	bool vForceEditMode) const
{
	bool res = false;
	
	if (vGlyph)
	{
		ImGui::PushID(vGlyph);
		ImGui::BeginGroup();
		
		bool selected = false;
		SelectionHelper::Instance()->IsGlyphIntersectedAndSelected(
			vFontInfos, vSize, vGlyph->glyph.Codepoint, &selected, 
			SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

		res = ImGui::ImageCheckButton(
			vFontInfos->m_ImFontAtlas.TexID, 
			&selected, vSize,
			ImVec2(vGlyph->glyph.U0, vGlyph->glyph.V0),
			ImVec2(vGlyph->glyph.U1, vGlyph->glyph.V1),
			ImVec2((float)vFontInfos->m_ImFontAtlas.TexWidth, 
				(float)vFontInfos->m_ImFontAtlas.TexHeight), 
			-1, vShowRect ? 3.0f : 0.0f);
		
		if (res)
		{
			if (vForceEditMode)
			{
				GlyphPane::Instance()->LoadGlyph(vProjectFile, vFontInfos, vGlyph);
			}
			else
			{
				SelectionHelper::Instance()->SelectWithToolOrApplyOnGlyph(
					vProjectFile, vFontInfos,
					vGlyph->glyph, 0, selected, true,
					SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
			}
		}

		if (m_GlyphEdition || vForceEditMode)
		{
			ImGui::SameLine();

			ImGui::BeginGroup();

			bool displayResetHeaderNameBtn = (vGlyph->newHeaderName != vGlyph->oldHeaderName);
			if (!vGlyph->m_editingName)
			{
				vGlyph->editHeaderName = vGlyph->newHeaderName;
				ImGui::PushItemWidth(
					GLYPH_EDIT_CONTROl_WIDTH -
					(displayResetHeaderNameBtn ? ImGui::GetFrameHeight() : 0.0f)
				);
			}
			else
			{
				float x = ImGui::GetCursorScreenPos().x;
				if (ImGui::Button(ICON_IGFS_OK "##okname"))
				{
					if (vNameupdated)
						*vNameupdated = true;
					vGlyph->newHeaderName = vGlyph->editHeaderName;
					SelectionHelper::Instance()->AnalyseSourceSelection(vProjectFile);
					vProjectFile->SetProjectChange();
					vGlyph->m_editingName = false;
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_IGFS_CANCEL "##cancelname"))
				{
					vGlyph->m_editingName = false;
				}
				ImGui::SameLine();
				ImGui::PushItemWidth(
					GLYPH_EDIT_CONTROl_WIDTH - 
					ImGui::GetCursorScreenPos().x + x -
					(displayResetHeaderNameBtn ? ImGui::GetFrameHeight() : 0.0f)
				);
			}
			snprintf(glyphNameBuffer, 511, "%s", vGlyph->editHeaderName.c_str());
			bool nameChanged = ImGui::InputText("##glyphname", glyphNameBuffer, 512);
			ImGui::PopItemWidth();
			if (nameChanged)
			{
				vGlyph->editHeaderName = std::string(glyphNameBuffer);
				vGlyph->m_editingName = true;
			}
			if (displayResetHeaderNameBtn)
			{
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetFrameHeight());
				if (ImGui::Button("R##resetname"))
				{
					if (vNameupdated)
						*vNameupdated = true;
					vGlyph->newHeaderName = vGlyph->oldHeaderName;
					SelectionHelper::Instance()->AnalyseSourceSelection(vProjectFile);
					vProjectFile->SetProjectChange();
				}
				ImGui::PopItemWidth();
			}

			bool displayResetCodePointBtn = ((uint32_t)vGlyph->newCodePoint != vGlyph->glyph.Codepoint);
			if (!vGlyph->m_editingCodePoint)
			{
				vGlyph->editCodePoint = vGlyph->newCodePoint;
				ImGui::PushItemWidth(
					GLYPH_EDIT_CONTROl_WIDTH -
					(displayResetCodePointBtn ? ImGui::GetFrameHeight() : 0.0f)
				);
			}
			else 
			{
				float x = ImGui::GetCursorScreenPos().x;
				bool btn = m_AutoUpdateCodepoint_WhenEditWithButtons;
				if (!btn)
					btn = ImGui::Button(ICON_IGFS_OK "##okcodepoint");
				if (btn)
				{
					if (vCodePointUpdated)
						*vCodePointUpdated = true;
					vGlyph->newCodePoint = (uint32_t)vGlyph->editCodePoint;// range 0 => 2^16;
					SelectionHelper::Instance()->AnalyseSourceSelection(vProjectFile);
					vProjectFile->SetProjectChange();
					vGlyph->m_editingCodePoint = false;
				}
				if (!m_AutoUpdateCodepoint_WhenEditWithButtons)
				{
					ImGui::SameLine();
					if (ImGui::Button(ICON_IGFS_CANCEL "##cancelcodepoint"))
					{
						vGlyph->m_editingCodePoint = false;
					}
					ImGui::SameLine();
					ImGui::PushItemWidth(
						GLYPH_EDIT_CONTROl_WIDTH - 
						ImGui::GetCursorScreenPos().x + x -
						(displayResetCodePointBtn ? ImGui::GetFrameHeight() : 0.0f)
					);
				}
				else
				{
					ImGui::PushItemWidth(
						GLYPH_EDIT_CONTROl_WIDTH -
						(displayResetCodePointBtn ? ImGui::GetFrameHeight() : 0.0f)
					);
				}
			}
			bool codePointChanged = ImGui::InputInt("##glyphnewcodepoint", &vGlyph->editCodePoint, 1, 10);
			ImGui::PopItemWidth();
			if (codePointChanged)
			{
				vGlyph->editCodePoint = ct::clamp<int>(vGlyph->editCodePoint, 0, 65535);
				vGlyph->m_editingCodePoint = true;
			}
			if (displayResetCodePointBtn)
			{
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetFrameHeight());
				if (ImGui::Button("R##resetcodepoint"))
				{
					if (vCodePointUpdated)
						*vCodePointUpdated = true;
					vGlyph->newCodePoint = vGlyph->glyph.Codepoint;
					SelectionHelper::Instance()->AnalyseSourceSelection(vProjectFile);
					vProjectFile->SetProjectChange();
				}
				ImGui::PopItemWidth();
			}
			ImGui::EndGroup();
		}

		ImGui::EndGroup();
		ImGui::PopID();
	}
	
	return res;
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontNoOrder(ProjectFile *vProjectFile, bool vShowTooltipInfos)
{
	if (vProjectFile)
	{
		for (auto & it : vProjectFile->m_Fonts)
		{
			DrawSelectionsByFontNoOrder_OneFontOnly(vProjectFile, it.second, true, false, false, vShowTooltipInfos);

			if (vProjectFile->m_Fonts.size() > 1)
				ImGui::Separator();
		}
		
		SelectionHelper::Instance()->SelectWithToolOrApply(
			vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
	}
}

static inline void DrawGlyphInfosToolTip(std::shared_ptr<FontInfos> vFontInfos, GlyphInfos *vGlyphInfos)
{
	if (ImGui::IsItemHovered())
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
	ProjectFile *vProjectFile,
	std::shared_ptr<FontInfos> vFontInfos,
	bool vWithFramedGroup,
	bool vForceEditMode,
	bool vForceEditModeOneColumn,
	bool vShowTooltipInfos)
{
	if (vProjectFile && vFontInfos)
	{
		if (vFontInfos->m_SelectedGlyphs.empty())
			return;

		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				uint32_t startCodePoint = vFontInfos->m_SelectedGlyphs.begin()->second.glyph.Codepoint;

				char buffer[1024] = "\0";
				snprintf(buffer, 1023, "Font %s / Start CodePoint %u / Count %u",
					vFontInfos->m_FontFileName.c_str(), startCodePoint,
					(uint32_t)vFontInfos->m_SelectedGlyphs.size());
				bool frm = true;
				if (vWithFramedGroup)
					frm = ImGui::BeginFramedGroup(buffer);
				if (frm)
				{
                    ImVec2 cell_size, glyph_size;
					uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size, m_GlyphEdition, vForceEditMode, vForceEditModeOneColumn);
					if (glyphCountX)
					{
						uint32_t idx = 0;
						uint32_t lastGlyphCodePoint = 0;
						ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
						bool showRangeColoring = vProjectFile->IsRangeColoringShown();

						bool nameUpdated = false;
						bool codepointUpdated = false;

						for (auto& it : vFontInfos->m_SelectedGlyphs)
						{
							uint32_t x = idx % glyphCountX;

							uint32_t codePoint = it.first;
							auto glyphInfo = &it.second;

							if (x) ImGui::SameLine();

							if (showRangeColoring)
							{
								if (codePoint != lastGlyphCodePoint + 1)
								{
									glyphRangeColoring = vProjectFile->GetColorFromInteger(codePoint);
								}

								ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
								ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
								ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
							}

							DrawGlyph(vProjectFile, vFontInfos,
								glyph_size, glyphInfo, false,
								&nameUpdated, &codepointUpdated, vForceEditMode);

							if (showRangeColoring)
							{
								ImGui::PopStyleColor(3);
							}

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
							PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
							PrepareSelectionMergedOrderedByGlyphNames(vProjectFile);
						}

						// update for other views
						if (codepointUpdated)
						{
							PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
							PrepareSelectionMergedOrderedByCodePoint(vProjectFile);
						}
					}

					if (vWithFramedGroup)
						ImGui::EndFramedGroup();
				}
			}
		}
	}
}

void FinalFontPane::PrepareSelectionByFontOrderedByCodePoint(ProjectFile *vProjectFile)
{
	if (vProjectFile)
	{
		for (auto & itFont : vProjectFile->m_Fonts)
		{
			itFont.second->m_GlyphsOrderedByCodePoints.clear();

			auto glyphs = itFont.second;
			for (auto &itGlyph : glyphs->m_SelectedGlyphs)
			{
				uint32_t codePoint = itGlyph.second.newCodePoint;
				auto glyphInfo = &itGlyph.second;

				glyphInfo->fontAtlas = itFont.second;

				itFont.second->m_GlyphsOrderedByCodePoints[codePoint].push_back(glyphInfo);
			}
		}
	}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByCodePoint(ProjectFile *vProjectFile, bool vShowTooltipInfos)
{
	if (vProjectFile)
	{
		for (auto & it : vProjectFile->m_Fonts)
		{
			DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(vProjectFile, it.second, true, false, false, vShowTooltipInfos);

			if (vProjectFile->m_Fonts.size() > 1)
				ImGui::Separator();
		}

		SelectionHelper::Instance()->SelectWithToolOrApply(
			vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
	}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(
	ProjectFile *vProjectFile, 
	std::shared_ptr<FontInfos> vFontInfos,
	bool vWithFramedGroup, 
	bool vForceEditMode, 
	bool vForceEditModeOneColumn,
	bool vShowTooltipInfos)
{
	if (vProjectFile && vFontInfos)
	{
		if (vFontInfos->m_GlyphsOrderedByCodePoints.empty())
			return;

		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				if (vFontInfos->m_GlyphsOrderedByCodePoints.empty())
				{
					PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
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
						frm = ImGui::BeginFramedGroup(buffer);
					if (frm)
					{
						ImVec2 cell_size, glyph_size;
						uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size, m_GlyphEdition, vForceEditMode, vForceEditModeOneColumn);
						if (glyphCountX)
						{
                            uint32_t idx = 0;
                            uint32_t lastGlyphCodePoint = 0;
							ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
							bool showRangeColoring = vProjectFile->IsRangeColoringShown();

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

									if (showRangeColoring)
									{
										if (glyphInfo->newCodePoint != lastGlyphCodePoint + 1)
										{
											glyphRangeColoring = vProjectFile->GetColorFromInteger(glyphInfo->newCodePoint);
										}

										ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
										ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
										ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
										ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
										ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
									}

									DrawGlyph(vProjectFile, vFontInfos,
										glyph_size, glyphInfo, glyphVector.size() > 1,
										&nameUpdated, &codepointUpdated, vForceEditMode);

									if (showRangeColoring)
									{
										ImGui::PopStyleColor(3);
									}

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
								PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
								PrepareSelectionMergedOrderedByGlyphNames(vProjectFile);
							}

							if (codepointUpdated)
							{
								PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
								PrepareSelectionMergedOrderedByCodePoint(vProjectFile);
							}
						}

						if (vWithFramedGroup)
							ImGui::EndFramedGroup();
					}
				}
			}
		}
	}
}

void FinalFontPane::PrepareSelectionByFontOrderedByGlyphNames(ProjectFile *vProjectFile)
{
	if (vProjectFile)
	{
		for (auto & itFont : vProjectFile->m_Fonts)
		{
			itFont.second->m_GlyphsOrderedByGlyphName.clear();

			auto glyphs = itFont.second;
			for (auto &itGlyph : glyphs->m_SelectedGlyphs)
			{
				auto glyphInfo = &itGlyph.second;

				glyphInfo->fontAtlas = itFont.second;

				itFont.second->m_GlyphsOrderedByGlyphName[glyphInfo->newHeaderName].push_back(glyphInfo);
			}
		}
	}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByGlyphNames(ProjectFile *vProjectFile, bool vShowTooltipInfos)
{
	if (vProjectFile)
	{
		for (auto & it : vProjectFile->m_Fonts)
		{
			DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(vProjectFile, it.second, true, false, false, vShowTooltipInfos);

			if (vProjectFile->m_Fonts.size() > 1)
				ImGui::Separator();
		}

		SelectionHelper::Instance()->SelectWithToolOrApply(
			vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
	}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(
	ProjectFile *vProjectFile, 
	std::shared_ptr<FontInfos> vFontInfos,
	bool vWithFramedGroup, 
	bool vForceEditMode, 
	bool vForceEditModeOneColumn,
	bool vShowTooltipInfos)
{
	if (vFontInfos->m_GlyphCodePointToName.empty())
		return;

	if (vProjectFile && vFontInfos)
	{
		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				if (vFontInfos->m_GlyphsOrderedByGlyphName.empty())
				{
					PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
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
						frm = ImGui::BeginFramedGroup(buffer);
					if (frm)
					{
						ImVec2 cell_size2, glyph_size;
						uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size2, &glyph_size, m_GlyphEdition, vForceEditMode, vForceEditModeOneColumn);
						if (glyphCountX)
						{
							int idx = 0;
                            uint32_t lastGlyphCodePoint = 0;
							ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
							bool showRangeColoring = vProjectFile->IsRangeColoringShown();

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

									if (showRangeColoring)
									{
										if (glyphInfo->newCodePoint != lastGlyphCodePoint + 1)
										{
											glyphRangeColoring = vProjectFile->GetColorFromInteger(glyphInfo->newCodePoint);
										}

										ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
										ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
										ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
										ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
										ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
									}

									DrawGlyph(vProjectFile, vFontInfos,
										glyph_size, glyphInfo, glyphVector.size() > 1,
										&nameUpdated, &codepointUpdated, vForceEditMode);

									if (showRangeColoring)
									{
										ImGui::PopStyleColor(3);
									}

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
								PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
								PrepareSelectionMergedOrderedByGlyphNames(vProjectFile);
							}

							if (codepointUpdated)
							{
								PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
								PrepareSelectionMergedOrderedByCodePoint(vProjectFile);
							}
						}

						if (vWithFramedGroup)
							ImGui::EndFramedGroup();
					}
				}
			}
		}
	}
}

void FinalFontPane::PrepareSelectionMergedNoOrder(ProjectFile *vProjectFile)
{
	m_GlyphsMergedNoOrder.clear();

	if (vProjectFile)
	{
		for (auto & itFont : vProjectFile->m_Fonts)
		{
			auto glyphs = itFont.second;
			for (auto &itGlyph : glyphs->m_SelectedGlyphs)
			{
				auto glyphInfo = &itGlyph.second;

				glyphInfo->fontAtlas = itFont.second;

				m_GlyphsMergedNoOrder.push_back(glyphInfo);
			}
		}
	}
}

void FinalFontPane::DrawSelectionMergedNoOrder(ProjectFile *vProjectFile)
{
	if (m_GlyphsMergedNoOrder.empty())
		return;

	if (vProjectFile)
	{
        ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphCodePoint = 0;
			ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			bool showRangeColoring = vProjectFile->IsRangeColoringShown();

			bool nameUpdated = false;
			bool codepointUpdated = false;

			for (auto& glyphInfo : m_GlyphsMergedNoOrder)
			{
				std::shared_ptr<FontInfos> vFontInfos = glyphInfo->fontAtlas;

				if (vFontInfos && glyphCountX)
				{
					if (vFontInfos->m_ImFontAtlas.IsBuilt())
					{
						if (vFontInfos->m_ImFontAtlas.TexID)
						{
							uint32_t x = idx++ % glyphCountX;

							if (x) ImGui::SameLine();

							if (showRangeColoring)
							{
								if (glyphInfo->newCodePoint != lastGlyphCodePoint + 1)
								{
									glyphRangeColoring = vProjectFile->GetColorFromInteger(glyphInfo->newCodePoint);
								}

								ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
								ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
								ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
							}

							DrawGlyph(vProjectFile, vFontInfos,
								glyph_size, glyphInfo, false,
								&nameUpdated, &codepointUpdated);

							if (showRangeColoring)
							{
								ImGui::PopStyleColor(3);
							}

							if (vProjectFile->m_FinalPane_ShowGlyphTooltip)
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
				}
			}

			SelectionHelper::Instance()->SelectWithToolOrApply(
				vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

			if (nameUpdated)
			{
				PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
				PrepareSelectionMergedOrderedByGlyphNames(vProjectFile);
			}

			if (codepointUpdated)
			{
				PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
				PrepareSelectionMergedOrderedByCodePoint(vProjectFile);
			}
		}
	}
}

void FinalFontPane::PrepareSelectionMergedOrderedByCodePoint(ProjectFile *vProjectFile)
{
	m_GlyphsMergedOrderedByCodePoints.clear();

	if (vProjectFile)
	{
		for (auto & itFont : vProjectFile->m_Fonts)
		{
			auto glyphs = itFont.second;
			for (auto &itGlyph : glyphs->m_SelectedGlyphs)
			{
				uint32_t codePoint = itGlyph.second.newCodePoint;
				auto glyphInfo = &itGlyph.second;

				glyphInfo->fontAtlas = itFont.second;

				m_GlyphsMergedOrderedByCodePoints[codePoint].push_back(glyphInfo);
			}
		}
	}
}

void FinalFontPane::DrawSelectionMergedOrderedByCodePoint(ProjectFile *vProjectFile)
{
	if (m_GlyphsMergedOrderedByCodePoints.empty())
		return;

	if (vProjectFile)
	{
		ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphCodePoint = 0;
			ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			bool showRangeColoring = vProjectFile->IsRangeColoringShown();

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
					std::shared_ptr<FontInfos> vFontInfos = glyphInfo->fontAtlas;

					if (vFontInfos && glyphCountX)
					{
						if (vFontInfos->m_ImFontAtlas.IsBuilt())
						{
							if (vFontInfos->m_ImFontAtlas.TexID)
							{
								uint32_t x = idx++ % glyphCountX;

								if (x) ImGui::SameLine();

								if (showRangeColoring)
								{
									if (glyphInfo->newCodePoint != lastGlyphCodePoint + 1)
									{
										glyphRangeColoring = vProjectFile->GetColorFromInteger(glyphInfo->newCodePoint);
									}

									ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
									ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
									ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
								}

								DrawGlyph(vProjectFile, vFontInfos,
									glyph_size, glyphInfo, glyphVector.size() > 1,
									&nameUpdated, &codepointUpdated);

								if (showRangeColoring)
								{
									ImGui::PopStyleColor(3);
								}

								if (vProjectFile->m_FinalPane_ShowGlyphTooltip)
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
					}
				}
			}

			SelectionHelper::Instance()->SelectWithToolOrApply(
				vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

			if (nameUpdated)
			{
				PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
				PrepareSelectionMergedOrderedByGlyphNames(vProjectFile);
			}

			if (codepointUpdated)
			{
				PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
				PrepareSelectionMergedOrderedByCodePoint(vProjectFile);
			}
		}
	}
}

void FinalFontPane::PrepareSelectionMergedOrderedByGlyphNames(ProjectFile *vProjectFile)
{
	m_GlyphsMergedOrderedByGlyphName.clear();

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			for (auto itGlyph : itFont.second->m_SelectedGlyphs)
			{
				auto glyphInfo = &itGlyph.second;
				glyphInfo->fontAtlas = itFont.second;

				m_GlyphsMergedOrderedByGlyphName[glyphInfo->newHeaderName].push_back(glyphInfo);
			}
		}
	}
}

void FinalFontPane::DrawSelectionMergedOrderedByGlyphNames(ProjectFile *vProjectFile)
{
	if (m_GlyphsMergedOrderedByGlyphName.empty())
		return;

	if (vProjectFile)
	{
        ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphCodePoint = 0;
			ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			bool showRangeColoring = vProjectFile->IsRangeColoringShown();

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
					std::shared_ptr<FontInfos> vFontInfos = glyphInfo->fontAtlas;

					if (vFontInfos && glyphCountX)
					{
						if (vFontInfos->m_ImFontAtlas.IsBuilt())
						{
							if (vFontInfos->m_ImFontAtlas.TexID)
							{
								uint32_t x = idx++ % glyphCountX;

								if (x) ImGui::SameLine();

								if (showRangeColoring)
								{
									if (glyphInfo->newCodePoint != lastGlyphCodePoint + 1)
									{
										glyphRangeColoring = vProjectFile->GetColorFromInteger(glyphInfo->newCodePoint);
									}

									ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
									ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
									ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
								}

								DrawGlyph(vProjectFile, vFontInfos,
									glyph_size, glyphInfo, glyphVector.size() > 1,
									&nameUpdated, &codepointUpdated);

								if (showRangeColoring)
								{
									ImGui::PopStyleColor(3);
								}

								if (vProjectFile->m_FinalPane_ShowGlyphTooltip)
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
					}
				}
			}

			SelectionHelper::Instance()->SelectWithToolOrApply(
				vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

			if (nameUpdated)
			{
				PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
				PrepareSelectionMergedOrderedByGlyphNames(vProjectFile);
			}

			if (codepointUpdated)
			{
				PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
				PrepareSelectionMergedOrderedByCodePoint(vProjectFile);
			}
		}
	}
}

