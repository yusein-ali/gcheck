cmake_minimum_required(VERSION 3.10)
project(gcheck)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

include_directories(${CMAKE_SOURCE_DIR}/include)

set(GCHECK_SOURCES
    src/gcheck.cpp
    src/user_object.cpp
    src/redirectors.cpp
    src/json.cpp
    src/console_writer.cpp
    src/argument.cpp
    src/stringify.cpp
    src/shared_allocator.cpp
    src/multiprocessing.cpp
    src/customtest.cpp
)

add_library(gcheck STATIC ${GCHECK_SOURCES})
add_library(gcheck_shared SHARED ${GCHECK_SOURCES})

target_compile_definitions(gcheck PRIVATE GCHECK_CONSTRUCT_DATA)
target_compile_definitions(gcheck_shared PRIVATE GCHECK_CONSTRUCT_DATA)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    COMMAND ${CMAKE_COMMAND} --build tests --target clean
)


add_executable(gcheck_exec ${GCHECK_SOURCES})

add_custom_target(run
    COMMAND gcheck_exec --json --option2
    DEPENDS gcheck_exec
)

# Create doxygen documentation target
find_package(Doxygen REQUIRED)
# set input and output files
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
# copy the doxygen configuration file and expand CMake variable
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

add_custom_target(doc_doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
