/**
 * @file cupertino_photo_picker.c
 * @brief Apple Photo Picker (PHPickerViewController) implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_photo_picker.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t cupertino_photo_picker_create(
    struct ui_engine *engine,
    const struct cupertino_photo_picker_descriptor *desc,
    struct cupertino_photo_picker **out_picker) {
  struct cupertino_photo_picker *picker;

  if (!engine || !desc || !out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->selection_limit < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->filter < 0 || (int)desc->filter > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct cupertino_photo_picker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_photo_picker));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(picker, 0, sizeof(*picker));
  picker->selection_limit = desc->selection_limit;
  picker->filter = desc->filter;
  picker->width = 393.0f;
  picker->height = 700.0f;

  *out_picker = picker;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_photo_picker_destroy(struct cupertino_photo_picker *picker) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(picker);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_photo_picker_add_asset(struct cupertino_photo_picker *picker,
                                 const char *id, const char *title,
                                 int is_video) {
  struct cupertino_photo_item *item;
  int idx;

  if (!picker || !id || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (picker->asset_count >= CUPERTINO_PHOTO_PICKER_MAX_ASSETS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = picker->asset_count;
  item = &picker->assets[idx];
  memset(item, 0, sizeof(*item));

#if defined(_MSC_VER)
  strncpy_s(item->id, sizeof(item->id), id, _TRUNCATE);
  strncpy_s(item->title, sizeof(item->title), title, _TRUNCATE);
#else
  strncpy(item->id, id, sizeof(item->id) - 1);
  item->id[sizeof(item->id) - 1] = '\0';
  strncpy(item->title, title, sizeof(item->title) - 1);
  item->title[sizeof(item->title) - 1] = '\0';
#endif

  item->is_video = is_video ? 1 : 0;
  item->is_selected = 0;
  item->selection_order = 0;
  item->is_visible = 1;

  picker->asset_count++;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_photo_picker_toggle_selection(struct cupertino_photo_picker *picker,
                                        int asset_index, int *out_is_selected) {
  struct cupertino_photo_item *item;
  int i;
  int order;

  if (!picker || asset_index < 0 || asset_index >= picker->asset_count ||
      !out_is_selected) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = &picker->assets[asset_index];

  if (item->is_selected) {
    /* Deselect */
    item->is_selected = 0;
    item->selection_order = 0;
    picker->selected_count--;

    /* Recompute order for remaining */
    order = 1;
    for (i = 0; i < picker->asset_count; i++) {
      if (picker->assets[i].is_selected) {
        picker->assets[i].selection_order = order++;
      }
    }
    *out_is_selected = 0;
  } else {
    /* Check selection limit */
    if (picker->selection_limit > 0 &&
        picker->selected_count >= picker->selection_limit) {
      return UI_ERROR_INVALID_ARGUMENT;
    }

    item->is_selected = 1;
    picker->selected_count++;
    item->selection_order = picker->selected_count;
    *out_is_selected = 1;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_photo_picker_get_selected_count(
    const struct cupertino_photo_picker *picker, int *out_count) {
  if (!picker || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = picker->selected_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_photo_picker_get_selection_limit(
    const struct cupertino_photo_picker *picker, int *out_limit) {
  if (!picker || !out_limit) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_limit = picker->selection_limit;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_photo_picker_clear_selection(struct cupertino_photo_picker *picker) {
  int i;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < picker->asset_count; i++) {
    picker->assets[i].is_selected = 0;
    picker->assets[i].selection_order = 0;
  }
  picker->selected_count = 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_photo_picker_set_filter(struct cupertino_photo_picker *picker,
                                  enum cupertino_photo_filter filter) {
  int i;

  if (!picker || (int)filter < 0 || (int)filter > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->filter = filter;
  for (i = 0; i < picker->asset_count; i++) {
    if (filter == CUPERTINO_PHOTO_FILTER_IMAGES && picker->assets[i].is_video) {
      picker->assets[i].is_visible = 0;
    } else if (filter == CUPERTINO_PHOTO_FILTER_VIDEOS &&
               !picker->assets[i].is_video) {
      picker->assets[i].is_visible = 0;
    } else {
      picker->assets[i].is_visible = 1;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_photo_picker_get_filter(const struct cupertino_photo_picker *picker,
                                  enum cupertino_photo_filter *out_filter) {
  if (!picker || !out_filter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_filter = picker->filter;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_photo_picker_filter_search(struct cupertino_photo_picker *picker,
                                     const char *query,
                                     int *out_matched_count) {
  int i;
  int count;

  if (!picker || !out_matched_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  count = 0;
  for (i = 0; i < picker->asset_count; i++) {
    int matches_filter = 1;
    if (picker->filter == CUPERTINO_PHOTO_FILTER_IMAGES &&
        picker->assets[i].is_video) {
      matches_filter = 0;
    } else if (picker->filter == CUPERTINO_PHOTO_FILTER_VIDEOS &&
               !picker->assets[i].is_video) {
      matches_filter = 0;
    }

    if (!matches_filter) {
      picker->assets[i].is_visible = 0;
      continue;
    }

    if (!query || strlen(query) == 0) {
      picker->assets[i].is_visible = 1;
      count++;
    } else {
      if (strstr(picker->assets[i].title, query) != NULL) {
        picker->assets[i].is_visible = 1;
        count++;
      } else {
        picker->assets[i].is_visible = 0;
      }
    }
  }

  *out_matched_count = count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_photo_picker_get_dimensions(
    const struct cupertino_photo_picker *picker, float *out_width,
    float *out_height) {
  if (!picker || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = picker->width;
  *out_height = picker->height;
  return UI_ERROR_NONE;
}
