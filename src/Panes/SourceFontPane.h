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

#include <Panes/Abstract/AbstractPane.h>
#include <ctools/ConfigAbstract.h>
#include <ctools/cTools.h>
#include <Gui/ImGuiWidgets.h>

#include <string>
#include <map>
#include <set>

class FontInfos;
class ProjectFile;
struct ImGuiWindow;
class SourceFontPane : public AbstractPane
{
private: // per pane settings to save
	//int m_GlyphSize_Policy_Count = 20;
	//float m_GlyphSize_Policy_Width = 40.0f;

private: // private vars
	ImGuiListClipper m_FontsClipper;

private: // private enum
	bool m_Show_ConfirmToCloseFont_Dialog = false;  // show confirm to close font dialog

public:
	void Init() override;
	void Unit() override;
	int DrawPanes(ProjectFile* vProjectFile, int vWidgetId) override;
	void DrawDialogsAndPopups(ProjectFile* vProjectFile) override;
	int DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas) override;

private: 
	void DrawFilterBar(ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos);
	bool IfCatchedByFilters(std::shared_ptr<FontInfos> vFontInfos, const std::string& vSymbolName);
	void DrawFontTexture(std::shared_ptr<FontInfos> vFontInfos);
	bool DrawGlyph(ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos, std::string vName, bool* vSelected, ImVec2 vGlyphSize, ImFontGlyph vGlyph, ImVec2 vHostTextureSize);
	void DrawFontAtlas_Virtual(ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos);
	
	// panes
	void DrawSourceFontPane(ProjectFile *vProjectFile);

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);

public: // singleton
	static SourceFontPane* Instance()
	{
		static SourceFontPane *_instance = new SourceFontPane();
		return _instance;
	}

protected:
	SourceFontPane(); // Prevent construction
	SourceFontPane(const SourceFontPane&) {}; // Prevent construction by copying
	SourceFontPane& operator =(const SourceFontPane&) { return *this; }; // Prevent assignment
	~SourceFontPane(); // Prevent unwanted destruction};
};

