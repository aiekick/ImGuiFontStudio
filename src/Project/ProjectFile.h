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

#include "FontInfos.h"
#include "Generator/Generator.h"

class ProjectFile : public conf::ConfigAbstract
{
public: // to save
	std::map<std::string, FontInfos> m_Fonts;
	bool m_ShowRangeColoring = false;
	ImVec4 m_RangeColoringHash = ImVec4(10, 15, 35, 0.5f);
	int m_Preview_Glyph_CountX = 20;
	std::string m_ProjectFilePathName;
	std::string m_ProjectFilePath;
	std::string m_MergedFontPrefix;
	GenModeFlags m_GenMode = (GenModeFlags)(
		GenModeFlags::GENERATOR_MODE_CURRENT_HEADER | 
		GenModeFlags::GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_NAMES | 
		GenModeFlags::GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES);

public: // dont save
	FontInfos *m_CurrentFont = 0;
	size_t m_CountSelectedGlyphs = 0; // for all fonts
	size_t m_CountFontWithSelectedGlyphs = 0; // for all fonts

private: // dont save
	bool m_IsLoaded = false;
	bool m_NeverSaved = false;
	bool m_IsThereAnyNotSavedChanged = false;

public:
	ProjectFile();
	ProjectFile(const std::string& vFilePathName);
	~ProjectFile();

	void Clear();
	void New();
	void New(const std::string& vFilePathName);
	bool Load();
	bool LoadAs(const std::string& vFilePathName);
	bool Save();
	bool SaveAs(const std::string& vFilePathName);
	bool IsLoaded();

	bool IsThereAnyNotSavedChanged();
	void SetProjectChange(bool vChange = true);

	void UpdateCountSelectedGlyphs();

	bool IsRangeColorignShown();

	std::string GetAbsolutePath(const std::string& vFilePathName);
	std::string GetRelativePath(const std::string& vFilePathName);

public: // Generation Mode
	void AddGenMode(GenModeFlags vFlags);
	void RemoveGenMode(GenModeFlags vFlags);
	GenModeFlags GetGenMode();
	bool IsGenMode(GenModeFlags vFlags);

public:
	std::string getXml(const std::string& vOffset);
	void setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent);

public: // utils
	ImVec4 GetColorFromInteger(int vInteger);
};

