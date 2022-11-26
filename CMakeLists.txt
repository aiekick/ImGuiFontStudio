cmake_minimum_required(VERSION 3.1)

set(PROJECT ImGuiFontStudio)

enable_language(C CXX)

project(${PROJECT} CXX)

set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "")

option(USE_VULKAN OFF)

## for group smake targets in the dir cmakeTargets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmakeTargets")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../../../src/Gui/CustomImGuiFileDialogConfig.h")
add_definitions(-DIMGUI_USER_CONFIG="../../../src/Gui/CustomImConfig.h")
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(-DIMGUI_ENABLE_FREETYPE)
add_definitions(-DUSE_IMGUI="${CMAKE_SOURCE_DIR}/3rdparty/imgui/imgui.h")
add_definitions(-DPROJECT_NAME="${PROJECT}")

include(cmake/3rdparty.cmake)

## other define
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")

set(FINAL_BIN_DIR ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_INSTALL_PREFIX "${FINAL_BIN_DIR}")
message("${PROJECT} => ${FINAL_BIN_DIR}")

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(ARCH x64)
else()
	set(ARCH x32)
endif()

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

add_definitions(-DUSE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS) ## for the resize imgui issue when we have child glfw windows

if(MSVC)
    # Ignore 4055 for glad
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DMSVC)
	add_definitions(-DNOMINMAX) ## for avoid conflicts with std::min and std::max
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) ## for disable crt secure warnings (du to no cross plafrom secure function for windows)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter")
endif ()

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX _debug)
endif()

file(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(main FILES ${PROJECT_MAIN})

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})

file(GLOB_RECURSE PROJECT_BACKEND_COMMON_RECURSE 
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.h
		${CMAKE_SOURCE_DIR}/Backends/common/freetype/imgui_freetype.cpp
		${CMAKE_SOURCE_DIR}/Backends/common/freetype/imgui_freetype.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${PROJECT_BACKEND_COMMON_RECURSE})

if (USE_VULKAN)
	file(GLOB PROJECT_MAIN ${CMAKE_SOURCE_DIR}/main_vulkan.cpp)
	file(GLOB_RECURSE PROJECT_BACKEND_VULKAN_RECURSE 
		${CMAKE_SOURCE_DIR}/Backends/vulkan/imgui_impl_vulkan_user_texture.cpp
		${CMAKE_SOURCE_DIR}/Backends/vulkan/imgui_impl_vulkan_user_texture.h)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${PROJECT_BACKEND_VULKAN_RECURSE})
else()
	file(GLOB PROJECT_MAIN ${CMAKE_SOURCE_DIR}/main_opengl.cpp)
	file(GLOB_RECURSE PROJECT_BACKEND_OPENGL_RECURSE 
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.h)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${PROJECT_BACKEND_OPENGL_RECURSE})
endif()
	
if (APPLE)
	##set(ICON icon.icns)
	##set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_executable(${PROJECT}
	MACOSX_BUNDLE
	##${ICON}
	${PROJECT_MAIN}
	${PROJECT_SRC_RECURSE}
	${PROJECT_BACKEND_COMMON_RECURSE}
	${PROJECT_BACKEND_VULKAN_RECURSE}
	${PROJECT_BACKEND_OPENGL_RECURSE}
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

if (USE_VULKAN)
	set(BACKEND_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
	set(BACKEND_LIBRARIES ${Vulkan_LIBRARIES})
else()
	set(BACKEND_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} ${GLAD_INCLUDE_DIR})
	set(BACKEND_LIBRARIES ${OPENGL_LIBRARIES} ${GLAD_LIBRARIES})
endif()

message("BACKEND_INCLUDE_DIRS : ${BACKEND_INCLUDE_DIRS}")
message("BACKEND_LIBRARIES : ${BACKEND_LIBRARIES}")

include_directories(
	Backends
	src
	3rdparty
	${BACKEND_INCLUDE_DIRS}
	${PLOG_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
	${SFNTLY_INCLUDE_DIR}
	${TINYXML2_INCLUDE_DIR}
	${IMGUIFILEDIALOG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIR}
)

target_link_libraries(${PROJECT} PRIVATE
	${BACKEND_LIBRARIES}
	${GLFW_LIBRARIES}
	${IMGUI_LIBRARIES}
	${CTOOLS_LIBRARIES}
	${SFNTLY_LIBRARIES}
	${TINYXML2_LIBRARIES}
	${IMGUIFILEDIALOG_LIBRARIES}
	${FREETYPE_LIBRARIES}
)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY projects DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY samples_Fonts DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY doc DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(FILES LICENSE MacOSXBundleInfo.plist.in README.md DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

set(${PROJECT}_Output_Name "${PROJECT}_${CMAKE_SYSTEM_NAME}$<$<CONFIG:Debug>:_Debug>$<$<CONFIG:Release>:_Release>$<$<CONFIG:MinSizeRel>:_MinSizeRel>$<$<CONFIG:RelWithDebInfo>:_RelWithDebInfo>_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${${PROJECT}_Output_Name}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")

########################################################
################# BUILD INCREMENT TOOL #################
########################################################

## only for Debug
add_subdirectory(3rdparty/buildInc)
set_target_properties(BuildInc PROPERTIES FOLDER 3rdparty/tools)

add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND BuildInc -prefix=${PROJECT} 1000:10 $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/src/Headers/${PROJECT}Build.h>
	DEPENDS ${PROJECT}
)

#############################################################
### INSTALL #################################################
#############################################################

install(TARGETS 
	${PROJECT} 
	RUNTIME 
	DESTINATION /
	COMPONENT APP_BINARY)

#############################################################
### VERSION #################################################
#############################################################

##file(READ ${CMAKE_SOURCE_DIR}/src/Headers/${PROJECT}Build.h BuildIdFile)
##string(REGEX MATCH "#define ${PROJECT}_BuildNumber ([0-9]*)" _ ${BuildIdFile})
##set(BuildNumber ${CMAKE_MATCH_1})
##string(REGEX MATCH "#define ${PROJECT}_MinorNumber ([0-9]*)" _ ${BuildIdFile})
##set(MinorNumber ${CMAKE_MATCH_1})
##string(REGEX MATCH "#define ${PROJECT}_MajorNumber ([0-9]*)" _ ${BuildIdFile})
##set(MajorNumber ${CMAKE_MATCH_1})
##set(${PROJECT}_Version "v${MajorNumber}.${MinorNumber}.${BuildNumber}")
##message("${PROJECT} Version to install : ${${PROJECT}_Version}")

#############################################################
### PACK ####################################################
## https://gitlab.kitware.com/cmake/community/-/wikis/doc/cpack/Component-Install-With-CPack
#############################################################

if (USE_PACKAGE_TO_NSIS)
	set(CPACK_GENERATOR "NSIS")
else()
	set(CPACK_GENERATOR "ZIP")
endif()
set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_ZIP_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_PACKAGE_FILE_NAME "${${PROJECT}_Output_Name}_v${MajorNumber}.${MinorNumber}.${BuildNumber}")
set(CPACK_PACKAGE_VENDOR "FunParadigm")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${${PROJECT}_Output_Name}")
set(CPACK_PACKAGE_VERSION "${MajorNumber}.${MinorNumber}.${BuildNumber}")
set(CPACK_PACKAGE_VERSION_MAJOR "${MajorNumber}")
set(CPACK_PACKAGE_VERSION_MINOR "${MinorNumber}")
set(CPACK_PACKAGE_VERSION_PATCH "${BuildNumber}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${FINAL_BIN_DIR}")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
##set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/VERSION")
SET(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_INSTALL_PREFIX}")
include(CPack)
