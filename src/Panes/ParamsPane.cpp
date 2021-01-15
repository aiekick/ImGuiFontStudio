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
#include "ParamsPane.h"

#include <ctools/FileHelper.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <MainFrame.h>
#include <Helper/SelectionHelper.h>
#include <Panes/FinalFontPane.h>
#include <Panes/Manager/LayoutManager.h>
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>
#include <Helper/AssetManager.h>

#include <cinttypes> // printf zu

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

static ProjectFile defaultProjectValues;
static FontInfos defaultFontInfosValues;

ParamsPane::ParamsPane() = default;
ParamsPane::~ParamsPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void ParamsPane::Init()
{
	
}

void ParamsPane::Unit()
{

}

int ParamsPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	paneWidgetId = vWidgetId;

	DrawParamsPane(vProjectFile);

	return paneWidgetId;
}

void ParamsPane::DrawDialogsAndPopups(ProjectFile * vProjectFile)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
		ImVec2 max = MainFrame::Instance()->m_DisplaySize;

		if (ImGuiFileDialog::Instance()->Display("OpenFontDlg", ImGuiWindowFlags_NoDocking, min, max))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				OpenFonts(vProjectFile, ImGuiFileDialog::Instance()->GetSelection());
			}

			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("SaveFontToPictureFile", ImGuiWindowFlags_NoDocking, min, max))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				auto atlas = dynamic_cast<ImFontAtlas*>((ImFontAtlas*)ImGuiFileDialog::Instance()->GetUserDatas());
				if (atlas)
				{
					GLuint textureToSave = (GLuint)(size_t)atlas->TexID;
					if (textureToSave)
					{
						auto win = MainFrame::Instance()->GetGLFWwindow();
						auto file = ImGuiFileDialog::Instance()->GetFilePathName();
						Generator::SaveTextureToPng(win, file, textureToSave,
							ct::uvec2(atlas->TexWidth, atlas->TexHeight), 4U);
						FileHelper::Instance()->OpenFile(file);
					}
				}
			}

			ImGuiFileDialog::Instance()->Close();
		}
	}
}

int ParamsPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	UNUSED(vProjectFile);
	UNUSED(vUserDatas);

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : PANES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void ParamsPane::DrawParamsPane(ProjectFile *vProjectFile)
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
				float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 4.0f;
				float mrw = maxWidth / 2.0f - ImGui::GetStyle().FramePadding.x;
				
				if (ImGui::BeginFramedGroup("Font File"))
				{
					if (ImGui::Button(ICON_IGFS_FOLDER_OPEN " Open Font", ImVec2(mrw, 0.0f)))
					{
						Action_Menu_OpenFont();
					}

					if (!vProjectFile->m_Fonts.empty())
					{
						ImGui::SameLine();

						if (ImGui::Button(ICON_IGFS_DESTROY " Close Font", ImVec2(mrw, 0.0f)))
						{
							Action_Menu_CloseFont();
						}

						ImGui::FramedGroupSeparator();

						ImGui::FramedGroupText("Opened Fonts");

						static int selection = 0;
						static ImGuiTableFlags flags =
							ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
							ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
							ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_Borders;
						if (ImGui::BeginTable("##fileTable", 2, flags, ImVec2(maxWidth - ImGui::GetStyle().FramePadding.x, 100)))
						{
							ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
							ImGui::TableSetupColumn("Font Files", ImGuiTableColumnFlags_WidthStretch, -1, 0);
							ImGui::TableSetupColumn("Edit", ImGuiTableColumnFlags_WidthFixed, 32, 1);
							ImGui::TableHeadersRow();
							
							for (auto itFont : vProjectFile->m_Fonts)
							{
								bool sel = false;

								ImGui::TableNextRow();
								
								if (ImGui::TableSetColumnIndex(0)) // first column
								{
									if (itFont.second->m_NeedFilePathResolve)
										ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.8f, 0.5f, 0.2f, 0.6f));

									ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
									selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
									sel = ImGui::Selectable(itFont.first.c_str(), vProjectFile->m_SelectedFont == itFont.second, 
										selectableFlags, ImVec2(0, ImGui::GetTextLineHeightWithSpacing()));

									if (itFont.second->m_NeedFilePathResolve)
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
									if (itFont.second->m_NeedFilePathResolve)
										ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 0.8f));
									if (ImGui::TransparentButton((itFont.second->m_NeedFilePathResolve ? ICON_IGFS_WARNING : ICON_IGFS_EDIT), 
										ImVec2(24, ImGui::GetTextLineHeightWithSpacing())))
									{
										std::string label;
										if (itFont.second->m_NeedFilePathResolve)
											label = "Search the good font file path " + itFont.first;
										else
											label = "Search an alternative font file " + itFont.first;

										ImGuiFileDialog::Instance()->OpenModal(
											"SolveBadFilePathName",
											label.c_str(), "Font File (*.ttf *.otf){.ttf,.otf}", ".",
											itFont.first.c_str(), 1, (IGFD::UserDatas)itFont.first.c_str());
									}
									if (itFont.second->m_NeedFilePathResolve)
										ImGui::PopStyleColor();
									if (ImGui::IsItemHovered())
									{
										if (itFont.second->m_NeedFilePathResolve)
											ImGui::SetTooltip("Font file Not Found\nClick for search the good file");
										else
											ImGui::SetTooltip("Click for search an alternative file\nbut keep selected glyphs");
									}
									ImGui::PopID();
								}

								if (sel)
								{
									if (!itFont.second->m_NeedFilePathResolve)
									{
										SelectFont(vProjectFile, itFont.second);
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
					vProjectFile->m_SelectedFont->DrawInfos(vProjectFile);

					LayoutManager::Instance()->DrawWidgets(vProjectFile, 0, "");

					if (ImGui::BeginFramedGroup("Glyphs"))
					{
						maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f;
						mrw = maxWidth / 4.0f - ImGui::GetStyle().FramePadding.x;

						ImGui::PushItemWidth(mrw);
						ImGui::RadioButtonLabeled("Zoom", "Zoom Each Glyphs for best fit", &vProjectFile->m_ZoomGlyphs);
						ImGui::SameLine();
						ImGui::RadioButtonLabeled("Base", "Show the base line of the font", &vProjectFile->m_ShowBaseLine, vProjectFile->m_ZoomGlyphs);
						ImGui::SameLine();
						ImGui::RadioButtonLabeled("OrgX", "Show the Origin X of the glyph", &vProjectFile->m_ShowOriginX, vProjectFile->m_ZoomGlyphs);
						ImGui::SameLine();
						ImGui::RadioButtonLabeled("AdvX", "Show the Advance X of the glyph", &vProjectFile->m_ShowAdvanceX, vProjectFile->m_ZoomGlyphs);
						ImGui::PopItemWidth();
						
						ImGui::EndFramedGroup(true);
					}

					if (ImGui::BeginFramedGroup("Font Layout"))
					{
						if (vProjectFile->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
						{
							const auto style = &ImGui::GetStyle();
							static float radioButtonWidth = ImGui::GetFrameHeight();
							float aw = ImGui::GetContentRegionAvail().x - style->ItemSpacing.x - radioButtonWidth;

							bool change = false;
							if (ImGui::SliderIntDefaultCompact(aw, "Glyph Count X", &vProjectFile->m_Preview_Glyph_CountX, 
								50, 1, defaultProjectValues.m_Preview_Glyph_CountX))
							{
								change = true;
								vProjectFile->m_GlyphSizePolicyChangeFromWidgetUse = true;
								vProjectFile->m_GlyphDisplayTuningMode = GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT;
								vProjectFile->m_Preview_Glyph_CountX = ct::maxi(vProjectFile->m_Preview_Glyph_CountX, 1); // can prevent bugs (like div by zero) everywhere when user input value
							}
							ImGui::SameLine();
							ImGui::RadioButtonLabeled_BitWize<GlyphDisplayTuningModeFlags>(
								ICON_IGFS_USED "##GlypCountIsMaster",
								ICON_IGFS_NOT_USED "##GlypCountIsMaster",
								"Apply Glyph Count Policy when Resized",
								&vProjectFile->m_GlyphDisplayTuningMode, GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT, radioButtonWidth, true);
							radioButtonWidth = ImGui::GetItemRectSize().x;

							if (ImGui::SliderFloatDefaultCompact(aw, "Glyph Width", &vProjectFile->m_Preview_Glyph_Width, 10.0f,
								300.0f, defaultProjectValues.m_Preview_Glyph_Width))
							{
								change = true;
								vProjectFile->m_GlyphSizePolicyChangeFromWidgetUse = true;
								vProjectFile->m_GlyphDisplayTuningMode = GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE;
								vProjectFile->m_Preview_Glyph_Width = ct::maxi(vProjectFile->m_Preview_Glyph_Width, 10.0f); // can prevent bugs (like div by zero) everywhere when user input value
							}

							ImGui::SameLine();
							ImGui::RadioButtonLabeled_BitWize<GlyphDisplayTuningModeFlags>(
								ICON_IGFS_USED "##GlypSizeIsMaster",
								ICON_IGFS_NOT_USED "##GlypSizeIsMaster",
								"Policy to be applied When resized :\n1) Glyph Width Policy\n2) Glyph Count Policy",
								&vProjectFile->m_GlyphDisplayTuningMode, GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE, radioButtonWidth, true);
							
							ImGui::FramedGroupSeparator();

							ImGui::Checkbox("Differential Colorations", &vProjectFile->m_ShowRangeColoring);
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
//// PRIVATE : ACTIONS, DIALOGS ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void ParamsPane::Action_Menu_OpenFont()
{
/*
open font :
	-	add action : open font
*/
	MainFrame::Instance()->GetActionSystem()->Clear();
	MainFrame::Instance()->GetActionSystem()->Add([this]()
		{
			ImGuiFileDialog::Instance()->OpenModal(
				"OpenFontDlg", "Open Font File", "Font File (*.ttf *.otf){.ttf,.otf},All Files (*.*){.*}", ".", 0);
			return true;
		});
}

void ParamsPane::Action_Menu_CloseFont()
{
/*
close font :
-	ok :
	-	glyphs selected :
		-	add action : show a confirmation dialog (ok/cancel for lose glyph selection)
		-	add action : close font
	-	no glyph selected :
		-	add action : close font
-	cancel :
	-	clear actions
*/
	MainFrame::Instance()->GetActionSystem()->Clear();
	Open_ConfirmToCloseFont_Dialog();
	MainFrame::Instance()->GetActionSystem()->Add([this]()
		{
			return Display_ConfirmToCloseFont_Dialog();
		});

	MainFrame::Instance()->GetActionSystem()->Add([this]()
		{
			// close font
			auto prj = MainFrame::Instance()->GetProject();
			if (prj)
			{
				if (prj->m_SelectedFont)
				{
					prj->m_SelectedFont->Clear();
					prj->m_Fonts.erase(prj->m_SelectedFont->m_FontFileName);
					if (!prj->m_Fonts.empty())
					{
						SelectFont(prj, prj->m_Fonts.begin()->second);
					}
					else
					{
						SelectFont(prj, nullptr);
					}
					prj->SetProjectChange();
				}
			}
			
			return true;
		});
}

void ParamsPane::Action_Cancel()
{
	MainFrame::Instance()->GetActionSystem()->Clear();
}

void ParamsPane::Open_ConfirmToCloseFont_Dialog()
{
	m_Show_ConfirmToCloseFont_Dialog = true;
}

void ParamsPane::Close_ConfirmToCloseFont_Dialog()
{
	m_Show_ConfirmToCloseFont_Dialog = false;
}

bool ParamsPane::Display_ConfirmToCloseFont_Dialog()
{
	bool res = false;

	if (m_Show_ConfirmToCloseFont_Dialog)
	{
		ImGui::OpenPopup("Are you sure to close this font ?");
		if (ImGui::BeginPopupModal("Are you sure to close this font ?", (bool*)0,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
		{
			ImGui::SetWindowPos(MainFrame::Instance()->m_DisplaySize * 0.5f - 
				ImGui::GetWindowSize() * 0.5f); // put the dlg in center of the frame

			ImGui::Text("You will lose your glyph selection / tuning");

			/*
			confirmation dialog for close font :
			-	ok :
				-	quit the dialog
			-	cancel :
				-	clear actions
			*/
			if (ImGui::Button("Confirm"))
			{
				res = true; // quit the action
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				Action_Cancel();
			}

			ImGui::EndPopup();
		}

		return res; // quit if true, else continue on the next frame
	}
	
	return true; // quit the action
}

//////////////////////////////////////////////////////////////////////////////
//// FONT ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void ParamsPane::OpenFonts(ProjectFile *vProjectFile, const std::map<std::string, std::string>& vFontFilePathNames)
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

void ParamsPane::OpenFont(ProjectFile *vProjectFile, const std::string& vFontFilePathName, bool vUpdateCount)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		auto ps = FileHelper::Instance()->ParsePathFileName(vFontFilePathName);
		if (ps.isOk)
		{
			std::string fontName = ps.name + "." + ps.ext;

			if (vProjectFile->m_Fonts.find(fontName) == vProjectFile->m_Fonts.end())
			{
				// create font 
				vProjectFile->m_Fonts[fontName] = std::make_shared<FontInfos>();
			}
						
			auto font = vProjectFile->m_Fonts[fontName];
			if (font)
			{
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
}

void ParamsPane::SelectFont(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos)
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

std::string ParamsPane::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + "<sourcefontpane>\n";

	str += vOffset + "\t<glyphsizepolicy_count>" + ct::toStr(m_GlyphSize_Policy_Count) + "</glyphsizepolicy_count>\n";
	str += vOffset + "\t<glyphsizepolicy_width>" + ct::toStr(m_GlyphSize_Policy_Width) + "</glyphsizepolicy_width>\n";

	str += vOffset + "</sourcefontpane>\n";

	return str;
}

bool ParamsPane::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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
		if (strName == "glyphsizepolicy_count")
			m_GlyphSize_Policy_Count = ct::ivariant(strValue).GetI();
		else if (strName == "glyphsizepolicy_width")
			m_GlyphSize_Policy_Width = ct::fvariant(strValue).GetF();
	}

	return true;
}