cmake_minimum_required(VERSION 3.16)
project(oauth2-gui VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Determine if c-multiplatform is two dirs above
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../CMakeLists.txt")
    message(STATUS "Found c-multiplatform in parent directory")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/cmp_build EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(
        c_multiplatform
        GIT_REPOSITORY https://github.com/SamuelMarks/c-multiplatform.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(c_multiplatform)
endif()

# We also need c-abstract-http, c-orm (these might be brought in by c-multiplatform, but we ensure they are available)
find_package(c_abstract_http QUIET)
if(NOT TARGET c_abstract_http AND NOT TARGET c-abstract-http)
    include(FetchContent)
    FetchContent_Declare(
        c_abstract_http
        GIT_REPOSITORY https://github.com/SamuelMarks/c-abstract-http.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(c_abstract_http)
endif()

find_package(c_orm QUIET)
if(NOT TARGET c_orm AND NOT TARGET c-orm)
    include(FetchContent)
    FetchContent_Declare(
        c_orm
        GIT_REPOSITORY https://github.com/SamuelMarks/c-orm.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(c_orm)
endif()

add_library(oauth2_gui_lib STATIC
    src/oauth2_gui.c
)
target_include_directories(oauth2_gui_lib PUBLIC include)
target_link_libraries(oauth2_gui_lib PUBLIC cmpc m3 f2 cupertino c-abstract-http c-orm)

add_executable(oauth2_gui_app src/main.c)
target_link_libraries(oauth2_gui_app PRIVATE oauth2_gui_lib)

# Tests
option(OAUTH2_GUI_BUILD_TESTS "Build tests" ON)
if(OAUTH2_GUI_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        greatest
        URL https://raw.githubusercontent.com/SamuelMarks/greatest/cmake-and-msvc/greatest.h
        DOWNLOAD_NO_EXTRACT TRUE
    )
    FetchContent_MakeAvailable(greatest)

    FetchContent_GetProperties(greatest SOURCE_DIR greatest_DIR)

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests/actual")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests/expected")
    
    add_executable(oauth2_gui_tests tests/test_oauth2_gui.c tests/test_main.c)
    target_compile_definitions(oauth2_gui_tests PRIVATE OAUTH2_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_include_directories(oauth2_gui_tests PRIVATE ${greatest_DIR} include)
    target_link_libraries(oauth2_gui_tests PRIVATE oauth2_gui_lib)
    add_test(NAME oauth2_gui_tests COMMAND oauth2_gui_tests)
endif()
