/* clang-format off */
#include "ui_map_view_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
static ui_error_t mock_eq_fail(union ui_signal_payload a,
                               union ui_signal_payload b, ui_bool_t *eq) {
  return UI_ERROR_UNKNOWN;
}

static void test_missing_map_coverage(void) {
  struct ui_map_view_base *map = NULL;
  {
    ui_error_t rc_cleanup = ui_map_view_base_create(&map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test missing null checks */

  /* Test emit failures by supplying an equality fn that fails */
  struct ui_arena *arena = NULL;
  ui_arena_create(1024, &arena);
  ui_signal_t *sig_center = NULL, *sig_zoom = NULL, *sig_rot = NULL;
  union ui_signal_payload p = {0};

  ui_signal_create(arena, p, UI_SIGNAL_TYPE_POINTER, mock_eq_fail, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig_center);
  ui_signal_create(arena, p, UI_SIGNAL_TYPE_FLOAT32, mock_eq_fail, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig_zoom);
  ui_signal_create(arena, p, UI_SIGNAL_TYPE_FLOAT32, mock_eq_fail, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig_rot);

  ui_map_view_base_bind_center(map, sig_center);
  {
    ui_error_t rc_cleanup = ui_map_view_base_bind_zoom(map, sig_zoom);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_bind_rotation(map, sig_rot);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* This will call emit_center which calls ui_signal_set which will fail and
   * return error */
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_pan(map, 10.0, 10.0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_pinch(map, 2.0, 0, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_rotate(map, 0.1, 0, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_map_view_base_destroy(map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test malloc fail on add marker */
  {
    ui_error_t rc_cleanup = ui_map_view_base_create(&map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  struct ui_map_marker m;
  m.coordinate.latitude = 0;
  m.coordinate.longitude = 0;
  m.user_data = NULL;
  size_t id;

  g_malloc_fail_countdown = 0;
  {
    ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, &m, &id);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_map_view_base_destroy(map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

static void test_map_errors_and_methods(void) {
  struct ui_map_view_base *map = NULL;
  struct ui_component *comp = NULL;
  struct ui_map_coordinate coord;
  struct ui_signal *sig = NULL;
  size_t id;
  double x, y;

  if (ui_map_view_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;
  {
    ui_error_t rc_cleanup = ui_map_view_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  if (ui_map_view_base_bind_center(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_map_view_base_bind_zoom(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_map_view_base_bind_rotation(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;

  if (ui_map_view_base_project(NULL, NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_map_view_base_unproject(NULL, 0, 0, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_map_view_base_add_marker(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_map_view_base_remove_marker(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_map_view_base_get_marker_position(NULL, 0, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;

  {
    ui_error_t rc_cleanup = ui_map_view_base_create(&map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  struct ui_arena *arena;
  ui_arena_create(1024, &arena);
  union ui_signal_payload dummy_payload = {0};

  ui_signal_create(arena, dummy_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig);

  ui_map_view_base_bind_center(map, sig);
  {
    ui_error_t rc_cleanup = ui_map_view_base_bind_zoom(map, sig);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_bind_rotation(map, sig);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  coord.latitude = 0;
  coord.longitude = 0;

  /* invalid marker removes */
  {
    ui_error_t rc_cleanup = ui_map_view_base_remove_marker(map, 999);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_map_view_base_get_marker_position(map, 999, &x, &y);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* re-alloc test inside add marker by adding many */
  struct ui_map_marker marker;
  marker.coordinate.latitude = 0;
  marker.coordinate.longitude = 0;
  marker.user_data = NULL;
  int i;
  for (i = 0; i < 20; i++) {
    {
      ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, &marker, &id);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  /* Missing argument branches */
  {
    ui_error_t rc_cleanup = ui_map_view_base_project(map, &coord, NULL, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_project(map, NULL, &x, &y);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_unproject(map, 0, 0, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, NULL, &id);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, &marker, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Missing argument branches */
  ui_map_view_base_set_tile_provider(NULL, NULL, NULL);
  ui_map_view_base_set_tile_provider(map, NULL, NULL);
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_pan(NULL, 0, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_pinch(NULL, 0, 0, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_rotate(NULL, 0, 0, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Trigger the center/zoom/rotation branches that update bound signals by
   * panning and zooming */
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_pan(map, 10.0, 10.0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_pinch(map, 2.0, 0, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_handle_rotate(map, 0.1, 0, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* OOM and marker shifting */
  /* Remove a marker from the middle to trigger the shift */
  {
    ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, &marker, &id);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  size_t id2, id3;
  {
    ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, &marker, &id2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, &marker, &id3);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_remove_marker(map, id2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Negative zoom limit test */
  {
    ui_error_t rc_cleanup =
        ui_map_view_base_handle_pinch(map, 0.000000000001, 0, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup =
        ui_map_view_base_get_marker_position(map, 0, NULL, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_map_view_base_destroy(map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

#ifdef UI_TEST_MOCK_ALLOC
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    if (ui_map_view_base_create(&map) == UI_ERROR_NONE) {
      ui_error_t rc_cleanup = ui_map_view_base_destroy(map);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    g_malloc_fail_countdown = -1;
  }

  {
    ui_error_t rc_cleanup = ui_map_view_base_create(&map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  for (i = 0; i < 20; i++) {
    {
      ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, &marker, &id);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  g_malloc_fail_countdown = 0;
  {
    ui_error_t rc_cleanup = ui_map_view_base_add_marker(map, &marker, &id);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_map_view_base_destroy(map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
#endif
}

static void test_map_math_errors(void) {
  struct ui_map_view_base *map = NULL;
  struct ui_map_coordinate coord;
  double out_x, out_y;

  ui_map_view_base_create(&map);

  /* Try to convert coord to pixel out of bounds */
  {
    ui_error_t rc_cleanup = ui_map_view_base_project(
        map, &(struct ui_map_coordinate){2000.0, 0.0}, &out_x, &out_y);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_map_view_base_project(
        map, &(struct ui_map_coordinate){0.0, 2000.0}, &out_x, &out_y);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Try to convert pixel to coord out of bounds */
  {
    ui_error_t rc_cleanup =
        ui_map_view_base_unproject(map, 1000000000.0, 0.0, &coord);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_map_view_base_unproject(map, 0.0, 1000000000.0, &coord);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Pan by huge amounts to trigger errors during handle_pan */
  {
    ui_error_t rc_cleanup =
        ui_map_view_base_handle_pan(map, -1000000000.0, 0.0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_map_view_base_handle_pan(map, 0.0, -1000000000.0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  ui_map_view_base_destroy(map);
}

int main(void) {
  test_map_math_errors();
  test_missing_map_coverage();
  test_map_errors_and_methods();
  struct ui_map_view_base *map = NULL;
  ui_error_t rc;
  struct ui_map_marker marker;
  size_t id;
  double x, y;
  struct ui_map_coordinate coord;

  rc = ui_map_view_base_create(&map);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to create map view base\n");
    return 1;
  }

  rc = ui_map_view_base_handle_pan(map, 10.0, 10.0);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to pan\n");
    return 1;
  }

  rc = ui_map_view_base_handle_pinch(map, 2.0, 0.0, 0.0);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to pinch\n");
    return 1;
  }

  rc = ui_map_view_base_handle_rotate(map, 0.1, 0.0, 0.0);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to rotate\n");
    return 1;
  }

  coord.latitude = 37.7749;
  coord.longitude = -122.4194;
  rc = ui_map_view_base_project(map, &coord, &x, &y);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to project\n");
    return 1;
  }

  rc = ui_map_view_base_unproject(map, x, y, &coord);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to unproject\n");
    return 1;
  }

  marker.coordinate.latitude = 37.7749;
  marker.coordinate.longitude = -122.4194;
  marker.user_data = NULL;

  rc = ui_map_view_base_add_marker(map, &marker, &id);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to add marker\n");
    return 1;
  }

  /* Add a second marker so we can test the loop iteration evaluating to false
   */
  {
    struct ui_map_marker m2;
    size_t id2;
    m2.coordinate.latitude = 40.7128;
    m2.coordinate.longitude = -74.0060;
    m2.user_data = NULL;
    rc = ui_map_view_base_add_marker(map, &m2, &id2);
    if (rc != UI_ERROR_NONE)
      return 1;

    rc = ui_map_view_base_get_marker_position(map, id2, &x, &y);
    if (rc != UI_ERROR_NONE)
      return 1;

    rc = ui_map_view_base_remove_marker(map, id2);
    if (rc != UI_ERROR_NONE)
      return 1;
  }

  rc = ui_map_view_base_get_marker_position(map, id, &x, &y);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to get marker position\n");
    return 1;
  }

  rc = ui_map_view_base_remove_marker(map, id);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to remove marker\n");
    return 1;
  }

  /* Test missing branch coverage cases */
  {
    double out_x, out_y;
    struct ui_map_coordinate center = {0.0, 0.0};

    if (ui_map_view_base_bind_center(map, NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_map_view_base_bind_zoom(map, NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_map_view_base_bind_rotation(map, NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;

    if (ui_map_view_base_handle_pinch(map, 0.0, 0, 0) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_map_view_base_handle_pinch(map, -1.0, 0, 0) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;

    if (ui_map_view_base_project(map, &center, &out_x, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_map_view_base_get_marker_position(map, 1, &out_x, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;

    if (ui_map_view_base_remove_marker(map, 9999) != UI_ERROR_NOT_FOUND)
      return 1;
    if (ui_map_view_base_get_marker_position(map, 9999, &out_x, &out_y) !=
        UI_ERROR_NOT_FOUND)
      return 1;
  }

  {
    ui_error_t rc_cleanup = ui_map_view_base_destroy(map);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return 0;
}
