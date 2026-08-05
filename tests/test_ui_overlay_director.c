/* clang-format off */
#include "ui_overlay_director.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

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

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_component_create(&comp);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_overlay_director_create(NULL, &dir));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_overlay_director_create(root, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_overlay_director_destroy(NULL));

  ui_overlay_director_create(root, &dir);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_overlay_director_mount_component(NULL, comp, 1, &overlay));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_overlay_director_mount_component(dir, NULL, 1, &overlay));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_overlay_director_mount_component(dir, comp, 1, NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_overlay_director_unmount(NULL, overlay));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_overlay_director_unmount(dir, NULL));

  /* Try unmount unknown overlay */
  ui_overlay_director_mount_component(dir, comp, 1, &overlay);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root2);
  ui_overlay_director_create(root2, &dir2);

  ASSERT_EQ(UI_ERROR_NOT_FOUND, ui_overlay_director_unmount(dir2, overlay));

  (void)ui_overlay_director_destroy(dir2);
  (void)ui_dom_node_destroy(root2);
  (void)ui_overlay_director_destroy(dir);
  (void)ui_dom_node_destroy(root);
  (void)ui_component_destroy(comp);
  return UI_ERROR_NONE;
}

static ui_error_t test_overlay_director_lifecycle(void) {
  struct ui_dom_node *root = NULL;
  struct ui_overlay_director *dir = NULL;
  struct ui_component *comp1 = NULL;
  struct ui_component *comp2 = NULL;
  struct ui_overlay *overlay1 = NULL;
  struct ui_overlay *overlay2 = NULL;
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

  /* Mount first overlay */
  err = ui_overlay_director_mount_component(dir, comp1, 100, &overlay1);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_TRUE(overlay1 != NULL);
  ASSERT_TRUE(root->first_child != NULL);

  /* Mount second overlay */
  err = ui_overlay_director_mount_component(dir, comp2, 200, &overlay2);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_TRUE(overlay2 != NULL);
  ASSERT_TRUE(root->first_child != root->last_child); /* Now two children */

  /* Unmount first (it is not the first_overlay in the list) */
  err = ui_overlay_director_unmount(dir, overlay1);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Unmount second */
  err = ui_overlay_director_unmount(dir, overlay2);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Destroy component manually since we unmounted it, wait, ui_component_mount
     transfers ownership? No, ui_component_destroy must be called by the user.
   */
  (void)ui_component_destroy(comp1);

  /* Destroy director (should unmount overlay2 automatically) */
  err = ui_overlay_director_destroy(dir);
  ASSERT_EQ(UI_ERROR_NONE, err);

  (void)ui_component_destroy(comp2);

  /* Root node should have 0 children now */
  ASSERT_TRUE(root->first_child == NULL);
  ASSERT_TRUE(root->last_child == NULL);

  (void)ui_dom_node_destroy(root);
  return UI_ERROR_NONE;
}

static ui_error_t test_oom(void) {
  struct ui_dom_node *root = NULL;
  struct ui_overlay_director *dir = NULL;
  struct ui_component *comp = NULL;
  struct ui_overlay *overlay = NULL;
  ui_error_t err;
  int i;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_component_create(&comp);

  /* Creation OOM */
  g_malloc_fail_countdown = 0;
  err = ui_overlay_director_create(root, &dir);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, err);

  ui_overlay_director_create(root, &dir);

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

  /* Mount OOM with existing child */
  ui_overlay_director_mount_component(dir, comp, 1, &overlay);
  for (i = 0; i < 20; i++) {
    struct ui_overlay *overlay2 = NULL;
    g_malloc_fail_countdown = i;
    err = ui_overlay_director_mount_component(dir, comp, 1, &overlay2);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (err == UI_ERROR_NONE) {
      ui_overlay_director_unmount(dir, overlay2);
      break;
    }
  }

  (void)ui_overlay_director_destroy(dir);
  (void)ui_dom_node_destroy(root);
  (void)ui_component_destroy(comp);
}

int main(void) {
  test_invalid_args();
  test_overlay_director_lifecycle();
  test_oom();

  printf("Tests passed: %d\n", s_tests_passed);
  printf("Tests failed: %d\n", s_tests_failed);

  if (s_tests_failed > 0) {
    return 1;
  }
  return 0;
}
