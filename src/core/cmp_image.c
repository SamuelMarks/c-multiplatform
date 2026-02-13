#include "cmpc/cmp_image.h"

#include <string.h>

#define CMP_IMAGE_VTABLE_COMPLETE(vtable)                                      \
  ((vtable)->decode != NULL && (vtable)->free_image != NULL)

static int cmp_image_mul_overflow(cmp_usize a, cmp_usize b,
                                  cmp_usize *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (a != 0 && b > ((cmp_usize) ~(cmp_usize)0) / a) {
    return CMP_ERR_OVERFLOW;
  }
  *out_value = a * b;
  return CMP_OK;
}

static int cmp_image_ppm_skip_ws(const cmp_u8 *data, cmp_usize size,
                                 cmp_usize *offset) {
  cmp_u8 ch;

  if (data == NULL || offset == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  while (*offset < size) {
    ch = data[*offset];
    if (ch == '#') {
      while (*offset < size && data[*offset] != '\n' && data[*offset] != '\r') {
        *offset += 1u;
      }
      continue;
    }
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' ||
        ch == '\v') {
      *offset += 1u;
      continue;
    }
    return CMP_OK;
  }

  return CMP_ERR_CORRUPT;
}

static int cmp_image_ppm_read_uint(const cmp_u8 *data, cmp_usize size,
                                   cmp_usize *offset, cmp_u32 *out_value) {
  cmp_u32 value;
  cmp_u32 max_div10;
  cmp_u32 max_mod10;
  cmp_u8 ch;
  int rc;

  if (data == NULL || offset == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_image_ppm_skip_ws(data, size, offset);
  if (rc != CMP_OK) {
    return rc;
  }
  if (*offset >= size) {
    return CMP_ERR_CORRUPT;
  }

  ch = data[*offset];
  if (ch < '0' || ch > '9') {
    return CMP_ERR_CORRUPT;
  }

  value = 0u;
  max_div10 = 0xFFFFFFFFu / 10u;
  max_mod10 = 0xFFFFFFFFu % 10u;

  while (*offset < size) {
    ch = data[*offset];
    if (ch < '0' || ch > '9') {
      break;
    }
    if (value > max_div10 ||
        (value == max_div10 && (cmp_u32)(ch - '0') > max_mod10)) {
      return CMP_ERR_OVERFLOW;
    }
    value = (value * 10u) + (cmp_u32)(ch - '0');
    *offset += 1u;
  }

  *out_value = value;
  return CMP_OK;
}

static int cmp_image_decode_ppm(const CMPImageDecodeRequest *request,
                                const CMPAllocator *allocator,
                                CMPImageData *out_image) {
  const cmp_u8 *data;
  cmp_u8 *pixels;
  cmp_usize offset;
  cmp_usize rgb_size;
  cmp_usize rgba_size;
  cmp_usize pixel_count;
  cmp_usize pixel_idx;
  cmp_u32 width;
  cmp_u32 height;
  cmp_u32 maxval;
  int rc;

  if (request == NULL || allocator == NULL || out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->data == NULL || request->size == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  data = (const cmp_u8 *)request->data;
  if (request->size < 3u) {
    return CMP_ERR_CORRUPT;
  }
  if (data[0] != 'P' || data[1] != '6') {
    return CMP_ERR_UNSUPPORTED;
  }

  offset = 2u;
  rc = cmp_image_ppm_read_uint(data, request->size, &offset, &width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_image_ppm_read_uint(data, request->size, &offset, &height);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_image_ppm_read_uint(data, request->size, &offset, &maxval);
  if (rc != CMP_OK) {
    return rc;
  }
  if (width == 0u || height == 0u) {
    return CMP_ERR_CORRUPT;
  }
  if (maxval != 255u) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = cmp_image_ppm_skip_ws(data, request->size, &offset);
  if (rc != CMP_OK) {
    return rc;
  }

  rc =
      cmp_image_mul_overflow((cmp_usize)width, (cmp_usize)height, &pixel_count);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_image_mul_overflow(pixel_count, 3u, &rgb_size);
  if (rc != CMP_OK) {
    return rc;
  }
  if (request->size - offset < rgb_size) {
    return CMP_ERR_CORRUPT;
  }

  if (request->format != CMP_IMAGE_FORMAT_ANY &&
      request->format != CMP_IMAGE_FORMAT_RGBA8) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = cmp_image_mul_overflow(pixel_count, 4u, &rgba_size);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = allocator->alloc(allocator->ctx, rgba_size, (void **)&pixels);
  if (rc != CMP_OK) {
    return rc;
  }

  pixel_idx = 0u;
  while (pixel_idx < pixel_count) {
    cmp_usize src_offset;
    cmp_usize dst_offset;

    src_offset = offset + (pixel_idx * 3u);
    dst_offset = pixel_idx * 4u;

    pixels[dst_offset] = data[src_offset];
    pixels[dst_offset + 1u] = data[src_offset + 1u];
    pixels[dst_offset + 2u] = data[src_offset + 2u];
    pixels[dst_offset + 3u] = 255u;
    pixel_idx += 1u;
  }

  out_image->format = CMP_IMAGE_FORMAT_RGBA8;
  out_image->width = width;
  out_image->height = height;
  out_image->stride = width * 4u;
  out_image->data = pixels;
  out_image->size = rgba_size;
  out_image->flags = 0u;
  return CMP_OK;
}

static int cmp_image_decode_raw_rgba8(const CMPImageDecodeRequest *request,
                                      const CMPAllocator *allocator,
                                      CMPImageData *out_image) {
  cmp_u8 *pixels;
  cmp_usize pixel_count;
  cmp_usize row_stride;
  cmp_usize tight_stride;
  cmp_usize expected_size;
  cmp_usize copy_size;
  cmp_usize row;
  int rc;

  if (request == NULL || allocator == NULL || out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->data == NULL || request->size == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->width == 0u || request->height == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (request->format != CMP_IMAGE_FORMAT_ANY &&
      request->format != CMP_IMAGE_FORMAT_RGBA8) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = cmp_image_mul_overflow((cmp_usize)request->width,
                              (cmp_usize)request->height, &pixel_count);
  if (rc != CMP_OK) {
    return rc;
  }

  tight_stride = (cmp_usize)request->width * 4u;
  if (request->stride != 0u) {
    row_stride = request->stride;
  } else {
    row_stride = tight_stride;
  }

  rc = cmp_image_mul_overflow(row_stride, (cmp_usize)request->height,
                              &expected_size);
  if (rc != CMP_OK) {
    return rc;
  }
  if (request->size < expected_size) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_image_mul_overflow(pixel_count, 4u, &copy_size);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = allocator->alloc(allocator->ctx, copy_size, (void **)&pixels);
  if (rc != CMP_OK) {
    return rc;
  }

  if (row_stride == tight_stride) {
    memcpy(pixels, request->data, (size_t)copy_size);
  } else {
    row = 0u;
    while (row < request->height) {
      const cmp_u8 *src_row;
      cmp_u8 *dst_row;

      src_row = (const cmp_u8 *)request->data + (row * row_stride);
      dst_row = pixels + (row * tight_stride);
      memcpy(dst_row, src_row, (size_t)tight_stride);
      row += 1u;
    }
  }

  out_image->format = CMP_IMAGE_FORMAT_RGBA8;
  out_image->width = request->width;
  out_image->height = request->height;
  out_image->stride = (cmp_u32)tight_stride;
  out_image->data = pixels;
  out_image->size = copy_size;
  out_image->flags = 0u;
  return CMP_OK;
}

int CMP_CALL cmp_image_config_init(CMPImageConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->env = NULL;
  config->allocator = NULL;
  return CMP_OK;
}

int CMP_CALL cmp_image_request_init(CMPImageDecodeRequest *request) {
  if (request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  request->utf8_path = NULL;
  request->data = NULL;
  request->size = 0u;
  request->encoding = CMP_IMAGE_ENCODING_AUTO;
  request->format = CMP_IMAGE_FORMAT_ANY;
  request->width = 0u;
  request->height = 0u;
  request->stride = 0u;
  return CMP_OK;
}

int CMP_CALL cmp_image_init(CMPImageDecoder *decoder,
                            const CMPImageConfig *config) {
  CMPImage image;
  CMPAllocator allocator;
  int rc;

  if (decoder == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_FALSE || decoder->image.vtable != NULL ||
      decoder->image.ctx != NULL) {
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

  decoder->image.ctx = NULL;
  decoder->image.vtable = NULL;
  decoder->has_backend = CMP_FALSE;

  if (config->env != NULL && config->env->vtable != NULL &&
      config->env->vtable->get_image != NULL) {
    memset(&image, 0, sizeof(image));
    rc = config->env->vtable->get_image(config->env->ctx, &image);
    if (rc == CMP_OK) {
      if (image.ctx == NULL || image.vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
      if (CMP_IMAGE_VTABLE_COMPLETE(image.vtable)) {
        decoder->image = image;
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

int CMP_CALL cmp_image_shutdown(CMPImageDecoder *decoder) {
  if (decoder == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  memset(decoder, 0, sizeof(*decoder));
  return CMP_OK;
}

int CMP_CALL cmp_image_decode(CMPImageDecoder *decoder,
                              const CMPImageDecodeRequest *request,
                              CMPImageData *out_image) {
  int rc;

  if (decoder == NULL || request == NULL || out_image == NULL) {
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

  memset(out_image, 0, sizeof(*out_image));

  if (decoder->has_backend == CMP_TRUE && decoder->image.vtable != NULL &&
      decoder->image.vtable->decode != NULL) {
    rc = decoder->image.vtable->decode(decoder->image.ctx, request,
                                       &decoder->allocator, out_image);
    if (rc == CMP_OK) {
      out_image->flags = CMP_IMAGE_DATA_FLAG_BACKEND;
      return CMP_OK;
    }
    if (rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  if (request->data == NULL || request->size == 0u) {
    return CMP_ERR_UNSUPPORTED;
  }

  if (request->encoding == CMP_IMAGE_ENCODING_AUTO) {
    rc = cmp_image_decode_ppm(request, &decoder->allocator, out_image);
    if (rc == CMP_OK || rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
    return CMP_ERR_UNSUPPORTED;
  }
  if (request->encoding == CMP_IMAGE_ENCODING_PPM) {
    return cmp_image_decode_ppm(request, &decoder->allocator, out_image);
  }
  if (request->encoding == CMP_IMAGE_ENCODING_RAW_RGBA8) {
    return cmp_image_decode_raw_rgba8(request, &decoder->allocator, out_image);
  }

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_image_free(CMPImageDecoder *decoder, CMPImageData *image) {
  int rc;

  if (decoder == NULL || image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (decoder->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (decoder->allocator.alloc == NULL || decoder->allocator.realloc == NULL ||
      decoder->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (image->size > 0u && image->data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if ((image->flags & CMP_IMAGE_DATA_FLAG_BACKEND) != 0u) {
    if (decoder->has_backend != CMP_TRUE || decoder->image.vtable == NULL ||
        decoder->image.vtable->free_image == NULL) {
      return CMP_ERR_UNSUPPORTED;
    }
    rc = decoder->image.vtable->free_image(decoder->image.ctx,
                                           &decoder->allocator, image);
    if (rc != CMP_OK) {
      return rc;
    }
  } else if (image->data != NULL) {
    rc = decoder->allocator.free(decoder->allocator.ctx, (void *)image->data);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  memset(image, 0, sizeof(*image));
  return CMP_OK;
}
