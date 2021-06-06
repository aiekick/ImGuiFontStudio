option(USE_GL_VERSION_CHECKER OFF)
option(USE_CONFIG_SYSTEM ON)

include_directories(
	${GLFW_INCLUDE_DIR}
)

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/ctools)

set_target_properties(ctools PROPERTIES FOLDER 3rdparty/aiekick)
set_target_properties(ctools PROPERTIES LINK_FLAGS "/ignore:4244")

