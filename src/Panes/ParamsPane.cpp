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

bool ParamsPane::Init()
{
	return true;
}

void ParamsPane::Unit()
{

}

int ParamsPane::DrawPanes(int vWidgetId, std::string vUserDatas)
{
	m_PaneWidgetId = vWidgetId;

	DrawParamsPane();

	return m_PaneWidgetId;
}

void ParamsPane::DrawDialogsAndPopups(std::string vUserDatas)
{
	if (ProjectFile::Instance()->IsLoaded())
	{
		ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
		ImVec2 max = MainFrame::Instance()->m_DisplaySize;

		if (ImGuiFileDialog::Instance()->Display("OpenFontDlg", ImGuiWindowFlags_NoDocking, min, max))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				OpenFonts(ImGuiFileDialog::Instance()->GetSelection());
			}

			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("SaveFontToPictureFile", ImGuiWindowFlags_NoDocking, min, max))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				auto tex = dynamic_cast<std::shared_ptr<TextureObject>*>((std::shared_ptr<TextureObject>*)ImGuiFileDialog::Instance()->GetUserDatas());
				if (tex && *tex)
				{
					auto win = MainFrame::Instance()->GetGLFWwindow();
					auto file = ImGuiFileDialog::Instance()->GetFilePathName();
#if VULKAN
					VkCommandPool command_pool = MainFrame::sMainWindowData.Frames[MainFrame::sMainWindowData.FrameIndex].CommandPool;
					if (TextureHelper::SaveTextureToPng(command_pool, win, file.c_str(), *tex))
#else
					if (TextureHelper::SaveTextureToPng(win, file.c_str(), *tex))
#endif
					{
						FileHelper::Instance()->OpenFile(file);
					}
				}
			}

			ImGuiFileDialog::Instance()->Close();
		}
	}
}

int ParamsPane::DrawWidgets(int vWidgetId, std::string vUserDatas)
{
	UNUSED(vUserDatas);

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : PANES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void ParamsPane::DrawParamsPane()
{
	if (LayoutManager::Instance()->m_Pane_Shown & m_PaneFlag)
	{
		if (ImGui::BeginFlag<PaneFlags>(m_PaneName,
			&LayoutManager::Instance()->m_Pane_Shown, m_PaneFlag,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (ProjectFile::Instance()->IsLoaded())
			{
				if (ImGui::BeginFramedGroup("Font File"))
				{
					float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x;
					float mrw = maxWidth / 2.0f;

					if (ImGui::ContrastedButton(ICON_IGFS_FOLDER_OPEN " Open Font", nullptr, nullptr, mrw))
					{
						Action_Menu_OpenFont();
					}

					if (!ProjectFile::Instance()->m_Fonts.empty())
					{
						ImGui::SameLine();

						if (ImGui::ContrastedButton(ICON_IGFS_DESTROY " Close Font", nullptr, nullptr, mrw))
						{
							Action_Menu_CloseFont();
						}

						ImGui::FramedGroupSeparator();

						ImGui::FramedGroupText("Opened Fonts");

						static int _countLines = 7;
						//ImGui::SliderIntDefaultCompact(ImGui::GetContentRegionAvail().x, "Count Lines", &_countLines, 0, 100, 7);

						static ImGuiTableFlags flags =
							ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
							ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
							ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_Borders;
						if (ImGui::BeginTable("##fileTable", 2, flags, ImVec2(-1.0f, _countLines * ImGui::GetTextLineHeightWithSpacing())))
						{
							ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
							ImGui::TableSetupColumn("Font Files", ImGuiTableColumnFlags_WidthStretch, -1, 0);
							ImGui::TableSetupColumn("Edit", ImGuiTableColumnFlags_WidthFixed, 32, 1);
							ImGui::TableHeadersRow();
							
							for (const auto& itFont : ProjectFile::Instance()->m_Fonts) // importnat need to sahre by address for the userdatas when need to resolve
							{
								bool sel = false;

								ImGui::TableNextRow();
								
								if (ImGui::TableSetColumnIndex(0)) // first column
								{
									if (itFont.second->m_NeedFilePathResolve)
										ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.8f, 0.5f, 0.2f, 0.6f));

									ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
									selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
									sel = ImGui::Selectable(itFont.first.c_str(), ProjectFile::Instance()->m_SelectedFont == itFont.second, 
										selectableFlags, ImVec2(0, 0));

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
									ImGui::PushID(++m_PaneWidgetId);
									if (itFont.second->m_NeedFilePathResolve)
										ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 0.8f));
									if (ImGui::TransparentButton((itFont.second->m_NeedFilePathResolve ? ICON_IGFS_WARNING : ICON_IGFS_EDIT), 
										ImVec2(24, 0)))
									{
										std::string label;
										if (itFont.second->m_NeedFilePathResolve)
											label = "Search the good font file path " + itFont.first;
										else
											label = "Search an alternative font file " + itFont.first;

										std::string path = ".";
										if (ProjectFile::Instance()->IsLoaded())
											path = ProjectFile::Instance()->m_ProjectFilePath;
										ImGuiFileDialog::Instance()->OpenModal(
											"SolveBadFilePathName",
											label.c_str(), "Font File (*.ttf *.otf){.ttf,.otf}", path,
											itFont.first.c_str(), 1, IGFDUserDatas(itFont.first.c_str()));
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
									//if (!itFont.second->m_NeedFilePathResolve)
									{
										SelectFont(itFont.second);
									}
								}
							}
							ImGui::EndTable();
						}
					}

					ImGui::EndFramedGroup();
				}

				if (ProjectFile::Instance()->m_SelectedFont)
				{
					ProjectFile::Instance()->m_SelectedFont->DrawInfos();

					LayoutManager::Instance()->DrawWidgets(0, "");

					if (ImGui::BeginFramedGroup("Glyphs"))
					{
						const float mrw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) * 0.25f;

						bool change = false;

						change |= ImGui::RadioButtonLabeled(mrw, "Zoom", "Zoom Each Glyphs for best fit", ProjectFile::Instance()->m_ZoomGlyphs);
						ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled(mrw, "Base", "Show the base line of the font", ProjectFile::Instance()->m_ShowBaseLine, ProjectFile::Instance()->m_ZoomGlyphs);
						ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled(mrw, "OrgX", "Show the Origin X of the glyph", ProjectFile::Instance()->m_ShowOriginX, ProjectFile::Instance()->m_ZoomGlyphs);
						ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled(mrw, "AdvX", "Show the Advance X of the glyph", ProjectFile::Instance()->m_ShowAdvanceX, ProjectFile::Instance()->m_ZoomGlyphs);
						
						if (change)
						{
							ProjectFile::Instance()->SetProjectChange();
						}

						ImGui::EndFramedGroup();
					}

					if (ImGui::BeginFramedGroup("Font Layout"))
					{
						if (ProjectFile::Instance()->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
						{
							static float radioButtonWidth = ImGui::GetFrameHeight();
							const float aw = ImGui::GetContentRegionAvail().x - radioButtonWidth - ImGui::GetStyle().ItemSpacing.x * 3.0f;

							bool change = false;
							if (ImGui::SliderIntDefaultCompact(aw, "Glyph Count X", &ProjectFile::Instance()->m_Preview_Glyph_CountX, 
								50, 1, defaultProjectValues.m_Preview_Glyph_CountX))
							{
								change = true;
								ProjectFile::Instance()->m_GlyphSizePolicyChangeFromWidgetUse = true;
								ProjectFile::Instance()->m_GlyphDisplayTuningMode = GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT;
								ProjectFile::Instance()->m_Preview_Glyph_CountX = ct::maxi(ProjectFile::Instance()->m_Preview_Glyph_CountX, 1); // can prevent bugs (like div by zero) everywhere when user input value
							}
							ImGui::SameLine();
							if (ImGui::RadioButtonLabeled_BitWize<GlyphDisplayTuningModeFlags>(radioButtonWidth,
								ICON_IGFS_USED "##GlypCountIsMaster",
								ICON_IGFS_NOT_USED "##GlypCountIsMaster",
								"Apply Glyph Count Policy when Resized",
								&ProjectFile::Instance()->m_GlyphDisplayTuningMode, GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT, true))
							{
								ProjectFile::Instance()->SetProjectChange();
							}
							radioButtonWidth = ImGui::GetItemRectSize().x;

							if (ImGui::SliderFloatDefaultCompact(aw, "Glyph Width", &ProjectFile::Instance()->m_Preview_Glyph_Width, 10.0f,
								300.0f, defaultProjectValues.m_Preview_Glyph_Width))
							{
								change = true;
								ProjectFile::Instance()->m_GlyphSizePolicyChangeFromWidgetUse = true;
								ProjectFile::Instance()->m_GlyphDisplayTuningMode = GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE;
								ProjectFile::Instance()->m_Preview_Glyph_Width = ct::maxi(ProjectFile::Instance()->m_Preview_Glyph_Width, 10.0f); // can prevent bugs (like div by zero) everywhere when user input value
							}

							ImGui::SameLine();
							if (ImGui::RadioButtonLabeled_BitWize<GlyphDisplayTuningModeFlags>(radioButtonWidth,
								ICON_IGFS_USED "##GlypSizeIsMaster",
								ICON_IGFS_NOT_USED "##GlypSizeIsMaster",
								"Policy to be applied When resized :\n1) Glyph Width Policy\n2) Glyph Count Policy",
								&ProjectFile::Instance()->m_GlyphDisplayTuningMode, GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE, true))
							{
								ProjectFile::Instance()->SetProjectChange();
							}
							
							ImGui::FramedGroupSeparator();

							ImGui::Checkbox("Differential Colorations", &ProjectFile::Instance()->m_ShowRangeColoring);
							if (ProjectFile::Instance()->IsRangeColoringShown())
							{
								change |= ImGui::SliderFloatDefaultCompact(-1.0f, "H x", &ProjectFile::Instance()->m_RangeColoringHash.x, 0, 50, defaultProjectValues.m_RangeColoringHash.x);
								change |= ImGui::SliderFloatDefaultCompact(-1.0f, "H y", &ProjectFile::Instance()->m_RangeColoringHash.y, 0, 50, defaultProjectValues.m_RangeColoringHash.y);
								change |= ImGui::SliderFloatDefaultCompact(-1.0f, "H z", &ProjectFile::Instance()->m_RangeColoringHash.z, 0, 50, defaultProjectValues.m_RangeColoringHash.z);
								//change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Alpha", ProjectFile::Instance()->m_RangeColoringHash.w, 0, 1, defaultProjectValues.m_RangeColoringHash.w);
							}

							if (change)
								ProjectFile::Instance()->SetProjectChange();
						}

						ImGui::EndFramedGroup();
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
			std::string path = ".";
			if (ProjectFile::Instance()->IsLoaded())
				path = ProjectFile::Instance()->m_ProjectFilePath;
			ImGuiFileDialog::Instance()->OpenModal(
				"OpenFontDlg", "Open Font File", "Font File (*.ttf *.otf){.ttf,.otf},All Files (*.*){.*}", path, 0);
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
			auto prj = ProjectFile::Instance();
			if (prj)
			{
				if (prj->m_SelectedFont)
				{
					prj->m_SelectedFont->Clear();
					prj->m_Fonts.erase(prj->m_SelectedFont->m_FontFileName);
					if (!prj->m_Fonts.empty())
					{
						SelectFont(prj->m_Fonts.begin()->second);
					}
					else
					{
						SelectFont(nullptr);
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
			ImGuiWindowFlags_NoResize))
		{
			ImGui::Text("You will lose your glyph selection / tuning");

			/*
			confirmation dialog for close font :
			-	ok :
				-	quit the dialog
			-	cancel :
				-	clear actions
			*/
			if (ImGui::ContrastedButton("Confirm"))
			{
				res = true; // quit the action
			}
			ImGui::SameLine();
			if (ImGui::ContrastedButton("Cancel"))
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

void ParamsPane::OpenFonts( const std::map<std::string, std::string>& vFontFilePathNames)
{
	if (ProjectFile::Instance()->IsLoaded())
	{
		bool res = false;
		for (auto & it : vFontFilePathNames)
		{
			res |= OpenFont(it.second, false);
		}

		if (res) // si au moins est bon
			ProjectFile::Instance()->UpdateCountSelectedGlyphs();
	}
}

bool ParamsPane::OpenFont( const std::string& vFontFilePathName, bool vUpdateCount)
{
	bool res = false;

	if (ProjectFile::Instance()->IsLoaded())
	{
		auto ps = FileHelper::Instance()->ParsePathFileName(vFontFilePathName);
		if (ps.isOk)
		{
			std::string fontName = ps.name + "." + ps.ext;

			if (ProjectFile::Instance()->m_Fonts.find(fontName) == ProjectFile::Instance()->m_Fonts.end())
			{
				// create font 
				ProjectFile::Instance()->m_Fonts[fontName] = FontInfos::Create();
			}
						
			auto font = ProjectFile::Instance()->m_Fonts[fontName];
			if (font)
			{
				if (font->LoadFont(vFontFilePathName))
				{
					if (ProjectFile::Instance()->m_FontToMergeIn.empty() ||
						ProjectFile::Instance()->m_FontToMergeIn == font->m_FontFileName)
					{
						SelectFont(font);
					}

					if (vUpdateCount)
						ProjectFile::Instance()->UpdateCountSelectedGlyphs();

					res = true;
				}
			}
		}
	}

	return res;
}

void ParamsPane::SelectFont( std::shared_ptr<FontInfos> vFontInfos)
{
	if (ProjectFile::Instance()->IsLoaded())
	{
		ProjectFile::Instance()->m_SelectedFont = vFontInfos;
		if (vFontInfos.use_count())
		{
			ProjectFile::Instance()->m_FontToMergeIn = vFontInfos->m_FontFileName;
		}
		ProjectFile::Instance()->SetProjectChange();
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