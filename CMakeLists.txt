cmake_minimum_required(VERSION 3.26)
project(cdemu)

set(THIRDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)
set(THIRDPARTY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rdparty)

set(CMAKE_C_FLAGS "-c -cc -q -w4 -3 -e25 -zq -oabehikls -d0 -bt=dos -mc")
set(CMAKE_CXX_FLAGS "-c -cc++ -q -w4 -3 -e25 -zq -oabehikls -d0 -bt=dos -mc")

add_subdirectory(3rdparty)
add_subdirectory(src)
