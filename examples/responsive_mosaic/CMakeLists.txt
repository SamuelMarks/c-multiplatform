cmake_minimum_required(VERSION 3.15)
project(c-multiplatform-responsive-mosaic C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (NOT TARGET ui_engine)
    message(STATUS "Building responsive_mosaic example out-of-tree")
    set(LOCAL_C_MULTIPLATFORM "${CMAKE_CURRENT_SOURCE_DIR}/../c-multiplatform")
    set(LOCAL_C_MULTIPLATFORM_PARENT "${CMAKE_CURRENT_SOURCE_DIR}/../../c-multiplatform")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../CMakeLists.txt" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../include/ui_engine.h")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../.." c-multiplatform-build)
    elseif(EXISTS "${LOCAL_C_MULTIPLATFORM}/CMakeLists.txt" AND EXISTS "${LOCAL_C_MULTIPLATFORM}/include/ui_engine.h")
        add_subdirectory("${LOCAL_C_MULTIPLATFORM}" c-multiplatform-build)
    elseif(EXISTS "${LOCAL_C_MULTIPLATFORM_PARENT}/CMakeLists.txt" AND EXISTS "${LOCAL_C_MULTIPLATFORM_PARENT}/include/ui_engine.h")
        add_subdirectory("${LOCAL_C_MULTIPLATFORM_PARENT}" c-multiplatform-build)
    else()
        include(FetchContent)
        FetchContent_Declare(
            c_multiplatform
            GIT_REPOSITORY https://github.com/SamuelMarks/c-multiplatform.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(c_multiplatform)
    endif()
endif()


# Locate and include PlatformLinkages.cmake
set(PLATFORM_LINKAGES_FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/PlatformLinkages.cmake")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/PlatformLinkages.cmake")
    # 0. In-tree relative path
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/PlatformLinkages.cmake")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../c-multiplatform/cmake/PlatformLinkages.cmake")
    # 1. Sibling directory relative path
    include("${CMAKE_CURRENT_SOURCE_DIR}/../c-multiplatform/cmake/PlatformLinkages.cmake")
else()
    # 2. Download fallback
    if(NOT EXISTS "${PLATFORM_LINKAGES_FILE}")
        message(STATUS "PlatformLinkages.cmake not found locally, downloading...")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmake")
        file(DOWNLOAD "https://raw.githubusercontent.com/SamuelMarks/c-multiplatform/master/cmake/PlatformLinkages.cmake" "${PLATFORM_LINKAGES_FILE}")
    endif()
    include("${PLATFORM_LINKAGES_FILE}")
endif()

add_executable(responsive_mosaic main.c)
target_link_libraries(responsive_mosaic PRIVATE ui_engine)
target_link_platform_libraries(responsive_mosaic PRIVATE)
