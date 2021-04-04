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
#include <Gui/ImWidgets.h>

#include <string>
#include <map>
#include <set>

class FontInfos;
class ProjectFile;
struct ImGuiWindow;
class ParamsPane : public AbstractPane
{
private: // per pane settings to save
	int m_GlyphSize_Policy_Count = 20;
	float m_GlyphSize_Policy_Width = 40.0f;
	
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

	void OpenFonts(ProjectFile* vProjectFile, const std::map<std::string, std::string>& vFontFilePathNames);
	bool OpenFont(ProjectFile* vProjectFile, const std::string& vFontFilePathName, bool vUpdateCount);
	void SelectFont(ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos);

private: 
	void DrawParamsPane(ProjectFile *vProjectFile);

private: // actions
	// via menu
	void Action_Menu_OpenFont();
	void Action_Menu_CloseFont();
	void Action_Cancel();
	void Open_ConfirmToCloseFont_Dialog(); // dialog
	void Close_ConfirmToCloseFont_Dialog(); // dialog
	bool Display_ConfirmToCloseFont_Dialog(); // dialog

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);

public: // singleton
	static ParamsPane* Instance()
	{
		static ParamsPane *_instance = new ParamsPane();
		return _instance;
	}

protected:
	ParamsPane(); // Prevent construction
	ParamsPane(const ParamsPane&) {}; // Prevent construction by copying
	ParamsPane& operator =(const ParamsPane&) { return *this; }; // Prevent assignment
	~ParamsPane(); // Prevent unwanted destruction};
};

