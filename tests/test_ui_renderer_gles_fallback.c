/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern ui_error_t ui_renderer_gles_fallback_init(struct ui_renderer *renderer);

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
extern int g_mock_gles2_create_fail;
#endif

int main(void) {
  struct ui_renderer renderer;
  ui_error_t rc;

  printf("Running ui_renderer_gles_fallback tests...\n");

  rc = ui_renderer_gles_fallback_init(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected UI_ERROR_INVALID_ARGUMENT on NULL renderer\n");
    return 1;
  }

  rc = ui_renderer_gles_fallback_init(&renderer);
  if (rc == UI_ERROR_NONE && renderer.vtable) {
    struct ui_rect rect = {0.0f, 0.0f, 10.0f, 10.0f};
    struct ui_color color = {1.0f, 0.0f, 0.0f, 0.5f};
    unsigned char buf[4];

    /* Valid cases */
    renderer.vtable->begin_frame(renderer.ctx, 800, 600);
    renderer.vtable->draw_rect(renderer.ctx, &rect, &color);
    renderer.vtable->draw_text(renderer.ctx, "test", NULL, &rect);
    renderer.vtable->draw_image(renderer.ctx, NULL, &rect);
    renderer.vtable->draw_gradient(renderer.ctx, &rect, NULL);
    renderer.vtable->draw_path(renderer.ctx, NULL, &color);
    renderer.vtable->push_clip(renderer.ctx, &rect);
    renderer.vtable->pop_clip(renderer.ctx);
    renderer.vtable->set_blend_mode(renderer.ctx, UI_CSS_BLEND_MODE_NORMAL);
    renderer.vtable->set_shadow(renderer.ctx, NULL);
    renderer.vtable->read_pixels(renderer.ctx, buf);
    renderer.vtable->end_frame(renderer.ctx);

    /* Invalid cases */
    renderer.vtable->begin_frame(NULL, 800, 600);
    renderer.vtable->draw_rect(NULL, &rect, &color);
    renderer.vtable->draw_rect(renderer.ctx, NULL, &color);
    renderer.vtable->draw_rect(renderer.ctx, &rect, NULL);
    renderer.vtable->read_pixels(NULL, buf);
    renderer.vtable->read_pixels(renderer.ctx, NULL);

    renderer.vtable->destroy(renderer.ctx);
  }

  renderer.vtable->destroy(NULL);

#ifdef UI_TEST_MOCK_ALLOC
  /* Malloc failure in ui_renderer_gles_fallback_init for GLES2 */
  {
    int i;
    for (i = 0; i < 5; i++) {
      g_malloc_fail_countdown = i;
      if (ui_renderer_gles_fallback_init(&renderer) == UI_ERROR_NONE)
        renderer.vtable->destroy(renderer.ctx);
    }
    g_malloc_fail_countdown = -1;
  }

  /* Force GLES2 to fail to test GL1 fallback failures */
  g_mock_gles2_create_fail = 1;
  {
    int i;
    for (i = 0; i < 5; i++) {
      g_malloc_fail_countdown = i;
      if (ui_renderer_gles_fallback_init(&renderer) == UI_ERROR_NONE)
        renderer.vtable->destroy(renderer.ctx);
    }
    g_malloc_fail_countdown = -1;
  }
  g_mock_gles2_create_fail = 0;
#endif

  printf("ui_renderer_gles_fallback tests passed.\n");
  return 0;
}
