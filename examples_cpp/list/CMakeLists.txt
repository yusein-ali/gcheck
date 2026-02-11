cmake_minimum_required(VERSION 3.16)

project(module3_t2_list LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/gcheck_include.cmake)
find_package(gcheck CONFIG REQUIRED)

add_executable(t2_list
    src/main.cpp
    src/list.cpp
)
target_include_directories(t2_list PRIVATE src)

add_executable(t2_list_test
    test/test_source.cpp
    src/list.cpp
)
target_include_directories(t2_list_test PRIVATE src)
target_link_libraries(t2_list_test PRIVATE gcheck::gcheck)

if(APPLE OR UNIX)
    set_target_properties(t2_list_test PROPERTIES
        BUILD_RPATH "${GCHECK_INSTALL_PREFIX}/lib"
    )
endif()
