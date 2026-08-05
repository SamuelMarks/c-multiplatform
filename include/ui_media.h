#ifndef UI_MEDIA_H
#define UI_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>

#include "ui_types.h"
/* clang-format on */

/**
 * @brief Enum defining types of media streams within a source.
 */
enum ui_media_stream_type {
  UI_MEDIA_STREAM_UNKNOWN = 0,
  UI_MEDIA_STREAM_AUDIO,
  UI_MEDIA_STREAM_VIDEO,
  UI_MEDIA_STREAM_SUBTITLES
};

/**
 * @brief Structure representing a demuxed, encoded media packet.
 */
struct ui_media_packet {
  void *data;
  size_t size;
  ui_int64 pts; /* Presentation timestamp in microseconds */
  ui_int64 dts; /* Decoding timestamp in microseconds */
  int stream_index;
  int is_keyframe;
};

/**
 * @brief Abstract vtable for demuxing and decoding media files/streams.
 */
struct ui_media_source {
  /**
   * @brief Opens a media source from a given URI or file path.
   *
   * @param source The media source instance.
   * @param uri The URI to open.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*open)(struct ui_media_source *source, const char *uri);

  /**
   * @brief Reads the next encoded packet from the demuxer.
   *
   * @param source The media source instance.
   * @param out_packet Pointer to receive the allocated packet. Must be freed by
   * caller.
   * @return UI_ERROR_NONE on success, or an EOF error if finished.
   */
  ui_error_t (*read_packet)(struct ui_media_source *source,
                            struct ui_media_packet *out_packet);

  /**
   * @brief Seeks to a specific timestamp in microseconds.
   *
   * @param source The media source instance.
   * @param timestamp_us Target time in microseconds.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*seek)(struct ui_media_source *source, ui_int64 timestamp_us);

  /**
   * @brief Gets the total duration of the media in microseconds.
   *
   * @param source The media source instance.
   * @param out_duration_us Pointer to receive the duration.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*get_duration)(struct ui_media_source *source,
                             ui_int64 *out_duration_us);

  /**
   * @brief Closes the media source and frees associated resources.
   *
   * @param source The media source instance.
   * @return UI_ERROR_NONE on success.
   */
  ui_error_t (*close)(struct ui_media_source *source);

  /**
   * @brief Opaque user data for the backend implementation.
   */
  void *user_data;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_MEDIA_H */
