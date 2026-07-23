/* clang-format off */
#include "../include/ui_css_highlight_api.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

struct ui_css_highlight {
  enum ui_css_highlight_type type;
  char *custom_name;
  struct ui_css_highlight_range *ranges;
  size_t range_count;
  size_t range_capacity;
};

enum ui_error ui_css_highlight_create(enum ui_css_highlight_type type,
                                      const char *custom_name,
                                      struct ui_css_highlight **out_highlight) {
  struct ui_css_highlight *hl = NULL;
  enum ui_error rc = UI_ERROR_NONE;
  size_t len = 0;

  if (!out_highlight) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }
  if (type == UI_CSS_HIGHLIGHT_TYPE_CUSTOM && !custom_name) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  hl = (struct ui_css_highlight *)UI_MALLOC(sizeof(struct ui_css_highlight));
  if (!hl) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  hl->type = type;
  hl->custom_name = NULL;
  hl->ranges = NULL;
  hl->range_count = 0;
  hl->range_capacity = 0;

  if (custom_name) {
    len = strlen(custom_name);
    hl->custom_name = (char *)UI_MALLOC(len + 1);
    if (!hl->custom_name) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
#if defined(_MSC_VER)
    strcpy_s(hl->custom_name, len + 1, custom_name);
#else
    strcpy(hl->custom_name, custom_name);
#endif
  }

  *out_highlight = hl;
  return UI_ERROR_NONE;

cleanup:
  if (hl) {
    /* If we get here, hl->custom_name failed to allocate. It is NULL. */
    UI_FREE(hl);
  }
  return rc;
}

enum ui_error ui_css_highlight_add_range(struct ui_css_highlight *highlight,
                                         size_t start_offset,
                                         size_t end_offset) {
  enum ui_error rc = UI_ERROR_NONE;
  size_t new_cap = 0;
  struct ui_css_highlight_range *new_ranges = NULL;

  if (!highlight) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (highlight->range_count >= highlight->range_capacity) {
    new_cap =
        highlight->range_capacity == 0 ? 4 : highlight->range_capacity * 2;
    new_ranges = (struct ui_css_highlight_range *)UI_MALLOC(
        new_cap * sizeof(struct ui_css_highlight_range));
    if (!new_ranges) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
    if (highlight->ranges) {
      memcpy(new_ranges, highlight->ranges,
             highlight->range_count * sizeof(struct ui_css_highlight_range));
      UI_FREE(highlight->ranges);
    }
    highlight->ranges = new_ranges;
    highlight->range_capacity = new_cap;
  }

  highlight->ranges[highlight->range_count].start_offset = start_offset;
  highlight->ranges[highlight->range_count].end_offset = end_offset;
  highlight->range_count++;

  return UI_ERROR_NONE;

cleanup:
  return rc;
}

void ui_css_highlight_destroy(struct ui_css_highlight *highlight) {
  if (highlight) {
    if (highlight->custom_name) {
      UI_FREE(highlight->custom_name);
    }
    if (highlight->ranges) {
      UI_FREE(highlight->ranges);
    }
    UI_FREE(highlight);
  }
}
