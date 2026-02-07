set(GCHECK_CMAKE_TESTS
    function_test
    io_test
    prerequisite
)

set(GCHECK_TEST_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(GCHECK_TEST_NAME "")
set(GCHECK_TEST_DIR "")
set(GCHECK_TEST_SOURCES "")

set(GCHECK_ENABLED_TEST_COUNT 0)
if(GCHECK_TEST_FUNCTION)
    math(EXPR GCHECK_ENABLED_TEST_COUNT "${GCHECK_ENABLED_TEST_COUNT} + 1")
    set(GCHECK_TEST_NAME function_test)
    set(GCHECK_TEST_DIR ${GCHECK_TEST_ROOT_DIR}/function_test)
endif()
if(GCHECK_TEST_IO)
    math(EXPR GCHECK_ENABLED_TEST_COUNT "${GCHECK_ENABLED_TEST_COUNT} + 1")
    set(GCHECK_TEST_NAME io_test)
    set(GCHECK_TEST_DIR ${GCHECK_TEST_ROOT_DIR}/io_test)
endif()
if(GCHECK_TEST_PREREQUISITE)
    math(EXPR GCHECK_ENABLED_TEST_COUNT "${GCHECK_ENABLED_TEST_COUNT} + 1")
    set(GCHECK_TEST_NAME prerequisite)
    set(GCHECK_TEST_DIR ${GCHECK_TEST_ROOT_DIR}/prerequisite)
endif()

if(GCHECK_BUILD_TESTS)
    if(GCHECK_ENABLED_TEST_COUNT GREATER 1)
        message(FATAL_ERROR "Only one test option can be enabled at a time")
    endif()

    if(GCHECK_ENABLED_TEST_COUNT EQUAL 0)
        message(FATAL_ERROR "GCHECK_BUILD_TESTS is ON but no test option was enabled")
    endif()
endif()

if(NOT GCHECK_TEST_NAME STREQUAL "")
    set(GCHECK_TEST_SOURCES ${GCHECK_TEST_DIR}/${GCHECK_TEST_NAME}.cpp)
endif()

if(GCHECK_BUILD_TESTS AND NOT EXISTS ${GCHECK_TEST_SOURCES})
    message(FATAL_ERROR "Selected test source was not found: ${GCHECK_TEST_SOURCES}")
endif()
