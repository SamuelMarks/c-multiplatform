/**
 * @file ui_video_decoder.h
 * @brief Abstraction layer for hardware video decoding.
 */

#ifndef UI_VIDEO_DECODER_H
#define UI_VIDEO_DECODER_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ui_video_decoder Video Decoder
 * @brief Abstraction layer for hardware video decoding.
 * @{
 */

/**
 * @struct ui_video_decoder
 * @brief Opaque handle representing an active video decoder.
 */
struct ui_video_decoder;

/**
 * @struct ui_video_decoder_config
 * @brief Configuration for creating a video decoder.
 */
struct ui_video_decoder_config {
  int width;    /**< The width of the video. */
  int height;   /**< The height of the video. */
  int codec_id; /**< Generic codec ID, e.g., H264, HEVC, VP9. */
};

/**
 * @struct ui_video_frame
 * @brief Represents a decoded video frame.
 */
struct ui_video_frame {
  void *data[3];   /**< Planar data pointers (e.g., Y, U, V or RGBA). */
  int linesize[3]; /**< Stride for each plane. */
  int width;       /**< Width of the frame. */
  int height;      /**< Height of the frame. */
  int format;      /**< Pixel format enum. */
  ui_int64 pts;    /**< Presentation timestamp in microseconds. */
};

/**
 * @struct ui_video_decoder_backend
 * @brief Abstract vtable for platform-specific hardware video decoding.
 */
struct ui_video_decoder_backend {
  /**
   * @brief Creates a new video decoder.
   *
   * @param backend The backend instance.
   * @param config The desired decoder configuration.
   * @param out_decoder Pointer to receive the allocated decoder handle.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*create_decoder)(struct ui_video_decoder_backend *backend,
                               const struct ui_video_decoder_config *config,
                               struct ui_video_decoder **out_decoder);

  /**
   * @brief Destroys a video decoder.
   *
   * @param backend The backend instance.
   * @param decoder The decoder to destroy.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*destroy_decoder)(struct ui_video_decoder_backend *backend,
                                struct ui_video_decoder *decoder);

  /**
   * @brief Submits a compressed packet to the decoder.
   *
   * @param backend The backend instance.
   * @param decoder The decoder.
   * @param packet_data Pointer to the compressed data.
   * @param packet_size Size of the compressed data.
   * @param pts Presentation timestamp in microseconds.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*decode_packet)(struct ui_video_decoder_backend *backend,
                              struct ui_video_decoder *decoder,
                              const void *packet_data, size_t packet_size,
                              ui_int64 pts);

  /**
   * @brief Retrieves the next decoded frame from the decoder, if available.
   *
   * @param backend The backend instance.
   * @param decoder The decoder.
   * @param out_frame Pointer to receive the decoded frame. Must be released
   * later.
   * @return UI_ERROR_NONE on success, or an error if no frame is ready yet.
   */
  ui_error_t (*get_frame)(struct ui_video_decoder_backend *backend,
                          struct ui_video_decoder *decoder,
                          struct ui_video_frame *out_frame);

  /**
   * @brief Releases a decoded frame obtained via get_frame.
   *
   * @param backend The backend instance.
   * @param decoder The decoder.
   * @param frame The frame to release.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*release_frame)(struct ui_video_decoder_backend *backend,
                              struct ui_video_decoder *decoder,
                              struct ui_video_frame *frame);

  void *user_data; /**< Opaque user data for the specific backend
                      implementation. */
};

/**
 * @brief Gets the platform-appropriate hardware video decoder backend.
 *
 * @param out_backend Pointer to receive the backend struct.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_video_decoder_get_default_backend(
    struct ui_video_decoder_backend *out_backend);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIDEO_DECODER_H */
