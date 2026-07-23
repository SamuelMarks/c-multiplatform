/* clang-format off */
#include "ui_theme_provider.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define FAIL_CHECK(line, expr)                                                 \
  do {                                                                         \
    if (expr) {                                                                \
      printf("Failed at line %d: %s\n", line, #expr);                          \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)

static int test_theme_provider(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  struct ui_theme_provider *provider;
  struct ui_dom_node *root, *child, *grandchild;
  struct ui_design_token_dict *resolved_dict;
  enum ui_error err;
  int failed = 0;

  err = ui_arena_create(4096, &arena);
  FAIL_CHECK(__LINE__, err != UI_ERROR_NONE);

  err = ui_design_token_dict_init(arena, &dict);
  FAIL_CHECK(__LINE__, err != UI_ERROR_NONE);

  err = ui_theme_provider_create(arena, &dict, &provider);
  FAIL_CHECK(__LINE__, err != UI_ERROR_NONE);

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  FAIL_CHECK(__LINE__, err != UI_ERROR_NONE);

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
  FAIL_CHECK(__LINE__, err != UI_ERROR_NONE);

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &grandchild);
  FAIL_CHECK(__LINE__, err != UI_ERROR_NONE);

  ui_dom_node_append_child(root, child);
  ui_dom_node_append_child(child, grandchild);

  err = ui_theme_provider_mount(provider, root);
  FAIL_CHECK(__LINE__, err != UI_ERROR_NONE);

  err = ui_theme_provider_get(grandchild, &resolved_dict);
  FAIL_CHECK(__LINE__, err != UI_ERROR_NONE || resolved_dict != &dict);

  /* Test ui_theme_provider_get NOT_FOUND branch by getting from an unmounted
   * tree */
  {
    struct ui_dom_node *unmounted_node = NULL;
    struct ui_design_token_dict *unmounted_dict = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &unmounted_node);
    err = ui_theme_provider_get(unmounted_node, &unmounted_dict);
    FAIL_CHECK(__LINE__, err != UI_ERROR_NOT_FOUND);
    ui_dom_node_destroy(unmounted_node);
  }

  /* Test invalid argument branches */
  FAIL_CHECK(__LINE__, ui_theme_provider_create(NULL, &dict, &provider) !=
                           UI_ERROR_INVALID_ARGUMENT);
  FAIL_CHECK(__LINE__, ui_theme_provider_create(arena, NULL, &provider) !=
                           UI_ERROR_INVALID_ARGUMENT);
  FAIL_CHECK(__LINE__, ui_theme_provider_create(arena, &dict, NULL) !=
                           UI_ERROR_INVALID_ARGUMENT);

  FAIL_CHECK(__LINE__,
             ui_theme_provider_mount(NULL, root) != UI_ERROR_INVALID_ARGUMENT);
  FAIL_CHECK(__LINE__, ui_theme_provider_mount(provider, NULL) !=
                           UI_ERROR_INVALID_ARGUMENT);

  FAIL_CHECK(__LINE__, ui_theme_provider_get(NULL, &resolved_dict) !=
                           UI_ERROR_INVALID_ARGUMENT);
  FAIL_CHECK(__LINE__,
             ui_theme_provider_get(root, NULL) != UI_ERROR_INVALID_ARGUMENT);

  /* Test ui_theme_provider_create memory allocation failure */
  {
    struct ui_theme_provider *oom_provider = NULL;
    struct ui_arena *small_arena = NULL;
    ui_arena_create(
        8, &small_arena); /* very small arena so alloc forces a new block */
    g_malloc_fail_countdown = 0; /* so the new block allocation fails */
    err = ui_theme_provider_create(small_arena, &dict, &oom_provider);
    FAIL_CHECK(__LINE__, err != UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;
    ui_arena_destroy(small_arena);
  }

  ui_dom_node_destroy(root);
  ui_arena_destroy(arena);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_theme_provider();
  if (!failed) {
    printf("test_ui_theme_provider passed\n");
  }
  return failed;
}
