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
				float cell_size_x = cell_size_y + ImGui::GetStyle().FramePadding.x + GLYPH_EDIT_CONTROl_WIDTH;
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
 //////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////

void SimpleGlyph_Solo::clear()
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
		clear();
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
	p += m_Translation;
	p.x = (int)(p.x * m_Scale.x);
	p.y = (int)(p.y * m_Scale.y);
	
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

ct::ivec2 SimpleGlyph_Solo::GetCoords(int32_t vContour, int32_t vPoint, double scale)
{
	return Scale(GetCoords(vContour, vPoint), scale);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

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
	ImFontGlyph vGlyph, std::string vOldName, 
	std::string vNewName, uint32_t vNewCodePoint)
{
	glyph = vGlyph;
	oldHeaderName = vOldName;
	newHeaderName = vNewName;
	newCodePoint = vNewCodePoint;
	if (newCodePoint == 0)
		newCodePoint = glyph.Codepoint;
}

GlyphInfos::~GlyphInfos() = default;

std::shared_ptr<FontInfos> GlyphInfos::GetFontInfos()
{
	if (!fontInfos.expired())
	{
		return fontInfos.lock();
	}

	return 0;
}

void GlyphInfos::SetFontInfos(std::shared_ptr<FontInfos> vFontInfos)
{
	fontInfos = vFontInfos;
	if (!vFontInfos)
		fontInfos.reset();
}
