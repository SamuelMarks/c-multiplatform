#include "cmpc/cmp_video.h"

#include <string.h>

#ifdef CMP_TESTING
static cmp_u32 g_cmp_video_test_read_u32_fail_after = 0u;
static cmp_u32 g_cmp_video_test_mul_overflow_fail_after = 0u;

int CMP_CALL cmp_video_test_set_read_u32_fail_after(cmp_u32 call_count) {
  g_cmp_video_test_read_u32_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL cmp_video_test_set_mul_overflow_fail_after(cmp_u32 call_count) {
  g_cmp_video_test_mul_overflow_fail_after = call_count;
  return CMP_OK;
}
#endif

#define CMP_VIDEO_VTABLE_COMPLETE(vtable)                                      \
  ((vtable)->open != NULL && (vtable)->close != NULL &&                        \
   (vtable)->read_frame != NULL)

#define CMP_VIDEO_FALLBACK_MAGIC 0x3056334du
#define CMP_VIDEO_FALLBACK_HEADER_SIZE 28u

typedef struct CMPVideoFallbackState {
  cmp_u32 width;
  cmp_u32 height;
  cmp_u32 format;
  cmp_u32 fps_num;
  cmp_u32 fps_den;
  cmp_u32 frame_count;
  cmp_u32 frame_index;
  cmp_usize frame_size;
  cmp_u8 *frames;
} CMPVideoFallbackState;

static int cmp_video_mul_overflow(cmp_usize a, cmp_usize b,
                                  cmp_usize *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_video_test_mul_overflow_fail_after > 0u) {
    g_cmp_video_test_mul_overflow_fail_after -= 1u;
    if (g_cmp_video_test_mul_overflow_fail_after == 0u) {
      return CMP_ERR_OVERFLOW;
    }
  }
#endif
  if (a != 0 && b > ((cmp_usize) ~(cmp_usize)0) / a) {
    return CMP_ERR_OVERFLOW;
  }
  *out_value = a * b;
  return CMP_OK;
}

static int cmp_video_read_u32_le(const cmp_u8 *data, cmp_usize size,
                                 cmp_usize offset, cmp_u32 *out_value) {
  if (data == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_video_test_read_u32_fail_after > 0u) {
    g_cmp_video_test_read_u32_fail_after -= 1u;
    if (g_cmp_video_test_read_u32_fail_after == 0u) {
      return CMP_ERR_IO;
    }
  }
#endif
  if (offset + 4u > size) {
    return CMP_ERR_CORRUPT;
  }

  *out_value = (cmp_u32)data[offset] | ((cmp_u32)data[offset + 1u] << 8u) |
               ((cmp_u32)data[offset + 2u] << 16u) |
               ((cmp_u32)data[offset + 3u] << 24u);
  return CMP_OK;
}

static int cmp_video_fallback_parse(const CMPVideoOpenRequest *request,
                                    CMPVideoFallbackState *state) {
  const cmp_u8 *data;
  cmp_u32 magic;
  int rc;

  if (request == NULL || state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->data == NULL || request->size < CMP_VIDEO_FALLBACK_HEADER_SIZE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  data = (const cmp_u8 *)request->data;
  rc = cmp_video_read_u32_le(data, request->size, 0u, &magic);
  if (rc != CMP_OK) {
    return rc;
  }
  if (magic != CMP_VIDEO_FALLBACK_MAGIC) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = cmp_video_read_u32_le(data, request->size, 4u, &state->width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_video_read_u32_le(data, request->size, 8u, &state->height);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_video_read_u32_le(data, request->size, 12u, &state->format);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_video_read_u32_le(data, request->size, 16u, &state->fps_num);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_video_read_u32_le(data, request->size, 20u, &state->fps_den);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_video_read_u32_le(data, request->size, 24u, &state->frame_count);
  if (rc != CMP_OK) {
    return rc;
  }

  if (state->width == 0u || state->height == 0u || state->frame_count == 0u ||
      state->fps_num == 0u || state->fps_den == 0u) {
    return CMP_ERR_CORRUPT;
  }

  if (state->format != CMP_VIDEO_FORMAT_RGBA8 &&
      state->format != CMP_VIDEO_FORMAT_ANY) {
    return CMP_ERR_UNSUPPORTED;
  }

  return CMP_OK;
}

static int cmp_video_fallback_open(CMPVideoDecoder *decoder,
                                   const CMPVideoOpenRequest *request) {
  CMPVideoFallbackState *state;
  cmp_usize pixel_count;
  cmp_usize frame_size;
  cmp_usize total_size;
  const cmp_u8 *data;
  cmp_u8 *frames;
  int rc;
  int free_rc;

  if (decoder == NULL || request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->allocator.alloc == NULL || decoder->allocator.realloc == NULL ||
      decoder->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = decoder->allocator.alloc(decoder->allocator.ctx, sizeof(*state),
                                (void **)&state);
  if (rc != CMP_OK) {
    return rc;
  }
  memset(state, 0, sizeof(*state));

  rc = cmp_video_fallback_parse(request, state);
  if (rc != CMP_OK) {
    free_rc = decoder->allocator.free(decoder->allocator.ctx, state);
    if (free_rc != CMP_OK) {
      return free_rc;
    }
    return rc;
  }

  rc = cmp_video_mul_overflow((cmp_usize)state->width, (cmp_usize)state->height,
                              &pixel_count);
  if (rc != CMP_OK) {
    free_rc = decoder->allocator.free(decoder->allocator.ctx, state);
    if (free_rc != CMP_OK) {
      return free_rc;
    }
    return rc;
  }
  rc = cmp_video_mul_overflow(pixel_count, 4u, &frame_size);
  if (rc != CMP_OK) {
    free_rc = decoder->allocator.free(decoder->allocator.ctx, state);
    if (free_rc != CMP_OK) {
      return free_rc;
    }
    return rc;
  }
  rc = cmp_video_mul_overflow(frame_size, (cmp_usize)state->frame_count,
                              &total_size);
  if (rc != CMP_OK) {
    free_rc = decoder->allocator.free(decoder->allocator.ctx, state);
    if (free_rc != CMP_OK) {
      return free_rc;
    }
    return rc;
  }

  if (request->size < CMP_VIDEO_FALLBACK_HEADER_SIZE + total_size) {
    free_rc = decoder->allocator.free(decoder->allocator.ctx, state);
    if (free_rc != CMP_OK) {
      return free_rc;
    }
    return CMP_ERR_CORRUPT;
  }

  rc = decoder->allocator.alloc(decoder->allocator.ctx, total_size,
                                (void **)&frames);
  if (rc != CMP_OK) {
    free_rc = decoder->allocator.free(decoder->allocator.ctx, state);
    if (free_rc != CMP_OK) {
      return free_rc;
    }
    return rc;
  }

  data = (const cmp_u8 *)request->data + CMP_VIDEO_FALLBACK_HEADER_SIZE;
  memcpy(frames, data, (size_t)total_size);

  state->frame_size = frame_size;
  state->frames = frames;
  state->frame_index = 0u;

  decoder->fallback_state = state;
  decoder->using_fallback = CMP_TRUE;
  decoder->opened = CMP_TRUE;
  return CMP_OK;
}

static int cmp_video_fallback_close(CMPVideoDecoder *decoder) {
  CMPVideoFallbackState *state;
  int rc;

  if (decoder == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (CMPVideoFallbackState *)decoder->fallback_state;
  if (state == NULL) {
    return CMP_ERR_STATE;
  }

  rc = decoder->allocator.free(decoder->allocator.ctx, state->frames);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = decoder->allocator.free(decoder->allocator.ctx, state);
  if (rc != CMP_OK) {
    return rc;
  }

  decoder->fallback_state = NULL;
  decoder->using_fallback = CMP_FALSE;
  decoder->opened = CMP_FALSE;
  return CMP_OK;
}

static int cmp_video_fallback_read_frame(CMPVideoDecoder *decoder,
                                         CMPVideoFrame *out_frame,
                                         CMPBool *out_has_frame) {
  CMPVideoFallbackState *state;
  cmp_u32 timestamp_ms;
  cmp_usize offset;
  cmp_usize numerator;
  cmp_usize denominator;

  if (decoder == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (CMPVideoFallbackState *)decoder->fallback_state;
  if (state == NULL) {
    return CMP_ERR_STATE;
  }

  if (state->frame_index >= state->frame_count) {
    memset(out_frame, 0, sizeof(*out_frame));
    *out_has_frame = CMP_FALSE;
    return CMP_OK;
  }

  offset = (cmp_usize)state->frame_index * state->frame_size;
  out_frame->format = CMP_VIDEO_FORMAT_RGBA8;
  out_frame->width = state->width;
  out_frame->height = state->height;
  out_frame->data = state->frames + offset;
  out_frame->size = state->frame_size;

  numerator = (cmp_usize)state->frame_index * 1000u * state->fps_den;
  denominator = state->fps_num;
  if (denominator == 0u) {
    timestamp_ms = 0u;
  } else if (numerator > 0xFFFFFFFFu * denominator) {
    timestamp_ms = 0xFFFFFFFFu;
  } else {
    timestamp_ms = (cmp_u32)(numerator / denominator);
  }
  out_frame->timestamp_ms = timestamp_ms;

  *out_has_frame = CMP_TRUE;
  state->frame_index += 1u;
  return CMP_OK;
}

int CMP_CALL cmp_video_config_init(CMPVideoConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->env = NULL;
  config->allocator = NULL;
  return CMP_OK;
}

int CMP_CALL cmp_video_request_init(CMPVideoOpenRequest *request) {
  if (request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  request->utf8_path = NULL;
  request->data = NULL;
  request->size = 0u;
  request->encoding = CMP_VIDEO_ENCODING_AUTO;
  request->format = CMP_VIDEO_FORMAT_ANY;
  return CMP_OK;
}

int CMP_CALL cmp_video_init(CMPVideoDecoder *decoder,
                            const CMPVideoConfig *config) {
  CMPVideo video;
  CMPAllocator allocator;
  int rc;

  if (decoder == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_FALSE || decoder->video.vtable != NULL ||
      decoder->video.ctx != NULL) {
    return CMP_ERR_STATE;
  }

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  decoder->video.ctx = NULL;
  decoder->video.vtable = NULL;
  decoder->has_backend = CMP_FALSE;
  decoder->opened = CMP_FALSE;
  decoder->using_fallback = CMP_FALSE;
  decoder->fallback_state = NULL;

  if (config->env != NULL && config->env->vtable != NULL &&
      config->env->vtable->get_video != NULL) {
    memset(&video, 0, sizeof(video));
    rc = config->env->vtable->get_video(config->env->ctx, &video);
    if (rc == CMP_OK) {
      if (video.ctx == NULL || video.vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
      if (CMP_VIDEO_VTABLE_COMPLETE(video.vtable)) {
        decoder->video = video;
        decoder->has_backend = CMP_TRUE;
      }
    } else if (rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  decoder->allocator = allocator;
  decoder->ready = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_video_shutdown(CMPVideoDecoder *decoder) {
  int rc;

  if (decoder == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  if (decoder->opened == CMP_TRUE) {
    rc = cmp_video_close(decoder);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  memset(decoder, 0, sizeof(*decoder));
  return CMP_OK;
}

int CMP_CALL cmp_video_open(CMPVideoDecoder *decoder,
                            const CMPVideoOpenRequest *request) {
  int rc;

  if (decoder == NULL || request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (decoder->opened != CMP_FALSE) {
    return CMP_ERR_STATE;
  }
  if (request->size > 0u && request->data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->data != NULL && request->size == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->data == NULL && request->utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (decoder->has_backend == CMP_TRUE && decoder->video.vtable != NULL &&
      decoder->video.vtable->open != NULL) {
    rc = decoder->video.vtable->open(decoder->video.ctx, request);
    if (rc == CMP_OK) {
      decoder->opened = CMP_TRUE;
      decoder->using_fallback = CMP_FALSE;
      return CMP_OK;
    }
    if (rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  if (request->data == NULL || request->size == 0u) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (request->format != CMP_VIDEO_FORMAT_ANY &&
      request->format != CMP_VIDEO_FORMAT_RGBA8) {
    return CMP_ERR_UNSUPPORTED;
  }

  if (request->encoding == CMP_VIDEO_ENCODING_AUTO ||
      request->encoding == CMP_VIDEO_ENCODING_M3V0) {
    rc = cmp_video_fallback_open(decoder, request);
    return rc;
  }

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_video_close(CMPVideoDecoder *decoder) {
  int rc;

  if (decoder == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (decoder->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  if (decoder->using_fallback == CMP_TRUE) {
    rc = cmp_video_fallback_close(decoder);
    if (rc != CMP_OK) {
      return rc;
    }
    return CMP_OK;
  }

  if (decoder->video.vtable == NULL || decoder->video.vtable->close == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = decoder->video.vtable->close(decoder->video.ctx);
  if (rc != CMP_OK) {
    return rc;
  }

  decoder->opened = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_video_read_frame(CMPVideoDecoder *decoder,
                                  CMPVideoFrame *out_frame,
                                  CMPBool *out_has_frame) {
  int rc;

  if (decoder == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (decoder->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  if (decoder->using_fallback == CMP_TRUE) {
    return cmp_video_fallback_read_frame(decoder, out_frame, out_has_frame);
  }

  if (decoder->video.vtable == NULL ||
      decoder->video.vtable->read_frame == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = decoder->video.vtable->read_frame(decoder->video.ctx, out_frame,
                                         out_has_frame);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_video_test_mul_overflow(cmp_usize a, cmp_usize b,
                                         cmp_usize *out_value) {
  return cmp_video_mul_overflow(a, b, out_value);
}

int CMP_CALL cmp_video_test_read_u32_le(const cmp_u8 *data, cmp_usize size,
                                        cmp_usize offset, cmp_u32 *out_value) {
  return cmp_video_read_u32_le(data, size, offset, out_value);
}

int CMP_CALL cmp_video_test_fallback_parse(const CMPVideoOpenRequest *request) {
  CMPVideoFallbackState state;

  memset(&state, 0, sizeof(state));
  return cmp_video_fallback_parse(request, &state);
}

int CMP_CALL cmp_video_test_fallback_open(CMPVideoDecoder *decoder,
                                          const CMPVideoOpenRequest *request) {
  return cmp_video_fallback_open(decoder, request);
}

int CMP_CALL cmp_video_test_fallback_close(CMPVideoDecoder *decoder) {
  return cmp_video_fallback_close(decoder);
}

int CMP_CALL cmp_video_test_fallback_read_frame_raw(CMPVideoDecoder *decoder,
                                                    CMPVideoFrame *out_frame,
                                                    CMPBool *out_has_frame) {
  return cmp_video_fallback_read_frame(decoder, out_frame, out_has_frame);
}

int CMP_CALL cmp_video_test_fallback_read_frame_case(
    cmp_u32 width, cmp_u32 height, cmp_u32 fps_num, cmp_u32 fps_den,
    cmp_u32 frame_count, cmp_u32 frame_index, cmp_usize frame_size,
    CMPVideoFrame *out_frame, CMPBool *out_has_frame) {
  CMPVideoFallbackState state;
  CMPVideoDecoder decoder;
  static cmp_u8 dummy[4];

  memset(&state, 0, sizeof(state));
  state.width = width;
  state.height = height;
  state.fps_num = fps_num;
  state.fps_den = fps_den;
  state.frame_count = frame_count;
  state.frame_index = frame_index;
  state.frame_size = frame_size;
  state.frames = dummy;

  memset(&decoder, 0, sizeof(decoder));
  decoder.fallback_state = &state;

  return cmp_video_fallback_read_frame(&decoder, out_frame, out_has_frame);
}
#endif
