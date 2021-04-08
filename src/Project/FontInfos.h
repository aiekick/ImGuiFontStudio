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
#include <ttfrrw/ttfrrw.h>

#include <Project/GlyphInfos.h>
#include <Generator/GenMode.h>
#include <ImguiImpl/freetype/imgui_freetype_unleashed.h>
#include <glad/glad.h>
#include "BaseGlyph.h"

#include <imgui/imgui.h>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <utility>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>

enum RasterizerEnum
{
	RASTERIZER_FREETYPE = 0,
	RASTERIZER_TTFRRW,
	RASTERIZER_Count
};

struct GlyphFilteringStats
{
	uint32_t m_CountGlyphs = 0;
	uint32_t m_CountFilteredGlyphs = 0;

	bool m_UseFilterColoring = false;
	uint32_t m_CountSimpleGlyphs = 0;
	ImVec4 SimpleColor = ImVec4(0.8f, 0.2f, 0.2f, 0.8f);
	uint32_t m_CountCompositeGlyphs = 0;
	ImVec4 CompositeColor = ImVec4(0.8f, 0.2f, 0.5f, 0.8f);
	uint32_t m_CountColoredGlyphs = 0;
	ImVec4 ColoredColor = ImVec4(0.2f, 0.5f, 0.2f, 0.8f);
	uint32_t m_CountLayerGlyphs = 0;
	ImVec4 LayerColor = ImVec4(0.3f, 0.8f, 0.4f, 0.8f);
	uint32_t m_CountMappedGlyphs = 0;
	ImVec4 MappedColor = ImVec4(0.5f, 0.2f, 0.5f, 0.8f);
	uint32_t m_CountUnMappedGlyphs = 0;
	ImVec4 UnMappedColor = ImVec4(0.5f, 0.8f, 0.4f, 0.8f);
	uint32_t m_CountMamedGlyphs = 0;
	ImVec4 NamedColor = ImVec4(0.2f, 0.5f, 0.8f, 0.8f);
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
	static std::atomic<uint32_t> countThreadInParallel;
	static uint32_t maxThreadInParallel;

private: // thread
	std::thread m_LoadingThread;
	std::atomic<bool> m_LoadingWorking;					// thread
	std::atomic<float> m_LoadingProgress;				// thread
	std::atomic<uint32_t> m_LoadingObjectsCount;		// thread
	std::function<void()> m_LoadingThreadFinishFunc;	// thread
	bool m_WaitingToBeLoaded = true;
	std::string m_FontFilePathNameToLoad;

public: // containers not to save
	std::unordered_map<GlyphIndex, BaseGlyph> m_Glyphs; // just search
	std::vector<std::string> m_GlyphNames;
	std::unordered_map<CodePoint, GlyphIndex> m_CodePointToGlyphIndex; // just search
	std::vector<GlyphIndex> m_FilteredGlyphs;
	std::vector<std::pair<std::string, std::string>> m_InfosToDisplay;
	GlyphFilteringStats m_GlyphFilteringStats;

	std::map<GlyphIndex, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsOrderedByGlyphIndex; // need order
	std::map<CodePoint, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsOrderedByCodePoints; // need order
	std::map<std::string, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsOrderedByGlyphName; // need order

public: // containers to save
	std::map<GlyphIndex, std::shared_ptr<GlyphInfos>> m_SelectedGlyphs; // need order
	std::set<std::string> m_Filters;

public: // not to save
	std::weak_ptr<FontInfos> m_This;
	ImFontAtlas m_ImFontAtlas;
	TTFRRW::TTFRRW m_TTFRRW;

	char m_SearchBuffer[1024] = "\0";
	ImFontConfig m_FontConfig;
	bool m_NeedFilePathResolve = false; // the path is not found, need resolve for not lost glyphs datas
	bool m_NameInDoubleFound = false;
	bool m_CodePointInDoubleFound = false;
	int m_Ascent = 0;
	int m_Descent = 0;
	int m_LineGap = 0;
	ct::ivec4 m_BoundingBox;
	float m_Point = 0.0f; // could be a double maybe
	float m_FontDiffScale = 0.0f; // for correct glyph asepct when glyp outside of Ascent - Descent by bbox.MaxY-bbox.MinY
	float m_AscentDiffScale = 0.0f; // same but Ascent / bbox.MaxY
	std::string m_FontFileName;
	ImGuiListClipper m_InfosToDisplayClipper;
	bool m_IsLoaded = false;

public: // to save
	std::string m_FontPrefix; // can be used for one/batch generation
	std::string m_FontFilePathName;
	int m_Oversample = 1; // for Rasterizer (AA)
	int m_FontSize = 17; // for Rasterizer
	float m_FontMultiply = 1.0f; // for Rasterizer
	int32_t m_FontPadding = 1; // for Rasterizer
	std::string m_GeneratedFileName; // the file name defined by the user in save dialog
	RasterizerEnum m_RasterizerMode = RasterizerEnum::RASTERIZER_FREETYPE;
	GLenum m_TextureFiltering = GL_LINEAR; // for texture generation
	uint32_t m_FreeTypeFlag = ImGuiFreeType_unleashed::FreeType_Default;
	uint32_t m_CardGlyphHeightInPixel = 40U; // glyph item height in card
	uint32_t m_CardCountRowsMax = 20U; // after this max, new columns
	bool m_EnabledForGeneration = true; // enable for generation (in abtch mode per font settings)
	bool m_CollapseFontInFinalPane = false; // collapse the font in final pane

public:// filtering to save
	bool m_GlyphFilteringOpOR = true; // or / and
	GlyphCategoryFlags m_GlyphDisplayCategoryFlags = GLYPH_CATEGORY_FLAG_ALL;

public: // callable
	bool LoadFont(
		ProjectFile* vProjectFile, 
		const std::string& vFontFilePathName,
		std::function<void()> vLoadingThreadFinishFunc);
	void Clear();
	std::string GetGlyphName(GlyphIndex vGlyphIndex);
	void DrawInfos(ProjectFile* vProjectFile);
	void DrawFilteringWidgets(ProjectFile* vProjectFile);
	void UpdateInfos();
	void UpdateFiltering();
	void UpdateSelectedGlyphs();
	void ClearTransforms(ProjectFile* vProjectFile);
	void ClearScales(ProjectFile* vProjectFile);
	void ClearTranslations(ProjectFile* vProjectFile);
	BaseGlyph* GetGlyphByGlyphIndex(GlyphIndex vGlyphIndex);
	BaseGlyph* GetFirstGlyphByCodePoint(CodePoint vCodePoint);
	ImFont* GetImFontPtr();
	bool IsUsable();

public: // thread
	bool LoadFontThreaded(
		ProjectFile* vProjectFile,
		const std::string& vFontFilePathName,
		std::atomic<bool>& vWorking,
		std::atomic<float>& vProgress,
		std::atomic<uint32_t>& vObjectsCount);
	void StartLoadingThread(
		ProjectFile* vProjectFile,
		const std::string& vFontFilePathName,
		std::function<void()> vLoadingThreadFinishFunc);
	void DrawLoadingProgressBar(ProjectFile* vProjectFile);
	bool StopLoadingThread();
	bool IsJoinable();
	void Join();
	void PendingThreadAction(ProjectFile* vProjectFile);

private: // DB's build
	void Build_Glyph_DataBase();

private: // Opengl Texture
	void CreateOrUpdateFontTexture();
	void DestroyFontTexture();

public: // Configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");

public: // Cons/Des tructors
	FontInfos();
	~FontInfos();
};


