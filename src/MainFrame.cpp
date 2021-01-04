// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#define IMGUIFONTSTUDIO_VERSION "Beta 0.7"

#include "MainFrame.h"

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <cctype>
#include <GLFW/glfw3.h>

#include <ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h>
#include <Generator/Generator.h>
#include <Gui/ImGuiWidgets.h>
#include <Panes/Manager/LayoutManager.h>
#include <Helper/ImGuiThemeHelper.h>
#include <Helper/Messaging.h>
#include <Helper/SelectionHelper.h>
#include <Helper/SettingsDlg.h>
#include <Panes/FinalFontPane.h>
#include <Panes/GeneratorPane.h>
#include <Panes/GlyphPane.h>
#include <Panes/SourceFontPane.h>
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>
#include <Res/CustomFont.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#define WIDGET_ID_MAGIC_NUMBER 4577
static int widgetId = WIDGET_ID_MAGIC_NUMBER;

MainFrame::MainFrame(GLFWwindow *vWin)
{
	m_Window = vWin;
}

MainFrame::~MainFrame()
{

}

void MainFrame::Init()
{
	char buf[256];
	snprintf(buf, 255, "ImGuiFontStudio %s", IMGUIFONTSTUDIO_VERSION);
	glfwSetWindowTitle(m_Window, buf);

	ImGuiThemeHelper::Instance()->ApplyStyleColorsDefault();
	
	LoadConfigFile("config.xml");

	LayoutManager::Instance()->Init();
}

void MainFrame::Unit()
{
	SaveConfigFile("config.xml");
}

void MainFrame::NewProject(const std::string& vFilePathName)
{
	m_ProjectFile.New(vFilePathName);
	SetAppTitle(vFilePathName);
}

void MainFrame::LoadProject(const std::string& vFilePathName)
{
	if (m_ProjectFile.LoadAs(vFilePathName))
	{
		SetAppTitle(vFilePathName);
		for (auto &it : m_ProjectFile.m_Fonts)
		{
			std::string absPath = m_ProjectFile.GetAbsolutePath(it.second.m_FontFilePathName);
			SourceFontPane::Instance()->OpenFont(&m_ProjectFile, absPath, false);
		}
		m_ProjectFile.UpdateCountSelectedGlyphs();
		m_ProjectFile.SetProjectChange(false);
	}
}

void MainFrame::SaveProject()
{
	m_ProjectFile.Save();
}

void MainFrame::SaveAsProject(const std::string& vFilePathName)
{
	m_ProjectFile.SaveAs(vFilePathName);

	if (m_NeedToCloseApp)
	{
		glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
	}
}

ProjectFile* MainFrame::GetProject()
{
	return &m_ProjectFile;
}

//////////////////////////////////////////////////////////////////////////////
//// DRAW ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define NewWidgetId() ++widgetId

void MainFrame::Display(ImVec2 vPos, ImVec2 vSize)
{
	widgetId = WIDGET_ID_MAGIC_NUMBER; // important for event catching on imgui widgets

	m_DisplayPos = vPos;
	m_DisplaySize = vSize;

	DrawDockPane(m_DisplayPos, m_DisplaySize);

	widgetId = LayoutManager::Instance()->DisplayPanes(&m_ProjectFile, widgetId);
	
	DisplayDialogsAndPopups();

	LayoutManager::Instance()->InitAfterFirstDisplay(m_DisplaySize);
}

void MainFrame::DrawDockPane(ImVec2 vPos, ImVec2 vSize)
{
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("MainDockPane", (bool*)0, window_flags);
	ImGui::PopStyleVar(3);

	float barH = 0.0f;
	auto curWin = ImGui::GetCurrentWindowRead();
	if (curWin)	barH = curWin->MenuBarHeight();
	ImGui::SetWindowSize(vSize - ImVec2(0.0f, barH));
	ImGui::SetWindowPos(vPos);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(ICON_IGFS_PROJECT " Project"))
		{
			if (ImGui::MenuItem(ICON_IGFS_FILE " New"))
			{
				m_ProjectFile.New();
			}

			if (ImGui::MenuItem(ICON_IGFS_FOLDER_OPEN " Open"))
			{
				// if change maybe save before open other
				if (m_ProjectFile.IsLoaded() && m_ProjectFile.IsThereAnyNotSavedChanged())
				{
					m_SaveDialogIfRequired = true;
					m_SaveChangeDialogActions.push_front([this]()
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("OpenProjectDlg", "Open Project File", "Project File{.ifs}", ".");
						m_SaveDialogIfRequired = false;
					});
				}
				else
				{
					igfd::ImGuiFileDialog::Instance()->OpenModal("OpenProjectDlg", "Open Project File", "Project File{.ifs}", ".");
				}
			}

			if (m_ProjectFile.IsLoaded())
			{
				ImGui::Separator();

				if (ImGui::MenuItem(ICON_IGFS_FOLDER_OPEN " Re Open"))
				{
					// if change maybe save before open other
					if (m_ProjectFile.IsLoaded() && m_ProjectFile.IsThereAnyNotSavedChanged())
					{
						m_SaveDialogIfRequired = true;
						m_SaveChangeDialogActions.push_front([this]()
							{
								LoadProject(m_ProjectFile.m_ProjectFilePathName);
								m_SaveDialogIfRequired = false;
							});
					}
					else
					{
						LoadProject(m_ProjectFile.m_ProjectFilePathName);
					}
				}
				
				ImGui::Separator();

				if (ImGui::MenuItem(ICON_IGFS_SAVE " Save"))
				{
					if (!m_ProjectFile.Save())
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("SaveProjectDlg", "Save Project File", "Project File{.ifs}", ".");
					}
				}

				if (ImGui::MenuItem(ICON_IGFS_SAVE " Save As"))
				{
					igfd::ImGuiFileDialog::Instance()->OpenModal("SaveProjectDlg", "Save Project File", "Project File{.ifs}", ".", 
						1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
				}

				ImGui::Separator();

				if (ImGui::MenuItem(ICON_IGFS_DESTROY " Close"))
				{
					// if change maybe save before close
					if (m_ProjectFile.IsThereAnyNotSavedChanged())
					{
						m_SaveDialogIfRequired = true;
						m_SaveChangeDialogActions.push_front([this]()
						{
							m_ProjectFile.Clear();
							m_SaveDialogIfRequired = false;
						});
					}
					else
					{
						m_ProjectFile.Clear();
					}
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem(ICON_IGFS_ABOUT " About"))
			{
				m_ShowAboutDialog = true;
			}
			
			ImGui::EndMenu();
		}
		
		ImGui::Spacing();

		LayoutManager::Instance()->DisplayMenu(vSize);

		ImGui::Spacing();
		
		if (ImGui::BeginMenu(ICON_IGFS_SETTINGS " Settings"))
		{
			/*if (ImGui::MenuItem("Settings"))
			{
				SettingsDlg::Instance()->OpenDialog();
			}*/

			if (ImGui::BeginMenu(ICON_IGFS_EDIT " Styles"))
			{
				ImGuiThemeHelper::Instance()->DrawMenu();

				ImGui::Separator();

				ImGui::MenuItem("Show ImGui", "", &m_ShowImGui);
				ImGui::MenuItem("Show ImGui Style", "", &m_ShowImGuiStyle);
				ImGui::MenuItem("Show ImGui Metric/Debug", "", &m_ShowMetric);

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (m_ProjectFile.IsThereAnyNotSavedChanged())
		{
			ImGui::Spacing(200.0f);

			if (ImGui::MenuItem(ICON_IGFS_SAVE " Save"))
			{
				if (!m_ProjectFile.Save())
				{
					igfd::ImGuiFileDialog::Instance()->OpenModal("SaveProjectDlg", "Save Project File", "Project File{.ifs}", ".", 0);
				}
			}
		}

		ImGui::EndMenuBar();
	}

	LayoutManager::Instance()->StartDockPane(dockspace_flags);

	ImGui::End();

	if (ImGui::BeginMainStatusBar())
	{
		Messaging::Instance()->Draw(&m_ProjectFile);

		ImGui::EndMainStatusBar();
	}
}

void MainFrame::DisplayDialogsAndPopups()
{
	ShowSaveDialogIfRequired();

	ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
	ImVec2 max = MainFrame::Instance()->m_DisplaySize;

	if (m_ProjectFile.IsLoaded())
	{
		LayoutManager::Instance()->DrawDialogsAndPopups(&m_ProjectFile);

		if (igfd::ImGuiFileDialog::Instance()->FileDialog("SolveBadFilePathName",
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
		{
			if (igfd::ImGuiFileDialog::Instance()->IsOk)
			{
				auto GoodFilePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
				auto UserDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());

				if (FileHelper::Instance()->IsFileExist(GoodFilePathName))
				{
					if (m_ProjectFile.m_Fonts.find(UserDatas) != m_ProjectFile.m_Fonts.end()) // found
					{
						ReRouteFontToFile(UserDatas, GoodFilePathName);
					}
				}
			}

			igfd::ImGuiFileDialog::Instance()->CloseDialog("SolveBadFilePathName");
		}
	}

	if (igfd::ImGuiFileDialog::Instance()->FileDialog("NewProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (igfd::ImGuiFileDialog::Instance()->IsOk)
		{
			NewProject(igfd::ImGuiFileDialog::Instance()->GetFilePathName());
		}

		igfd::ImGuiFileDialog::Instance()->CloseDialog("NewProjectDlg");
	}

	if (igfd::ImGuiFileDialog::Instance()->FileDialog("OpenProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (igfd::ImGuiFileDialog::Instance()->IsOk)
		{
			LoadProject(igfd::ImGuiFileDialog::Instance()->GetFilePathName());
		}

		igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenProjectDlg");
	}

	if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (igfd::ImGuiFileDialog::Instance()->IsOk)
		{
			auto file = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
			if (!FileHelper::Instance()->IsFileExist(file))
			{
				SaveAsProject(igfd::ImGuiFileDialog::Instance()->GetFilePathName());
			}
			else
			{

			}
		}

		igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveProjectDlg");
	}

	if (m_ShowAboutDialog)
		ShowAboutDialog(&m_ShowAboutDialog);
	if (m_ShowImGui)
		ImGui::ShowDemoWindow(&m_ShowImGui);
	if (m_ShowImGuiStyle)
		ImGui::ShowCustomStyleEditor(&m_ShowImGuiStyle);
	if (m_ShowMetric)
		ImGui::ShowMetricsWindow(&m_ShowMetric);

	//SettingsDlg::Instance()->DrawDialog();
}

void MainFrame::ShowAboutDialog(bool *vOpen)
{
	ImGui::Begin("About ImGuiFontStudio", vOpen,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

	// VERSION
	char buf[256];
	snprintf(buf, 255, "Version : ImGuiFontStudio %s", IMGUIFONTSTUDIO_VERSION);
	ImGui::ClickableTextUrl(buf, "https://github.com/aiekick/ImGuiFontStudio");

	ImGui::Separator();

	ImGui::Text("License :");
	ImGui::Indent();
	{
		ImGui::ClickableTextUrl(u8R"(Copyright 2020 Stephane Cuillerdier (aka Aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.)", "https://github.com/aiekick/ImGuiFontStudio/blob/master/LICENSE", false);
	}
	ImGui::Unindent();

	ImGui::Separator();

	ImGui::Text("My Accounts :");
	ImGui::Indent();
	{
		ImGui::ClickableTextUrl("Twitter", "https://twitter.com/aiekick");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("Instagram", "https://www.instagram.com/aiekick/");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("Sketchfab", "https://sketchfab.com/Aiekick");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("Github", "https://github.com/aiekick");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("ArtStation", "https://www.artstation.com/aiekick");
		ImGui::SameLine();
		ImGui::ClickableTextUrl("Shadertoy", "https://www.shadertoy.com/user/aiekick");
	}
	ImGui::Unindent();

	ImGui::Separator();

	ImGui::Text("Inspired by :");
	ImGui::Indent();
	{
		//IconFontCppHeaders
		ImGui::ClickableTextUrl("IconFontCppHeaders (Zlib)", "https://github.com/juliettef/IconFontCppHeaders");
		ImGui::SameLine(); ImGui::Text("by"); ImGui::SameLine();
		ImGui::ClickableTextUrl("Juliette Foucaut @juulcat", "https://twitter.com/juulcat");
	}
	ImGui::Unindent();

	ImGui::Separator();

	ImGui::Text("Frameworks / Libraries / Api's used :");
	ImGui::Indent();
	{
		//glfw3
		ImGui::ClickableTextUrl("Glfw (ZLIB)", "http://www.glfw.org/");
		//ImGui
		ImGui::ClickableTextUrl("Dear ImGui (Docking branch) (MIT)", "https://github.com/ocornut/imgui");
		ImGui::SameLine(); ImGui::Text("by"); ImGui::SameLine();
		ImGui::ClickableTextUrl("Omar Cornut @Ocornut", "https://twitter.com/ocornut");
		//glad
		ImGui::ClickableTextUrl("Glad (MIT)", "https://github.com/Dav1dde/glad");
		//stb
		ImGui::ClickableTextUrl("Stb (MIT)", "https://github.com/nothings/stb");
		ImGui::SameLine(); ImGui::Text("by"); ImGui::SameLine();
		ImGui::ClickableTextUrl("Sean Barrett @Nothings", "https://twitter.com/nothings");
		//tinyxml2
		ImGui::ClickableTextUrl("tinyxml2 (ZLIB)", "https://github.com/leethomason/tinyxml2");
		//dirent
		ImGui::ClickableTextUrl("dirent (MIT)", "https://github.com/tronkko/dirent/blob/master/include/dirent.h");
		//sfntly
		ImGui::ClickableTextUrl("sfntly (Apache 2.0)", "https://github.com/rillig/sfntly");
		//cTools
		ImGui::ClickableTextUrl("cTools (MIT)", "https://github.com/aiekick/cTools");
		//ImGuiFileDialog
		ImGui::ClickableTextUrl("ImGuiFileDialog (MIT)", "https://github.com/aiekick/ImGuiFileDialog");
	}
	ImGui::Unindent();

	ImGui::End();
}

void MainFrame::SetAppTitle(const std::string& vFilePathName)
{
	auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
	if (ps.isOk)
	{
		char bufTitle[1024];
		snprintf(bufTitle, 1023, "ImGuiFontStudio %s - Project : %s.ifs", IMGUIFONTSTUDIO_VERSION, ps.name.c_str());
		glfwSetWindowTitle(m_Window, bufTitle);
	}
}

///////////////////////////////////////////////////////
//// SAVE DIALOG WHEN APP CLOSED //////////////////////
///////////////////////////////////////////////////////

void MainFrame::ShowSaveDialogIfRequired()
{
	if (!m_SaveDialogIfRequired)
		return;

	if (m_ProjectFile.IsLoaded())
	{
		if (m_ProjectFile.IsThereAnyNotSavedChanged())
		{
			bool choiceMade = false;

			ImGui::OpenPopup("Do you want to save before ?");
			if (ImGui::BeginPopupModal("Do you want to save before ?", (bool*)0,
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
			{
				if (ImGui::Button("Save"))
				{
					choiceMade = true;
					m_SaveChangeDialogActions.push_front([this]()
					{
						SaveProject();
						m_SaveDialogIfRequired = false;
					});
					if (m_NeedToCloseApp)
					{
						m_SaveChangeDialogActions.push_front([this]()
						{
							glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
						});
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Save As"))
				{
					choiceMade = true;
					m_SaveChangeDialogActions.push_front([this]()
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("SaveProjectDlg", "Save Project File", "Project File{.ifs}", 
							".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
						m_SaveDialogIfRequired = false;
					});
				}
				
				if (ImGui::Button("Continue without saving"))
				{
					choiceMade = true;
					if (m_NeedToCloseApp)
					{
						m_SaveChangeDialogActions.push_front([this]()
						{
							glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
						});
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					choiceMade = true;
					m_SaveChangeDialogActions.clear();
				}

				ImGui::EndPopup();

				if (choiceMade)
				{
					if (!m_SaveChangeDialogActions.empty())
					{
						for (auto &action : m_SaveChangeDialogActions)
						{
							action();
						}
					}
					else
					{
						m_SaveDialogIfRequired = false;
					}
				}
			}
		}
	}
}

void MainFrame::ReRouteFontToFile(const std::string& vFontNameToReRoute, const std::string& vGoodFilePathName)
{
	auto ps = FileHelper::Instance()->ParsePathFileName(vGoodFilePathName);
	if (ps.isOk)
	{
		auto font = m_ProjectFile.m_Fonts[vFontNameToReRoute];
		font.m_FontFilePathName = m_ProjectFile.GetRelativePath(vGoodFilePathName);
		font.m_FontFileName = ps.name + "." + ps.ext;
		m_ProjectFile.m_Fonts[font.m_FontFileName] = font;
		if (font.m_FontFileName != vFontNameToReRoute)
			m_ProjectFile.m_Fonts.erase(vFontNameToReRoute);
		SourceFontPane::Instance()->OpenFont(&m_ProjectFile, font.m_FontFilePathName, true);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::IWantToCloseTheApp()
{
	// some changes to save before closing
	if (m_ProjectFile.IsLoaded() && m_ProjectFile.IsThereAnyNotSavedChanged())
	{
		// force close dialog
		igfd::ImGuiFileDialog::Instance()->CloseDialog();

		m_SaveDialogIfRequired = true;
		m_NeedToCloseApp = true;
	}
	else
	{
		glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainFrame::getXml(const std::string& vOffset)
{
	std::string str;

	str += ImGuiThemeHelper::Instance()->getXml(vOffset);
	str += LayoutManager::Instance()->getXml(vOffset);
	str += vOffset + "<bookmarks>" + igfd::ImGuiFileDialog::Instance()->SerializeBookmarks() + "</bookmarks>\n";
	str += vOffset + "<showaboutdialog>" + (m_ShowAboutDialog ? "true" : "false") + "</showaboutdialog>\n";
	str += vOffset + "<showimgui>" + (m_ShowImGui ? "true" : "false") + "</showimgui>\n";
	str += vOffset + "<showmetric>" + (m_ShowMetric ? "true" : "false") + "</showmetric>\n";
	str += vOffset + "<showimguistyle>" + (m_ShowImGuiStyle ? "true" : "false") + "</showimguistyle>\n";
	str += vOffset + "<project>" + m_ProjectFile.m_ProjectFilePathName + "</project>\n";
	
	return str;
}

bool MainFrame::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != 0)
		strParentName = vParent->Value();

	ImGuiThemeHelper::Instance()->setFromXml(vElem, vParent);
	LayoutManager::Instance()->setFromXml(vElem, vParent);

	if (strName == "bookmarks")
		igfd::ImGuiFileDialog::Instance()->DeserializeBookmarks(strValue);
	else if (strName == "project")
		LoadProject(strValue);
	else if (strName == "showaboutdialog")
		m_ShowAboutDialog = ct::ivariant(strValue).GetB();
	else if (strName == "showimgui")
		m_ShowImGui = ct::ivariant(strValue).GetB();
	else if (strName == "showmetric")
		m_ShowMetric = ct::ivariant(strValue).GetB();
	else if (strName == "showimguistyle")
		m_ShowImGuiStyle = ct::ivariant(strValue).GetB();

	return true;
}