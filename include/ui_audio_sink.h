#ifndef UI_AUDIO_SINK_H
#define UI_AUDIO_SINK_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>

#include "ui_types.h"
/* clang-format on */

/**
 * @brief Opaque handle representing an active audio sink.
 */
struct ui_audio_sink;

/**
 * @brief Configuration for creating an audio sink.
 */
struct ui_audio_sink_config {
  int sample_rate; /**< E.g., 44100 or 48000 */
  int channels;    /**< E.g., 2 for stereo */
  int frame_size;  /**< Number of bytes per frame (e.g., 4 for 16-bit stereo) */
};

/**
 * @brief Abstract vtable for platform-specific audio output sinks.
 */
struct ui_audio_sink_backend {
  /**
   * @brief Creates a new audio sink.
   *
   * @param backend The backend instance.
   * @param config The desired audio configuration.
   * @param out_sink Pointer to receive the allocated sink handle.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*create_sink)(struct ui_audio_sink_backend *backend,
                            const struct ui_audio_sink_config *config,
                            struct ui_audio_sink **out_sink);

  /**
   * @brief Destroys an audio sink.
   *
   * @param backend The backend instance.
   * @param sink The sink to destroy.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*destroy_sink)(struct ui_audio_sink_backend *backend,
                             struct ui_audio_sink *sink);

  /**
   * @brief Writes interleaved audio frames to the sink.
   *
   * @param backend The backend instance.
   * @param sink The sink.
   * @param frames Pointer to the audio data.
   * @param num_frames Number of frames to write.
   * @param out_frames_written Pointer to receive the actual number of frames
   * written.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*write_frames)(struct ui_audio_sink_backend *backend,
                             struct ui_audio_sink *sink, const void *frames,
                             int num_frames, int *out_frames_written);

  /**
   * @brief Gets the current playback delay (latency) in microseconds.
   *
   * @param backend The backend instance.
   * @param sink The sink.
   * @param out_delay_us Pointer to receive the delay.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*get_delay)(struct ui_audio_sink_backend *backend,
                          struct ui_audio_sink *sink, ui_int64 *out_delay_us);

  /**
   * @brief Starts playback on the sink.
   *
   * @param backend The backend instance.
   * @param sink The sink.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*start)(struct ui_audio_sink_backend *backend,
                      struct ui_audio_sink *sink);

  /**
   * @brief Stops playback on the sink.
   *
   * @param backend The backend instance.
   * @param sink The sink.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*stop)(struct ui_audio_sink_backend *backend,
                     struct ui_audio_sink *sink);

  /**
   * @brief Opaque user data for the specific backend implementation.
   */
  void *user_data;
};

/**
 * @brief Gets the platform-appropriate WASAPI/CoreAudio/ALSA backend.
 *
 * @param out_backend Pointer to receive the backend struct.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_AUDIO_SINK_H */
