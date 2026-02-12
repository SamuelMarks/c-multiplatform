cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED M3_ROOT)
    set(M3_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
endif()
get_filename_component(M3_ROOT "${M3_ROOT}" ABSOLUTE)

find_program(XCODEBUILD_EXECUTABLE xcodebuild)
if(NOT XCODEBUILD_EXECUTABLE)
    message(FATAL_ERROR "xcodebuild not found; install Xcode command line tools.")
endif()

if(NOT DEFINED IOS_SCHEME)
    message(FATAL_ERROR "IOS_SCHEME is required (e.g., -DIOS_SCHEME=MyApp).")
endif()

if(DEFINED IOS_PROJECT AND DEFINED IOS_WORKSPACE)
    message(FATAL_ERROR "Specify only one of IOS_PROJECT or IOS_WORKSPACE.")
endif()
if(NOT DEFINED IOS_PROJECT AND NOT DEFINED IOS_WORKSPACE)
    message(FATAL_ERROR "Set IOS_PROJECT or IOS_WORKSPACE to an Xcode project/workspace path.")
endif()

if(NOT DEFINED IOS_CONFIGURATION)
    set(IOS_CONFIGURATION Release)
endif()

if(NOT DEFINED IOS_EXPORT_OPTIONS_PLIST)
    set(IOS_EXPORT_OPTIONS_PLIST "${CMAKE_CURRENT_LIST_DIR}/exportOptions.plist")
endif()

if(NOT EXISTS "${IOS_EXPORT_OPTIONS_PLIST}")
    message(FATAL_ERROR "Export options plist not found: ${IOS_EXPORT_OPTIONS_PLIST}")
endif()

set(ARCHIVE_DIR "${M3_ROOT}/packaging/ios/archive")
set(EXPORT_DIR "${ARCHIVE_DIR}/export")
file(MAKE_DIRECTORY "${ARCHIVE_DIR}")
file(MAKE_DIRECTORY "${EXPORT_DIR}")

set(ARCHIVE_PATH "${ARCHIVE_DIR}/${IOS_SCHEME}.xcarchive")

function(m3_run)
    execute_process(COMMAND ${ARGV} RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Command failed (exit ${rc}): ${ARGV}")
    endif()
endfunction()

if(DEFINED IOS_PROJECT)
    m3_run(${XCODEBUILD_EXECUTABLE}
        -project "${IOS_PROJECT}"
        -scheme "${IOS_SCHEME}"
        -configuration "${IOS_CONFIGURATION}"
        -archivePath "${ARCHIVE_PATH}"
        archive)
else()
    m3_run(${XCODEBUILD_EXECUTABLE}
        -workspace "${IOS_WORKSPACE}"
        -scheme "${IOS_SCHEME}"
        -configuration "${IOS_CONFIGURATION}"
        -archivePath "${ARCHIVE_PATH}"
        archive)
endif()

m3_run(${XCODEBUILD_EXECUTABLE}
    -exportArchive
    -archivePath "${ARCHIVE_PATH}"
    -exportOptionsPlist "${IOS_EXPORT_OPTIONS_PLIST}"
    -exportPath "${EXPORT_DIR}")

message(STATUS "IPA export complete: ${EXPORT_DIR}")
