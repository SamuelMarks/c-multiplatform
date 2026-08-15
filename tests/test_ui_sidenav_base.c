/* clang-format off */
#include "ui_sidenav_base.h"
#include "ui_error.h"

struct ui_sidenav_base {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *drawer_node;
  struct ui_dom_node *main_node;
  struct ui_component *drawer_content;
  struct ui_component *main_content;
  enum ui_sidenav_mode mode;
  enum ui_sidenav_position position;
  int is_open;
  struct ui_overlay_director *director;
  struct ui_backdrop *backdrop_logic;
  struct ui_component *backdrop_component;
  struct ui_overlay *backdrop_overlay;
  ui_sidenav_on_close_t on_close;
  void *user_data;
};

#include "ui_event.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t mock_on_close_error(struct ui_sidenav_base *sidenav,
                                      void *user_data) {
  (void)sidenav;
  (void)user_data;
  return UI_ERROR_OUT_OF_MEMORY;
}
static ui_error_t mock_on_close(struct ui_sidenav_base *sidenav,
                                void *user_data) {
  int *called = (int *)user_data;
  (void)sidenav;
  *called = 1;
  return UI_ERROR_NONE;
}

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    ui_error_t err = (expr);                                                   \
    if (err != UI_ERROR_NONE) {                                                \
      printf("Failed at line %d: %d\n", __LINE__, err);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    ui_error_t err = (expr);                                                   \
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
  (void)ui_sidenav_base_destroy(NULL);

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

  /* Set open while in SIDE mode to hit branch */
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 0));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));

  /* Change position while open */
  ASSERT_SUCCESS(
      ui_sidenav_base_set_position(sidenav, UI_SIDENAV_POSITION_START));

  /* Change mode to OVER while open to test backdrop mount (without director, it
   * shouldn't crash) */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_OVER));

  /* Process event while OVER mode is active but no director/backdrop is mounted
   */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));

  /* Set director while open in OVER mode (should attempt to mount backdrop) */
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_overlay_director_create(root, &director);
  ASSERT_SUCCESS(ui_sidenav_base_set_overlay_director(sidenav, director));

  /* Change mode to PUSH, set director again to hit branch where is_open && mode
   * != OVER */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_PUSH));
  ASSERT_SUCCESS(ui_sidenav_base_set_overlay_director(sidenav, director));

  /* Change mode to PUSH while open to test backdrop unmount */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_PUSH));

  /* Change mode to OVER while open to remount */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_OVER));

  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 0));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));

  /* Test process_event branches */
  /* is_open == 0 */
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 0));
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));

  /* is_open == 1, mode != OVER */
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_PUSH));
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));

  /* is_open == 1, mode == OVER, backdrop == NULL */
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 0));
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_OVER));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));
  /* Wait, opening in OVER mode automatically mounts the backdrop IF the
   * director is set! */
  /* We must set director to NULL first so it doesn't mount. */
  ASSERT_SUCCESS(ui_sidenav_base_set_overlay_director(sidenav, NULL));
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));

  /* Restore director for the rest of tests */
  ASSERT_SUCCESS(ui_sidenav_base_set_overlay_director(sidenav, director));
  /* And remount backdrop by toggling open */
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 0));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));

  /* Send escape when open in OVER mode (should trigger on_close) */
  close_called = 0;
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));
  if (!close_called)
    return 1;

  /* Send enter to hit should_dismiss == 0 branch */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));

  /* Now disable on_close to test default behavior */
  ASSERT_SUCCESS(ui_sidenav_base_set_on_close(sidenav, NULL, NULL));
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ASSERT_SUCCESS(ui_sidenav_base_process_event(sidenav, &ev, 0.0));

  /* Should be closed now */
  ASSERT_SUCCESS(ui_sidenav_base_is_open(sidenav, &is_open));
  if (is_open)
    return 1;

  /* Close when already closed */
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 0));

  /* Trigger unmount_backdrop errors by manually unmounting */
  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_OVER));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));
  ui_overlay_director_unmount(sidenav->director, sidenav->backdrop_overlay);
  /* This should now fail because unmount_backdrop fails */
  ui_sidenav_base_set_open(sidenav, 0);

  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_OVER));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));
  ui_overlay_director_unmount(sidenav->director, sidenav->backdrop_overlay);
  ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_PUSH);

  ASSERT_SUCCESS(ui_sidenav_base_set_mode(sidenav, UI_SIDENAV_MODE_OVER));
  ASSERT_SUCCESS(ui_sidenav_base_set_open(sidenav, 1));
  ui_overlay_director_unmount(sidenav->director, sidenav->backdrop_overlay);
  /* test destroy path */
  ui_sidenav_base_destroy(sidenav);
  sidenav = NULL;

  /* Need a new sidenav for remaining tests */
  ui_sidenav_base_create(&sidenav);
  ui_sidenav_base_set_overlay_director(sidenav, director);

  /* Test mount_backdrop error on open by mocking director error?
     Actually mount_backdrop fails if we can't allocate inside
     ui_component_create or something. We can trigger it with
     g_malloc_fail_countdown */

  /* Test event processing failures */
  ui_sidenav_base_set_open(sidenav, 1);
  ui_sidenav_base_set_on_close(sidenav, mock_on_close_error, NULL);
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ui_sidenav_base_process_event(sidenav, &ev, 0.0);

  /* Fallback close failure */
  ui_sidenav_base_set_on_close(sidenav, NULL, NULL);
  /* Make unmount fail to cause set_open(0) to fail */
  ui_overlay_director_unmount(sidenav->director, sidenav->backdrop_overlay);
  ui_sidenav_base_process_event(sidenav, &ev, 0.0);
  ui_sidenav_base_set_open(sidenav, 0); /* will fail, clean up manually */
  sidenav->is_open = 0;

  /* Test mount_backdrop failure in set_overlay_director */
  ui_sidenav_base_set_open(sidenav, 1);
  sidenav->director = NULL; /* remove director so it's ready to be set */
  if (sidenav->backdrop_component) {
    ui_component_destroy(sidenav->backdrop_component);
    sidenav->backdrop_component = NULL;
  }
  g_malloc_fail_countdown = 0;
  ui_sidenav_base_set_overlay_director(sidenav, director);
  g_malloc_fail_countdown = -1;
  sidenav->director = director;

  /* Test mount_backdrop failure in update_dom_state */
  ui_sidenav_base_set_open(sidenav, 0);
  if (sidenav->backdrop_component) {
    ui_component_destroy(sidenav->backdrop_component);
    sidenav->backdrop_component = NULL;
  }
  g_malloc_fail_countdown = 0;
  ui_sidenav_base_set_open(sidenav, 1);
  g_malloc_fail_countdown = -1;

  /* Trigger update_dom_state mount_backdrop failure */
  ui_sidenav_base_set_open(sidenav, 1);
  if (sidenav->backdrop_component) {
    ui_overlay_director_unmount(sidenav->director, sidenav->backdrop_overlay);
    ui_component_destroy(sidenav->backdrop_component);
    sidenav->backdrop_component = NULL;
    sidenav->backdrop_overlay = NULL;
  }
  g_malloc_fail_countdown = 0;
  ui_sidenav_base_set_position(sidenav, UI_SIDENAV_POSITION_END);
  g_malloc_fail_countdown = -1;

  printf("Testing OOM on create...\n");
  {
    int i;
    for (i = 0; i < 200; i++) {
      g_malloc_fail_countdown = i;
      if (ui_sidenav_base_create(&sidenav) == UI_ERROR_NONE) {
        (void)ui_sidenav_base_destroy(sidenav);
        break;
      }
    }
    g_malloc_fail_countdown = -1;
  }

  {
    int i;
    for (i = 0; i < 200; i++) {
      ui_sidenav_base_create(&sidenav);
      ui_sidenav_base_set_overlay_director(sidenav, director);
      g_malloc_fail_countdown = i;
      if (ui_sidenav_base_set_open(sidenav, 1) == UI_ERROR_NONE) {
        (void)ui_sidenav_base_destroy(sidenav);
        break;
      }
      g_malloc_fail_countdown = -1;
      (void)ui_sidenav_base_destroy(sidenav);
    }
  }
  g_malloc_fail_countdown = -1;

  (void)ui_component_destroy(content_comp1);
  (void)ui_component_destroy(content_comp2);
  (void)ui_overlay_director_destroy(director);
  (void)ui_dom_node_destroy(root);

  printf("All tests passed.\n");
  return 0;
}
