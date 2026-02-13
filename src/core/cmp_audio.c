#include "cmpc/cmp_audio.h"

#include <string.h>

#define CMP_AUDIO_VTABLE_COMPLETE(vtable)                                      \
  ((vtable)->decode != NULL && (vtable)->free_audio != NULL)

static int cmp_audio_read_u16_le(const cmp_u8 *data, cmp_usize size,
                                 cmp_usize offset, cmp_u16 *out_value) {
  if (data == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (offset + 2u > size) {
    return CMP_ERR_CORRUPT;
  }

  *out_value = (cmp_u16)(data[offset] | ((cmp_u16)data[offset + 1u] << 8u));
  return CMP_OK;
}

static int cmp_audio_read_u32_le(const cmp_u8 *data, cmp_usize size,
                                 cmp_usize offset, cmp_u32 *out_value) {
  if (data == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (offset + 4u > size) {
    return CMP_ERR_CORRUPT;
  }

  *out_value = (cmp_u32)data[offset] | ((cmp_u32)data[offset + 1u] << 8u) |
               ((cmp_u32)data[offset + 2u] << 16u) |
               ((cmp_u32)data[offset + 3u] << 24u);
  return CMP_OK;
}

static int cmp_audio_decode_wav(const CMPAudioDecodeRequest *request,
                                const CMPAllocator *allocator,
                                CMPAudioData *out_audio) {
  const cmp_u8 *data;
  cmp_usize offset;
  cmp_usize chunk_size;
  cmp_u32 riff_size;
  cmp_u16 audio_format;
  cmp_u16 channels;
  cmp_u32 sample_rate;
  cmp_u16 block_align;
  cmp_u16 bits_per_sample;
  const cmp_u8 *data_chunk;
  cmp_usize data_size;
  cmp_usize frames;
  cmp_u8 *pcm;
  int rc;

  if (request == NULL || allocator == NULL || out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->data == NULL || request->size < 12u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  data = (const cmp_u8 *)request->data;
  if (memcmp(data, "RIFF", 4u) != 0 || memcmp(data + 8u, "WAVE", 4u) != 0) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = cmp_audio_read_u32_le(data, request->size, 4u, &riff_size);
  if (rc != CMP_OK) {
    return rc;
  }
  if (riff_size + 8u > request->size) {
    return CMP_ERR_CORRUPT;
  }

  audio_format = 0u;
  channels = 0u;
  sample_rate = 0u;
  block_align = 0u;
  bits_per_sample = 0u;
  data_chunk = NULL;
  data_size = 0u;

  offset = 12u;
  while (offset + 8u <= request->size) {
    cmp_u32 chunk_id;
    cmp_u32 chunk_len;

    rc = cmp_audio_read_u32_le(data, request->size, offset, &chunk_id);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_audio_read_u32_le(data, request->size, offset + 4u, &chunk_len);
    if (rc != CMP_OK) {
      return rc;
    }

    offset += 8u;
    chunk_size = (cmp_usize)chunk_len;
    if (offset + chunk_size > request->size) {
      return CMP_ERR_CORRUPT;
    }

    if (chunk_id == 0x20746d66u) {
      if (chunk_size < 16u) {
        return CMP_ERR_CORRUPT;
      }
      rc = cmp_audio_read_u16_le(data, request->size, offset, &audio_format);
      if (rc != CMP_OK) {
        return rc;
      }
      rc = cmp_audio_read_u16_le(data, request->size, offset + 2u, &channels);
      if (rc != CMP_OK) {
        return rc;
      }
      rc =
          cmp_audio_read_u32_le(data, request->size, offset + 4u, &sample_rate);
      if (rc != CMP_OK) {
        return rc;
      }
      rc = cmp_audio_read_u16_le(data, request->size, offset + 12u,
                                 &block_align);
      if (rc != CMP_OK) {
        return rc;
      }
      rc = cmp_audio_read_u16_le(data, request->size, offset + 14u,
                                 &bits_per_sample);
      if (rc != CMP_OK) {
        return rc;
      }
    } else if (chunk_id == 0x61746164u) {
      data_chunk = data + offset;
      data_size = chunk_size;
    }

    offset += chunk_size;
    if ((chunk_size & 1u) != 0u) {
      offset += 1u;
    }
  }

  if (audio_format == 0u || channels == 0u || sample_rate == 0u ||
      block_align == 0u || bits_per_sample == 0u || data_chunk == NULL) {
    return CMP_ERR_CORRUPT;
  }
  if (audio_format != 1u) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (bits_per_sample != 16u) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (block_align != (cmp_u16)(channels * 2u)) {
    return CMP_ERR_CORRUPT;
  }
  if (data_size % block_align != 0u) {
    return CMP_ERR_CORRUPT;
  }

  frames = data_size / block_align;

  rc = allocator->alloc(allocator->ctx, data_size, (void **)&pcm);
  if (rc != CMP_OK) {
    return rc;
  }

  memcpy(pcm, data_chunk, (size_t)data_size);

  out_audio->format = CMP_AUDIO_FORMAT_S16;
  out_audio->channels = channels;
  out_audio->sample_rate = sample_rate;
  out_audio->frames = frames;
  out_audio->data = pcm;
  out_audio->size = data_size;
  out_audio->flags = 0u;
  return CMP_OK;
}

int CMP_CALL cmp_audio_config_init(CMPAudioConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->env = NULL;
  config->allocator = NULL;
  return CMP_OK;
}

int CMP_CALL cmp_audio_request_init(CMPAudioDecodeRequest *request) {
  if (request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  request->utf8_path = NULL;
  request->data = NULL;
  request->size = 0u;
  request->encoding = CMP_AUDIO_ENCODING_AUTO;
  return CMP_OK;
}

int CMP_CALL cmp_audio_init(CMPAudioDecoder *decoder,
                            const CMPAudioConfig *config) {
  CMPAudio audio;
  CMPAllocator allocator;
  int rc;

  if (decoder == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_FALSE || decoder->audio.vtable != NULL ||
      decoder->audio.ctx != NULL) {
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

  decoder->audio.ctx = NULL;
  decoder->audio.vtable = NULL;
  decoder->has_backend = CMP_FALSE;

  if (config->env != NULL && config->env->vtable != NULL &&
      config->env->vtable->get_audio != NULL) {
    memset(&audio, 0, sizeof(audio));
    rc = config->env->vtable->get_audio(config->env->ctx, &audio);
    if (rc == CMP_OK) {
      if (audio.ctx == NULL || audio.vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
      if (CMP_AUDIO_VTABLE_COMPLETE(audio.vtable)) {
        decoder->audio = audio;
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

int CMP_CALL cmp_audio_shutdown(CMPAudioDecoder *decoder) {
  if (decoder == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  memset(decoder, 0, sizeof(*decoder));
  return CMP_OK;
}

int CMP_CALL cmp_audio_decode(CMPAudioDecoder *decoder,
                              const CMPAudioDecodeRequest *request,
                              CMPAudioData *out_audio) {
  int rc;

  if (decoder == NULL || request == NULL || out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (decoder->allocator.alloc == NULL || decoder->allocator.realloc == NULL ||
      decoder->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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

  memset(out_audio, 0, sizeof(*out_audio));

  if (decoder->has_backend == CMP_TRUE && decoder->audio.vtable != NULL &&
      decoder->audio.vtable->decode != NULL) {
    rc = decoder->audio.vtable->decode(decoder->audio.ctx, request,
                                       &decoder->allocator, out_audio);
    if (rc == CMP_OK) {
      out_audio->flags = CMP_AUDIO_DATA_FLAG_BACKEND;
      return CMP_OK;
    }
    if (rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  if (request->data == NULL || request->size == 0u) {
    return CMP_ERR_UNSUPPORTED;
  }

  if (request->encoding == CMP_AUDIO_ENCODING_AUTO) {
    rc = cmp_audio_decode_wav(request, &decoder->allocator, out_audio);
    if (rc == CMP_OK || rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
    return CMP_ERR_UNSUPPORTED;
  }
  if (request->encoding == CMP_AUDIO_ENCODING_WAV) {
    return cmp_audio_decode_wav(request, &decoder->allocator, out_audio);
  }

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_audio_free(CMPAudioDecoder *decoder, CMPAudioData *audio) {
  int rc;

  if (decoder == NULL || audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (decoder->allocator.alloc == NULL || decoder->allocator.realloc == NULL ||
      decoder->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (audio->size > 0u && audio->data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if ((audio->flags & CMP_AUDIO_DATA_FLAG_BACKEND) != 0u) {
    if (decoder->has_backend != CMP_TRUE || decoder->audio.vtable == NULL ||
        decoder->audio.vtable->free_audio == NULL) {
      return CMP_ERR_UNSUPPORTED;
    }
    rc = decoder->audio.vtable->free_audio(decoder->audio.ctx,
                                           &decoder->allocator, audio);
    if (rc != CMP_OK) {
      return rc;
    }
  } else if (audio->data != NULL) {
    rc = decoder->allocator.free(decoder->allocator.ctx, (void *)audio->data);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  memset(audio, 0, sizeof(*audio));
  return CMP_OK;
}
