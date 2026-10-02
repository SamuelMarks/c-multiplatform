/**
 * @file cupertino_toolbar.c
 * @brief Cupertino Toolbar component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_toolbar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_toolbar_mock_create_fail = 0;
int g_cupertino_toolbar_mock_destroy_fail = 0;
int g_cupertino_toolbar_mock_set_title_fail = 0;
int g_cupertino_toolbar_mock_set_mode_fail = 0;

static ui_error_t mock_toolbar_base_create(struct ui_toolbar_base **out_base) {
  if (g_cupertino_toolbar_mock_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_toolbar_base_create(out_base);
}
#undef ui_toolbar_base_create
/** @cond */
#define ui_toolbar_base_create mock_toolbar_base_create
/** @endcond */

static ui_error_t mock_toolbar_base_destroy(struct ui_toolbar_base *base) {
  if (g_cupertino_toolbar_mock_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_toolbar_base_destroy)(base);
}
#undef ui_toolbar_base_destroy
/** @cond */
#define ui_toolbar_base_destroy mock_toolbar_base_destroy
/** @endcond */

static ui_error_t mock_toolbar_base_set_title(struct ui_toolbar_base *base,
                                              const char *title) {
  if (g_cupertino_toolbar_mock_set_title_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_toolbar_base_set_title(base, title);
}
#undef ui_toolbar_base_set_title
/** @cond */
#define ui_toolbar_base_set_title mock_toolbar_base_set_title
/** @endcond */

static ui_error_t mock_toolbar_base_set_mode(struct ui_toolbar_base *base,
                                             enum ui_toolbar_mode mode) {
  if (g_cupertino_toolbar_mock_set_mode_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_toolbar_base_set_mode(base, mode);
}
#undef ui_toolbar_base_set_mode
/** @cond */
#define ui_toolbar_base_set_mode mock_toolbar_base_set_mode
/** @endcond */
#endif

#define CUPERTINO_TOOLBAR_STANDARD_HEIGHT 44.0f

ui_error_t
cupertino_toolbar_create(struct ui_engine *engine,
                         const struct cupertino_toolbar_descriptor *desc,
                         struct cupertino_toolbar **out_toolbar) {
  struct cupertino_toolbar *toolbar;
  ui_error_t rc;

  if (!engine || !desc || !out_toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar = (struct cupertino_toolbar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_toolbar));
  if (!toolbar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(toolbar, 0, sizeof(*toolbar));
  toolbar->is_translucent = desc->is_translucent ? 1 : 0;

  rc = ui_toolbar_base_create(&toolbar->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(toolbar);
    return rc;
  }

  if (desc->title) {
    rc = ui_toolbar_base_set_title(toolbar->base, desc->title);
    if (rc != UI_ERROR_NONE) {
      ui_error_t destroy_rc;
      destroy_rc = ui_toolbar_base_destroy(toolbar->base);
      if (destroy_rc != UI_ERROR_NONE) {
        /* Keep original error */
      }
      C_MULTIPLATFORM_FREE(toolbar);
      return rc;
    }
  }

  rc = ui_toolbar_base_set_mode(toolbar->base, UI_TOOLBAR_MODE_STICKY);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_toolbar_base_destroy(toolbar->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(toolbar);
    return rc;
  }

  *out_toolbar = toolbar;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_toolbar_destroy(struct cupertino_toolbar *toolbar) {
  ui_error_t rc;

  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toolbar->base) {
    rc = ui_toolbar_base_destroy(toolbar->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    toolbar->base = NULL;
  }

  C_MULTIPLATFORM_FREE(toolbar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_toolbar_add_action(struct cupertino_toolbar *toolbar,
                                        const char *title, int is_destructive,
                                        int is_disabled, size_t *out_index) {
  struct cupertino_toolbar_item *item;
  size_t idx;

  if (!toolbar || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toolbar->item_count >= CUPERTINO_TOOLBAR_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = toolbar->item_count;
  item = &toolbar->items[idx];
  memset(item, 0, sizeof(*item));
  item->type = CUPERTINO_TOOLBAR_ITEM_ACTION;
  item->is_destructive = is_destructive ? 1 : 0;
  item->is_disabled = is_disabled ? 1 : 0;

#if defined(_MSC_VER)
  strncpy_s(item->title, sizeof(item->title), title, sizeof(item->title) - 1);
#else
  strncpy(item->title, title, sizeof(item->title) - 1);
  item->title[sizeof(item->title) - 1] = '\0';
#endif

  toolbar->item_count++;
  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_toolbar_add_flexible_space(struct cupertino_toolbar *toolbar,
                                     size_t *out_index) {
  struct cupertino_toolbar_item *item;
  size_t idx;

  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toolbar->item_count >= CUPERTINO_TOOLBAR_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = toolbar->item_count;
  item = &toolbar->items[idx];
  memset(item, 0, sizeof(*item));
  item->type = CUPERTINO_TOOLBAR_ITEM_FLEXIBLE_SPACE;

  toolbar->item_count++;
  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_toolbar_add_fixed_space(struct cupertino_toolbar *toolbar,
                                             float width, size_t *out_index) {
  struct cupertino_toolbar_item *item;
  size_t idx;

  if (!toolbar || width < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toolbar->item_count >= CUPERTINO_TOOLBAR_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = toolbar->item_count;
  item = &toolbar->items[idx];
  memset(item, 0, sizeof(*item));
  item->type = CUPERTINO_TOOLBAR_ITEM_FIXED_SPACE;
  item->width = width;

  toolbar->item_count++;
  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_toolbar_get_item_count(const struct cupertino_toolbar *toolbar,
                                 size_t *out_count) {
  if (!toolbar || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = toolbar->item_count;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_toolbar_set_item_disabled(struct cupertino_toolbar *toolbar,
                                    size_t index, int is_disabled) {
  if (!toolbar || index >= toolbar->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->items[index].is_disabled = is_disabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_toolbar_is_item_disabled(const struct cupertino_toolbar *toolbar,
                                   size_t index, int *out_disabled) {
  if (!toolbar || index >= toolbar->item_count || !out_disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_disabled = toolbar->items[index].is_disabled;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_toolbar_get_height(const struct cupertino_toolbar *toolbar,
                                        float *out_height) {
  if (!toolbar || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_height = CUPERTINO_TOOLBAR_STANDARD_HEIGHT;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_toolbar_get_base(struct cupertino_toolbar *toolbar,
                                      struct ui_toolbar_base **out_base) {
  if (!toolbar || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = toolbar->base;
  return UI_ERROR_NONE;
}
