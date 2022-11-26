// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "LayoutManager.h"

#include <ctools/FileHelper.h>
#include <ctools/Logger.h>

#include <imgui/imgui_internal.h>

LayoutManager::LayoutManager() = default;
LayoutManager::~LayoutManager() = default;

void LayoutManager::AddPane(
	AbstractPaneWeak vPane,
	const std::string& vName,
	const PaneCategoryName& vCategory,
	const PaneDisposal& vPaneDisposal,
	const bool& vIsOpenedDefault,
	const bool& vIsFocusedDefault)
{
	PaneFlags flag = (1 << ++m_FlagCount);
	AddPane(vPane, vName, vCategory, flag, vPaneDisposal, vIsOpenedDefault, vIsFocusedDefault);
}

void LayoutManager::AddPane(
	AbstractPaneWeak vPane,
	const std::string& vName,
	const PaneCategoryName& vCategory,
	const PaneFlags& vFlag,
	const PaneDisposal& vPaneDisposal,
	const bool& vIsOpenedDefault,
	const bool& vIsFocusedDefault)
{
	if (vFlag == 0) return;
	if (vPane.expired()) return;
	if (vName.empty()) return;
	if (m_PanesByName.find(vName) != m_PanesByName.end()) return; // pane name not already exist
	if (m_PanesByFlag.find(vFlag) != m_PanesByFlag.end()) return; // pane flag not already exist
	
	auto panePtr = vPane.lock();
	if (panePtr)
	{
		panePtr->m_PaneName = vName;
		panePtr->m_PaneFlag = vFlag;
		panePtr->m_PaneDisposal = vPaneDisposal;
		panePtr->m_OpenedDefault = vIsOpenedDefault;
		panePtr->m_FocusedDefault = vIsFocusedDefault;
		if (vIsOpenedDefault)
			m_Pane_Opened_Default |= panePtr->m_PaneFlag;
		if (vIsFocusedDefault)
			m_Pane_Focused_Default |= panePtr->m_PaneFlag;
		m_PanesByDisposal[panePtr->m_PaneDisposal] = panePtr;
		m_PanesByName[panePtr->m_PaneName] = panePtr;
		m_PanesByFlag[panePtr->m_PaneFlag] = panePtr;
		m_PanesInDisplayOrder[vCategory].push_back(panePtr);
	}
}

void LayoutManager::SetPaneDisposalSize(const PaneDisposal& vPaneDisposal, const float& vSize)
{
	if (vPaneDisposal == PaneDisposal::CENTRAL ||
		vPaneDisposal == PaneDisposal::Count)
		return;

	m_PaneDisposalSizes[(int)vPaneDisposal] = vSize;
}

void LayoutManager::Init(const std::string& vMenuLabel, const std::string& vDefaultMenuLabel)
{
	assert(!vMenuLabel.empty());
	assert(!vDefaultMenuLabel.empty());

	m_MenuLabel = vMenuLabel;
	m_DefaultMenuLabel = vDefaultMenuLabel;

	if (!FileHelper::Instance()->IsFileExist("imgui.ini"))
	{
		m_FirstLayout = true; // need default layout
		LogVarDebug("We will apply default layout :)");
	}
}

void LayoutManager::Unit()
{
	m_PanesByDisposal.clear();
	m_PanesByName.clear();
	m_PanesInDisplayOrder.clear();
	m_PanesByFlag.clear();
}

bool LayoutManager::InitPanes()
{
	bool res = true;

	for (const auto& pane : m_PanesByFlag)
	{
		auto panePtr = pane.second.lock();
		if (panePtr)
		{
			res &= panePtr->Init();
		}
	}

	return res;
}

void LayoutManager::UnitPanes()
{
	for (const auto& pane : m_PanesByFlag)
	{
		auto panePtr = pane.second.lock();
		if (panePtr)
		{
			panePtr->Unit();
		}
	}
}

void LayoutManager::InitAfterFirstDisplay(const ImVec2& vSize)
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

bool LayoutManager::BeginDockSpace(const ImGuiDockNodeFlags& vFlags)
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
	return g.DebugHoveredDockNode == nullptr && g.HoveredWindow == nullptr;
}

void LayoutManager::ApplyInitialDockingLayout(const ImVec2& vSize)
{
	ImVec2 _size = vSize;

	if (IS_FLOAT_EQUAL(_size.x, 0.0f) || IS_FLOAT_EQUAL(_size.y, 0.0f))
	{
		if (IS_FLOAT_EQUAL(m_LastSize.x, 0.0f) || IS_FLOAT_EQUAL(m_LastSize.y, 0.0f))
		{
			return;
		}

		_size = m_LastSize;
	}

	ImGui::DockBuilderRemoveNode(m_DockSpaceID); // Clear out existing layout
	ImGui::DockBuilderAddNode(m_DockSpaceID, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImGui::DockBuilderSetNodeSize(m_DockSpaceID, _size);

	// just for readability
	const auto& left_size = m_PaneDisposalSizes[1];
	const auto& right_size = m_PaneDisposalSizes[2];
	const auto& bottom_size = m_PaneDisposalSizes[3];
	const auto& top_size = m_PaneDisposalSizes[4];

	auto dockMainID = m_DockSpaceID; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
	const auto dockLeftID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, left_size / _size.x, nullptr, &dockMainID);
	const auto dockRightID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, right_size / (_size.x - left_size), nullptr, &dockMainID);
	const auto dockBottomID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, bottom_size / _size.y, nullptr, &dockMainID);
	const auto dockTopID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Up, top_size / (_size.y - bottom_size), nullptr, &dockMainID);

	for (const auto& pane : m_PanesByName)
	{
		auto panePtr = pane.second.lock();
		if (panePtr)
		{
			switch (panePtr->m_PaneDisposal)
			{
			case PaneDisposal::CENTRAL:
			{
				ImGui::DockBuilderDockWindow(pane.first.c_str(), dockMainID);
				break;
			}
			case PaneDisposal::LEFT:
			{
				ImGui::DockBuilderDockWindow(pane.first.c_str(), dockLeftID);
				break;
			}
			case PaneDisposal::RIGHT:
			{
				ImGui::DockBuilderDockWindow(pane.first.c_str(), dockRightID);
				break;
			}
			case PaneDisposal::BOTTOM:
			{
				ImGui::DockBuilderDockWindow(pane.first.c_str(), dockBottomID);
				break;
			}
			case PaneDisposal::TOP:
			{
				ImGui::DockBuilderDockWindow(pane.first.c_str(), dockTopID);
				break;
			}
			};
		}
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

void LayoutManager::DisplayMenu(const ImVec2& vSize)
{
	if (ImGui::BeginMenu(m_MenuLabel.c_str()))
	{
		if (ImGui::MenuItem(m_DefaultMenuLabel.c_str()))
		{
			ApplyInitialDockingLayout(vSize);
		}

		ImGui::Separator();

		bool _menuOpened = false;
		for (const auto& paneCategory : m_PanesInDisplayOrder)
		{
			_menuOpened = false;

			if (!paneCategory.first.empty())
			{
				_menuOpened = ImGui::BeginMenu(paneCategory.first.c_str());
			}

			if (paneCategory.first.empty() || _menuOpened)
			{
				for (auto pane : paneCategory.second)
				{
					auto panePtr = pane.lock();
					if (panePtr && panePtr->CanWeDisplay())
					{
						LayoutManager_MenuItem<PaneFlags>(panePtr->m_PaneName.c_str(), "", &m_Pane_Shown, panePtr->m_PaneFlag);
					}
				}
			}
			
			if (_menuOpened)
			{
				ImGui::EndMenu();
			}
		}
		
		ImGui::EndMenu();
	}
}

int LayoutManager::DisplayPanes(const uint32_t& vCurrentFrame, const int& vWidgetId, const std::string& vUserDatas)
{
	auto widgetId = vWidgetId;

	for (const auto& pane : m_PanesByFlag)
	{
		auto panePtr = pane.second.lock();
		if (panePtr && panePtr->CanWeDisplay())
		{
			if (panePtr->m_ShowPaneAtFirstCall)
			{
				ShowSpecificPane(panePtr->m_PaneFlag);
				panePtr->m_ShowPaneAtFirstCall = false;
			}

			if (panePtr->m_HidePaneAtFirstCall)
			{
				HideSpecificPane(panePtr->m_PaneFlag);
				panePtr->m_HidePaneAtFirstCall = false;
			}

			widgetId = panePtr->DrawPanes(vCurrentFrame, widgetId, vUserDatas, m_Pane_Shown);
		}
	}

	return widgetId;
}

void LayoutManager::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const std::string& vUserDatas)
{
	for (const auto& pane : m_PanesByFlag)
	{
		auto panePtr = pane.second.lock();
		if (panePtr && panePtr->CanWeDisplay())
		{
			panePtr->DrawDialogsAndPopups(vCurrentFrame, vUserDatas);
		}
	}
}

int LayoutManager::DrawWidgets(const uint32_t& vCurrentFrame, const int& vWidgetId, const std::string& vUserDatas)
{
	auto widgetId = vWidgetId;

	for (const auto& pane : m_PanesByFlag)
	{
		auto panePtr = pane.second.lock();
		if (panePtr && panePtr->CanWeDisplay())
		{
			widgetId = panePtr->DrawWidgets(vCurrentFrame, widgetId, vUserDatas);
		}
	}

	return widgetId;
}

void LayoutManager::ShowSpecificPane(const PaneFlags& vPane)
{
	m_Pane_Shown = (PaneFlags)((int32_t)m_Pane_Shown | (int32_t)vPane);
}

void LayoutManager::HideSpecificPane(const PaneFlags& vPane)
{
	m_Pane_Shown = (PaneFlags)((int32_t)m_Pane_Shown & ~(int32_t)vPane);
}

void LayoutManager::FocusSpecificPane(const PaneFlags& vPane)
{
	ShowSpecificPane(vPane);

	if (m_PanesByFlag.find(vPane) != m_PanesByFlag.end())
	{
		auto panePtr = m_PanesByFlag.at(vPane).lock();
		if (panePtr)
		{
			FocusSpecificPane(panePtr->m_PaneName);
		}
	}
}

void LayoutManager::ShowAndFocusSpecificPane(const PaneFlags& vPane)
{
	ShowSpecificPane(vPane);
	FocusSpecificPane(vPane);
}

bool LayoutManager::IsSpecificPaneFocused(const PaneFlags& vPane)
{
	if (m_PanesByFlag.find(vPane) != m_PanesByFlag.end())
	{
		auto panePtr = m_PanesByFlag.at(vPane).lock();
		if (panePtr)
		{
			return IsSpecificPaneFocused(panePtr->m_PaneName);
		}
	}

	return false;
}

void LayoutManager::AddSpecificPaneToExisting(const std::string& vNewPane, const std::string& vExistingPane)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vExistingPane.c_str());
	if (window)
	{
		auto dockid = window->DockId;
		ImGui::DockBuilderDockWindow(vNewPane.c_str(), dockid);
	}
}

///////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////
///////////////////////////////////////////////////////

bool LayoutManager::IsSpecificPaneFocused(const std::string& vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel.c_str());
	if (window)
	{
		return 
			window->DockTabIsVisible || 
			window->ViewportOwned;
	}
	return false;
}

void LayoutManager::FocusSpecificPane(const std::string& vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel.c_str());
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
		auto panePtr = pane.second.lock();
		if (panePtr && IsSpecificPaneFocused(panePtr->m_PaneName))
			flag = (PaneFlags)((int32_t)flag | (int32_t)pane.first);
	}

	return flag;
}

void LayoutManager::Internal_SetFocusedPanes(const PaneFlags& vActivePanes)
{
	for (const auto& pane : m_PanesByFlag)
	{
		auto panePtr = pane.second.lock();
		if (panePtr && vActivePanes & pane.first)
			FocusSpecificPane(panePtr->m_PaneName);
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