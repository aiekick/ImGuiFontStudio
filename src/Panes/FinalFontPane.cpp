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
#include <Helper/Profiler.h>

#include <Helper/TranslationSystem.h>

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
	ZoneScoped;
}

void FinalFontPane::Unit()
{
	ZoneScoped;
}

int FinalFontPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	ZoneScoped;

	paneWidgetId = vWidgetId;

	DrawFinalFontPane(vProjectFile);
	DrawSelectedFontPane(vProjectFile);

	return paneWidgetId;
}

void FinalFontPane::DrawDialogsAndPopups(ProjectFile * /*vProjectFile*/)
{
	ZoneScoped;
}

int FinalFontPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	ZoneScoped;

	UNUSED(vProjectFile);
	UNUSED(vUserDatas);

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FinalFontPane::DrawFinalFontPane(ProjectFile *vProjectFile)
{
	ZoneScoped;

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
						if (ImGui::BeginMenu(TSLOC(FFP,ViewMode)))
						{
							ImGui::MenuItem<FinalFontPaneModeFlags>("by Font, ordered by GlyphIndex", "",
								&m_FinalFontPaneModeFlags, 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_GLYPHINDEX, true);

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

							if (ImGui::MenuItem<FinalFontPaneModeFlags>("Merged, ordered by GlyphIndex", "",
								&m_FinalFontPaneModeFlags , 
								FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_GLYPHINDEX, true))
							{
								PrepareSelectionMergedOrderedByGlyphIndex(vProjectFile);
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

					if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_GLYPHINDEX)
					{
						DrawSelectionsByFontOrderedByGlyphIndex(vProjectFile, vProjectFile->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT)
					{
						DrawSelectionsByFontOrderedByCodePoint(vProjectFile, vProjectFile->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES)
					{
						DrawSelectionsByFontOrderedByGlyphNames(vProjectFile, vProjectFile->m_FinalPane_ShowGlyphTooltip);
					}
					else if (m_FinalFontPaneModeFlags & FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_GLYPHINDEX)
					{
						DrawSelectionMergedOrderedByGlyphIndex(vProjectFile);
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
	ZoneScoped;

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
				if (vProjectFile->m_SelectedFont.use_count())
				{
					if (vProjectFile->m_SelectedFont->IsUsable())
					{
						if (ImGui::BeginMenuBar())
						{
							if (ImGui::BeginMenu("Sorting"))
							{
								if (ImGui::MenuItem<SelectedFontPaneModeFlags>("by GlyphIndex", "",
									&m_SelectedFontPaneModeFlags,
									SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_GLYPHINDEX, true))
								{
									PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
								}
								
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

						if (m_SelectedFontPaneModeFlags & SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_GLYPHINDEX)
						{
							DrawSelectionsByFontOrderedByGlyphIndex_OneFontOnly(vProjectFile, vProjectFile->m_SelectedFont, false, true, false, vProjectFile->m_CurrentPane_ShowGlyphTooltip);
						}
						else if (m_SelectedFontPaneModeFlags & SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT)
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
		}

		ImGui::End();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
////// PUBLIC : PREPARATON ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void FinalFontPane::SetFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags)
{
	ZoneScoped;

	m_FinalFontPaneModeFlags = (FinalFontPaneModeFlags)(vFinalFontPaneModeFlags); /// set
}

bool FinalFontPane::IsFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags)
{
	ZoneScoped;

	return (m_FinalFontPaneModeFlags & vFinalFontPaneModeFlags) == vFinalFontPaneModeFlags; // check
}

bool FinalFontPane::IsSelectedFontPaneMode(SelectedFontPaneModeFlags vSelectedFontPaneModeFlags)
{
	ZoneScoped;

	return (m_SelectedFontPaneModeFlags & vSelectedFontPaneModeFlags) == vSelectedFontPaneModeFlags; // check
}

void FinalFontPane::PrepareSelection(ProjectFile *vProjectFile)
{
	ZoneScoped;

	if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_GLYPHINDEX))
	{
		// nothing to prepare because this is the default view => pointed on FontInfos->m_SelectedGlyphs
	}
	else if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT))
	{
		PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
	}
	else if (IsSelectedFontPaneMode(SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES))
	{
		PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
	}

	if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_GLYPHINDEX))
	{
		// nothing to prepare because this is the default view => pointed on FontInfos->m_SelectedGlyphs
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT))
	{
		PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES))
	{
		PrepareSelectionByFontOrderedByGlyphNames(vProjectFile);
	}
	else if (IsFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_GLYPHINDEX))
	{
		PrepareSelectionMergedOrderedByGlyphIndex(vProjectFile);
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
	std::shared_ptr<GlyphInfos> vGlyph, bool vShowRect,
	bool *vNameupdated, bool *vGlyphIndexUpdated, 
	bool vForceEditMode)
{
	ZoneScoped;

	int res = false;
	
	if (vFontInfos.use_count() && vGlyph.use_count())
	{
		if (vFontInfos->IsUsable())
		{
			ImGui::PushID(vGlyph.get());
			ImGui::BeginGroup();

			bool selected = false;
			SelectionHelper::Instance()->IsGlyphIntersectedAndSelected(
				vFontInfos, vSize, vGlyph->glyph.glyphIndex, &selected,
				SelectionContainerEnum::SELECTION_CONTAINER_FINAL);

			ct::fvec2 trans = vGlyph->m_Translation * vFontInfos->m_Point;
			ct::fvec2 scale = vGlyph->m_Scale;
			res = GlyphInfos::DrawGlyphButton(
				paneWidgetId,
				vProjectFile, vFontInfos,
				&selected, vSize, &vGlyph->glyph, -1,
				ImVec2(trans.x, trans.y), ImVec2(scale.x, scale.y),
				-1, vShowRect ? 3.0f : 0.0f);

			if (res)
			{
				// left button : check == 1
				// right button  : check == 2
				if (res == 2) // put glyph on glyph pane
				{
					GlyphPane::Instance()->LoadGlyph(vProjectFile, vFontInfos, vGlyph);
				}

				if (vForceEditMode)
				{
					GlyphPane::Instance()->LoadGlyph(vProjectFile, vFontInfos, vGlyph);
				}
				else
				{
					SelectionHelper::Instance()->SelectWithToolOrApplyOnGlyph(
						vProjectFile, vFontInfos,
						&vGlyph->glyph, 0, selected, true,
						SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
				}
			}

			if (m_GlyphEdition || vForceEditMode)
			{
				ImGui::SameLine();

				ImGui::BeginGroup();

				bool displayResetHeaderNameBtn = (vGlyph->newHeaderName != vGlyph->glyph.name);
				if (!vGlyph->m_editingName)
				{
					vGlyph->editHeaderName = vGlyph->newHeaderName;
					ImGui::PushItemWidth(
						GLYPH_EDIT_CONTROL_WIDTH -
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
						GLYPH_EDIT_CONTROL_WIDTH -
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
						vGlyph->newHeaderName = vGlyph->glyph.name;
						SelectionHelper::Instance()->AnalyseSourceSelection(vProjectFile);
						vProjectFile->SetProjectChange();
					}
					ImGui::PopItemWidth();
				}

				bool displayResetGlyphIndexBtn = ((uint32_t)vGlyph->newCodePoint != vGlyph->glyph.glyphIndex);
				if (!vGlyph->m_editingGlyphIndex)
				{
					vGlyph->editGlyphIndex = vGlyph->newCodePoint;
					ImGui::PushItemWidth(
						GLYPH_EDIT_CONTROL_WIDTH -
						(displayResetGlyphIndexBtn ? ImGui::GetFrameHeight() : 0.0f)
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
						if (vGlyphIndexUpdated)
							*vGlyphIndexUpdated = true;
						vGlyph->newCodePoint = (uint32_t)vGlyph->editGlyphIndex;// range 0 => 2^16;
						SelectionHelper::Instance()->AnalyseSourceSelection(vProjectFile);
						vProjectFile->SetProjectChange();
						vGlyph->m_editingGlyphIndex = false;
					}
					if (!m_AutoUpdateCodepoint_WhenEditWithButtons)
					{
						ImGui::SameLine();
						if (ImGui::Button(ICON_IGFS_CANCEL "##cancelcodepoint"))
						{
							vGlyph->m_editingGlyphIndex = false;
						}
						ImGui::SameLine();
						ImGui::PushItemWidth(
							GLYPH_EDIT_CONTROL_WIDTH -
							ImGui::GetCursorScreenPos().x + x -
							(displayResetGlyphIndexBtn ? ImGui::GetFrameHeight() : 0.0f)
						);
					}
					else
					{
						ImGui::PushItemWidth(
							GLYPH_EDIT_CONTROL_WIDTH -
							(displayResetGlyphIndexBtn ? ImGui::GetFrameHeight() : 0.0f)
						);
					}
				}

				bool codePointChanged = ImGui::InputInt("##glyphnewcodepoint", &vGlyph->editGlyphIndex, 1, 10);
				ImGui::PopItemWidth();
				if (codePointChanged)
				{
					vGlyph->editGlyphIndex = ct::clamp<int>(vGlyph->editGlyphIndex, 0, 65535);
					vGlyph->m_editingGlyphIndex = true;
				}
				if (displayResetGlyphIndexBtn)
				{
					ImGui::SameLine();
					ImGui::PushItemWidth(ImGui::GetFrameHeight());
					if (ImGui::Button("R##resetcodepoint"))
					{
						if (vGlyphIndexUpdated)
							*vGlyphIndexUpdated = true;
						vGlyph->newCodePoint = vGlyph->glyph.glyphIndex;
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
	}
	
	return (res > 0);
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByGlyphIndex(ProjectFile *vProjectFile, bool vShowTooltipInfos)
{
	ZoneScoped;

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			DrawSelectionsByFontOrderedByGlyphIndex_OneFontOnly(vProjectFile, itFont.second, true, false, false, vShowTooltipInfos);
		}
		
		SelectionHelper::Instance()->SelectWithToolOrApply(
			vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
	}
}

static inline void DrawGlyphInfosToolTip(std::shared_ptr<FontInfos> vFontInfos, std::shared_ptr<GlyphInfos> vGlyphInfos)
{
	ZoneScoped;

	if (ImGui::IsItemHovered() && vFontInfos.use_count() && vGlyphInfos.use_count())
	{
		ImGui::SetTooltip("glyph index : %i\nnew name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s\nCount Layers : %u\nCount Parents : %u",
			vGlyphInfos->glyph.glyphIndex,
			vGlyphInfos->newHeaderName.c_str(),
			(int)vGlyphInfos->newCodePoint,
			vGlyphInfos->glyph.name.c_str(),
			(int)vGlyphInfos->glyph.glyphIndex,
			vFontInfos->m_FontFileName.c_str(),
			(uint32_t)vGlyphInfos->glyph.layers.size(),
			(uint32_t)vGlyphInfos->glyph.parents.size());
	}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByGlyphIndex_OneFontOnly(
	ProjectFile *vProjectFile,
	std::shared_ptr<FontInfos> vFontInfos,
	bool vWithFramedGroup,
	bool vForceEditMode,
	bool vForceEditModeOneColumn,
	bool vShowTooltipInfos)
{
	ZoneScoped;

	if (vProjectFile && vFontInfos.use_count())
	{
		if (vFontInfos->IsUsable())
		{
			if (vFontInfos->m_SelectedGlyphs.empty()) return;

			if (vFontInfos->m_ImFontAtlas.IsBuilt())
			{
				if (vFontInfos->m_ImFontAtlas.TexID)
				{
					if (vFontInfos->m_SelectedGlyphs.begin()->second)
					{
						uint32_t startGlyphIndex = vFontInfos->m_SelectedGlyphs.begin()->second->glyph.glyphIndex;

						char buffer[1024] = "\0";
						snprintf(buffer, 1023, "Font %s / Start GlyphIndex %u / Count %u",
							vFontInfos->m_FontFileName.c_str(), startGlyphIndex,
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
								uint32_t lastGlyphGlyphIndex = 0;
								ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
								bool showRangeColoring = vProjectFile->IsRangeColoringShown();

								bool nameUpdated = false;
								bool codepointUpdated = false;

								for (auto& it : vFontInfos->m_SelectedGlyphs)
								{
									uint32_t x = idx % glyphCountX;

									uint32_t codePoint = it.first;
									auto glyphInfo = it.second;

									if (x) ImGui::SameLine();

									if (showRangeColoring)
									{
										if (codePoint != lastGlyphGlyphIndex + 1)
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

									lastGlyphGlyphIndex = codePoint;

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
	}
}

void FinalFontPane::PrepareSelectionByFontOrderedByCodePoint(ProjectFile *vProjectFile)
{
	ZoneScoped;

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			if (itFont.second.use_count())
			{
				if (itFont.second->IsUsable())
				{
					itFont.second->m_GlyphsOrderedByGlyphIndex.clear();

					for (auto& itGlyph : itFont.second->m_SelectedGlyphs)
					{
						if (itGlyph.second)
						{
							itGlyph.second->SetFontInfos(itFont.second);
							itFont.second->m_GlyphsOrderedByGlyphIndex[itGlyph.second->newCodePoint].push_back(itGlyph.second);
						}
					}
				}
			}
		}
	}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByCodePoint(ProjectFile *vProjectFile, bool vShowTooltipInfos)
{
	ZoneScoped;

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(vProjectFile, itFont.second, true, false, false, vShowTooltipInfos);
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
	ZoneScoped;

	if (vProjectFile && vFontInfos.use_count())
	{
		if (!vFontInfos->IsUsable())	return;

			if (vFontInfos->m_GlyphsOrderedByGlyphIndex.empty())
			return;

		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				if (vFontInfos->m_GlyphsOrderedByGlyphIndex.empty())
				{
					PrepareSelectionByFontOrderedByCodePoint(vProjectFile);
				}
				else
				{
                    uint32_t startGlyphIndex = vFontInfos->m_GlyphsOrderedByGlyphIndex.begin()->second[0]->newCodePoint;

					char buffer[1024] = "\0";
					snprintf(buffer, 1023, "Font %s / Start GlyphIndex %u / Count %u",
						vFontInfos->m_FontFileName.c_str(), startGlyphIndex,
						(uint32_t)vFontInfos->m_GlyphsOrderedByGlyphIndex.size());
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
                            uint32_t lastGlyphGlyphIndex = 0;
							ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
							bool showRangeColoring = vProjectFile->IsRangeColoringShown();

							bool nameUpdated = false;
							bool codepointUpdated = false;

							for (auto &itGlyph : vFontInfos->m_GlyphsOrderedByGlyphIndex)
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
										if (glyphInfo->newCodePoint != lastGlyphGlyphIndex + 1)
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

										//if (ImGui::IsItemHovered())
										//{
										//	ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
										//		glyphInfo->newHeaderName.c_str(),
										//		(int)glyphInfo->newCodePoint,
										//		glyphInfo->oldHeaderName.c_str(),
										//		(int)glyphInfo->glyph.glyphIndex,
										//		vFontInfos->m_FontFileName.c_str());
										//}
									}

									lastGlyphGlyphIndex = codePoint;
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
	ZoneScoped;

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			if (itFont.second.use_count())
			{
				if (itFont.second->IsUsable())
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
		}
	}
}

// this func can be called by FinalFontPane et SelectedFontPane
// but these two panes have a specific flag for show the tooltip
// so we need to pass this flag in parameter
void FinalFontPane::DrawSelectionsByFontOrderedByGlyphNames(ProjectFile *vProjectFile, bool vShowTooltipInfos)
{
	ZoneScoped;

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(vProjectFile, itFont.second, true, false, false, vShowTooltipInfos);
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
	ZoneScoped;

	if (vProjectFile && vFontInfos.use_count())
	{
		if (vFontInfos->IsUsable())
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
								uint32_t lastGlyphGlyphIndex = 0;
								ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
								bool showRangeColoring = vProjectFile->IsRangeColoringShown();

								bool nameUpdated = false;
								bool codepointUpdated = false;

								for (auto& itGlyph : vFontInfos->m_GlyphsOrderedByGlyphName)
								{
									auto glyphVector = itGlyph.second;

									// si plus d'un glyph ici, alors deux glyph partagent le meme codepoint
									// et il va falloir le montrer
									// un rerange sera necesaire
									for (auto& glyphInfo : glyphVector)
									{
										uint32_t x = idx++ % glyphCountX;

										if (x) ImGui::SameLine();

										if (showRangeColoring)
										{
											if (glyphInfo->newCodePoint != lastGlyphGlyphIndex + 1)
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

											//if (ImGui::IsItemHovered())
											//{
											//	ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
											//		glyphInfo->newHeaderName.c_str(),
											//		(int)glyphInfo->newCodePoint,
											//		glyphInfo->oldHeaderName.c_str(),
											//		(int)glyphInfo->glyph.glyphIndex,
											//		vFontInfos->m_FontFileName.c_str());
											//}
										}

										lastGlyphGlyphIndex = glyphInfo->newCodePoint;
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
}

void FinalFontPane::PrepareSelectionMergedOrderedByGlyphIndex(ProjectFile *vProjectFile)
{
	ZoneScoped;

	m_GlyphsMergedOrderedByGlyphIndex.clear();

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			if (itFont.second.use_count())
			{
				if (itFont.second->IsUsable())
				{
					for (auto& itGlyph : itFont.second->m_SelectedGlyphs)
					{
						if (itGlyph.second)
						{
							itGlyph.second->SetFontInfos(itFont.second);
							m_GlyphsMergedOrderedByGlyphIndex.push_back(itGlyph.second);
						}
					}
				}
			}
		}
	}
}

void FinalFontPane::DrawSelectionMergedOrderedByGlyphIndex(ProjectFile *vProjectFile)
{
	ZoneScoped;

	if (m_GlyphsMergedOrderedByGlyphIndex.empty())
		return;

	if (vProjectFile)
	{
        ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphGlyphIndex = 0;
			ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			bool showRangeColoring = vProjectFile->IsRangeColoringShown();

			bool nameUpdated = false;
			bool codepointUpdated = false;

			for (auto& glyphInfo : m_GlyphsMergedOrderedByGlyphIndex)
			{
				auto fontInfos = glyphInfo->GetFontInfos();
				if (!fontInfos.expired() && glyphCountX)
				{
					auto fontInfosPtr = fontInfos.lock();
					if (fontInfosPtr.use_count())
					{
						if (fontInfosPtr->IsUsable())
						{
							if (fontInfosPtr->m_ImFontAtlas.IsBuilt())
							{
								if (fontInfosPtr->m_ImFontAtlas.TexID)
								{
									uint32_t x = idx++ % glyphCountX;

									if (x) ImGui::SameLine();

									if (showRangeColoring)
									{
										if (glyphInfo->newCodePoint != lastGlyphGlyphIndex + 1)
										{
											glyphRangeColoring = vProjectFile->GetColorFromInteger(glyphInfo->newCodePoint);
										}

										ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
										ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
										ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
										ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
										ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
									}

									DrawGlyph(vProjectFile, fontInfosPtr,
										glyph_size, glyphInfo, false,
										&nameUpdated, &codepointUpdated);

									if (showRangeColoring)
									{
										ImGui::PopStyleColor(3);
									}

									if (vProjectFile->m_FinalPane_ShowGlyphTooltip)
									{
										DrawGlyphInfosToolTip(fontInfosPtr, glyphInfo);

										/*if (ImGui::IsItemHovered())
										{
											ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
												glyphInfo->newHeaderName.c_str(),
												(int)glyphInfo->newCodePoint,
												glyphInfo->oldHeaderName.c_str(),
												(int)glyphInfo->glyph.glyphIndex,
												vFontInfos->m_FontFileName.c_str());
										}*/
									}

									lastGlyphGlyphIndex = glyphInfo->newCodePoint;
								}
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

void FinalFontPane::PrepareSelectionMergedOrderedByCodePoint(ProjectFile *vProjectFile)
{
	ZoneScoped;

	m_GlyphsMergedOrderedByCodePoint.clear();

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			if (vProjectFile->m_SelectedFont.use_count())
			{
				if (vProjectFile->m_SelectedFont->IsUsable())
				{
					for (auto& itGlyph : itFont.second->m_SelectedGlyphs)
					{
						if (itGlyph.second)
						{
							itGlyph.second->SetFontInfos(itFont.second);
							m_GlyphsMergedOrderedByCodePoint[itGlyph.second->newCodePoint].push_back(itGlyph.second);
						}
					}
				}
			}
		}
	}
}

void FinalFontPane::DrawSelectionMergedOrderedByCodePoint(ProjectFile *vProjectFile)
{
	ZoneScoped;

	if (m_GlyphsMergedOrderedByCodePoint.empty())
		return;

	if (vProjectFile)
	{
		ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphGlyphIndex = 0;
			ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			bool showRangeColoring = vProjectFile->IsRangeColoringShown();

			bool nameUpdated = false;
			bool codepointUpdated = false;

			for (auto& itGlyph : m_GlyphsMergedOrderedByCodePoint)
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
							if (fontInfosPtr->IsUsable())
							{
								if (fontInfosPtr->m_ImFontAtlas.IsBuilt())
								{
									if (fontInfosPtr->m_ImFontAtlas.TexID)
									{
										uint32_t x = idx++ % glyphCountX;

										if (x) ImGui::SameLine();

										if (showRangeColoring)
										{
											if (glyphInfo->newCodePoint != lastGlyphGlyphIndex + 1)
											{
												glyphRangeColoring = vProjectFile->GetColorFromInteger(glyphInfo->newCodePoint);
											}

											ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
											ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
											ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
											ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
											ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
										}

										DrawGlyph(vProjectFile, fontInfosPtr,
											glyph_size, glyphInfo, glyphVector.size() > 1,
											&nameUpdated, &codepointUpdated);

										if (showRangeColoring)
										{
											ImGui::PopStyleColor(3);
										}

										if (vProjectFile->m_FinalPane_ShowGlyphTooltip)
										{
											DrawGlyphInfosToolTip(fontInfosPtr, glyphInfo);

											/*if (ImGui::IsItemHovered())
											{
												ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
													glyphInfo->newHeaderName.c_str(),
													(int)glyphInfo->newCodePoint,
													glyphInfo->oldHeaderName.c_str(),
													(int)glyphInfo->glyph.glyphIndex,
													vFontInfos->m_FontFileName.c_str());
											}*/
										}

										lastGlyphGlyphIndex = codePoint;
									}
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
}

void FinalFontPane::PrepareSelectionMergedOrderedByGlyphNames(ProjectFile *vProjectFile)
{
	ZoneScoped;

	m_GlyphsMergedOrderedByGlyphName.clear();

	if (vProjectFile)
	{
		for (auto itFont : vProjectFile->m_Fonts)
		{
			if (itFont.second.use_count())
			{
				if (itFont.second->IsUsable())
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
		}
	}
}

void FinalFontPane::DrawSelectionMergedOrderedByGlyphNames(ProjectFile *vProjectFile)
{
	ZoneScoped;

	if (m_GlyphsMergedOrderedByGlyphName.empty())
		return;

	if (vProjectFile)
	{
        ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size, m_GlyphEdition);
		if (glyphCountX)
		{
			uint32_t idx = 0;
			uint32_t lastGlyphGlyphIndex = 0;
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
					auto fontInfos = glyphInfo->GetFontInfos();
					if (!fontInfos.expired() && glyphCountX)
					{
						auto fontInfosPtr = fontInfos.lock();
						if (fontInfosPtr.use_count())
						{
							if (fontInfosPtr->IsUsable())
							{
								if (fontInfosPtr->m_ImFontAtlas.IsBuilt())
								{
									if (fontInfosPtr->m_ImFontAtlas.TexID)
									{
										uint32_t x = idx++ % glyphCountX;

										if (x) ImGui::SameLine();

										if (showRangeColoring)
										{
											if (glyphInfo->newCodePoint != lastGlyphGlyphIndex + 1)
											{
												glyphRangeColoring = vProjectFile->GetColorFromInteger(glyphInfo->newCodePoint);
											}

											ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
											ImVec4 bh = glyphRangeColoring; bh.w = 0.75f;
											ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
											ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
											ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
										}

										DrawGlyph(vProjectFile, fontInfosPtr,
											glyph_size, glyphInfo, glyphVector.size() > 1,
											&nameUpdated, &codepointUpdated);

										if (showRangeColoring)
										{
											ImGui::PopStyleColor(3);
										}

										if (vProjectFile->m_FinalPane_ShowGlyphTooltip)
										{
											DrawGlyphInfosToolTip(fontInfosPtr, glyphInfo);

											/*if (ImGui::IsItemHovered())
											{
												ImGui::SetTooltip("new name : %s\nnew codepoint : %i\nold name : %s\nold codepoint : %i\nfont : %s",
													glyphInfo->newHeaderName.c_str(),
													(int)glyphInfo->newCodePoint,
													glyphInfo->oldHeaderName.c_str(),
													(int)glyphInfo->glyph.glyphIndex,
													vFontInfos->m_FontFileName.c_str());
											}*/
										}

										lastGlyphGlyphIndex = glyphInfo->newCodePoint;
									}
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
}

std::string FinalFontPane::getXml(const std::string& vOffset, const std::string& vUserDatas = "")
{
	ZoneScoped;

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
	ZoneScoped;

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