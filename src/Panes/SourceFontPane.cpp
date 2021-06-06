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

bool SourceFontPane::Init()
{
	return true;
}

void SourceFontPane::Unit()
{

}

int SourceFontPane::DrawPanes(int vWidgetId, std::string vUserDatas)
{
	m_PaneWidgetId = vWidgetId;

	DrawSourceFontPane();
	
	return m_PaneWidgetId;
}

void SourceFontPane::DrawDialogsAndPopups(std::string vUserDatas)
{
	
}

int SourceFontPane::DrawWidgets(int vWidgetId, std::string vUserDatas)
{
	UNUSED(vUserDatas);

	if (ProjectFile::Instance()->IsLoaded())
	{
		if (LayoutManager::Instance()->IsSpecificPaneFocused(m_PaneFlag))
		{
			if (ImGui::BeginFramedGroup("Source Font Pane"))
			{
				const float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x;
				const float mrw = maxWidth * 0.5f;

				bool change = false;

				change |= ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(mrw,
					ICON_IGFS_GLYPHS " Glyphs", "Show Font Glyphs",
					&ProjectFile::Instance()->m_SourceFontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH, true);

				ImGui::SameLine();

				change |= ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(mrw,
					ICON_IGFS_TEXTURE " Texture", "Show Font Texture",
					&ProjectFile::Instance()->m_SourceFontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE, true);

				if (change)
				{
					ProjectFile::Instance()->SetProjectChange();
				}

				ImGui::EndFramedGroup();
			}
		}
	}

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : PANES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SourceFontPane::DrawSourceFontPane()
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
				if (ProjectFile::Instance()->m_SelectedFont)
				{
					if (ProjectFile::Instance()->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
					{
						if (ImGui::BeginMenuBar())
						{
							if (ImGui::BeginMenu("Infos"))
							{
								if (ImGui::MenuItem("Show Tooltip", "", ProjectFile::Instance()->m_SourcePane_ShowGlyphTooltip))
								{
									ProjectFile::Instance()->SetProjectChange();
								}

								ImGui::EndMenu();
							}

							ImGui::Spacing();

							SelectionHelper::Instance()->DrawSelectionMenu(SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

							ImGui::Spacing();
							
							if (ProjectFile::Instance()->m_Preview_Glyph_CountX)
							{
								DrawFilterBar(ProjectFile::Instance()->m_SelectedFont);
							}

							ImGui::EndMenuBar();
						}
						
						DrawFontAtlas_Virtual(ProjectFile::Instance()->m_SelectedFont);
					}
					else if (ProjectFile::Instance()->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE)
					{
						DrawFontTexture(ProjectFile::Instance()->m_SelectedFont);
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

void SourceFontPane::DrawFilterBar( std::shared_ptr<FontInfos> vFontInfos)
{
	if (vFontInfos.use_count())
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
			ProjectFile::Instance()->SetProjectChange();
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
			ProjectFile::Instance()->SetProjectChange();
			vFontInfos->UpdateFiltering();
		}
	}
}

void SourceFontPane::DrawFontAtlas_Virtual( std::shared_ptr<FontInfos> vFontInfos)
{
	auto win = ImGui::GetCurrentWindowRead();
	if (win)
	{
		win->DrawList->ChannelsSplit(2);

		if (ProjectFile::Instance()->IsLoaded() &&
			vFontInfos.use_count())
		{
			ProjectFile::Instance()->m_Preview_Glyph_CountX = ct::maxi(ProjectFile::Instance()->m_Preview_Glyph_CountX, 1);

			if (vFontInfos->m_ImFontAtlas.IsBuilt())
			{
				if (vFontInfos->m_ImFontAtlas.TexID)
				{
					if (!vFontInfos->m_FilteredGlyphs.empty())
					{
						ImVec2 cell_size, glyph_size;
						uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(&cell_size, &glyph_size);
						if (glyphCountX)
						{
							uint32_t idx = 0, lastGlyphCodePoint = 0;
							m_GlyphButtonStateColor[0] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
							bool showRangeColoring = ProjectFile::Instance()->IsRangeColoringShown();
							
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
										uint32_t glyphIdx = i + j * glyphCountX;
										if (glyphIdx < countGlyphs)
										{
											auto glyph = *(vFontInfos->m_FilteredGlyphs.begin() + glyphIdx);

											std::string name = vFontInfos->m_GlyphCodePointToName[glyph.Codepoint];
											bool colored = vFontInfos->m_ColoredGlyphs[glyph.Codepoint];

											uint32_t x = idx % glyphCountX;

											if (x) ImGui::SameLine();

											GlyphInfos::GetGlyphButtonColorsForCodePoint(showRangeColoring,
												glyph.Codepoint, lastGlyphCodePoint, m_GlyphButtonStateColor);

											win->DrawList->ChannelsSetCurrent(1);

											bool selected = false;

											// draw selection square in channel 1
											SelectionHelper::Instance()->IsGlyphIntersectedAndSelected(
												vFontInfos, glyph_size, glyph.Codepoint, &selected,
												SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

											win->DrawList->ChannelsSetCurrent(0);

											// draw glyph in channel 0
											int check = GlyphInfos::DrawGlyphButton(
												m_PaneWidgetId, vFontInfos->GetImFont(), 
												&selected, glyph_size, &glyph, m_GlyphButtonStateColor, colored);
											if (check)
											{
												// left button : check == 1
												// right button  : check == 2

												SelectionHelper::Instance()->SelectWithToolOrApplyOnGlyph(
													vFontInfos,
													glyph, idx, selected, true,
													SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
											}

											if (ProjectFile::Instance()->m_SourcePane_ShowGlyphTooltip)
											{
												if (ImGui::IsItemHovered())
												{
													ImGui::SetTooltip("name : %s\ncodepoint : %i", name.c_str(), (int)glyph.Codepoint);
												}
											}

											lastGlyphCodePoint = glyph.Codepoint;
											idx++;
										}
									}
								}
							}
							m_VirtualClipper.End();
						}

						SelectionHelper::Instance()->SelectWithToolOrApply(
							SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
					}
				}
			}
		}

		win->DrawList->ChannelsMerge();
	}
}

void SourceFontPane::DrawFontTexture(std::shared_ptr<FontInfos> vFontInfos)
{
	if (vFontInfos.use_count())
	{
		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::MenuItem("Save to File"))
					{
						std::string path = ".";
						if (MainFrame::Instance()->GetProject()->IsLoaded())
							path = MainFrame::Instance()->GetProject()->m_ProjectFilePath;
						ImGuiFileDialog::Instance()->OpenModal("SaveFontToPictureFile", "Svae Font Testure to File", ".png", 
							path, 0, IGFDUserDatas(&vFontInfos->m_ImFontAtlas), ImGuiFileDialogFlags_ConfirmOverwrite);
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

std::string SourceFontPane::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
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