/**
 * @file ui_font_manager.c
 * @brief ui_font_manager.c implementation.
 */
/**
 * @file ui_font_manager.c
 * @brief Implementation of the UI font manager.
 */
/* clang-format off */
#include "../include/ui_font_manager.h"
#include "ui_internal_mem.h"

#if defined(__GNUC__) || defined(__clang__)
#if !defined(__clang__)
#endif
#elif defined(_MSC_VER)
#endif

/* #define STB_TRUETYPE_IMPLEMENTATION */
/** @brief STB TrueType memory allocation */
#define STBTT_malloc(x,u)  ((u) ? C_MULTIPLATFORM_MALLOC(x) : C_MULTIPLATFORM_MALLOC(x))
/** @brief internal */
#define STBTT_free(x,u)    do { if (u) {} C_MULTIPLATFORM_FREE(x); } while (0)
#include "stb_truetype.h"

#if defined(__GNUC__) || defined(__clang__)
#elif defined(_MSC_VER)
#endif

#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @struct ui_font
 * @brief Represents a loaded font instance.
 */
struct ui_font {
  stbtt_fontinfo info;        /**< info */
  unsigned char *data;        /**< data */
  size_t size;                /**< size */
  char family[128];           /**< family */
  int weight;                 /**< weight */
  int is_italic;              /**< is_italic */
  enum ui_font_status status; /**< status */
  struct ui_font_axis *axes;  /**< axes */
  int axis_count;             /**< axis_count */
  struct ui_font *next;       /**< next */
};

/**
 * @struct ui_font_manager
 * @brief Manages a collection of loaded fonts.
 */
struct ui_font_manager {
  struct ui_font *head; /**< head */
};

/**
 * @brief Creates a new font manager.
 * @param[out] out_manager Pointer to store the created font manager.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Destroys a font manager and frees its fonts.
 * @param[in,out] manager The font manager to destroy.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Loads a font from a memory buffer.
 * @param[in,out] manager The font manager.
 * @param[in] font_data The memory buffer containing the font data.
 * @param[in] data_size The size of the memory buffer in bytes.
 * @param[out] out_font Pointer to store the loaded font.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Loads a TrueType or OpenType font from the filesystem into the font
 * manager.
 * @param[in,out] manager The font manager.
 * @param[in] file_path Path to the TrueType or OpenType font file.
 * @param[out] out_font Pointer to store the loaded font handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_font_manager_load_font_file(struct ui_font_manager *manager,
                                          const char *file_path,
                                          struct ui_font **out_font) {
  FILE *f;
  long file_size;
  unsigned char *buffer;
  size_t bytes_read;
  ui_error_t rc;

  if (!manager || !file_path || !out_font) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  {
    errno_t err = fopen_s(&f, file_path, "rb");
    if (err != 0 || !f) {
      return UI_ERROR_NOT_FOUND;
    }
  }
#else
  f = fopen(file_path, "rb");
  if (!f) {
    return UI_ERROR_NOT_FOUND;
  }
#endif

  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return UI_ERROR_IO_FAILED;
  }
  file_size = ftell(f);
  if (file_size <= 0) {
    fclose(f);
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (fseek(f, 0, SEEK_SET) != 0) {
    fclose(f);
    return UI_ERROR_IO_FAILED;
  }

  buffer = (unsigned char *)C_MULTIPLATFORM_MALLOC((size_t)file_size);
  if (!buffer) {
    fclose(f);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  bytes_read = fread(buffer, 1, (size_t)file_size, f);
  fclose(f);
  if (bytes_read != (size_t)file_size) {
    C_MULTIPLATFORM_FREE(buffer);
    return UI_ERROR_IO_FAILED;
  }

  rc = ui_font_manager_load_font_memory(manager, buffer, (size_t)file_size,
                                        out_font);
  C_MULTIPLATFORM_FREE(buffer);
  return rc;
}

/**
 * @brief Retrieves glyph metrics for a codepoint.
 * @param[in] font The font to query.
 * @param[in] codepoint The unicode codepoint.
 * @param[in] font_size The size of the font in pixels.
 * @param[out] out_metrics Pointer to store the retrieved metrics.
 * @return UI_ERROR_NONE on success.
 */
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
  out_metrics->bearing_x = (int)((float)lsb * scale);
  out_metrics->bearing_y = -y0; /* usually y0 is negative for bearing */
  out_metrics->advance = (int)((float)advance * scale);

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves vertical metrics for a font.
 * @param[in] font The font to query.
 * @param[in] font_size The size of the font in pixels.
 * @param[out] out_ascent Pointer to store the ascent.
 * @param[out] out_descent Pointer to store the descent.
 * @param[out] out_line_gap Pointer to store the line gap.
 * @return UI_ERROR_NONE on success.
 */
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

static unsigned short read_u16_be(const unsigned char *p) {
  return (unsigned short)(((unsigned short)p[0] << 8) | (unsigned short)p[1]);
}

static short read_s16_be(const unsigned char *p) {
  return (short)read_u16_be(p);
}

static unsigned int read_u32_be(const unsigned char *p) {
  return (((unsigned int)p[0] << 24) | ((unsigned int)p[1] << 16) |
          ((unsigned int)p[2] << 8) | (unsigned int)p[3]);
}

static int get_glyph_coverage_index(const unsigned char *data, size_t size,
                                    size_t cov_offset, int glyph) {
  unsigned short format;
  if (cov_offset + 4 > size) {
    return -1;
  }
  format = read_u16_be(data + cov_offset);
  if (format == 1) {
    unsigned short glyph_count = read_u16_be(data + cov_offset + 2);
    size_t i;
    if (cov_offset + 4 + (size_t)glyph_count * 2 > size) {
      return -1;
    }
    for (i = 0; i < glyph_count; ++i) {
      if ((int)read_u16_be(data + cov_offset + 4 + i * 2) == glyph) {
        return (int)i;
      }
    }
  } else if (format == 2) {
    unsigned short range_count = read_u16_be(data + cov_offset + 2);
    size_t i;
    if (cov_offset + 4 + (size_t)range_count * 6 > size) {
      return -1;
    }
    for (i = 0; i < range_count; ++i) {
      size_t rec = cov_offset + 4 + i * 6;
      int start = (int)read_u16_be(data + rec);
      int end = (int)read_u16_be(data + rec + 2);
      int start_cov = (int)read_u16_be(data + rec + 4);
      if (glyph >= start && glyph <= end) {
        return start_cov + (glyph - start);
      }
    }
  }
  return -1;
}

static int get_glyph_class(const unsigned char *data, size_t size,
                           size_t class_def_offset, int glyph) {
  unsigned short format;
  if (class_def_offset == 0 || class_def_offset + 4 > size) {
    return 0;
  }
  format = read_u16_be(data + class_def_offset);
  if (format == 1) {
    int start_glyph = (int)read_u16_be(data + class_def_offset + 2);
    unsigned short glyph_count = read_u16_be(data + class_def_offset + 4);
    if (glyph >= start_glyph && glyph < start_glyph + (int)glyph_count) {
      size_t idx = (size_t)(glyph - start_glyph);
      if (class_def_offset + 6 + (idx + 1) * 2 <= size) {
        return (int)read_u16_be(data + class_def_offset + 6 + idx * 2);
      }
    }
  } else if (format == 2) {
    unsigned short range_count = read_u16_be(data + class_def_offset + 2);
    size_t i;
    if (class_def_offset + 4 + (size_t)range_count * 6 > size) {
      return 0;
    }
    for (i = 0; i < range_count; ++i) {
      size_t rec = class_def_offset + 4 + i * 6;
      int start = (int)read_u16_be(data + rec);
      int end = (int)read_u16_be(data + rec + 2);
      int cls = (int)read_u16_be(data + rec + 4);
      if (glyph >= start && glyph <= end) {
        return cls;
      }
    }
  }
  return 0;
}

static int parse_gpos_pair_pos(const unsigned char *data, size_t size,
                               int glyph1, int glyph2, int *out_kern) {
  size_t gpos_offset = 0;
  size_t lookup_list_offset, i, num_tables;
  unsigned short num_lookups;

  if (size < 12) {
    return 0;
  }
  num_tables = (size_t)read_u16_be(data + 4);
  if (12 + num_tables * 16 > size) {
    return 0;
  }
  for (i = 0; i < num_tables; ++i) {
    size_t entry = 12 + i * 16;
    if (data[entry] == 'G' && data[entry + 1] == 'P' &&
        data[entry + 2] == 'O' && data[entry + 3] == 'S') {
      gpos_offset = (size_t)read_u32_be(data + entry + 8);
      break;
    }
  }
  if (gpos_offset == 0 || gpos_offset + 10 > size) {
    return 0;
  }
  lookup_list_offset =
      gpos_offset + (size_t)read_u16_be(data + gpos_offset + 8);
  if (lookup_list_offset + 2 > size) {
    return 0;
  }
  num_lookups = read_u16_be(data + lookup_list_offset);
  for (i = 0; i < num_lookups; ++i) {
    size_t l_offset;
    unsigned short lookup_type, subtable_count, s;
    if (lookup_list_offset + 2 + (i + 1) * 2 > size) {
      break;
    }
    l_offset = lookup_list_offset +
               (size_t)read_u16_be(data + lookup_list_offset + 2 + i * 2);
    if (l_offset + 6 > size) {
      continue;
    }
    lookup_type = read_u16_be(data + l_offset);
    if (lookup_type != 2) {
      continue;
    }
    subtable_count = read_u16_be(data + l_offset + 4);
    for (s = 0; s < subtable_count; ++s) {
      size_t sub_offset;
      unsigned short pos_format, cov_offset;
      if (l_offset + 6 + (s + 1) * 2 > size) {
        break;
      }
      sub_offset = l_offset + (size_t)read_u16_be(data + l_offset + 6 + s * 2);
      if (sub_offset + 4 > size) {
        continue;
      }
      pos_format = read_u16_be(data + sub_offset);
      cov_offset = read_u16_be(data + sub_offset + 2);
      if (get_glyph_coverage_index(data, size, sub_offset + cov_offset,
                                   glyph1) < 0) {
        continue;
      }
      if (pos_format == 2) {
        unsigned short vfmt1, class_def1_off, class_def2_off, class1_count,
            class2_count;
        int cls1, cls2;
        if (sub_offset + 16 > size) {
          continue;
        }
        vfmt1 = read_u16_be(data + sub_offset + 4);
        class_def1_off = read_u16_be(data + sub_offset + 8);
        class_def2_off = read_u16_be(data + sub_offset + 10);
        class1_count = read_u16_be(data + sub_offset + 12);
        class2_count = read_u16_be(data + sub_offset + 14);
        cls1 = get_glyph_class(data, size, sub_offset + class_def1_off, glyph1);
        cls2 = get_glyph_class(data, size, sub_offset + class_def2_off, glyph2);
        if (cls1 < (int)class1_count && cls2 < (int)class2_count &&
            (vfmt1 & 0x0004)) {
          size_t val_record_offset =
              sub_offset + 16 +
              ((size_t)cls1 * (size_t)class2_count + (size_t)cls2) * 2;
          if (val_record_offset + 2 <= size) {
            short x_advance = read_s16_be(data + val_record_offset);
            if (x_advance != 0) {
              *out_kern = (int)x_advance;
              return 1;
            }
          }
        }
      }
    }
  }
  return 0;
}

/**
 * @brief Retrieves kerning advance between two codepoints.
 * @param[in] font The font to query.
 * @param[in] codepoint1 The first unicode codepoint.
 * @param[in] codepoint2 The second unicode codepoint.
 * @param[in] font_size The size of the font in pixels.
 * @param[out] out_kerning Pointer to store the kerning value.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_font_get_kerning(struct ui_font *font, int codepoint1,
                               int codepoint2, float font_size,
                               float *out_kerning) {
  float scale;
  int kern = 0;
  int glyph1, glyph2;

  if (!font || !out_kerning) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scale = stbtt_ScaleForPixelHeight(&font->info, font_size);
  glyph1 = stbtt_FindGlyphIndex(&font->info, codepoint1);
  glyph2 = stbtt_FindGlyphIndex(&font->info, codepoint2);

  if (font->data && font->size > 0 &&
      parse_gpos_pair_pos(font->data, font->size, glyph1, glyph2, &kern)) {
    *out_kerning = (float)kern * scale;
    return UI_ERROR_NONE;
  }

  kern = stbtt_GetCodepointKernAdvance(&font->info, codepoint1, codepoint2);
  *out_kerning = (float)kern * scale;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the raw font data buffer.
 * @param[in] font The font to query.
 * @param[out] out_data Pointer to store the font data buffer.
 * @param[out] out_size Pointer to store the size of the buffer.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_font_get_data(struct ui_font *font,
                            const unsigned char **out_data, size_t *out_size) {
  if (!font || !out_data || !out_size) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_data = font->data;
  *out_size = font->size;
  return UI_ERROR_NONE;
}

/**
 * @brief Generates a texture atlas for a set of codepoints.
 * @param[in] font The font to use.
 * @param[in] font_size The font size in pixels.
 * @param[in] codepoints Array of codepoints to pack.
 * @param[in] codepoint_count Number of codepoints.
 * @param[out] out_atlas_rgba Pointer to store the allocated RGBA atlas buffer.
 * @param[out] out_width Pointer to store the width of the generated atlas.
 * @param[out] out_height Pointer to store the height of the generated atlas.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Frees a generated texture atlas.
 * @param[in,out] atlas_rgba The atlas buffer to free.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_font_free_atlas(unsigned char *atlas_rgba) {
  if (!atlas_rgba) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(atlas_rgba);
  return UI_ERROR_NONE;
}
/**
 * @brief Sets metadata for a font (family, weight, italic).
 * @param[in,out] font The font to update.
 * @param[in] family The font family string.
 * @param[in] weight The font weight.
 * @param[in] is_italic 1 if italic, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Gets the current loading status of a font.
 * @param[in] font The font to query.
 * @param[out] out_status Pointer to store the status.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_font_get_status(struct ui_font *font,
                              enum ui_font_status *out_status) {
  if (!font || !out_status)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_status = font->status;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the loading status of a font.
 * @param[in,out] font The font to update.
 * @param[in] status The status to set.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_font_set_status(struct ui_font *font,
                              enum ui_font_status status) {
  if (!font)
    return UI_ERROR_INVALID_ARGUMENT;
  font->status = status;
  return UI_ERROR_NONE;
}

/**
 * @brief Finds a loaded font matching the provided metadata.
 * @param[in] manager The font manager.
 * @param[in] family The font family.
 * @param[in] weight The font weight.
 * @param[in] is_italic The italic flag.
 * @param[out] out_font Pointer to store the found font.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if not found.
 */
ui_error_t ui_font_manager_find_font(struct ui_font_manager *manager,
                                     const char *family, int weight,
                                     int is_italic, struct ui_font **out_font) {
  const char *p;
  const char *start;
  char candidate[128];
  size_t len;
  struct ui_font *curr;

  if (!manager || !family || !out_font) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = family;
  while (*p) {
    while (*p == ' ' || *p == '\t' || *p == ',') {
      p++;
    }
    if (*p == '\0') {
      break;
    }
    start = p;
    while (*p && *p != ',') {
      p++;
    }
    len = (size_t)(p - start);
    while (len > 0 && (start[len - 1] == ' ' || start[len - 1] == '\t' ||
                       start[len - 1] == '\'' || start[len - 1] == '\"')) {
      len--;
    }
    while (len > 0 && (*start == '\'' || *start == '\"')) {
      start++;
      len--;
    }
    if (len > 0 && len < sizeof(candidate)) {
      memcpy(candidate, start, len);
      candidate[len] = '\0';

      curr = manager->head;
      while (curr) {
        if (strcmp(curr->family, candidate) == 0 && curr->weight == weight &&
            curr->is_italic == is_italic) {
          *out_font = curr;
          return UI_ERROR_NONE;
        }
        curr = curr->next;
      }
    }
  }

  *out_font = NULL;
  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Sets axis variations for a font.
 * @param[in,out] font The font to update.
 * @param[in] axes The array of font axes.
 * @param[in] axis_count The number of axes.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Retrieves the current axis variations for a font.
 * @param[in] font The font to query.
 * @param[out] out_axes Pointer to store the array of axes.
 * @param[out] out_axis_count Pointer to store the number of axes.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Tests ui font manager coverage.
 * @return UI_ERROR_NONE on success.
 */
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

  return ui_font_manager_destroy(manager);
}
#endif
