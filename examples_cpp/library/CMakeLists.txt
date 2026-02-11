cmake_minimum_required(VERSION 3.16)

project(module3_t4_library LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/gcheck_include.cmake)
find_package(gcheck CONFIG REQUIRED)

set(T4_SRC
    src/book.cpp
    src/customer.cpp
    src/library.cpp
)

add_executable(t4_library
    src/main.cpp
    ${T4_SRC}
)
target_include_directories(t4_library PRIVATE src)

add_executable(t4_library_test
    test/test_source.cpp
    ${T4_SRC}
)
target_include_directories(t4_library_test PRIVATE src)
target_link_libraries(t4_library_test PRIVATE gcheck::gcheck)

add_library(t4_library_requirements OBJECT
    test/test_requirements.cpp
)
target_include_directories(t4_library_requirements PRIVATE src)

if(APPLE OR UNIX)
    set_target_properties(t4_library_test PROPERTIES BUILD_RPATH "${GCHECK_INSTALL_PREFIX}/lib")
endif()
