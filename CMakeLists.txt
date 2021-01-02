cmake_minimum_required(VERSION 3.1)

set(PROJECT ImGuiFontStudio)

enable_language(C CXX)

project(${PROJECT} CXX)

include(cmake/3rdparty.cmake)

## some defines for debug mode
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")

add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../../../src/Gui/CustomImGuiFileDialogConfig.h")

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

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(ARCH x64)
else()
	set(ARCH x32)
endif()

if (APPLE)
	set_target_properties(${PROJECT} PROPERTIES
		OUTPUT_NAME "${PROJECT}_${ARCH}"
		MACOSX_BUNDLE_EXECUTABLE_NAME "${PROJECT}_${ARCH}"
		MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT}_${ARCH}"
		FOLDER "${PROJECT}_${ARCH}"
		##RESOURCE icon.icns
		##MACOSX_BUNDLE_SHORT_VERSION_STRING "0.5"
		##MACOSX_BUNDLE_LONG_VERSION_STRING "Beta 0.5"
		##MACOSX_BUNDLE_ICON_FILE icon.icns
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
elseif(WIN32)
	if (MINGW)
		set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${PROJECT}_Mingw_${ARCH}")
	elseif (CYGWIN)
		set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${PROJECT}_Cygwin_${ARCH}")
	elseif(MSVC)
		set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${PROJECT}_Msvc_${ARCH}")
	endif()
elseif(UNIX)
	set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${PROJECT}_${ARCH}")
endif()

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY projects DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY samples_Fonts DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY doc DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(FILES LICENSE MacOSXBundleInfo.plist.in README.md DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

include_directories(
	src
	3rdparty
	${PLOG_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${GLAD_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
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
