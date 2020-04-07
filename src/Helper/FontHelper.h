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

#include <string>
#include <set>
#include <map>
#include <unordered_map>

#include "sfntly/tag.h"
#include "sfntly/font.h"
#include "sfntly/port/type.h"
#include "sfntly/port/refcount.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/table/core/post_script_table.h"
#include "sfntly/table/truetype/glyph_table.h"
#include "sfntly/table/truetype/loca_table.h"

typedef int32_t FontId;
typedef int32_t CodePoint;
typedef int32_t GlyphId;
typedef std::pair<FontId, GlyphId> FontGlyphId;
struct Glyph
{
	CodePoint codepoint = 0;
	GlyphId glyphid = 0;
	FontId fontId = 0;
	Glyph(CodePoint vCdp, GlyphId vGid, FontId vFid)
	{
		codepoint = vCdp;
		glyphid = vGid;
		fontId = vFid;
	}
};
typedef std::pair<int32_t, std::string> CodePointName;

class FontInstance
{
public:

public:
	sfntly::Ptr<sfntly::Font> m_Font;
	sfntly::Ptr<sfntly::CMapTable::CMap> m_CMapTable;
	sfntly::Ptr<sfntly::LocaTable> m_LocaTable;
	sfntly::Ptr<sfntly::GlyphTable> m_GlyfTable;
	std::map<int32_t, int32_t> m_CharMap; // codepoint to glyph id
	std::map<int32_t, int32_t> m_ReversedCharMap; // glyph id to codepoint
	std::set<int32_t> m_ResolvedSet;
	std::map<int32_t, std::string> m_NewGlyphNames;
	std::map<CodePoint, CodePoint> m_NewGlyphCodePoints;
	std::map<int32_t, int32_t> m_OldToNewGlyfId;
	std::vector<int32_t> m_NewToOldGlyfId;
};

class FontHelper
{
public:
	FontHelper();
	~FontHelper();

public: // read
	bool OpenFontFile(
		const std::string& vFontFilePathName,
		std::map<CodePoint, std::string> vNewNames,
		std::map<CodePoint, CodePoint> vNewCodePoints);
	bool GenerateFontFile(const std::string& vFontFilePathName, bool vUsePostTable);


private:
	std::vector<FontInstance> m_Fonts;
	sfntly::Ptr<sfntly::FontFactory> m_FontFactory;
	sfntly::Ptr<sfntly::Font::Builder> m_FontBuilder;
	std::map<CodePoint, FontGlyphId> m_CharMap; // codepoint to glyph id
	std::map<FontGlyphId, CodePoint> m_ReversedCharMap; // glyph id to codepoint
	std::map<CodePoint, std::string> m_GlyphNames;
	std::set<FontGlyphId> m_ResolvedSet; // set of font id / glyph id
	std::map<FontGlyphId, GlyphId> m_OldToNewGlyfId;
	std::map<GlyphId, std::vector<GlyphId>> m_NewToOldGlyfId;

private: // post table - version / count / size / offsets
	const int32_t table_Version = 0x20000;
	const int32_t count_StandardNames = 258;
	const int32_t size_Header = 32;
	std::unordered_map<std::string, int32_t> m_InvertedStandardNames;
	std::unordered_map<std::string, int32_t> InvertNameMap();
	int32_t MergeCharacterMaps();

public:
	sfntly::Font* LoadFontFile(const char* font_path);

private:
	// imported/based on/modified from sfntly
	void LoadFontFiles(const char* font_path, sfntly::FontFactory* factory, sfntly::FontArray* fonts);
	bool SerializeFont(const char* font_path, sfntly::Font* font);
	bool SerializeFont(const char* font_path, sfntly::FontFactory* factory, sfntly::Font* font);
	sfntly::Font* AssembleFont(bool vUsePostTable);
	bool Assemble_Glyf_Loca_Maxp_Tables();
	bool Assemble_CMap_Table();
	bool Assemble_Hmtx_Hhea_Tables();
	bool Assemble_Post_Table(std::map<CodePoint, std::string> vSelection);
	bool Assemble_Meta_Table();
	bool Assemble_Head_Table();
	void FillCharacterMap(FontInstance *vFontInstance, std::map<CodePoint, std::string> vSelection);
	void FillResolvedCompositeGlyphs(FontInstance *vFontInstance, std::map<CodePoint, CodePoint> chars_to_glyph_ids);

};

