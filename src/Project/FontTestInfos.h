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
#include <Helper/SelectionHelper.h>

#include <string>
#include <set>
#include <vector>
#include <memory>

class ProjectFile;
class FontTestInfos : public conf::ConfigAbstract
{
public: // not to save
	std::weak_ptr<FontInfos> m_TestFont;
	typedef std::pair<uint32_t, std::string> FontInfosCodePoint_ToLoad;
	std::map<uint32_t, FontInfosCodePoint_ToLoad> m_GlyphToInsert_ToLoad; // pos in word, glyph
	char m_InputBuffer[500] = "ImGuiFontStudio\0";

public: // to save
	float m_PreviewFontSize = 100.0f;
	bool m_ShowBaseLine = true;
	std::string m_TestFontName;
	std::map<uint32_t, FontInfosCodePoint> m_GlyphToInsert; // pos in word, glyph
	std::string m_TestString = "ImGuiFontStudio";
	
public: // callable
	void Clear();
	void Load();
	void ResizeInsertedGlyphs(uint32_t vPos, bool vExpandOrReduce);

public: // Configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");

public: // Cons/Des tructors
	FontTestInfos();
	~FontTestInfos();
};


