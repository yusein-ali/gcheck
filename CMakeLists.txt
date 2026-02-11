cmake_minimum_required(VERSION 3.16)

project(gcheck VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.doxygen
        @ONLY
    )

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.doxygen
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen into doc/html"
        VERBATIM
    )
else()
    message(STATUS "Doxygen not found: 'doc' target will not be available.")
endif()

option(GCHECK_BUILD_STATIC "Build static gcheck library" ON)
option(GCHECK_BUILD_SHARED "Build shared gcheck library" OFF)
option(GCHECK_ENABLE_CONSTRUCT_DATA "Define GCHECK_CONSTRUCT_DATA" OFF)
option(GCHECK_BUILD_TESTS "Build a test executable from tests/" OFF)
option(GCHECK_TEST_FUNCTION "Build tests/function_test/function_test.cpp" OFF)
option(GCHECK_TEST_FUNCTION_2 "Build tests/function_test_2 test executable" OFF)
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
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
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

    set(GCHECK_INSTALL_TARGETS)

    if(GCHECK_BUILD_STATIC)
        add_library(gcheck_static STATIC)
        if(GCHECK_BUILD_SHARED)
            set_target_properties(gcheck_static PROPERTIES OUTPUT_NAME gcheck EXPORT_NAME gcheck_static)
        else()
            set_target_properties(gcheck_static PROPERTIES OUTPUT_NAME gcheck EXPORT_NAME gcheck)
        endif()
        gcheck_configure_library(gcheck_static)
        list(APPEND GCHECK_INSTALL_TARGETS gcheck_static)
    endif()

    if(GCHECK_BUILD_SHARED)
        add_library(gcheck_shared SHARED)
        set_target_properties(gcheck_shared PROPERTIES OUTPUT_NAME gcheck EXPORT_NAME gcheck)
        gcheck_configure_library(gcheck_shared)
        list(APPEND GCHECK_INSTALL_TARGETS gcheck_shared)
    endif()

    set(GCHECK_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/gcheck" CACHE STRING "Installation directory for gcheck CMake package files")

    install(
        TARGETS ${GCHECK_INSTALL_TARGETS}
        EXPORT gcheckTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gcheck
        FILES_MATCHING PATTERN "*.h"
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcheckConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/gcheckConfig.cmake
        INSTALL_DESTINATION ${GCHECK_INSTALL_CMAKEDIR}
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/gcheckConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        EXPORT gcheckTargets
        FILE gcheckTargets.cmake
        NAMESPACE gcheck::
        DESTINATION ${GCHECK_INSTALL_CMAKEDIR}
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/gcheckConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/gcheckConfigVersion.cmake
        DESTINATION ${GCHECK_INSTALL_CMAKEDIR}
    )
endif()
