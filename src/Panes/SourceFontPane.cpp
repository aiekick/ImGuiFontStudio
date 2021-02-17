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
#include "SourceFontPane.h"

#include <ctools/FileHelper.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <MainFrame.h>
#include <Helper/SelectionHelper.h>
#include <Panes/FinalFontPane.h>
#include <Panes/Manager/LayoutManager.h>
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>
#include <Project/GlyphInfos.h>
#include <Panes/GlyphPane.h>
#include <Helper/Profiler.h>
#include <Panes/ParamsPane.h>

#include <cinttypes> // printf zu

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

static ProjectFile defaultProjectValues;
static FontInfos defaultFontInfosValues;

SourceFontPane::SourceFontPane() = default;
SourceFontPane::~SourceFontPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SourceFontPane::Init()
{
	ZoneScoped;
}

void SourceFontPane::Unit()
{
	ZoneScoped;
}

int SourceFontPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	ZoneScoped;

	paneWidgetId = vWidgetId;

	DrawSourceFontPane(vProjectFile);
	
	return paneWidgetId;
}

void SourceFontPane::DrawDialogsAndPopups(ProjectFile * vProjectFile)
{
	ZoneScoped;

	UNUSED(vProjectFile);
}

int SourceFontPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	ZoneScoped;

	UNUSED(vUserDatas);

	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (LayoutManager::Instance()->IsSpecificPaneFocused(PaneFlags::PANE_SOURCE))
		{
			if (vProjectFile->m_SelectedFont.use_count())
			{
				if (vProjectFile->m_SelectedFont->IsUsable())
				{
					vProjectFile->m_SelectedFont->DrawInfos(vProjectFile);

					vProjectFile->m_SelectedFont->DrawFilteringWidgets(vProjectFile);

					if (ImGui::BeginFramedGroup("Glyph / Texture"))
					{
						const float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x;
						const float mrw = maxWidth * 0.5f;

						bool change = false;

						change |= ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(
							mrw,
							ICON_IGFS_GLYPHS " Glyphs", "Show Font Glyphs",
							&vProjectFile->m_SourceFontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH, true);

						ImGui::SameLine();

						change |= ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(
							mrw,
							ICON_IGFS_TEXTURE " Texture", "Show Font Texture",
							&vProjectFile->m_SourceFontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE, true);

						if (change)
						{
							vProjectFile->SetProjectChange();
						}

						ImGui::EndFramedGroup(true);
					}
				}
			}
		}
	}

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : PANES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SourceFontPane::DrawSourceFontPane(ProjectFile *vProjectFile)
{
	ZoneScoped;

	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_SOURCE)
	{
		if (ImGui::Begin<PaneFlags>(SOURCE_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_SOURCE,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile && vProjectFile->IsLoaded())
			{
				if (vProjectFile->m_SelectedFont)
				{
					if (vProjectFile->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
					{
						if (ImGui::BeginMenuBar())
						{
							if (ImGui::BeginMenu("Infos"))
							{
								if (ImGui::MenuItem("Show Tooltip", "", &vProjectFile->m_SourcePane_ShowGlyphTooltip))
								{
									vProjectFile->SetProjectChange();
								}

								ImGui::EndMenu();
							}

							ImGui::Spacing();

							SelectionHelper::Instance()->DrawSelectionMenu(vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

							ImGui::Spacing();
							
							if (vProjectFile->m_Preview_Glyph_CountX)
							{
								DrawFilterBar(vProjectFile, vProjectFile->m_SelectedFont);
							}

							ImGui::EndMenuBar();
						}
						
						DrawFontAtlas_Virtual(vProjectFile, vProjectFile->m_SelectedFont);
					}
					else if (vProjectFile->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE)
					{
						DrawFontTexture(vProjectFile->m_SelectedFont);
					}
				}
			}
		}

		ImGui::End();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : WIDGETS, VIEW //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SourceFontPane::DrawFilterBar(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos)
{
	ZoneScoped;

	if (vProjectFile && vFontInfos.use_count())
	{
		ImGui::PushID(vFontInfos.get());
		ImGui::Text("Filters");
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("will search for any filter separated\n\tby a coma ',' or a space ' '");
		if (ImGui::MenuItem(ICON_IGFS_DESTROY "##clearFilter"))
		{
			ct::ResetBuffer(vFontInfos->m_SearchBuffer);
			vFontInfos->m_Filters.clear();
			vProjectFile->SetProjectChange();
			vFontInfos->UpdateFiltering();
		}
		ImGui::PushItemWidth(400);
		bool filterChanged = ImGui::InputText("##Filter", vFontInfos->m_SearchBuffer, 1023);
		ImGui::PopItemWidth();
		ImGui::PopID();
		if (filterChanged)
		{
			vFontInfos->m_Filters.clear();
			std::string s = vFontInfos->m_SearchBuffer;
			auto arr = ct::splitStringToVector(s, ", ");
			for (const auto &it : arr)
			{
				vFontInfos->m_Filters.insert(it);
			}
			vProjectFile->SetProjectChange();
			vFontInfos->UpdateFiltering();
		}
	}
}

void SourceFontPane::DrawFontAtlas_Virtual(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos)
{
	ZoneScoped;

	auto win = ImGui::GetCurrentWindowRead();
	if (win)
	{
		win->DrawList->ChannelsSplit(2);

		if (vProjectFile && vProjectFile->IsLoaded() &&	vFontInfos.use_count())
		{
			if (vFontInfos->IsUsable()) // outside of thread
			{
				vProjectFile->m_Preview_Glyph_CountX = ct::maxi(vProjectFile->m_Preview_Glyph_CountX, 1);

				if (vFontInfos->m_ImFontAtlas.IsBuilt())
				{
					if (vFontInfos->m_ImFontAtlas.TexID)
					{
						auto imFont = vFontInfos->GetImFontPtr();
						if (imFont && !vFontInfos->m_FilteredGlyphs.empty())
						{
							ImVec2 hostTextureSize = ImVec2(
								(float)vFontInfos->m_ImFontAtlas.TexWidth,
								(float)vFontInfos->m_ImFontAtlas.TexHeight);
							ImVec2 cell_size, glyph_size;
							uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);
							if (glyphCountX)
							{
								uint32_t idx = 0, lastGlyphCodePoint = 0;
								ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
								const bool showRangeColoring = vProjectFile->IsRangeColoringShown();
								const bool showColorFiltering = vFontInfos->m_GlyphFilteringStats.m_UseFilterColoring;

								uint32_t countGlyphs = (uint32_t)vFontInfos->m_FilteredGlyphs.size();
								int rowCount = (int)ct::ceil((double)countGlyphs / (double)glyphCountX);

								m_VirtualClipper.Begin(rowCount, cell_size.y);
								while (m_VirtualClipper.Step())
								{
									for (int j = m_VirtualClipper.DisplayStart; j < m_VirtualClipper.DisplayEnd; j++)
									{
										if (j < 0) continue;

										for (uint32_t i = 0; i < glyphCountX; i++)
										{
											ZoneScopedN("Src DrawFontAtlas_Virtual : Draw One Glyph");

											uint32_t glyphIdx = i + j * glyphCountX;
											if (glyphIdx < countGlyphs)
											{
												auto glyph = vFontInfos->GetGlyphByGlyphIndex(*(vFontInfos->m_FilteredGlyphs.begin() + glyphIdx));
												if (glyph)
												{
													uint32_t x = idx % glyphCountX;

													if (x) ImGui::SameLine();

													if (showRangeColoring)
													{
														if (glyph->codePoint != lastGlyphCodePoint + 1)
														{
															glyphRangeColoring = vProjectFile->GetColorFromInteger(glyph->codePoint);
														}

														ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
														ImVec4 bh = glyphRangeColoring; bh.w = 0.8f;
														ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
														ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
														ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
													}
													else if (showColorFiltering)
													{
														if (glyph->category != GLYPH_CATEGORY_FLAG_NONE)
														{
															if ((glyph->category & GLYPH_CATEGORY_FLAG_SIMPLE) &&
																(vFontInfos->m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_SIMPLE))
																glyphRangeColoring = vFontInfos->m_GlyphFilteringStats.SimpleColor;
															else if ((glyph->category & GLYPH_CATEGORY_FLAG_COMPOSITE) &&
																(vFontInfos->m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_COMPOSITE))
																glyphRangeColoring = vFontInfos->m_GlyphFilteringStats.CompositeColor;
															else if ((glyph->category & GLYPH_CATEGORY_FLAG_COLORED) &&
																(vFontInfos->m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_COLORED))
																glyphRangeColoring = vFontInfos->m_GlyphFilteringStats.ColoredColor;
															else if ((glyph->category & GLYPH_CATEGORY_FLAG_LAYER &&
																(vFontInfos->m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_LAYER)))
																glyphRangeColoring = vFontInfos->m_GlyphFilteringStats.LayerColor;
															else if ((glyph->category & GLYPH_CATEGORY_FLAG_MAPPED) &&
																(vFontInfos->m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_MAPPED))
																glyphRangeColoring = vFontInfos->m_GlyphFilteringStats.MappedColor;
															else if ((glyph->category & GLYPH_CATEGORY_FLAG_UNMAPPED) &&
																(vFontInfos->m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_UNMAPPED))
																glyphRangeColoring = vFontInfos->m_GlyphFilteringStats.UnMappedColor;
															else if ((glyph->category & GLYPH_CATEGORY_FLAG_NAMED) &&
																(vFontInfos->m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_NAMED))
																glyphRangeColoring = vFontInfos->m_GlyphFilteringStats.NamedColor;

															ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
															ImVec4 bh = glyphRangeColoring; bh.w = 0.8f;
															ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
															ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
															ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
														}
													}

													win->DrawList->ChannelsSetCurrent(1);

													bool selected = false;

													// draw selection square in channel 1
													SelectionHelper::Instance()->IsGlyphIntersectedAndSelected(
														vFontInfos, glyph_size, glyph->glyphIndex, &selected,
														SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

													win->DrawList->ChannelsSetCurrent(0);

													// draw glyph in channel 0
													int check = GlyphInfos::DrawGlyphButton(paneWidgetId, vProjectFile, vFontInfos, &selected, glyph_size, glyph);
													if (check)
													{
														// left button : check == 1
														// right button  : check == 2

														SelectionHelper::Instance()->SelectWithToolOrApplyOnGlyph(
															vProjectFile, vFontInfos,
															glyph, idx, selected, true,
															SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
													}

													if (showRangeColoring || showColorFiltering)
													{
														ImGui::PopStyleColor(3);
													}

													if (vProjectFile->m_SourcePane_ShowGlyphTooltip)
													{
														if (ImGui::IsItemHovered())
														{
															ImGui::SetTooltip("Name : %s\nGlyphIndex : %u\nCodePoint : %u\nCount Layers : %u\nCount Parents : %u", 
																glyph->name.c_str(), glyph->glyphIndex, glyph->codePoint, 
																(uint32_t)glyph->layers.size(), (uint32_t)glyph->parents.size());
														}
													}

													lastGlyphCodePoint = glyph->codePoint;
													idx++;
												}
											}
										}
									}
								}
								m_VirtualClipper.End();
							}

							SelectionHelper::Instance()->SelectWithToolOrApply(
								vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
						}
					}
				}
			}
		}

		win->DrawList->ChannelsMerge();
	}
}

void SourceFontPane::DrawFontTexture(std::shared_ptr<FontInfos> vFontInfos)
{
	ZoneScoped;

	if (vFontInfos.use_count())
	{
		if (vFontInfos->IsUsable())
		{
			if (vFontInfos->m_ImFontAtlas.IsBuilt())
			{
				if (vFontInfos->m_ImFontAtlas.TexID)
				{
					if (ImGui::BeginMenuBar())
					{
						if (ImGui::MenuItem("Save to File"))
						{
							ImGuiFileDialog::Instance()->OpenModal("SaveFontToPictureFile", "Svae Font Testure to File", ".png",
								".", 0, IGFDUserDatas(&vFontInfos->m_ImFontAtlas), ImGuiFileDialogFlags_ConfirmOverwrite);
						}

						ImGui::EndMenuBar();
					}

					float w = ImGui::GetContentRegionAvail().x;
					float h = w * (float)vFontInfos->m_ImFontAtlas.TexHeight;
					if (vFontInfos->m_ImFontAtlas.TexWidth > 0)
						h /= (float)vFontInfos->m_ImFontAtlas.TexWidth;
					// for colored glyph we need to render glyphs with color of Vec4(1,1,1,1).
					// so for light theme we will paint a black background
					ImGui::PlainImageWithBG(vFontInfos->m_ImFontAtlas.TexID,
						ImVec2(w, h), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1));
				}
			}
		}
	}
}

std::string SourceFontPane::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	UNUSED(vOffset);
	UNUSED(vUserDatas);

	std::string str;

	//str += vOffset + "<sourcefontpane>\n";

	//str += vOffset + "\t<glyphsizepolicy_count>" + ct::toStr(m_GlyphSize_Policy_Count) + "</glyphsizepolicy_count>\n";
	//str += vOffset + "\t<glyphsizepolicy_width>" + ct::toStr(m_GlyphSize_Policy_Width) + "</glyphsizepolicy_width>\n";

	//str += vOffset + "</sourcefontpane>\n";

	return str;
}

bool SourceFontPane::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "sourcefontpane")
	{
		/*if (strName == "glyphsizepolicy_count")
			m_GlyphSize_Policy_Count = ct::ivariant(strValue).GetI();
		else if (strName == "glyphsizepolicy_width")
			m_GlyphSize_Policy_Width = ct::fvariant(strValue).GetF();*/
	}

	return true;
}