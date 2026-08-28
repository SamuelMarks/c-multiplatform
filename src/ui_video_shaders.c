/**
 * @file ui_video_shaders.c
 * @brief ui_video_shaders.c implementation.
 */
/* clang-format off */
#include "../include/ui_video_shaders.h"
#include "../include/ui_shader_manager.h"
#include <stddef.h>
/* clang-format on */

/** @brief YUV vertex shader source */
static const char *yuv_vertex_source = "attribute vec4 a_position;\n"
                                       "attribute vec2 a_texcoord;\n"
                                       "varying vec2 v_texcoord;\n"
                                       "void main() {\n"
                                       "    gl_Position = a_position;\n"
                                       "    v_texcoord = a_texcoord;\n"
                                       "}\n";

/** @brief YUV fragment shader source */
static const char *yuv_fragment_source =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_tex_y;\n"
    "uniform sampler2D u_tex_u;\n"
    "uniform sampler2D u_tex_v;\n"
    "void main() {\n"
    "    float y = texture2D(u_tex_y, v_texcoord).r;\n"
    "    float u = texture2D(u_tex_u, v_texcoord).r - 0.5;\n"
    "    float v = texture2D(u_tex_v, v_texcoord).r - 0.5;\n"
    "    float r = y + 1.402 * v;\n"
    "    float g = y - 0.344 * u - 0.714 * v;\n"
    "    float b = y + 1.772 * u;\n"
    "    gl_FragColor = vec4(r, g, b, 1.0);\n"
    "}\n";

/* \brief ui_error
 */
ui_error_t ui_video_shaders_get_yuv_program(struct ui_shader_manager *manager,
                                            unsigned int *out_program_id) {
  if (!manager || !out_program_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_shader_manager_get_program(manager, "yuv_video", yuv_vertex_source,
                                       yuv_fragment_source, out_program_id);
}
