file(GLOB IMGUIFILEDIALOG_FILES 
	${CMAKE_SOURCE_DIR}/3rdparty/ImGuiFileDialog/ImGuiFileDialog.h 
	${CMAKE_SOURCE_DIR}/3rdparty/ImGuiFileDialog/ImGuiFileDialog.cpp
	${CMAKE_SOURCE_DIR}/3rdparty/ImGuiFileDialog/ImGuiFileDialogConfig.h)
add_library(ImGuiFileDialog STATIC ${IMGUIFILEDIALOG_FILES})
set_target_properties(ImGuiFileDialog PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ImGuiFileDialog PROPERTIES FOLDER 3rdparty/aiekick)
set(IMGUIFILEDIALOG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ImGuiFileDialog)
set(IMGUIFILEDIALOG_LIBRARIES ImGuiFileDialog)
