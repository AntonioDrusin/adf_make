cmake_minimum_required(VERSION 3.30)
project(adf_make C)

enable_testing()
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")


include_directories(${CMAKE_SOURCE_DIR}/adflib/Lib/Win32)
file(GLOB SOURCES
        "${CMAKE_SOURCE_DIR}/*.c"
        "${CMAKE_SOURCE_DIR}/*.h"
)
file(GLOB LIB_SOURCES ${CMAKE_SOURCE_DIR}/adflib/Lib/*.*)
file(GLOB WIN_SOURCES ${CMAKE_SOURCE_DIR}/adflib/Lib/Win32/*.*)

add_executable(adf_make ${SOURCES} ${LIB_SOURCES} ${WIN_SOURCES})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET adf_make POST_BUILD
            COMMAND ${CMAKE_STRIP} $<TARGET_FILE:adf_make>)
endif()

add_subdirectory(google_tests)