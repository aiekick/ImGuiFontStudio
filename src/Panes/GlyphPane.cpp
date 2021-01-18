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

#include <MainFrame.h>

#include <Panes/Manager/LayoutManager.h>
#include <Gui/ImGuiWidgets.h>
#ifdef _DEBUG
#include <Panes/DebugPane.h>
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <sfntly/font_factory.h>

GlyphPane::GlyphPane() = default;
GlyphPane::~GlyphPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void GlyphPane::Init()
{
	
}

void GlyphPane::Unit()
{

}

int GlyphPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	paneWidgetId = vWidgetId;

	DrawGlyphPane(vProjectFile);

	return paneWidgetId;
}

void GlyphPane::DrawDialogsAndPopups(ProjectFile* /*vProjectFile*/)
{

}

int GlyphPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	UNUSED(vUserDatas);

	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (LayoutManager::Instance()->IsSpecificPaneFocused(PaneFlags::PANE_GLYPH))
		{
			const float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 4.0f;
			const float mrw = maxWidth / 1.0f - ImGui::GetStyle().FramePadding.x;

			if (ImGui::BeginFramedGroup("Glyph Pane"))
			{
				if (ImGui::SliderFloatDefaultCompact(mrw, "Scale", &vProjectFile->m_GlyphPreview_Scale, 0.01f, 2.0f, 1.0f))
				{
					vProjectFile->SetProjectChange();
				}

				if (ImGui::SliderIntDefaultCompact(mrw, "Segments", &vProjectFile->m_GlyphPreview_QuadBezierCountSegments, 0, 50, 0))
				{
					vProjectFile->SetProjectChange();
				}

				//ImGui::Checkbox("Stroke or Fill", &_stroke);
				if (ImGui::Checkbox("Control Lines", &vProjectFile->m_GlyphPreview_ShowControlLines))
				{
					vProjectFile->SetProjectChange();
				}
			}

			ImGui::EndFramedGroup(true);
		}
	}

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void GlyphPane::DrawGlyphPane(ProjectFile *vProjectFile)
{
	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_GLYPH)
	{
		if (ImGui::Begin<PaneFlags>(GLYPH_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_GLYPH,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				static float _ZoomPrecision = 200.0f;
				static float _ZoomPrecisionRatio = 1.0f / _ZoomPrecision;
				if (ImGui::SliderFloatDefaultCompact(ImGui::GetContentRegionAvail().x, "Zoom Precision", &_ZoomPrecision, 1.0f, 2000.0f, 200.0f))
				{
					_ZoomPrecision = ImMax(_ZoomPrecision, 1.0f);
					_ZoomPrecisionRatio = 1.0f / _ZoomPrecision;
				}

				if (ImGui::IsWindowHovered())
				{
					if (IS_FLOAT_DIFFERENT(ImGui::GetIO().MouseWheel, 0.0f))
					{
						vProjectFile->m_GlyphPreview_Scale += ImGui::GetIO().MouseWheel * _ZoomPrecisionRatio;
						vProjectFile->m_GlyphPreview_Scale = ImMax(vProjectFile->m_GlyphPreview_Scale, 0.00001f);
					}
				}

				DrawSimpleGlyph(vProjectFile);
			}
		}

		ImGui::End();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// LOAD GLYPH ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer

static int limitContour = 0;

bool GlyphPane::LoadGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, std::weak_ptr<GlyphInfos> vGlyphInfos)
{
	bool res = false;

	if (vFontInfos && !vGlyphInfos.expired())
	{
		auto glyphInfosPtr = vGlyphInfos.lock();
		if (glyphInfosPtr)
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
							uint32_t codePoint = glyphInfosPtr->glyph.Codepoint;
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
									m_GlyphToDisplay = vGlyphInfos;
									glyphInfosPtr->simpleGlyph.LoadSimpleGlyph(glyph);
									limitContour = glyphInfosPtr->simpleGlyph.GetCountContours();
									glyphInfosPtr->simpleGlyph.m_Translation = glyphInfosPtr->m_Translation;
									glyphInfosPtr->simpleGlyph.m_Scale = glyphInfosPtr->m_Scale;
#ifdef _DEBUG
									DebugPane::Instance()->SetGlyphToDebug(m_GlyphToDisplay);
#endif
									// show and active the glyph pane
									LayoutManager::Instance()->ShowAndFocusSpecificPane(PaneFlags::PANE_GLYPH);

									res = true;
								}
							}
						}
					}
				}
			}
		}
	}

	return res;
}

void GlyphPane::Clear()
{
	if (!m_GlyphToDisplay.expired())
	{
		auto glyphInfosPtr = m_GlyphToDisplay.lock();
		if (glyphInfosPtr)
		{
			glyphInfosPtr->simpleGlyph.Clear();

		}
	}
	m_GlyphToDisplay.reset();
	limitContour = 0;

#ifdef _DEBUG
	DebugPane::Instance()->Clear();
#endif
}

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer
bool GlyphPane::DrawSimpleGlyph(ProjectFile* vProjectFile)
{
	if (!m_GlyphToDisplay.expired() && vProjectFile->m_SelectedFont)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;
		auto drawList = window->DrawList;

		/////////////////////////////////////////////////

		auto glyphInfosPtr = m_GlyphToDisplay.lock();
		if (glyphInfosPtr)
		{
			auto g = &glyphInfosPtr->simpleGlyph;
			if (g->isValid)
			{
				int cmax = (int)g->coords.size();
				ct::ivec4 rc = g->rc;

				ImVec2 contentSize = ImGui::GetContentRegionAvail();
				ImRect glypRect = ImRect(
					((float)rc.x) * vProjectFile->m_GlyphPreview_Scale, 
					((float)rc.y) * vProjectFile->m_GlyphPreview_Scale, 
					((float)(rc.z - rc.x)) * vProjectFile->m_GlyphPreview_Scale, 
					((float)(rc.w - rc.y)) * vProjectFile->m_GlyphPreview_Scale);

				bool change = false;
				float aw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f) * 0.5f;
				change |= ImGui::SliderFloatDefaultCompact(aw, "Trans X", &glyphInfosPtr->m_Translation.x, (float)-rc.z, (float)rc.z, 0.0f); ImGui::SameLine();
				change |= ImGui::SliderFloatDefaultCompact(aw, "Trans Y", &glyphInfosPtr->m_Translation.y, (float)-rc.w, (float)rc.w, 0.0f);
				//change |= ImGui::SliderFloatDefaultCompact(aw, "Scale X", &glyphInfosPtr->m_Scale.x, 0.01f, 3.0f, 1.0f); ImGui::SameLine();
				//change |= ImGui::SliderFloatDefaultCompact(aw, "Scale Y", &glyphInfosPtr->m_Scale.y, 0.01f, 3.0f, 1.0f);

				if (change)
				{
					// will come back with svg or/and glyph edition
					g->m_Translation = glyphInfosPtr->m_Translation; // wip
					g->m_Scale = glyphInfosPtr->m_Scale;// for the moment scale is overwrite by merged system
					vProjectFile->SetProjectChange();
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
				ImGui::Text("the transformed glyph must keep in this rect : %i %i %i %i", rc.x, rc.y, rc.z, rc.w);

				// x 0 + blue
				drawList->AddLine(
					ct::toImVec2(g->Scale(ct::ivec2(0, (int32_t)ct::floor(rc.y)), vProjectFile->m_GlyphPreview_Scale)) + pos,
					ct::toImVec2(g->Scale(ct::ivec2(0, (int32_t)ct::floor(rc.w)), vProjectFile->m_GlyphPreview_Scale)) + pos,
					ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), 2.0f);

				// Ascent
				drawList->AddLine(
					ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.x), vProjectFile->m_SelectedFont->m_Ascent), vProjectFile->m_GlyphPreview_Scale)) + pos,
					ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.z), vProjectFile->m_SelectedFont->m_Ascent), vProjectFile->m_GlyphPreview_Scale)) + pos,
					ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 2.0f);

				// y 0
				drawList->AddLine(
					ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.x), 0), vProjectFile->m_GlyphPreview_Scale)) + pos,
					ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.z), 0), vProjectFile->m_GlyphPreview_Scale)) + pos,
					ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 1.0f);

				// Descent
				drawList->AddLine(
					ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.x), vProjectFile->m_SelectedFont->m_Descent), vProjectFile->m_GlyphPreview_Scale)) + pos,
					ct::toImVec2(g->Scale(ct::ivec2((int32_t)ct::floor(rc.z), vProjectFile->m_SelectedFont->m_Descent), vProjectFile->m_GlyphPreview_Scale)) + pos,
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

					drawList->PathLineTo(ct::toImVec2(g->GetCoords(c, firstOn, vProjectFile->m_GlyphPreview_Scale)) + pos);

					for (int i = 0; i < pmax; i++)
					{
						int icurr = firstOn + i + 1;
						int inext = firstOn + i + 2;
						ct::ivec2 cur = g->GetCoords(c, icurr, vProjectFile->m_GlyphPreview_Scale);

						if (g->IsOnCurve(c, icurr))
						{
							drawList->PathLineTo(ct::toImVec2(cur) + pos);
						}
						else
						{
							ct::ivec2 nex = g->GetCoords(c, inext, vProjectFile->m_GlyphPreview_Scale);
							if (!g->IsOnCurve(c, inext))
							{
								nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
								nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
							}
							drawList->PathBezierQuadraticCurveTo(
								ct::toImVec2(cur) + pos,
								ct::toImVec2(nex) + pos,
								vProjectFile->m_GlyphPreview_QuadBezierCountSegments);
						}
					}

					drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Text), true);

#ifdef _DEBUG
					DebugPane::Instance()->DrawGlyphCurrentPoint(vProjectFile->m_GlyphPreview_Scale, pos, drawList);
#endif

					if (vProjectFile->m_GlyphPreview_ShowControlLines) // control lines
					{
						drawList->PathLineTo(ct::toImVec2(g->GetCoords(c, firstOn, vProjectFile->m_GlyphPreview_Scale)) + pos);

						for (int i = 0; i < pmax; i++)
						{
							int icurr = firstOn + i + 1;
							int inext = firstOn + i + 2;
							ct::ivec2 cur = g->GetCoords(c, icurr, vProjectFile->m_GlyphPreview_Scale);
							if (g->IsOnCurve(c, icurr))
							{
								drawList->PathLineTo(ct::toImVec2(cur) + pos);
							}
							else
							{
								ct::ivec2 nex = g->GetCoords(c, inext, vProjectFile->m_GlyphPreview_Scale);
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
	}

	return true;
}
