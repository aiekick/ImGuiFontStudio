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
#include<Gui/ImWidgets.h>

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
	ImGuiListClipper m_VirtualClipper;

private: // private enum
	bool m_Show_ConfirmToCloseFont_Dialog = false;  // show confirm to close font dialog

	ImVec4 m_GlyphButtonStateColor[3] = { ImVec4(), ImVec4(), ImVec4() };

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas, PaneFlags& vInOutPaneShown)  override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)  override;

private: 
	void DrawFilterBar(std::shared_ptr<FontInfos> vFontInfos);
	void DrawFontTexture(std::shared_ptr<FontInfos> vFontInfos);
	void DrawFontAtlas_Virtual(std::shared_ptr<FontInfos> vFontInfos);
	
	// panes
	void DrawSourceFontPane(PaneFlags& vInOutPaneShown);

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);

public: // singleton
	static std::shared_ptr<SourceFontPane> Instance()
	{
		static auto _instance = std::make_shared<SourceFontPane>();
		return _instance;
	}

public:
	SourceFontPane(); // Prevent construction
	SourceFontPane(const SourceFontPane&) {}; // Prevent construction by copying
	SourceFontPane& operator =(const SourceFontPane&) { return *this; }; // Prevent assignment
	~SourceFontPane(); // Prevent unwanted destruction};
};

