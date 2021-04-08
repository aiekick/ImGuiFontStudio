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
#include <Gui/ImWidgets.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <Helper/Profiler.h>

#include <cinttypes> // printf zu

DebugPane::DebugPane() = default;
DebugPane::~DebugPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void DebugPane::Init()
{
	ZoneScoped;
}

void DebugPane::Unit()
{
	ZoneScoped;
}

int DebugPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	ZoneScoped;

	paneWidgetId = vWidgetId;

	DrawDebugPane(vProjectFile);

	return paneWidgetId;
}

void DebugPane::DrawDialogsAndPopups(ProjectFile* /*vProjectFile*/)
{
	ZoneScoped;
}

int DebugPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	ZoneScoped;

	UNUSED(vProjectFile);
	UNUSED(vUserDatas);

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void DebugPane::DrawDebugPane(ProjectFile *vProjectFile)
{
	ZoneScoped;

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

void DebugPane::SetGlyphToDebug(std::weak_ptr<GlyphInfos> vGlyphInfos)
{
	ZoneScoped;

	m_GlyphToDisplay = vGlyphInfos;
}

void DebugPane::Clear()
{
	ZoneScoped;

	m_GlyphToDisplay.reset();
}

ct::ivec2 DebugPane::GetGlyphCurrentPoint()
{
	ZoneScoped;

	return m_GlyphCurrentPoint;
}

void DebugPane::DrawGlyphCurrentPoint(ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin, ImDrawList *vImDrawList)
{
	ZoneScoped;

#define LocalToScreen(a) getLocalToScreen(ImVec2((float)a.x, (float)a.y), vZoneStart, vWorldBBoxOrigin, vWorlBBoxSize, vWorldScale, ImVec2((float)vLocalBBoxOrigin.x, (float)vLocalBBoxOrigin.y))
#define ScreenToLocal(a) getScreenToLocal(ImVec2((float)a.x, (float)a.y), vZoneStart, vWorldBBoxOrigin, vWorlBBoxSize, vWorldScale, ImVec2((float)vLocalBBoxOrigin.x, (float)vLocalBBoxOrigin.y))

	if (!m_GlyphToDisplay.expired())
	{
		auto m_GlyphPtr = m_GlyphToDisplay.lock();
		if (m_GlyphPtr.use_count())
		{
			auto g = &(m_GlyphPtr->simpleGlyph);
			if (g->isValid)
			{
				int cmax = (int)g->m_Glyph.contours.size();
				ct::ivec2 cp = DebugPane::Instance()->GetGlyphCurrentPoint();
				if (cp.x >= 0 && cp.x < cmax)
				{
					int pmax = (int)g->m_Glyph.contours[cp.x].m_Points.size();
					ct::ivec2 cur = g->LocalToScreen(g->GetCoords(&g->m_Glyph, cp.x, cp.y));
					ImVec2 posCircle = ct::toImVec2(cur);
					vImDrawList->AddCircleFilled(posCircle, 5.0f, ImGui::GetColorU32(ImVec4(1, 1, 0, 1)));
				}
			}
		}
	}

#undef LocalToScreen
#undef ScreenToLocal

}

void DebugPane::DrawDebugGlyphPane(ProjectFile* /*vProjectFile*/)
{
	ZoneScoped;

	if (!m_GlyphToDisplay.expired())
	{
		auto m_GlyphPtr = m_GlyphToDisplay.lock();
		if (m_GlyphPtr.use_count())
		{
			auto g = &(m_GlyphPtr->simpleGlyph);
			if (g->isValid)
			{
				int _c = 0;
				for (auto& co : g->m_Glyph.contours)
				{
					ImGui::PushID(++paneWidgetId);
					bool res = ImGui::CollapsingHeader_SmallHeight("Contour", 0.7f, -1);
					ImGui::PopID();
					if (res)
					{
						int _i = 0;
						for (auto& pt : co.m_Points)
						{
							ImGui::Selectable_FramedText("[%i] oc[%s] x:%i y:%i", _i, (co.m_OnCurve[_i] ? "x" : " "), pt.x, pt.y);
							if (ImGui::IsItemHovered())
								m_GlyphCurrentPoint = ct::ivec2(_c, _i);
							_i++;
						}
					}
					_c++;
				}
			}
		}
	}
}

#endif