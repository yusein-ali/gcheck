cmake_minimum_required(VERSION 3.16)

project(module3_t5_pokemon LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(GCHECK_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../../../build/install" ABSOLUTE)
list(APPEND CMAKE_PREFIX_PATH "${GCHECK_INSTALL_PREFIX}")
find_package(gcheck CONFIG REQUIRED)

add_executable(t5_pokemon
    src/main.cpp
    src/pokemon.cpp
)
target_include_directories(t5_pokemon PRIVATE src)

add_executable(t5_pokemon_test
    test/test_source.cpp
    src/pokemon.cpp
)
target_include_directories(t5_pokemon_test PRIVATE src)
target_link_libraries(t5_pokemon_test PRIVATE gcheck::gcheck)

if(APPLE OR UNIX)
    set_target_properties(t5_pokemon_test PROPERTIES BUILD_RPATH "${GCHECK_INSTALL_PREFIX}/lib")
endif()
