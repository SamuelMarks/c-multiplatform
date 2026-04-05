cmake_minimum_required(VERSION 3.14)
project(cmp_material_catalog C)

set(CMAKE_C_STANDARD 90)

include(FetchContent)

if(NOT TARGET cmp)
    # Prevent infinite recursion: disable example building in the parent framework when fetched
    set(CMP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/cmp.h")
        FetchContent_Declare(cmp SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
    else()
        FetchContent_Declare(cmp GIT_REPOSITORY https://github.com/SamuelMarks/c-multiplatform.git GIT_TAG master UPDATE_COMMAND git fetch origin master COMMAND git reset --hard origin/master)
    endif()
    FetchContent_MakeAvailable(cmp)
endif()

if(NOT TARGET cfs)
    FetchContent_Declare(cfs GIT_REPOSITORY https://github.com/SamuelMarks/c-fs.git GIT_TAG master UPDATE_COMMAND git fetch origin master COMMAND git reset --hard origin/master)
    FetchContent_MakeAvailable(cfs)
endif()

if(NOT TARGET c-abstract-http)
    FetchContent_Declare(c_abstract_http GIT_REPOSITORY https://github.com/SamuelMarks/c-abstract-http.git GIT_TAG master UPDATE_COMMAND git fetch origin master COMMAND git reset --hard origin/master)
    FetchContent_MakeAvailable(c_abstract_http)
endif()

if(NOT TARGET c-orm)
    FetchContent_Declare(c_orm GIT_REPOSITORY https://github.com/SamuelMarks/c-orm.git GIT_TAG master UPDATE_COMMAND git fetch origin master COMMAND git reset --hard origin/master)
    FetchContent_MakeAvailable(c_orm)
endif()

if(NOT TARGET c_cdd)
    FetchContent_Declare(cdd_c GIT_REPOSITORY https://github.com/SamuelMarks/cdd-c.git GIT_TAG master UPDATE_COMMAND git fetch origin master COMMAND git reset --hard origin/master)
    FetchContent_MakeAvailable(cdd_c)
endif()

add_executable(material_catalog 
    src/main.c
    src/material_catalog.c
)

target_include_directories(material_catalog PRIVATE include)
target_link_libraries(material_catalog PRIVATE cmp cfs c-abstract-http c-orm c_cdd)

if(NOT TARGET greatest)
    FetchContent_Declare(greatest URL https://raw.githubusercontent.com/SamuelMarks/greatest/refs/heads/cmake-and-msvc/greatest.h DOWNLOAD_NO_EXTRACT TRUE)
    FetchContent_MakeAvailable(greatest)
    add_library(greatest INTERFACE)
    target_include_directories(greatest INTERFACE ${greatest_SOURCE_DIR})
endif()

add_executable(test_material_catalog
    tests/test_material_catalog.c
    src/material_catalog.c
)
target_include_directories(test_material_catalog PRIVATE include)
target_link_libraries(test_material_catalog PRIVATE cmp cfs c-abstract-http c-orm c_cdd greatest)

add_executable(screenshot_generator
    tests/screenshot_generator.c
    src/material_catalog.c
)
target_include_directories(screenshot_generator PRIVATE include)
target_link_libraries(screenshot_generator PRIVATE cmp cfs c-abstract-http c-orm c_cdd)

enable_testing()
add_test(NAME test_material_catalog COMMAND test_material_catalog)
if(DEFINED TEST_ENV_VARS)
    set_tests_properties(test_material_catalog PROPERTIES ENVIRONMENT "${TEST_ENV_VARS}")
endif()
