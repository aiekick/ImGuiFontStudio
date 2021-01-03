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

#ifdef _DEBUG

#include "DebugPane.h"

#include <MainFrame.h>

#include <Panes/Manager/LayoutManager.h>
#include <Gui/ImGuiWidgets.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <cinttypes> // printf zu

DebugPane::DebugPane() = default;
DebugPane::~DebugPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void DebugPane::Init()
{
	
}

void DebugPane::Unit()
{

}

int DebugPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	paneWidgetId = vWidgetId;

	DrawDebugPane(vProjectFile);

	return paneWidgetId;
}

void DebugPane::DrawDialogsAndPopups(ProjectFile* /*vProjectFile*/)
{

}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void DebugPane::DrawDebugPane(ProjectFile *vProjectFile)
{
	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_DEBUG)
	{
		if (ImGui::Begin<PaneFlags>(DEBUG_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_DEBUG,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				if (LayoutManager::Instance()->IsSpecificPaneFocused(PaneFlags::PANE_GLYPH))
				{
					DrawDebugGlyphPane(vProjectFile);
				}
			}
		}

		ImGui::End();
	}
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void DebugPane::SetGlyphToDebug(const GlyphInfos& vGlyphInfos)
{
	m_GlyphToDisplay = vGlyphInfos;
}

ct::ivec2 DebugPane::GetGlyphCurrentPoint()
{
	return m_GlyphCurrentPoint;
}

void DebugPane::DrawGlyphCurrentPoint(float vPreviewScale, ImVec2 vScreenPos, ImDrawList *vImDrawList)
{
	auto g = &(m_GlyphToDisplay.simpleGlyph);
	if (g->isValid)
	{
		int cmax = (int)g->coords.size();
		ct::ivec2 cp = DebugPane::Instance()->GetGlyphCurrentPoint();
		if (cp.x >= 0 && cp.x < cmax)
		{
			int pmax = (int)g->coords[cp.x].size();
			int firstOn = 0;
			for (int p = 0; p < pmax; p++)
			{
				if (g->IsOnCurve(cp.x, p))
				{
					firstOn = p;
					break;
				}
			}

			int icurr = firstOn + cp.y + 1;
			ct::ivec2 cur = g->GetCoords(cp.x, icurr, vPreviewScale);
			ImVec2 posCircle = ct::toImVec2(cur) + vScreenPos;
			vImDrawList->AddCircleFilled(posCircle, 5.0f, ImGui::GetColorU32(ImVec4(1, 1, 0, 1)));
		}
	}
}

void DebugPane::DrawDebugGlyphPane(ProjectFile* /*vProjectFile*/)
{
	auto g = &(m_GlyphToDisplay.simpleGlyph);
	if (g->isValid)
	{
		int _c = 0;
		for (auto &co : g->coords)
		{
			ImGui::PushID(++paneWidgetId);
			bool res = ImGui::CollapsingHeader_SmallHeight("Contour", 0.7f, -1, true);
			ImGui::PopID();
			if (res)
			{
				int _i = 0;
				for (auto &pt : co)
				{
					ImGui::Selectable_FramedText("[%i] x:%i y:%i", _i, pt.x, pt.y);
					if (ImGui::IsItemHovered())
						m_GlyphCurrentPoint = ct::ivec2(_c, _i);
					_i++;
				}
			}
			_c++;
		}
	}
}

#endif