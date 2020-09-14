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

#include "ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h"

#include "MainFrame.h"

#include "Gui/GuiLayout.h"
#include "Panes/FinalFontPane.h"
#include "Helper/SelectionHelper.h"

#include <cinttypes> // printf zu

#include <FileHelper.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

static int SourceFontPane_WidgetId = 0;
#define NEW_ID ++SourceFontPane_WidgetId
static ProjectFile defaultProjectValues;
static FontInfos defaultFontInfosValues;

SourceFontPane::SourceFontPane() = default;
SourceFontPane::~SourceFontPane() = default;

int SourceFontPane::DrawSourceFontPane(ProjectFile *vProjectFile, int vWidgetId) const
{
	SourceFontPane_WidgetId = vWidgetId;

	if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_SOURCE)
	{
		if (ImGui::Begin<PaneFlags>(SOURCE_PANE, 
			&GuiLayout::m_Pane_Shown, PaneFlags::PANE_SOURCE,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile && vProjectFile->IsLoaded())
			{
				if (vProjectFile->m_CurrentFont)
				{
					if (m_FontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
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
								DrawFilterBar(vProjectFile, vProjectFile->m_CurrentFont);
							}

							ImGui::EndMenuBar();
						}
						
						DrawFontAtlas_Virtual(vProjectFile, vProjectFile->m_CurrentFont);
					}
					else if (m_FontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE)
					{
						DrawFontTexture(vProjectFile->m_CurrentFont);
					}
				}
			}
		}

		ImGui::End();
	}

	return SourceFontPane_WidgetId;
}

int SourceFontPane::DrawParamsPane(ProjectFile *vProjectFile, int vWidgetId)
{
	if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_PARAM)
	{
		if (ImGui::Begin<PaneFlags>(PARAM_PANE,
			&GuiLayout::m_Pane_Shown, PaneFlags::PANE_PARAM,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile && vProjectFile->IsLoaded())
			{
				if (ImGui::BeginFramedGroup("Font File"))
				{
					if (ImGui::Button(ICON_IGFS_FOLDER_OPEN " Open Font"))
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("OpenFontDlg", "Open Font File", "Font File (*.ttf *.otf){.ttf,.otf}", ".", 0);
					}

					if (!vProjectFile->m_Fonts.empty())
					{
						ImGui::SameLine();

						if (ImGui::Button(ICON_IGFS_DESTROY " Close Font"))
						{
							CloseCurrentFont(vProjectFile);
						}

						ImGui::Text("Opened Fonts :");

						ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - ImGui::GetStyle().IndentSpacing);

						if (ImGui::ListBoxHeader("##OpenedFontsListBox", 5))
						{
							for (auto & itFont : vProjectFile->m_Fonts)
							{
								bool sel = false, infosClick = false, infosHovered = false;
								if (itFont.second.m_NeedFilePathResolve)
								{
									ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.8f, 0.5f, 0.2f, 0.6f));
									sel = ImGui::SelectableWithBtn(itFont.first.c_str(), vProjectFile->m_CurrentFont == &itFont.second,
										ICON_IGFS_WARNING, &infosClick, &infosHovered, ImVec4(0.8f, 0.8f, 0.0f, 0.8f), ImVec4(0.8f, 0.5f, 0.5f, 0.8f));
									ImGui::PopStyleColor();

									if (infosHovered)
									{
										ImGui::SetTooltip("Font file Not Found\nClick for search the good file");
									}

									if (infosClick)
									{
										std::string label = "Search good file for Font " + itFont.first;

										igfd::ImGuiFileDialog::Instance()->OpenModal(
											"SolveBadFilePathName",
											label.c_str(), "Font File (*.ttf *.otf){.ttf,.otf}", ".",
											itFont.first, 1, (igfd::UserDatas)itFont.first.c_str());
									}
								}
								else
								{
									sel = ImGui::SelectableWithBtn(itFont.first.c_str(), vProjectFile->m_CurrentFont == &itFont.second,
										ICON_IGFS_EDIT, &infosClick, &infosHovered, ImGui::GetStyleColorVec4(ImGuiCol_Text), ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));

									if (infosHovered)
									{
										ImGui::SetTooltip("Click for search an alternative file\nbut keep selected glyphs");
									}

									if (infosClick)
									{
										std::string label = "Search an alternative file for Font " + itFont.first;

										igfd::ImGuiFileDialog::Instance()->OpenModal(
											"SolveBadFilePathName",
											label.c_str(), "Font File (*.ttf *.otf){.ttf,.otf}", ".",
											itFont.first, 1, (igfd::UserDatas)itFont.first.c_str());
									}
								}
								if (sel)
								{
									if (!itFont.second.m_NeedFilePathResolve)
									{
										SelectFont(vProjectFile, &itFont.second);
									}
								}
							}

							ImGui::ListBoxFooter();
						}

						ImGui::PopItemWidth();
					}

					ImGui::EndFramedGroup(true);
				}

				if (vProjectFile->m_CurrentFont)
				{
					vProjectFile->m_CurrentFont->DrawInfos();

					if (ImGui::BeginFramedGroup("Pane Mode"))
					{
						ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(
							ICON_IGFS_GLYPHS " Glyphs", "Show Font Glyphs", 
							&m_FontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH, 0.0f, true);
						
						ImGui::SameLine();

						ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(
							ICON_IGFS_TEXTURE " Texture", "Show Font Texture", 
							&m_FontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE, 0.0f, true);
						
						ImGui::EndFramedGroup(true);
					}

					if (ImGui::BeginFramedGroup("Font Preview"))
					{
						ImGui::PushItemWidth(150.0f);

						bool needFontReGen = false;

						const float aw = ImGui::GetContentRegionAvailWidth();

						needFontReGen |= ImGui::SliderIntDefaultCompact(aw, "Size", &vProjectFile->m_CurrentFont->m_FontSize, 7, 50, defaultFontInfosValues.m_FontSize);
						needFontReGen |= ImGui::SliderIntDefaultCompact(aw, "Over Sample", &vProjectFile->m_CurrentFont->m_Oversample, 1, 5, defaultFontInfosValues.m_Oversample);

						if (needFontReGen)
						{
							vProjectFile->m_CurrentFont->m_FontSize = ct::clamp(vProjectFile->m_CurrentFont->m_FontSize, 7, 50);
							vProjectFile->m_CurrentFont->m_Oversample = ct::clamp(vProjectFile->m_CurrentFont->m_Oversample, 1, 5);
							OpenFont(vProjectFile, vProjectFile->m_CurrentFont->m_FontFilePathName, false);
							vProjectFile->SetProjectChange();
						}

						if (m_FontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
						{
							bool change = ImGui::SliderIntDefaultCompact(aw, "Width Count", &vProjectFile->m_Preview_Glyph_CountX, 50, 1, defaultProjectValues.m_Preview_Glyph_CountX);

							ImGui::FramedGroupSeparator();

							ImGui::Checkbox("Show Range Colors", &vProjectFile->m_ShowRangeColoring);
							if (vProjectFile->IsRangeColorignShown())
							{
								change |= ImGui::SliderFloatDefaultCompact(aw, "H x", &vProjectFile->m_RangeColoringHash.x, 0, 50, defaultProjectValues.m_RangeColoringHash.x);
								change |= ImGui::SliderFloatDefaultCompact(aw, "H y", &vProjectFile->m_RangeColoringHash.y, 0, 50, defaultProjectValues.m_RangeColoringHash.y);
								change |= ImGui::SliderFloatDefaultCompact(aw, "H z", &vProjectFile->m_RangeColoringHash.z, 0, 50, defaultProjectValues.m_RangeColoringHash.z);
								//change |= ImGui::SliderFloatDefaultCompact(aw, "Alpha", &vProjectFile->m_RangeColoringHash.w, 0, 1, defaultProjectValues.m_RangeColoringHash.w);
							}

							if (change)
								vProjectFile->SetProjectChange();
						}

						ImGui::PopItemWidth();

						ImGui::EndFramedGroup(true);
					}
				}
			}
		}

		ImGui::End();
	}

	return vWidgetId;
}

void SourceFontPane::DrawFilterBar(ProjectFile *vProjectFile, FontInfos *vFontInfos)
{
	if (vProjectFile && vFontInfos)
	{
		ImGui::PushID(vFontInfos);
		ImGui::Text("Filters"); ImGui::SameLine();
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("will search for any filter separated\n\tby a coma ',' or a space ' '");
		ImGui::SameLine();
		if (ImGui::MenuItem(ICON_IGFS_DESTROY "##clearFilter"))
		{
			ct::ResetBuffer(vFontInfos->m_SearchBuffer);
			vFontInfos->m_Filters.clear();
			vProjectFile->SetProjectChange();
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
		}
	}
}

bool SourceFontPane::IfCatchedByFilters(FontInfos *vFontInfos, const std::string& vSymbolName)
{
	if (vFontInfos)
	{
		if (vFontInfos->m_Filters.empty())
		{
			return true;
		}
		else
		{
			for (const auto &it : vFontInfos->m_Filters)
			{
				if (vSymbolName.find(it) != std::string::npos) // found
				{
					return true;
				}
			}
		}
	}

	return false;
}

void SourceFontPane::DrawFontAtlas_Virtual(ProjectFile *vProjectFile, FontInfos *vFontInfos)
{
    if (vProjectFile && vProjectFile->IsLoaded() &&
        vFontInfos && vProjectFile->m_Preview_Glyph_CountX)
    {
        if (vFontInfos->m_ImFontAtlas.IsBuilt())
        {
            ImFont* font = vFontInfos->m_ImFontAtlas.Fonts[0];

            if (vFontInfos->m_ImFontAtlas.TexID)
            {
                ImVec2 hostTextureSize = ImVec2(
                        (float)vFontInfos->m_ImFontAtlas.TexWidth,
                        (float)vFontInfos->m_ImFontAtlas.TexHeight);
                int countGlyphX = vProjectFile->m_Preview_Glyph_CountX;
                ImVec2 maxSize = ImGui::GetContentRegionAvail();
                float glyphSize = ct::floor(maxSize.x / (float)countGlyphX);
                ImVec2 cell_size(glyphSize, glyphSize);
                cell_size -= ImGui::GetStyle().ItemSpacing;
                cell_size -= ImGui::GetStyle().FramePadding * 2.0f;
                int idx = 0; uint32_t lastGlyphCodePoint = 0;
                ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
                if (!font->Glyphs.empty())
                {
                    size_t countGlyphs = font->Glyphs.size();
                    int rowCount = (int)ct::ceil((double)countGlyphs / (double)countGlyphX);
                    // ImGuiListClipper infos, found in https://github.com/ocornut/imgui/issues/150 // for virtual list display
                    ImGuiListClipper clipper(rowCount, glyphSize);
                    {
                        for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++)
                        {
                            for (int i = 0; i < countGlyphX; i++)
                            {
                                size_t glyphIdx = i  + j * countGlyphX;
                                if (glyphIdx < countGlyphs)
                                {
                                    auto glyph = *(font->Glyphs.begin() + glyphIdx);

                                    std::string name = vFontInfos->m_GlyphCodePointToName[glyph.Codepoint];
                                    if (IfCatchedByFilters(vFontInfos, name))
                                    {
                                        int x = idx % countGlyphX;

                                        if (x) ImGui::SameLine();

                                        if (vProjectFile->IsRangeColorignShown())
                                        {
                                            if (glyph.Codepoint != lastGlyphCodePoint + 1)
                                            {
                                                glyphRangeColoring = vProjectFile->GetColorFromInteger(glyph.Codepoint);
                                            }

                                            ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
                                            ImVec4 bh = glyphRangeColoring; bh.w = 0.8f;
                                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
                                            ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
                                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
                                        }

                                        bool selected = false;
                                        SelectionHelper::Instance()->IsGlyphIntersectedAndSelected(
                                                vFontInfos,	cell_size, glyph.Codepoint, &selected,
                                                SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

                                        ImGui::PushID(NEW_ID);
                                        bool check = ImGui::ImageCheckButton(vFontInfos->m_ImFontAtlas.TexID, &selected, cell_size,
                                                                             ImVec2(glyph.U0, glyph.V0), ImVec2(glyph.U1, glyph.V1), hostTextureSize);
                                        ImGui::PopID();

                                        if (check)
                                        {
                                            SelectionHelper::Instance()->SelectWithToolOrApplyOnGlyph(
                                                    vProjectFile, vFontInfos,
                                                    glyph, idx, selected, true,
                                                    SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
                                        }

                                        if (vProjectFile->IsRangeColorignShown())
                                        {
                                            ImGui::PopStyleColor(3);
                                        }

                                        if (vProjectFile->m_SourcePane_ShowGlyphTooltip)
                                        {
                                            if (ImGui::IsItemHovered())
                                            {
                                                ImGui::SetTooltip("name : %s\ncodepoint : %i\nadv x : %.2f\nuv0 : (%.3f,%.3f)\nuv1 : (%.3f,%.3f)",
                                                                  name.c_str(), (int)glyph.Codepoint, glyph.AdvanceX, glyph.U0, glyph.V0, glyph.U1, glyph.V1);
                                            }
                                        }

                                        lastGlyphCodePoint = glyph.Codepoint;
                                        idx++;
                                    }
                                }
                            }
                        }
                    }
                    clipper.End();
                }

                SelectionHelper::Instance()->SelectWithToolOrApply(
                        vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
            }
        }
    }
}

void SourceFontPane::DrawFontTexture(FontInfos *vFontInfos)
{
	if (vFontInfos)
	{
		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				float w = ImGui::GetContentRegionAvailWidth();
				float h = w * (float)vFontInfos->m_ImFontAtlas.TexHeight;
				if (vFontInfos->m_ImFontAtlas.TexWidth > 0)
					h /= (float)vFontInfos->m_ImFontAtlas.TexWidth;
				ImGui::Image(vFontInfos->m_ImFontAtlas.TexID, 
					ImVec2(w, h), ImVec2(0, 0), ImVec2(1, 1), 
					ImGui::GetStyleColorVec4(ImGuiCol_Text));
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//// FONT ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void SourceFontPane::OpenFonts(ProjectFile *vProjectFile, const std::map<std::string, std::string>& vFontFilePathNames)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		for (auto & it : vFontFilePathNames)
		{
			OpenFont(vProjectFile, it.second, false);
		}
		vProjectFile->UpdateCountSelectedGlyphs();
	}
}

void SourceFontPane::OpenFont(ProjectFile *vProjectFile, const std::string& vFontFilePathName, bool vUpdateCount)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		auto ps = FileHelper::Instance()->ParsePathFileName(vFontFilePathName);
		if (ps.isOk)
		{
			std::string fontName = ps.name + "." + ps.ext;
			FontInfos *font = &vProjectFile->m_Fonts[fontName];
			if (font->LoadFont(vProjectFile, vFontFilePathName))
			{
				if (vProjectFile->m_FontToMergeIn.empty() ||
				    vProjectFile->m_FontToMergeIn == font->m_FontFileName)
				{
					SelectFont(vProjectFile, font);
				}

				if (vUpdateCount)
					vProjectFile->UpdateCountSelectedGlyphs();
			}
		}
	}
}

void SourceFontPane::CloseCurrentFont(ProjectFile *vProjectFile)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vProjectFile->m_CurrentFont)
		{
			vProjectFile->m_CurrentFont->Clear();			
			vProjectFile->m_Fonts.erase(vProjectFile->m_CurrentFont->m_FontFileName);
			if (!vProjectFile->m_Fonts.empty())
			{
				SelectFont(vProjectFile, &vProjectFile->m_Fonts.begin()->second);
			}
			else
			{
				SelectFont(vProjectFile, nullptr);
			}
			vProjectFile->SetProjectChange();
		}
	}
}

void SourceFontPane::SelectFont(ProjectFile *vProjectFile, FontInfos *vFontInfos)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		vProjectFile->m_CurrentFont = vFontInfos;
		if (vFontInfos)
		{
			vProjectFile->m_FontToMergeIn = vFontInfos->m_FontFileName;
		}
		vProjectFile->SetProjectChange();
	}
}

void SourceFontPane::DrawDialosAndPopups(ProjectFile *vProjectFile)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
		ImVec2 max = MainFrame::Instance()->m_DisplaySize;

		if (igfd::ImGuiFileDialog::Instance()->FileDialog("OpenFontDlg", ImGuiWindowFlags_NoDocking, min, max))
		{
			if (igfd::ImGuiFileDialog::Instance()->IsOk)
			{
				OpenFonts(vProjectFile, igfd::ImGuiFileDialog::Instance()->GetSelection());
			}

			igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenFontDlg");
		}
	}
}