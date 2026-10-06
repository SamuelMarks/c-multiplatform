cmake_minimum_required(VERSION 3.15)
project(compose_material_catalog C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SAMPLER_SOURCES
    ../sampler/src/sampler/sampler_error.c
    ../sampler/src/sampler/sampler_models.c
    ../sampler/src/sampler/sampler_theme.c
    ../sampler/src/sampler/sampler_preferences.c
    ../sampler/src/sampler/sampler_nav.c
    ../sampler/src/sampler/sampler_top_app_bar.c
    ../sampler/src/sampler/sampler_specification_screen.c
    ../sampler/src/sampler/sampler_home.c
    ../sampler/src/sampler/sampler_component_screen.c
    ../sampler/src/sampler/sampler_example_screen.c
    ../sampler/src/sampler/sampler_theme_picker.c
    ../sampler/src/sampler/m2/m2_home.c
    ../sampler/src/sampler/samples/sample_buttons.c
    ../sampler/src/sampler/samples/sample_stub.c
)

add_executable(compose_material_catalog main.c ${SAMPLER_SOURCES})
target_include_directories(compose_material_catalog PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../sampler/include
)
target_link_libraries(compose_material_catalog PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
target_link_platform_libraries(compose_material_catalog PRIVATE)
if(NOT MSVC)
    target_link_libraries(compose_material_catalog PRIVATE m)
endif()
