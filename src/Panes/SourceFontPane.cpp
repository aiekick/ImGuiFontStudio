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
#include "Gui/ImGuiWidgets.h"
#include "Project/ProjectFile.h"
#include "Panes/FinalFontPane.h"
#include "Helper/SelectionHelper.h"
#include "Generator/Generator.h"

#include <cinttypes> // printf zu

#include <FileHelper.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

static int SourceFontPane_WidgetId = 0;
#define NEW_ID ++SourceFontPane_WidgetId

SourceFontPane::SourceFontPane()
{

}

SourceFontPane::~SourceFontPane()
{
	
}
int SourceFontPane::DrawSourceFontPane(ProjectFile *vProjectFile, int vWidgetId)
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
			if (vProjectFile->IsLoaded())
			{
				if (vProjectFile->m_CurrentFont)
				{
					if (ImGui::BeginMenuBar())
					{
						SelectionHelper::Instance()->DrawSelectionMenu(vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

						ImGui::EndMenuBar();
					}

					if (m_FontPaneFlags & FontPaneFlags::FONT_PANE_GLYPH)
					{
						DrawFontAtlas(vProjectFile, vProjectFile->m_CurrentFont);
					}
					else if (m_FontPaneFlags & FontPaneFlags::FONT_PANE_TEXTURE)
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
			if (vProjectFile->IsLoaded())
			{
				if (ImGui::BeginFramedGroup("Font File"))
				{
					if (ImGui::Button(ICON_IGFS_FOLDER_OPEN " Open Font"))
					{
						ImGuiFileDialog::Instance()->OpenModal("OpenFontDlg", "Open Font File", ".ttf\0.otf\0\0", ".", 0);
					}

					if (vProjectFile->m_Fonts.size() > 0)
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

										ImGuiFileDialog::Instance()->OpenModal(
											"SolveBadFilePathName",
											label.c_str(), ".ttf\0\0", ".",
											itFont.first, 1, (UserDatas)itFont.first.c_str());
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

										ImGuiFileDialog::Instance()->OpenModal(
											"SolveBadFilePathName",
											label.c_str(), ".ttf\0\0", ".",
											itFont.first, 1, (UserDatas)itFont.first.c_str());
									}
								}
								if (sel)
								{
									if (!itFont.second.m_NeedFilePathResolve)
									{
										vProjectFile->m_CurrentFont = &itFont.second;
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
						ImGui::RadioButtonLabeled_BitWize<FontPaneFlags>(
							ICON_IGFS_GLYPHS " Glyphs", "Show Font Glyphs", 
							&m_FontPaneFlags, FontPaneFlags::FONT_PANE_GLYPH, 0.0f, true); 
						
						ImGui::SameLine();

						ImGui::RadioButtonLabeled_BitWize<FontPaneFlags>(
							ICON_IGFS_TEXTURE " Texture", "Show Font Texture", 
							&m_FontPaneFlags, FontPaneFlags::FONT_PANE_TEXTURE, 0.0f, true);
						
						ImGui::EndFramedGroup(true);
					}

					if (ImGui::BeginFramedGroup("Font Preview"))
					{
						ImGui::PushItemWidth(150.0f);

						bool needFontReGen = false;

						needFontReGen |= ImGui::SliderInt("Size", &vProjectFile->m_CurrentFont->m_FontSize, 7, 50);
						needFontReGen |= ImGui::SliderInt("Over Sample", &vProjectFile->m_CurrentFont->m_Oversample, 1, 5);

						if (needFontReGen)
						{
							vProjectFile->m_CurrentFont->m_FontSize = ct::clamp(vProjectFile->m_CurrentFont->m_FontSize, 7, 50);
							vProjectFile->m_CurrentFont->m_Oversample = ct::clamp(vProjectFile->m_CurrentFont->m_Oversample, 1, 5);
							OpenFont(vProjectFile, vProjectFile->m_CurrentFont->m_FontFilePathName, false);
							vProjectFile->SetProjectChange();
						}

						if (m_FontPaneFlags & FontPaneFlags::FONT_PANE_GLYPH)
						{
							bool change = ImGui::SliderInt("Width Count", &vProjectFile->m_Preview_Glyph_CountX, 50, 1);

							ImGui::Checkbox("Display Range Coloring", &vProjectFile->m_ShowRangeColoring);
							if (vProjectFile->IsRangeColorignShown())
							{
								ImGui::Indent();

								change |= ImGui::SliderFloat("H x", &vProjectFile->m_RangeColoringHash.x, 0, 50);
								change |= ImGui::SliderFloat("H y", &vProjectFile->m_RangeColoringHash.y, 0, 50);
								change |= ImGui::SliderFloat("H z", &vProjectFile->m_RangeColoringHash.z, 0, 50);
								//ImGui::SliderFloat("Alpha", &vProjectFile->m_RangeColoringHash.w, 0, 1);

								ImGui::Unindent();
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
	if (vFontInfos)
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::Text("Filter "); ImGui::SameLine();
			ImGui::PushID(vFontInfos);
			if (ImGui::InputText("##Filter", vFontInfos->m_SearchBuffer, 1023))
			{
				vFontInfos->m_Filters.clear();
				std::string s = vFontInfos->m_SearchBuffer;
				auto arr = ct::splitStringToVector(s, ',');
				for (const auto &it : arr)
				{
					vFontInfos->m_Filters.insert(it);
				}
				vProjectFile->SetProjectChange();
			}
			ImGui::PopID();
		}
		ImGui::EndMenuBar();
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

void SourceFontPane::DrawFontAtlas(ProjectFile *vProjectFile, FontInfos *vFontInfos)
{
	if (vProjectFile && 
		vProjectFile->IsLoaded() &&
		vFontInfos && 
		vProjectFile->m_Preview_Glyph_CountX)
	{
		DrawFilterBar(vProjectFile, vFontInfos);

		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			ImFont* font = vFontInfos->m_ImFontAtlas.Fonts[0];

			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				ImVec2 hostTextureSize = ImVec2(
					(float)vFontInfos->m_ImFontAtlas.TexWidth, 
					(float)vFontInfos->m_ImFontAtlas.TexHeight);
				int countGlyphX = vProjectFile->m_Preview_Glyph_CountX;
				float maxWidth = ImGui::GetContentRegionAvailWidth();
				float glyphSize = floorf(maxWidth / (float)countGlyphX);
				ImVec2 cell_size(glyphSize, glyphSize);
				cell_size -= ImGui::GetStyle().ItemSpacing;
				cell_size -= ImGui::GetStyle().FramePadding * 2.0f;
				int idx = 0; uint32_t lastGlyphCodePoint = 0;
				ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
				for (auto glyph : font->Glyphs)
				{
					std::string name = vFontInfos->m_GlyphCodePointNames[glyph.Codepoint];
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

						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("name : %s\ncodepoint : %u\nadv x : %.2f\nuv0 : (%.3f,%.3f)\nuv1 : (%.3f,%.3f)",
								name.c_str(), glyph.Codepoint, glyph.AdvanceX, glyph.U0, glyph.V0, glyph.U1, glyph.V1);
						}

						lastGlyphCodePoint = glyph.Codepoint;
						idx++;
					}
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
				float h = w * vFontInfos->m_ImFontAtlas.TexHeight;
				if (vFontInfos->m_ImFontAtlas.TexWidth)
					h /= vFontInfos->m_ImFontAtlas.TexWidth;
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

void SourceFontPane::OpenFonts(ProjectFile *vProjectFile, std::map<std::string, std::string> vFontFilePathNames)
{
	for (auto & it : vFontFilePathNames)
	{
		OpenFont(vProjectFile, it.second, false);
	}
	vProjectFile->UpdateCountSelectedGlyphs();
}

void SourceFontPane::OpenFont(ProjectFile *vProjectFile, std::string vFontFilePathName, bool vUpdateCount)
{
	if (vProjectFile->IsLoaded())
	{
		auto ps = FileHelper::Instance()->ParsePathFileName(vFontFilePathName);
		if (ps.isOk)
		{
			std::string fontName = ps.name + "." + ps.ext;
			FontInfos *font = &vProjectFile->m_Fonts[fontName];
			if (font->LoadFont(vProjectFile, vFontFilePathName))
			{
				vProjectFile->m_CurrentFont = font;
				if (vUpdateCount)
					vProjectFile->UpdateCountSelectedGlyphs();
			}
		}
	}
}

void SourceFontPane::CloseCurrentFont(ProjectFile *vProjectFile)
{
	if (vProjectFile->IsLoaded())
	{
		if (vProjectFile->m_CurrentFont)
		{
			vProjectFile->m_CurrentFont->Clear();			
			vProjectFile->m_Fonts.erase(vProjectFile->m_CurrentFont->m_FontFileName);
			if (!vProjectFile->m_Fonts.empty())
			{
				vProjectFile->m_CurrentFont = &vProjectFile->m_Fonts.begin()->second;
			}
			else
			{
				vProjectFile->m_CurrentFont = 0;
			}
			vProjectFile->SetProjectChange();
		}
	}
}

void SourceFontPane::DrawDialosAndPopups(ProjectFile *vProjectFile)
{
	if (vProjectFile->IsLoaded())
	{
		ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
		ImVec2 max = MainFrame::Instance()->m_DisplaySize;

		if (ImGuiFileDialog::Instance()->FileDialog("OpenFontDlg", ImGuiWindowFlags_NoDocking, min, max))
		{
			if (ImGuiFileDialog::Instance()->IsOk)
			{
				OpenFonts(vProjectFile, ImGuiFileDialog::Instance()->GetSelection());
			}

			ImGuiFileDialog::Instance()->CloseDialog("OpenFontDlg");
		}
	}
}