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

#include "Gui/GuiLayout.h"
#include "Gui/ImGuiWidgets.h"
#ifdef _DEBUG
#include "Panes/DebugPane.h"
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cTools.h>
#include <FileHelper.h>

#include "sfntly/font_factory.h"

static int GlyphPane_WidgetId = 0;

GlyphPane::GlyphPane() = default;
GlyphPane::~GlyphPane() = default;

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
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				if (ImGui::BeginMenuBar())
				{
					ImGui::PushItemWidth(100.0f);
					if (ImGui::SliderFloat("Scale", &vProjectFile->m_GlyphPreview_Scale, 0.01f, 2.0f))
					{
						vProjectFile->SetProjectChange();
					}
					ImGui::PopItemWidth();

					ImGui::PushItemWidth(100.0f);
					if (ImGui::SliderInt("Segments", &vProjectFile->m_GlyphPreview_QuadBezierCountSegments, 1, 50))
					{
						vProjectFile->SetProjectChange();
					}
					ImGui::PopItemWidth();
					
					//ImGui::Checkbox("Stroke or Fill", &_stroke);
					if (ImGui::Checkbox("Control Lines", &vProjectFile->m_GlyphPreview_ShowControlLines))
					{
						vProjectFile->SetProjectChange();
					}

					ImGui::EndMenuBar();
				}

				DrawSimpleGlyph(
					&m_GlyphToDisplay, 
					vProjectFile->m_CurrentFont, 
					vProjectFile->m_GlyphPreview_Scale, 
					vProjectFile->m_GlyphPreview_QuadBezierCountSegments,
					vProjectFile->m_GlyphPreview_ShowControlLines);
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

static int limitContour = 0;

bool GlyphPane::LoadGlyph(ProjectFile *vProjectFile, FontInfos* vFontInfos, GlyphInfos *vGlyphInfos)
{
	bool res = false;

	if (vFontInfos && vGlyphInfos)
	{
		std::string fontPathName = vProjectFile->GetAbsolutePath(vFontInfos->m_FontFilePathName);

		if (FileHelper::Instance()->IsFileExist(fontPathName))
		{
			FontHelper m_FontHelper;

			m_fontInstance.m_Font.Attach(m_FontHelper.LoadFontFile(fontPathName.c_str()));
			if (m_fontInstance.m_Font)
			{
				sfntly::Ptr<sfntly::CMapTable> cmap_table = down_cast<sfntly::CMapTable*>(m_fontInstance.m_Font->GetTable(sfntly::Tag::cmap));
				m_fontInstance.m_CMapTable.Attach(cmap_table->GetCMap(sfntly::CMapTable::WINDOWS_BMP));
				if (m_fontInstance.m_CMapTable)
				{
					m_fontInstance.m_GlyfTable = down_cast<sfntly::GlyphTable*>(m_fontInstance.m_Font->GetTable(sfntly::Tag::glyf));
					m_fontInstance.m_LocaTable = down_cast<sfntly::LocaTable*>(m_fontInstance.m_Font->GetTable(sfntly::Tag::loca));

					if (m_fontInstance.m_GlyfTable && m_fontInstance.m_LocaTable)
					{
						uint32_t codePoint = vGlyphInfos->glyph.Codepoint;
                        uint32_t glyphId = m_fontInstance.m_CMapTable->GlyphId(codePoint);
                        uint32_t length = m_fontInstance.m_LocaTable->GlyphLength(glyphId);
                        uint32_t offset = m_fontInstance.m_LocaTable->GlyphOffset(glyphId);

						// Get the GLYF table for the current glyph id.
						auto g = m_fontInstance.m_GlyfTable->GetGlyph(offset, length);
						
						if (g->GlyphType() == sfntly::GlyphType::kSimple)
						{
							auto glyph = down_cast<sfntly::GlyphTable::SimpleGlyph*>(g);
							if (glyph)
							{
								m_GlyphToDisplay = *vGlyphInfos;
								m_GlyphToDisplay.simpleGlyph.LoadSimpleGlyph(glyph);
								limitContour = m_GlyphToDisplay.simpleGlyph.GetCountContours();
#ifdef _DEBUG
								DebugPane::Instance()->SetGlyphToDebug(m_GlyphToDisplay);
#endif
								// show and active the glyph pane
								GuiLayout::Instance()->ShowAndFocusPane(PaneFlags::PANE_GLYPH);
								
								res = true;
							}
						}
					}
				}
			}
		}
	}

	return res;
}

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer
bool GlyphPane::DrawSimpleGlyph(
        GlyphInfos *vGlyph, FontInfos* vFontInfos,
        float vScale, int vCountSegments, bool vControlLines)
{
	if (vGlyph && vFontInfos)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;
		auto drawList = window->DrawList;

		/////////////////////////////////////////////////

		auto g = &vGlyph->simpleGlyph;
		if (g->isValid)
		{
			int cmax = (int)g->coords.size();
			ct::ivec4 rc = g->rc;

			ImVec2 contentSize = ImGui::GetContentRegionMax();
			ImRect glypRect = ImRect(((float)rc.x) * vScale, ((float)rc.y) * vScale, ((float)(rc.z - rc.x)) * vScale, ((float)(rc.w - rc.y)) * vScale);

			bool change = false;
			ImGui::PushItemWidth(200.0f);
			//change |= ImGui::SliderInt("tx", &_tx, -rc.z, rc.z); ImGui::SameLine();
			//change |= ImGui::SliderFloat("sx", &_sx, 0.01f, 10.0f);
			//change |= ImGui::SliderInt("ty", &_ty, -rc.w, rc.w); ImGui::SameLine();
			//change |= ImGui::SliderFloat("sy", &_sy, 0.01f, 10.0f);
			ImGui::PopItemWidth();

			if (change)
			{
				// will come back with svg or/and glyph edition
				//g->m_Translation = ct::ivec2(_tx, _ty); not functionnal for the moment
				//g->m_Scale = ct::dvec2((double)_sx, (double)_sy); for the moment scale is overwrite by merged system
			}

			ImVec2 glyphCenter = glypRect.GetCenter();
			ImVec2 pos = ImGui::GetCursorScreenPos() + contentSize * 0.5f - glyphCenter;

			if (ImGui::BeginMenuBar())
			{
				ImGui::PushItemWidth(100.0f);
				ImGui::SliderInt("Contours", &limitContour, 0, cmax);
				ImGui::PopItemWidth();

				ImGui::EndMenuBar();
			}

			ImGui::Text("You can select glyph in Current Font Pane");
			ImGui::Text("rc %i %i %i %i", rc.x, rc.y, rc.z, rc.w);
			
			// x 0 + blue
			drawList->AddLine(
				ct::toImVec2(g->Scale(ct::ivec2(0, (int32_t)ct::floor(rc.y * g->m_Scale.x)), vScale)) + pos,
				ct::toImVec2(g->Scale(ct::ivec2(0, (int32_t)ct::floor(rc.w * g->m_Scale.y)), vScale)) + pos,
				ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), 2.0f);

			// Ascent
			drawList->AddLine(
				ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.x * g->m_Scale.x), vFontInfos->m_Ascent), vScale)) + pos,
				ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.z * g->m_Scale.x), vFontInfos->m_Ascent), vScale)) + pos,
				ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 2.0f);

			// y 0
			drawList->AddLine(
				ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.x * g->m_Scale.x), 0), vScale)) + pos,
				ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.z * g->m_Scale.x), 0), vScale)) + pos,
				ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 1.0f);

			// Descent
			drawList->AddLine(
				ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.x * g->m_Scale.x), vFontInfos->m_Descent), vScale)) + pos,
				ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.z * g->m_Scale.x), vFontInfos->m_Descent), vScale)) + pos,
				ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 2.0f);

			for (int c = 0; c < cmax; c++)
			{
				if (c >= limitContour) break;

				int pmax = (int)g->coords[c].size();

				int firstOn = 0;
				for (int p = 0; p < pmax; p++)
				{
					if (g->IsOnCurve(c, p))
					{
						firstOn = p;
						break;
					}
				}

				// curve

				drawList->PathLineTo(ct::toImVec2(g->GetCoords(c, firstOn, vScale)) + pos);

				for (int i = 0; i < pmax; i++)
				{
					int icurr = firstOn + i + 1;
					int inext = firstOn + i + 2;
					ct::ivec2 cur = g->GetCoords(c, icurr, vScale);

					if (g->IsOnCurve(c, icurr))
					{
						drawList->PathLineTo(ct::toImVec2(cur) + pos);
					}
					else
					{
						ct::ivec2 nex = g->GetCoords(c, inext, vScale);
						if (!g->IsOnCurve(c, inext))
						{
							nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
							nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
						}
						drawList->PathQuadBezierCurveTo(
							ct::toImVec2(cur) + pos,
							ct::toImVec2(nex) + pos, 
							vCountSegments);
					}
				}

				drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Text), true);

#ifdef _DEBUG
				DebugPane::Instance()->DrawGlyphCurrentPoint(vScale, pos, drawList);
#endif

				if (vControlLines) // control lines
				{
					drawList->PathLineTo(ct::toImVec2(g->GetCoords(c, firstOn, vScale)) + pos);

					for (int i = 0; i < pmax; i++)
					{
						int icurr = firstOn + i + 1;
						int inext = firstOn + i + 2;
						ct::ivec2 cur = g->GetCoords(c, icurr, vScale);
						if (g->IsOnCurve(c, icurr))
						{
							drawList->PathLineTo(ct::toImVec2(cur) + pos);
						}
						else
						{
							ct::ivec2 nex = g->GetCoords(c, inext, vScale);
							if (!g->IsOnCurve(c, inext))
							{
								nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
								nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5); 
							}
							drawList->PathLineTo(ct::toImVec2(cur) + pos);
							drawList->PathLineTo(ct::toImVec2(nex) + pos);
						}
					}

					drawList->PathStroke(ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), true);
				}
			}
		}
	}

	return true;
}
