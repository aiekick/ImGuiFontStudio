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
#include<Gui/ImWidgets.h>

#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <cinttypes> // printf zu

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool DebugPane::Init()
{
	return true;
}

void DebugPane::Unit()
{

}

int DebugPane::DrawPanes(const uint32_t& /*vCurrentFrame*/, int vWidgetId, std::string /*vUserDatas*/, PaneFlags& vInOutPaneShown)
{
	m_PaneWidgetId = vWidgetId;

	if (vInOutPaneShown & m_PaneFlag)
	{
		static ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar;
		if (ImGui::Begin<PaneFlags>(m_PaneName,
			&vInOutPaneShown, m_PaneFlag, flags))
		{
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
			auto win = ImGui::GetCurrentWindowRead();
			if (win->Viewport->Idx != 0)
				flags |= ImGuiWindowFlags_NoResize;// | ImGuiWindowFlags_NoTitleBar;
			else
				flags = ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_MenuBar;
#endif
			if (ProjectFile::Instance()->IsLoaded())
			{
				if (LayoutManager::Instance()->IsSpecificPaneFocused(m_PaneFlag))
				{
					DrawDebugGlyphPane();
				}
			}
		}

		//MainFrame::sAnyWindowsHovered |= ImGui::IsWindowHovered();

		ImGui::End();
	}

	return m_PaneWidgetId;
}

void DebugPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, std::string /*vUserDatas*/)
{

}

int DebugPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, int vWidgetId, std::string /*vUserDatas*/)
{
	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void DebugPane::SetGlyphToDebug(std::weak_ptr<GlyphInfos> vGlyphInfos)
{
	m_GlyphToDisplay = vGlyphInfos;
}

void DebugPane::Clear()
{
	m_GlyphToDisplay.reset();
}

ct::ivec2 DebugPane::GetGlyphCurrentPoint()
{
	return m_GlyphCurrentPoint;
}

void DebugPane::DrawGlyphCurrentPoint(float vPreviewScale, ImVec2 vScreenPos, ImDrawList *vImDrawList)
{
	UNUSED(vPreviewScale);
	UNUSED(vScreenPos);
	UNUSED(vImDrawList);

	/*if (!m_GlyphToDisplay.expired())
	{
		auto m_GlyphPtr = m_GlyphToDisplay.lock();
		if (m_GlyphPtr.use_count())
		{
			auto g = &(m_GlyphPtr->simpleGlyph);
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
	}*/
}

void DebugPane::DrawDebugGlyphPane()
{
	if (!m_GlyphToDisplay.expired())
	{
		auto m_GlyphPtr = m_GlyphToDisplay.lock();
		if (m_GlyphPtr.use_count())
		{
			auto g = &(m_GlyphPtr->simpleGlyph);
			if (g->isValid)
			{
				int _c = 0;
				for (auto& co : g->coords)
				{
					ImGui::PushID(++m_PaneWidgetId);
					ImGui::SetNextItemOpen(true);
					bool res = ImGui::CollapsingHeader_SmallHeight("Contour", 0.7f, -1, true);
					ImGui::PopID();
					if (res)
					{
						int _i = 0;
						for (auto& pt : co)
						{
							ImGui::Selectable_FramedText("[%i] x:%i y:%i", _i, pt.x, pt.y);
							if (ImGui::IsItemHovered())
								m_GlyphCurrentPoint = ct::ivec2(_c, _i);
							++_i;
						}
					}
					++_c;
				}
			}
		}
	}
}

#endif