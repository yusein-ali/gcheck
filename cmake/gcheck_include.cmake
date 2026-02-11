# Resolve GCHECK_INSTALL_PREFIX with the following priority:
# 1) environment variable GCHECK_INSTALL_DIR
# 2) bootstrap from GCHECK_SRC_DIR into local build tree
# 3) fallback to repository default: <repo>/build/install

if(DEFINED ENV{GCHECK_INSTALL_DIR} AND NOT "$ENV{GCHECK_INSTALL_DIR}" STREQUAL "")
    set(GCHECK_INSTALL_PREFIX "$ENV{GCHECK_INSTALL_DIR}")
elseif(DEFINED ENV{GCHECK_SRC_DIR} AND NOT "$ENV{GCHECK_SRC_DIR}" STREQUAL "")
    set(GCHECK_SRC_DIR "$ENV{GCHECK_SRC_DIR}")
    set(GCHECK_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/gcheck_install")
    set(GCHECK_BOOTSTRAP_BUILD_DIR "${CMAKE_BINARY_DIR}/_gcheck_build")

    message(STATUS "GCHECK_INSTALL_DIR not set. Bootstrapping gcheck from GCHECK_SRC_DIR=${GCHECK_SRC_DIR}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}"
            -S "${GCHECK_SRC_DIR}"
            -B "${GCHECK_BOOTSTRAP_BUILD_DIR}"
            -DGCHECK_BUILD_SHARED=ON
            -DGCHECK_BUILD_STATIC=OFF
            -DGCHECK_BUILD_TESTS=OFF
            -DCMAKE_INSTALL_PREFIX=${GCHECK_INSTALL_PREFIX}
        RESULT_VARIABLE _gcheck_cfg_result
    )
    if(NOT _gcheck_cfg_result EQUAL 0)
        message(FATAL_ERROR "Failed to configure gcheck from GCHECK_SRC_DIR (code ${_gcheck_cfg_result})")
    endif()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" --build "${GCHECK_BOOTSTRAP_BUILD_DIR}" --target gcheck_shared
        RESULT_VARIABLE _gcheck_build_result
    )
    if(NOT _gcheck_build_result EQUAL 0)
        message(FATAL_ERROR "Failed to build gcheck_shared from GCHECK_SRC_DIR (code ${_gcheck_build_result})")
    endif()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" --install "${GCHECK_BOOTSTRAP_BUILD_DIR}"
        RESULT_VARIABLE _gcheck_install_result
    )
    if(NOT _gcheck_install_result EQUAL 0)
        message(FATAL_ERROR "Failed to install gcheck from GCHECK_SRC_DIR (code ${_gcheck_install_result})")
    endif()
else()
    get_filename_component(GCHECK_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../build/install" ABSOLUTE)
endif()

list(APPEND CMAKE_PREFIX_PATH "${GCHECK_INSTALL_PREFIX}")
