/*
 * \file ui_ink_base.c
 * \brief Implementation of the generic ink drawing component.
 */
/* clang-format off */
#include "ui_ink_base.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <math.h>
/* clang-format on */

/**
 * @struct ui_ink_base
 * \struct ui_ink_base
 * \brief Internal state for a generic ink component tracking raw and smoothed
 * points.
 */
struct ui_ink_base {
  struct ui_component *component;  /**< component */
  struct ui_ink_event *raw_points; /**< raw_points */
  size_t raw_capacity;             /**< raw_capacity */
  size_t raw_count;                /**< raw_count */

  struct ui_ink_event *smoothed_points; /**< smoothed_points */
  size_t smoothed_capacity;             /**< smoothed_capacity */
  size_t smoothed_count;                /**< smoothed_count */
};

/*
 * \brief Creates a new ink base component.
 * \param[out] out_ink Pointer to store the created ink component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_ink_base_create(struct ui_ink_base **out_ink) {
  struct ui_ink_base *ink;
  struct ui_dom_node *root_node = NULL;
  ui_error_t rc;

  if (!out_ink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ink =
      (struct ui_ink_base *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_ink_base));
  if (!ink) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(ink, 0, sizeof(struct ui_ink_base));

  rc = ui_component_create(&ink->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ink);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(ink->component);
    C_MULTIPLATFORM_FREE(ink);
    return rc;
  }

  {
    ui_error_t rc2 = ui_dom_node_set_tag_name(root_node, "canvas");
    if (rc2 != UI_ERROR_NONE) {
      (void)ui_component_destroy(ink->component);
      (void)ui_dom_node_destroy(root_node);
      C_MULTIPLATFORM_FREE(ink);
      return rc2;
    }
  }
  {
    ui_error_t rc3 = ui_dom_node_set_attribute(root_node, "role", "img");
    if (rc3 != UI_ERROR_NONE) {
      (void)ui_component_destroy(ink->component);
      (void)ui_dom_node_destroy(root_node);
      C_MULTIPLATFORM_FREE(ink);
      return rc3;
    }
  }
  ink->component->shadow_root = root_node;

  *out_ink = ink;
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys an ink base component.
 * \param[in,out] ink The ink component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_ink_base_destroy(struct ui_ink_base *ink) {
  if (!ink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (ink->component) {
    (void)ui_component_destroy(ink->component);
  }
  if (ink->raw_points) {
    C_MULTIPLATFORM_FREE(ink->raw_points);
  }
  if (ink->smoothed_points) {
    C_MULTIPLATFORM_FREE(ink->smoothed_points);
  }
  C_MULTIPLATFORM_FREE(ink);
  return UI_ERROR_NONE;
}

/*
 * \brief Retrieves the underlying DOM component of the ink widget.
 * \param[in,out] ink The ink component.
 * \param[out] out_component Pointer to store the DOM component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_ink_base_get_component(struct ui_ink_base *ink,
                                     struct ui_component **out_component) {
  if (!ink || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = ink->component;
  return UI_ERROR_NONE;
}

/*
 * \brief Interpolates a point using Catmull-Rom splines.
 * \param[in] p0 First control point.
 * \param[in] p1 Second control point (start of segment).
 * \param[in] p2 Third control point (end of segment).
 * \param[in] p3 Fourth control point.
 * \param[in] t Interpolation parameter (0.0 to 1.0).
 * \param[out] out The interpolated point.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t interpolate_catmull_rom(const struct ui_ink_event *p0,
                                          const struct ui_ink_event *p1,
                                          const struct ui_ink_event *p2,
                                          const struct ui_ink_event *p3,
                                          float t, struct ui_ink_event *out) {
  float t2, t3;
  t2 = t * t;
  t3 = t2 * t;

  out->x = 0.5f * ((2.0f * p1->x) + (-p0->x + p2->x) * t +
                   (2.0f * p0->x - 5.0f * p1->x + 4.0f * p2->x - p3->x) * t2 +
                   (-p0->x + 3.0f * p1->x - 3.0f * p2->x + p3->x) * t3);
  out->y = 0.5f * ((2.0f * p1->y) + (-p0->y + p2->y) * t +
                   (2.0f * p0->y - 5.0f * p1->y + 4.0f * p2->y - p3->y) * t2 +
                   (-p0->y + 3.0f * p1->y - 3.0f * p2->y + p3->y) * t3);

  out->pressure = p1->pressure + (p2->pressure - p1->pressure) * t;
  out->tilt_x = p1->tilt_x + (p2->tilt_x - p1->tilt_x) * t;
  out->tilt_y = p1->tilt_y + (p2->tilt_y - p1->tilt_y) * t;
  out->azimuth = p1->azimuth + (p2->azimuth - p1->azimuth) * t;
  out->timestamp = p1->timestamp + (p2->timestamp - p1->timestamp) * t;
  return UI_ERROR_NONE;
}

/*
 * \brief Appends a smoothed curve segment between points to the ink context.
 * \param[in,out] ink The ink context.
 * \param[in] p0 First control point.
 * \param[in] p1 Second control point.
 * \param[in] p2 Third control point.
 * \param[in] p3 Fourth control point.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t append_smoothed_segment(struct ui_ink_base *ink,
                                          const struct ui_ink_event *p0,
                                          const struct ui_ink_event *p1,
                                          const struct ui_ink_event *p2,
                                          const struct ui_ink_event *p3) {
  int i;
  int steps = 4;
  for (i = 1; i <= steps; ++i) {
    float t = (float)i / (float)steps;
    struct ui_ink_event smoothed;
    (void)interpolate_catmull_rom(p0, p1, p2, p3, t, &smoothed);

    if (ink->smoothed_count >= ink->smoothed_capacity) {
      size_t new_cap = ink->smoothed_capacity * 2;
      struct ui_ink_event *new_arr = C_MULTIPLATFORM_REALLOC(
          ink->smoothed_points, (size_t)new_cap * sizeof(struct ui_ink_event));
      if (!new_arr) {
        return UI_ERROR_OUT_OF_MEMORY;
      }
      ink->smoothed_points = new_arr;
      ink->smoothed_capacity = new_cap;
    }
    ink->smoothed_points[ink->smoothed_count++] = smoothed;
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Adds a raw pointer event to the ink stroke and smooths it.
 * \param[in,out] ink The ink context.
 * \param[in] event The raw ink event to add.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_ink_base_add_event(struct ui_ink_base *ink,
                                 const struct ui_ink_event *event) {
  if (!ink || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ink->raw_count >= ink->raw_capacity) {
    size_t new_cap = ink->raw_capacity == 0 ? 16 : ink->raw_capacity * 2;
    struct ui_ink_event *new_arr = C_MULTIPLATFORM_REALLOC(
        ink->raw_points, (size_t)new_cap * sizeof(struct ui_ink_event));
    if (!new_arr) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    ink->raw_points = new_arr;
    ink->raw_capacity = new_cap;
  }
  ink->raw_points[ink->raw_count++] = *event;

  if (ink->raw_count == 1) {
    size_t new_cap = 32;
    struct ui_ink_event *new_arr = C_MULTIPLATFORM_REALLOC(
        ink->smoothed_points, (size_t)new_cap * sizeof(struct ui_ink_event));
    if (!new_arr) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    ink->smoothed_points = new_arr;
    ink->smoothed_capacity = new_cap;
    ink->smoothed_points[ink->smoothed_count++] = *event;
  } else if (ink->raw_count >= 3) {
    size_t n = ink->raw_count;
    const struct ui_ink_event *p0 =
        (n >= 4) ? &ink->raw_points[n - 4] : &ink->raw_points[0];
    const struct ui_ink_event *p1 = &ink->raw_points[n - 3];
    const struct ui_ink_event *p2 = &ink->raw_points[n - 2];
    const struct ui_ink_event *p3 = &ink->raw_points[n - 1];
    return append_smoothed_segment(ink, p0, p1, p2, p3);
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Completes the current ink stroke, finalizing the smoothing.
 * \param[in,out] ink The ink context.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_ink_base_finish_stroke(struct ui_ink_base *ink) {
  if (!ink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ink->raw_count == 2) {
    ink->smoothed_points[ink->smoothed_count++] = ink->raw_points[1];
  } else if (ink->raw_count >= 3) {
    size_t n = ink->raw_count;
    const struct ui_ink_event *p0 = &ink->raw_points[n - 3];
    const struct ui_ink_event *p1 = &ink->raw_points[n - 2];
    const struct ui_ink_event *p2 = &ink->raw_points[n - 1];
    const struct ui_ink_event *p3 = &ink->raw_points[n - 1];
    return append_smoothed_segment(ink, p0, p1, p2, p3);
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Retrieves the count of smoothed points in the ink stroke.
 * \param[in] ink The ink context.
 * \param[out] out_count Pointer to store the point count.
 * \return UI_ERROR_NONE on success.
 */
/*
 * \brief Retrieves a specific smoothed point from the ink stroke.
 * \param[in] ink The ink context.
 * \param[in] index The index of the point.
 * \param[out] out_point Pointer to store the retrieved point.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_ink_base_get_smoothed_points_count(struct ui_ink_base *ink,
                                                 size_t *out_count) {
  if (!ink || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = ink->smoothed_count;
  return UI_ERROR_NONE;
}

/*
 * \brief Retrieves a specific smoothed point from the ink stroke.
 * \param[in] ink The ink context.
 * \param[in] index The index of the point.
 * \param[out] out_point Pointer to store the retrieved point.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_ink_base_get_smoothed_point(struct ui_ink_base *ink, size_t index,
                                          struct ui_ink_event *out_point) {
  if (!ink || !out_point) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (index >= ink->smoothed_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  *out_point = ink->smoothed_points[index];
  return UI_ERROR_NONE;
}
