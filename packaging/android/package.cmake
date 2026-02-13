cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMP_ROOT)
    set(CMP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
endif()
get_filename_component(CMP_ROOT "${CMP_ROOT}" ABSOLUTE)

set(ANDROID_DIR "${CMP_ROOT}/packaging/android")

find_program(GRADLE_EXECUTABLE gradle)
if(NOT GRADLE_EXECUTABLE)
    message(FATAL_ERROR "gradle not found; install Gradle or use a wrapper.")
endif()

if(NOT EXISTS "${ANDROID_DIR}/keystore.properties")
    message(FATAL_ERROR "Missing keystore.properties in packaging/android. Copy keystore.properties.template and fill in release signing values.")
endif()

function(cmp_run)
    execute_process(COMMAND ${ARGV} RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Command failed (exit ${rc}): ${ARGV}")
    endif()
endfunction()

cmp_run(${GRADLE_EXECUTABLE} --no-daemon -p "${ANDROID_DIR}" clean)
cmp_run(${GRADLE_EXECUTABLE} --no-daemon -p "${ANDROID_DIR}" assembleRelease)
cmp_run(${GRADLE_EXECUTABLE} --no-daemon -p "${ANDROID_DIR}" bundleRelease)

message(STATUS "Android artifacts are in ${ANDROID_DIR}/app/build/outputs")
