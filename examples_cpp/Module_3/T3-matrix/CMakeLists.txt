cmake_minimum_required(VERSION 3.16)

project(module3_t3_matrix LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(GCHECK_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../../../build/install" ABSOLUTE)
list(APPEND CMAKE_PREFIX_PATH "${GCHECK_INSTALL_PREFIX}")
find_package(gcheck CONFIG REQUIRED)

add_executable(t3_matrix
    src/main.cpp
    src/matrix.cpp
)
target_include_directories(t3_matrix PRIVATE src)

add_executable(t3_matrix_test
    test/test_source.cpp
    src/matrix.cpp
)
target_include_directories(t3_matrix_test PRIVATE src)
target_link_libraries(t3_matrix_test PRIVATE gcheck::gcheck)

if(APPLE OR UNIX)
    set_target_properties(t3_matrix_test PROPERTIES
        BUILD_RPATH "${GCHECK_INSTALL_PREFIX}/lib"
    )
endif()
