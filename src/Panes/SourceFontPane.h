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

#include "ctools/cTools.h"
#include "Gui/ImGuiWidgets.h"

#include <string>
#include <map>
#include <set>

class FontInfos;
class ProjectFile;
struct ImGuiWindow;
class SourceFontPane
{
public:
	enum SourceFontPaneFlags
	{
		SOURCE_FONT_PANE_NONE = 0,
		SOURCE_FONT_PANE_GLYPH = (1 << 0),
		SOURCE_FONT_PANE_TEXTURE = (1 << 1)
	} m_FontPaneFlags = SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH;

public:
	int DrawSourceFontPane(ProjectFile *vProjectFile, int vWidgetId);
	int DrawParamsPane(ProjectFile *vProjectFile, int vWidgetId);

	void OpenFonts(ProjectFile *vProjectFile, std::map<std::string, std::string> vFontFilePathNames);
	void OpenFont(ProjectFile *vProjectFile, std::string vFontFilePathName, bool vUpdateCount);
	
	void DrawDialosAndPopups(ProjectFile * vProjectFile);
	void CloseCurrentFont(ProjectFile *vProjectFile);
	void SelectFont(ProjectFile *vProjectFile, FontInfos *vFontInfos);

private:
	void DrawFilterBar(ProjectFile *vProjectFile, FontInfos *vFontInfos);
	bool IfCatchedByFilters(FontInfos *vFontInfos, const std::string& vSymbolName);

private:
	void DrawFontAtlas(ProjectFile *vProjectFile, FontInfos *vFontInfos);
	void DrawFontTexture(FontInfos *vFontInfos);
	
public: // singleton
	static SourceFontPane *Instance()
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

