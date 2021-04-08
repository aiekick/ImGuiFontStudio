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
#include <Helper/Profiler.h>

#include <utility>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <Gui/ImWidgets.h>
#include <Helper/ThemeHelper.h>
#include <Helper/AssetManager.h>
#include <Panes/DebugPane.h>
#include <Helper/MeshDecomposer.h>

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
	ZoneScoped;

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
	ZoneScoped;

	isValid = false;
}

void SimpleGlyph_Solo::LoadGlyph(BaseGlyph *vGlyph, ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (vGlyph)
	{
		Clear();
		m_Glyph = *vGlyph;
		ComputeWholeGlyphMesh(vProjectFile->m_GlyphPreview_QuadBezierCountSegments, vProjectFile->m_PartitionAlgo);

		isValid = !m_Glyph.contours.empty();
	}
}

int SimpleGlyph_Solo::GetCountContours(BaseGlyph* vGlyph)
{
	ZoneScoped;

	return (int)vGlyph->contours.size();
}

int SimpleGlyph_Solo::GetCountPoints(BaseGlyph* vGlyph)
{
	ZoneScoped;

	int pmax = 0;

	for (auto c : vGlyph->contours)
	{
		pmax = ct::maxi(pmax, (int)c.m_Points.size());
	}

	return pmax;
}

// will explore all glyph, main + layers
int SimpleGlyph_Solo::GetMaxContours()
{
	ZoneScoped;

	int maxContours = GetCountContours(&m_Glyph);

	for (auto layer : m_Layers)
	{
		maxContours = ct::maxi(maxContours, GetCountContours(&layer.first));
	}

	return maxContours;
}

// will explore all glyph, main + layers
int SimpleGlyph_Solo::GetMaxPoints()
{
	ZoneScoped;

	int pmax = GetCountPoints(&m_Glyph);

	for (auto layer : m_Layers)
	{
		pmax = ct::maxi(pmax, GetCountPoints(&layer.first));
	}

	return pmax;
}

ct::ivec2 SimpleGlyph_Solo::GetCoords(BaseGlyph* vGlyph, int32_t vContour, int32_t vPoint)
{
	ZoneScoped;

	ct::ivec2 pt;

	if (vGlyph)
	{
		if (vContour < vGlyph->contours.size())
		{
			int count = (int)vGlyph->contours[vContour].m_Points.size();
			auto p = vGlyph->contours[vContour].m_Points[vPoint % count];

			// apply transformation
			pt.x = (int)(p.x * m_Scale.x);
			pt.y = (int)(p.y * m_Scale.y);
			pt += ct::ivec2(m_Translation);
			return pt;
		}
	}

	return pt;
}

bool SimpleGlyph_Solo::IsOnCurve(BaseGlyph* vGlyph, int32_t vContour, int32_t vPoint)
{
	ZoneScoped;

	if (vGlyph)
	{
		if (vContour < vGlyph->contours.size())
		{
			int count = (int)vGlyph->contours[vContour].m_OnCurve.size();
			return vGlyph->contours[vContour].m_OnCurve[vPoint % count];
		}
	}

	return false;
}

ct::ivec2 SimpleGlyph_Solo::Scale(BaseGlyph* vGlyph, ct::ivec2 p, double scale) const
{
	ZoneScoped;

	if (vGlyph)
	{
		return ct::ivec2(
			(int)round(scale * ((double)p.x - (double)vGlyph->bbox.x)),
			(int)round(scale * ((double)vGlyph->bbox.w - (double)p.y)));
	}
	return ct::ivec2(0, 0);
}

/*ct::ivec2 SimpleGlyph_Solo::GetCoords(int32_t vContour, int32_t vPoint, double scale)
{
	ZoneScoped;

	return Scale(GetCoords(vContour, vPoint), scale);
}*/

void SimpleGlyph_Solo::ClearTransform()
{
	ZoneScoped;

	ClearTranslation();
	ClearScale();
}

void SimpleGlyph_Solo::ClearTranslation()
{
	ZoneScoped;

	m_Translation = 0.0f;
}

void SimpleGlyph_Solo::ClearScale()
{
	ZoneScoped;

	m_Scale = 1.0f;
}

static inline ct::fvec2 SimpleGlyph_Solo_ImBezierQuadraticCalc(const ct::fvec2& p1, const ct::fvec2& p2, const ct::fvec2& p3, float t)
{
	float u = 1.0f - t;
	float w1 = u * u;
	float w2 = 2 * u * t;
	float w3 = t * t;
	return ct::fvec2(w1 * p1.x + w2 * p2.x + w3 * p3.x, w1 * p1.y + w2 * p2.y + w3 * p3.y);
}

static inline void SimpleGlyph_Solo_PathBezierQuadraticCurveToCasteljau(std::list<ct::fvec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float tess_tol, int level)
{
	float dx = x3 - x1, dy = y3 - y1;
	float det = (x2 - x3) * dy - (y2 - y3) * dx;
	if (det * det * 4.0f < tess_tol * (dx * dx + dy * dy))
	{
		const auto pt = ct::fvec2(x3, y3);
		path->push_back(pt);
	}
	else if (level < 10)
	{
		float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
		float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
		float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
		SimpleGlyph_Solo_PathBezierQuadraticCurveToCasteljau(path, x1, y1, x12, y12, x123, y123, tess_tol, level + 1);
		SimpleGlyph_Solo_PathBezierQuadraticCurveToCasteljau(path, x123, y123, x23, y23, x3, y3, tess_tol, level + 1);
	}
}

static inline void SimpleGlyph_Solo_PathBezierQuadraticCurveTo(std::list<ct::fvec2>* path, const ct::fvec2& p2, const ct::fvec2& p3, int num_segments)
{
	ct::fvec2 p1 = path->back();
	if (num_segments == 0)
	{
		const float CurveTessellationTol = 1.25f;
		SimpleGlyph_Solo_PathBezierQuadraticCurveToCasteljau(path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, CurveTessellationTol, 0);// Auto-tessellated
	}
	else
	{
		float t_step = 1.0f / (float)num_segments;
		for (int i_step = 1; i_step <= num_segments; i_step++)
			path->push_back(SimpleGlyph_Solo_ImBezierQuadraticCalc(p1, p2, p3, t_step * i_step));
	}
}

GlyphContour SimpleGlyph_Solo::ComputeAbsolutePointsFromContour(TTFRRW::Contour vContour, int vQuadBezierCountSegments)
{
	ZoneScoped;
	
	// glyph
	const int pmax = (int)vContour.m_Points.size();

	bool found = false;
	int32_t firstPointOnCurve = 0;
	for (int32_t pointID = 0; pointID < pmax; pointID++)
	{
		if (vContour.IsOnCurve(pointID))
		{
			firstPointOnCurve = pointID;
			found = true;
			break;
		}
	}

	std::list<ct::fvec2> tmp;
	if (found)
	{
		const auto firstPoint = vContour.GetCoords(firstPointOnCurve,
			TTFRRW::fvec2(m_Scale.x, m_Scale.y),
			TTFRRW::fvec2(m_Translation.x, m_Translation.y));
		tmp.push_back(ct::fvec2(firstPoint.x, firstPoint.y));
		for (int32_t pointID = 0; pointID < pmax; pointID++)
		{
			int32_t icurr = firstPointOnCurve + pointID + 1;
			int32_t inext = firstPointOnCurve + pointID + 2;
			const auto curPoint = vContour.GetCoords(icurr,
				TTFRRW::fvec2(m_Scale.x, m_Scale.y),
				TTFRRW::fvec2(m_Translation.x, m_Translation.y));
			if (vContour.IsOnCurve(icurr))
				tmp.push_back(ct::fvec2(curPoint.x, curPoint.y));
			else
			{
				auto nextPoint = vContour.GetCoords(inext,
					TTFRRW::fvec2(m_Scale.x, m_Scale.y),
					TTFRRW::fvec2(m_Translation.x, m_Translation.y));
				if (!vContour.IsOnCurve(inext))
				{
					nextPoint.x = (int)(((double)nextPoint.x + (double)curPoint.x) * 0.5);
					nextPoint.y = (int)(((double)nextPoint.y + (double)curPoint.y) * 0.5);
				}
				SimpleGlyph_Solo_PathBezierQuadraticCurveTo(&tmp,
					ct::fvec2(curPoint.x, curPoint.y), ct::fvec2(nextPoint.x, nextPoint.y),
					vQuadBezierCountSegments);
			}
		}
	}
	else
	{
		const auto fp = vContour.GetCoords(0,
			TTFRRW::fvec2(m_Scale.x, m_Scale.y),
			TTFRRW::fvec2(m_Translation.x, m_Translation.y));
		const auto lp = vContour.GetCoords(1,
			TTFRRW::fvec2(m_Scale.x, m_Scale.y),
			TTFRRW::fvec2(m_Translation.x, m_Translation.y));
		ct::fvec2 cp;
		cp.x = (fp.x + lp.x) * 0.5f;
		cp.y = (fp.y + lp.y) * 0.5f;
		tmp.push_back(cp);
		for (int32_t pointID = 0; pointID < pmax; pointID++)
		{
			const auto curPoint = vContour.GetCoords(pointID + 1,
				TTFRRW::fvec2(m_Scale.x, m_Scale.y),
				TTFRRW::fvec2(m_Translation.x, m_Translation.y));
			auto nextPoint = vContour.GetCoords(pointID + 2,
				TTFRRW::fvec2(m_Scale.x, m_Scale.y),
				TTFRRW::fvec2(m_Translation.x, m_Translation.y));
			nextPoint.x = (nextPoint.x + curPoint.x) * 0.5f;
			nextPoint.y = (nextPoint.y + curPoint.y) * 0.5f;
			SimpleGlyph_Solo_PathBezierQuadraticCurveTo(&tmp,
				ct::fvec2(curPoint.x, curPoint.y), ct::fvec2(nextPoint.x, nextPoint.y),
				vQuadBezierCountSegments);
		}
	}
	
	GlyphContour result;

	//printf("-----------\n");
	tmp.unique();
	if (tmp.back() == tmp.front())
		tmp.erase(tmp.begin());
	for (auto p : tmp)
	{
		//printf("p %.2f %.2f\n", p.x, p.y);
		result.push_back(p);
	}
	//printf("-----------\n");

	return result;
}

GlyphMesh SimpleGlyph_Solo::ComputeGlyphMesh(BaseGlyph vGlyph, int vQuadBezierCountSegments, PartitionAlgoFlags vPartitionAlgoFlags)
{
	GlyphMesh res;

	if (!vGlyph.contours.empty())
	{
		for (auto c : vGlyph.contours)
		{
			auto points = ComputeAbsolutePointsFromContour(c, vQuadBezierCountSegments);
			res.push_back(points);
		}
		res = MeshDecomposer::Decompose(vGlyph.glyphIndex, res, vPartitionAlgoFlags);
	}

	return res;
}

void SimpleGlyph_Solo::ComputeWholeGlyphMesh(int vQuadBezierCountSegments, PartitionAlgoFlags vPartitionAlgoFlags)
{
	m_Glyph.mesh = ComputeGlyphMesh(
		m_Glyph,
		vQuadBezierCountSegments,
		vPartitionAlgoFlags);

	if (!m_Glyph.fontInfos.expired())
	{
		auto fontPtr = m_Glyph.fontInfos.lock();
		if (fontPtr)
		{
			m_Layers.clear();
			for (auto layer : m_Glyph.layers)
			{
				auto glyph = fontPtr->GetGlyphByGlyphIndex(layer);
				if (glyph)
				{
					glyph->mesh = ComputeGlyphMesh(
						*glyph,
						vQuadBezierCountSegments,
						vPartitionAlgoFlags);
					m_Layers.push_back(std::pair<BaseGlyph, bool>(*glyph, true));
				}
			}
		}
	}
}

ImVec2 SimpleGlyph_Solo::getScreenToLocal(ImVec2 vScreenPos, ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin)
{
	ZoneScoped;

	ImVec2 localPos;
	localPos.x = (vScreenPos.x - vZoneStart.x - vWorldBBoxOrigin.x) / vWorldScale + vLocalBBoxOrigin.x;
	localPos.y = (vWorlBBoxSize.y - (vScreenPos.y - vZoneStart.y - vWorldBBoxOrigin.y)) / vWorldScale + vLocalBBoxOrigin.y;
	return localPos;
}

ImVec2 SimpleGlyph_Solo::getLocalToScreen(ImVec2 vLocalPos, ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin)
{
	ZoneScoped;

	ImVec2 screenPos;
	screenPos.x = (vLocalPos.x - vLocalBBoxOrigin.x) * vWorldScale + vWorldBBoxOrigin.x + vZoneStart.x;
	screenPos.y = vWorlBBoxSize.y - (vLocalPos.y - vLocalBBoxOrigin.y) * vWorldScale + vWorldBBoxOrigin.y + vZoneStart.y;
	return screenPos;
}

ct::ivec2 SimpleGlyph_Solo::TransformCoords(ct::ivec2 vPoint)
{
	ZoneScoped;

	vPoint.x = (int)(vPoint.x * m_Scale.x);
	vPoint.y = (int)(vPoint.y * m_Scale.y);
	vPoint += ct::ivec2(m_Translation);

	return vPoint;
}

void SimpleGlyph_Solo::DrawCurves(
	BaseGlyph *vGlyph,
	ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, 
	float vWorldScale, ImVec2 vLocalBBoxOrigin, ImDrawList* vImDrawList, 
	int vMaxPoint, int vMaxContour, int vQuadBezierCountSegments, 
	GlyphDrawingFlags vGlyphDrawingFlags)
{
	if (!vGlyph) return;

#define LocalToScreen(a) getLocalToScreen(ImVec2((float)a.x, (float)a.y), vZoneStart, vWorldBBoxOrigin, vWorlBBoxSize, vWorldScale, vLocalBBoxOrigin)
#define ScreenToLocal(a) getScreenToLocal(ImVec2((float)a.x, (float)a.y), vZoneStart, vWorldBBoxOrigin, vWorlBBoxSize, vWorldScale, vLocalBBoxOrigin)

	// glyph
	int32_t contourId = 0;
	for (auto contour : vGlyph->contours)
	{
		if (contourId >= vMaxContour) break;

		int pmax = (int)contour.m_Points.size();

		bool found = false;
		int32_t firstPointOnCurve = 0;
		for (int32_t pointID = 0; pointID < pmax; pointID++)
		{
			if (IsOnCurve(vGlyph, contourId, pointID))
			{
				firstPointOnCurve = pointID;
				found = true;
				break;
			}
		}

		if (found)
		{
			// ca ca marche bien
			vImDrawList->PathLineTo(LocalToScreen(GetCoords(vGlyph, contourId, firstPointOnCurve)));
			for (int32_t pointID = 0; pointID < pmax; pointID++)
			{
				if (pointID >= vMaxPoint) break;
				
				int32_t icurr = firstPointOnCurve + pointID + 1;
				int32_t inext = firstPointOnCurve + pointID + 2;
				ct::ivec2 cur = GetCoords(vGlyph, contourId, icurr);
				if (IsOnCurve(vGlyph, contourId, icurr))
					vImDrawList->PathLineTo(LocalToScreen(cur));
				else
				{
					ct::ivec2 nex = GetCoords(vGlyph, contourId, inext);
					if (!IsOnCurve(vGlyph, contourId, inext))
					{
						nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
						nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
					}
					vImDrawList->PathBezierQuadraticCurveTo(LocalToScreen(cur), LocalToScreen(nex), vQuadBezierCountSegments);
				}
			}
			vImDrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Text), false);
		}
		else // aucun sur courbe
		{
			auto fp = GetCoords(vGlyph, contourId, 0);
			auto lp = GetCoords(vGlyph, contourId, 1);
			ImVec2 cp;
			cp.x = (fp.x + lp.x) * 0.5f;
			cp.y = (fp.y + lp.y) * 0.5f;
			vImDrawList->PathLineTo(LocalToScreen(cp));

			for (int32_t pointID = 0; pointID < pmax; pointID++)
			{
				if (pointID >= vMaxPoint) break;
				
				ct::ivec2 cur = GetCoords(vGlyph, contourId, pointID + 1);
				ct::ivec2 nex = GetCoords(vGlyph, contourId, pointID + 2);
				nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
				nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
				vImDrawList->PathBezierQuadraticCurveTo(LocalToScreen(cur), LocalToScreen(nex), vQuadBezierCountSegments);
			}

			vImDrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Text), false);
		}

#ifdef _DEBUG
		DebugPane::Instance()->DrawGlyphCurrentPoint(vZoneStart, vWorldBBoxOrigin, vWorlBBoxSize, vWorldScale, vLocalBBoxOrigin, vImDrawList);
#endif

		if (vGlyphDrawingFlags & GLYPH_DRAWING_GLYPH_CONTROL_LINES) // control lines
		{
			vImDrawList->PathLineTo(LocalToScreen(GetCoords(vGlyph, contourId, 0)));
			
			for (int32_t pointID = 0; pointID < pmax; pointID++)
			{
				if (pointID >= vMaxPoint) break;

				vImDrawList->PathLineTo(LocalToScreen(GetCoords(vGlyph, contourId, pointID + 1)));
				vImDrawList->PathLineTo(LocalToScreen(GetCoords(vGlyph, contourId, pointID + 2)));
			}

			vImDrawList->PathStroke(ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), false);
		}

		contourId++;
	}

#undef LocalToScreen
#undef ScreenToLocal
}

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer
// we will display the glyph metrics like here : https://www.libsdl.org/projects/SDL_ttf/docs/metrics.png
void SimpleGlyph_Solo::DrawGlyph(
	float vGlobalScale, 
	std::shared_ptr<FontInfos> vFontInfos,
	std::shared_ptr<GlyphInfos> vGlyphInfos,
	int vMaxPoint, int vMaxContour, int vQuadBezierCountSegments,
	GlyphDrawingFlags vGlyphDrawingFlags)
{
	ZoneScoped;

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

			const float ascent = (float)vFontInfos->m_Ascent;
			const float descent = (float)vFontInfos->m_Descent;
			const float advanceX = vGlyphInfos->glyph.AdvanceX * vFontInfos->m_Point;

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
				min = LocalToScreen(ImVec2((float)m_Glyph.bbox.x + m_Translation.x, (float)m_Glyph.bbox.y + m_Translation.y));
				max = LocalToScreen(ImVec2((float)m_Glyph.bbox.z + m_Translation.x, (float)m_Glyph.bbox.w + m_Translation.y));
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

			if (!m_Layers.empty())
			{
				for (auto glyph : m_Layers)
				{
					if (glyph.second)
					{
						if (!glyph.first.mesh.empty())
						{
							for (auto c : glyph.first.mesh)
							{
								for (auto p : c)
								{
									drawList->PathLineTo(getLocalToScreen(ImVec2((float)p.x, (float)p.y), contentStart, fbboxOrign, fontBBoxSize, newScale, ImVec2((float)frc.x, (float)frc.y)));
								}
								drawList->PathFillConvex(ImGui::GetColorU32(glyph.first.color[m_Glyph.glyphIndex]));
							}
						}
						else
						{
							DrawCurves(&glyph.first,
								contentStart, fbboxOrign, fontBBoxSize,
								newScale, ImVec2((float)frc.x, (float)frc.y), drawList,
								vMaxPoint, vMaxContour, vQuadBezierCountSegments, vGlyphDrawingFlags);
						}
					}
				}
			}
			else
			{
				if (!m_Glyph.mesh.empty())
				{
					for (auto c : m_Glyph.mesh)
					{
						for (auto p : c)
						{
							drawList->PathLineTo(getLocalToScreen(ImVec2((float)p.x, (float)p.y), contentStart, fbboxOrign, fontBBoxSize, newScale, ImVec2((float)frc.x, (float)frc.y)));
						}
						drawList->PathFillConvex(ImGui::GetColorU32(m_Glyph.color[m_Glyph.glyphIndex]));
					}
				}
				else
				{
					DrawCurves(&m_Glyph,
						contentStart, fbboxOrign, fontBBoxSize,
						newScale, ImVec2((float)frc.x, (float)frc.y), drawList,
						vMaxPoint, vMaxContour, vQuadBezierCountSegments, vGlyphDrawingFlags);
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
	BaseGlyph vGlyph, std::string vOldName,
	std::string vNewName, uint32_t vNewGlyphIndex,
	ImVec2 vTranslation, ImVec2 vScale)
{
	ZoneScoped;

	assert(vFontInfos.use_count() != 0);
	return std::make_shared<GlyphInfos>(vFontInfos, vGlyph, vOldName, vNewName, vNewGlyphIndex, vTranslation, vScale);
}

GlyphInfos::GlyphInfos()
{
	ZoneScoped;

	glyph = BaseGlyph();
}

GlyphInfos::GlyphInfos(
	std::weak_ptr<FontInfos> vFontInfos,
	BaseGlyph vGlyph, std::string vOldName, 
	std::string vNewName, uint32_t vNewGlyphIndex, 
	ImVec2 vTranslation, ImVec2 vScale)
{
	ZoneScoped;

	fontInfos = vFontInfos;
	glyph = vGlyph;
	glyph.name = vOldName;
	newHeaderName = vNewName;
	newCodePoint = vNewGlyphIndex;
	if (newCodePoint == 0)
		newCodePoint = glyph.glyphIndex;
	m_Translation = vTranslation;
	m_Scale = vScale;
}

GlyphInfos::~GlyphInfos() = default;

std::weak_ptr<FontInfos> GlyphInfos::GetFontInfos()
{
	ZoneScoped;

	return fontInfos;
}

void GlyphInfos::SetFontInfos(std::weak_ptr<FontInfos> vFontInfos)
{
	ZoneScoped;

	fontInfos = vFontInfos;
}

int GlyphInfos::DrawGlyphButton(
	int &vWidgetPushId, // by adress because we want modify it
	ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
	bool* vSelected, ImVec2 vGlyphSize, BaseGlyph *vGlyph, int vParentGlyphIndex,
	ImVec2 vTranslation, ImVec2 vScale,
	int frame_padding, float vRectThickNess, ImVec4 vRectColor)
{
	ZoneScoped;

	int res = 0;

	if (vFontInfos && vGlyph)
	{
		const auto imFont = vFontInfos->GetImFontPtr();
		if (imFont && imFont->ContainerAtlas)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (window->SkipItems)
				return false;

			ImGuiContext& g = *GImGui;
			const ImGuiStyle& style = g.Style;

			ImGui::PushID(++vWidgetPushId);
			ImGui::PushID((void*)(intptr_t)imFont->ContainerAtlas->TexID);
			const ImGuiID id = window->GetID("#image");
			ImGui::PopID();
			ImGui::PopID();

			const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
			ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vGlyphSize + padding * 2);
			ImGui::ItemSize(bb);
			if (!ImGui::ItemAdd(bb, id))
				return false;

			bool hovered, held;
			const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
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
			if (!ThemeHelper::m_UseShadow)
			{
#endif
				// normal
				ImGui::RenderFrame(bb.Min, bb.Max, col, true, rounding);
#ifdef USE_SHADOW
			}
			else
			{
				if (ThemeHelper::m_UseTextureForShadow)
				{
					ImTextureID texId = (ImTextureID)AssetManager::Instance()->m_Textures["btn"].glTex;
					window->DrawList->AddImage(texId, bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
				}
				else
				{
					// inner shadow
					ImVec4 cb = ImColor(col).Value; // color base : cb
					float sha = ThemeHelper::Instance()->m_ShadowStrength;
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

			const ImVec2 glyphSize = vGlyphSize / vFontInfos->m_FontDiffScale;
			const ImVec2 pScale = glyphSize / imFont->FontSize;
			const float adv = vGlyph->AdvanceX * pScale.y;
			const float offsetX = bb.GetSize().x * 0.5f - adv * 0.5f; // horizontal centering of the glyph
			const float offsetY = bb.GetSize().y * 0.5f - glyphSize.y * vFontInfos->m_AscentDiffScale * 0.5f; // vertical centering of the glyph
			const ImVec2 trans = vTranslation * pScale;
			const ImVec2 scale = vScale;

			if (!vProjectFile->m_ZoomGlyphs)
			{
				if (vProjectFile->m_ShowBaseLine)// draw base line
				{
					const float base = imFont->Ascent * pScale.y;
					window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + offsetY + base), ImVec2(bb.Max.x, bb.Min.y + offsetY + base), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 2.0f); // base line
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

			/*ImU32 textCol = ImGui::GetColorU32(ImGuiCol_Text);
			if ((vGlyph->category & GLYPH_CATEGORY_FLAG_COLORED) == GLYPH_CATEGORY_FLAG_COLORED)
			{
				textCol = ImGui::GetColorU32(ImVec4(1,1,1,1));
				// no more need since modification is imgui done
				// todo : maybe when i need to simulate a color when the glyph creation will be possible
			}*/

			ImU32 symbolCol = ImGui::GetColorU32(ImGuiCol_Text);
			if (vGlyph->category & GLYPH_CATEGORY_FLAG_LAYER)
			{
				if (!vGlyph->color.empty())
				{
					ImVec4 symbolColV4 = vGlyph->color[vGlyph->glyphIndex]; //vGlyph->color.begin()->second;
					if (vParentGlyphIndex >= 0)
						symbolColV4 = vGlyph->color[vParentGlyphIndex];
					symbolColV4.w = 1.0f;
					symbolCol = ImGui::GetColorU32(symbolColV4);
				}
			}
			
			RenderGlyph(imFont, window->DrawList,
				glyphSize.y,
				bb.Min, bb.Max, ImVec2(offsetX, offsetY),
				symbolCol,
				(ImWchar)vGlyph->glyphIndex,
				trans, scale,
				vProjectFile->m_ZoomGlyphs);

			ImGui::PopClipRect();
		}
	}

	return res;
}

void GlyphInfos::RenderGlyph(ImFont* vFont, ImDrawList* vDrawList, float vGlyphHeight, ImVec2 vMin, ImVec2 vMax, ImVec2 vOffset, ImU32 vCol, ImWchar vGlyphGlyphIndex, ImVec2 vTranslation, ImVec2 vScale, bool vZoomed)
{
	ZoneScoped;

	if (vFont && vFont->ContainerAtlas && vDrawList && vGlyphHeight > 0.0f) // (vGlyphSize > 0.0 for avoid div by zero)
	{
		const ImFontGlyph* glyph = vFont->FindGlyph(vGlyphGlyphIndex);
		if (!glyph || !glyph->Visible)
			return;

		const float scale = (vGlyphHeight >= 0.0f) ? (vGlyphHeight / vFont->FontSize) : 1.0f;
		//vPos.x = IM_FLOOR(vPos.x);
		//vPos.y = IM_FLOOR(vPos.y);

		ImVec2 pMin(0, 0), pMax(0, 0);

		if (vZoomed)
		{
			ImVec2 gSize = ImVec2(glyph->X1 - glyph->X0, glyph->Y1 - glyph->Y0);
			if (IS_FLOAT_EQUAL(gSize.y, 0.0f))
				return;
			const float ratioX = gSize.x / gSize.y;
			const float newX = vGlyphHeight * ratioX;
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

		const ImVec2 uv0 = ImVec2(glyph->U0, glyph->V0);
		const ImVec2 uv1 = ImVec2(glyph->U1, glyph->V1);

		ImU32 glyph_col = vCol;
		if (glyph->Colored)
			glyph_col |= ~IM_COL32_A_MASK;

		vDrawList->PushTextureID(vFont->ContainerAtlas->TexID);
		vDrawList->PrimReserve(6, 4);
		vDrawList->PrimRectUV(pMin, pMax, uv0, uv1, glyph_col);
		vDrawList->PopTextureID();
	}
}