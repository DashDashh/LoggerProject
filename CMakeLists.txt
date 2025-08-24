cmake_minimum_required(VERSION 3.14)
project(LoggerProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" OFF)

# Library
add_library(LoggerLib Logger.cpp Logger.h)
target_include_directories(LoggerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Application
add_executable(LoggerApp Main.cpp)
target_link_libraries(LoggerApp LoggerLib)

# Tests with GoogleTest
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd718599d30c5a4435c0c8.zip
    )
    
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    add_executable(LoggerTests test.cpp)
    target_link_libraries(LoggerTests LoggerLib gtest_main)
    
    include(GoogleTest)
    gtest_discover_tests(LoggerTests)
endif()
