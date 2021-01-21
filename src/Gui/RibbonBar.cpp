#include "RibbonBar.h"
#include <imgui/imgui.h>
#include <Res/CustomFont.h>
#include <Project/ProjectFile.h>

void RibbonBar::Draw(ProjectFile *vProjectFile)
{
	if (ImGui::BeginTabBar("Main Tab Bar"))
	{
		if (ImGui::BeginTabItem(ICON_IGFS_PROJECT " Project"))
		{
			if (ImGui::Button(ICON_IGFS_FILE " New"))
			{
				//Action_Menu_NewProject();
			}

			ImGui::SameLine();

			if (ImGui::Button(ICON_IGFS_FOLDER_OPEN " Open"))
			{
				//Action_Menu_OpenProject();
			}

			if (vProjectFile && vProjectFile->IsLoaded())
			{
				ImGui::SameLine();

				if (ImGui::Button(ICON_IGFS_FOLDER_OPEN " Re Open"))
				{
					//Action_Menu_ReOpenProject();
				}

				ImGui::SameLine();

				if (ImGui::Button(ICON_IGFS_SAVE " Save"))
				{
					//Action_Menu_SaveProject();
				}

				ImGui::SameLine();

				if (ImGui::Button(ICON_IGFS_SAVE " Save As"))
				{
					//Action_Menu_SaveAsProject();
				}

				ImGui::SameLine();

				if (ImGui::Button(ICON_IGFS_DESTROY " Close"))
				{
					//Action_Menu_CloseProject();
				}
			}

			ImGui::SameLine();

			if (ImGui::Button(ICON_IGFS_ABOUT " About"))
			{
				//m_ShowAboutDialog = true;
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}