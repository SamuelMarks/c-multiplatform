/* clang-format off */
#include "ui_popover_base.h"
#include "ui_overlay_director.h"
#include "ui_focus_manager.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include "ui_geometry_anchor.h"
#include "ui_event.h"
#include <stdio.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

extern int g_malloc_fail_countdown;
#ifdef UI_TEST_MOCK_ALLOC
extern int g_popover_mock_fail;
extern int g_popover_backdrop_destroy_mock_fail;
extern int g_popover_component_destroy_mock_fail;
extern int g_popover_dom_node_destroy_mock_fail;
extern int g_popover_remove_child_mock_fail;
extern int g_popover_unmount_mock_fail;
extern void
ui_popover_base_test_clear_shadow_root(struct ui_popover_base *popover);
#endif

static int test_popover_lifecycle(void) {
  struct ui_popover_base *popover;
  ui_error_t rc;
  int is_open = 0;

  rc = ui_popover_base_create(&popover);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_popover_base_is_open(popover, &is_open);
  if (is_open)
    return 1;

  {
    ui_error_t rc_cleanup = ui_popover_base_destroy(popover);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  return 0;
}

static int test_popover_open_close(void) {
  struct ui_popover_base *popover;
  struct ui_overlay_director *director;
  struct ui_focus_manager *focus_mgr;
  struct ui_dom_node *root_node;
  struct ui_dom_node *content_node;
  struct ui_layout_node trigger;
  struct ui_anchor_config anchor;
  ui_error_t rc;
  int is_open = 0;

  rc = ui_popover_base_create(&popover);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);
  ui_focus_manager_create(&focus_mgr);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);

  trigger.x = 100.0f;
  trigger.y = 100.0f;
  trigger.width = 50.0f;
  trigger.height = 50.0f;

  anchor.target_x = UI_ANCHOR_EDGE_START;
  anchor.target_y = UI_ANCHOR_EDGE_END;
  anchor.overlay_x = UI_ANCHOR_EDGE_START;
  anchor.overlay_y = UI_ANCHOR_EDGE_START;
  anchor.offset_x = 0.0f;
  anchor.offset_y = 0.0f;

  rc = ui_popover_base_open(popover, content_node, director, focus_mgr,
                            &trigger, &anchor, 800.0f, 600.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Double open */
  rc = ui_popover_base_open(popover, content_node, director, focus_mgr,
                            &trigger, &anchor, 800.0f, 600.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_popover_base_is_open(popover, &is_open);
  if (!is_open)
    return 1;

  /* Remove content to test empty root on close */
  if (content_node->parent)
    ui_dom_node_remove_child(content_node->parent, content_node);

  rc = ui_popover_base_close(popover);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Double close */
  rc = ui_popover_base_close(popover);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_popover_base_is_open(popover, &is_open);
  if (is_open)
    return 1;

  {
    ui_error_t rc_cleanup = ui_popover_base_destroy(popover);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_focus_manager_destroy(focus_mgr);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(director);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(content_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  /* Note: content_node is conceptually owned by the user, but since the popover
     mounted it, destroying the popover tree during close cleans it up. However,
     we removed it above, so we must free it. */

  return 0;
}

static int test_popover_click_outside(void) {
  struct ui_popover_base *popover;
  struct ui_overlay_director *director;
  struct ui_dom_node *root_node;
  struct ui_dom_node *content_node;
  struct ui_layout_node trigger;
  struct ui_anchor_config anchor;
  struct ui_event ev;
  ui_error_t rc;
  int is_open = 0;

  rc = ui_popover_base_create(&popover);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);

  trigger.x = 100.0f;
  trigger.y = 100.0f;
  trigger.width = 50.0f;
  trigger.height = 50.0f;

  anchor.target_x = UI_ANCHOR_EDGE_START;
  anchor.target_y = UI_ANCHOR_EDGE_END;
  anchor.overlay_x = UI_ANCHOR_EDGE_START;
  anchor.overlay_y = UI_ANCHOR_EDGE_START;
  anchor.offset_x = 0.0f;
  anchor.offset_y = 0.0f;

  /* Process event when closed */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_popover_base_process_event(popover, &ev);

  /* Open the popover */
  ui_popover_base_open(popover, content_node, director, NULL, &trigger, &anchor,
                       800.0f, 600.0f);
  ui_popover_base_is_open(popover, &is_open);
  if (!is_open)
    return 1;

  /* Simulate click inside the bounds (which are 100, 150 -> 300, 300 based on
   * anchor/mock sizes) */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0; /* Left click */
  ev.event_data.mouse.x = 150;
  ev.event_data.mouse.y = 200;
  ui_popover_base_process_event(popover, &ev);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_popover_base_process_event(popover, &ev);
  ui_popover_base_is_open(popover, &is_open);
  if (!is_open)
    return 1;

  /* Simulate click outside the bounds */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 10;
  ev.event_data.mouse.y = 10;
  ui_popover_base_process_event(popover, &ev);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_popover_base_process_event(popover, &ev);
  ui_popover_base_is_open(popover, &is_open);
  if (is_open)
    return 1;

  /* Close via destroy while open */
  ui_popover_base_open(popover, content_node, director, NULL, &trigger, &anchor,
                       800.0f, 600.0f);
  {
    ui_error_t rc_cleanup = ui_popover_base_destroy(popover);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(director);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(content_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  return 0;
}

static int test_popover_nulls(void) {
  struct ui_popover_base *popover = NULL;
  struct ui_dom_node *content = (struct ui_dom_node *)0x123;
  struct ui_overlay_director *director = (struct ui_overlay_director *)0x123;
  struct ui_focus_manager *focus_mgr = (struct ui_focus_manager *)0x123;
  struct ui_layout_node trigger;
  struct ui_anchor_config anchor;
  struct ui_event ev;
  int is_open;
  struct ui_signal *sig = (struct ui_signal *)0x123;
  struct ui_computed *comp = NULL;

  if (ui_popover_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    ui_error_t rc_cleanup = ui_popover_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  if (ui_popover_base_open(NULL, content, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_popover_base_create(&popover);
  if (ui_popover_base_open(popover, NULL, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_popover_base_open(popover, content, NULL, focus_mgr, &trigger, &anchor,
                           800, 600) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_popover_base_open(popover, content, director, focus_mgr, NULL, &anchor,
                           800, 600) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_popover_base_open(popover, content, director, focus_mgr, &trigger,
                           NULL, 800, 600) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_popover_base_process_event(NULL, &ev) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_popover_base_process_event(popover, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_popover_base_close(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_popover_base_is_open(NULL, &is_open) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_popover_base_is_open(popover, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_popover_base_bind_open(NULL, sig) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_popover_base_bind_open(popover, sig) != UI_ERROR_NONE)
    return 1;

  if (ui_popover_base_get_animating_signal(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_popover_base_get_animating_signal(popover, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_popover_base_get_animating_signal(popover, &comp) != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_popover_base_destroy(popover);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  return 0;
}

static int test_popover_oom_and_errors(void) {
  struct ui_popover_base *popover = NULL;
  struct ui_overlay_director *director;
  struct ui_dom_node *root_node;
  struct ui_dom_node *content_node;
  struct ui_dom_node *dummy_parent = NULL;
  struct ui_layout_node trigger;
  struct ui_anchor_config anchor;
  struct ui_focus_manager *focus_mgr;
  int i;

  g_malloc_fail_countdown = 0;
  if (ui_popover_base_create(&popover) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    if (ui_popover_base_create(&popover) == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_popover_base_destroy(popover);
        if (rc_cleanup != UI_ERROR_NONE) {
          return 1;
        }
      }
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);
  ui_focus_manager_create(&focus_mgr);

  trigger.x = 100.0f;
  trigger.y = 100.0f;
  trigger.width = 50.0f;
  trigger.height = 50.0f;

  anchor.target_x = UI_ANCHOR_EDGE_START;
  anchor.target_y = UI_ANCHOR_EDGE_END;
  anchor.overlay_x = UI_ANCHOR_EDGE_START;
  anchor.overlay_y = UI_ANCHOR_EDGE_START;
  anchor.offset_x = 0.0f;
  anchor.offset_y = 0.0f;

  ui_popover_base_create(&popover);

  /* Fail dom node creation via malloc */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    ui_error_t rc =
        ui_popover_base_open(popover, content_node, director, focus_mgr,
                             &trigger, &anchor, 800, 600);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      ui_popover_base_close(popover);
      break;
    }
  }

  /* Fail ui_dom_node_append_child by giving content_node a parent */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dummy_parent);
  ui_dom_node_append_child(dummy_parent, content_node);

  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) == UI_ERROR_NONE)
    return 1;

  ui_dom_node_destroy(dummy_parent); /* also destroys content_node */

  {
    ui_error_t rc_cleanup = ui_popover_base_destroy(popover);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_focus_manager_destroy(focus_mgr);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(director);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  return 0;
}

#ifdef UI_TEST_MOCK_ALLOC
static int test_popover_mock_failures(void) {
  struct ui_popover_base *popover = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_dom_node *content_node = NULL;
  struct ui_dom_node *dummy_parent = NULL;
  struct ui_layout_node trigger;
  struct ui_anchor_config anchor;
  struct ui_focus_manager *focus_mgr = NULL;
  ui_error_t rc;

  trigger.x = 100.0f;
  trigger.y = 100.0f;
  trigger.width = 50.0f;
  trigger.height = 50.0f;

  anchor.target_x = UI_ANCHOR_EDGE_START;
  anchor.target_y = UI_ANCHOR_EDGE_END;
  anchor.overlay_x = UI_ANCHOR_EDGE_START;
  anchor.overlay_y = UI_ANCHOR_EDGE_START;
  anchor.offset_x = 0.0f;
  anchor.offset_y = 0.0f;

  /* 1. backdrop_create fails, component_destroy succeeds */
  g_popover_mock_fail = 1;
  if (ui_popover_base_create(&popover) != UI_ERROR_UNKNOWN)
    return 1;

  /* 2. backdrop_create fails, component_destroy fails */
  g_popover_mock_fail = 1;
  g_popover_component_destroy_mock_fail = 1;
  if (ui_popover_base_create(&popover) != UI_ERROR_UNKNOWN)
    return 1;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);
  ui_focus_manager_create(&focus_mgr);

  /* 3. anchor_compute fails */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 2;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 4. set_attribute role fails, destroy succeeds */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 3;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 5. set_attribute role fails, destroy fails */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 3;
  g_popover_dom_node_destroy_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 6. set_attribute aria-modal fails, destroy succeeds */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 4;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 7. set_attribute aria-modal fails, destroy fails */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 4;
  g_popover_dom_node_destroy_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 8. set_attribute style fails, destroy succeeds */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 5;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 9. set_attribute style fails, destroy fails */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 5;
  g_popover_dom_node_destroy_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 10. append_child fails, destroy fails */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dummy_parent);
  ui_dom_node_append_child(dummy_parent, content_node);
  ui_popover_base_create(&popover);
  g_popover_dom_node_destroy_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);
  ui_dom_node_destroy(dummy_parent);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);

  /* 11. overlay mount fails: remove_child succeeds, destroy succeeds */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 6;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 12. overlay mount fails: remove_child fails */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 6;
  g_popover_remove_child_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 13. overlay mount fails: remove_child succeeds, destroy fails */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 6;
  g_popover_dom_node_destroy_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 14. push_trap fails: unmount succeeds, remove_child succeeds, destroy
   * succeeds */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 7;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 15. push_trap fails: unmount fails */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 7;
  g_popover_unmount_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 16. push_trap fails: unmount succeeds, remove_child fails */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 7;
  g_popover_remove_child_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 17. push_trap fails: unmount succeeds, remove_child succeeds, destroy fails
   */
  ui_popover_base_create(&popover);
  g_popover_mock_fail = 7;
  g_popover_dom_node_destroy_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 18. backdrop_process_event fails */
  ui_popover_base_create(&popover);
  ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                       &anchor, 800, 600);
  g_popover_mock_fail = 8;
  {
    struct ui_event ev;
    ev.type = UI_EVENT_MOUSE_DOWN;
    if (ui_popover_base_process_event(popover, &ev) != UI_ERROR_UNKNOWN)
      return 1;
  }
  ui_popover_base_close(popover);
  ui_popover_base_destroy(popover);

  /* 19. Open while open, but close fails */
  ui_popover_base_create(&popover);
  ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                       &anchor, 800, 600);
  g_popover_unmount_mock_fail = 1;
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_close(popover);
  ui_popover_base_destroy(popover);

  /* 20. Close: pop_trap fails, unmount fails, remove_child fails, destroy fails
   * (rc != NONE, rc == NONE is false for all) */
  ui_popover_base_create(&popover);
  ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                       &anchor, 800, 600);
  g_popover_mock_fail = 9;              /* pop_trap fails -> rc is UNKNOWN */
  g_popover_unmount_mock_fail = 1;      /* unmount fails -> rc != NONE */
  g_popover_remove_child_mock_fail = 1; /* remove_child fails -> rc != NONE */
  g_popover_dom_node_destroy_mock_fail =
      1; /* dom_node_destroy fails -> rc != NONE */
  if (ui_popover_base_close(popover) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 21. Close: remove_child fails while rc == NONE -> rc = rc_cleanup */
  ui_popover_base_create(&popover);
  ui_popover_base_open(popover, content_node, director, NULL, &trigger, &anchor,
                       800, 600);
  g_popover_remove_child_mock_fail = 1;
  if (ui_popover_base_close(popover) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 22. Close: dom_node_destroy fails while rc == NONE -> rc = rc_cleanup */
  ui_popover_base_create(&popover);
  ui_popover_base_open(popover, content_node, director, NULL, &trigger, &anchor,
                       800, 600);
  g_popover_dom_node_destroy_mock_fail = 1;
  if (ui_popover_base_close(popover) != UI_ERROR_UNKNOWN)
    return 1;
  ui_popover_base_destroy(popover);

  /* 23. Close when shadow_root is NULL (root == NULL branch) */

  ui_popover_base_create(&popover);
  ui_popover_base_open(popover, content_node, director, NULL, &trigger, &anchor,
                       800, 600);
  /* Unmount manually to clear shadow_root branch */
  ui_popover_base_test_clear_shadow_root(popover);
  if (ui_popover_base_close(popover) != UI_ERROR_NONE)
    return 1;
  ui_popover_base_destroy(popover);

  /* 24. Destroy while open: close fails, backdrop_destroy fails,
   * component_destroy fails */
  ui_dom_node_destroy(content_node);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);
  ui_popover_base_create(&popover);
  if (ui_popover_base_open(popover, content_node, director, focus_mgr, &trigger,
                           &anchor, 800, 600) != UI_ERROR_NONE)
    return 1;
  g_popover_unmount_mock_fail = 1; /* close fails -> rc becomes UNKNOWN */
  g_popover_backdrop_destroy_mock_fail =
      1; /* backdrop_destroy fails -> rc != NONE */
  g_popover_component_destroy_mock_fail =
      1; /* component_destroy fails -> rc != NONE */
  if (ui_popover_base_destroy(popover) != UI_ERROR_UNKNOWN)
    return 1;

  /* 25. Destroy: backdrop_destroy fails while closed -> rc becomes UNKNOWN */
  ui_popover_base_create(&popover);
  g_popover_backdrop_destroy_mock_fail = 1;
  if (ui_popover_base_destroy(popover) != UI_ERROR_UNKNOWN)
    return 1;

  /* 26. Destroy: component_destroy fails while closed -> rc becomes UNKNOWN */
  ui_popover_base_create(&popover);
  g_popover_component_destroy_mock_fail = 1;
  if (ui_popover_base_destroy(popover) != UI_ERROR_UNKNOWN)
    return 1;

  ui_focus_manager_destroy(focus_mgr);
  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root_node);
  ui_dom_node_destroy(content_node);

  return 0;
}
#endif
int main(void) {
  int result = 0;
  printf("Running ui_popover_base tests...\n");

  if (test_popover_lifecycle() != 0) {
    printf("test_popover_lifecycle FAILED\n");
    result = 1;
  }
  if (test_popover_open_close() != 0) {
    printf("test_popover_open_close FAILED\n");
    result = 1;
  }
  if (test_popover_click_outside() != 0) {
    printf("test_popover_click_outside FAILED\n");
    result = 1;
  }
  if (test_popover_nulls() != 0) {
    printf("test_popover_nulls FAILED\n");
    result = 1;
  }
  if (test_popover_oom_and_errors() != 0) {
    printf("test_popover_oom_and_errors FAILED\n");
    result = 1;
  }
#ifdef UI_TEST_MOCK_ALLOC
  if (test_popover_mock_failures() != 0) {
    printf("test_popover_mock_failures FAILED\n");
    result = 1;
  }
#endif

  if (result == 0) {
    printf("All popover base tests PASSED\n");
  } else {
    printf("Some popover base tests FAILED\n");
  }

  return result;
}
