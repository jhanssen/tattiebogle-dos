set(SOURCES_SMALL
    ATAIOINT.C
    ATAIOPIO.C
    ATAIOREG.C
    ATAIOSUB.C
    ATAIOTMR.C
    ATAIOTRC.C
)
set(SOURCES
    ${SOURCES_SMALL}
    ATAIOISA.C
    ATAIOPCI.C
)

add_library(atadrvr_small STATIC ${SOURCES_SMALL})
target_include_directories(atadrvr_small PUBLIC ${THIRDPARTY_DIR}/atadrvr)

add_library(atadrvr STATIC ${SOURCES})
target_include_directories(atadrvr PUBLIC ${THIRDPARTY_DIR}/atadrvr)

set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)
