/* clang-format off */
#include "../include/ui_font_manager.h"
#include "ui_internal_mem.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wtype-limits"
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)  ((void)(u),C_MULTIPLATFORM_MALLOC(x))
#define STBTT_free(x,u)    ((void)(u),C_MULTIPLATFORM_FREE(x))
#include "../include/stb_truetype.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <string.h>
/* clang-format on */

struct ui_font {
  stbtt_fontinfo info;
  unsigned char *data;
  size_t size;
  char family[128];
  int weight;
  int is_italic;
  enum ui_font_status status;
  struct ui_font_axis *axes;
  int axis_count;
  struct ui_font *next;
};

/** \brief ui_font_manager */
struct ui_font_manager {
  struct ui_font *head;
};

ui_error_t ui_font_manager_create(struct ui_font_manager **out_manager) {
  struct ui_font_manager *manager;

  if (!out_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager = (struct ui_font_manager *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_font_manager));
  if (!manager) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  manager->head = NULL;
  *out_manager = manager;

  return UI_ERROR_NONE;
}

ui_error_t ui_font_manager_destroy(struct ui_font_manager *manager) {
  struct ui_font *current;
  struct ui_font *next;

  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = manager->head;
  while (current) {
    next = current->next;
    C_MULTIPLATFORM_FREE(current->data);
    if (current->axes) {
      C_MULTIPLATFORM_FREE(current->axes);
    }
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  C_MULTIPLATFORM_FREE(manager);
  return UI_ERROR_NONE;
}

ui_error_t ui_font_manager_load_font_memory(struct ui_font_manager *manager,
                                            const unsigned char *font_data,
                                            size_t data_size,
                                            struct ui_font **out_font) {
  struct ui_font *font;
  size_t i;
  int offset;

  if (!manager || !font_data || data_size == 0 || !out_font) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  font = (struct ui_font *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_font));
  if (!font) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(font, 0, sizeof(struct ui_font));

  font->data = (unsigned char *)C_MULTIPLATFORM_MALLOC(data_size);
  if (!font->data) {
    C_MULTIPLATFORM_FREE(font);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  for (i = 0; i < data_size; ++i) {
    font->data[i] = font_data[i];
  }
  font->size = data_size;
  font->axes = NULL;
  font->axis_count = 0;

  offset = stbtt_GetFontOffsetForIndex(font->data, 0);
  if (offset < 0 || !stbtt_InitFont(&font->info, font->data, offset)) {
    C_MULTIPLATFORM_FREE(font->data);
    C_MULTIPLATFORM_FREE(font);
    return UI_ERROR_UNKNOWN;
  }

  font->next = manager->head;
  manager->head = font;

  *out_font = font;
  return UI_ERROR_NONE;
}

ui_error_t ui_font_get_glyph_metrics(struct ui_font *font, int codepoint,
                                     float font_size,
                                     struct ui_glyph_metrics *out_metrics) {
  float scale;
  int advance, lsb;
  int x0, y0, x1, y1;

  if (!font || !out_metrics) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scale = stbtt_ScaleForPixelHeight(&font->info, font_size);
  stbtt_GetCodepointHMetrics(&font->info, codepoint, &advance, &lsb);
  stbtt_GetCodepointBitmapBox(&font->info, codepoint, scale, scale, &x0, &y0,
                              &x1, &y1);

  out_metrics->width = x1 - x0;
  out_metrics->height = y1 - y0;
  out_metrics->bearing_x = (int)(lsb * scale);
  out_metrics->bearing_y = -y0; /* usually y0 is negative for bearing */
  out_metrics->advance = (int)(advance * scale);

  return UI_ERROR_NONE;
}

ui_error_t ui_font_get_vmetrics(struct ui_font *font, float font_size,
                                float *out_ascent, float *out_descent,
                                float *out_line_gap) {
  float scale;
  int ascent, descent, line_gap;

  if (!font || !out_ascent || !out_descent || !out_line_gap) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scale = stbtt_ScaleForPixelHeight(&font->info, font_size);
  stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &line_gap);

  *out_ascent = (float)ascent * scale;
  *out_descent = (float)descent * scale;
  *out_line_gap = (float)line_gap * scale;

  return UI_ERROR_NONE;
}

ui_error_t ui_font_get_kerning(struct ui_font *font, int codepoint1,
                               int codepoint2, float font_size,
                               float *out_kerning) {
  float scale;
  int kern;

  if (!font || !out_kerning) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scale = stbtt_ScaleForPixelHeight(&font->info, font_size);
  kern = stbtt_GetCodepointKernAdvance(&font->info, codepoint1, codepoint2);

  *out_kerning = kern * scale;
  return UI_ERROR_NONE;
}

ui_error_t ui_font_get_data(struct ui_font *font,
                            const unsigned char **out_data, size_t *out_size) {
  if (!font || !out_data || !out_size) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_data = font->data;
  *out_size = font->size;
  return UI_ERROR_NONE;
}

ui_error_t ui_font_generate_atlas(struct ui_font *font, float font_size,
                                  const int *codepoints, int codepoint_count,
                                  unsigned char **out_atlas_rgba,
                                  int *out_width, int *out_height) {
  int atlas_width = 512;
  int atlas_height = 512;
  unsigned char *alpha_pixels;
  unsigned char *rgba_pixels;
  stbtt_pack_context spc;
  stbtt_packedchar *chardata;
  int i;
  ui_error_t rc = UI_ERROR_NONE;
  int pack_success;

  if (!font || !codepoints || codepoint_count <= 0 || !out_atlas_rgba ||
      !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Start with a fixed guess for atlas size. A true system would dynamically
   * resize. */
  alpha_pixels = (unsigned char *)C_MULTIPLATFORM_MALLOC(
      (size_t)(atlas_width * atlas_height));
  if (!alpha_pixels) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  chardata = (stbtt_packedchar *)C_MULTIPLATFORM_MALLOC(
      sizeof(stbtt_packedchar) * (size_t)codepoint_count);
  if (!chardata) {
    C_MULTIPLATFORM_FREE(alpha_pixels);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  pack_success = stbtt_PackBegin(&spc, alpha_pixels, atlas_width, atlas_height,
                                 0, 1, NULL);
  if (!pack_success) {
    rc = UI_ERROR_UNKNOWN;
    goto cleanup;
  }

  /* Set up pack ranges - assuming codepoints are a single contiguous range for
     simplicity here, or we pack them one by one. But stbtt_PackFontRanges
     expects ranges. To support arbitrary arrays, we can pack them individually.
   */

  stbtt_PackSetOversampling(&spc, 1, 1);

  for (i = 0; i < codepoint_count; ++i) {
    stbtt_pack_range pr;
    pr.font_size = font_size;
    pr.first_unicode_codepoint_in_range = codepoints[i];
    pr.array_of_unicode_codepoints = NULL;
    pr.num_chars = 1;
    pr.chardata_for_range = &chardata[i];

    if (!stbtt_PackFontRanges(&spc, font->data, 0, &pr, 1)) {
      /* Fails if atlas is too small */
      rc = UI_ERROR_UNKNOWN;
      stbtt_PackEnd(&spc);
      goto cleanup;
    }
  }

  stbtt_PackEnd(&spc);

  /* Convert single-channel alpha to RGBA */
  rgba_pixels = (unsigned char *)C_MULTIPLATFORM_MALLOC(
      (size_t)(atlas_width * atlas_height * 4));
  if (!rgba_pixels) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  for (i = 0; i < atlas_width * atlas_height; ++i) {
    rgba_pixels[i * 4 + 0] = 255;
    rgba_pixels[i * 4 + 1] = 255;
    rgba_pixels[i * 4 + 2] = 255;
    rgba_pixels[i * 4 + 3] = alpha_pixels[i];
  }

  *out_atlas_rgba = rgba_pixels;
  *out_width = atlas_width;
  *out_height = atlas_height;

cleanup:
  C_MULTIPLATFORM_FREE(alpha_pixels);
  C_MULTIPLATFORM_FREE(chardata);
  return rc;
}

ui_error_t ui_font_free_atlas(unsigned char *atlas_rgba) {
  if (!atlas_rgba) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(atlas_rgba);
  return UI_ERROR_NONE;
}
ui_error_t ui_font_set_metadata(struct ui_font *font, const char *family,
                                int weight, int is_italic) {
  if (!font || !family)
    return UI_ERROR_INVALID_ARGUMENT;
  UI_STRNCPY(font->family, sizeof(font->family), family,
             sizeof(font->family) - 1);
  font->family[sizeof(font->family) - 1] = '\0';
  font->weight = weight;
  font->is_italic = is_italic;
  return UI_ERROR_NONE;
}

ui_error_t ui_font_get_status(struct ui_font *font,
                              enum ui_font_status *out_status) {
  if (!font || !out_status)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_status = font->status;
  return UI_ERROR_NONE;
}

ui_error_t ui_font_set_status(struct ui_font *font,
                              enum ui_font_status status) {
  if (!font)
    return UI_ERROR_INVALID_ARGUMENT;
  font->status = status;
  return UI_ERROR_NONE;
}

ui_error_t ui_font_manager_find_font(struct ui_font_manager *manager,
                                     const char *family, int weight,
                                     int is_italic, struct ui_font **out_font) {
  struct ui_font *curr;
  if (!manager || !family || !out_font)
    return UI_ERROR_INVALID_ARGUMENT;

  curr = manager->head;
  while (curr) {
    if (strcmp(curr->family, family) == 0 && curr->weight == weight &&
        curr->is_italic == is_italic) {
      *out_font = curr;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  *out_font = NULL;
  return UI_ERROR_NOT_FOUND;
}

ui_error_t ui_font_set_variations(struct ui_font *font,
                                  const struct ui_font_axis *axes,
                                  int axis_count) {
  int i;

  if (!font) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (axis_count > 0 && !axes) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (font->axes) {
    C_MULTIPLATFORM_FREE(font->axes);
    font->axes = NULL;
  }
  font->axis_count = 0;

  if (axis_count > 0) {
    font->axes = (struct ui_font_axis *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_font_axis) * (size_t)axis_count);
    if (!font->axes) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < axis_count; ++i) {
      font->axes[i] = axes[i];
    }
    font->axis_count = axis_count;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_font_get_variations(struct ui_font *font,
                                  struct ui_font_axis **out_axes,
                                  int *out_axis_count) {
  if (!font || !out_axes || !out_axis_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_axes = font->axes;
  *out_axis_count = font->axis_count;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
ui_error_t ui_test_font_manager_coverage_in_src(void);

ui_error_t ui_test_font_manager_coverage_in_src(void) {
  struct ui_font_manager *manager = NULL;
  struct ui_font *font = NULL;
  struct ui_font *found;
  struct ui_font_axis axes[2];
  enum ui_font_status status;
  struct ui_font_axis *out_axes;
  int count;
  const unsigned char *d;
  size_t s;
  float a, d_met, g_met;
  float kern;
  struct ui_glyph_metrics metrics;

  ui_font_manager_create(&manager);

  /* Trigger stbtt_InitFont failure branch */
  {
    unsigned char bad_ttf[128];
    struct ui_font *bad_font = NULL;
    memset(bad_ttf, 0, 128);
    ui_font_manager_load_font_memory(manager, bad_ttf, sizeof(bad_ttf),
                                     &bad_font);
  }

  {
    static const unsigned char dummy_ttf[] = {
        0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x63, 0x6d, 0x61, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c,
        0x00, 0x00, 0x00, 0x14, 0x68, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x36, 0x68, 0x68, 0x65, 0x61,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x24,
        0x68, 0x6d, 0x74, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xea,
        0x00, 0x00, 0x00, 0x08, 0x67, 0x6c, 0x79, 0x66, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xf2, 0x00, 0x00, 0x00, 0x01, 0x6c, 0x6f, 0x63, 0x61,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x00, 0x00, 0x00, 0x04,
        0x6d, 0x61, 0x78, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf7,
        0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00};
    ui_font_manager_load_font_memory(manager, dummy_ttf, sizeof(dummy_ttf),
                                     &font);
  }

  ui_font_set_metadata(font, "MyFont", 400, 1);
  ui_font_manager_find_font(manager, "MyFont", 400, 1, &found);

  axes[0].tag = 1;
  axes[0].value = 1.0f;
  axes[1].tag = 2;
  axes[1].value = 2.0f;
  ui_font_set_variations(font, axes, 2);
  ui_font_set_variations(font, NULL, 0);
  ui_font_set_variations(font, axes, 1);

  ui_font_set_status(font, UI_FONT_STATUS_LOADED);
  ui_font_get_status(font, &status);

  ui_font_get_variations(font, &out_axes, &count);

  ui_font_get_data(font, &d, &s);

  ui_font_get_vmetrics(font, 16.0f, &a, &d_met, &g_met);

  ui_font_get_kerning(font, 'A', 'B', 16.0f, &kern);

  ui_font_get_glyph_metrics(font, 'A', 16.0f, &metrics);

  /* Skipping generate_atlas */

  /* OOM branches for variations */
  g_malloc_fail_countdown = 0;
  ui_font_set_variations(font, axes, 2);
  g_malloc_fail_countdown = -1;

  /* Re-set variations so we can test destroying a font with variations */
  ui_font_set_variations(font, axes, 2);

  {
    int cp[] = {'A'};
    unsigned char *atlas = NULL;
    int w, h;
    g_malloc_fail_countdown = 0;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
    g_malloc_fail_countdown = 1;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
    g_malloc_fail_countdown = 2;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
    g_malloc_fail_countdown = 3;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
    g_malloc_fail_countdown = 4;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
    g_malloc_fail_countdown = 5;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);

    /* Fail PackFontRanges */
    g_malloc_fail_countdown = -1;
    ui_font_generate_atlas(font, 600.0f, cp, 1, &atlas, &w, &h);
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
    ui_font_free_atlas(atlas);
  }

  (void)ui_font_manager_destroy(manager);
  return UI_ERROR_NONE;
}
#endif
