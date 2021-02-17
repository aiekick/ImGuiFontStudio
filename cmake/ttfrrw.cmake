add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/ttfrrw)
   
message("TTFRRW_INCLUDE_DIR : ${TTFRRW_INCLUDE_DIR}")
message("TTFRRW_LIBRARIES : ${TTFRRW_LIBRARIES}")
message("TTFRRW_LIB_DIR : ${TTFRRW_LIB_DIR}")
   
set_target_properties(ttfrrw PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ttfrrw PROPERTIES FOLDER 3rdparty/aiekick)
