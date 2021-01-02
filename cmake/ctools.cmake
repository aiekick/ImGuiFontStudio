set(CTOOLS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ctools)
file(GLOB CTOOLS_SOURCES ${CTOOLS_INCLUDE_DIR}/*.cpp)
file(GLOB CTOOLS_HEADERS ${CTOOLS_INCLUDE_DIR}/*.h)
                 
add_library(ctools STATIC ${CTOOLS_SOURCES} ${CTOOLS_HEADERS})

add_definitions(-DGLAD)
add_definitions(-DIMGUI)
add_definitions(-DUSE_IMGUI="../imgui/imgui.h")
add_definitions(-DUSE_GLFW3)

include_directories(
    ${CTOOLS_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/3rdparty)
    
target_link_libraries(ctools
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES})
    
set_target_properties(ctools PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ctools PROPERTIES FOLDER 3rdparty/aiekick)

set(CTOOLS_LIBRARIES ctools)

