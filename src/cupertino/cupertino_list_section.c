/**
 * @file cupertino_list_section.c
 * @brief Cupertino List Section component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_list_section.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_list_section_mock_base_create_fail = 0;
int g_cupertino_list_section_mock_set_orientation_fail = 0;
int g_cupertino_list_section_mock_base_destroy_fail = 0;
int g_cupertino_list_section_mock_tile_get_base_fail = 0;
int g_cupertino_list_section_mock_append_item_fail = 0;
struct ui_list_base *g_cupertino_list_section_last_created_base = NULL;

static ui_error_t mock_list_base_create(struct ui_list_base **out_base) {
  ui_error_t rc;
  if (g_cupertino_list_section_mock_base_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  rc = ui_list_base_create(out_base);
  if (rc == UI_ERROR_NONE) {
    g_cupertino_list_section_last_created_base = *out_base;
  }
  return rc;
}
#undef ui_list_base_create
/** @cond */
#define ui_list_base_create mock_list_base_create
/** @endcond */

static ui_error_t
mock_list_base_set_orientation(struct ui_list_base *list,
                               enum ui_list_orientation orientation) {
  if (g_cupertino_list_section_mock_set_orientation_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_list_base_set_orientation(list, orientation);
}
#undef ui_list_base_set_orientation
/** @cond */
#define ui_list_base_set_orientation mock_list_base_set_orientation
/** @endcond */

static ui_error_t mock_list_base_destroy(struct ui_list_base *list) {
  if (g_cupertino_list_section_mock_base_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_list_base_destroy)(list);
}
#undef ui_list_base_destroy
/** @cond */
#define ui_list_base_destroy mock_list_base_destroy
/** @endcond */

static ui_error_t
mock_cupertino_list_tile_get_base(struct cupertino_list_tile *tile,
                                  struct ui_list_item_base **out_base) {
  if (g_cupertino_list_section_mock_tile_get_base_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_list_tile_get_base(tile, out_base);
}
#undef cupertino_list_tile_get_base
/** @cond */
#define cupertino_list_tile_get_base mock_cupertino_list_tile_get_base
/** @endcond */

static ui_error_t
mock_ui_list_base_append_item(struct ui_list_base *list,
                              struct ui_list_item_base *item) {
  if (g_cupertino_list_section_mock_append_item_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_list_base_append_item(list, item);
}
#undef ui_list_base_append_item
/** @cond */
#define ui_list_base_append_item mock_ui_list_base_append_item
/** @endcond */
#endif

ui_error_t cupertino_list_section_create(
    struct ui_engine *engine,
    const struct cupertino_list_section_descriptor *desc,
    struct cupertino_list_section **out_section) {
  struct cupertino_list_section *section;
  ui_error_t rc;

  if (!engine || !desc || !out_section) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  section = (struct cupertino_list_section *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_list_section));
  if (!section) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(section, 0, sizeof(*section));
  section->style = desc->style;

  if (desc->header) {
#if defined(_MSC_VER)
    strncpy_s(section->header, sizeof(section->header), desc->header,
              sizeof(section->header) - 1);
#else
    strncpy(section->header, desc->header, sizeof(section->header) - 1);
    section->header[sizeof(section->header) - 1] = '\0';
#endif
  }

  if (desc->footer) {
#if defined(_MSC_VER)
    strncpy_s(section->footer, sizeof(section->footer), desc->footer,
              sizeof(section->footer) - 1);
#else
    strncpy(section->footer, desc->footer, sizeof(section->footer) - 1);
    section->footer[sizeof(section->footer) - 1] = '\0';
#endif
  }

  rc = ui_list_base_create(&section->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(section);
    return rc;
  }

  rc =
      ui_list_base_set_orientation(section->base, UI_LIST_ORIENTATION_VERTICAL);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_list_base_destroy(section->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(section);
    return rc;
  }

  *out_section = section;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_section_destroy(struct cupertino_list_section *section) {
  ui_error_t rc;

  if (!section) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (section->base) {
    rc = ui_list_base_destroy(section->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    section->base = NULL;
  }

  C_MULTIPLATFORM_FREE(section);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_section_set_header(struct cupertino_list_section *section,
                                  const char *header) {
  if (!section) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (header) {
#if defined(_MSC_VER)
    strncpy_s(section->header, sizeof(section->header), header,
              sizeof(section->header) - 1);
#else
    strncpy(section->header, header, sizeof(section->header) - 1);
    section->header[sizeof(section->header) - 1] = '\0';
#endif
  } else {
    section->header[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_section_get_header(const struct cupertino_list_section *section,
                                  const char **out_header) {
  if (!section || !out_header) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_header = (section->header[0] != '\0') ? section->header : NULL;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_section_set_footer(struct cupertino_list_section *section,
                                  const char *footer) {
  if (!section) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (footer) {
#if defined(_MSC_VER)
    strncpy_s(section->footer, sizeof(section->footer), footer,
              sizeof(section->footer) - 1);
#else
    strncpy(section->footer, footer, sizeof(section->footer) - 1);
    section->footer[sizeof(section->footer) - 1] = '\0';
#endif
  } else {
    section->footer[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_section_get_footer(const struct cupertino_list_section *section,
                                  const char **out_footer) {
  if (!section || !out_footer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_footer = (section->footer[0] != '\0') ? section->footer : NULL;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_section_add_tile(struct cupertino_list_section *section,
                                struct cupertino_list_tile *tile,
                                size_t *out_index) {
  struct ui_list_item_base *base_item;
  size_t idx;
  ui_error_t rc;

  if (!section || !tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (section->tile_count >= CUPERTINO_LIST_SECTION_MAX_TILES) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = cupertino_list_tile_get_base(tile, &base_item);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_list_base_append_item(section->base, base_item);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  idx = section->tile_count;
  section->tiles[idx] = tile;
  section->tile_count++;

  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_section_get_tile_count(
    const struct cupertino_list_section *section, size_t *out_count) {
  if (!section || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = section->tile_count;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_section_get_tile(const struct cupertino_list_section *section,
                                size_t index,
                                struct cupertino_list_tile **out_tile) {
  if (!section || index >= section->tile_count || !out_tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_tile = section->tiles[index];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_section_is_separator_visible(
    const struct cupertino_list_section *section, size_t index,
    int *out_visible) {
  if (!section || index >= section->tile_count || !out_visible) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* In Inset Grouped style, hide separator on the last item */
  if (section->style == CUPERTINO_LIST_SECTION_INSET_GROUPED &&
      index == section->tile_count - 1) {
    *out_visible = 0;
  } else {
    *out_visible = 1;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_section_get_separator_inset(
    const struct cupertino_list_section *section, size_t index,
    float *out_inset_leading) {
  if (!section || index >= section->tile_count || !out_inset_leading) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (section->tiles[index]->leading_icon[0] != '\0') {
    *out_inset_leading = CUPERTINO_LIST_SECTION_ICON_INSET;
  } else {
    *out_inset_leading = CUPERTINO_LIST_SECTION_DEFAULT_INSET;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_section_get_base(struct cupertino_list_section *section,
                                struct ui_list_base **out_base) {
  if (!section || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = section->base;
  return UI_ERROR_NONE;
}
