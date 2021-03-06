cmake_minimum_required(VERSION 3.1)

set(PROJECT ImGuiFontStudio)

enable_language(C CXX)

project(${PROJECT} CXX)

option(USE_VULKAN OFF)

## for group smake targets in the dir cmakeTargets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmakeTargets")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

include(cmake/3rdparty.cmake)

add_definitions(-DIMGUI)
add_definitions(-DUSE_IMGUI="../imgui/imgui.h")
add_definitions(-DUSE_GLFW3)
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../../../src/Gui/CustomImGuiFileDialogConfig.h")

## other define
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DMSVC)
	add_definitions(-DNOMINMAX) ## for avoid conflicts with std::min and std::max
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) ## fot disable crt secure warnings (du to no cross plafrom secure function for windows)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter")
endif ()

file(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(main FILES ${PROJECT_MAIN})

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})

file(GLOB_RECURSE PROJECT_BACKEND_COMMON_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/Backends/common/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/Backends/common/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/Backends/common/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Backends/common/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${PROJECT_BACKEND_COMMON_RECURSE})

if (USE_VULKAN)
	file(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main_vulkan.cpp)
	file(GLOB_RECURSE PROJECT_BACKEND_VULKAN_RECURSE 
		${CMAKE_CURRENT_SOURCE_DIR}/Backends/vulkan/*.c
		${CMAKE_CURRENT_SOURCE_DIR}/Backends/vulkan/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/Backends/vulkan/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Backends/vulkan/*.hpp)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${PROJECT_BACKEND_VULKAN_RECURSE})
else()
	file(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main_opengl.cpp)
	file(GLOB_RECURSE PROJECT_BACKEND_OPENGL_RECURSE 
		${CMAKE_CURRENT_SOURCE_DIR}/Backends/opengl/*.c
		${CMAKE_CURRENT_SOURCE_DIR}/Backends/opengl/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/Backends/opengl/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Backends/opengl/*.hpp)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${PROJECT_BACKEND_OPENGL_RECURSE})
endif()

source_group(main FILES ${PROJECT_MAIN})
	
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

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY projects DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY samples_Fonts DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(DIRECTORY doc DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(FILES LICENSE MacOSXBundleInfo.plist.in README.md DESTINATION "${CMAKE_SOURCE_DIR}/bin/${ARCH}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

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
