#ifndef UI_VIDEO_SHADERS_H
#define UI_VIDEO_SHADERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

struct ui_shader_manager;

/**
 * @brief Compiles and retrieves the standard YUV-to-RGB fragment shader for
 * video rendering.
 *
 * @param manager The shader manager.
 * @param out_program_id Pointer to receive the compiled program ID.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_video_shaders_get_yuv_program(struct ui_shader_manager *manager,
                                 unsigned int *out_program_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIDEO_SHADERS_H */
