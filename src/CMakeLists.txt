set(SOURCES main.cpp)

add_executable(cdemu ${SOURCES})
target_link_libraries(cdemu atadrvr)
