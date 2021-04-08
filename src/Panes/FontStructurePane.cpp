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

#include "FontStructurePane.h"

#include <MainFrame.h>

#include <Panes/Manager/LayoutManager.h>
#include <Gui/ImWidgets.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <Helper/Profiler.h>
#include <cinttypes> // printf zu

FontStructurePane::FontStructurePane() = default;
FontStructurePane::~FontStructurePane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FontStructurePane::Init()
{
    ZoneScoped;
}

void FontStructurePane::Unit()
{
    ZoneScoped;
}

int FontStructurePane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
    ZoneScoped;

    paneWidgetId = vWidgetId;

    DrawFontStructurePane(vProjectFile);

    return paneWidgetId;
}

void FontStructurePane::DrawDialogsAndPopups(ProjectFile* /*vProjectFile*/)
{
    ZoneScoped;
}

int FontStructurePane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
    ZoneScoped;

    UNUSED(vProjectFile);
    UNUSED(vUserDatas);

    return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FontStructurePane::DrawFontStructurePane(ProjectFile *vProjectFile)
{
    ZoneScoped;

    if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_FONT_STRUCTURE)
    {
        if (ImGui::Begin<PaneFlags>(FONT_STRUCTURE_PANE,
                &LayoutManager::m_Pane_Shown, PaneFlags::PANE_FONT_STRUCTURE,
                //ImGuiWindowFlags_NoTitleBar |
                //ImGuiWindowFlags_MenuBar |
                //ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                //ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            if (vProjectFile &&  vProjectFile->IsLoaded())
            {
                if (ImGui::Button("Analyse Font"))
                {
					std::string fontFilePathName = FileHelper::Instance()->CorrectSlashTypeForFilePathName(vProjectFile->m_SelectedFont->m_FontFilePathName);

					if (!FileHelper::Instance()->IsAbsolutePath(fontFilePathName))
					{
						fontFilePathName = vProjectFile->GetAbsolutePath(fontFilePathName);
					}

					m_FontParser.ParseFont(fontFilePathName);
                }

                DisplayAnalyze();
            }
        }

        ImGui::End();
    }
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FontStructurePane::DisplayAnalyze()
{
    ZoneScoped;

    paneWidgetId = m_FontParser.draw(paneWidgetId);
}
