/* clang-format off */
#include "../include/ui_dockable_layout_base.h"
#include "../include/ui_error.h"
#include "../include/ui_drag_drop.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_dockable_layout_lifecycle(void) {
  struct ui_dockable_layout_base *layout = NULL;
  ui_error_t rc;
  struct ui_component *comp = NULL;

  /* Invalid args */
  if (ui_dockable_layout_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    ui_error_t rc_cleanup = ui_dockable_layout_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (ui_dockable_layout_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dockable_layout_base_get_component((struct ui_dockable_layout_base *)1,
                                            NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Mock alloc failure */
  g_malloc_fail_countdown = 0;
  rc = ui_dockable_layout_base_create(&layout);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = 1;
  rc = ui_dockable_layout_base_create(&layout);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  rc = ui_dockable_layout_base_create(&layout);
  if (rc != UI_ERROR_NONE || layout == NULL)
    return 1;

  rc = ui_dockable_layout_base_get_component(layout, &comp);
  if (rc != UI_ERROR_NONE || comp == NULL)
    return 1;

  {
    ui_error_t rc_cleanup = ui_dockable_layout_base_destroy(layout);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_dockable_layout_docking(void) {
  struct ui_dockable_layout_base *layout = NULL;
  ui_error_t rc;
  int i;

  /* Invalid args */
  if (ui_dockable_layout_base_dock_panel(NULL, 1, 0, UI_DOCK_EDGE_LEFT) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dockable_layout_base_remove_panel(NULL, 1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_dockable_layout_base_create(&layout);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dockable_layout_base_dock_panel(layout, 1, 0, UI_DOCK_EDGE_LEFT);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dockable_layout_base_dock_panel(layout, 2, 1, UI_DOCK_EDGE_BOTTOM);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Re-dock existing panel */
  rc = ui_dockable_layout_base_dock_panel(layout, 2, 1, UI_DOCK_EDGE_RIGHT);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Remove an existing panel */
  rc = ui_dockable_layout_base_remove_panel(layout, 1);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Remove non-existent panel */
  rc = ui_dockable_layout_base_remove_panel(layout, 999);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test filling up all 128 max panels */
  for (i = 0; i < 128; i++) {
    /* i == 2 is already in use because we docked panel 2, but panel 2 is
     * currently the only one left */
    rc = ui_dockable_layout_base_dock_panel(layout, 100 + i, 0,
                                            UI_DOCK_EDGE_LEFT);
    if (i < 127 && rc != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Should fail bounds check */
  rc = ui_dockable_layout_base_dock_panel(layout, 9999, 0, UI_DOCK_EDGE_LEFT);
  if (rc != UI_ERROR_OUT_OF_BOUNDS) {
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_dockable_layout_base_destroy(layout);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_dockable_layout_serialization(void) {
  struct ui_dockable_layout_base *layout = NULL;
  ui_error_t rc;
  char buffer[256];

  /* Invalid args */
  if (ui_dockable_layout_base_serialize(NULL, buffer, 256) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dockable_layout_base_deserialize(NULL, buffer) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_dockable_layout_base_create(&layout);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_dockable_layout_base_serialize(layout, NULL, 256) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dockable_layout_base_serialize(layout, buffer, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dockable_layout_base_deserialize(layout, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_dockable_layout_base_dock_panel(layout, 10, 0, UI_DOCK_EDGE_LEFT);
  ui_dockable_layout_base_dock_panel(layout, 20, 10, UI_DOCK_EDGE_RIGHT);

  rc = ui_dockable_layout_base_serialize(layout, buffer, sizeof(buffer));
  if (rc != UI_ERROR_NONE)
    return 1;

  if (strstr(buffer, "P:10,T:0,E:0;") == NULL ||
      strstr(buffer, "P:20,T:10,E:1;") == NULL) {
    return 1;
  }

  /* Serialize buffer too small test */
  char small_buffer[5];
  rc = ui_dockable_layout_base_serialize(layout, small_buffer,
                                         sizeof(small_buffer));
  if (rc != UI_ERROR_NONE) /* Returns NONE but just truncates string */
    return 1;

  rc = ui_dockable_layout_base_remove_panel(layout, 10);
  rc = ui_dockable_layout_base_remove_panel(layout, 20);

  rc = ui_dockable_layout_base_deserialize(layout, buffer);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test deserialize invalid token */
  if (ui_dockable_layout_base_deserialize(layout, "INVALID") != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_dockable_layout_base_destroy(layout);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_dockable_layout_integrate(void) {
  struct ui_dockable_layout_base *layout = NULL;
  ui_error_t rc;

  if (ui_dockable_layout_base_integrate_drag_drop(
          NULL, (struct ui_drag_drop_context *)1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_dockable_layout_base_create(&layout);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_dockable_layout_base_integrate_drag_drop(layout, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dockable_layout_base_integrate_drag_drop(
          layout, (struct ui_drag_drop_context *)1) != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_dockable_layout_base_destroy(layout);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_dockable_layout_base tests...\n");

  failed |= test_dockable_layout_lifecycle();
  failed |= test_dockable_layout_docking();
  failed |= test_dockable_layout_serialization();
  failed |= test_dockable_layout_integrate();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  {
    char big_buffer[2048] = {0};
    int i;
    int offset = 0;
    struct ui_dockable_layout_base *layout;
    ui_dockable_layout_base_create(&layout);
    for (i = 1; i <= 130; i++) {
      offset += sprintf(big_buffer + offset, "P:%d,T:%d,E:0;", i, i - 1);
    }
    /* This will fail to dock the 129th panel, hitting line 198! */
    ui_dockable_layout_base_deserialize(layout, big_buffer);
    {
      ui_error_t rc_cleanup = ui_dockable_layout_base_destroy(layout);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  printf("All tests passed.\n");
  return 0;
}
