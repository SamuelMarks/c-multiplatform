/**
 * @file cupertino_page_control.c
 * @brief Cupertino Page Control (dot indicator) implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_page_control.h"
#include "ui_internal_mem.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_page_control_mock_base_create_fail = 0;
int g_cupertino_page_control_mock_base_destroy_fail = 0;
int g_cupertino_page_control_mock_base_set_pages_fail = 0;
int g_cupertino_page_control_mock_base_set_page_fail = 0;

static ui_error_t
mock_ui_page_control_base_create(struct ui_page_control_base **out_base) {
  if (g_cupertino_page_control_mock_base_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_page_control_base_create(out_base);
}
#undef ui_page_control_base_create
/** @cond */
#define ui_page_control_base_create mock_ui_page_control_base_create
/** @endcond */

static ui_error_t
mock_ui_page_control_base_destroy(struct ui_page_control_base *base) {
  if (g_cupertino_page_control_mock_base_destroy_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_page_control_base_destroy(base);
}
#undef ui_page_control_base_destroy
/** @cond */
#define ui_page_control_base_destroy mock_ui_page_control_base_destroy
/** @endcond */

static ui_error_t
mock_ui_page_control_base_set_number_of_pages(struct ui_page_control_base *base,
                                              int count) {
  if (g_cupertino_page_control_mock_base_set_pages_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_page_control_base_set_number_of_pages(base, count);
}
#undef ui_page_control_base_set_number_of_pages
/** @cond */
#define ui_page_control_base_set_number_of_pages                               \
  mock_ui_page_control_base_set_number_of_pages
/** @endcond */

static ui_error_t
mock_ui_page_control_base_set_current_page(struct ui_page_control_base *base,
                                           int page) {
  if (g_cupertino_page_control_mock_base_set_page_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_page_control_base_set_current_page(base, page);
}
#undef ui_page_control_base_set_current_page
/** @cond */
#define ui_page_control_base_set_current_page                                  \
  mock_ui_page_control_base_set_current_page
/** @endcond */

#endif /* UI_TEST_MOCK_ALLOC */

ui_error_t cupertino_page_control_create(
    struct ui_engine *engine,
    const struct cupertino_page_control_descriptor *desc,
    struct cupertino_page_control **out_control) {
  struct cupertino_page_control *ctrl;
  ui_error_t rc;

  if (!engine || !desc || !out_control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->number_of_pages < 0 ||
      (desc->number_of_pages > 0 &&
       (desc->current_page < 0 ||
        desc->current_page >= desc->number_of_pages))) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctrl = (struct cupertino_page_control *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_page_control));
  if (!ctrl) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ctrl, 0, sizeof(*ctrl));
  ctrl->number_of_pages = desc->number_of_pages;
  ctrl->current_page = desc->current_page;
  ctrl->hides_for_single_page = desc->hides_for_single_page ? 1 : 0;
  ctrl->page_indicator_tint_color = (desc->page_indicator_tint_color != 0)
                                        ? desc->page_indicator_tint_color
                                        : UI_COLOR_ARGB(80, 255, 255, 255);
  ctrl->current_page_indicator_tint_color =
      (desc->current_page_indicator_tint_color != 0)
          ? desc->current_page_indicator_tint_color
          : UI_COLOR_ARGB(255, 255, 255, 255);

  rc = ui_page_control_base_create(&ctrl->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ctrl);
    return rc;
  }

  rc = ui_page_control_base_set_number_of_pages(ctrl->base,
                                                ctrl->number_of_pages);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_page_control_base_destroy(ctrl->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(ctrl);
    return rc;
  }

  rc = ui_page_control_base_set_current_page(ctrl->base, ctrl->current_page);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_page_control_base_destroy(ctrl->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(ctrl);
    return rc;
  }

  *out_control = ctrl;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_control_destroy(struct cupertino_page_control *control) {
  ui_error_t rc;

  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->base) {
    rc = ui_page_control_base_destroy(control->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    control->base = NULL;
  }

  C_MULTIPLATFORM_FREE(control);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_control_set_number_of_pages(
    struct cupertino_page_control *control, int count) {
  ui_error_t rc;

  if (!control || count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->number_of_pages = count;
  if (count == 0) {
    control->current_page = 0;
  } else if (control->current_page >= count) {
    control->current_page = count - 1;
  }

  if (control->base) {
    rc = ui_page_control_base_set_number_of_pages(control->base, count);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_page_control_base_set_current_page(control->base,
                                               control->current_page);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_control_get_number_of_pages(
    const struct cupertino_page_control *control, int *out_count) {
  if (!control || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = control->number_of_pages;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_control_set_current_page(struct cupertino_page_control *control,
                                        int page) {
  ui_error_t rc;

  if (!control || page < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (control->number_of_pages > 0 && page >= control->number_of_pages) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->current_page = page;
  if (control->base) {
    rc = ui_page_control_base_set_current_page(control->base, page);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_control_get_current_page(
    const struct cupertino_page_control *control, int *out_page) {
  if (!control || !out_page) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_page = control->current_page;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_control_scrub(struct cupertino_page_control *control,
                                        float touch_x, int *out_page_changed) {
  float pitch;
  float rel_x;
  int target_page;
  ui_error_t rc;

  if (!control || !out_page_changed) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->number_of_pages <= 0) {
    *out_page_changed = 0;
    return UI_ERROR_NONE;
  }

  control->is_scrubbing = 1;
  control->scrub_touch_x = touch_x;

  pitch =
      CUPERTINO_PAGE_CONTROL_DOT_DIAMETER + CUPERTINO_PAGE_CONTROL_DOT_SPACING;
  rel_x = touch_x - 16.0f;
  if (rel_x < 0.0f) {
    target_page = 0;
  } else {
    target_page = (int)((rel_x + (pitch * 0.5f)) / pitch);
    if (target_page >= control->number_of_pages) {
      target_page = control->number_of_pages - 1;
    }
  }

  if (target_page != control->current_page) {
    control->current_page = target_page;
    if (control->base) {
      rc = ui_page_control_base_set_current_page(control->base, target_page);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    *out_page_changed = 1;
  } else {
    *out_page_changed = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_control_end_scrub(struct cupertino_page_control *control) {
  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->is_scrubbing = 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_control_get_dot_scale(
    const struct cupertino_page_control *control, int dot_index,
    float *out_scale) {
  int dist;

  if (!control || !out_scale || dot_index < 0 ||
      dot_index >= control->number_of_pages) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->number_of_pages <=
      CUPERTINO_PAGE_CONTROL_MAX_UNCOMPRESSED_DOTS) {
    *out_scale = 1.0f;
    return UI_ERROR_NONE;
  }

  dist = abs(dot_index - control->current_page);
  if (dist <= 3) {
    *out_scale = 1.0f;
  } else if (dist == 4) {
    *out_scale = 0.66f;
  } else {
    *out_scale = 0.33f;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_control_get_dimensions(
    const struct cupertino_page_control *control, float *out_width,
    float *out_height) {
  float dots_width;

  if (!control || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->hides_for_single_page && control->number_of_pages <= 1) {
    *out_width = 0.0f;
    *out_height = 0.0f;
    return UI_ERROR_NONE;
  }

  if (control->number_of_pages == 0) {
    *out_width = 0.0f;
    *out_height = 0.0f;
    return UI_ERROR_NONE;
  }

  dots_width =
      ((float)control->number_of_pages * CUPERTINO_PAGE_CONTROL_DOT_DIAMETER) +
      ((float)(control->number_of_pages - 1) *
       CUPERTINO_PAGE_CONTROL_DOT_SPACING);

  *out_width = dots_width + 32.0f;
  *out_height = CUPERTINO_PAGE_CONTROL_HEIGHT;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_control_get_base(struct cupertino_page_control *control,
                                struct ui_page_control_base **out_base) {
  if (!control || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = control->base;
  return UI_ERROR_NONE;
}
