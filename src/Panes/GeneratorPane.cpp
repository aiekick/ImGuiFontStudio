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

#include "GeneratorPane.h"

#include "MainFrame.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cTools.h>
#include <FileHelper.h>
#include "Gui/GuiLayout.h"
#include "Gui/ImGuiWidgets.h"
#include "Helper/Messaging.h"
#include "Helper/SelectionHelper.h"
#include "Helper/ImGuiThemeHelper.h"
#include "Panes/FinalFontPane.h"
#include "Panes/SourceFontPane.h"
#include "Project/ProjectFile.h"
#include "Generator/Generator.h"

#include <cinttypes> // printf zu

static int GeneratorPane_WidgetId = 0;

GeneratorPane::GeneratorPane()
{
	
}

GeneratorPane::~GeneratorPane()
{
	
}

///////////////////////////////////////////////////////////////////////////////////
//// STATUS FLAGS /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void GeneratorPane::AllowStatus(GeneratorStatusFlags vGeneratorStatusFlags)
{
	m_GeneratorStatusFlags = (GeneratorStatusFlags)(m_GeneratorStatusFlags | vGeneratorStatusFlags); // add
}

void GeneratorPane::ProhibitStatus(GeneratorStatusFlags vGeneratorStatusFlags)
{
	m_GeneratorStatusFlags = (GeneratorStatusFlags)(m_GeneratorStatusFlags & ~vGeneratorStatusFlags); // remove flag
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int GeneratorPane::DrawGeneratorPane(ProjectFile *vProjectFile, int vWidgetId)
{
	GeneratorPane_WidgetId = vWidgetId;

	if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_GENERATOR)
	{
		if (ImGui::Begin<PaneFlags>(GENERATOR_PANE,
			&GuiLayout::m_Pane_Shown, PaneFlags::PANE_GENERATOR,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile && vProjectFile->IsLoaded())
			{
				if (SourceFontPane::Instance()->m_FontPaneFlags & SourceFontPane::SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
				{
					SelectionHelper::Instance()->DrawMenu(vProjectFile);

					GeneratorPane_WidgetId = DrawFontsGenerator(vProjectFile, GeneratorPane_WidgetId);
				}
			}
		}

		ImGui::End();
	}

	return GeneratorPane_WidgetId;
}

int GeneratorPane::DrawFontsGenerator(ProjectFile *vProjectFile, int vWidgetId)
{
	if (ImGui::BeginFramedGroup("Font Generation"))
	{
		if (vProjectFile->m_CurrentFont)
		{
			bool btnClick = false;
			std::string exts;

#ifdef _DEBUG
			if (ImGui::Button("Quick Font Current"))
			{
				vProjectFile->m_GenMode = (GenModeFlags)0;
				vProjectFile->AddGenMode(GenModeFlags::GENERATOR_MODE_CURRENT_FONT); // font + header
				vProjectFile->AddGenMode(GenModeFlags::GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES);
				std::string path = FileHelper::Instance()->GetAppPath() + "/exports";
				path = FileHelper::Instance()->CorrectFilePathName(path);
				FileHelper::Instance()->CreateDirectoryIfNotExist(path);
				Generator::Instance()->Generate(path, "test.ttf", vProjectFile);
			}
			if (ImGui::Button("Quick Font Merged"))
			{
				vProjectFile->m_GenMode = (GenModeFlags)0;
				vProjectFile->AddGenMode(GenModeFlags::GENERATOR_MODE_MERGED_FONT); // font + header
				vProjectFile->AddGenMode(GenModeFlags::GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES);
				std::string path = FileHelper::Instance()->GetAppPath() + "/exports";
				path = FileHelper::Instance()->CorrectFilePathName(path);
				FileHelper::Instance()->CreateDirectoryIfNotExist(path);
				Generator::Instance()->Generate(path, "test.ttf", vProjectFile);
			}
#endif
			bool change = false;
			ImGui::Text("Modes : ");
			ImGui::Indent();
			{
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Current", "Current Font",
					&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_CURRENT, 50.0f, true, true,
					GenModeFlags::GENERATOR_MODE_RADIO_CUR_BAT_MER);
				if (vProjectFile->m_Fonts.size() > 1)
				{
					ImGui::SameLine();
					change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Batch", "Font by Font",
						&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_BATCH, 50.0f, true, true,
						GenModeFlags::GENERATOR_MODE_RADIO_CUR_BAT_MER);

					if (vProjectFile->m_CountFontWithSelectedGlyphs > 1 &&
						m_GeneratorStatusFlags & GeneratorStatusFlags::GENERATOR_STATUS_FONT_MERGE_ALLOWED)
					{
						ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Merged", "Fonts Merged in one",
							&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_MERGED, 50.0f, true, true,
							GenModeFlags::GENERATOR_MODE_RADIO_CUR_BAT_MER);
					}
					else // select batch only
					{
						vProjectFile->RemoveGenMode(GenModeFlags::GENERATOR_MODE_MERGED);
						vProjectFile->AddGenMode(GenModeFlags::GENERATOR_MODE_BATCH);
					}
				}
				else // select current only
				{
					vProjectFile->RemoveGenMode(GenModeFlags::GENERATOR_MODE_MERGED);
					vProjectFile->RemoveGenMode(GenModeFlags::GENERATOR_MODE_BATCH);
					vProjectFile->AddGenMode(GenModeFlags::GENERATOR_MODE_CURRENT);
				}
			}
			ImGui::Unindent();

			ImGui::Text("Features : ");
			ImGui::Indent();
			{
				if (m_GeneratorStatusFlags & GeneratorStatusFlags::GENERATOR_STATUS_FONT_HEADER_GENERATION_ALLOWED)
				{
					change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Header", "Header File",
						&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_HEADER, 50.0f);
					ImGui::SameLine();
				}
				else
				{
					vProjectFile->RemoveGenMode(GenModeFlags::GENERATOR_MODE_HEADER);
				}
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Font", "Font File",
					&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_FONT, 50.0f, false, false,
					GenModeFlags::GENERATOR_MODE_RADIO_FONT_CPP);
				ImGui::SameLine();
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Cpp", "Source File for c++\n\twith font as a bytes array",
					&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_CPP, 50.0f, false, false,
					GenModeFlags::GENERATOR_MODE_RADIO_FONT_CPP);
			}
			ImGui::Unindent();

			ImGui::Text("Settings : ");
			if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_HEADER))
			{
				ImGui::Indent();
				{
					ImGui::Text("Header : ");
					ImGui::Indent();
					{
						ImGui::Text("Order Glyphs by :");
						change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("CodePoint", "order by glyph CodePoint",
							&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_CODEPOINT, 50.0f, true, true,
							GenModeFlags::GENERATOR_MODE_RADIO_CDP_NAMES);
						ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Names", "order by glyph Name",
							&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_NAMES, 50.0f, true, true,
							GenModeFlags::GENERATOR_MODE_RADIO_CDP_NAMES);
					}
					ImGui::Unindent();
				}
				ImGui::Unindent();
			}
			if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_FONT))
			{
				ImGui::Indent();
				{
					ImGui::Text("Font : ");
					ImGui::Indent();
					{
						change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Export Names", "export glyph names in font file (increase size)",
							&vProjectFile->m_GenMode, GenModeFlags::GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES, 50.0f);
					}
					ImGui::Unindent();
				}
				ImGui::Unindent();
			}
			
			if (CheckGeneratioConditions(vProjectFile))
			{
				ImGui::Indent();

				if (ImGui::Button(ICON_IGFS_GENERATE " Generate"))
				{
					btnClick = true;
					if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_FONT)) exts = ".ttf\0\0";
					else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_CPP)) exts = ".cpp\0\0";
					else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_HEADER)) exts = ".h\0\0";
				}

				ImGui::Unindent();
			}

			if (change)
			{
				vProjectFile->SetProjectChange();
			}

			if (btnClick)
			{
				static char extTypes[512] = "\0";
#ifdef MSVC
				strncpy_s(extTypes, exts.c_str(), ct::mini((int)exts.size(), 511));
#else
				strncpy(extTypes, exts.c_str(), exts.size());
#endif
				ImGuiFileDialog::Instance()->OpenModal(
					"GenerateFileDlg",
					"Location and name where create the file", extTypes, ".",
					vProjectFile->m_CurrentFont->m_FontFileName,
					std::bind(&GeneratorPane::GeneratorFileDialogPane, this,
						std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
					200, 1, vProjectFile);
			}
		}
		else
		{
			ImGui::Text("No Selected Font.");
		}

		ImGui::EndFramedGroup(true);
	}

	return vWidgetId;
}

bool GeneratorPane::CheckGeneratioConditions(ProjectFile *vProjectFile)
{
	bool res = true;

	if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_HEADER) ||
		vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_FONT) ||
		vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_CPP))
	{
		
	}
	else
	{
		res = false;
		ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "Can't generate.\n\tSelect one feature at least");
	}

	if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_MERGED) &&
		!(vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_FONT) ||
			vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_CPP)))
	{
		res = false;
		ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "Merged mode require the\ngeneration of font or cpp.\nPlease Select one of\nthese two at least");
	}

	if (res)
	{
		size_t errorsCount = 0;
		
		ImGui::Text("Font Status :");
		ImGui::Indent();
		{
			vProjectFile->m_CountFontWithSelectedGlyphs = 0;
			for (auto &font : vProjectFile->m_Fonts)
			{
				if (font.second.m_CodePointInDoubleFound || font.second.m_NameInDoubleFound)
				{
					ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "%s : NOK", font.second.m_FontFileName.c_str());
					errorsCount++;
				}
				else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_MERGED) && font.second.m_SelectedGlyphs.size() == 0)
				{
					ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "%s : NOK", font.second.m_FontFileName.c_str());
					ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "No Glyphs are selected.\n\tYou need it in Merged mode");
					errorsCount++;
				}
				else
				{
					ImGui::TextColored(ImGuiThemeHelper::Instance()->goodColor, "%s : OK", font.second.m_FontFileName.c_str());
					ImGui::Indent();
					if (font.second.m_SelectedGlyphs.size() == 0) // no glyphs to extract, we wille extract alls, current and batch only
					{
						ImGui::TextColored(ImGuiThemeHelper::Instance()->goodColor, "No Glyphs are selected.\n\tAll font glyphs will be exported");
					}
					else
					{
						ImGui::TextColored(ImGuiThemeHelper::Instance()->goodColor, "%zu Glyphs selected", font.second.m_SelectedGlyphs.size());
						vProjectFile->m_CountFontWithSelectedGlyphs++;
					}
					ImGui::Unindent();
				}
			}

			ImGui::Text("%zu Glyphs selected\n\tin %zu fonts",
				vProjectFile->m_CountSelectedGlyphs,
				vProjectFile->m_CountFontWithSelectedGlyphs);
		}
		ImGui::Unindent();

		ImGui::Spacing();

		ImGui::Text("Generation :");
		ImGui::Indent();
		if (errorsCount < vProjectFile->m_Fonts.size())
		{
			if (errorsCount > 0)
			{
				ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "Can partially generate\n\tCheck status bar");
			}
			else
			{
				ImGui::TextColored(ImGuiThemeHelper::Instance()->goodColor, "Can fully generate");
			}

			if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_MERGED))
			{
				ImGui::TextColored(ImGuiThemeHelper::Instance()->goodColor,
					"The selected font\n\tscale / bounding box\n\twill be used for merge in\n\tall other font glyphs");
			}
		}
		else
		{
			ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "Can't generate\n\tCheck status bar");
			res = false;
		}

		ImGui::Unindent();
	}

	return res;
}

// file dialog pane
void GeneratorPane::GeneratorFileDialogPane(std::string /*vFilter*/, UserDatas vUserDatas, 
	bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	cAssert(vCantContinue != 0, "ImGuiFileDialog Pane param vCantContinue is NULL");

	bool canContinue = true;

	auto prj = (ProjectFile*)vUserDatas;
	if (prj)
	{
		if (prj->IsGenMode(GenModeFlags::GENERATOR_MODE_HEADER))
		{
			#define PREFIX_MAX_SIZE 49
			static char prefixBuffer[PREFIX_MAX_SIZE + 1] = "\0";

			if (prj->IsGenMode(GenModeFlags::GENERATOR_MODE_CURRENT))
			{
				if (prj->m_CurrentFont)
				{
					bool cond = !prj->m_CurrentFont->m_FontPrefix.empty();
					snprintf(prefixBuffer, PREFIX_MAX_SIZE, "%s", prj->m_CurrentFont->m_FontPrefix.c_str());
					ImGui::TextWrapped("Header Prefix for\n\t%s :", prj->m_CurrentFont->m_FontFileName.c_str());
					if (ImGui::InputText_Validation("##FontPrefix", prefixBuffer, PREFIX_MAX_SIZE,
						&cond, "You must Define a\nfont prefix for continue"))
					{
						prj->m_CurrentFont->m_FontPrefix = std::string(prefixBuffer);
						prj->SetProjectChange();
					}
					canContinue = !prj->m_CurrentFont->m_FontPrefix.empty();
				}
				else
				{
					// Normally, cant have this issue, but kepp for fewer code modification
					ImGui::Text("No Font Selected. Can't continue");
					canContinue = false;
				}
			}
			else if (prj->IsGenMode(GenModeFlags::GENERATOR_MODE_BATCH))
			{
				canContinue = true;
				std::map<std::string, int> prefixs;
				for (auto &font : prj->m_Fonts)
				{
					snprintf(prefixBuffer, PREFIX_MAX_SIZE, "%s", font.second.m_FontPrefix.c_str());
					
					bool cond = !font.second.m_FontPrefix.empty(); // not empty
					if (prefixs.find(font.second.m_FontPrefix) == prefixs.end())
					{
						prefixs[font.second.m_FontPrefix] = 1;
					}
					else
					{
						cond &= (prefixs[font.second.m_FontPrefix] == 0); // must be unique
					}
					ImGui::TextWrapped("Header Prefix for\n\t%s :", font.second.m_FontFileName.c_str());

					ImGui::PushID(&font);
					bool res = ImGui::InputText_Validation("##FontPrefix", prefixBuffer, PREFIX_MAX_SIZE,
						&cond, "You must Define a\nfont prefix and unique for continue");
					ImGui::PopID();
					if (res)
					{
						font.second.m_FontPrefix = std::string(prefixBuffer);
						prj->SetProjectChange();
					}
					prefixs[font.second.m_FontPrefix]++;
					canContinue &= cond;
				}
			}
			else if (prj->IsGenMode(GenModeFlags::GENERATOR_MODE_MERGED))
			{
				bool cond = !prj->m_MergedFontPrefix.empty();
				
				snprintf(prefixBuffer, PREFIX_MAX_SIZE, "%s", prj->m_MergedFontPrefix.c_str());
				ImGui::Text("Header Font Prefix :");
				if (ImGui::InputText_Validation("##FontPrefix", prefixBuffer, PREFIX_MAX_SIZE,
					&cond, "You must Define a\nfont prefix for continue"))
				{
					prj->m_MergedFontPrefix = prefixBuffer;
					prj->SetProjectChange();
				}

				canContinue = !prj->m_MergedFontPrefix.empty();
			}
			
		}

		if (vCantContinue)
		{
			*vCantContinue = canContinue;
		}
	}
}

void GeneratorPane::DrawDialosAndPopups(ProjectFile *vProjectFile)
{
	ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
	ImVec2 max = MainFrame::Instance()->m_DisplaySize;

	if (ImGuiFileDialog::Instance()->FileDialog("GenerateFileDlg", ImGuiWindowFlags_NoDocking, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk)
		{
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
			Generator::Instance()->Generate(filePath, fileName, vProjectFile);
		}

		ImGuiFileDialog::Instance()->CloseDialog("GenerateFileDlg");
	}
}

