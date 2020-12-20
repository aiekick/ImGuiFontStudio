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

#include "glad/glad.h"
#include <imgui.h>

#include <ConfigAbstract.h>
#include "Project/ProjectFile.h"

#include <functional>
#include <string>
#include <vector>
#include <map>

struct GLFWwindow;
class MainFrame : public conf::ConfigAbstract
{
public:
	bool leftMouseClicked = false;
	bool leftMouseReleased = false;
	bool rightMouseClicked = false;
	ImVec2 m_DisplaySize = ImVec2(100, 100);

private:
	ProjectFile m_ProjectFile;	

	bool m_ShowImGui = false;
	bool m_ShowImGuiStyle = false;
	bool m_ShowAboutDialog = false;

public:
	void Init();
	void Unit();

	void NewProject(const std::string& vFilePathName);
	void LoadProject(const std::string& vFilePathName);
	void SaveProject();
	void SaveAsProject(const std::string& vFilePathName);
	ProjectFile* GetProject();

	void Display(ImVec2 vSize);

public: // save : on quit or project loading
	void IWantToCloseTheApp(); // user want close app, but we want to ensure its saved

private: // save : on quit or project loading
	bool m_SaveDialogIfRequired = false;
	bool m_NeedToCloseApp = false;
	std::list<std::function<void()>> m_SaveChangeDialogActions;
	void ShowSaveDialogIfRequired(); // show a dilaog because the project file is not saved
	void ReRouteFontToFile(const std::string& vFontNameToReRoute, const std::string& vGoodFilePathName);

private: // imgui pane / dialogs
	void DrawDockPane(ImVec2 vSize);
	void DisplayDialogsAndPopups();
	void ShowAboutDialog(bool *vOpen);
	
private:
	void SetAppTitle(const std::string& vFilePathName);

public: // configuration
	std::string getXml(const std::string& vOffset);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent);

public: // singleton
	static MainFrame *Instance(GLFWwindow *vWin = 0)
	{
		static MainFrame *_instance = new MainFrame(vWin);
		return _instance;
	}

protected:
	GLFWwindow *m_Window = 0;
	MainFrame(GLFWwindow *vWin); // Prevent construction
	MainFrame(const MainFrame&) {}; // Prevent construction by copying
	MainFrame& operator =(const MainFrame&) { return *this; }; // Prevent assignment
	~MainFrame(); // Prevent unwanted destruction
};

