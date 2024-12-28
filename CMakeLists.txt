cmake_minimum_required(VERSION 3.30)
project(adf_make)

enable_testing()
include(CTest)

set(CMAKE_CXX_STANDARD 98)

include_directories(${CMAKE_SOURCE_DIR}/adflib/Lib/Win32)
file(GLOB SOURCES
        "${CMAKE_SOURCE_DIR}/*.c"
        "${CMAKE_SOURCE_DIR}/*.h"
)
file(GLOB LIB_SOURCES ${CMAKE_SOURCE_DIR}/adflib/Lib/*.*)
file(GLOB WIN_SOURCES ${CMAKE_SOURCE_DIR}/adflib/Lib/Win32/*.*)
add_executable(adf_make main.c ${SOURCES} ${LIB_SOURCES} ${WIN_SOURCES})

add_subdirectory(google_tests)