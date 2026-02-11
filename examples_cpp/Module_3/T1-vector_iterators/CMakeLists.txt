cmake_minimum_required(VERSION 3.16)

project(module3_t1_vector_iterators LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Repository root (gcheck project root)
get_filename_component(GCHECK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

# Default "installed" location requested by the project workflow.
set(GCHECK_INSTALL_PREFIX "${GCHECK_SOURCE_DIR}/build/install" CACHE PATH "Path to installed gcheck artifacts")
set(GCHECK_BUILD_DIR "${GCHECK_SOURCE_DIR}/build" CACHE PATH "Build directory used to bootstrap gcheck")

if(WIN32)
    set(_gcheck_lib_name gcheck.dll)
elseif(APPLE)
    set(_gcheck_lib_name libgcheck.dylib)
else()
    set(_gcheck_lib_name libgcheck.so)
endif()

set(GCHECK_INSTALL_LIB "${GCHECK_INSTALL_PREFIX}/lib/${_gcheck_lib_name}")
set(GCHECK_INSTALL_INCLUDE_DIR "${GCHECK_INSTALL_PREFIX}/include")
set(GCHECK_INSTALL_GCHECK_INCLUDE_DIR "${GCHECK_INSTALL_INCLUDE_DIR}/gcheck")

# Step 1: try to locate existing installed artifacts in build/install.
set(_gcheck_ready TRUE)
if(NOT EXISTS "${GCHECK_INSTALL_LIB}")
    set(_gcheck_ready FALSE)
endif()
if(NOT EXISTS "${GCHECK_INSTALL_GCHECK_INCLUDE_DIR}/tests.h")
    set(_gcheck_ready FALSE)
endif()

# Step 2: if missing, build shared gcheck and stage it into build/install.
if(NOT _gcheck_ready)
    message(STATUS "gcheck install artifacts not found in ${GCHECK_INSTALL_PREFIX}; bootstrapping shared gcheck")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -S "${GCHECK_SOURCE_DIR}" -B "${GCHECK_BUILD_DIR}"
                -DGCHECK_BUILD_SHARED=ON
                -DGCHECK_BUILD_STATIC=OFF
                -DGCHECK_BUILD_TESTS=OFF
        RESULT_VARIABLE _gcheck_cfg_result
    )
    if(NOT _gcheck_cfg_result EQUAL 0)
        message(FATAL_ERROR "Failed to configure gcheck in ${GCHECK_BUILD_DIR} (code ${_gcheck_cfg_result})")
    endif()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" --build "${GCHECK_BUILD_DIR}" --target gcheck_shared
        RESULT_VARIABLE _gcheck_build_result
    )
    if(NOT _gcheck_build_result EQUAL 0)
        message(FATAL_ERROR "Failed to build gcheck_shared in ${GCHECK_BUILD_DIR} (code ${_gcheck_build_result})")
    endif()

    # Manual staging to the requested install tree.
    file(MAKE_DIRECTORY "${GCHECK_INSTALL_PREFIX}/lib")
    file(MAKE_DIRECTORY "${GCHECK_INSTALL_GCHECK_INCLUDE_DIR}")

    set(_gcheck_built_lib "${GCHECK_BUILD_DIR}/${_gcheck_lib_name}")
    if(NOT EXISTS "${_gcheck_built_lib}")
        message(FATAL_ERROR "Built gcheck shared library not found at ${_gcheck_built_lib}")
    endif()

    file(COPY "${_gcheck_built_lib}" DESTINATION "${GCHECK_INSTALL_PREFIX}/lib")
    file(COPY "${GCHECK_SOURCE_DIR}/include/" DESTINATION "${GCHECK_INSTALL_GCHECK_INCLUDE_DIR}")

    if(NOT EXISTS "${GCHECK_INSTALL_LIB}" OR NOT EXISTS "${GCHECK_INSTALL_GCHECK_INCLUDE_DIR}/tests.h")
        message(FATAL_ERROR "gcheck bootstrap completed, but install artifacts are still missing in ${GCHECK_INSTALL_PREFIX}")
    endif()
endif()

# Step 3: consume installed gcheck artifacts.
add_library(gcheck::gcheck SHARED IMPORTED GLOBAL)
set_target_properties(gcheck::gcheck PROPERTIES
    IMPORTED_LOCATION "${GCHECK_INSTALL_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${GCHECK_INSTALL_INCLUDE_DIR}"
)

add_executable(t1_vector_iterators
    src/main.cpp
    src/vector_it.cpp
)
target_include_directories(t1_vector_iterators PRIVATE src)

add_executable(t1_vector_iterators_test
    test/test_source.cpp
    src/vector_it.cpp
)
target_include_directories(t1_vector_iterators_test PRIVATE src)
target_link_libraries(t1_vector_iterators_test PRIVATE gcheck::gcheck)

if(APPLE OR UNIX)
    set_target_properties(t1_vector_iterators_test PROPERTIES
        BUILD_RPATH "${GCHECK_INSTALL_PREFIX}/lib"
    )
endif()
