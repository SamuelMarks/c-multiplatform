cmake_minimum_required(VERSION 3.15)

if(EMSCRIPTEN)

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

add_executable(web_hello main.c)
    target_link_libraries(web_hello PRIVATE ui_engine)
target_link_platform_libraries(web_hello PRIVATE)

    # We output .js. Our web/index.html will load it.
    # To use our shell template from web/index.html, we could also output .html.
    # For now, let's copy index.html to the output directory so we can easily test.
    add_custom_command(TARGET web_hello POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/web/index.html
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/index.html
        COMMAND ${CMAKE_COMMAND} -E rename
            $<TARGET_FILE:web_hello>
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/c-multiplatform-engine.js
    )
endif()
