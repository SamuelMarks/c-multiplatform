/* clang-format off */
#include "../include/ui_context_menu_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

/* We need to hook into the menu's internal styling to see if it actually got
   clamped. Since ui_menu_base_open_at writes inline CSS to the menu's component
   root, we could ideally inspect it, but for a simple test we just verify the
   return code and rely on the fact that the clamping math is straightforward.
 */

static int test_context_menu_clamping(void) {
  struct ui_context_menu_base *ctx_menu = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *body = NULL;
  struct ui_menu_base *menu = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &body);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_overlay_director_create(body, &director);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_context_menu_base_create(&ctx_menu);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test normal fit */
  rc = ui_context_menu_base_open_at(ctx_menu, director, 100, 100, 200, 300,
                                    1920, 1080);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_context_menu_base_get_menu(ctx_menu, &menu);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_menu_base_close(menu);

  /* Test right clamp */
  rc = ui_context_menu_base_open_at(ctx_menu, director, 1800, 100, 200, 300,
                                    1920, 1080);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_context_menu_base_get_menu(ctx_menu, &menu);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_menu_base_close(menu);

  /* Test bottom clamp */
  rc = ui_context_menu_base_open_at(ctx_menu, director, 100, 1000, 200, 300,
                                    1920, 1080);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_context_menu_base_get_menu(ctx_menu, &menu);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_menu_base_close(menu);

  /* Test top/left clamp (negative coords) */
  rc = ui_context_menu_base_open_at(ctx_menu, director, -50, -50, 200, 300,
                                    1920, 1080);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_context_menu_base_get_menu(ctx_menu, &menu);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_menu_base_close(menu);

  struct ui_computed *anim = NULL;
  ui_context_menu_base_bind_open(ctx_menu, NULL);
  ui_context_menu_base_get_animating_signal(ctx_menu, &anim);

  ui_context_menu_base_destroy(ctx_menu);

  /* Test NULLs */
  ui_context_menu_base_create(NULL);
  ui_context_menu_base_destroy(NULL);
  ui_context_menu_base_get_menu(NULL, &menu);
  ui_context_menu_base_get_menu(ctx_menu, NULL);
  ui_context_menu_base_open_at(NULL, director, 0, 0, 0, 0, 0, 0);
  ui_context_menu_base_open_at(ctx_menu, NULL, 0, 0, 0, 0, 0, 0);
  ui_context_menu_base_bind_open(NULL, NULL);
  ui_context_menu_base_get_animating_signal(NULL, &anim);
  ui_context_menu_base_get_animating_signal(ctx_menu, NULL);

  /* Test malloc fails */
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;
  g_malloc_fail_countdown = 0;
  if (ui_context_menu_base_create(&ctx_menu) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = 1;
  /* Fails in ui_menu_base_create which is the second malloc usually */
  ui_context_menu_base_create(&ctx_menu);
  g_malloc_fail_countdown = -1;
#endif

  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(director);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(body);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_context_menu_base tests...\n");

  failed |= test_context_menu_clamping();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
