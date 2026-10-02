/**
 * @file test_ui_dpi_scaling.c
 * @brief Unit tests for high-DPI scaling, scale factor query, and DPI change
 * events.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/ui_window_backend.h"
#include "../include/ui_event.h"
#include "../include/ui_error.h"
#include "../include/ui_geometry.h"
/* clang-format on */

/**
 * @brief Custom DPI change callback for testing.
 * @param user_data Pointer to user context.
 * @param old_scale Previous scale factor.
 * @param new_scale New scale factor.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t mock_dpi_callback(void *user_data, float old_scale,
                                    float new_scale) {
  int *call_count = (int *)user_data;
  if (call_count) {
    (*call_count)++;
  }
  if (old_scale < 0.0f || new_scale < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Mock get_scale_factor implementation for backend tests.
 * @param backend Pointer to backend.
 * @param window Pointer to window.
 * @param out_scale_factor Pointer to store scale factor.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on NULL.
 */
static ui_error_t mock_get_scale_factor(struct ui_window_backend *backend,
                                        struct ui_window *window,
                                        float *out_scale_factor) {
  if (!backend || !window || !out_scale_factor) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_scale_factor = 2.0f;
  return UI_ERROR_NONE;
}

/**
 * @brief Mock get_framebuffer_size implementation.
 * @param backend Pointer to backend.
 * @param window Pointer to window.
 * @param out_fb_width Pointer to store framebuffer width.
 * @param out_fb_height Pointer to store framebuffer height.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t mock_get_framebuffer_size(struct ui_window_backend *backend,
                                            struct ui_window *window,
                                            int *out_fb_width,
                                            int *out_fb_height) {
  float scale = 1.0f;
  ui_error_t rc;

  if (!backend || !window || !out_fb_width || !out_fb_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = mock_get_scale_factor(backend, window, &scale);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  *out_fb_width = (int)(640.0f * scale);
  *out_fb_height = (int)(480.0f * scale);
  return UI_ERROR_NONE;
}

/**
 * @brief Mock set_on_dpi_change_callback.
 * @param backend Pointer to backend.
 * @param window Pointer to window.
 * @param callback Callback function.
 * @param user_data User data.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t mock_set_on_dpi_change_callback(
    struct ui_window_backend *backend, struct ui_window *window,
    ui_error_t (*callback)(void *, float, float), void *user_data) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (callback) {
    ui_error_t rc = callback(user_data, 1.0f, 2.0f);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Coordinate transformation test function: transforms logical points to
 * physical pixels.
 * @param logical_x Logical X coordinate in points.
 * @param logical_y Logical Y coordinate in points.
 * @param scale_factor DPI scale factor.
 * @param out_physical_x Output physical pixel X.
 * @param out_physical_y Output physical pixel Y.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t transform_logical_to_physical(float logical_x,
                                                float logical_y,
                                                float scale_factor,
                                                int *out_physical_x,
                                                int *out_physical_y) {
  if (!out_physical_x || !out_physical_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (scale_factor <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_physical_x = (int)(logical_x * scale_factor);
  *out_physical_y = (int)(logical_y * scale_factor);
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_window_backend backend;
  struct ui_window *dummy_window;
  float scale = 0.0f;
  int fb_w = 0;
  int fb_h = 0;
  int cb_invocations = 0;
  int phys_x = 0;
  int phys_y = 0;
  struct ui_event evt;
  ui_error_t rc;

  dummy_window = (struct ui_window *)1;
  memset(&backend, 0, sizeof(backend));
  backend.get_scale_factor = mock_get_scale_factor;
  backend.get_framebuffer_size = mock_get_framebuffer_size;
  backend.set_on_dpi_change_callback = mock_set_on_dpi_change_callback;

  /* 1. Test get_scale_factor error validation */
  rc = backend.get_scale_factor(NULL, dummy_window, &scale);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  rc = backend.get_scale_factor(&backend, NULL, &scale);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 2;
  }
  rc = backend.get_scale_factor(&backend, dummy_window, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 3;
  }
  rc = backend.get_scale_factor(&backend, dummy_window, &scale);
  if (rc != UI_ERROR_NONE || scale != 2.0f) {
    return 4;
  }

  /* 2. Test get_framebuffer_size error validation */
  rc = backend.get_framebuffer_size(NULL, dummy_window, &fb_w, &fb_h);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 5;
  }
  rc = backend.get_framebuffer_size(&backend, NULL, &fb_w, &fb_h);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 6;
  }
  rc = backend.get_framebuffer_size(&backend, dummy_window, NULL, &fb_h);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 7;
  }
  rc = backend.get_framebuffer_size(&backend, dummy_window, &fb_w, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 8;
  }
  rc = backend.get_framebuffer_size(&backend, dummy_window, &fb_w, &fb_h);
  if (rc != UI_ERROR_NONE || fb_w != 1280 || fb_h != 960) {
    return 9;
  }

  /* 3. Test set_on_dpi_change_callback */
  rc = backend.set_on_dpi_change_callback(NULL, dummy_window, mock_dpi_callback,
                                          &cb_invocations);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 10;
  }
  rc = backend.set_on_dpi_change_callback(&backend, NULL, mock_dpi_callback,
                                          &cb_invocations);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 11;
  }
  rc = backend.set_on_dpi_change_callback(&backend, dummy_window,
                                          mock_dpi_callback, &cb_invocations);
  if (rc != UI_ERROR_NONE || cb_invocations != 1) {
    return 12;
  }

  /* 4. Test ui_geometry_transform_logical_to_physical & physical_to_logical */
  rc = ui_geometry_transform_logical_to_physical(100.0f, 200.0f, 0.0f, &phys_x,
                                                 &phys_y);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 13;
  }
  rc = ui_geometry_transform_logical_to_physical(100.0f, 200.0f, -1.0f, &phys_x,
                                                 &phys_y);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 14;
  }
  rc = ui_geometry_transform_logical_to_physical(100.0f, 200.0f, 1.5f, NULL,
                                                 &phys_y);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 15;
  }
  rc = ui_geometry_transform_logical_to_physical(100.0f, 200.0f, 1.5f, &phys_x,
                                                 NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return 16;
  }
  rc = ui_geometry_transform_logical_to_physical(100.0f, 200.0f, 2.0f, &phys_x,
                                                 &phys_y);
  if (rc != UI_ERROR_NONE || phys_x != 200 || phys_y != 400) {
    return 17;
  }

  {
    float log_x = 0.0f;
    float log_y = 0.0f;
    rc = ui_geometry_transform_physical_to_logical(200, 400, 2.0f, &log_x,
                                                   &log_y);
    if (rc != UI_ERROR_NONE || log_x != 100.0f || log_y != 200.0f) {
      return 18;
    }
    rc = ui_geometry_transform_physical_to_logical(200, 400, 0.0f, &log_x,
                                                   &log_y);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      return 19;
    }
  }

  /* 5. Test UI_EVENT_WINDOW_DPI_CHANGED payload */
  memset(&evt, 0, sizeof(evt));
  evt.type = UI_EVENT_WINDOW_DPI_CHANGED;
  evt.event_data.dpi.old_scale_factor = 1.0f;
  evt.event_data.dpi.new_scale_factor = 2.0f;

  if (evt.type != UI_EVENT_WINDOW_DPI_CHANGED ||
      evt.event_data.dpi.old_scale_factor != 1.0f ||
      evt.event_data.dpi.new_scale_factor != 2.0f) {
    return 18;
  }

  printf("%s\n", "test_ui_dpi_scaling passed.");
  return 0;
}
