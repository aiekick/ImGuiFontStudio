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

#define IMGUIFONTSTUDIO_VERSION "Beta 0.8"

#include "MainFrame.h"

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <cctype>
#include <GLFW/glfw3.h>

#include <ImGuiFileDialog/ImGuiFileDialog.h>
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
#include <Panes/ParamsPane.h>
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>
#include <Res/CustomFont.h>
#include <Helper/AssetManager.h>

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
#ifdef USE_SHADOW
	AssetManager::Instance()->Clear();
#endif
}

void MainFrame::Init()
{
	char buf[256];
	snprintf(buf, 255, "ImGuiFontStudio %s", IMGUIFONTSTUDIO_VERSION);
	glfwSetWindowTitle(m_Window, buf);

	ImGuiThemeHelper::Instance()->ApplyStyleColorsDefault();
	
	LoadConfigFile("config.xml");

	LayoutManager::Instance()->Init();

#ifdef USE_RIBBONBAR
	m_RibbonBar.Init();
#endif

#ifdef USE_SHADOW
	AssetManager::Instance()->LoadTexture2D("btn", "src/res/btn.png");
#endif
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
		for (auto it : m_ProjectFile.m_Fonts)
		{
			std::string absPath = m_ProjectFile.GetAbsolutePath(it.second->m_FontFilePathName);
			ParamsPane::Instance()->OpenFont(&m_ProjectFile, absPath, false);
		}
		m_ProjectFile.UpdateCountSelectedGlyphs();
		m_ProjectFile.SetProjectChange(false);
	}
	else
	{
		Messaging::Instance()->AddError(true, nullptr, nullptr,
			"Failed to load project %s", vFilePathName.c_str());
	}
}

bool MainFrame::SaveProject()
{
	return m_ProjectFile.Save();
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

void MainFrame::OpenAboutDialog()
{
	m_ShowAboutDialog = true;
}

void MainFrame::DrawDockPane(ImVec2 vPos, ImVec2 vSize)
{
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoTitleBar |
#ifndef USE_RIBBONBAR
		ImGuiWindowFlags_MenuBar |
#endif
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

#ifndef USE_RIBBONBAR
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(ICON_IGFS_PROJECT " Project"))
		{
			if (ImGui::MenuItem(ICON_IGFS_FILE " New"))
			{
				Action_Menu_NewProject();
			}

			if (ImGui::MenuItem(ICON_IGFS_FOLDER_OPEN " Open"))
			{
				Action_Menu_OpenProject();
			}

			if (m_ProjectFile.IsLoaded())
			{
				ImGui::Separator();

				if (ImGui::MenuItem(ICON_IGFS_FOLDER_OPEN " Re Open"))
				{
					Action_Menu_ReOpenProject();
				}
				
				ImGui::Separator();

				if (ImGui::MenuItem(ICON_IGFS_SAVE " Save"))
				{
					Action_Menu_SaveProject();
				}

				if (ImGui::MenuItem(ICON_IGFS_SAVE " Save As"))
				{
					Action_Menu_SaveAsProject();
				}

				ImGui::Separator();

				if (ImGui::MenuItem(ICON_IGFS_DESTROY " Close"))
				{
					Action_Menu_CloseProject();
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
			if (ImGui::MenuItem("Settings"))
			{
				SettingsDlg::Instance()->OpenDialog();
			}

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
				Action_Menu_SaveProject();
			}
		}

		ImGui::EndMenuBar();
	}
#else
	m_RibbonBar.Draw(&m_ProjectFile);
#endif

	LayoutManager::Instance()->StartDockPane(dockspace_flags, vSize);

	ImGui::End();

	if (ImGui::BeginMainStatusBar())
	{
		Messaging::Instance()->Draw(&m_ProjectFile);

		// ImGui Infos
		ImGuiIO io = ImGui::GetIO();
		std::string fps = ct::toStr("Dear ImGui  %s (Docking Branch) - %.1f ms/frame (%.1f fps)", ImGui::GetVersion(), 1000.0f / io.Framerate, io.Framerate);
		ImVec2 size = ImGui::CalcTextSize(fps.c_str());
		ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
		ImGui::Text("%s", fps.c_str());

		ImGui::EndMainStatusBar();
	}
}

void MainFrame::DisplayDialogsAndPopups()
{
	m_ActionSystem.RunActions();

	if (m_ProjectFile.IsLoaded())
	{
		LayoutManager::Instance()->DrawDialogsAndPopups(&m_ProjectFile);

		ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
		ImVec2 max = MainFrame::Instance()->m_DisplaySize;
		
		if (ImGuiFileDialog::Instance()->Display("SolveBadFilePathName",
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				auto GoodFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				auto UserDatas = std::string((const char*)ImGuiFileDialog::Instance()->GetUserDatas());

				if (FileHelper::Instance()->IsFileExist(GoodFilePathName))
				{
					if (m_ProjectFile.m_Fonts.find(UserDatas) != m_ProjectFile.m_Fonts.end()) // found
					{
						ReRouteFontToFile(UserDatas, GoodFilePathName);
					}
				}
			}

			ImGuiFileDialog::Instance()->Close();
		}
	}

	if (m_ShowAboutDialog)
		ShowAboutDialog(&m_ShowAboutDialog);
	if (m_ShowImGui)
		ImGui::ShowDemoWindow(&m_ShowImGui);
	if (m_ShowImGuiStyle)
		ImGuiThemeHelper::Instance()->ShowCustomStyleEditor(&m_ShowImGuiStyle);
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
		//Freetype2
		ImGui::ClickableTextUrl("FreeType2 (FTL)", "https://github.com/freetype/freetype2");
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
//// SAVE DIALOG WHEN UN SAVED CHANGES ////////////////
///////////////////////////////////////////////////////

void MainFrame::OpenUnSavedDialog()
{
	// force close dialog if any dialog is opened
	ImGuiFileDialog::Instance()->Close();

	m_SaveDialogIfRequired = true;
}
void MainFrame::CloseUnSavedDialog()
{
	m_SaveDialogIfRequired = false;
}

bool MainFrame::ShowUnSavedDialog()
{
	bool res = false;

	if (m_SaveDialogIfRequired)
	{
		if (m_ProjectFile.IsLoaded())
		{
			if (m_ProjectFile.IsThereAnyNotSavedChanged())
			{
				/*
				Unsaved dialog behavior :
				-	save :
					-	insert action : save project
				-	save as :
					-	insert action : save as project
				-	continue without saving :
					-	quit unsaved dialog
				-	cancel :
					-	clear actions
				*/

				ImGui::CloseCurrentPopup();
				ImGui::OpenPopup("Do you want to save before ?");
				if (ImGui::BeginPopupModal("Do you want to save before ?", (bool*)0,
					ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
				{
					if (ImGui::Button("Save"))
					{
						res = Action_UnSavedDialog_SaveProject();
					}
					ImGui::SameLine();
					if (ImGui::Button("Save As"))
					{
						Action_UnSavedDialog_SaveAsProject();
					}

					if (ImGui::Button("Continue without saving"))
					{
						res = true; // quit the action
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						Action_Cancel();
					}

					ImGui::EndPopup();
				}
			}
		}

		return res; // quit if true, else continue on the next frame
	}

	return true; // quit the action
}

///////////////////////////////////////////////////////
//// ACTIONS //////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::Action_Menu_NewProject()
{
/*
new project :
-	unsaved :
	-	add action : show unsaved dialog
	-	add action : new project
-	saved :
	-	add action : new project
*/
	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			m_ProjectFile.New();
			return true; // one time action
		});
}

void MainFrame::Action_Menu_OpenProject()
{
/*
open project : 
-	unsaved :
	-	add action : show unsaved dialog
	-	add action : open project
-	saved :
	-	add action : open project
*/
	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			CloseUnSavedDialog();
			ImGuiFileDialog::Instance()->OpenModal(
				"OpenProjectDlg", "Open Project File", "Project File{.ifs}", ".");
			return true;
		});
	m_ActionSystem.Add([this]()
		{
			return Display_OpenProjectDialog();
		});
}

void MainFrame::Action_Menu_ReOpenProject()
{
/*
re open project : 
-	unsaved :
	-	add action : show unsaved dialog
	-	add action : re open project
-	saved :
	-	add action : re open project
*/
	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			LoadProject(m_ProjectFile.m_ProjectFilePathName);
			return true;
		});
}

void MainFrame::Action_Menu_SaveProject()
{
/*
save project :
-	never saved :
	-	add action : save as project
-	saved in a file beofre :
	-	add action : save project
*/
	m_ActionSystem.Clear();
	m_ActionSystem.Add([this]()
		{
			if (!SaveProject())
			{
				CloseUnSavedDialog();
				ImGuiFileDialog::Instance()->OpenModal(
					"SaveProjectDlg", "Save Project File", "Project File{.ifs}", ".");
			}
			return true;
		});
	m_ActionSystem.Add([this]()
		{
			return Display_SaveProjectDialog();
		});
}

void MainFrame::Action_Menu_SaveAsProject()
{
/*
save as project :
-	add action : save as project
*/
	m_ActionSystem.Clear();
	m_ActionSystem.Add([this]()
		{
			CloseUnSavedDialog();
			ImGuiFileDialog::Instance()->OpenModal(
				"SaveProjectDlg", "Save Project File", "Project File{.ifs}", ".",
				1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
			return true;
		});
	m_ActionSystem.Add([this]()
		{
			return Display_SaveProjectDialog();
		});
}

void MainFrame::Action_Menu_CloseProject()
{
/*
Close project :
-	unsaved :
	-	add action : show unsaved dialog
	-	add action : Close project
-	saved :
	-	add action : Close project
*/
	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			m_ProjectFile.Clear();
			return true;
		});
}

void MainFrame::Action_Window_CloseApp()
{
	if (m_NeedToCloseApp) return; // block next call to close app when running
/*
Close app :
-	unsaved :
	-	add action : show unsaved dialog
	-	add action : Close app
-	saved :
	-	add action : Close app
*/
	m_NeedToCloseApp = true;

	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			glfwSetWindowShouldClose(m_Window, GL_TRUE); // close app
			return true;
		});
}

void MainFrame::Action_OpenUnSavedDialog_IfNeeded()
{
	if (m_ProjectFile.IsLoaded() &&
		m_ProjectFile.IsThereAnyNotSavedChanged())
	{
		OpenUnSavedDialog();
		m_ActionSystem.Add([this]()
			{
				return ShowUnSavedDialog();
			});
	}
}

void MainFrame::Action_Cancel()
{
/*
-	cancel :
	-	clear actions
*/
	CloseUnSavedDialog();
	m_ActionSystem.Clear();
	m_NeedToCloseApp = false;
}

bool MainFrame::Action_UnSavedDialog_SaveProject()
{
	bool res = SaveProject();
	if (!res)
	{
		m_ActionSystem.Insert([this]()
			{
				return Display_SaveProjectDialog();
			});
		m_ActionSystem.Insert([this]()
			{
				CloseUnSavedDialog();
				ImGuiFileDialog::Instance()->OpenModal(
					"SaveProjectDlg", "Save Project File", "Project File{.ifs}",
					".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
				return true;
			});
	}
	return res;
}

void MainFrame::Action_UnSavedDialog_SaveAsProject()
{
	m_ActionSystem.Insert([this]()
		{
			return Display_SaveProjectDialog();
		});
	m_ActionSystem.Insert([this]()
		{
			CloseUnSavedDialog();
			ImGuiFileDialog::Instance()->OpenModal(
				"SaveProjectDlg", "Save Project File", "Project File{.ifs}",
				".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
			return true;
		});
}

void MainFrame::Action_UnSavedDialog_Cancel()
{
	Action_Cancel();
}

///////////////////////////////////////////////////////
//// DIALOG FUNCS /////////////////////////////////////
///////////////////////////////////////////////////////

bool MainFrame::Display_OpenProjectDialog()
{
	// need to return false to continue to be displayed next frame

	ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
	ImVec2 max = MainFrame::Instance()->m_DisplaySize;
	
	if (ImGuiFileDialog::Instance()->Display("OpenProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			CloseUnSavedDialog();
			LoadProject(ImGuiFileDialog::Instance()->GetFilePathName());
		}
		else // cancel
		{
			Action_Cancel(); // we interrupts all actions
		}

		ImGuiFileDialog::Instance()->Close();

		return true;
	}

	return false;
}

bool MainFrame::Display_SaveProjectDialog()
{
	// need to return false to continue to be displayed next frame

	ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
	ImVec2 max = MainFrame::Instance()->m_DisplaySize;
	
	if (ImGuiFileDialog::Instance()->Display("SaveProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			CloseUnSavedDialog();
			auto file = ImGuiFileDialog::Instance()->GetFilePathName();
			SaveAsProject(ImGuiFileDialog::Instance()->GetFilePathName());
		}
		else // cancel
		{
			Action_Cancel(); // we interrupts all actions
		}

		ImGuiFileDialog::Instance()->Close();

		return true;
	}

	return false;
}

///////////////////////////////////////////////////////
//// RE ROUTE /////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::ReRouteFontToFile(const std::string& vFontNameToReRoute, const std::string& vGoodFilePathName)
{
	auto ps = FileHelper::Instance()->ParsePathFileName(vGoodFilePathName);
	if (ps.isOk)
	{
		auto fontInfos = m_ProjectFile.m_Fonts[vFontNameToReRoute];
		if (fontInfos.use_count())
		{
			fontInfos->m_FontFilePathName = m_ProjectFile.GetRelativePath(vGoodFilePathName);
			fontInfos->m_FontFileName = ps.name + "." + ps.ext;
			m_ProjectFile.m_Fonts[fontInfos->m_FontFileName] = fontInfos;
			if (fontInfos->m_FontFileName != vFontNameToReRoute)
				m_ProjectFile.m_Fonts.erase(vFontNameToReRoute);
			ParamsPane::Instance()->OpenFont(&m_ProjectFile, fontInfos->m_FontFilePathName, true);
		}
	}
}

///////////////////////////////////////////////////////
//// APP CLOSING //////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::IWantToCloseTheApp()
{
	Action_Window_CloseApp();
}

///////////////////////////////////////////////////////
//// DROP /////////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::JustDropFiles(int count, const char** paths)
{
	std::map<std::string, std::string> dicoFont;
	std::string prj;

	for (int i = 0; i < count; i++)
	{
		// file
		auto f = std::string(paths[i]);
		
		// lower case
		auto f_opt = f;
		for (auto& c : f_opt)
			c = (char)std::tolower((int)c);

		// well known extention
		if (	f_opt.find(".ttf") != std::string::npos			// truetype (.ttf)
			||	f_opt.find(".otf") != std::string::npos			// opentype (.otf)
			//||	f_opt.find(".ttc") != std::string::npos		// ttf/otf collection for futur (.ttc)
			)
		{
			dicoFont[f] = f;
		}
		if (f_opt.find(".ifs") != std::string::npos)
		{
			prj = f;
		}
	}

	// priority to project file
	if (!prj.empty())
	{
		LoadProject(prj);
	}
	// then font files
	else if (!dicoFont.empty()) // some file are ok for opening
	{
		// if no project is available, we will create it
		if (!m_ProjectFile.IsLoaded())
			NewProject(""); // with empty path, will have to ne saved later

		// try to open fonts
		ParamsPane::Instance()->OpenFonts(&m_ProjectFile, dicoFont);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainFrame::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += ImGuiThemeHelper::Instance()->getXml(vOffset);
	str += LayoutManager::Instance()->getXml(vOffset, "app");
	str += vOffset + "<bookmarks>" + ImGuiFileDialog::Instance()->SerializeBookmarks() + "</bookmarks>\n";
	str += vOffset + "<showaboutdialog>" + (m_ShowAboutDialog ? "true" : "false") + "</showaboutdialog>\n";
	str += vOffset + "<showimgui>" + (m_ShowImGui ? "true" : "false") + "</showimgui>\n";
	str += vOffset + "<showmetric>" + (m_ShowMetric ? "true" : "false") + "</showmetric>\n";
	str += vOffset + "<showimguistyle>" + (m_ShowImGuiStyle ? "true" : "false") + "</showimguistyle>\n";
	str += vOffset + "<project>" + m_ProjectFile.m_ProjectFilePathName + "</project>\n";
	
	return str;
}

bool MainFrame::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

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
	LayoutManager::Instance()->setFromXml(vElem, vParent, "app");

	if (strName == "bookmarks")
		ImGuiFileDialog::Instance()->DeserializeBookmarks(strValue);
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