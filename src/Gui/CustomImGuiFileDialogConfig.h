#pragma once

// uncomment and modify defines under for customize ImGuiFileDialog

// widget
// button widget use for compose path
//#define IMGUI_PATH_BUTTON ImGui::Button
// standar button
//#define IMGUI_BUTTON ImGui::Button

#include <Res/CustomFont.h>

// locales string
#define createDirButtonString ICON_IGFS_ADD
#define okButtonString ICON_IGFS_OK " OK"
#define cancelButtonString ICON_IGFS_CANCEL " Cancel"
#define resetButtonString ICON_IGFS_RESET
#define drivesButtonString ICON_IGFS_DRIVES
#define searchString ICON_IGFS_SEARCH
#define dirEntryString ICON_IGFS_FOLDER
#define linkEntryString ICON_IGFS_LINK
#define fileEntryString ICON_IGFS_FILE
//#define fileNameString "File Name : "
//#define buttonResetSearchString "Reset search"
//#define buttonDriveString "Drives"
//#define buttonResetPathString "Reset to current directory"
//#define buttonCreateDirString "Create Directory"

// theses icons will appear in table headers
#define USE_CUSTOM_SORTING_ICON
#define tableHeaderAscendingIcon ICON_IGFS_SORT_ASC
#define tableHeaderDescendingIcon ICON_IGFS_SORT_DESC
#define tableHeaderFileNameString " File name"
#define tableHeaderFileSizeString " Size"
#define tableHeaderFileDateString " Date"