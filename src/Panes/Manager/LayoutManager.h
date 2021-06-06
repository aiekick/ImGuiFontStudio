/*
MIT License

Copyright (c) 2021 Stephane Cuillerdier (aka Aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <ctools/ConfigAbstract.h>
#include "AbstractPane.h"
#include <imgui/imgui.h>

namespace ImGui
{
	// ImGui::Begin for bitwize
	template<typename T>
	IMGUI_API bool Begin(const char* name, T* vContainer, T vFlag, ImGuiWindowFlags flags)
	{
		bool check = *vContainer & vFlag;
		const bool res = Begin(name, &check, flags);
		if (check) *vContainer = (T)(*vContainer | vFlag); // add
		else *vContainer = (T)(*vContainer & ~vFlag); // remove
		return res;
	}
}

class ProjectFile;
class LayoutManager : public conf::ConfigAbstract
{
private:
	ImGuiID m_DockSpaceID = 0;
	bool m_FirstLayout = false;
	bool m_FirstStart = true;
	char m_MenuLabel[PANE_NAME_BUFFER_SIZE + 1] = "";
	char m_DefaultMenuLabel[PANE_NAME_BUFFER_SIZE + 1] = "";

protected:
	std::map<PaneDisposal, AbstractPane*> m_PanesByDisposal;
	std::map<const char*, AbstractPane*> m_PanesByName;
	std::map<PaneFlags, AbstractPane*> m_PanesByFlag;
	
public:
	PaneFlags m_Pane_Focused_Default = 0;
	PaneFlags m_Pane_Opened_Default = 0;
	PaneFlags m_Pane_Shown = 0;
	PaneFlags m_Pane_Focused = 0;
	PaneFlags m_Pane_Hovered = 0;
	PaneFlags m_Pane_LastHovered = 0;
	ImVec2 m_LastSize;

public:
	void AddPane(
		AbstractPane*vPane,
		const char* vName,
		PaneFlags vFlag, 
		PaneDisposal vPaneDisposal, 
		bool vIsOpenedDefault, 
		bool vIsFocusedDefault);

public:
	void Init(const char* vMenuLabel, const char* vDefautlMenuLabel);
	void Unit();
	void InitAfterFirstDisplay(ImVec2 vSize);
	bool BeginDockSpace(ImGuiDockNodeFlags vFlags);
	void EndDockSpace();
	bool IsDockSpaceHoleHovered();

	void ApplyInitialDockingLayout(ImVec2 vSize = ImVec2(0, 0));

	virtual void DisplayMenu(ImVec2 vSize);
	virtual int DisplayPanes(int vWidgetId, std::string vUserDatas = "");
	virtual void DrawDialogsAndPopups(std::string vUserDatas = "");
	virtual int DrawWidgets(int vWidgetId, std::string vUserDatas = "");

	void ShowSpecificPane(PaneFlags vPane);
	void HideSpecificPane(PaneFlags vPane);
	void FocusSpecificPane(PaneFlags vPane);
	void FocusSpecificPane(const char* vlabel);
	void ShowAndFocusSpecificPane(PaneFlags vPane);
	bool IsSpecificPaneFocused(PaneFlags vPane);
	bool IsSpecificPaneFocused(const char* vlabel);
	void AddSpecificPaneToExisting(const char* vNewPane, const char* vExistingPane);

private: // configuration
	PaneFlags Internal_GetFocusedPanes();
	void Internal_SetFocusedPanes(PaneFlags vActivePanes);

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
	LayoutManager(const LayoutManager&) = delete;
	LayoutManager& operator =(const LayoutManager&) = delete;
	virtual ~LayoutManager(); // Prevent unwanted destruction
};

