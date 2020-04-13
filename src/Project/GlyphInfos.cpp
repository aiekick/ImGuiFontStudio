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
			onCurve.push_back(std::vector<bool>());
			coords.push_back(std::vector<ct::ivec2>());
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

		// countbytes, value
		auto x_OrginalCoordDatas = vGlyph->xOrginalCoordDatas();
		originalCoords.resize(x_OrginalCoordDatas.size());
		int idx = 0;
		for (auto &it : x_OrginalCoordDatas)
		{
			originalCoords[idx++].x = (double)it.second;
		}
		auto y_OrginalCoordDatas = vGlyph->yOrginalCoordDatas();
		originalCoords.resize(y_OrginalCoordDatas.size());
		idx = 0;
		for (auto &it : y_OrginalCoordDatas)
		{
			originalCoords[idx++].y = (double)it.second;
		}
	}
}

int SimpleGlyph_Solo::GetCountContours()
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

ct::ivec2 SimpleGlyph_Solo::Scale(ct::ivec2 p, double scale)
{
	return ct::ivec2(
		(int)round(scale * ((double)p.x - (double)rc.x)),
		(int)round(scale * ((double)rc.w - (double)p.y)));
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
	std::string vNewName, ImWchar vNewCodePoint)
{
	glyph = vGlyph;
	oldHeaderName = vOldName;
	newHeaderName = vNewName;
	newCodePoint = vNewCodePoint;
	if (newCodePoint == 0)
		newCodePoint = glyph.Codepoint;
}

GlyphInfos::~GlyphInfos()
{

}