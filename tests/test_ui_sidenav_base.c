/* clang-format off */
#include "ui_sidenav_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include <stdio.h>
/* clang-format on */

static enum ui_error mock_on_close(struct ui_sidenav_base *sidenav,
                                   void *user_data) {
  int *called = (int *)user_data;
  (void)sidenav;
  *called = 1;
  return UI_ERROR_NONE;
}

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    enum ui_error err = (expr);                                                \
    if (err != UI_ERROR_NONE) {                                                \
      printf("Failed at line %d: %d\n", __LINE__, err);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    enum ui_error err = (expr);                                                \
    if (err != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             err);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  struct ui_sidenav_base *sidenav = NULL;
  struct ui_component *comp = NULL;
  struct ui_component *content_comp1 = NULL;
  struct ui_component *content_comp2 = NULL;
  struct ui_event ev;
  int is_open;
  int close_called = 0;
  struct ui_signal *signal = NULL;

  /* Null checks */
  ASSERT_EQ(ui_sidenav_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);
  ui_sidenav_base_destroy(NULL);

  ASSERT_EQ(ui_sidenav_base_set_mode(NULL, UI_SIDENAV_MODE_OVER),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_set_position(NULL, UI_SIDENAV_POSITION_START),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_set_drawer_content(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_set_main_content(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_set_open(NULL, 1), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_is_open(NULL, &is_open), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_set_overlay_director(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_set_on_close(NULL, mock_on_close, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_process_event(NULL, &ev, 0.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_get_component(NULL, &comp),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_bind_active_index(NULL, signal),
            UI_ERROR_INVALID_ARGUMENT);

  /* Create components for content testing */
  ASSERT_SUCCESS(ui_component_create(&content_comp1));
  ASSERT_SUCCESS(ui_component_create(&content_comp2));

  ASSERT_SUCCESS(ui_sidenav_base_create(&sidenav));
  ASSERT_EQ(ui_sidenav_base_is_open(sidenav, NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_process_event(sidenav, NULL, 0.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sidenav_base_get_component(sidenav, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_sidenav_base_set_drawer_content(sidenav, NULL));
  ASSERT_SUCCESS(ui_sidenav_base_set_main_content(sidenav, NULL));
  ASSERT_SUCCESS(ui_sidenav_base_set_drawer_content(sidenav, content_comp1));
  ASSERT_SUCCESS(ui_sidenav_base_set_main_content(sidenav, content_comp2));

  ASSERT_SUCCESS(
      ui_sidenav_base_set_on_close(sidenav, mock_on_close, &close_called));
  ASSERT_SUCCESS(
      ui_sidenav_base_bind_active_index(sidenav, (struct ui_signal *)1));

  ASSERT_SUCCESS(ui_sidenav_base_get_component(sidenav, &comp));
  if (!comp)
    return 1;

  /* Check initial state */
  ASSERT_SUCCESS(ui_sidenav_base_is_open(sidenav, &is_open));
  if (is_open)
    return 1;

  /* Event process when closed (should do nothing) */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));

  /* Test mode & position */
  ASSERT_SUCCESS(
      ui_sidenav_base_set_position(sidenav, UI_SIDENAV_POSITION_END));
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_PUSH));

  /* Open in PUSH mode */
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(
      sidenav, 1)); /* Repeated should return NONE without effect */

  ASSERT_SUCCESS(ui_sidenav_base_is_open(sidenav, &is_open));
  if (!is_open)
    return 1;

  /* Change mode to SIDE while open */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_SIDE));

  /* Change position while open */
  ASSERT_SUCCESS(
      ui_sidenav_base_set_position(sidenav, UI_SIDENAV_POSITION_START));

  /* Change mode to OVER while open to test backdrop mount (without director, it
   * shouldn't crash) */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_OVER));

  /* Set director while open in OVER mode (should attempt to mount backdrop) */
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_overlay_director_create(root, &director);
  ASSERT_SUCCESS(ui_sidenav_base_set_overlay_director(sidenav, director));

  /* Change mode to PUSH while open to test backdrop unmount */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_PUSH));

  /* Change mode to OVER while open to remount */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_OVER));

  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 0));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));

  /* Send escape when open in OVER mode (should trigger on_close) */
  close_called = 0;
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));
  if (!close_called)
    return 1;

  /* Now disable on_close to test default behavior */
  ASSERT_SUCCESS(ui_sidenav_base_set_on_close(sidenav, NULL, NULL));
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));

  /* Should be closed now */
  ASSERT_SUCCESS(ui_sidenav_base_is_open(sidenav, &is_open));
  if (is_open)
    return 1;

  /* Close when already closed */
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 0));

  ui_sidenav_base_destroy(sidenav);
  ui_component_destroy(content_comp1);
  ui_component_destroy(content_comp2);
  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root);

  printf("All tests passed.\n");
  return 0;
}
