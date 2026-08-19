/**
 * @file ui_video_shaders.h
 * @brief Standard shaders for hardware video decoding.
 */

#ifndef UI_VIDEO_SHADERS_H
#define UI_VIDEO_SHADERS_H

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ui_video_shaders Video Shaders
 * @brief Utilities for obtaining platform-standard video rendering shaders.
 * @{
 */

struct ui_shader_manager;

/**
 * @brief Compiles and retrieves the standard YUV-to-RGB fragment shader for
 * video rendering.
 *
 * @param manager The shader manager instance.
 * @param out_program_id Pointer to receive the compiled program ID.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_video_shaders_get_yuv_program(struct ui_shader_manager *manager,
                                            unsigned int *out_program_id);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIDEO_SHADERS_H */
