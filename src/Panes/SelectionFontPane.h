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
#include<Gui/ImWidgets.h>

#include <imgui/imgui.h>
#include <map>
#include <string>
#include <vector>

enum SelectedFontPaneModeFlags
{
	SELECTED_FONT_PANE_NONE = 0,
	SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT = (1 << 0),
	SELECTED_FONT_PANE_ORDERED_BY_NAMES = (1 << 1),
};

class GlyphInfos;
class ProjectFile;
class FontInfos;
class SelectionFontPane : public AbstractPane
{
private:
	SelectedFontPaneModeFlags m_SelectedFontPaneModeFlags =
		SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES;
	
	bool m_GlyphEdition = false;
	bool m_AutoUpdateCodepoint_WhenEditWithButtons = false;

	ImVec4 m_GlyphButtonStateColor[3] = { ImVec4(), ImVec4(), ImVec4() };

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas, PaneFlags& vInOutPaneShown)  override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)  override;

	// Preparation
	bool IsSelectedFontPaneMode(SelectedFontPaneModeFlags vSelectedFontPaneModeFlags);
	void PrepareSelection();

private:
	void DrawSelectedFontPane(PaneFlags& vInOutPaneShown);

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);

public: // singleton
	static std::shared_ptr<SelectionFontPane> Instance()
	{
		static auto _instance = std::make_shared<SelectionFontPane>();
		return _instance;
	}

public:
	SelectionFontPane(); // Prevent construction
	SelectionFontPane(const SelectionFontPane&) {}; // Prevent construction by copying
	SelectionFontPane& operator =(const SelectionFontPane&) { return *this; }; // Prevent assignment
	~SelectionFontPane(); // Prevent unwanted destruction};
};

