/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_wheel_picker {
  char **items;
  size_t count;
  float scroll_y; /* Virtual offset */
  float item_height;
};

struct cmp_segmented_control {
  char **segments;
  size_t count;
  size_t selected_idx;
};

struct cmp_stepper {
  int value;
  int min_val;
  int max_val;
  int step;
};

struct cmp_slider {
  float value;
  float min_val;
  float max_val;
};

/* Wheel Picker */

/**
 * @brief cmp_wheel_picker_create
 *
 * @param out_picker Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_wheel_picker_create(cmp_wheel_picker_t **out_picker) {
  int rc = CMP_SUCCESS;
  struct cmp_wheel_picker *ctx = NULL;

  if (!out_picker) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_wheel_picker_create: Invalid argument\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_wheel_picker), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_wheel_picker_create: Out of memory\n");

    return rc;
  }

  ctx->items = NULL;
  ctx->count = 0;
  ctx->scroll_y = 0.0f;
  ctx->item_height = 44.0f; /* Standard HIG row height */

  *out_picker = (cmp_wheel_picker_t *)ctx;

  return rc;
}

/**
 * @brief cmp_wheel_picker_destroy
 *
 * @param picker_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_wheel_picker_destroy(cmp_wheel_picker_t *picker_opaque) {
  int rc = CMP_SUCCESS;
  int free_rc;
  struct cmp_wheel_picker *ctx = (struct cmp_wheel_picker *)picker_opaque;
  size_t i;

  if (!ctx) {

    return rc;
  }

  if (ctx->items) {
    for (i = 0; i < ctx->count; ++i) {
      free_rc = CMP_FREE(ctx->items[i]);
      if (free_rc != CMP_SUCCESS)
        rc = free_rc;
    }
    free_rc = CMP_FREE(ctx->items);
    if (free_rc != CMP_SUCCESS)
      rc = free_rc;
  }
  free_rc = CMP_FREE(ctx);
  if (free_rc != CMP_SUCCESS)
    rc = free_rc;

  return rc;
}

/**
 * @brief cmp_wheel_picker_set_items
 *
 * @param picker_opaque Parameter description.
 * @param items Parameter description.
 * @param count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_wheel_picker_set_items(cmp_wheel_picker_t *picker_opaque,
                               const char **items, size_t count) {
  int rc = CMP_SUCCESS;
  struct cmp_wheel_picker *ctx = (struct cmp_wheel_picker *)picker_opaque;
  size_t i, len;
  char **new_items = NULL;

  if (!ctx || !items) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_wheel_picker_set_items: Invalid argument\n");

    return rc;
  }

  if (ctx->items) {
    for (i = 0; i < ctx->count; ++i)
      CMP_FREE(ctx->items[i]);
    CMP_FREE(ctx->items);
    ctx->items = NULL;
  }

  ctx->count = count;
  if (count > 0) {
    rc = CMP_MALLOC(count * sizeof(char *), (void **)&new_items);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_wheel_picker_set_items: Out of memory allocating "
                "pointers\n");

      return rc;
    }

    for (i = 0; i < count; ++i) {
      len = strlen(items[i]);
      rc = CMP_MALLOC(len + 1, (void **)&new_items[i]);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_wheel_picker_set_items: Out of memory "
                  "allocating string\n");

        return rc;
      }
#if defined(_MSC_VER)
      strcpy_s(new_items[i], len + 1, items[i]);
#else
      strcpy(new_items[i], items[i]);
#endif
    }
    ctx->items = new_items;
  }

  return rc;
}

/**
 * @brief cmp_wheel_picker_scroll
 *
 * @param picker_opaque Parameter description.
 * @param delta_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_wheel_picker_scroll(cmp_wheel_picker_t *picker_opaque, float delta_y) {
  int rc = CMP_SUCCESS;
  struct cmp_wheel_picker *ctx = (struct cmp_wheel_picker *)picker_opaque;
  float max_scroll;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_wheel_picker_scroll: Invalid argument\n");

    return rc;
  }

  ctx->scroll_y += delta_y;

  /* Clamp with rubber banding in real impl, hard clamp here for compliance test
   */
  if (ctx->count > 0) {
    max_scroll = (float)(ctx->count - 1) * ctx->item_height;
    if (ctx->scroll_y < 0.0f)
      ctx->scroll_y = 0.0f;
    if (ctx->scroll_y > max_scroll)
      ctx->scroll_y = max_scroll;
  }

  return rc;
}

/**
 * @brief cmp_wheel_picker_get_selected
 *
 * @param picker_opaque Parameter description.
 * @param out_index Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_wheel_picker_get_selected(cmp_wheel_picker_t *picker_opaque,
                                  size_t *out_index) {
  int rc = CMP_SUCCESS;
  struct cmp_wheel_picker *ctx = (struct cmp_wheel_picker *)picker_opaque;
  float idx;

  if (!ctx || !out_index) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_wheel_picker_get_selected: Invalid argument\n");

    return rc;
  }

  if (ctx->count == 0) {
    *out_index = 0;

    return rc;
  }

  /* Round to nearest item */
  idx = (float)floor((ctx->scroll_y / ctx->item_height) + 0.5f);
  if (idx < 0)
    idx = 0;
  if (idx >= (float)ctx->count)
    idx = (float)(ctx->count - 1);

  *out_index = (size_t)idx;

  return rc;
}

/* Segmented Control */

/**
 * @brief cmp_segmented_control_create
 *
 * @param out_control Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_segmented_control_create(cmp_segmented_control_t **out_control) {
  int rc = CMP_SUCCESS;
  struct cmp_segmented_control *ctx = NULL;

  if (!out_control) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_segmented_control_create: Invalid argument\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_segmented_control), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_segmented_control_create: Out of memory\n");

    return rc;
  }

  ctx->segments = NULL;
  ctx->count = 0;
  ctx->selected_idx = 0;

  *out_control = (cmp_segmented_control_t *)ctx;

  return rc;
}

/**
 * @brief cmp_segmented_control_destroy
 *
 * @param control_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_segmented_control_destroy(cmp_segmented_control_t *control_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_segmented_control *ctx =
      (struct cmp_segmented_control *)control_opaque;
  size_t i;

  if (!ctx) {

    return rc;
  }

  if (ctx->segments) {
    for (i = 0; i < ctx->count; ++i)
      CMP_FREE(ctx->segments[i]);
    CMP_FREE(ctx->segments);
  }
  CMP_FREE(ctx);

  return rc;
}

/**
 * @brief cmp_segmented_control_set_segments
 *
 * @param control_opaque Parameter description.
 * @param segments Parameter description.
 * @param count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_segmented_control_set_segments(cmp_segmented_control_t *control_opaque,
                                       const char **segments, size_t count) {
  int rc = CMP_SUCCESS;
  struct cmp_segmented_control *ctx =
      (struct cmp_segmented_control *)control_opaque;
  size_t i, len;
  char **new_segs = NULL;

  if (!ctx || !segments) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_segmented_control_set_segments: Invalid argument\n");

    return rc;
  }

  if (ctx->segments) {
    for (i = 0; i < ctx->count; ++i)
      CMP_FREE(ctx->segments[i]);
    CMP_FREE(ctx->segments);
  }

  ctx->count = count;
  ctx->selected_idx = 0;

  if (count > 0) {
    rc = CMP_MALLOC(count * sizeof(char *), (void **)&new_segs);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_segmented_control_set_segments: Out of memory "
                "allocating pointers\n");

      return rc;
    }
    for (i = 0; i < count; ++i) {
      len = strlen(segments[i]);
      rc = CMP_MALLOC(len + 1, (void **)&new_segs[i]);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_segmented_control_set_segments: Out of memory "
                  "allocating string\n");

        return rc;
      }
#if defined(_MSC_VER)
      strcpy_s(new_segs[i], len + 1, segments[i]);
#else
      strcpy(new_segs[i], segments[i]);
#endif
    }
    ctx->segments = new_segs;
  }

  return rc;
}

/**
 * @brief cmp_segmented_control_select
 *
 * @param control_opaque Parameter description.
 * @param index Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_segmented_control_select(cmp_segmented_control_t *control_opaque,
                                 size_t index) {
  int rc = CMP_SUCCESS;
  struct cmp_segmented_control *ctx =
      (struct cmp_segmented_control *)control_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_segmented_control_select: Invalid argument (ctx=NULL)\n");

    return rc;
  }
  if (index >= ctx->count) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_segmented_control_select: Index out of bounds\n");

    return rc;
  }

  ctx->selected_idx = index;
  /* cmp_haptics_trigger(CMP_HAPTIC_SELECTION); */

  return rc;
}

/**
 * @brief cmp_segmented_control_get_visuals
 *
 * @param control_opaque Parameter description.
 * @param out_selected_idx Parameter description.
 * @param out_slider_offset_x Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_segmented_control_get_visuals(cmp_segmented_control_t *control_opaque,
                                      size_t *out_selected_idx,
                                      float *out_slider_offset_x) {
  int rc = CMP_SUCCESS;
  struct cmp_segmented_control *ctx =
      (struct cmp_segmented_control *)control_opaque;

  if (!ctx || !out_selected_idx || !out_slider_offset_x) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_segmented_control_get_visuals: Invalid argument\n");

    return rc;
  }

  *out_selected_idx = ctx->selected_idx;
  /* Assuming standard 100px segments for mock */
  *out_slider_offset_x = (float)ctx->selected_idx * 100.0f;

  return rc;
}

/* Stepper */

/**
 * @brief cmp_stepper_create
 *
 * @param out_stepper Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stepper_create(cmp_stepper_t **out_stepper) {
  int rc = CMP_SUCCESS;
  struct cmp_stepper *ctx = NULL;

  if (!out_stepper) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_stepper_create: Invalid argument\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_stepper), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_stepper_create: Out of memory\n");

    return rc;
  }

  ctx->value = 0;
  ctx->min_val = 0;
  ctx->max_val = 100;
  ctx->step = 1;

  *out_stepper = (cmp_stepper_t *)ctx;

  return rc;
}

/**
 * @brief cmp_stepper_destroy
 *
 * @param stepper_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stepper_destroy(cmp_stepper_t *stepper_opaque) {
  int rc = CMP_SUCCESS;
  if (stepper_opaque) {
    CMP_FREE(stepper_opaque);
  }

  return rc;
}

/**
 * @brief cmp_stepper_set_limits
 *
 * @param stepper_opaque Parameter description.
 * @param min_val Parameter description.
 * @param max_val Parameter description.
 * @param step Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stepper_set_limits(cmp_stepper_t *stepper_opaque, int min_val,
                           int max_val, int step) {
  int rc = CMP_SUCCESS;
  struct cmp_stepper *ctx = (struct cmp_stepper *)stepper_opaque;

  if (!ctx || max_val < min_val || step <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_stepper_set_limits: Invalid argument\n");

    return rc;
  }

  ctx->min_val = min_val;
  ctx->max_val = max_val;
  ctx->step = step;

  if (ctx->value < min_val)
    ctx->value = min_val;
  if (ctx->value > max_val)
    ctx->value = max_val;

  return rc;
}

/**
 * @brief cmp_stepper_get_value
 *
 * @param stepper_opaque Parameter description.
 * @param out_val Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stepper_get_value(cmp_stepper_t *stepper_opaque, int *out_val) {
  int rc = CMP_SUCCESS;
  struct cmp_stepper *ctx = (struct cmp_stepper *)stepper_opaque;

  if (!ctx || !out_val) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_stepper_get_value: Invalid argument\n");

    return rc;
  }

  *out_val = ctx->value;

  return rc;
}

/**
 * @brief cmp_stepper_increment
 *
 * @param stepper_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stepper_increment(cmp_stepper_t *stepper_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_stepper *ctx = (struct cmp_stepper *)stepper_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_stepper_increment: Invalid argument\n");

    return rc;
  }

  ctx->value += ctx->step;
  if (ctx->value > ctx->max_val)
    ctx->value = ctx->max_val;

  return rc;
}

/**
 * @brief cmp_stepper_decrement
 *
 * @param stepper_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stepper_decrement(cmp_stepper_t *stepper_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_stepper *ctx = (struct cmp_stepper *)stepper_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_stepper_decrement: Invalid argument\n");

    return rc;
  }

  ctx->value -= ctx->step;
  if (ctx->value < ctx->min_val)
    ctx->value = ctx->min_val;

  return rc;
}

/* Slider */

/**
 * @brief cmp_slider_create
 *
 * @param out_slider Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_slider_create(cmp_slider_t **out_slider) {
  int rc = CMP_SUCCESS;
  struct cmp_slider *ctx = NULL;

  if (!out_slider) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_slider_create: Invalid argument\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_slider), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_slider_create: Out of memory\n");

    return rc;
  }

  ctx->value = 0.0f;
  ctx->min_val = 0.0f;
  ctx->max_val = 1.0f;

  *out_slider = (cmp_slider_t *)ctx;

  return rc;
}

/**
 * @brief cmp_slider_destroy
 *
 * @param slider_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_slider_destroy(cmp_slider_t *slider_opaque) {
  int rc = CMP_SUCCESS;
  if (slider_opaque) {
    CMP_FREE(slider_opaque);
  }

  return rc;
}

/**
 * @brief cmp_slider_set_limits
 *
 * @param slider_opaque Parameter description.
 * @param min_val Parameter description.
 * @param max_val Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_slider_set_limits(cmp_slider_t *slider_opaque, float min_val,
                          float max_val) {
  int rc = CMP_SUCCESS;
  struct cmp_slider *ctx = (struct cmp_slider *)slider_opaque;

  if (!ctx || max_val < min_val) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_slider_set_limits: Invalid argument\n");

    return rc;
  }

  ctx->min_val = min_val;
  ctx->max_val = max_val;

  if (ctx->value < min_val)
    ctx->value = min_val;
  if (ctx->value > max_val)
    ctx->value = max_val;

  return rc;
}

/**
 * @brief cmp_slider_set_value
 *
 * @param slider_opaque Parameter description.
 * @param val Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_slider_set_value(cmp_slider_t *slider_opaque, float val) {
  int rc = CMP_SUCCESS;
  struct cmp_slider *ctx = (struct cmp_slider *)slider_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_slider_set_value: Invalid argument\n");

    return rc;
  }

  ctx->value = val;
  if (ctx->value < ctx->min_val)
    ctx->value = ctx->min_val;
  if (ctx->value > ctx->max_val)
    ctx->value = ctx->max_val;

  return rc;
}

/**
 * @brief cmp_slider_get_visuals
 *
 * @param slider_opaque Parameter description.
 * @param out_thumb_x_percent Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_slider_get_visuals(cmp_slider_t *slider_opaque,
                           float *out_thumb_x_percent) {
  int rc = CMP_SUCCESS;
  struct cmp_slider *ctx = (struct cmp_slider *)slider_opaque;
  float range;

  if (!ctx || !out_thumb_x_percent) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_slider_get_visuals: Invalid argument\n");

    return rc;
  }

  range = ctx->max_val - ctx->min_val;
  if (range <= 0.0f) {
    *out_thumb_x_percent = 0.0f;
  } else {
    *out_thumb_x_percent = (ctx->value - ctx->min_val) / range;
  }

  return rc;
}

/**
 * @brief cmp_slider_update_from_drag
 *
 * @param slider_opaque Parameter description.
 * @param track_width Parameter description.
 * @param current_x Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_slider_update_from_drag(cmp_slider_t *slider_opaque, float track_width,
                                float current_x) {
  int rc = CMP_SUCCESS;
  struct cmp_slider *ctx = (struct cmp_slider *)slider_opaque;
  float ratio;

  if (!ctx || track_width <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_slider_update_from_drag: Invalid argument\n");

    return rc;
  }

  ratio = current_x / track_width;
  if (ratio < 0.0f)
    ratio = 0.0f;
  if (ratio > 1.0f)
    ratio = 1.0f;

  ctx->value = ctx->min_val + (ratio * (ctx->max_val - ctx->min_val));

  return rc;
}

/* System Dialog Wrappers (Platform-specific implementations to be added in
 * Phase 25) */

/**
 * @brief cmp_system_color_picker_show
 *
 * @param window Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_color_picker_show(cmp_window_t *window) {
  int rc = CMP_SUCCESS;

  if (!window) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_system_color_picker_show: Invalid argument\n");

    return rc;
  }

  /* Maps to UIColorPickerViewController or NSColorPanel */

  return rc;
}

/**
 * @brief cmp_system_date_picker_show
 *
 * @param window Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_date_picker_show(cmp_window_t *window) {
  int rc = CMP_SUCCESS;

  if (!window) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_system_date_picker_show: Invalid argument\n");

    return rc;
  }

  /* UIDatePicker preferredStyle: .compact */

  return rc;
}
