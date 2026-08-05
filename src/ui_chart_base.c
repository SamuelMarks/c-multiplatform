/* clang-format off */
#include "ui_chart_base.h"
#include "ui_internal_mem.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
#include <math.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_chart_mock_fail = -1;
#endif

struct ui_chart_base {
  struct ui_arena *arena;
  enum ui_chart_coordinate_system coord_system;

  struct ui_chart_scale_config x_scale;
  struct ui_chart_scale_config y_scale;

  struct ui_dom_rect draw_bounds;

  ui_signal_t *topology_signal;
};

static ui_error_t void_equality(union ui_signal_payload a,
                                union ui_signal_payload b,
                                ui_bool_t *out_equal) {
  (void)a;
  (void)b;
  /* Internal signal callback assumes out_equal is valid */
  *out_equal = UI_FALSE; /* Always trigger */
  return UI_ERROR_NONE;
}

ui_error_t ui_chart_base_create(struct ui_arena *arena,
                                enum ui_chart_coordinate_system coord_system,
                                struct ui_chart_base **out_chart) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !out_chart) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_chart_base), 8, &ptr);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      err = UI_ERROR_OUT_OF_MEMORY;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (err != UI_ERROR_NONE)
    return err;

  *out_chart = (struct ui_chart_base *)ptr;
  (*out_chart)->arena = arena;
  (*out_chart)->coord_system = coord_system;

  /* Default bounds and scales */
  (*out_chart)->draw_bounds.x = 0;
  (*out_chart)->draw_bounds.y = 0;
  (*out_chart)->draw_bounds.width = 100;
  (*out_chart)->draw_bounds.height = 100;

  (*out_chart)->x_scale.type = UI_CHART_SCALE_LINEAR;
  (*out_chart)->x_scale.domain_min = 0.0;
  (*out_chart)->x_scale.domain_max = 1.0;
  (*out_chart)->x_scale.range_min = 0.0;
  (*out_chart)->x_scale.range_max = 100.0;

  (*out_chart)->y_scale.type = UI_CHART_SCALE_LINEAR;
  (*out_chart)->y_scale.domain_min = 0.0;
  (*out_chart)->y_scale.domain_max = 1.0;
  (*out_chart)->y_scale.range_min =
      100.0; /* Inverted for Y-down coords usually */
  (*out_chart)->y_scale.range_max = 0.0;

  initial_payload.ptr_val = NULL;
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_POINTER,
                         void_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_chart)->topology_signal);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      err = UI_ERROR_OUT_OF_MEMORY;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (err != UI_ERROR_NONE)
    return err;

  return UI_ERROR_NONE;
}

ui_error_t ui_chart_base_destroy(struct ui_chart_base *chart) {
  if (!chart)
    return UI_ERROR_INVALID_ARGUMENT;
  (void)ui_signal_destroy(chart->topology_signal);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_chart_base_set_x_scale(struct ui_chart_base *chart,
                          const struct ui_chart_scale_config *config) {
  union ui_signal_payload payload;
  if (!chart || !config)
    return UI_ERROR_INVALID_ARGUMENT;

  chart->x_scale = *config;

  payload.ptr_val = NULL;
  return ui_signal_set(chart->topology_signal, payload);
}

/** \brief ui_error */
ui_error_t
ui_chart_base_set_y_scale(struct ui_chart_base *chart,
                          const struct ui_chart_scale_config *config) {
  union ui_signal_payload payload;
  if (!chart || !config)
    return UI_ERROR_INVALID_ARGUMENT;

  chart->y_scale = *config;

  payload.ptr_val = NULL;
  return ui_signal_set(chart->topology_signal, payload);
}

ui_error_t ui_chart_base_set_draw_bounds(struct ui_chart_base *chart,
                                         const struct ui_dom_rect *bounds) {
  union ui_signal_payload payload;
  if (!chart || !bounds)
    return UI_ERROR_INVALID_ARGUMENT;

  chart->draw_bounds = *bounds;

  payload.ptr_val = NULL;
  return ui_signal_set(chart->topology_signal, payload);
}

static ui_error_t scale_value(double val,
                              const struct ui_chart_scale_config *scale,
                              double *out_val) {
  double domain_span = scale->domain_max - scale->domain_min;
  double range_span = scale->range_max - scale->range_min;
  double normalized = 0.0;

#ifdef UI_TEST_MOCK_ALLOC
  if (g_chart_mock_fail == 0) {
    g_chart_mock_fail = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_chart_mock_fail > 0)
    g_chart_mock_fail--;
#endif

  if (domain_span == 0.0) {
    *out_val = scale->range_min; /* Avoid div/0 */
    return UI_ERROR_NONE;
  }

  if (scale->type == UI_CHART_SCALE_LOGARITHMIC) {
    /* Basic log mapping. Guard against <= 0 domains in real implementation */
    double log_min = log(scale->domain_min > 0 ? scale->domain_min : 1e-10);
    double log_max = log(scale->domain_max > 0 ? scale->domain_max : 1e-10);
    double log_val = log(val > 0 ? val : 1e-10);
    normalized = (log_val - log_min) / (log_max - log_min);
  } else {
    /* Linear and Time (unix epoch seconds) map the same mathematically */
    normalized = (val - scale->domain_min) / domain_span;
  }

  *out_val = scale->range_min + (normalized * range_span);
  return UI_ERROR_NONE;
}

static ui_error_t unscale_value(double pixel,
                                const struct ui_chart_scale_config *scale,
                                double *out_val) {
  double domain_span = scale->domain_max - scale->domain_min;
  double range_span = scale->range_max - scale->range_min;
  double normalized = 0.0;

#ifdef UI_TEST_MOCK_ALLOC
  if (g_chart_mock_fail == 0) {
    g_chart_mock_fail = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_chart_mock_fail > 0)
    g_chart_mock_fail--;
#endif

  if (range_span == 0.0) {
    *out_val = scale->domain_min;
    return UI_ERROR_NONE;
  }

  normalized = (pixel - scale->range_min) / range_span;

  if (scale->type == UI_CHART_SCALE_LOGARITHMIC) {
    double log_min = log(scale->domain_min > 0 ? scale->domain_min : 1e-10);
    double log_max = log(scale->domain_max > 0 ? scale->domain_max : 1e-10);
    double log_val = log_min + (normalized * (log_max - log_min));
    *out_val = exp(log_val);
  } else {
    *out_val = scale->domain_min + (normalized * domain_span);
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_chart_base_data_to_pixel(const struct ui_chart_base *chart,
                                       double data_x, double data_y,
                                       struct ui_dom_point *out_pixel_point) {
  if (!chart || !out_pixel_point)
    return UI_ERROR_INVALID_ARGUMENT;

  if (chart->coord_system == UI_CHART_COORDINATE_CARTESIAN) {
    ui_error_t rc;
    out_pixel_point->x = 0.0f;
    rc = scale_value(data_x, &chart->x_scale, &out_pixel_point->x);
    if (rc != UI_ERROR_NONE)
      return rc;
    out_pixel_point->x += chart->draw_bounds.x;
    out_pixel_point->y = 0.0f;
    rc = scale_value(data_y, &chart->y_scale, &out_pixel_point->y);
    if (rc != UI_ERROR_NONE)
      return rc;
    out_pixel_point->y += chart->draw_bounds.y;
  } else if (chart->coord_system == UI_CHART_COORDINATE_POLAR) {
    /* data_x is angle (radians), data_y is radius */
    double cx = chart->draw_bounds.x + chart->draw_bounds.width / 2.0;
    double cy = chart->draw_bounds.y + chart->draw_bounds.height / 2.0;
    double radius = 0.0;
    double angle = 0.0;
    ui_error_t rc;
    rc = scale_value(data_y, &chart->y_scale, &radius);
    if (rc != UI_ERROR_NONE)
      return rc;
    /* In polar, x_scale could map abstract angle (e.g. 0-100%) to 0-2PI */
    rc = scale_value(data_x, &chart->x_scale, &angle);
    if (rc != UI_ERROR_NONE)
      return rc;

    out_pixel_point->x = cx + (radius * cos(angle));
    out_pixel_point->y = cy + (radius * sin(angle));
  }

  out_pixel_point->z = 0.0;
  out_pixel_point->w = 1.0;

  return UI_ERROR_NONE;
}

ui_error_t ui_chart_base_pixel_to_data(const struct ui_chart_base *chart,
                                       double pixel_x, double pixel_y,
                                       double *out_data_x, double *out_data_y) {

  if (!chart || !out_data_x || !out_data_y)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Bounds check */
  if (pixel_x < chart->draw_bounds.x ||
      pixel_x > chart->draw_bounds.x + chart->draw_bounds.width ||
      pixel_y < chart->draw_bounds.y ||
      pixel_y > chart->draw_bounds.y + chart->draw_bounds.height) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (chart->coord_system == UI_CHART_COORDINATE_CARTESIAN) {
    double rel_x = pixel_x - chart->draw_bounds.x;
    double rel_y = pixel_y - chart->draw_bounds.y;
    ui_error_t rc;
    rc = unscale_value(rel_x, &chart->x_scale, out_data_x);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = unscale_value(rel_y, &chart->y_scale, out_data_y);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (chart->coord_system == UI_CHART_COORDINATE_POLAR) {
    double cx = chart->draw_bounds.x + chart->draw_bounds.width / 2.0;
    double cy = chart->draw_bounds.y + chart->draw_bounds.height / 2.0;
    double dx = pixel_x - cx;
    double dy = pixel_y - cy;
    ui_error_t rc;

    double radius = sqrt((dx * dx) + (dy * dy));
    double angle = atan2(dy, dx);
    if (angle < 0)
      angle += 2.0 * 3.14159265358979323846; /* Normalize to 0-2PI */

    rc = unscale_value(radius, &chart->y_scale, out_data_y);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = unscale_value(angle, &chart->x_scale, out_data_x);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_chart_base_get_topology_signal(struct ui_chart_base *chart,
                                             ui_signal_t **out_signal) {
  if (!chart || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = chart->topology_signal;
  return UI_ERROR_NONE;
}
