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

#include "MainFrame.h"

#include "Gui/GuiLayout.h"
#include "Gui/ImGuiWidgets.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cTools.h>
#include <FileHelper.h>

#include <cinttypes> // printf zu

static int FontStructurePane_WidgetId = 0;

FontStructurePane::FontStructurePane() = default;
FontStructurePane::~FontStructurePane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int FontStructurePane::DrawFontStructurePane(ProjectFile *vProjectFile, int vWidgetId)
{
    FontStructurePane_WidgetId = vWidgetId;

    if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_FONT_STRUCTURE)
    {
        if (ImGui::Begin<PaneFlags>(FONT_STRUCTURE_PANE,
                &GuiLayout::m_Pane_Shown, PaneFlags::PANE_FONT_STRUCTURE,
                //ImGuiWindowFlags_NoTitleBar |
                //ImGuiWindowFlags_MenuBar |
                //ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                //ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            if (vProjectFile &&  vProjectFile->IsLoaded())
            {
				if (vProjectFile->m_CurrentFont)
				{
					if (ImGui::Button("Analyse Font"))
					{
						std::string fontFilePathName = FileHelper::Instance()->CorrectFilePathName(vProjectFile->m_CurrentFont->m_FontFilePathName);

						if (!FileHelper::Instance()->IsAbsolutePath(fontFilePathName))
						{
							fontFilePathName = vProjectFile->GetAbsolutePath(fontFilePathName);
						}

						m_FontParser.ParseFont(fontFilePathName);
					}
				}

                FontStructurePane_WidgetId = DisplayAnalyze(FontStructurePane_WidgetId);
            }
        }

        ImGui::End();
    }

    return FontStructurePane_WidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int FontStructurePane::DisplayAnalyze(int vWidgetId)
{
	vWidgetId = m_FontParser.m_FontAnalyzed.draw(vWidgetId);

	return vWidgetId;
}