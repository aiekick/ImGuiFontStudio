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

#include "GlyphPane.h"

#include "MainFrame.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cTools.h>
#include <FileHelper.h>

#include "Gui/GuiLayout.h"
#include "Gui/ImGuiWidgets.h"
#include "Helper/Messaging.h"
#include "Helper/SelectionHelper.h"
#include "Panes/FinalFontPane.h"
#include "Panes/SourceFontPane.h"
#include "Project/ProjectFile.h"
#include "Generator/Generator.h"

#include <cinttypes> // printf zu

static int GlyphPane_WidgetId = 0;

GlyphPane::GlyphPane()
{
	
}

GlyphPane::~GlyphPane()
{
	
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int GlyphPane::DrawGlyphPane(ProjectFile *vProjectFile, int vWidgetId)
{
	GlyphPane_WidgetId = vWidgetId;

	if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_GLYPH)
	{
		if (ImGui::Begin<PaneFlags>(GLYPH_PANE,
			&GuiLayout::m_Pane_Shown, PaneFlags::PANE_GLYPH,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile && vProjectFile->IsLoaded())
			{
				
			}
		}

		ImGui::End();
	}

	return GlyphPane_WidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// LOAD GLYPH ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer

bool GlyphPane::LoadGlyph(FontInfos* vFontInfos, ImWchar vCodepoint)
{
	return true;
}