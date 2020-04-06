set(SFNTLY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/sfntly)
set(SFNTLY_LIBRARIES ${SFNTLY_LIBRARIES} sfntly)
	
add_definitions(-DSFNTLY_NO_EXCEPTION)
##add_definitions(-std=c++11)
add_definitions(-DSFNTLY_EXPERIMENTAL)

file(GLOB SFNTLY_CORE_FILES ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/*.h ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/*.cc)
file(GLOB SFNTLY_PORT_FILES ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/port/*.h ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/port/*.cc)
file(GLOB SFNTLY_DATA_FILES ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/data/*.h ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/data/*.cc)
file(GLOB SFNTLY_MATH_FILES ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/math/*.h ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/math/*.cc)
file(GLOB SFNTLY_TABLE_COMMON_FILES ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/table/*.h ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/table/*.cc)
file(GLOB SFNTLY_TABLE_BITMAP_FILES ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/table/bitmap/*.h ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/table/bitmap/*.cc)
file(GLOB SFNTLY_TABLE_CORE_FILES ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/table/core/*.h ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/table/core/*.cc)
file(GLOB SFNTLY_TABLE_TTF_FILES ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/table/truetype/*.h ${CMAKE_SOURCE_DIR}/3rdparty/sfntly/table/truetype/*.cc)
source_group(core FILES ${SFNTLY_CORE_FILES})
source_group(ports FILES ${SFNTLY_PORT_FILES})
source_group(data FILES ${SFNTLY_DATA_FILES})
source_group(math FILES ${SFNTLY_MATH_FILES})
source_group(table FILES ${SFNTLY_TABLE_COMMON_FILES})
source_group(table\\bitmap FILES ${SFNTLY_TABLE_BITMAP_FILES})
source_group(table\\core FILES ${SFNTLY_TABLE_CORE_FILES})
source_group(table\\truetype FILES ${SFNTLY_TABLE_TTF_FILES})
add_library(sfntly STATIC
      	    ${SFNTLY_CORE_FILES}
      	    ${SFNTLY_PORT_FILES}
      	    ${SFNTLY_DATA_FILES}
      	    ${SFNTLY_MATH_FILES}
      	    ${SFNTLY_TABLE_COMMON_FILES}
      	    ${SFNTLY_TABLE_BITMAP_FILES}
      	    ${SFNTLY_TABLE_CORE_FILES}
      	    ${SFNTLY_TABLE_TTF_FILES})
set_target_properties(sfntly PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(sfntly PROPERTIES FOLDER 3rdparty)

