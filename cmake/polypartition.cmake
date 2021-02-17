set(POLYPARTITION_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/polypartition/src)
file(GLOB POLYPARTITION_SOURCES 
	${POLYPARTITION_INCLUDE_DIR}/*.cpp
	${POLYPARTITION_INCLUDE_DIR}/*.h
)
                 
add_library(polypartition STATIC ${POLYPARTITION_SOURCES})
    
set_target_properties(polypartition PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(polypartition PROPERTIES FOLDER 3rdparty)

set(POLYPARTITION_LIBRARIES polypartition)

