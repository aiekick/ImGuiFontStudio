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

#define PARAM_PANE "Parameters"
#define SOURCE_PANE "Source Fonts"
#define FINAL_PANE "Final Font"
#define GENERATOR_PANE "Generator"
#define SELECTED_FONT_PANE "Selected Font"
#define GLYPH_PANE "Glyph Edition"
#define FONT_STRUCTURE_PANE "Font Structure"
#define FONT_PREVIEW_PANE "Font Preview"
#ifdef _DEBUG
#define DEBUG_PANE "Debug"
#endif

enum PaneFlags
{
	PANE_NONE = 0,
	PANE_SELECTED_FONT = (1 << 1),
	PANE_SOURCE = (1 << 2),
	PANE_FINAL = (1 << 3),
	PANE_PARAM = (1 << 4),
	PANE_GENERATOR = (1 << 5),
	PANE_GLYPH = (1 << 6),
	PANE_FONT_STRUCTURE = (1 << 7),
	PANE_FONT_PREVIEW = (1 << 8),
#ifdef _DEBUG
	PANE_DEBUG = (1 << 9),
#endif
	PANE_OPENED_DEFAULT =
		PANE_SELECTED_FONT | PANE_SOURCE | PANE_FINAL | PANE_PARAM | PANE_GENERATOR,// | PANE_FONT_PREVIEW,
	PANE_FOCUS_DEFAULT =
		PANE_SELECTED_FONT | PANE_SOURCE | PANE_PARAM | PANE_GENERATOR | PANE_FONT_PREVIEW
};

class ProjectFile;
class LayoutManager : public conf::ConfigAbstract
{
private:
	ImGuiID m_DockSpaceID = 0;
	bool m_FirstLayout = false;
	bool m_FirstStart = true;

public:
	static PaneFlags m_Pane_Shown;
	static PaneFlags m_Pane_Focused;
	PaneFlags m_Pane_Hovered = PaneFlags::PANE_NONE;
	PaneFlags m_Pane_LastHovered = PaneFlags::PANE_NONE;
	ImVec2 m_LastSize;

public:
	void Init();
	void Unit();
	void InitAfterFirstDisplay(ImVec2 vSize);
	void StartDockPane(ImGuiDockNodeFlags vFlags, ImVec2 vSize);
	void ApplyInitialDockingLayout(ImVec2 vSize = ImVec2(0, 0));
	void DisplayMenu(ImVec2 vSize);
	int DisplayPanes(ProjectFile *vProjectFile, int vWidgetId);
	void DrawDialogsAndPopups(ProjectFile* vProjectFile);
	int DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas);
	void ShowSpecificPane(PaneFlags vPane);
	void FocusSpecificPane(PaneFlags vPane);
	void ShowAndFocusSpecificPane(PaneFlags vPane);
	bool IsSpecificPaneFocused(PaneFlags vPane);
	void AddSpecificPaneToExisting(const char* vNewPane, const char* vExistingPane);

private:
	void FocusSpecificPane(const char *vlabel);
	bool IsSpecificPaneFocused(const char *vlabel);

private: // configuration
	PaneFlags GetFocusedPanes();
	void SetFocusedPanes(PaneFlags vActivePanes);

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");

public: // singleton
	static LayoutManager *Instance()
	{
		static auto *_instance = new LayoutManager();
		return _instance;
	}

protected:
	LayoutManager(); // Prevent construction
	LayoutManager(const LayoutManager&) {}; // Prevent construction by copying
	LayoutManager& operator =(const LayoutManager&) { return *this; }; // Prevent assignment
	~LayoutManager(); // Prevent unwanted destruction
};

