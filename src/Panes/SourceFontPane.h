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
private:
	ImGuiListClipper m_FontsClipper;

public:
	enum SourceFontPaneFlags
	{
		SOURCE_FONT_PANE_NONE = 0,
		SOURCE_FONT_PANE_GLYPH = (1 << 0),
		SOURCE_FONT_PANE_TEXTURE = (1 << 1)
	} m_FontPaneFlags = SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH;

public:
	int DrawSourceFontPane(ProjectFile *vProjectFile, int vWidgetId) const;
	int DrawParamsPane(ProjectFile *vProjectFile, int vWidgetId);

	static void OpenFonts(ProjectFile *vProjectFile, const std::map<std::string, std::string>& vFontFilePathNames);
	static void OpenFont(ProjectFile *vProjectFile, const std::string& vFontFilePathName, bool vUpdateCount);
	
	static void DrawDialosAndPopups(ProjectFile * vProjectFile);
	static void CloseCurrentFont(ProjectFile *vProjectFile);
	static void SelectFont(ProjectFile *vProjectFile, FontInfos *vFontInfos);

private:
	static void DrawFilterBar(ProjectFile *vProjectFile, FontInfos *vFontInfos);
	static bool IfCatchedByFilters(FontInfos *vFontInfos, const std::string& vSymbolName);

private:
	//void DrawFontAtlas(ProjectFile *vProjectFile, FontInfos *vFontInfos);
	static void DrawFontTexture(FontInfos *vFontInfos);

private: // experimental => virtual list
    static void DrawFontAtlas_Virtual(ProjectFile *vProjectFile, FontInfos *vFontInfos);

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

