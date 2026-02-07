cmake_minimum_required(VERSION 3.16)

project(gcheck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GCHECK_BUILD_STATIC "Build static gcheck library" ON)
option(GCHECK_BUILD_SHARED "Build shared gcheck library" OFF)
option(GCHECK_ENABLE_CONSTRUCT_DATA "Define GCHECK_CONSTRUCT_DATA" OFF)
option(GCHECK_BUILD_TESTS "Build a test executable from tests/" OFF)
option(GCHECK_TEST_FUNCTION "Build tests/function_test/function_test.cpp" OFF)
option(GCHECK_TEST_IO "Build tests/io_test/io_test.cpp" OFF)
option(GCHECK_TEST_PREREQUISITE "Build tests/prerequisite/prerequisite.cpp" OFF)

file(GLOB GCHECK_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*
)

set(GCHECK_COMPILE_OPTIONS
    -Wall
    -Wextra
    -pedantic
)

function(gcheck_configure_library target_name)
    target_sources(${target_name} PRIVATE ${GCHECK_SOURCES})
    target_include_directories(${target_name}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_compile_options(${target_name} PRIVATE ${GCHECK_COMPILE_OPTIONS})

    if(GCHECK_ENABLE_CONSTRUCT_DATA)
        target_compile_definitions(${target_name} PUBLIC GCHECK_CONSTRUCT_DATA)
    endif()
endfunction()

include(${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cmake)

if(GCHECK_BUILD_TESTS)
    add_executable(${GCHECK_TEST_NAME} ${GCHECK_TEST_SOURCES} ${GCHECK_SOURCES})
    target_compile_options(${GCHECK_TEST_NAME} PRIVATE ${GCHECK_COMPILE_OPTIONS})
    target_compile_definitions(${GCHECK_TEST_NAME} PRIVATE GCHECK_BUILD_TESTS)
    target_include_directories(${GCHECK_TEST_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    if(GCHECK_ENABLE_CONSTRUCT_DATA)
        target_compile_definitions(${GCHECK_TEST_NAME} PRIVATE GCHECK_CONSTRUCT_DATA)
    endif()
else()
    if(NOT GCHECK_BUILD_STATIC AND NOT GCHECK_BUILD_SHARED)
        message(FATAL_ERROR "Enable at least one of GCHECK_BUILD_STATIC or GCHECK_BUILD_SHARED")
    endif()

    if(WIN32 AND GCHECK_BUILD_SHARED)
        message(FATAL_ERROR "GCHECK_BUILD_SHARED is not supported on Windows")
    endif()

    if(GCHECK_BUILD_STATIC)
        add_library(gcheck_static STATIC)
        set_target_properties(gcheck_static PROPERTIES OUTPUT_NAME gcheck)
        gcheck_configure_library(gcheck_static)
    endif()

    if(GCHECK_BUILD_SHARED)
        add_library(gcheck_shared SHARED)
        set_target_properties(gcheck_shared PROPERTIES OUTPUT_NAME gcheck)
        gcheck_configure_library(gcheck_shared)
    endif()
endif()
