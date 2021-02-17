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
#include <memory>
#include <ctools/cTools.h>
#include <ttfrrw/ttfrrw.h>
#include <Project/BaseGlyph.h>
#include <Helper/MeshDecomposer.h>

#define GLYPH_EDIT_CONTROL_WIDTH 180.0f

class ProjectFile;
class GlyphDisplayHelper
{
public:
	static int CalcGlyphsCountAndSize(			/* return new glyph count x				*/
		ProjectFile* vProjectFile,				/* project file for save some vars		*/
		ImVec2* vCellSize,						/* cell size							*/
		ImVec2* vGlyphSize,						/* glyph size (cell - paddings)			*/
		bool vGlyphEdited = false,				/* in edition							*/
		bool vForceEditMode = false,			/* edition forced						*/
		bool vForceEditModeOneColumn = false);	/* edition in one column				*/
};

typedef int GlyphDrawingFlags;
enum _GlyphDrawingFlags
{
	GLYPH_DRAWING_NONE = 0,
	GLYPH_DRAWING_LEGENDS = (1 << 0), // show legends
	GLYPH_DRAWING_FONT_AXIS_X = (1 << 1), // show font axis
	GLYPH_DRAWING_FONT_AXIS_Y = (1 << 2), // show font axis
	GLYPH_DRAWING_FONT_ORIGIN_XY = (1 << 3), // show font axis
	GLYPH_DRAWING_FONT_BBOX = (1 << 4), // show font bounding box
	GLYPH_DRAWING_FONT_ASCENT = (1 << 5), // show font ascent
	GLYPH_DRAWING_FONT_DESCENT = (1 << 6), // show font descent
	GLYPH_DRAWING_GLYPH_BBOX = (1 << 7), // show glyph bounding box
	GLYPH_DRAWING_GLYPH_ADVANCEX = (1 << 8), // show glyph advance x
	GLYPH_DRAWING_GLYPH_CONTROL_LINES = (1 << 9), // show glyph advance x
	GLYPH_DRAWING_CANVAS_GRID = (1 << 10), // show font axis

	GLYPH_DRAWING_GLYPH_Default =
		GLYPH_DRAWING_LEGENDS |
		GLYPH_DRAWING_FONT_AXIS_X |
		GLYPH_DRAWING_FONT_AXIS_Y |
		GLYPH_DRAWING_FONT_ORIGIN_XY |
		GLYPH_DRAWING_FONT_BBOX |
		GLYPH_DRAWING_FONT_ASCENT |
		GLYPH_DRAWING_FONT_DESCENT |
		GLYPH_DRAWING_GLYPH_BBOX |
		GLYPH_DRAWING_GLYPH_ADVANCEX |
		GLYPH_DRAWING_CANVAS_GRID
};

class FontInfos;
class GlyphInfos;
class SimpleGlyph_Solo
{
public:
	bool isValid = false;

public:
	BaseGlyph m_Glyph;
	std::vector<std::pair<BaseGlyph, bool>> m_Layers;
	ct::fvec2 m_Translation; // translation in first
	ct::fvec2 m_Scale = 1.0f; // scale in second

private:
	GlyphContour ComputeAbsolutePointsFromContour(TTFRRW::Contour vContour, int vQuadBezierCountSegments);
	GlyphMesh ComputeGlyphMesh(BaseGlyph vGlyph, int vQuadBezierCountSegments, PartitionAlgoFlags vPartitionAlgoFlags);

public:
	ImVec2 getScreenToLocal(ImVec2 vScreenPos, ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin);
	ImVec2 getLocalToScreen(ImVec2 vLocalPos, ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, float vWorldScale, ImVec2 vLocalBBoxOrigin);
	ct::ivec2 TransformCoords(ct::ivec2 vPoint);

public:
	void Clear();
	void LoadGlyph(BaseGlyph *vGlyph, ProjectFile* vProjectFile);
	int GetCountContours(BaseGlyph* vGlyph);
	int GetCountPoints(BaseGlyph* vGlyph);
	int GetMaxContours(); // will explore all glyph, main + layers
	int GetMaxPoints();
	ct::ivec2 GetCoords(BaseGlyph* vGlyph, int32_t vContour, int32_t vPoint);
	bool IsOnCurve(BaseGlyph* vGlyph, int32_t vContour, int32_t vPoint);
	ct::ivec2 Scale(BaseGlyph* vGlyph, ct::ivec2 p, double scale) const;
	//ct::ivec2 GetCoords(int32_t vContour, int32_t vPoint, double scale);
	void ClearTransform();
	void ClearTranslation();
	void ClearScale();
	void ComputeWholeGlyphMesh(int vQuadBezierCountSegments, PartitionAlgoFlags vPartitionAlgoFlags);

public: // ImGui
	void DrawCurves(
		BaseGlyph* vGlyph,
		ImVec2 vZoneStart, ImVec2 vWorldBBoxOrigin, ImVec2 vWorlBBoxSize, 
		float vWorldScale, ImVec2 vLocalBBoxOrigin, ImDrawList* vImDrawList, 
		int vMaxPoint, int vMaxContour, int vQuadBezierCountSegments, GlyphDrawingFlags vGlyphDrawingFlags);
	void DrawGlyph(
		float vGlobalScale,
		std::shared_ptr<FontInfos> vFontInfos,
		std::shared_ptr<GlyphInfos> vGlyphInfos,
		int vMaxPoint, int vMaxContour, int vQuadBezierCountSegments,
		GlyphDrawingFlags vGlyphDrawingFlags);
};

class GlyphInfos
{
public:
	static std::shared_ptr<GlyphInfos> Create(
		std::weak_ptr<FontInfos> vFontInfos,
		BaseGlyph vGlyph, std::string vOldName,
		std::string vNewName, uint32_t vNewGlyphIndex = 0,
		ImVec2 vTranslation = ImVec2(0, 0), ImVec2 vScale = ImVec2(1, 1));
	// 0 => none, 1 => left pressed, 2 => right pressed
	static int DrawGlyphButton(
		int& vWidgetPushId, // by adress because we want modify it
		ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
		bool* vSelected, ImVec2 vGlyphSize, BaseGlyph* vGlyph, int vParentGlyphIndex = -1,
		ImVec2 vTranslation = ImVec2(0, 0), ImVec2 vScale = ImVec2(1, 1),
		int frame_padding = -1, float vRectThickNess = 0.0f, ImVec4 vRectColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	static void RenderGlyph(
		ImFont* vFont, ImDrawList* vDrawList, 
		float vGlyphHeight, ImVec2 vMin, ImVec2 vMax, ImVec2 vOffset,
		ImU32 vCol, 
		ImWchar vGlyphGlyphIndex, 
		ImVec2 vTranslation, ImVec2 vScale, 
		bool vZoomed);

private:
	std::weak_ptr<FontInfos> fontInfos;

public:
	BaseGlyph glyph = {};
	std::string newHeaderName;
	uint32_t newCodePoint = 0;
	SimpleGlyph_Solo simpleGlyph;
	//CompositeGlyph_Solo compositeGlyph;
	ct::fvec2 m_Translation = 0.0f;
	ct::fvec2 m_Scale = 1.0f;

	// filled during generation only
	// not to use outside of generation
	ct::ivec4 m_FontBoundingBox;
	int m_FontAscent = 0;
	int m_FontDescent = 0;
	
public: // for interaction only
	bool m_editingName = false;
	bool m_editingGlyphIndex = false;
	int editGlyphIndex = 0;
	std::string editHeaderName;

public:
	GlyphInfos();
	GlyphInfos(
		std::weak_ptr<FontInfos> vFontInfos,
		BaseGlyph vGlyph, std::string vOldName,
		std::string vNewName, uint32_t vNewGlyphIndex, 
		ImVec2 vTranslation, ImVec2 vScale);
	~GlyphInfos();

	std::weak_ptr<FontInfos> GetFontInfos();
	void SetFontInfos(std::weak_ptr<FontInfos> vFontInfos);
};
