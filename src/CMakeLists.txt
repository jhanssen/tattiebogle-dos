set(SOURCES main.cpp atapi.cpp args.cpp)

add_executable(cdemu ${SOURCES})
target_link_libraries(cdemu atadrvr)
