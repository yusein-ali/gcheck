cmake_minimum_required(VERSION 3.16)

project(module3_overload_operators LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/gcheck_include.cmake)
find_package(gcheck CONFIG REQUIRED)

set(T4_SRC
    src/book.cpp
    src/customer.cpp
    src/library.cpp
)

add_executable(overload_operators
    src/main.cpp
    ${T4_SRC}
)
target_include_directories(overload_operators PRIVATE src)

add_executable(overload_operators_test
    test/test_source.cpp
    ${T4_SRC}
)
target_include_directories(overload_operators_test PRIVATE src)
target_link_libraries(overload_operators_test PRIVATE gcheck::gcheck)

if(APPLE OR UNIX)
    set_target_properties(overload_operators_test PROPERTIES BUILD_RPATH "${GCHECK_INSTALL_PREFIX}/lib")
endif()
