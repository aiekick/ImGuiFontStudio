cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0023 NEW)

set(PROJECT ImGuiFontStudio)

enable_language(C CXX)

project(${PROJECT} CXX)

include(cmake/3rdparty.cmake)

## some defines for debug mode
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")

add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)

if(UNIX)
    if(APPLE)
        add_definitions(-DAPPLE)
    else()
        add_definitions(-DLINUX)
    endif()
    add_definitions(-DUNIX)
elseif(WIN32)
    add_definitions(-DWIN32)
	if(MINGW)
		add_definitions(-DMINGW32)
	endif()
    set(DIRENT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/dirent/include)
    include_directories(${DIRENT_INCLUDE_DIR})
endif()

if(MSVC)
    # Ignore 4055 for glad
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /IGNORE:4055")
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DMSVC)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
endif ()

add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../../../src/Gui/CustomImGuiFileDialogConfig.h")

file(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(main FILES ${PROJECT_MAIN})

file(GLOB_RECURSE PROJECT_SRC_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})


if (APPLE)
	##set(ICON icon.icns)
	##set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_executable(${PROJECT}
	MACOSX_BUNDLE
	##${ICON}
	${PROJECT_MAIN}
	${PROJECT_SRC_RECURSE}
)

if (APPLE)
	if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
		set_target_properties(${PROJECT}
			PROPERTIES
			OUTPUT_NAME "ImGuiFontStudio_Osx_x64"
			MACOSX_BUNDLE_EXECUTABLE_NAME "ImGuiFontStudio_Osx_x64"
			MACOSX_BUNDLE_BUNDLE_NAME "ImGuiFontStudio_Osx_x64"
			FOLDER "ImGuiFontStudio_Osx_x64"
			##RESOURCE icon.icns
			MACOSX_BUNDLE_SHORT_VERSION_STRING "0.5"
			MACOSX_BUNDLE_LONG_VERSION_STRING "Beta 0.5"
			##MACOSX_BUNDLE_ICON_FILE icon.icns
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
	else()
		set_target_properties(${PROJECT}
			PROPERTIES
			OUTPUT_NAME "ImGuiFontStudio_Osx_x32"
			MACOSX_BUNDLE_EXECUTABLE_NAME "ImGuiFontStudio_Osx_x32"
			MACOSX_BUNDLE_BUNDLE_NAME "ImGuiFontStudio_Osx_x32"
			FOLDER "ImGuiFontStudio_Osx_x32"
			##RESOURCE icon.icns
			MACOSX_BUNDLE_SHORT_VERSION_STRING "0.5"
			MACOSX_BUNDLE_LONG_VERSION_STRING "Beta 0.5"
			##MACOSX_BUNDLE_ICON_FILE icon.icns
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
	endif()
elseif(WIN32)
	if (MINGW)
		if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
			set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "ImGuiFontStudio_Mingw32_x64")
		else()
			set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "ImGuiFontStudio_Mingw32_x32")
		endif()
	elseif (CYGWIN)
		if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
			set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "ImGuiFontStudio_Cygwin_x64")
		else()
			set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "ImGuiFontStudio_Cygwin_x32")
		endif()
	elseif(MSVC)
		if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
			set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "ImGuiFontStudio_Msvc_x64")
		else()
			set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "ImGuiFontStudio_Msvc_x32")
		endif()
	endif()
elseif(UNIX)
	if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
		set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "ImGuiFontStudio_Linux_x64")
	else()
		set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "ImGuiFontStudio_Linux_x32")
	endif()
endif()

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin/x64")
else()
	set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin/x32")
endif()

install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

include_directories(
	src
	${PLOG_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${GLAD_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
	${CTOOLS_INCLUDE_DIR}
	${SFNTLY_INCLUDE_DIR}
	${TINYXML2_INCLUDE_DIR}
	${IMGUIFILEDIALOG_INCLUDE_DIR}
)

target_link_libraries(${PROJECT} PRIVATE
	${OPENGL_LIBRARIES}
	${GLFW_LIBRARIES}
	${GLAD_LIBRARIES}
	${IMGUI_LIBRARIES}
	${CTOOLS_LIBRARIES}
	${SFNTLY_LIBRARIES}
	${TINYXML2_LIBRARIES}
	${IMGUIFILEDIALOG_LIBRARIES}
)
