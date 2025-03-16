set(SOURCES
    ATAIOINT.C
    ATAIOISA.C
    ATAIOPCI.C
    ATAIOPIO.C
    ATAIOREG.C
    ATAIOSUB.C
    ATAIOTMR.C
    ATAIOTRC.C
    )

add_library(atadrvr STATIC ${SOURCES})
target_include_directories(atadrvr PUBLIC ${THIRDPARTY_DIR}/atadrvr)
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)
