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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cTools.h>
#include <FileHelper.h>

#include <cinttypes> // printf zu

#include "sfntly/tag.h"
#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/port/memory_output_stream.h"
#include "sfntly/port/file_input_stream.h"
#include "sfntly/table/truetype/loca_table.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/table/core/maximum_profile_table.h"
#include "sfntly/table/core/post_script_table.h"
#include "sfntly/table/core/horizontal_header_table.h"
#include "sfntly/table/core/horizontal_metrics_table.h"
#include "sfntly/port/type.h"
#include "sfntly/port/refcount.h"

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
				static float _scale = 1.0f;
				
				if (ImGui::BeginMenuBar())
				{
					ImGui::SliderFloat("Scale", &_scale, 0.01f, 1.0f);

					ImGui::EndMenuBar();
				}

				DrawSimpleGlyph(m_SimpleGlyph, _scale);
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

bool GlyphPane::LoadGlyph(ProjectFile *vProjectFile, FontInfos* vFontInfos, ImWchar vCodepoint)
{
	bool res = false;

	if (vFontInfos)
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
						int32_t glyphId = m_fontInstance.m_CMapTable->GlyphId(vCodepoint);
						int32_t length = m_fontInstance.m_LocaTable->GlyphLength(glyphId);
						int32_t offset = m_fontInstance.m_LocaTable->GlyphOffset(glyphId);

						// Get the GLYF table for the current glyph id.
						auto g = m_fontInstance.m_GlyfTable->GetGlyph(offset, length);
						
						if (g->GlyphType() == sfntly::GlyphType::kSimple)
						{
							m_SimpleGlyph = down_cast<sfntly::GlyphTable::SimpleGlyph*>(g);
						}
						
						res = true;
					}
				}
			}
		}
	}

	return res;
}

bool GlyphPane::DrawSimpleGlyph(sfntly::Ptr<sfntly::GlyphTable::SimpleGlyph> vSimpleGlyph, float vScale)
{
	if (vSimpleGlyph)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;
		auto drawList = window->DrawList;

		/////////////////////////////////////////////////

		int MARGIN = 10;

		for (int c = 0, cmax = vSimpleGlyph->NumberOfContours(); c < cmax; c++)
		{
			ScreenCoordinateMapper screen(vSimpleGlyph, c, MARGIN, vScale, vSimpleGlyph->XMin(), vSimpleGlyph->YMax());
			int pmax = vSimpleGlyph->numberOfPoints(c);

			int firstOn = 0;
			for (int p = 0; p < pmax; p++)
			{
				if (screen.onCurve(p))
				{
					firstOn = p;
					break;
				}
			}

			for (int i = 0; i < pmax; i++)
			{
				int icurr = firstOn + i + 1;
				int inext = firstOn + i + 2;

				int currx = screen.cx(icurr);
				int curry = screen.cy(icurr);
				if (screen.onCurve(icurr))
				{
					drawList->PathLineTo(ImVec2(currx, curry) + ImGui::GetCursorScreenPos());
				}
				else
				{
					double nextx = screen.cx(inext);
					double nexty = screen.cy(inext);
					if (!screen.onCurve(inext))
					{
						nextx = 0.5 * (currx + nextx);
						nexty = 0.5 * (curry + nexty);
					}
					drawList->PathLineTo(ImVec2(currx, curry) + ImGui::GetCursorScreenPos());
					drawList->PathLineTo(ImVec2(nextx, nexty) + ImGui::GetCursorScreenPos());
					//path.quadTo(currx, curry, nextx, nexty);
				}
			}

			drawList->PathFillConvex(ImGui::GetColorU32(ImVec4(0, 0, 0, 1)));
		}
	}

	return true;
}
