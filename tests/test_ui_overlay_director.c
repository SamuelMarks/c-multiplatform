/* clang-format off */
#include "ui_overlay_director.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
#ifdef UI_TEST_MOCK_ALLOC
extern int g_overlay_mock_fail;
extern int g_overlay_destroy_mock_fail;
extern int g_overlay_remove_child_mock_fail;
#endif

static int s_tests_passed = 0;
static int s_tests_failed = 0;

#define ASSERT_TRUE(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
      s_tests_failed++;                                                        \
    } else {                                                                   \
      s_tests_passed++;                                                        \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      fprintf(stderr, "FAIL: %s:%d: expected %d, got %d\n", __FILE__,          \
              __LINE__, (int)(expected), (int)(actual));                       \
      s_tests_failed++;                                                        \
    } else {                                                                   \
      s_tests_passed++;                                                        \
    }                                                                          \
  } while (0)

static ui_error_t test_invalid_args(void) {
  struct ui_dom_node *root = NULL;
  struct ui_overlay_director *dir = NULL;
  struct ui_component *comp = NULL;
  struct ui_overlay *overlay = NULL;
  struct ui_overlay_director *dir2 = NULL;
  struct ui_dom_node *root2 = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_overlay_director_create(NULL, &dir));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_overlay_director_create(root, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_overlay_director_destroy(NULL));

  rc = ui_overlay_director_create(root, &dir);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_overlay_director_mount_component(NULL, comp, 1, &overlay));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_overlay_director_mount_component(dir, NULL, 1, &overlay));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_overlay_director_mount_component(dir, comp, 1, NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_overlay_director_unmount(NULL, overlay));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_overlay_director_unmount(dir, NULL));

  rc = ui_overlay_director_mount_component(dir, comp, 1, &overlay);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root2, &dir2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_NOT_FOUND, ui_overlay_director_unmount(dir2, overlay));

  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(dir2);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root2);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(dir);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(comp);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t test_overlay_director_lifecycle(void) {
  struct ui_dom_node *root = NULL;
  struct ui_overlay_director *dir = NULL;
  struct ui_component *comp1 = NULL;
  struct ui_component *comp2 = NULL;
  struct ui_component *comp3 = NULL;
  struct ui_overlay *overlay1 = NULL;
  struct ui_overlay *overlay2 = NULL;
  struct ui_overlay *overlay3 = NULL;
  ui_error_t err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, err);

  err = ui_overlay_director_create(root, &dir);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_TRUE(dir != NULL);

  err = ui_component_create(&comp1);
  ASSERT_EQ(UI_ERROR_NONE, err);

  err = ui_component_create(&comp2);
  ASSERT_EQ(UI_ERROR_NONE, err);

  err = ui_component_create(&comp3);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Mount first overlay */
  err = ui_overlay_director_mount_component(dir, comp1, 100, &overlay1);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_TRUE(overlay1 != NULL);
  ASSERT_TRUE(root->first_child != NULL);

  /* Mount second overlay */
  err = ui_overlay_director_mount_component(dir, comp2, 200, &overlay2);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_TRUE(overlay2 != NULL);

  /* Mount third overlay */
  err = ui_overlay_director_mount_component(dir, comp3, 300, &overlay3);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_TRUE(overlay3 != NULL);

  /* Unmount first (head of list is overlay3, overlay1 is tail -> prev != NULL)
   */
  err = ui_overlay_director_unmount(dir, overlay1);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Unmount head (overlay3 is head of list -> prev == NULL) */
  err = ui_overlay_director_unmount(dir, overlay3);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* overlay2 is still mounted; test destroying director with active overlay */
  err = ui_overlay_director_destroy(dir);
  ASSERT_EQ(UI_ERROR_NONE, err);

  {
    ui_error_t rc_cleanup = ui_component_destroy(comp1);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(comp2);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(comp3);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }

  /* Root node should have 0 children now */
  ASSERT_TRUE(root->first_child == NULL);
  ASSERT_TRUE(root->last_child == NULL);

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t test_unmount_detached_parent(void) {
  struct ui_dom_node *root = NULL;
  struct ui_overlay_director *dir = NULL;
  struct ui_component *comp = NULL;
  struct ui_overlay *overlay = NULL;
  ui_error_t err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, err);

  err = ui_overlay_director_create(root, &dir);
  ASSERT_EQ(UI_ERROR_NONE, err);

  err = ui_component_create(&comp);
  ASSERT_EQ(UI_ERROR_NONE, err);

  err = ui_overlay_director_mount_component(dir, comp, 50, &overlay);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Manually detach the wrapper node from root before calling unmount */
  err = ui_dom_node_remove_child(root, root->first_child);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Unmount with p == NULL */
  err = ui_overlay_director_unmount(dir, overlay);
  ASSERT_EQ(UI_ERROR_NONE, err);

  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(dir);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(comp);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t test_oom(void) {
  struct ui_dom_node *root = NULL;
  struct ui_overlay_director *dir = NULL;
  struct ui_component *comp = NULL;
  struct ui_overlay *overlay = NULL;
  ui_error_t err;
  int i;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, err);
  err = ui_component_create(&comp);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Creation OOM */
  g_malloc_fail_countdown = 0;
  err = ui_overlay_director_create(root, &dir);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, err);

  err = ui_overlay_director_create(root, &dir);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Mount OOM (no children) */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    err = ui_overlay_director_mount_component(dir, comp, 1, &overlay);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (err == UI_ERROR_NONE) {
      ui_overlay_director_unmount(dir, overlay);
      break;
    }
  }

  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(dir);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(comp);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
static ui_error_t test_mock_failures(void) {
  struct ui_dom_node *root = NULL;
  struct ui_overlay_director *dir = NULL;
  struct ui_component *comp1 = NULL;
  struct ui_component *comp2 = NULL;
  struct ui_overlay *overlay1 = NULL;
  struct ui_overlay *overlay2 = NULL;
  ui_error_t err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, err);
  err = ui_component_create(&comp1);
  ASSERT_EQ(UI_ERROR_NONE, err);
  err = ui_component_create(&comp2);
  ASSERT_EQ(UI_ERROR_NONE, err);
  err = ui_overlay_director_create(root, &dir);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* 1. set_tag_name failure, destroy succeeds */
  g_overlay_mock_fail = 2;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 2. set_tag_name failure, destroy fails */
  g_overlay_mock_fail = 2;
  g_overlay_destroy_mock_fail = 1;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 3. set_attribute style failure, destroy succeeds */
  g_overlay_mock_fail = 3;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 4. set_attribute style failure, destroy fails */
  g_overlay_mock_fail = 3;
  g_overlay_destroy_mock_fail = 1;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 5. set_attribute data-overlay failure, destroy succeeds */
  g_overlay_mock_fail = 4;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 6. set_attribute data-overlay failure, destroy fails */
  g_overlay_mock_fail = 4;
  g_overlay_destroy_mock_fail = 1;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 7. append_child failure, destroy succeeds */
  g_overlay_mock_fail = 5;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 8. append_child failure, destroy fails */
  g_overlay_mock_fail = 5;
  g_overlay_destroy_mock_fail = 1;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 9. component_mount failure, remove_child succeeds, destroy succeeds */
  g_overlay_mock_fail = 6;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 10. component_mount failure, remove_child fails */
  g_overlay_mock_fail = 6;
  g_overlay_remove_child_mock_fail = 1;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 11. component_mount failure, remove_child succeeds, destroy fails */
  g_overlay_mock_fail = 6;
  g_overlay_destroy_mock_fail = 1;
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 12. Mount succeeds, then unmount remove_child fails */
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_NONE, err);
  g_overlay_remove_child_mock_fail = 1;
  err = ui_overlay_director_unmount(dir, overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 13. Mount succeeds, then unmount destroy fails */
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_NONE, err);
  g_overlay_destroy_mock_fail = 1;
  err = ui_overlay_director_unmount(dir, overlay1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  /* 14. Destroy with two overlays where unmount fails for both */
  err = ui_overlay_director_mount_component(dir, comp1, 1, &overlay1);
  ASSERT_EQ(UI_ERROR_NONE, err);
  err = ui_overlay_director_mount_component(dir, comp2, 2, &overlay2);
  ASSERT_EQ(UI_ERROR_NONE, err);
  g_overlay_destroy_mock_fail = 2;
  err = ui_overlay_director_destroy(dir);
  ASSERT_EQ(UI_ERROR_UNKNOWN, err);

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(comp1);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(comp2);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  return UI_ERROR_NONE;
}
#endif

int main(void) {
  if (test_invalid_args() != UI_ERROR_NONE) {
    s_tests_failed++;
  }
  if (test_overlay_director_lifecycle() != UI_ERROR_NONE) {
    s_tests_failed++;
  }
  if (test_unmount_detached_parent() != UI_ERROR_NONE) {
    s_tests_failed++;
  }
  if (test_oom() != UI_ERROR_NONE) {
    s_tests_failed++;
  }
#ifdef UI_TEST_MOCK_ALLOC
  if (test_mock_failures() != UI_ERROR_NONE) {
    s_tests_failed++;
  }
#endif

  printf("Tests passed: %d\n", s_tests_passed);
  printf("Tests failed: %d\n", s_tests_failed);

  if (s_tests_failed > 0) {
    return 1;
  }
  return 0;
}
