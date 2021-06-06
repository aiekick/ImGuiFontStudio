// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "LayoutManager.h"

#include <ctools/FileHelper.h>
#include <ctools/Logger.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

LayoutManager::LayoutManager() = default;
LayoutManager::~LayoutManager() = default;

void LayoutManager::AddPane(
	AbstractPane *vPane,
	const char* vName,
	PaneFlags vFlag,
	PaneDisposal vPaneDisposal,
	bool vIsOpenedDefault, 
	bool vIsFocusedDefault)
{
	assert(vFlag); // flag != 0
	assert(vPane); // vPane != 0
	assert(vName && strlen(vName)); // vPane->m_PaneName not nullptr
	assert(m_PanesByName.find(vName) == m_PanesByName.end()); // pane name not already exist
	assert(m_PanesByFlag.find(vFlag) == m_PanesByFlag.end()); // pane flag not already exist
	
	strncpy(vPane->m_PaneName, vName, ct::mini((size_t)PANE_NAME_BUFFER_SIZE, strlen(vName)));
	vPane->m_PaneFlag = vFlag;
	vPane->m_PaneDisposal = vPaneDisposal;
	vPane->m_OpenedDefault = vIsOpenedDefault;
	vPane->m_FocusedDefault = vIsFocusedDefault;
	if (vIsOpenedDefault)
		m_Pane_Opened_Default |= vPane->m_PaneFlag;
	if (vIsFocusedDefault)
		m_Pane_Focused_Default |= vPane->m_PaneFlag;
	m_PanesByDisposal[vPane->m_PaneDisposal] = vPane;
	m_PanesByName[vPane->m_PaneName] = vPane;
	m_PanesByFlag[vPane->m_PaneFlag] = vPane;
}

void LayoutManager::Init(const char* vMenuLabel, const char* vDefautlMenuLabel)
{
	assert(vMenuLabel);
	assert(vDefautlMenuLabel);

	strncpy(m_MenuLabel, vMenuLabel, ct::mini((size_t)PANE_NAME_BUFFER_SIZE, strlen(vMenuLabel)));
	strncpy(m_DefaultMenuLabel, vDefautlMenuLabel, ct::mini((size_t)PANE_NAME_BUFFER_SIZE, strlen(vDefautlMenuLabel)));

	if (!FileHelper::Instance()->IsFileExist("imgui.ini"))
	{
		m_FirstLayout = true; // need default layout
		LogVar("We will apply default layout :)");
	}
}

void LayoutManager::Unit()
{
	for (const auto& pane : m_PanesByFlag)
	{
		pane.second->Unit();
	}
}

void LayoutManager::InitAfterFirstDisplay(ImVec2 vSize)
{
	if (m_FirstLayout)
	{
		ApplyInitialDockingLayout(vSize);
		m_FirstLayout = false;
	}

	if (m_FirstStart)
	{
		// focus after start of panes
		Internal_SetFocusedPanes(m_Pane_Focused);
		m_FirstStart = false;
	}
}

bool LayoutManager::BeginDockSpace(ImGuiDockNodeFlags vFlags)
{
	const auto viewport = ImGui::GetMainViewport();

	m_LastSize = viewport->Size;

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	auto host_window_flags = 0;
	host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
	host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	if (vFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		host_window_flags |= ImGuiWindowFlags_NoBackground;

	char label[100 + 1];
	ImFormatString(label, 100, "DockSpaceViewport_%08X", viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	const auto res = ImGui::Begin(label, nullptr, host_window_flags);
	ImGui::PopStyleVar(3);

	m_DockSpaceID = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(m_DockSpaceID, ImVec2(0.0f, 0.0f), vFlags);

	return res;
}

void LayoutManager::EndDockSpace()
{
	ImGui::End();
}

bool LayoutManager::IsDockSpaceHoleHovered()
{
	auto& g = *GImGui;
	return g.HoveredDockNode == nullptr && g.HoveredWindow == nullptr;
}

void LayoutManager::ApplyInitialDockingLayout(ImVec2 vSize)
{
	auto size = ImVec2((float)vSize.x, (float)vSize.y);

#define IS_FLOAT_EQUAL(a,b) (fabs((a) - (b)) < FLT_EPSILON)

	if (IS_FLOAT_EQUAL(size.x, 0.0) || IS_FLOAT_EQUAL(size.y, 0.0))
	{
		size = m_LastSize;

		if (IS_FLOAT_EQUAL(m_LastSize.x, 0.0) || IS_FLOAT_EQUAL(m_LastSize.y, 0.0))
		{
			return;
		}
	}

#undef IS_FLOAT_EQUAL

	ImGui::DockBuilderRemoveNode(m_DockSpaceID); // Clear out existing layout
	ImGui::DockBuilderAddNode(m_DockSpaceID, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImGui::DockBuilderSetNodeSize(m_DockSpaceID, vSize);

	const auto leftPaneDefaultWidth = 200.0f;
	const auto rightPaneDefaultWidth = 200.0f;
	const auto bottomPaneDefaultWidth = 200.0f;
	const auto topPaneDefaultWidth = 200.0f;

	auto dockMainID = m_DockSpaceID; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
	const auto dockLeftID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, leftPaneDefaultWidth / vSize.x, nullptr, &dockMainID);
	const auto dockRightID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, rightPaneDefaultWidth / vSize.x, nullptr, &dockMainID);
	const auto dockBottomID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, bottomPaneDefaultWidth / vSize.y, nullptr, &dockMainID);
	const auto dockTopID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Up, topPaneDefaultWidth / vSize.y, nullptr, &dockMainID);

	for (const auto& pane : m_PanesByName)
	{
		switch (pane.second->m_PaneDisposal)
		{
			case PaneDisposal::CENTRAL:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockMainID);
				break;
			}
			case PaneDisposal::LEFT:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockLeftID);
				break;
			}
			case PaneDisposal::RIGHT:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockRightID);
				break;
			}
			case PaneDisposal::BOTTOM:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockBottomID);
				break;
			}
			case PaneDisposal::TOP:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockTopID);
				break;
			}
		};
	}
	
	ImGui::DockBuilderFinish(m_DockSpaceID);

	m_Pane_Shown = m_Pane_Opened_Default; // will show when pane will be passed
	m_Pane_Focused = m_Pane_Focused_Default;

	Internal_SetFocusedPanes(m_Pane_Focused);
}

template<typename T>
static bool LayoutManager_MenuItem(const char* label, const char* shortcut, T* vContainer, T vFlag, bool vOnlyOneSameTime = false)
{
	bool selected = *vContainer & vFlag;
	const bool res = ImGui::MenuItem(label, shortcut, &selected, true);
	if (res) 
	{
		if (selected) 
		{
			if (vOnlyOneSameTime) 
				*vContainer = vFlag; // set
			else 
				*vContainer = (T)(*vContainer | vFlag);// add
		}
		else if (!vOnlyOneSameTime) 
				*vContainer = (T)(*vContainer & ~vFlag); // remove
	}
	return res;
}

void LayoutManager::DisplayMenu(ImVec2 vSize)
{
	if (ImGui::BeginMenu(m_MenuLabel))
	{
		if (ImGui::MenuItem(m_DefaultMenuLabel))
		{
			ApplyInitialDockingLayout(vSize);
		}

		ImGui::Separator();

		static char buffer[100 + 1] = "\0";
		for (const auto& pane : m_PanesByName)
		{
			snprintf(buffer, 100, "%s Pane", pane.first);
			LayoutManager_MenuItem<PaneFlags>(buffer, "", &m_Pane_Shown, pane.second->m_PaneFlag);
		}
		
		ImGui::EndMenu();
	}
}

int LayoutManager::DisplayPanes(int vWidgetId, std::string vUserDatas)
{
	for (const auto& pane : m_PanesByFlag)
	{
		vWidgetId = pane.second->DrawPanes(vWidgetId, vUserDatas);
	}

	return vWidgetId;
}

void LayoutManager::DrawDialogsAndPopups(std::string vUserDatas)
{
	for (const auto& pane : m_PanesByFlag)
	{
		pane.second->DrawDialogsAndPopups(vUserDatas);
	}
}

int LayoutManager::DrawWidgets(int vWidgetId, std::string vUserDatas)
{
	for (const auto& pane : m_PanesByFlag)
	{
		vWidgetId = pane.second->DrawWidgets(vWidgetId, vUserDatas);
	}

	return vWidgetId;
}

void LayoutManager::ShowSpecificPane(PaneFlags vPane)
{
	m_Pane_Shown = (PaneFlags)((int32_t)m_Pane_Shown | (int32_t)vPane);
}

void LayoutManager::HideSpecificPane(PaneFlags vPane)
{
	m_Pane_Shown = (PaneFlags)((int32_t)m_Pane_Shown & ~(int32_t)vPane);
}

void LayoutManager::FocusSpecificPane(PaneFlags vPane)
{
	ShowSpecificPane(vPane);

	if (m_PanesByFlag.find(vPane) != m_PanesByFlag.end())
	{
		FocusSpecificPane(m_PanesByFlag[vPane]->m_PaneName);
	}
}

void LayoutManager::ShowAndFocusSpecificPane(PaneFlags vPane)
{
	ShowSpecificPane(vPane);
	FocusSpecificPane(vPane);
}

bool LayoutManager::IsSpecificPaneFocused(PaneFlags vPane)
{
	if (m_PanesByFlag.find(vPane) != m_PanesByFlag.end())
	{
		return IsSpecificPaneFocused(m_PanesByFlag[vPane]->m_PaneName);
	}

	return false;
}

void LayoutManager::AddSpecificPaneToExisting(const char* vNewPane, const char* vExistingPane)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vExistingPane);
	if (window)
	{
		auto dockid = window->DockId;
		ImGui::DockBuilderDockWindow(vNewPane, dockid);
	}
}

///////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////
///////////////////////////////////////////////////////

bool LayoutManager::IsSpecificPaneFocused(const char *vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel);
	if (window)
	{
		return 
			window->DockTabIsVisible || 
			window->ViewportOwned;
	}
	return false;
}

void LayoutManager::FocusSpecificPane(const char *vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel);
	if (window)
	{
		if(!window->DockTabIsVisible)
			ImGui::FocusWindow(window);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION PRIVATE ////////////////////////////
///////////////////////////////////////////////////////

PaneFlags LayoutManager::Internal_GetFocusedPanes()
{
	PaneFlags flag = 0;

	for (const auto& pane : m_PanesByFlag)
	{
		if (IsSpecificPaneFocused(pane.second->m_PaneName))
			flag = (PaneFlags)((int32_t)flag | (int32_t)pane.first);
	}

	return flag;
}

void LayoutManager::Internal_SetFocusedPanes(PaneFlags vActivePanes)
{
	for (const auto& pane : m_PanesByFlag)
	{
		if (vActivePanes & pane.first)
			FocusSpecificPane(pane.second->m_PaneName);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION PUBLIC /////////////////////////////
///////////////////////////////////////////////////////

std::string LayoutManager::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	if (vUserDatas == "app")
	{
		str += vOffset + "<layout>\n";
		m_Pane_Focused = Internal_GetFocusedPanes();
		str += vOffset + "\t<panes opened=\"" + ct::ivariant((int32_t)m_Pane_Shown).GetS() + "\" active=\"" + ct::ivariant((int32_t)m_Pane_Focused).GetS() + "\"/>\n";
		str += vOffset + "</layout>\n";
	}
	else if (vUserDatas == "project")
	{
		// per pane settings
	}

	return str;
}

bool LayoutManager::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	// The value of this child identifies the name of this element
	std::string strName = "";
	std::string strValue = "";
	std::string strParentName = "";

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != 0)
		strParentName = vParent->Value();

	if (vUserDatas == "app")
	{
		if (strParentName == "layout")
		{
			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "opened")
					m_Pane_Shown = (PaneFlags)ct::ivariant(attValue).GetI();
				if (attName == "active")
					m_Pane_Focused = (PaneFlags)ct::ivariant(attValue).GetI();
			}
		}
	}
	else if (vUserDatas == "project")
	{

	}

	return true;
}