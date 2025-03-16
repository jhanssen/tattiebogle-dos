set(SOURCES MINDRVR.C)

add_library(mindrvr STATIC ${SOURCES})
target_include_directories(mindrvr PUBLIC ${THIRDPARTY_DIR}/mindrvr)
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)
