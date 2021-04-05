#pragma once

// uncomment and modify defines under for customize ImGuiFileDialog

//#define MAX_FILE_DIALOG_NAME_BUFFER 1024
//#define MAX_PATH_BUFFER_SIZE 1024

#include <Gui/ImWidgets.h>

#define USE_EXPLORATION_BY_KEYS
// this mapping by default is for GLFW but you can use another
//#include <GLFW/glfw3.h> 
// Up key for explore to the top
#define IGFD_KEY_UP 265 //GLFW_KEY_UP
// Down key for explore to the bottom
#define IGFD_KEY_DOWN 264 // GLFW_KEY_DOWN
// Enter key for open directory
#define IGFD_KEY_ENTER 257 // GLFW_KEY_ENTER
// BackSpace for comming back to the last directory
#define IGFD_KEY_BACKSPACE 259 // GLFW_KEY_BACKSPACE

// widget
// filter combobox width
//#define FILTER_COMBO_WIDTH 120.0f
// button widget use for compose path
#define IMGUI_PATH_BUTTON ImGui::ContrastedButton
// standard button
#define IMGUI_BUTTON ImGui::ContrastedButton

#include <Res/CustomFont.h>

// locales string
#define createDirButtonString ICON_IGFS_ADD
#define okButtonString ICON_IGFS_OK " OK"
#define cancelButtonString ICON_IGFS_CANCEL " Cancel"
#define resetButtonString ICON_IGFS_RESET
#define drivesButtonString ICON_IGFS_DRIVES
#define searchString ICON_IGFS_SEARCH
#define dirEntryString " " ICON_IGFS_FOLDER
#define linkEntryString " " ICON_IGFS_LINK
#define fileEntryString " " ICON_IGFS_FILE
//#define fileNameString "File Name : "
//#define buttonResetSearchString "Reset search"
//#define buttonDriveString "Drives"
//#define buttonResetPathString "Reset to current directory"
//#define buttonCreateDirString "Create Directory"
//#define OverWriteDialogTitleString "The file Already Exist !"
//#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
#define OverWriteDialogConfirmButtonString ICON_IGFS_OK " Confirm"
#define OverWriteDialogCancelButtonString ICON_IGFS_CANCEL " Cancel"

// theses icons will appear in table headers
#define USE_CUSTOM_SORTING_ICON
#define tableHeaderAscendingIcon ICON_IGFS_SORT_ASC
#define tableHeaderDescendingIcon ICON_IGFS_SORT_DESC
#define tableHeaderFileNameString " File name"
#define tableHeaderFileSizeString " Size"
#define tableHeaderFileDateString " Date"

#define USE_BOOKMARK
#define defaultBookmarkPaneWith 200.0f
//#define IMGUI_TOGGLE_BUTTON ToggleButton
#define bookmarksButtonString ICON_IGFS_BOOKMARK
//#define bookmarksButtonHelpString "Bookmark"
#define addBookmarkButtonString ICON_IGFS_ADD
#define removeBookmarkButtonString ICON_IGFS_REMOVE