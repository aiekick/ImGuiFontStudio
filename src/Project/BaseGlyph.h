#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <ttfrrw/ttfrrw.h>
#include <imgui/imgui.h>

typedef uint32_t CodePoint;
typedef uint32_t GlyphIndex;
typedef std::vector<ct::fvec2> GlyphContour;
typedef std::vector<GlyphContour> GlyphMesh;

typedef int GlyphCategoryFlags;
enum GlyphCategoryFlags_
{
	GLYPH_CATEGORY_FLAG_NONE = 0,
	GLYPH_CATEGORY_FLAG_SIMPLE = (1 << 0),
	GLYPH_CATEGORY_FLAG_COMPOSITE = (1 << 1),
	GLYPH_CATEGORY_FLAG_MAPPED = (1 << 2),
	GLYPH_CATEGORY_FLAG_UNMAPPED = (1 << 3),
	GLYPH_CATEGORY_FLAG_COLORED = (1 << 4),
	GLYPH_CATEGORY_FLAG_LAYER = (1 << 5),
	GLYPH_CATEGORY_FLAG_NAMED = (1 << 6),
	GLYPH_CATEGORY_FLAG_ALL = 
		GLYPH_CATEGORY_FLAG_SIMPLE | 
		GLYPH_CATEGORY_FLAG_COMPOSITE | 
		GLYPH_CATEGORY_FLAG_MAPPED | 
		GLYPH_CATEGORY_FLAG_UNMAPPED |
		GLYPH_CATEGORY_FLAG_COLORED |
		GLYPH_CATEGORY_FLAG_LAYER |
		GLYPH_CATEGORY_FLAG_NAMED
};

class FontInfos;
struct BaseGlyph
{
	GlyphIndex glyphIndex = 0;

	CodePoint codePoint = 0;

	float AdvanceX = 0.0f;	

	float X0 = 0.0f; 
	float Y0 = 0.0f; 
	float X1 = 0.0f; 
	float Y1 = 0.0f;

	float U0 = 0.0f; 
	float V0 = 0.0f; 
	float U1 = 0.0f; 
	float V1 = 0.0f;

	GlyphCategoryFlags category = GLYPH_CATEGORY_FLAG_NONE;

	std::string name;

	ImVec4 bbox;
	std::vector<TTFRRW::Contour> contours; // contours from ttfrrw
	std::weak_ptr<FontInfos> fontInfos; // fontinfox
	GlyphMesh mesh; // poly partition result ready to display

	// layers
	// this is the parent GlyphIndex.
	// a layer can be used with many other parent glyph
	std::unordered_map<GlyphIndex, ImVec4> color;
	std::unordered_map<GlyphIndex, uint16_t> paletteIndex;
	std::set<GlyphIndex> parents; // parent of glyphindex

	// main glyph
	std::vector<GlyphIndex> layers; // layers of glyphindex
};