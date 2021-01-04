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
#include <imgui/imgui.h>
#include <string>

#include <Project/FontInfos.h>
#include <Generator/Generator.h>
#include <Panes/SourceFontPane.h>

enum GlyphDisplayTuningModeFlags
{
	GLYPH_DISPLAY_TUNING_MODE_NONE = 0,
	GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT = (1 << 0),
	GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE = (1 << 1),
};

class ProjectFile : public conf::ConfigAbstract
{
public: // to save
	std::map<std::string, FontInfos> m_Fonts;
	bool m_ShowRangeColoring = false;
	ImVec4 m_RangeColoringHash = ImVec4(10, 15, 35, 0.5f);
	int m_Preview_Glyph_CountX = 20;
	float m_Preview_Glyph_Width = 50;
	std::string m_ProjectFilePathName;
	std::string m_ProjectFilePath;
	std::string m_MergedFontPrefix;
	GenModeFlags m_GenMode =
	        GENERATOR_MODE_CURRENT_HEADER |
		    //GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_NAMES |
		    GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES;
	bool m_CurrentPane_ShowGlyphTooltip = true;
	bool m_SourcePane_ShowGlyphTooltip = true;
	bool m_FinalPane_ShowGlyphTooltip = true;
	std::string m_FontToMergeIn;
	float m_GlyphPreview_Scale = 1.0f;
	int m_GlyphPreview_QuadBezierCountSegments = 0; // count segments per bezier quad, 0 mean auto tesselation
	bool m_GlyphPreview_ShowControlLines = false;
	GlyphDisplayTuningModeFlags m_GlyphDisplayTuningMode =
		GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT;
	SourceFontPaneFlags m_SourceFontPaneFlags = 
		SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH;
	uint32_t m_CardGlyphHeightInPixel = 40U; // ine item height in card
	uint32_t m_CardCountRowsMax = 20U; // after this max, new columns

public: // dont save
	FontInfos *m_SelectedFont = nullptr;
	size_t m_CountSelectedGlyphs = 0; // for all fonts
	size_t m_CountFontWithSelectedGlyphs = 0; // for all fonts
    bool m_NameFoundInDouble = false;
    bool m_CodePointFoundInDouble = false;

private: // dont save
	bool m_IsLoaded = false;
	bool m_NeverSaved = false;
	bool m_IsThereAnyNotSavedChanged = false;

public:
	ProjectFile();
	explicit ProjectFile(const std::string& vFilePathName);
	~ProjectFile();

	void Clear();
	void New();
	void New(const std::string& vFilePathName);
	bool Load();
	bool LoadAs(const std::string vFilePathName); // ils wanted to not pass the adress for re open case
	bool Save();
	bool SaveAs(const std::string& vFilePathName);
	bool IsLoaded() const;

	bool IsThereAnyNotSavedChanged() const;
	void SetProjectChange(bool vChange = true);

	void UpdateCountSelectedGlyphs();

	bool IsRangeColoringShown() const;

	std::string GetAbsolutePath(const std::string& vFilePathName) const;
	std::string GetRelativePath(const std::string& vFilePathName) const;

public: // Generation Mode
	void AddGenMode(GenModeFlags vFlags);
	void RemoveGenMode(GenModeFlags vFlags);
	GenModeFlags GetGenMode() const;
	bool IsGenMode(GenModeFlags vFlags) const;

public:
	std::string getXml(const std::string& vOffset) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent) override;

public: // utils
	ImVec4 GetColorFromInteger(uint32_t vInteger) const;
};

