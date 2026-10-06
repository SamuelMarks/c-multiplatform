cmake_minimum_required(VERSION 3.15)
project(c-multiplatform-oauth2-sync-flow-example C)

# Set C89 standard strictly
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Check if we are building as part of the main project (in-tree)
if (NOT TARGET c-multiplatform-cdk)
    message(STATUS "Building oauth2_sync_flow example out-of-tree")

    # Resolving path to c-multiplatform
    set(LOCAL_C_MULTIPLATFORM "${CMAKE_CURRENT_SOURCE_DIR}/../c-multiplatform")
    set(LOCAL_C_MULTIPLATFORM_PARENT "${CMAKE_CURRENT_SOURCE_DIR}/../../c-multiplatform")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../CMakeLists.txt" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../include/c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino.h")
        message(STATUS "Found c-multiplatform in parent directory (standalone build in-repo)")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../.." c-multiplatform-build)
    elseif(EXISTS "${LOCAL_C_MULTIPLATFORM}/CMakeLists.txt" AND EXISTS "${LOCAL_C_MULTIPLATFORM}/include/c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino.h")
        message(STATUS "Found c-multiplatform at ../c-multiplatform")
        add_subdirectory("${LOCAL_C_MULTIPLATFORM}" c-multiplatform-build)
    elseif(EXISTS "${LOCAL_C_MULTIPLATFORM_PARENT}/CMakeLists.txt" AND EXISTS "${LOCAL_C_MULTIPLATFORM_PARENT}/include/c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino.h")
        message(STATUS "Found c-multiplatform at ../../c-multiplatform")
        add_subdirectory("${LOCAL_C_MULTIPLATFORM_PARENT}" c-multiplatform-build)
    else()
        message(STATUS "c-multiplatform not found locally, fetching from GitHub master branch...")
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

if(MSVC)
    set(OAUTH2_COMPILE_OPTIONS /W4 /WX /Gy)
else()
    set(OAUTH2_COMPILE_OPTIONS -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
endif()

if (NOT TARGET oauth2_sync_flow_example)
    add_executable(oauth2_sync_flow_example main.c oauth2_types.c offline_store.c oauth2_client.c app_state.c ui_dl_list.c views.c)
    target_compile_options(oauth2_sync_flow_example PRIVATE ${OAUTH2_COMPILE_OPTIONS})
    target_link_libraries(oauth2_sync_flow_example PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino parson)
    target_link_platform_libraries(oauth2_sync_flow_example PRIVATE)

    if (BUILD_TESTING OR C_MULTIPLATFORM_BUILD_TESTS)
        add_executable(test_oauth2_types oauth2_types.c tests/test_oauth2_types.c)
        target_compile_options(test_oauth2_types PRIVATE ${OAUTH2_COMPILE_OPTIONS})
        target_link_libraries(test_oauth2_types PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
        add_test(NAME test_oauth2_types COMMAND test_oauth2_types)

        add_executable(test_offline_store oauth2_types.c offline_store.c tests/test_offline_store.c)
        target_compile_options(test_offline_store PRIVATE ${OAUTH2_COMPILE_OPTIONS})
        target_link_libraries(test_offline_store PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino parson)
        add_test(NAME test_offline_store COMMAND test_offline_store)

        add_executable(test_oauth2_client oauth2_types.c oauth2_client.c tests/test_oauth2_client.c)
        target_compile_options(test_oauth2_client PRIVATE ${OAUTH2_COMPILE_OPTIONS})
        target_link_libraries(test_oauth2_client PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
        add_test(NAME test_oauth2_client COMMAND test_oauth2_client)

        add_executable(test_app_state oauth2_types.c offline_store.c oauth2_client.c app_state.c tests/test_app_state.c)
        target_compile_options(test_app_state PRIVATE ${OAUTH2_COMPILE_OPTIONS})
        target_link_libraries(test_app_state PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino parson)
        add_test(NAME test_app_state COMMAND test_app_state)

        add_executable(test_ui_dl_list oauth2_types.c ui_dl_list.c tests/test_ui_dl_list.c)
        target_compile_options(test_ui_dl_list PRIVATE ${OAUTH2_COMPILE_OPTIONS})
        target_link_libraries(test_ui_dl_list PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
        add_test(NAME test_ui_dl_list COMMAND test_ui_dl_list)

        add_executable(test_oauth2_views oauth2_types.c offline_store.c oauth2_client.c app_state.c ui_dl_list.c views.c tests/test_views.c)
        target_compile_options(test_oauth2_views PRIVATE ${OAUTH2_COMPILE_OPTIONS})
        target_link_libraries(test_oauth2_views PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino parson)
        add_test(NAME test_oauth2_views COMMAND test_oauth2_views)

        add_executable(test_oauth2_e2e oauth2_types.c offline_store.c oauth2_client.c app_state.c ui_dl_list.c views.c tests/test_oauth2_e2e.c)
        target_compile_options(test_oauth2_e2e PRIVATE ${OAUTH2_COMPILE_OPTIONS})
        target_link_libraries(test_oauth2_e2e PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino parson)
        add_test(NAME test_oauth2_e2e COMMAND test_oauth2_e2e)
    endif()
endif()
