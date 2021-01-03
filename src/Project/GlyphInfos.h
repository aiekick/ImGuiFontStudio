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
#pragma once

#include <ctools/ConfigAbstract.h>
#include <tinyxml2/tinyxml2.h>
#include <imgui/imgui.h>
#include <string>

#include <ctools/cTools.h>
#include <sfntly/table/truetype/glyph_table.h>

#define GLYH_EDIT_CONTROl_WIDTH 180.0f

class ProjectFile;
class GlyphDisplayHelper
{
public:
	static float currentPaneAvailWidth;

public:
	static int CalcGlyphsCountAndSize(			/* return new glyph count x				*/
		ProjectFile* vProjectFile,				/* project file for save some vars		*/
		ImVec2* vCellSize,						/* cell size							*/
		ImVec2* vGlyphSize,						/* glyph size (cell - paddings)			*/
		bool vGlyphEdited = false,				/* in edition							*/
		bool vForceEditMode = false,			/* edition forced						*/
		bool vForceEditModeOneColumn = false);	/* edition in one column				*/
};

class SimpleGlyph_Solo
{
public:
	bool isValid = false;

public:
	std::vector<std::vector<ct::ivec2>> coords;
	std::vector<std::vector<bool>> onCurve;
	ct::ivec4 rc;
	ct::ivec2 m_Translation = 0; // translation in first
	ct::dvec2 m_Scale = 1.0; // scale in second

public:
	void clear();
	void LoadSimpleGlyph(sfntly::GlyphTable::SimpleGlyph *vGlyph);
	int GetCountContours() const;
	ct::ivec2 GetCoords(int32_t vContour, int32_t vPoint);
	bool IsOnCurve(int32_t vContour, int32_t vPoint);
	ct::ivec2 Scale(ct::ivec2 p, double scale) const;
	ct::ivec2 GetCoords(int32_t vContour, int32_t vPoint, double scale);
};

class FontInfos;
class GlyphInfos
{
public:
	ImFontGlyph glyph{};
	int glyphIndex = 0;
	std::string oldHeaderName;
	std::string newHeaderName;
	uint32_t newCodePoint = 0;
	FontInfos *fontAtlas = 0;
	SimpleGlyph_Solo simpleGlyph;
	ct::ivec4 m_FontBoundingBox;
	int m_FontAscent = 0;
	int m_FontDescent = 0;

public: // for interaction only
	bool m_editingName = false;
	bool m_editingCodePoint = false;
	int editCodePoint = 0;
	std::string editHeaderName;

public:
	GlyphInfos();
	GlyphInfos(
		ImFontGlyph vGlyph, std::string vOldName, 
		std::string vNewName, uint32_t vNewCodePoint = 0);
	~GlyphInfos();
};
