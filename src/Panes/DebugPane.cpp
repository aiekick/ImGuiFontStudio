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

#include "MainFrame.h"

#include "Gui/GuiLayout.h"
#include "Gui/ImGuiWidgets.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cTools.h>
#include <FileHelper.h>

#include <cinttypes> // printf zu

static int DebugPane_WidgetId = 0;

DebugPane::DebugPane() = default;
DebugPane::~DebugPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int DebugPane::DrawDebugPane(ProjectFile *vProjectFile, int vWidgetId)
{
	DebugPane_WidgetId = vWidgetId;

	if (GuiLayout::m_Pane_Shown & PaneFlags::PANE_DEBUG)
	{
		if (ImGui::Begin<PaneFlags>(DEBUG_PANE,
			&GuiLayout::m_Pane_Shown, PaneFlags::PANE_DEBUG,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				if (GuiLayout::Instance()->IsPaneActive(PaneFlags::PANE_GLYPH))
				{
					DebugPane_WidgetId = DrawDebugGlyphPane(vProjectFile, DebugPane_WidgetId);
				}
			}
		}

		ImGui::End();
	}

	return DebugPane_WidgetId;
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

int DebugPane::DrawDebugGlyphPane(ProjectFile* /*vProjectFile*/, int vWidgetId)
{
	auto g = &(m_GlyphToDisplay.simpleGlyph);
	if (g->isValid)
	{
		int _c = 0;
		for (auto &co : g->coords)
		{
			ImGui::PushID(++vWidgetId);
			bool res = ImGui::CollapsingHeader_SmallHeight("Contour", 0.5f, -1, true);
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

	return vWidgetId;
}

#endif