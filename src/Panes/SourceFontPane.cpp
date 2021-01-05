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
#include <ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h>

#include <MainFrame.h>
#include <Helper/SelectionHelper.h>
#include <Panes/FinalFontPane.h>
#include <Panes/Manager/LayoutManager.h>
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>

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
	
}

void SourceFontPane::Unit()
{

}

int SourceFontPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	paneWidgetId = vWidgetId;

	DrawSourceFontPane(vProjectFile);
	DrawParamsPane(vProjectFile);

	return paneWidgetId;
}

void SourceFontPane::DrawDialogsAndPopups(ProjectFile * vProjectFile)
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

		if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveFontToPictureFile", ImGuiWindowFlags_NoDocking, min, max))
		{
			if (igfd::ImGuiFileDialog::Instance()->IsOk)
			{
				auto atlas = dynamic_cast<ImFontAtlas*>((ImFontAtlas*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
				if (atlas)
				{
					GLuint textureToSave = (GLuint)(size_t)atlas->TexID;
					if (textureToSave)
					{
						auto win = MainFrame::Instance()->GetGLFWwindow();
						auto file = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
						Generator::SaveTextureToPng(win, file, textureToSave,
							ct::uvec2(atlas->TexWidth, atlas->TexHeight), 4U);
						FileHelper::Instance()->OpenFile(file);
					}
				}
			}

			igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveFontToPictureFile");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : PANES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SourceFontPane::DrawSourceFontPane(ProjectFile *vProjectFile)
{
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

void SourceFontPane::DrawParamsPane(ProjectFile *vProjectFile)
{
	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_PARAM)
	{
		if (ImGui::Begin<PaneFlags>(PARAM_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_PARAM,
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
							CloseSelectedFont(vProjectFile);
						}

						ImGui::Text("Opened Fonts :");

						const float aw = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f;

						static int selection = 0;
						static ImGuiTableFlags flags =
							ImGuiTableFlags_SizingPolicyFixed |	ImGuiTableFlags_RowBg |
							ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
							ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_Borders;
						if (ImGui::BeginTable("##fileTable", 2, flags, ImVec2(aw, 100)))
						{
							ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
							ImGui::TableSetupColumn("Font Files", ImGuiTableColumnFlags_WidthStretch, -1, 0);
							ImGui::TableSetupColumn("Act", ImGuiTableColumnFlags_WidthAuto, -1, 1);
							//ImGui::TableHeadersRow(); not needed
							
							for (auto & itFont : vProjectFile->m_Fonts)
							{
								bool sel = false;

								ImGui::TableNextRow();
								if (itFont.second.m_NeedFilePathResolve)
								{
									if (ImGui::TableSetColumnIndex(0)) // first column
									{
										ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.8f, 0.5f, 0.2f, 0.6f));
										ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
										selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
										sel = ImGui::Selectable(itFont.first.c_str(), vProjectFile->m_SelectedFont == &itFont.second, selectableFlags);
										ImGui::PopStyleColor();

										float cw = ImGui::GetContentRegionAvail().x;
										float sw = ImGui::CalcTextSize(itFont.first.c_str()).x;
										if (sw > cw)
										{
											if (ImGui::IsItemHovered())
												ImGui::SetTooltip(itFont.first.c_str());
										}
									}
									if (ImGui::TableSetColumnIndex(1)) // second column
									{
										ImGui::PushID(++paneWidgetId);
										if (ImGui::TransparentButton(ICON_IGFS_EDIT))
										{
											std::string label = "Search good file for Font " + itFont.first;

											igfd::ImGuiFileDialog::Instance()->OpenModal(
												"SolveBadFilePathName",
												label.c_str(), "Font File (*.ttf *.otf){.ttf,.otf}", ".",
												itFont.first, 1, (igfd::UserDatas)itFont.first.c_str());
										}
										if (ImGui::IsItemHovered())
										{
											ImGui::SetTooltip("Font file Not Found\nClick for search the good file");
										}
										ImGui::PopID();
									}
								}
								else
								{
									if (ImGui::TableSetColumnIndex(0)) // first column
									{
										ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
										selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
										sel = ImGui::Selectable(itFont.first.c_str(), vProjectFile->m_SelectedFont == &itFont.second, selectableFlags);

										float cw = ImGui::GetContentRegionAvail().x;
										float sw = ImGui::CalcTextSize(itFont.first.c_str()).x;
										if (sw > cw)
										{
											if (ImGui::IsItemHovered())
												ImGui::SetTooltip(itFont.first.c_str());
										}
									}
									if (ImGui::TableSetColumnIndex(1)) // second column
									{
										ImGui::PushID(++paneWidgetId);
										if (ImGui::TransparentButton(ICON_IGFS_EDIT))
										{
											std::string label = "Search an alternative file for Font " + itFont.first;

											igfd::ImGuiFileDialog::Instance()->OpenModal(
												"SolveBadFilePathName",
												label.c_str(), "Font File (*.ttf *.otf){.ttf,.otf}", ".",
												itFont.first, 1, (igfd::UserDatas)itFont.first.c_str());
										}
										if (ImGui::IsItemHovered())
										{
											ImGui::SetTooltip("Click for search an alternative file\nbut keep selected glyphs");
										}
										ImGui::PopID();
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
							ImGui::EndTable();
						}
					}

					ImGui::EndFramedGroup(true);
				}

				if (vProjectFile->m_SelectedFont)
				{
					vProjectFile->m_SelectedFont->DrawInfos();

					if (ImGui::BeginFramedGroup("Pane Mode"))
					{
						ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(
							ICON_IGFS_GLYPHS " Glyphs", "Show Font Glyphs", 
							&vProjectFile->m_SourceFontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH, 0.0f, true);
						
						ImGui::SameLine();

						ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(
							ICON_IGFS_TEXTURE " Texture", "Show Font Texture", 
							&vProjectFile->m_SourceFontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE, 0.0f, true);
						
						ImGui::EndFramedGroup(true);
					}

					if (ImGui::BeginFramedGroup("Font Preview"))
					{
						bool needFontReGen = false;

						float aw = ImGui::GetContentRegionAvail().x;

						needFontReGen |= ImGui::SliderIntDefaultCompact(aw, "Font Size", &vProjectFile->m_SelectedFont->m_FontSize, 7, 50, defaultFontInfosValues.m_FontSize);
						needFontReGen |= ImGui::SliderIntDefaultCompact(aw, "Font Anti-aliasing", &vProjectFile->m_SelectedFont->m_Oversample, 1, 5, defaultFontInfosValues.m_Oversample);

						if (needFontReGen)
						{
							vProjectFile->m_SelectedFont->m_FontSize = ct::clamp(vProjectFile->m_SelectedFont->m_FontSize, 7, 50);
							vProjectFile->m_SelectedFont->m_Oversample = ct::clamp(vProjectFile->m_SelectedFont->m_Oversample, 1, 5);
							OpenFont(vProjectFile, vProjectFile->m_SelectedFont->m_FontFilePathName, false);
							vProjectFile->SetProjectChange();
						}

						if (vProjectFile->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
						{
							const auto style = &ImGui::GetStyle();
							static float radioButtonWidth = ImGui::GetFrameHeight();
							aw = ImGui::GetContentRegionAvail().x - style->ItemSpacing.x - radioButtonWidth;

							bool change = false;
							if (ImGui::SliderIntDefaultCompact(aw, "Glyph Count X", &vProjectFile->m_Preview_Glyph_CountX, 50, 1, defaultProjectValues.m_Preview_Glyph_CountX))
							{
								change = true;
								vProjectFile->m_GlyphDisplayTuningMode = GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT;
								vProjectFile->m_Preview_Glyph_CountX = ct::maxi(vProjectFile->m_Preview_Glyph_CountX, 1); // can prevent bugs (like div by zero) everywhere when user input value
							}
							ImGui::SameLine();
							ImGui::RadioButtonLabeled_BitWize<GlyphDisplayTuningModeFlags>(
								ICON_IGFS_USED "##GlypCountIsMaster",
								ICON_IGFS_NOT_USED "##GlypCountIsMaster",
								"Apply Glyph Count Policy on Resize",
								&vProjectFile->m_GlyphDisplayTuningMode, GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT, radioButtonWidth, true);
							radioButtonWidth = ImGui::GetItemRectSize().x;

							if (ImGui::SliderFloatDefaultCompact(aw, "Glyph Width", &vProjectFile->m_Preview_Glyph_Width, 10.0f,
								GlyphDisplayHelper::currentPaneAvailWidth * 0.5f, defaultProjectValues.m_Preview_Glyph_Width))
							{
								change = true;
								vProjectFile->m_GlyphDisplayTuningMode = GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE;
								vProjectFile->m_Preview_Glyph_Width = ct::maxi(vProjectFile->m_Preview_Glyph_Width, 10.0f); // can prevent bugs (like div by zero) everywhere when user input value
							}

							ImGui::SameLine();
							ImGui::RadioButtonLabeled_BitWize<GlyphDisplayTuningModeFlags>(
								ICON_IGFS_USED "##GlypSizeIsMaster",
								ICON_IGFS_NOT_USED "##GlypSizeIsMaster",
								"Apply Glyph Width Policy on Resize",
								&vProjectFile->m_GlyphDisplayTuningMode, GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE, radioButtonWidth, true);
							
							ImGui::FramedGroupSeparator();

							ImGui::Checkbox("Show Range Colors", &vProjectFile->m_ShowRangeColoring);
							if (vProjectFile->IsRangeColoringShown())
							{
								change |= ImGui::SliderFloatDefaultCompact(aw, "H x", &vProjectFile->m_RangeColoringHash.x, 0, 50, defaultProjectValues.m_RangeColoringHash.x);
								change |= ImGui::SliderFloatDefaultCompact(aw, "H y", &vProjectFile->m_RangeColoringHash.y, 0, 50, defaultProjectValues.m_RangeColoringHash.y);
								change |= ImGui::SliderFloatDefaultCompact(aw, "H z", &vProjectFile->m_RangeColoringHash.z, 0, 50, defaultProjectValues.m_RangeColoringHash.z);
								//change |= ImGui::SliderFloatDefaultCompact(aw, "Alpha", &vProjectFile->m_RangeColoringHash.w, 0, 1, defaultProjectValues.m_RangeColoringHash.w);
							}

							if (change)
								vProjectFile->SetProjectChange();
						}

						ImGui::EndFramedGroup(true);
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

void SourceFontPane::DrawFilterBar(ProjectFile *vProjectFile, FontInfos *vFontInfos)
{
	if (vProjectFile && vFontInfos)
	{
		ImGui::PushID(vFontInfos);
		ImGui::Text("Filters");
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("will search for any filter separated\n\tby a coma ',' or a space ' '");
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
				ImVec2 cell_size, glyph_size;
				uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);
				if (glyphCountX)
				{
					uint32_t idx = 0, lastGlyphCodePoint = 0;
					ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
					bool showRangeColoring = vProjectFile->IsRangeColoringShown();
					if (!font->Glyphs.empty())
					{
						uint32_t countGlyphs = (uint32_t)font->Glyphs.size();
						int rowCount = (int)ct::ceil((double)countGlyphs / (double)glyphCountX);
						ImGuiListClipper m_Clipper;

						/*ImGui::SetTooltip(
	"cell size : %.2f, %.2f\n\
	glyph size : %.2f, %.2f\n\
	count glyphs : x:%u,y:%i\n\
	line height : %.0f",
							cell_size.x, cell_size.y,
							glyph_size.x, glyph_size.y,
							glyphCountX, rowCount,
							cell_size.y);*/

						m_Clipper.Begin(rowCount, cell_size.y);
						while (m_Clipper.Step())
						{
							for (int j = m_Clipper.DisplayStart; j < m_Clipper.DisplayEnd; j++)
							{
								if (j < 0) continue;

								for (uint32_t i = 0; i < glyphCountX; i++)
								{
									uint32_t glyphIdx = i + j * glyphCountX;
									if (glyphIdx < countGlyphs)
									{
										auto glyph = *(font->Glyphs.begin() + glyphIdx);

										std::string name = vFontInfos->m_GlyphCodePointToName[glyph.Codepoint];
										if (IfCatchedByFilters(vFontInfos, name))
										{
											uint32_t x = idx % glyphCountX;

											if (x) ImGui::SameLine();

											if (showRangeColoring)
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
												vFontInfos, glyph_size, glyph.Codepoint, &selected,
												SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

											ImGui::PushID(NewWidgetId());
											bool check = ImGui::ImageCheckButton(vFontInfos->m_ImFontAtlas.TexID, &selected, glyph_size,
												ImVec2(glyph.U0, glyph.V0), ImVec2(glyph.U1, glyph.V1), hostTextureSize);
											ImGui::PopID();

											if (check)
											{
												SelectionHelper::Instance()->SelectWithToolOrApplyOnGlyph(
													vProjectFile, vFontInfos,
													glyph, idx, selected, true,
													SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
											}

											if (showRangeColoring)
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
						m_Clipper.End();
					}

					SelectionHelper::Instance()->SelectWithToolOrApply(
						vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
				}
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
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::MenuItem("Save to File"))
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("SaveFontToPictureFile", "Svae Font Testure to File", ".png", 
							".", 0, igfd::UserDatas(&vFontInfos->m_ImFontAtlas), ImGuiFileDialogFlags_ConfirmOverwrite);
					}

					ImGui::EndMenuBar();
				}

				float w = ImGui::GetContentRegionAvail().x;
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

void SourceFontPane::CloseSelectedFont(ProjectFile *vProjectFile)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vProjectFile->m_SelectedFont)
		{
			vProjectFile->m_SelectedFont->Clear();			
			vProjectFile->m_Fonts.erase(vProjectFile->m_SelectedFont->m_FontFileName);
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
		vProjectFile->m_SelectedFont = vFontInfos;
		if (vFontInfos)
		{
			vProjectFile->m_FontToMergeIn = vFontInfos->m_FontFileName;
		}
		vProjectFile->SetProjectChange();
	}
}

bool SourceFontPane::IsFlagSet(ProjectFile* vProjectFile, SourceFontPaneFlags vFlag)
{
	return (vProjectFile->m_SourceFontPaneFlags & vFlag);
}
