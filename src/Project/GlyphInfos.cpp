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
#include "GlyphInfos.h"

#include <Project/ProjectFile.h>

#include <utility>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <Gui/ImGuiWidgets.h>
#include <Helper/ImGuiThemeHelper.h>
#include <Helper/AssetManager.h>
#include <Panes/DebugPane.h>

 ///////////////////////////////////////////////////////////////////////////////////
 //// PUBLIC : STATIC //////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////////////////////

int GlyphDisplayHelper::CalcGlyphsCountAndSize(
	ProjectFile* vProjectFile,				/* project file for save some vars  */
	ImVec2* vCellSize,						/* cell size						*/
	ImVec2* vGlyphSize,						/* glyph size (cell - paddings)		*/
	bool vGlyphEdited,						/* in edition						*/
	bool vForceEditMode,					/* edition forced					*/
	bool vForceEditModeOneColumn			/* edition in one column			*/)
{
	if (vProjectFile && vCellSize && vGlyphSize)
	{
		float aw = ImGui::GetContentRegionAvail().x;
		
		int glyphCount = vProjectFile->m_Preview_Glyph_CountX;
		float glyphWidth = (float)vProjectFile->m_Preview_Glyph_Width;
		
		// GlyphSize est Menant, puis glyphCount est appliqué
		if (vProjectFile->m_GlyphDisplayTuningMode & GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE)
		{
			glyphCount = (int)(aw / ct::maxi(glyphWidth, 1.0f));
			glyphWidth = aw / (float)ct::maxi(glyphCount, 1);
			if (vProjectFile->m_GlyphSizePolicyChangeFromWidgetUse)
			{
				vProjectFile->m_Preview_Glyph_CountX = glyphCount;
			}
		}
		// GlyphCount est Menant, dont m_Preview_Glyph_CountX n'est jamais réécrit en dehors du user
		else if (vProjectFile->m_GlyphDisplayTuningMode & GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT)
		{
			glyphWidth = aw / (float)ct::maxi(glyphCount, 1);
			if (vProjectFile->m_GlyphSizePolicyChangeFromWidgetUse)
			{
				vProjectFile->m_Preview_Glyph_Width = glyphWidth;
			}
		}
			
		if (glyphCount > 0)
		{
			*vCellSize = ImVec2(glyphWidth, glyphWidth);
			*vGlyphSize = *vCellSize - ImGui::GetStyle().ItemSpacing;

			if (vGlyphEdited || vForceEditMode)
			{
				// we will forgot vGlyphCountX
				// cell_size_y will be item height x 2 + padding_y
				// cell_size_x will be cell_size_y + padding_x + 100
				const ImVec2 label_size = ImGui::CalcTextSize("gt", nullptr, true);
				const ImVec2 frame_size = ImGui::CalcItemSize(ImVec2(0, 0), ImGui::CalcItemWidth(),
					label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f);
				float cell_size_y = frame_size.y * 2.0f + ImGui::GetStyle().FramePadding.y;
				float cell_size_x = cell_size_y + ImGui::GetStyle().FramePadding.x + GLYPH_EDIT_CONTROL_WIDTH;
				glyphCount = ct::maxi(1, (int)ct::floor(aw / cell_size_x));
				*vGlyphSize = ImVec2(cell_size_y, cell_size_y);
			}

			if (vForceEditModeOneColumn && vForceEditMode)
			{
				glyphCount = 1;
			}

			*vGlyphSize -= ImGui::GetStyle().FramePadding * 2.0f;
		}

		return glyphCount;
	}

	assert(0);
	return 0;
}

 //////////////////////////////////////////////////////////
 //// SIMPLE GLYPH ////////////////////////////////////////
 //////////////////////////////////////////////////////////

void SimpleGlyph_Solo::Clear()
{
	coords.clear();
	onCurve.clear();
	isValid = false;
	rc = 0;
}

void SimpleGlyph_Solo::LoadSimpleGlyph(sfntly::GlyphTable::SimpleGlyph *vGlyph)
{
	if (vGlyph)
	{
		vGlyph->Initialize();
		Clear();
		int cmax = vGlyph->NumberOfContours();
		for (int c = 0; c < cmax; c++)
		{
			onCurve.emplace_back();
			coords.emplace_back();
			int pmax = vGlyph->numberOfPoints(c);
			for (int p = 0; p < pmax; p++)
			{
				int32_t x = vGlyph->xCoordinate(c, p);
				int32_t y = vGlyph->yCoordinate(c, p);
				coords[c].push_back(ct::ivec2(x, y));
				bool oc = vGlyph->onCurve(c, p);
				onCurve[c].push_back(oc);
			}
		}
		isValid = !coords.empty();
		rc.x = vGlyph->XMin();
		rc.y = vGlyph->YMin();
		rc.z = vGlyph->XMax();
		rc.w = vGlyph->YMax();
	}
}

int SimpleGlyph_Solo::GetCountContours() const
{
	return (int)coords.size();
}

ct::ivec2 SimpleGlyph_Solo::GetCoords(int32_t vContour, int32_t vPoint)
{
	int count = (int)coords[vContour].size();

	ct::ivec2 p = coords[vContour][vPoint % count];

	// apply transformation
	p.x = (int)(p.x * m_Scale.x);
	p.y = (int)(p.y * m_Scale.y);
	p += ct::ivec2(m_Translation);

	return p;
}

bool SimpleGlyph_Solo::IsOnCurve(int32_t vContour, int32_t vPoint)
{
	int count = (int)coords[vContour].size();
	return onCurve[vContour][vPoint % count];
}

ct::ivec2 SimpleGlyph_Solo::Scale(ct::ivec2 p, double scale) const
{
	return {
		(int)round(scale * ((double)p.x - (double)rc.x)),
		(int)round(scale * ((double)rc.w - (double)p.y))};
}

/*ct::ivec2 SimpleGlyph_Solo::GetCoords(int32_t vContour, int32_t vPoint, double scale)
{
	return Scale(GetCoords(vContour, vPoint), scale);
}*/

void SimpleGlyph_Solo::ClearTransform()
{
	ClearTranslation();
	ClearScale();
}

void SimpleGlyph_Solo::ClearTranslation()
{
	m_Translation = 0.0f;
}

void SimpleGlyph_Solo::ClearScale()
{
	m_Scale = 1.0f;
}


ImVec2 SimpleGlyph_Solo::getScreenToLocal(ImVec2 vScreenPos, ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin)
{
	ImVec2 localPos;
	localPos.x = (vScreenPos.x - vZoneStart.x - vWorldBBoxOrigin.x) / vWorldScale + vLocalBBoxOrigin.x;
	localPos.y = (vWorlBBoxSize.y - (vScreenPos.y - vZoneStart.y - vWorldBBoxOrigin.y)) / vWorldScale + vLocalBBoxOrigin.y;
	return localPos;
}

ImVec2 SimpleGlyph_Solo::getLocalToScreen(ImVec2 vLocalPos, ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin)
{
	ImVec2 screenPos;
	screenPos.x = (vLocalPos.x - vLocalBBoxOrigin.x) * vWorldScale + vWorldBBoxOrigin.x + vZoneStart.x;
	screenPos.y = vWorlBBoxSize.y - (vLocalPos.y - vLocalBBoxOrigin.y) * vWorldScale + vWorldBBoxOrigin.y + vZoneStart.y;
	return screenPos;
}

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer
// we will display the glyph metrics like here : https://www.libsdl.org/projects/SDL_ttf/docs/metrics.png
void SimpleGlyph_Solo::DrawCurves(
	float vGlobalScale, 
	std::shared_ptr<FontInfos> vFontInfos,
	std::shared_ptr<GlyphInfos> vGlyphInfos,
	int vMaxContour, int vQuadBezierCountSegments, 
	GlyphDrawingFlags vGlyphDrawingFlags)
{
	if (isValid && vFontInfos.use_count() && vGlyphInfos.use_count())
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
		auto drawList = window->DrawList;

		ImVec2 contentStart = ImGui::GetCursorScreenPos();
		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		ImVec2 contentCenter = contentStart + contentSize * 0.5f;

		ImGui::PushClipRect(contentStart, contentStart + contentSize, false);
		{
			drawList->AddRectFilled(contentStart, contentStart + contentSize, ImGui::GetColorU32(ImGuiCol_ChildBg));
			drawList->AddRect(contentStart, contentStart + contentSize, ImGui::GetColorU32(ImGuiCol_Text));

			auto frc = vFontInfos->m_BoundingBox;
			ImRect fontBBox = ImRect((float)frc.x, (float)frc.y, (float)frc.z, (float)frc.w);
			ImVec2 pScale = contentSize / fontBBox.GetSize();
			float newScale = ImMin(pScale.x, pScale.y) * vGlobalScale;
			fontBBox = ImRect(contentCenter - fontBBox.GetSize() * 0.5f * newScale, contentCenter + fontBBox.GetSize() * 0.5f * newScale);
			ImVec2 fontBBoxSize = fontBBox.GetSize();
			ImVec2 fbboxOrign = fontBBox.Min - contentStart;

			//drawList->AddCircleFilled(contentCenter, 10.0f, ImGui::GetColorU32(ImGuiCol_Text));
			//drawList->AddRect(fontBBox.Min, fontBBox.Max, ImGui::GetColorU32(ImGuiCol_Text));
			const float tlh = ImGui::GetTextLineHeight();

#define LocalToScreen(a) getLocalToScreen(ImVec2((float)a.x, (float)a.y), contentStart, fbboxOrign, fontBBoxSize, newScale, ImVec2((float)frc.x, (float)frc.y))
#define ScreenToLocal(a) getScreenToLocal(ImVec2((float)a.x, (float)a.y), contentStart, fbboxOrign, fontBBoxSize, newScale, ImVec2((float)frc.x, (float)frc.y))

			///////////////////////////////////////////////////////////////
			// show pos in local space
			if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(fontBBox.Min, fontBBox.Max))
			{
				ImVec2 localPos = ScreenToLocal(ImGui::GetMousePos());
				ImGui::SetTooltip("px : %.2f\npy : %.2f", localPos.x, localPos.y);
			}
			///////////////////////////////////////////////////////////////

			float ascent = (float)vFontInfos->m_Ascent;
			float descent = (float)vFontInfos->m_Descent;
			float advanceX = vGlyphInfos->glyph.AdvanceX / vFontInfos->m_Point;

			ImVec2 min, max;
			float triangleWidth = 20.0f;
			float triangleHeight = 20.0f;

			float step = 200.0f;
			float ox = 0.0f - step;
			float lx = advanceX + step;
			float oy = descent - step;
			float ly = ascent + step;

			
			// canvas grid // code from ImGui::Demo::CustomRendering::Canvas
			if (vGlyphDrawingFlags & GLYPH_DRAWING_CANVAS_GRID)
			{
				const ImU32 lineCol = ImGui::GetColorU32(ImGuiCol_Text, 0.25f);
				ImVec2 canvas_org = LocalToScreen(ImVec2(0.0f, 0.0f));
				min = contentStart;
				max = contentStart + contentSize;
				const float GRID_STEP = 32.0f;
				float x0 = canvas_org.x - ImFloor((canvas_org.x - min.x) / GRID_STEP) * GRID_STEP;
				float y0 = canvas_org.y - ImFloor((canvas_org.y - min.y) / GRID_STEP) * GRID_STEP;
				for (float x = x0; x < max.x; x += GRID_STEP)
					drawList->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), lineCol);
				for (float y = y0; y < max.y; y += GRID_STEP)
					drawList->AddLine(ImVec2(min.x, y), ImVec2(max.x, y), lineCol);
			}
			
			const ImU32 textCol = ImGui::GetColorU32(ImGuiCol_Text);
			
			// origin axis X
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_AXIS_X)
			{
				const ImU32 XAxisCol = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
				min = LocalToScreen(ImVec2(ox, 0.0f));
				max = LocalToScreen(ImVec2(lx, 0.0f));
				drawList->AddLine(min, max, XAxisCol, 2.0f);
				drawList->AddTriangleFilled(
					max,
					max - ImVec2(triangleWidth, triangleHeight * 0.5f),
					max - ImVec2(triangleWidth, triangleHeight * -0.5f),
					XAxisCol);
			}

			// origin axis Y
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_AXIS_Y)
			{
				const ImU32 YAxisCol = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered);
				min = LocalToScreen(ImVec2(0.0f, oy));
				max = LocalToScreen(ImVec2(0.0f, ly));
				drawList->AddLine(min, max, YAxisCol, 2.0f);
				drawList->AddTriangleFilled(
					max,
					max + ImVec2(triangleWidth * 0.5f, triangleHeight),
					max + ImVec2(triangleWidth * -0.5f, triangleHeight),
					YAxisCol);
			}

			// font bounding box
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_BBOX)
			{
				min = LocalToScreen(ImVec2((float)frc.x, (float)frc.y));
				max = LocalToScreen(ImVec2((float)frc.z, (float)frc.w));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, min.y - tlh), textCol, "Font BBox");
				drawList->AddRect(min, max, textCol, 0.0f, 15, 2.0f);
			}

			// glyph bounding box
			if (vGlyphDrawingFlags & GLYPH_DRAWING_GLYPH_BBOX)
			{
				min = LocalToScreen(ImVec2((float)rc.x + m_Translation.x, (float)rc.y + m_Translation.y));
				max = LocalToScreen(ImVec2((float)rc.z + m_Translation.x, (float)rc.w + m_Translation.y));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, max.y - tlh), textCol, "Glyph BBox");
				drawList->AddRect(min, max, textCol, 0.0f, 15, 2.0f);
			}

			// adv x
			if (vGlyphDrawingFlags & GLYPH_DRAWING_GLYPH_ADVANCEX)
			{
				const ImU32 advxCol = ImGui::GetColorU32(ImGuiCol_PlotLines);
				min = LocalToScreen(ImVec2(advanceX, oy));
				max = LocalToScreen(ImVec2(advanceX, ly));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, max.y - tlh), textCol, "Glyph Advance X");
				drawList->AddLine(min, max, advxCol, 2.0f);
			}

			// Ascent
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_ASCENT)
			{
				const ImU32 ascCol = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
				min = LocalToScreen(ImVec2(ox, ascent));
				max = LocalToScreen(ImVec2(lx, ascent));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, max.y - tlh), textCol, "Font Ascent");
				drawList->AddLine(min, max, ascCol, 2.0f);
			}

			// Descent
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_DESCENT)
			{
				const ImU32 descCol = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
				min = LocalToScreen(ImVec2(ox, descent));
				max = LocalToScreen(ImVec2(lx, descent));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, max.y - tlh), textCol, "Font Descent");
				drawList->AddLine(min, max, descCol, 2.0f);
			}


			// glyph
			int cmax = (int)coords.size();
			for (int c = 0; c < cmax; c++)
			{
				if (c >= vMaxContour) break;

				int pmax = (int)coords[c].size();

				int firstOn = 0;
				for (int p = 0; p < pmax; p++)
				{
					if (IsOnCurve(c, p))
					{
						firstOn = p;
						break;
					}
				}

				// curve

				drawList->PathLineTo(LocalToScreen(GetCoords(c, firstOn)));

				for (int i = 0; i < pmax; i++)
				{
					int icurr = firstOn + i + 1;
					int inext = firstOn + i + 2;
					ct::ivec2 cur = GetCoords(c, icurr);

					if (IsOnCurve(c, icurr))
					{
						drawList->PathLineTo(LocalToScreen(cur));
					}
					else
					{
						ct::ivec2 nex = GetCoords(c, inext);
						if (!IsOnCurve(c, inext))
						{
							nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
							nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
						}
						drawList->PathBezierQuadraticCurveTo(LocalToScreen(cur), LocalToScreen(nex), vQuadBezierCountSegments);
					}
				}

				drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Text), true);

#ifdef _DEBUG
				//DebugPane::Instance()->DrawGlyphCurrentPoint(newScale, posOrigin, drawList);
#endif

				if (vGlyphDrawingFlags & GLYPH_DRAWING_GLYPH_CONTROL_LINES) // control lines
				{
					drawList->PathLineTo(LocalToScreen(GetCoords(c, firstOn)));

					for (int i = 0; i < pmax; i++)
					{
						int icurr = firstOn + i + 1;
						int inext = firstOn + i + 2;
						ct::ivec2 cur = GetCoords(c, icurr);
						if (IsOnCurve(c, icurr))
						{
							drawList->PathLineTo(LocalToScreen(cur));
						}
						else
						{
							ct::ivec2 nex = GetCoords(c, inext);
							if (!IsOnCurve(c, inext))
							{
								nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
								nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
							}
							drawList->PathLineTo(LocalToScreen(cur));
							drawList->PathLineTo(LocalToScreen(nex));
						}
					}

					drawList->PathStroke(ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), true);
				}
			}

			// origin point
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_ORIGIN_XY)
			{
				const ImU32 PointCol = ImGui::GetColorU32(ImGuiCol_Text, 1.0f);
				min = LocalToScreen(ImVec2(0.0f, 0.0f));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
				{
					const char* txt = "Origin";
					ImVec2 txtSize = ImGui::CalcTextSize(txt);
					drawList->AddText(ImVec2(min.x - txtSize.x - tlh * 0.5f, min.y - tlh), textCol, txt);
				}
				drawList->AddCircleFilled(min, 5.0f, PointCol);
			}

#undef LocalToScreen
#undef ScreenToLocal
		}
		ImGui::PopClipRect();
	}
}

//////////////////////////////////////////////////////////
//// COMPOSITE GLYPH /////////////////////////////////////
//////////////////////////////////////////////////////////

void CompositeGlyph_Solo::Clear()
{
	coords.clear();
	onCurve.clear();
	isValid = false;
	rc = 0;
}

void CompositeGlyph_Solo::LoadCompositeGlyph(sfntly::GlyphTable::CompositeGlyph* vGlyph)
{
	if (vGlyph)
	{
		vGlyph->Initialize();
		Clear();
		/*int cmax = vGlyph->NumberOfContours();
		for (int c = 0; c < cmax; c++)
		{
			onCurve.emplace_back();
			coords.emplace_back();
			int pmax = vGlyph->numberOfPoints(c);
			for (int p = 0; p < pmax; p++)
			{
				int32_t x = vGlyph->xCoordinate(c, p);
				int32_t y = vGlyph->yCoordinate(c, p);
				coords[c].push_back(ct::ivec2(x, y));
				bool oc = vGlyph->onCurve(c, p);
				onCurve[c].push_back(oc);
			}
		}
		isValid = !coords.empty();
		rc.x = vGlyph->XMin();
		rc.y = vGlyph->YMin();
		rc.z = vGlyph->XMax();
		rc.w = vGlyph->YMax();*/
	}
}

int CompositeGlyph_Solo::GetCountContours() const
{
	return (int)coords.size();
}

ct::ivec2 CompositeGlyph_Solo::GetCoords(int32_t vContour, int32_t vPoint)
{
	int count = (int)coords[vContour].size();

	ct::ivec2 p = coords[vContour][vPoint % count];

	// apply transformation
	p.x = (int)(p.x * m_Scale.x);
	p.y = (int)(p.y * m_Scale.y);
	p += ct::ivec2(m_Translation);

	return p;
}

bool CompositeGlyph_Solo::IsOnCurve(int32_t vContour, int32_t vPoint)
{
	int count = (int)coords[vContour].size();
	return onCurve[vContour][vPoint % count];
}

ct::ivec2 CompositeGlyph_Solo::Scale(ct::ivec2 p, double scale) const
{
	return {
		(int)round(scale * ((double)p.x - (double)rc.x)),
		(int)round(scale * ((double)rc.w - (double)p.y)) };
}

/*ct::ivec2 SimpleGlyph_Solo::GetCoords(int32_t vContour, int32_t vPoint, double scale)
{
	return Scale(GetCoords(vContour, vPoint), scale);
}*/

void CompositeGlyph_Solo::ClearTransform()
{
	ClearTranslation();
	ClearScale();
}

void CompositeGlyph_Solo::ClearTranslation()
{
	m_Translation = 0.0f;
}

void CompositeGlyph_Solo::ClearScale()
{
	m_Scale = 1.0f;
}

ImVec2 CompositeGlyph_Solo::getScreenToLocal(ImVec2 vScreenPos, ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin)
{
	ImVec2 localPos;
	localPos.x = (vScreenPos.x - vZoneStart.x - vWorldBBoxOrigin.x) / vWorldScale + vLocalBBoxOrigin.x;
	localPos.y = (vWorlBBoxSize.y - (vScreenPos.y - vZoneStart.y - vWorldBBoxOrigin.y)) / vWorldScale + vLocalBBoxOrigin.y;
	return localPos;
}

ImVec2 CompositeGlyph_Solo::getLocalToScreen(ImVec2 vLocalPos, ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin)
{
	ImVec2 screenPos;
	screenPos.x = (vLocalPos.x - vLocalBBoxOrigin.x) * vWorldScale + vWorldBBoxOrigin.x + vZoneStart.x;
	screenPos.y = vWorlBBoxSize.y - (vLocalPos.y - vLocalBBoxOrigin.y) * vWorldScale + vWorldBBoxOrigin.y + vZoneStart.y;
	return screenPos;
}

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer
// we will display the glyph metrics like here : https://www.libsdl.org/projects/SDL_ttf/docs/metrics.png
void CompositeGlyph_Solo::DrawCurves(
	float vGlobalScale,
	std::shared_ptr<FontInfos> vFontInfos,
	std::shared_ptr<GlyphInfos> vGlyphInfos,
	int vMaxContour, int vQuadBezierCountSegments,
	GlyphDrawingFlags vGlyphDrawingFlags)
{
	if (isValid && vFontInfos.use_count() && vGlyphInfos.use_count())
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
		auto drawList = window->DrawList;

		ImVec2 contentStart = ImGui::GetCursorScreenPos();
		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		ImVec2 contentCenter = contentStart + contentSize * 0.5f;

		ImGui::PushClipRect(contentStart, contentStart + contentSize, false);
		{
			drawList->AddRectFilled(contentStart, contentStart + contentSize, ImGui::GetColorU32(ImGuiCol_ChildBg));
			drawList->AddRect(contentStart, contentStart + contentSize, ImGui::GetColorU32(ImGuiCol_Text));

			auto frc = vFontInfos->m_BoundingBox;
			ImRect fontBBox = ImRect((float)frc.x, (float)frc.y, (float)frc.z, (float)frc.w);
			ImVec2 pScale = contentSize / fontBBox.GetSize();
			float newScale = ImMin(pScale.x, pScale.y) * vGlobalScale;
			fontBBox = ImRect(contentCenter - fontBBox.GetSize() * 0.5f * newScale, contentCenter + fontBBox.GetSize() * 0.5f * newScale);
			ImVec2 fontBBoxSize = fontBBox.GetSize();
			ImVec2 fbboxOrign = fontBBox.Min - contentStart;

			//drawList->AddCircleFilled(contentCenter, 10.0f, ImGui::GetColorU32(ImGuiCol_Text));
			//drawList->AddRect(fontBBox.Min, fontBBox.Max, ImGui::GetColorU32(ImGuiCol_Text));
			const float tlh = ImGui::GetTextLineHeight();

#define LocalToScreen(a) getLocalToScreen(ImVec2((float)a.x, (float)a.y), contentStart, fbboxOrign, fontBBoxSize, newScale, ImVec2((float)frc.x, (float)frc.y))
#define ScreenToLocal(a) getScreenToLocal(ImVec2((float)a.x, (float)a.y), contentStart, fbboxOrign, fontBBoxSize, newScale, ImVec2((float)frc.x, (float)frc.y))

			///////////////////////////////////////////////////////////////
			// show pos in local space
			if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(fontBBox.Min, fontBBox.Max))
			{
				ImVec2 localPos = ScreenToLocal(ImGui::GetMousePos());
				ImGui::SetTooltip("px : %.2f\npy : %.2f", localPos.x, localPos.y);
			}
			///////////////////////////////////////////////////////////////

			float ascent = (float)vFontInfos->m_Ascent;
			float descent = (float)vFontInfos->m_Descent;
			float advanceX = vGlyphInfos->glyph.AdvanceX / vFontInfos->m_Point;

			ImVec2 min, max;
			float triangleWidth = 20.0f;
			float triangleHeight = 20.0f;

			float step = 200.0f;
			float ox = 0.0f - step;
			float lx = advanceX + step;
			float oy = descent - step;
			float ly = ascent + step;


			// canvas grid // code from ImGui::Demo::CustomRendering::Canvas
			if (vGlyphDrawingFlags & GLYPH_DRAWING_CANVAS_GRID)
			{
				const ImU32 lineCol = ImGui::GetColorU32(ImGuiCol_Text, 0.25f);
				ImVec2 canvas_org = LocalToScreen(ImVec2(0.0f, 0.0f));
				min = contentStart;
				max = contentStart + contentSize;
				const float GRID_STEP = 32.0f;
				float x0 = canvas_org.x - ImFloor((canvas_org.x - min.x) / GRID_STEP) * GRID_STEP;
				float y0 = canvas_org.y - ImFloor((canvas_org.y - min.y) / GRID_STEP) * GRID_STEP;
				for (float x = x0; x < max.x; x += GRID_STEP)
					drawList->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), lineCol);
				for (float y = y0; y < max.y; y += GRID_STEP)
					drawList->AddLine(ImVec2(min.x, y), ImVec2(max.x, y), lineCol);
			}

			const ImU32 textCol = ImGui::GetColorU32(ImGuiCol_Text);

			// origin axis X
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_AXIS_X)
			{
				const ImU32 XAxisCol = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
				min = LocalToScreen(ImVec2(ox, 0.0f));
				max = LocalToScreen(ImVec2(lx, 0.0f));
				drawList->AddLine(min, max, XAxisCol, 2.0f);
				drawList->AddTriangleFilled(
					max,
					max - ImVec2(triangleWidth, triangleHeight * 0.5f),
					max - ImVec2(triangleWidth, triangleHeight * -0.5f),
					XAxisCol);
			}

			// origin axis Y
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_AXIS_Y)
			{
				const ImU32 YAxisCol = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered);
				min = LocalToScreen(ImVec2(0.0f, oy));
				max = LocalToScreen(ImVec2(0.0f, ly));
				drawList->AddLine(min, max, YAxisCol, 2.0f);
				drawList->AddTriangleFilled(
					max,
					max + ImVec2(triangleWidth * 0.5f, triangleHeight),
					max + ImVec2(triangleWidth * -0.5f, triangleHeight),
					YAxisCol);
			}

			// font bounding box
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_BBOX)
			{
				min = LocalToScreen(ImVec2((float)frc.x, (float)frc.y));
				max = LocalToScreen(ImVec2((float)frc.z, (float)frc.w));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, min.y - tlh), textCol, "Font BBox");
				drawList->AddRect(min, max, textCol, 0.0f, 15, 2.0f);
			}

			// glyph bounding box
			if (vGlyphDrawingFlags & GLYPH_DRAWING_GLYPH_BBOX)
			{
				min = LocalToScreen(ImVec2((float)rc.x + m_Translation.x, (float)rc.y + m_Translation.y));
				max = LocalToScreen(ImVec2((float)rc.z + m_Translation.x, (float)rc.w + m_Translation.y));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, max.y - tlh), textCol, "Glyph BBox");
				drawList->AddRect(min, max, textCol, 0.0f, 15, 2.0f);
			}

			// adv x
			if (vGlyphDrawingFlags & GLYPH_DRAWING_GLYPH_ADVANCEX)
			{
				const ImU32 advxCol = ImGui::GetColorU32(ImGuiCol_PlotLines);
				min = LocalToScreen(ImVec2(advanceX, oy));
				max = LocalToScreen(ImVec2(advanceX, ly));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, max.y - tlh), textCol, "Glyph Advance X");
				drawList->AddLine(min, max, advxCol, 2.0f);
			}

			// Ascent
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_ASCENT)
			{
				const ImU32 ascCol = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
				min = LocalToScreen(ImVec2(ox, ascent));
				max = LocalToScreen(ImVec2(lx, ascent));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, max.y - tlh), textCol, "Font Ascent");
				drawList->AddLine(min, max, ascCol, 2.0f);
			}

			// Descent
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_DESCENT)
			{
				const ImU32 descCol = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
				min = LocalToScreen(ImVec2(ox, descent));
				max = LocalToScreen(ImVec2(lx, descent));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
					drawList->AddText(ImVec2(min.x + tlh, max.y - tlh), textCol, "Font Descent");
				drawList->AddLine(min, max, descCol, 2.0f);
			}


			// glyph
			int cmax = (int)coords.size();
			for (int c = 0; c < cmax; c++)
			{
				if (c >= vMaxContour) break;

				int pmax = (int)coords[c].size();

				int firstOn = 0;
				for (int p = 0; p < pmax; p++)
				{
					if (IsOnCurve(c, p))
					{
						firstOn = p;
						break;
					}
				}

				// curve

				drawList->PathLineTo(LocalToScreen(GetCoords(c, firstOn)));

				for (int i = 0; i < pmax; i++)
				{
					int icurr = firstOn + i + 1;
					int inext = firstOn + i + 2;
					ct::ivec2 cur = GetCoords(c, icurr);

					if (IsOnCurve(c, icurr))
					{
						drawList->PathLineTo(LocalToScreen(cur));
					}
					else
					{
						ct::ivec2 nex = GetCoords(c, inext);
						if (!IsOnCurve(c, inext))
						{
							nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
							nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
						}
						drawList->PathBezierQuadraticCurveTo(LocalToScreen(cur), LocalToScreen(nex), vQuadBezierCountSegments);
					}
				}

				drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Text), true);

#ifdef _DEBUG
				//DebugPane::Instance()->DrawGlyphCurrentPoint(newScale, posOrigin, drawList);
#endif

				if (vGlyphDrawingFlags & GLYPH_DRAWING_GLYPH_CONTROL_LINES) // control lines
				{
					drawList->PathLineTo(LocalToScreen(GetCoords(c, firstOn)));

					for (int i = 0; i < pmax; i++)
					{
						int icurr = firstOn + i + 1;
						int inext = firstOn + i + 2;
						ct::ivec2 cur = GetCoords(c, icurr);
						if (IsOnCurve(c, icurr))
						{
							drawList->PathLineTo(LocalToScreen(cur));
						}
						else
						{
							ct::ivec2 nex = GetCoords(c, inext);
							if (!IsOnCurve(c, inext))
							{
								nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
								nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
							}
							drawList->PathLineTo(LocalToScreen(cur));
							drawList->PathLineTo(LocalToScreen(nex));
						}
					}

					drawList->PathStroke(ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), true);
				}
			}

			// origin point
			if (vGlyphDrawingFlags & GLYPH_DRAWING_FONT_ORIGIN_XY)
			{
				const ImU32 PointCol = ImGui::GetColorU32(ImGuiCol_Text, 1.0f);
				min = LocalToScreen(ImVec2(0.0f, 0.0f));
				if (vGlyphDrawingFlags & GLYPH_DRAWING_LEGENDS)
				{
					const char* txt = "Origin";
					ImVec2 txtSize = ImGui::CalcTextSize(txt);
					drawList->AddText(ImVec2(min.x - txtSize.x - tlh * 0.5f, min.y - tlh), textCol, txt);
				}
				drawList->AddCircleFilled(min, 5.0f, PointCol);
			}

#undef LocalToScreen
#undef ScreenToLocal
		}
		ImGui::PopClipRect();
	}
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

std::shared_ptr<GlyphInfos> GlyphInfos::Create(
	std::weak_ptr<FontInfos> vFontInfos,
	ImFontGlyph vGlyph, std::string vOldName,
	std::string vNewName, uint32_t vNewCodePoint,
	ImVec2 vTranslation, ImVec2 vScale)
{
	assert(vFontInfos.use_count() != 0);
	return std::make_shared<GlyphInfos>(vFontInfos, vGlyph, vOldName, vNewName, vNewCodePoint, vTranslation, vScale);
}

GlyphInfos::GlyphInfos()
{
	glyph.AdvanceX = 0;
	glyph.Codepoint = 0;
	glyph.U0 = 0;
	glyph.U1 = 0;
	glyph.V0 = 0;
	glyph.V1 = 0;
	glyph.Visible = false;
	glyph.X0 = 0;
	glyph.X1 = 0;
	glyph.Y0 = 0;
	glyph.Y1 = 0;
}

GlyphInfos::GlyphInfos(
	std::weak_ptr<FontInfos> vFontInfos,
	ImFontGlyph vGlyph, std::string vOldName, 
	std::string vNewName, uint32_t vNewCodePoint, 
	ImVec2 vTranslation, ImVec2 vScale)
{
	fontInfos = vFontInfos;
	glyph = vGlyph;
	oldHeaderName = vOldName;
	newHeaderName = vNewName;
	newCodePoint = vNewCodePoint;
	if (newCodePoint == 0)
		newCodePoint = glyph.Codepoint;
	m_Translation = vTranslation;
	m_Scale = vScale;

	if (!fontInfos.expired())
	{
		auto fontInfosPtr = fontInfos.lock();
		if (fontInfosPtr.use_count())
		{
			m_Colored = fontInfosPtr->m_ColoredGlyphs[glyph.Codepoint];
		}
	}
}

GlyphInfos::~GlyphInfos() = default;

std::weak_ptr<FontInfos> GlyphInfos::GetFontInfos()
{
	return fontInfos;
}

void GlyphInfos::SetFontInfos(std::weak_ptr<FontInfos> vFontInfos)
{
	fontInfos = vFontInfos;

	if (!fontInfos.expired())
	{
		auto fontInfosPtr = fontInfos.lock();
		if (fontInfosPtr.use_count())
		{
			m_Colored = fontInfosPtr->m_ColoredGlyphs[glyph.Codepoint];
		}
	}
}

int GlyphInfos::DrawGlyphButton(
	int &vWidgetPushId, // by adress because we want modify it
	ProjectFile* vProjectFile, ImFont* vFont,
	bool* vSelected, ImVec2 vGlyphSize, const ImFontGlyph *vGlyph, 
	bool vColored,
	ImVec2 vTranslation, ImVec2 vScale,
	int frame_padding, float vRectThickNess, ImVec4 vRectColor)
{
	int res = 0;

	if (vFont && vGlyph)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImGui::PushID(++vWidgetPushId);
		ImGui::PushID((void*)(intptr_t)vFont->ContainerAtlas->TexID);
		const ImGuiID id = window->GetID("#image");
		ImGui::PopID();
		ImGui::PopID();

		const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
		ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vGlyphSize + padding * 2);
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
		if (pressed)
		{
			if (vSelected)
			{
				*vSelected = !*vSelected;
			}

			if (g.ActiveIdMouseButton == 0) // left click
				res = 1;
			if (g.ActiveIdMouseButton == 1) // right click
				res = 2;
		}

		// Render
		const ImU32 col = ImGui::GetColorU32(((held && hovered) || (vSelected && *vSelected)) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderNavHighlight(bb, id);

		const float rounding = ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding);
#ifdef USE_SHADOW
		if (!ImGuiThemeHelper::m_UseShadow)
		{
#endif
			// normal
			ImGui::RenderFrame(bb.Min, bb.Max, col, true, rounding);
#ifdef USE_SHADOW
		}
		else
		{
			if (ImGuiThemeHelper::m_UseTextureForShadow)
			{
				ImTextureID texId = (ImTextureID)AssetManager::Instance()->m_Textures["btn"].glTex;
				window->DrawList->AddImage(texId, bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
			}
			else
			{
				// inner shadow
				ImVec4 cb = ImColor(col).Value; // color base : cb
				float sha = ImGuiThemeHelper::Instance()->m_ShadowStrength;
				ImVec4 cbd = ImVec4(cb.x * sha, cb.y * sha, cb.z * sha, cb.w * 0.9f); // color base darker : cbd
				ImGui::RenderInnerShadowFrame(bb.Min, bb.Max, col, ImGui::GetColorU32(cbd), ImGui::GetColorU32(ImGuiCol_WindowBg), true, rounding);
			}
		}
		ImGui::AddInvertedRectFilled(window->DrawList, bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_WindowBg), rounding, ImDrawCornerFlags_All);
#endif

		// double codepoint / name rect display
		if (vRectThickNess > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vRectColor), 0.0, 15, vRectThickNess);
		}

		ImGui::PushClipRect(bb.Min, bb.Max, true);
		
		bb.Min += style.FramePadding;
		bb.Max -= style.FramePadding;

		ImVec2 pScale = vGlyphSize / vFont->FontSize;
		float adv = vGlyph->AdvanceX * pScale.y;
		float offsetX = bb.GetSize().x * 0.5f - adv * 0.5f; // horizontal centering of the glyph
		ImVec2 trans = vTranslation * pScale;
		ImVec2 scale = vScale;

		if (!vProjectFile->m_ZoomGlyphs)
		{
			if (vProjectFile->m_ShowBaseLine)// draw base line
			{
				float asc = vFont->Ascent * pScale.y;
				window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + asc), ImVec2(bb.Max.x, bb.Min.y + asc), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 2.0f); // base line
			}

			if (vProjectFile->m_ShowOriginX) // draw origin x
			{
				window->DrawList->AddLine(ImVec2(bb.Min.x + offsetX, bb.Min.y), ImVec2(bb.Min.x + offsetX, bb.Max.y), ImGui::GetColorU32(ImGuiCol_PlotLinesHovered), 2.0f); // base line
			}

			if (vProjectFile->m_ShowAdvanceX) // draw advance X
			{
				window->DrawList->AddLine(ImVec2(bb.Min.x + adv + offsetX, bb.Min.y), ImVec2(bb.Min.x + adv + offsetX, bb.Max.y), ImGui::GetColorU32(ImGuiCol_PlotLines), 2.0f); // base line
			}
		}

		ImU32 textCol = ImGui::GetColorU32(ImGuiCol_Text);
		if (vColored)
		{
			textCol = ImGui::GetColorU32(ImVec4(1,1,1,1));
		}
		RenderGlyph(vFont, window->DrawList,
			vGlyphSize.y, 
			bb.Min, bb.Max, ImVec2(offsetX,0),
			textCol,
			(ImWchar)vGlyph->Codepoint,
			trans, scale,
			vProjectFile->m_ZoomGlyphs);
		
		ImGui::PopClipRect();
	}

	return res;
}

void GlyphInfos::RenderGlyph(ImFont* vFont, ImDrawList* vDrawList, float vGlyphHeight, ImVec2 vMin, ImVec2 vMax, ImVec2 vOffset, ImU32 vCol, ImWchar vGlyphCodePoint, ImVec2 vTranslation, ImVec2 vScale, bool vZoomed)
{
	if (vFont && vFont->ContainerAtlas && vDrawList && vGlyphHeight > 0.0f) // (vGlyphSize > 0.0 for avoid div by zero)
	{
		const ImFontGlyph* glyph = vFont->FindGlyph(vGlyphCodePoint);
		if (!glyph || !glyph->Visible)
			return;

		float scale = (vGlyphHeight >= 0.0f) ? (vGlyphHeight / vFont->FontSize) : 1.0f;
		//vPos.x = IM_FLOOR(vPos.x);
		//vPos.y = IM_FLOOR(vPos.y);

		ImVec2 pMin(0, 0), pMax(0, 0);

		if (vZoomed)
		{
			ImVec2 gSize = ImVec2(glyph->X1 - glyph->X0, glyph->Y1 - glyph->Y0);
			if (IS_FLOAT_EQUAL(gSize.y, 0.0f))
				return;
			float ratioX = gSize.x / gSize.y;
			float newX = vGlyphHeight * ratioX;
			gSize = ImVec2(vGlyphHeight, vGlyphHeight / ratioX) * 0.5f;
			if (newX < vGlyphHeight)
				gSize = ImVec2(newX, vGlyphHeight) * 0.5f;
			ImVec2 center = ImRect(vMin, vMax).GetCenter();

			pMin = center - gSize;
			pMax = center + gSize;
		}
		else
		{
			pMin = ImVec2(vMin.x + vOffset.x + glyph->X0 * scale * vScale.x + vTranslation.x, vMin.y + vOffset.y + glyph->Y0 * scale * vScale.y - vTranslation.y);
			pMax = ImVec2(vMin.x + vOffset.x + glyph->X1 * scale * vScale.x + vTranslation.x, vMin.y + vOffset.y + glyph->Y1 * scale * vScale.y - vTranslation.y);
		}

		ImVec2 uv0 = ImVec2(glyph->U0, glyph->V0);
		ImVec2 uv1 = ImVec2(glyph->U1, glyph->V1);

		vDrawList->PushTextureID(vFont->ContainerAtlas->TexID);
		vDrawList->PrimReserve(6, 4);
		vDrawList->PrimRectUV(pMin, pMax, uv0, uv1, vCol);
		vDrawList->PopTextureID();
	}
}