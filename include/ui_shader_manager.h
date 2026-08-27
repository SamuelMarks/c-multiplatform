/**
 * @file ui_shader_manager.h
 * @brief Shader compilation, caching, and uniform management.
 */

#ifndef UI_SHADER_MANAGER_H
#define UI_SHADER_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

struct ui_shader_manager;

/**
 * @brief Creates a shader manager for compiling and caching shader programs.
 *
 * @param out_manager Pointer to receive the allocated manager.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_shader_manager_create(struct ui_shader_manager **out_manager);

/**
 * @brief Destroys a shader manager, freeing all cached shaders.
 *
 * @param manager The manager to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_shader_manager_destroy(struct ui_shader_manager *manager);

/**
 * @brief Compiles and links a shader program, or returns it from the cache if
 * already compiled.
 *
 * @param manager The shader manager.
 * @param name A unique identifier for the shader program.
 * @param vertex_source The vertex shader source code.
 * @param fragment_source The fragment shader source code.
 * @param out_program_id Pointer to receive the compiled program ID (GLuint).
 * @return UI_ERROR_NONE on success, or UI_ERROR_UNKNOWN if compilation fails.
 */
ui_error_t ui_shader_manager_get_program(struct ui_shader_manager *manager,
                                         const char *name,
                                         const char *vertex_source,
                                         const char *fragment_source,
                                         unsigned int *out_program_id);

/**
 * @brief Sets a matrix uniform for the current shader.
 * @param manager The shader manager.
 * @param location The uniform location.
 * @param name The uniform name (for debugging or fallback).
 * @param matrix The 4x4 matrix data.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t
ui_shader_manager_set_uniform_matrix(struct ui_shader_manager *manager,
                                     unsigned int location, const char *name,
                                     const float *matrix);
/**
 * @brief Sets a color uniform for the current shader.
 * @param manager The shader manager.
 * @param location The uniform location.
 * @param name The uniform name (for debugging or fallback).
 * @param r The red channel (0-1).
 * @param g The green channel (0-1).
 * @param b The blue channel (0-1).
 * @param a The alpha channel (0-1).
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t
ui_shader_manager_set_uniform_color(struct ui_shader_manager *manager,
                                    unsigned int location, const char *name,
                                    float r, float g, float b, float a);
/**
 * @brief Sets a float uniform for the current shader.
 * @param manager The shader manager.
 * @param location The uniform location.
 * @param name The uniform name (for debugging or fallback).
 * @param value The float value.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t
ui_shader_manager_set_uniform_float(struct ui_shader_manager *manager,
                                    unsigned int location, const char *name,
                                    float value);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SHADER_MANAGER_H */
