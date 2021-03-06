if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(X11 REQUIRED)

  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "X11 Xi library is required")
  endif ()
endif ()

if (USE_VULKAN)
	find_package(Vulkan REQUIRED)
	add_definitions(-DVULKAN)
else()
	set(OpenGL_GL_PREFERENCE GLVND)
	find_package(OpenGL REQUIRED)
	add_definitions(-DGLAD)
	include(cmake/glad.cmake)
endif()

include(cmake/glfw.cmake)
include(cmake/imgui.cmake)
include(cmake/ctools.cmake)
include(cmake/sfntly.cmake)
include(cmake/tinyxml2.cmake)
include(cmake/imguifiledialog.cmake)
include(cmake/freetype.cmake)
