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

#include <ConfigAbstract.h>
#include "tinyxml2/tinyxml2.h"
#include <imgui.h>
#include <string>

#include <cTools.h>
#include "sfntly/table/truetype/glyph_table.h"

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
	// countbytes, value
	//std::vector<std::pair<int32_t, int32_t>> x_OrginalCoordDatas;
	//std::vector<std::pair<int32_t, int32_t>> y_OrginalCoordDatas;
	std::vector<ct::dvec2> originalCoords;

public:
	void clear();
	void LoadSimpleGlyph(sfntly::GlyphTable::SimpleGlyph *vGlyph);
	int GetCountContours();
	ct::ivec2 GetCoords(int32_t vContour, int32_t vPoint);
	bool IsOnCurve(int32_t vContour, int32_t vPoint);
	ct::ivec2 Scale(ct::ivec2 p, double scale);
	ct::ivec2 GetCoords(int32_t vContour, int32_t vPoint, double scale);
};

class FontInfos;
class GlyphInfos
{
public:
	ImFontGlyph glyph;
	std::string oldHeaderName;
	std::string newHeaderName;
	ImWchar newCodePoint = 0;
	FontInfos *fontAtlas = 0;
	SimpleGlyph_Solo simpleGlyph;

public: // for interaction only
	bool m_editingName = false;
	bool m_editingCodePoint = false;
	int editCodePoint = 0;
	std::string editHeaderName;

public:
	GlyphInfos();
	GlyphInfos(
		ImFontGlyph vGlyph, std::string vOldName, 
		std::string vNewName, ImWchar vNewCodePoint = 0);
	~GlyphInfos();
};
