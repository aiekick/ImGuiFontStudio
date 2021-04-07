#include "GenMode.h"

#include <imgui/imgui.h>
#include <Project/ProjectFile.h>
#include <Project/FontInfos.h>
#include <Gui/ImWidgets.h>


void GenMode::AddGenMode(GenModeFlags vFlags)
{
	m_GenModeFlags |= vFlags;
}

void GenMode::RemoveGenMode(GenModeFlags vFlags)
{
	m_GenModeFlags &= ~vFlags;
}

GenModeFlags GenMode::GetGenMode() const
{
	return m_GenModeFlags;
}

bool GenMode::IsGenMode(GenModeFlags vFlags) const
{
	return (m_GenModeFlags & vFlags);
}


void GenMode::ManageFlag(bool vSelected, GenModeFlags* vContainer, GenModeFlags vFlag, bool vOneOrZeroAtTime, bool vAlwaysOne, GenModeFlags vFlagsToTakeIntoAccount)
{
	if (vContainer)
	{
		if (vSelected) {
			if (vOneOrZeroAtTime) {
				if (vFlagsToTakeIntoAccount) {
					if (vFlag & vFlagsToTakeIntoAccount) {
						*vContainer = (GenModeFlags)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
						*vContainer = (GenModeFlags)(*vContainer | vFlag); // add
					}
				}
				else *vContainer = vFlag; // set
			}
			else {
				if (vFlagsToTakeIntoAccount) {
					if (vFlag & vFlagsToTakeIntoAccount) {
						*vContainer = (GenModeFlags)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
						*vContainer = (GenModeFlags)(*vContainer | vFlag); // add
					}
				}
				else *vContainer = (GenModeFlags)(*vContainer | vFlag); // add
			}
		}
		else {
			if (vOneOrZeroAtTime) {
				if (!vAlwaysOne) *vContainer = (GenModeFlags)(0); // remove all
			}
			else *vContainer = (GenModeFlags)(*vContainer & ~vFlag); // remove one
		}
	}
}

void GenMode::ManageFlag(bool vSelected, ProjectFile* vProjectFile, GenModeFlags vFlag, bool vOneOrZeroAtTime, bool vAlwaysOne, GenModeFlags vFlagsToTakeIntoAccount)
{
	if (vProjectFile)
	{
		vProjectFile->SetProjectChange();
		ManageFlag(vSelected, &vProjectFile->m_GenModeFlags, vFlag, vOneOrZeroAtTime, vAlwaysOne, vFlagsToTakeIntoAccount);
		for (auto font : vProjectFile->m_Fonts)
		{
			if (font.second.use_count())
			{
				ManageFlag(vSelected, &font.second->m_GenModeFlags, vFlag, vOneOrZeroAtTime, vAlwaysOne, vFlagsToTakeIntoAccount);
			}
		}
	}
}

bool GenMode::RadioButtonLabeled_BitWize_GenMode(
	float vWidth, const char* vLabel, const char* vHelp, ProjectFile* vProjectFile, GenModeFlags vFlag,
	bool vOneOrZeroAtTime, //only one selcted at a time
	bool vAlwaysOne, // radio behavior, always one selected
	GenModeFlags vFlagsToTakeIntoAccount,
	bool vDisableSelection,
	ImFont* vLabelFont) // radio witl use only theses flags
{
	bool selected = vProjectFile->m_GenModeFlags & vFlag;
	const bool res = ImGui::RadioButtonLabeled(vWidth, vLabel, vHelp, &selected, vDisableSelection, vLabelFont);
	if (res) 
	{
		ManageFlag(selected, vProjectFile, vFlag, vOneOrZeroAtTime, vAlwaysOne, vFlagsToTakeIntoAccount);
	}
	return res;
}

bool GenMode::RadioButtonLabeled_BitWize_GenMode(float vWidth, 
	const char* vLabelOK, const char* vLabelNOK, const char* vHelp, 
	ProjectFile* vProjectFile, GenModeFlags vFlag,
	bool vOneOrZeroAtTime, //only one selected at a time
	bool vAlwaysOne, // radio behavior, always one selected
	GenModeFlags vFlagsToTakeIntoAccount,
	bool vDisableSelection,
	ImFont* vLabelFont) // radio witl use only theses flags
{
	bool selected = vProjectFile->m_GenModeFlags & vFlag;
	const char* label = (selected ? vLabelOK : vLabelNOK);
	const bool res = ImGui::RadioButtonLabeled(vWidth, label, vHelp, &selected, vDisableSelection, vLabelFont);
	if (res) 
	{
		ManageFlag(selected, vProjectFile, vFlag, vOneOrZeroAtTime, vAlwaysOne, vFlagsToTakeIntoAccount);
	}
	return res;
}