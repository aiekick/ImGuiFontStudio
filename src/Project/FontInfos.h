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

#include <Project/GlyphInfos.h>
#include <common/freetype/imgui_freetype.h>
#include <glad/glad.h>
#include <Generator/GenMode.h>

#include <imgui/imgui.h>
#include <string>
#include <set>
#include <vector>
#include <utility>
#include <memory>

enum RasterizerEnum
{
	RASTERIZER_STB = 0,
	RASTERIZER_FREETYPE,
	RASTERIZER_Count
};

struct GlyphsRange
{
//	std::set<uint32_t> datas;
	int rangeStart = 0;
	int rangeEnd = 0;
};

class ProjectFile;
class FontInfos : public conf::ConfigAbstract, public GenMode
{
public:
	static std::shared_ptr<FontInfos> Create();
	
public: // not to save
	std::weak_ptr<FontInfos> m_This;
	ImFontAtlas m_ImFontAtlas;
	std::vector<std::string> m_GlyphNames;
	std::map<uint32_t, std::string> m_GlyphCodePointToName;
	std::map<uint32_t, uint32_t> m_GlyphCodePointToGlyphIndex;
	std::map<uint32_t, uint32_t> m_GlyphGlyphIndexToCodePoint;
	std::map<uint32_t, bool> m_ColoredGlyphs; // codepoint, true/false if colored
	char m_SearchBuffer[1024] = "\0";
	ImFontConfig m_FontConfig;
	bool m_NeedFilePathResolve = false; // the path is not found, need resolve for not lost glyphs datas
	bool m_NameInDoubleFound = false;
	bool m_CodePointInDoubleFound = false;
	std::map<uint32_t, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsOrderedByCodePoints;
	std::map<std::string, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsOrderedByGlyphName;
	int m_Ascent = 0;
	int m_Descent = 0;
	int m_LineGap = 0;
	ct::ivec4 m_BoundingBox;
	float m_Point = 0.0f;
	std::string m_FontFileName;
	std::vector<std::pair<std::string, std::string>> m_InfosToDisplay;
	ImGuiListClipper m_InfosToDisplayClipper;
	std::vector<ImFontGlyph> m_FilteredGlyphs;

public: // to save
	std::map<uint32_t, std::shared_ptr<GlyphInfos>> m_SelectedGlyphs;
	std::string m_FontPrefix; // peut servir pour la generation par lot
	std::string m_FontFilePathName;
	int m_Oversample = 1; // for Rasterizer (AA)
	int m_FontSize = 17; // for Rasterizer
	float m_FontMultiply = 1.0f; // for Rasterizer
	int32_t m_FontPadding = 1; // for Rasterizer
	std::string m_GeneratedFileName; // the file name defined by the user in save dialog
	std::set<std::string> m_Filters; // use map just for have binary tree search
	RasterizerEnum m_RasterizerMode = RasterizerEnum::RASTERIZER_FREETYPE;
	uint32_t m_FreeTypeFlag = ImGuiFreeType::FreeType_Default;
	GLenum m_TextureFiltering = GL_LINEAR; // for texture generation
	uint32_t m_CardGlyphHeightInPixel = 40U; // glyph item height in card
	uint32_t m_CardCountRowsMax = 20U; // after this max, new columns
	bool m_EnabledForGeneration = true; // enable for generation (in abtch mode per font settings)
	bool m_CollapseFontInFinalPane = false; // collapse the font in final pane

public: // callable
	bool LoadFont(ProjectFile *vProjectFile, const std::string& vFontFilePathName);
	void Clear();
	std::string GetGlyphName(uint32_t vCodePoint);
	void DrawInfos(ProjectFile* vProjectFile);
	void UpdateInfos();
	void UpdateFiltering();
	void UpdateSelectedGlyphs(ImFont* vFont);
	void ClearTransforms(ProjectFile* vProjectFile);
	void ClearScales(ProjectFile* vProjectFile);
	void ClearTranslations(ProjectFile* vProjectFile);
	ImFont* GetImFont();

private: // Glyph Names Extraction / DB
	void FillGlyphNames();
	void GenerateCodePointToGlypNamesDB();
	void FillGlyphColoreds();

private: // Opengl Texture
	void CreateFontTexture();
	void DestroyFontTexture();

public: // Configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");

public: // Cons/Des tructors
	FontInfos();
	~FontInfos();
};


