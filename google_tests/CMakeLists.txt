﻿cmake_minimum_required(VERSION 3.14)
project(google_tests)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
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
        arguments_tests
        arguments_tests.cc
        ../arguments.c
)
target_link_libraries(
        arguments_tests
        GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(arguments_tests)