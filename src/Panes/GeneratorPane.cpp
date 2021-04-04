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

#include <MainFrame.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Panes/Manager/LayoutManager.h>
#include <Gui/ImGuiWidgets.h>
#include <Helper/Messaging.h>
#include <Helper/SelectionHelper.h>
#include <Helper/ImGuiThemeHelper.h>
#include <Panes/FinalFontPane.h>
#include <Panes/SourceFontPane.h>
#include <Project/ProjectFile.h>
#include <Generator/Generator.h>
#include <Project/FontInfos.h>

#include <cinttypes> // printf zu

static ProjectFile defaultProjectFile;
static FontInfos defaultFontInfos;

GeneratorPane::GeneratorPane() = default;
GeneratorPane::~GeneratorPane() = default;

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
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void GeneratorPane::Init()
{
	
}

void GeneratorPane::Unit()
{

}

int GeneratorPane::DrawPanes(ProjectFile* vProjectFile, int vWidgetId)
{
	paneWidgetId = vWidgetId;

	DrawGeneratorPane(vProjectFile);

	return paneWidgetId;
}

void GeneratorPane::DrawDialogsAndPopups(ProjectFile* vProjectFile)
{
	if (vProjectFile)
	{
		ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
		ImVec2 max = MainFrame::Instance()->m_DisplaySize;

		if (ImGuiFileDialog::Instance()->Display("GenerateFileDlg", ImGuiWindowFlags_NoDocking, min, max))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				vProjectFile->m_LastGeneratedPath = ImGuiFileDialog::Instance()->GetCurrentPath();
				vProjectFile->m_LastGeneratedFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
				Generator::Instance()->Generate(vProjectFile);
			}

			ImGuiFileDialog::Instance()->Close();
		}
	}
}

int GeneratorPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	UNUSED(vProjectFile);
	UNUSED(vUserDatas);
	
	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void GeneratorPane::DrawGeneratorPane(ProjectFile *vProjectFile)
{
	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_GENERATOR)
	{
		if (ImGui::Begin<PaneFlags>(GENERATOR_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_GENERATOR,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile && vProjectFile->IsLoaded())
			{
				if (vProjectFile->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
				{
					SelectionHelper::Instance()->DrawMenu(vProjectFile);

					DrawFontsGenerator(vProjectFile);
				}
			}
		}

		ImGui::End();
	}
}

void GeneratorPane::DrawFontsGenerator(ProjectFile *vProjectFile)
{
	if (ImGui::BeginFramedGroup("Font Generation"))
	{
		float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x;
		ImVec2 btnSize = ImVec2(maxWidth - ImGui::GetStyle().FramePadding.x, 0);

		if (vProjectFile->m_SelectedFont)
		{
			bool btnClick = false;
			std::string exts;

#ifdef _DEBUG
			if (ImGui::Button("Quick Font Current", btnSize))
			{
				vProjectFile->m_GenModeFlags = (GenModeFlags)0;
				vProjectFile->AddGenMode(GENERATOR_MODE_CURRENT_FONT); // font + header
				vProjectFile->AddGenMode(GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES);
				std::string path = FileHelper::Instance()->GetAppPath() + "/exports";
				path = FileHelper::Instance()->CorrectSlashTypeForFilePathName(path);
				FileHelper::Instance()->CreateDirectoryIfNotExist(path);
				Generator::Instance()->Generate(vProjectFile, path, "test.ttf");
			}
			if (ImGui::Button("Quick Font Merged", btnSize))
			{
				bool disableGlyphReScale = vProjectFile->IsGenMode(GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE);
				vProjectFile->m_GenModeFlags = (GenModeFlags)0;
				vProjectFile->AddGenMode(GENERATOR_MODE_MERGED_FONT); // font + header
				vProjectFile->AddGenMode(GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES);
				if (disableGlyphReScale)
					vProjectFile->AddGenMode(GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE);
				std::string path = FileHelper::Instance()->GetAppPath() + "/exports";
				path = FileHelper::Instance()->CorrectSlashTypeForFilePathName(path);
				FileHelper::Instance()->CreateDirectoryIfNotExist(path);
				Generator::Instance()->Generate(vProjectFile, path, "test.ttf");
			}
			if (ImGui::Button("Quick Header Font Current", btnSize))
			{
				vProjectFile->m_GenModeFlags = (GenModeFlags)0;
				vProjectFile->AddGenMode(GENERATOR_MODE_CURRENT_HEADER); // header
				std::string path = FileHelper::Instance()->GetAppPath() + "/exports";
				path = FileHelper::Instance()->CorrectSlashTypeForFilePathName(path);
				FileHelper::Instance()->CreateDirectoryIfNotExist(path);
				Generator::Instance()->Generate(vProjectFile, path, "test.h");
			}
			if (ImGui::Button("Quick Card Font Current", btnSize))
			{
				vProjectFile->m_GenModeFlags = (GenModeFlags)0;
				vProjectFile->AddGenMode(GENERATOR_MODE_CURRENT_CARD); // card
				std::string path = FileHelper::Instance()->GetAppPath() + "/exports";
				path = FileHelper::Instance()->CorrectSlashTypeForFilePathName(path);
				FileHelper::Instance()->CreateDirectoryIfNotExist(path);
				Generator::Instance()->Generate(vProjectFile, path, "test.png");
			}

			ImGui::FramedGroupSeparator();
#endif
			bool change = false;
			ImGui::FramedGroupText("Modes");
			bool batchModeDisabled = true;
			bool mergeModeDisabled = true;
			if (vProjectFile->m_Fonts.size() > 1)
			{
				batchModeDisabled = false;

				if (vProjectFile->m_CountFontWithSelectedGlyphs > 1 &&
					m_GeneratorStatusFlags & GeneratorStatusFlags::GENERATOR_STATUS_FONT_MERGE_ALLOWED)
				{
					mergeModeDisabled = false;
				}
				else // unselect merge mode
				{
					vProjectFile->RemoveGenMode(GENERATOR_MODE_MERGED);
					if (!vProjectFile->IsGenMode(GENERATOR_MODE_BATCH) &&
						!vProjectFile->IsGenMode(GENERATOR_MODE_CURRENT))
					{
						vProjectFile->AddGenMode(GENERATOR_MODE_BATCH);
					}
				}
			}
			else // select current only
			{
				vProjectFile->RemoveGenMode(GENERATOR_MODE_MERGED);
				vProjectFile->RemoveGenMode(GENERATOR_MODE_BATCH);
				vProjectFile->AddGenMode(GENERATOR_MODE_CURRENT);
			}

			float mrw = maxWidth / 3.0f - ImGui::GetStyle().FramePadding.x;
			change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Current", "Current Font",
				&vProjectFile->m_GenModeFlags, GENERATOR_MODE_CURRENT, mrw, true, true,
				GENERATOR_MODE_RADIO_CUR_BAT_MER);
			ImGui::SameLine();
			change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Batch", "Font by Font",
				&vProjectFile->m_GenModeFlags, GENERATOR_MODE_BATCH, mrw, true, true,
				GENERATOR_MODE_RADIO_CUR_BAT_MER, batchModeDisabled);
			ImGui::SameLine();
			change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Merged", "Fonts Merged in one",
				&vProjectFile->m_GenModeFlags, GENERATOR_MODE_MERGED, mrw, true, true,
				GENERATOR_MODE_RADIO_CUR_BAT_MER, mergeModeDisabled);

			ImGui::FramedGroupText("Features");
			bool headerModeDisabled = true;
			if (m_GeneratorStatusFlags & GeneratorStatusFlags::GENERATOR_STATUS_FONT_HEADER_GENERATION_ALLOWED)
			{
				headerModeDisabled = false;
			}
			else
			{
				vProjectFile->RemoveGenMode(GENERATOR_MODE_HEADER);
				vProjectFile->RemoveGenMode(GENERATOR_MODE_CARD);
			}

			mrw = maxWidth / 2.0f - ImGui::GetStyle().FramePadding.x;
			change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Header", "Header File",
				&vProjectFile->m_GenModeFlags, GENERATOR_MODE_HEADER, mrw,
				false, false, GENERATOR_MODE_NONE, headerModeDisabled);
			ImGui::SameLine();
			change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Card", "Card Picture",
				&vProjectFile->m_GenModeFlags, GENERATOR_MODE_CARD, mrw,
				false, false, GENERATOR_MODE_NONE, headerModeDisabled);

			// un header est lié a un TTF ou un CPP ne petu aps etre les deux
			// donc on fait soit l'un soit l'autre
			change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Font", "Font File",
				&vProjectFile->m_GenModeFlags, GENERATOR_MODE_FONT, mrw,
				true, false, GENERATOR_MODE_RADIO_FONT_SRC);
			ImGui::SameLine();
			change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Src", "Source File for C++/C#\n\twith font as a bytes array",
				&vProjectFile->m_GenModeFlags, GENERATOR_MODE_SRC, mrw,
				true, false, GENERATOR_MODE_RADIO_FONT_SRC);

			ImGui::FramedGroupText("Settings");
			if (vProjectFile->IsGenMode(GENERATOR_MODE_HEADER) || 
				vProjectFile->IsGenMode(GENERATOR_MODE_SRC))
			{
				mrw = maxWidth / 3.0f - ImGui::GetStyle().FramePadding.x;
				ImGui::Text("Header / Src Languages : ");
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>(
					"C", "Embedded font as a Byte Array for C",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_LANG_C, mrw,
					false, false, GENERATOR_MODE_RADIO_LANG);
				ImGui::SameLine();
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>(
					"C++", "Embedded font as a Byte Array for C++",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_LANG_CPP, mrw,
					false, false, GENERATOR_MODE_RADIO_LANG);
				ImGui::SameLine();
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>(
					"C#", "Embedded font as a Byte Array for C#",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_LANG_CSHARP, mrw,
					false, false, GENERATOR_MODE_RADIO_LANG);

#ifdef _DEBUG
/*				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>(
					"Lua", "Embedded font as a Byte Array for LUA",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_LANG_LUA, mrw,
					false, false, GENERATOR_MODE_RADIO_LANG);
				ImGui::SameLine();
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>(
					"Python", "Embedded font as a Byte Array for Python",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_LANG_PYTHON, mrw,
					false, false, GENERATOR_MODE_RADIO_LANG);
				ImGui::SameLine();*/
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>(
					"Rust", "Embedded font as a Byte Array for Rust",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_LANG_RUST, mrw,
					false, false, GENERATOR_MODE_RADIO_LANG);
#endif
			}

			if (vProjectFile->IsGenMode(GENERATOR_MODE_MERGED))
			{
				ImGui::FramedGroupText("Merged Mode");
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>(
					"Disable Glyph Re Write", "if your fonts have same size,\nit can be more safe for the moment (bad generated font is some case)\nto disable glyph re write.\nonly needed if we must change glyph scale",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE, 
					maxWidth - ImGui::GetStyle().FramePadding.x);
			}

			if (vProjectFile->IsGenMode(GENERATOR_MODE_FONT))
			{
				ImGui::FramedGroupText("Font");
				change |= ImGui::RadioButtonLabeled_BitWize<GenModeFlags>("Export Names", "export glyph names in font file (increase size)",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES, 
					maxWidth - ImGui::GetStyle().FramePadding.x);
			}

			ImGui::FramedGroupSeparator();

			if (CheckAndDisplayGenerationConditions(vProjectFile))
			{
				ImGui::RadioButtonLabeled_BitWize<GenModeFlags>(
					"Auto Opening", "Auto Opening of Generated Files in associated app after generation",
					&vProjectFile->m_GenModeFlags, GENERATOR_MODE_OPEN_GENERATED_FILES_AUTO,
					maxWidth - ImGui::GetStyle().FramePadding.x);
				if (ImGui::Button(ICON_IGFS_GENERATE " Generate", ImVec2(maxWidth - ImGui::GetStyle().FramePadding.x, 0.0f)))
				{
					btnClick = true;
					if (vProjectFile->IsGenMode(GENERATOR_MODE_FONT)) exts = ".ttf";
					else if (vProjectFile->IsGenMode(GENERATOR_MODE_SRC))
					{
						if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_C)) exts = ".c";
						else if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CPP)) exts = ".cpp";
						else if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CSHARP)) exts = ".cs";
					}
					else if (vProjectFile->IsGenMode(GENERATOR_MODE_HEADER))
					{
						if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_C) ||
							vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CPP)) exts = ".h";
						else if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CSHARP)) exts = ".cs";
					}
					else if (vProjectFile->IsGenMode(GENERATOR_MODE_CARD)) exts = ".png";
				}
			}

			if (change)
			{
				vProjectFile->SetProjectChange();

				// check messages for maybe case by case activate or deactivate features
				// bascally, is there some issue with names or codepoint between font but not in fonts
				// we msut deactivate merge mode, but enable current and batch mode
				ModifyConfigurationAccordingToSelectedFeaturesAndErrors(vProjectFile);
			}

			if (btnClick)
			{
				assert(!exts.empty());

				static char extTypes[512] = "\0";
#ifdef MSVC
				strncpy_s(extTypes, exts.c_str(), ct::mini((int)exts.size(), 511));
#else
				strncpy(extTypes, exts.c_str(), exts.size());
#endif
				if (vProjectFile->IsGenMode(GENERATOR_MODE_HEADER_CARD_SRC))
				{
					if (vProjectFile->IsGenMode(GENERATOR_MODE_BATCH))
					{
						std::string path = vProjectFile->m_LastGeneratedPath;
						if (path.empty() || path == ".")
							path = vProjectFile->m_ProjectFilePath;
						ImGuiFileDialog::Instance()->OpenModal(
							"GenerateFileDlg",
							"Location where create the files", 0, path, vProjectFile->m_LastGeneratedFileName,
							std::bind(&GeneratorPane::GeneratorFileDialogPane, this,
								std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
							200, 1, vProjectFile, ImGuiFileDialogFlags_ConfirmOverwrite);
					}
					else
					{
						std::string path = vProjectFile->m_LastGeneratedPath;
						if (path.empty() || path == ".")
							path = vProjectFile->m_ProjectFilePath;
						ImGuiFileDialog::Instance()->OpenModal(
							"GenerateFileDlg",
							"Location and name where create the file", extTypes, path, vProjectFile->m_SelectedFont->m_GeneratedFileName,
							std::bind(&GeneratorPane::GeneratorFileDialogPane, this,
								std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
							200, 1, vProjectFile, ImGuiFileDialogFlags_ConfirmOverwrite);
					}
				}
				else
				{
					if (vProjectFile->IsGenMode(GENERATOR_MODE_BATCH))
					{
						std::string path = vProjectFile->m_LastGeneratedPath;
						if (path.empty() || path == ".")
							path = vProjectFile->m_ProjectFilePath;
						ImGuiFileDialog::Instance()->OpenModal(
							"GenerateFileDlg",
							"Location where create the files", 0, path, vProjectFile->m_LastGeneratedFileName,
							1, vProjectFile, ImGuiFileDialogFlags_ConfirmOverwrite);
					}
					else
					{
						std::string path = vProjectFile->m_LastGeneratedPath;
						if (path.empty() || path == ".")
							path = vProjectFile->m_ProjectFilePath;
						ImGuiFileDialog::Instance()->OpenModal(
							"GenerateFileDlg",
							"Location and name where create the file", extTypes, path, vProjectFile->m_SelectedFont->m_GeneratedFileName,
							1, vProjectFile, ImGuiFileDialogFlags_ConfirmOverwrite);
					}
				}
			}
		}
		else
		{
			ImGui::Text("No Selected Font.");
		}

		ImGui::EndFramedGroup(true);
	}
}

/*
Always on feature must be selected : Header or Card or Font or CPP
Card can be alone
header need cpp or font
Cpp and Font cant be generated both at same time
*/
bool GeneratorPane::CheckAndDisplayGenerationConditions(ProjectFile *vProjectFile)
{
	bool res = true;

	ShowGenerationStatus(vProjectFile);

	// always a feature must be selected
	if (vProjectFile->IsGenMode(GENERATOR_MODE_HEADER) ||
		vProjectFile->IsGenMode(GENERATOR_MODE_CARD) ||
		vProjectFile->IsGenMode(GENERATOR_MODE_FONT) ||
		vProjectFile->IsGenMode(GENERATOR_MODE_SRC))
	{
		
	}
	else
	{
		res = false;
		ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "Can't generate.\n\tSelect one feature at least");
	}

	// not remembered why i done that.. so disabled for the moment
	/*if (vProjectFile->IsGenMode(GENERATOR_MODE_MERGED) &&
		!(vProjectFile->IsGenMode(GENERATOR_MODE_FONT) ||
			vProjectFile->IsGenMode(GENERATOR_MODE_SRC)))
	{
		res = false;
		ImGui::TextColored(ImGuiThemeHelper::Instance()->badColor, "Merged mode require the\ngeneration of font or cpp.\nPlease Select one of\nthese two at least");
	}*/

	bool needOneLangageSelectedAtLeast = 
		(vProjectFile->IsGenMode(GENERATOR_MODE_LANG_C) ||
		vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CPP) ||
		vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CSHARP));

	if (vProjectFile->IsGenMode(GENERATOR_MODE_HEADER))
	{
		// header need SRC or Font at least
		if (!(vProjectFile->IsGenMode(GENERATOR_MODE_FONT) ||
			vProjectFile->IsGenMode(GENERATOR_MODE_SRC)))
		{
			res = false;
			ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->badColor, "the Header is linked ot a font or a cpp.\nPlease Select Cpp or Font at least");
		}

		// need on language at mini
		if (!needOneLangageSelectedAtLeast)
		{
			res = false;
			ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->badColor, "A language must be selected for the generation of the Header file");
		}
	}

	if (vProjectFile->IsGenMode(GENERATOR_MODE_SRC))
	{
		// need on language at mini
		if (!needOneLangageSelectedAtLeast)
		{
			res = false;
			ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->badColor, "A language must be selected for the generation of the Source file");
		}
	}
	// check of codepoint/name in double 
	if (res)
	{
		size_t errorsCount = 0;
		
		ImGui::FramedGroupText("Font Status");
		vProjectFile->m_CountFontWithSelectedGlyphs = 0;
		for (auto font : vProjectFile->m_Fonts)
		{
			if (font.second->m_CodePointInDoubleFound || font.second->m_NameInDoubleFound)
			{
				ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->badColor, "%s : NOK", font.second->m_FontFileName.c_str());
				errorsCount++;
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_MERGED) && font.second->m_SelectedGlyphs.empty())
			{
				ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->badColor, "%s : NOK\nNo Glyphs are selected.\nYou need it in Merged mode", font.second->m_FontFileName.c_str());
				errorsCount++;
			}
			else
			{
				if (font.second->m_SelectedGlyphs.empty()) // no glyphs to extract, we wille extract alls, current and batch only
				{
					ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->badColor, "%s : NOK\nNo Glyphs are selected.\nCan't generate.", font.second->m_FontFileName.c_str());
					errorsCount++;
				}
				else
				{
					ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->goodColor, "%s : OK\n%u Glyphs selected", font.second->m_FontFileName.c_str(), font.second->m_SelectedGlyphs.size());
					vProjectFile->m_CountFontWithSelectedGlyphs++;
				}
			}
		}

		ImGui::FramedGroupText("%u Glyphs selected in %u fonts",
			vProjectFile->m_CountSelectedGlyphs,
			vProjectFile->m_CountFontWithSelectedGlyphs);

		ImGui::FramedGroupSeparator();

		if (errorsCount < vProjectFile->m_Fonts.size())
		{
			if (errorsCount > 0)
			{
				ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->badColor, "Can partially generate\nCheck status bar");
			}
			else
			{
				ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->goodColor, "Can fully generate");
			}

			if (vProjectFile->IsGenMode(GENERATOR_MODE_MERGED))
			{
				ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->goodColor,
					"The selected font\nscale / bounding box\nwill be used for merge in\nall other font glyphs");
			}
		}
		else
		{
			ImGui::FramedGroupText(ImGuiThemeHelper::Instance()->badColor, "Can't generate\nCheck status bar");
			res = false;
		}
	}

	return res;
}

void GeneratorPane::ShowGenerationStatus(ProjectFile* vProjectFile)
{
	if (ImGui::CollapsingHeader("Generation Status", ImGuiTreeNodeFlags_Bullet))
	{
		auto _countLines = vProjectFile->m_Fonts.size() + 1U;

		static ImGuiTableFlags flags =
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
			ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_Borders;
		if (ImGui::BeginTable("##fileTable", 3, flags, ImVec2(-1.0f, _countLines * ImGui::GetFrameHeightWithSpacing())))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, -1, 1); // used or not for this generation
			ImGui::TableSetupColumn("Font", ImGuiTableColumnFlags_WidthStretch, -1, 0);
			ImGui::TableSetupColumn("Features", ImGuiTableColumnFlags_WidthFixed, -1, 2); // export names
			
			ImGui::TableHeadersRow();

			uint32_t idx = 0;
			for (const auto& itFont : vProjectFile->m_Fonts)
			{
				if (itFont.second.use_count())
				{
					ImGui::TableNextRow();

					ImGui::PushID(itFont.second.get());
					if (ImGui::TableSetColumnIndex(0))
					{
						static bool v = false;
						ImGui::PushItemWidth(20.0f);
						static char buffer[10];
						ImGui::RadioButtonLabeled(itoa(idx++, buffer, 10), "Enable/Disable", &v, false);
						ImGui::PopItemWidth();
					}
					if (ImGui::TableSetColumnIndex(1))
					{
						static char buffer[255];
						snprintf(buffer, 254, "%u Selected Glyphs", (uint32_t)itFont.second->m_SelectedGlyphs.size());
						ImGui::FramedGroupTextHelp(buffer, "%s", itFont.second->m_FontFileName.c_str());
					}
					if (ImGui::TableSetColumnIndex(2))
					{
						static bool v[4] = { false, false, false, false };
						v[0] = vProjectFile->IsGenMode(GENERATOR_MODE_HEADER);
						v[1] = vProjectFile->IsGenMode(GENERATOR_MODE_CARD);
						v[2] = vProjectFile->IsGenMode(GENERATOR_MODE_FONT);
						v[3] = vProjectFile->IsGenMode(GENERATOR_MODE_SRC);
						ImGui::BeginGroup();
						ImGui::PushItemWidth(20.0f);
						ImGui::RadioButtonLabeled("H", "Header Feature", v[0], false); ImGui::SameLine();
						ImGui::PopItemWidth();
						ImGui::PushItemWidth(20.0f);
						ImGui::RadioButtonLabeled("C", "Card Feature", v[1], false); ImGui::SameLine();
						ImGui::PopItemWidth();
						ImGui::PushItemWidth(20.0f);
						ImGui::RadioButtonLabeled("F", "Font Feature", v[2], false); ImGui::SameLine();
						ImGui::PopItemWidth();
						ImGui::PushItemWidth(20.0f);
						ImGui::RadioButtonLabeled("S", "Src Feature", v[3], false);
						ImGui::PopItemWidth();
						ImGui::EndGroup();
					}
					ImGui::PopID();
				}
			}
			ImGui::EndTable();
		}
	}
}

/*
errors types :
 - if one font have codepoints in double : disable font generation until solved for this font only
 - if one font have name in double : disable header file generation and name export in font file for this font only
 - if fonts have codepoint in double between fonts but not per fonts : merge mode disables for all fonts
 - if font have name in double between fonts nut not per font : disable header file generation and name export in merge mode

 it seem we need to have a gen mode per target
 in batch mode if a font have issue with header we need to not gen them but we need to show that to the user
*/
void GeneratorPane::ModifyConfigurationAccordingToSelectedFeaturesAndErrors(ProjectFile* vProjectFile)
{
	if (vProjectFile)
	{
		// current font
		if (vProjectFile->m_SelectedFont.use_count())
		{

		}

		// if one font have codepoints in double : disable font generation until solved for this font only
		if (vProjectFile->m_NameFoundInDouble || vProjectFile->m_CodePointFoundInDouble)
		{
			
		}
	}
}

// file dialog pane
void GeneratorPane::GeneratorFileDialogPane(const char* vFilter, IGFDUserDatas vUserDatas,
	bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	UNUSED(vFilter);

	LogAssert(vCantContinue != 0, "ImGuiFileDialog Pane param vCantContinue is NULL");

	bool canContinue = true;

	auto prj = (ProjectFile*)vUserDatas;
	if (prj)
	{
		if (prj->IsGenMode(GENERATOR_MODE_HEADER_CARD_SRC))
		{
			#define PREFIX_MAX_SIZE 49
			static char prefixBuffer[PREFIX_MAX_SIZE + 1] = "\0";
			
			if (prj->IsGenMode(GENERATOR_MODE_CURRENT))
			{
				if (prj->m_SelectedFont)
				{
					if (ImGui::BeginFramedGroup(prj->m_SelectedFont->m_FontFileName.c_str()))
					{
						const float aw = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x;
						
						const bool cond = !prj->m_SelectedFont->m_FontPrefix.empty();
						snprintf(prefixBuffer, PREFIX_MAX_SIZE, "%s", prj->m_SelectedFont->m_FontPrefix.c_str());
						ImGui::FramedGroupText("Prefix :");
						ImGui::PushItemWidth(aw);
						if (ImGui::InputText_Validation("##FontPrefix", prefixBuffer, PREFIX_MAX_SIZE,
							&cond, "You must Define a\nfont prefix for continue"))
						{
							prj->m_SelectedFont->m_FontPrefix = std::string(prefixBuffer);
							prj->SetProjectChange();
						}
						ImGui::PopItemWidth();

						canContinue = !prj->m_SelectedFont->m_FontPrefix.empty();

						if (prj->IsGenMode(GENERATOR_MODE_CARD))
						{
							ImGui::FramedGroupSeparator();

							ImGui::FramedGroupText("Card");

							bool ch = ImGui::SliderUIntDefaultCompact(aw, "Glyph Height", &prj->m_SelectedFont->m_CardGlyphHeightInPixel, 1U, 200U, defaultFontInfos.m_CardGlyphHeightInPixel);
							ch |= ImGui::SliderUIntDefaultCompact(aw, "Max Rows", &prj->m_SelectedFont->m_CardCountRowsMax, 10U, 1000U, defaultFontInfos.m_CardCountRowsMax);
							if (ch) prj->SetProjectChange();


							canContinue &= (prj->m_SelectedFont->m_CardGlyphHeightInPixel > 0) && (prj->m_SelectedFont->m_CardCountRowsMax > 0);
						}

						ImGui::EndFramedGroup();
					}
				}
				else
				{
					// Normally, cant have this issue, but kepp for fewer code modification
					ImGui::FramedGroupText("No Font Selected. Can't continue");
					canContinue = false;
				}
			}
			else if (prj->IsGenMode(GENERATOR_MODE_BATCH))
			{
				canContinue = true;
				
				#define FILENAME_MAX_SIZE 1024
				static char fileNameBuffer[FILENAME_MAX_SIZE + 1] = "\0";
				
				std::map<std::string, int> filenames;
				std::map<std::string, int> prefixs;
				for (auto& font : prj->m_Fonts)
				{
					if (font.second)
					{
						if (ImGui::BeginFramedGroup(font.second->m_FontFileName.c_str()))
						{
							const float aw = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x;

							ImGui::FramedGroupText("File Name :");

							snprintf(fileNameBuffer, FILENAME_MAX_SIZE, "%s", font.second->m_GeneratedFileName.c_str());
							bool filenameCond = !font.second->m_GeneratedFileName.empty(); // not empty
							if (filenames.find(font.second->m_GeneratedFileName) == filenames.end())
							{
								filenames[font.second->m_GeneratedFileName] = 1;
							}
							else
							{
								filenameCond &= (filenames[font.second->m_GeneratedFileName] == 0); // must be unique
							}
							ImGui::PushID(&font);
							ImGui::PushItemWidth(aw);
							const bool resFN = ImGui::InputText_Validation("##FontFileName", fileNameBuffer, FILENAME_MAX_SIZE,
								&filenameCond, "You must Define a\nfont file name unique for continue");
							ImGui::PopItemWidth();
							ImGui::PopID();
							if (resFN)
							{
								font.second->m_GeneratedFileName = std::string(fileNameBuffer);
								prj->SetProjectChange();
							}
							filenames[font.second->m_GeneratedFileName]++;
							canContinue &= filenameCond;

							ImGui::FramedGroupText("Prefix :");

							snprintf(prefixBuffer, PREFIX_MAX_SIZE, "%s", font.second->m_FontPrefix.c_str());
							bool prefixCond = !font.second->m_FontPrefix.empty(); // not empty
							if (prefixs.find(font.second->m_FontPrefix) == prefixs.end())
							{
								prefixs[font.second->m_FontPrefix] = 1;
							}
							else
							{
								prefixCond &= (prefixs[font.second->m_FontPrefix] == 0); // must be unique
							}

							ImGui::PushID(&font);
							ImGui::PushItemWidth(aw);
							const bool resPF = ImGui::InputText_Validation("##FontPrefix", prefixBuffer, PREFIX_MAX_SIZE,
								&prefixCond, "You must Define a\nfont prefix and unique for continue");
							ImGui::PopItemWidth();
							ImGui::PopID();
							if (resPF)
							{
								font.second->m_FontPrefix = std::string(prefixBuffer);
								prj->SetProjectChange();
							}
							prefixs[font.second->m_FontPrefix]++;
							canContinue &= prefixCond;

							if (prj->IsGenMode(GENERATOR_MODE_CARD))
							{
								ImGui::FramedGroupSeparator();

								ImGui::FramedGroupText("Card");

								ImGui::PushID(&font);
								bool ch = ImGui::SliderUIntDefaultCompact(aw, "Glyph Height", &font.second->m_CardGlyphHeightInPixel, 1U, 200U, defaultFontInfos.m_CardGlyphHeightInPixel);
								ch |= ImGui::SliderUIntDefaultCompact(aw, "Max Rows", &font.second->m_CardCountRowsMax, 10U, 1000U, defaultFontInfos.m_CardCountRowsMax);
								if (ch) prj->SetProjectChange();
								ImGui::PopID();
								canContinue &= (font.second->m_CardGlyphHeightInPixel > 0) && (font.second->m_CardCountRowsMax > 0);
							}

							ImGui::EndFramedGroup();
						}
					}
				}
			}
			else if (prj->IsGenMode(GENERATOR_MODE_MERGED))
			{
				if (ImGui::BeginFramedGroup(0))
				{
					ImGui::FramedGroupSeparator();
					
					const float aw = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x;
					
					const bool cond = !prj->m_MergedFontPrefix.empty();

					snprintf(prefixBuffer, PREFIX_MAX_SIZE, "%s", prj->m_MergedFontPrefix.c_str());
					ImGui::FramedGroupText("Prefix :");
					ImGui::PushItemWidth(aw);
					if (ImGui::InputText_Validation("##FontPrefix", prefixBuffer, PREFIX_MAX_SIZE,
						&cond, "You must Define a\nfont prefix for continue"))
					{
						prj->m_MergedFontPrefix = prefixBuffer;
						prj->SetProjectChange();
					}
					ImGui::PopItemWidth();

					canContinue = !prj->m_MergedFontPrefix.empty();
					
					if (prj->IsGenMode(GENERATOR_MODE_CARD))
					{
						ImGui::FramedGroupSeparator();

						ImGui::FramedGroupText("Card :");

						bool ch = ImGui::SliderUIntDefaultCompact(aw, "Glyph Height", &prj->m_MergedCardGlyphHeightInPixel, 1U, 200U, defaultProjectFile.m_MergedCardGlyphHeightInPixel);
						ch |= ImGui::SliderUIntDefaultCompact(aw, "Max Rows", &prj->m_MergedCardCountRowsMax, 10U, 1000U, defaultProjectFile.m_MergedCardCountRowsMax);
						if (ch) prj->SetProjectChange();

						canContinue &= (prj->m_MergedCardGlyphHeightInPixel > 0) && (prj->m_MergedCardCountRowsMax > 0);
					}

					ImGui::EndFramedGroup();
				}
			}
		}

		if (vCantContinue)
		{
			*vCantContinue = canContinue;
		}
	}
}

