cmake_minimum_required(VERSION 3.16)

project(module3_method_test LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/gcheck_include.cmake)
find_package(gcheck CONFIG REQUIRED)

add_executable(method_test_private_state_example
    method_test_private_state_example.cpp
)
target_include_directories(method_test_private_state_example PRIVATE ../../include)
target_link_libraries(method_test_private_state_example PRIVATE gcheck::gcheck)

if(APPLE OR UNIX)
    set_target_properties(method_test_private_state_example PROPERTIES BUILD_RPATH "${GCHECK_INSTALL_PREFIX}/lib")
endif()
