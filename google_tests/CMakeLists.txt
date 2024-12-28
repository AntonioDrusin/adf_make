cmake_minimum_required(VERSION 3.14)
project(google_tests)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
        adf_tests
        arguments_tests.cc
        filelist_tests.cc
        ../arguments.c
        ../list_files.c
)
target_link_libraries(
        adf_tests
        GTest::gtest_main
        GTest::gmock
)
target_include_directories(adf_tests PRIVATE
        ${gtest_SOURCE_DIR}/include
        ${gmock_SOURCE_DIR}/include
)
include(GoogleTest)
gtest_discover_tests(adf_tests)