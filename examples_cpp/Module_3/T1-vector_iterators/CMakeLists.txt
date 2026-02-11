cmake_minimum_required(VERSION 3.16)

project(module3_t1_vector_iterators LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/gcheck_include.cmake)
find_package(gcheck CONFIG REQUIRED)

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
