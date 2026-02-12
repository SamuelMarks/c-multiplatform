cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED M3_BINARY_DIR)
    message(FATAL_ERROR "M3_BINARY_DIR is required (path to configured build directory).")
endif()
get_filename_component(M3_BINARY_DIR "${M3_BINARY_DIR}" ABSOLUTE)

find_program(CPACK_EXECUTABLE cpack)
if(NOT CPACK_EXECUTABLE)
    message(FATAL_ERROR "cpack not found; install CMake with CPack.")
endif()

if(NOT DEFINED M3_CPACK_GENERATOR)
    if(WIN32)
        set(M3_CPACK_GENERATOR "WIX")
    elseif(APPLE)
        set(M3_CPACK_GENERATOR "DragNDrop")
    else()
        set(M3_CPACK_GENERATOR "TGZ")
    endif()
endif()

execute_process(
    COMMAND ${CPACK_EXECUTABLE} -G ${M3_CPACK_GENERATOR}
    WORKING_DIRECTORY "${M3_BINARY_DIR}"
    RESULT_VARIABLE rc)

if(NOT rc EQUAL 0)
    message(FATAL_ERROR "cpack failed (exit ${rc}).")
endif()

message(STATUS "Desktop package generated in ${M3_BINARY_DIR}")
