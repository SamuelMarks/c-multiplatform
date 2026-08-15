/* clang-format off */
#include "../include/ui_renderer.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

void test_ui_renderer_oom_fallback(void);
void test_ui_renderer_oom_fallback_error(void);
void test_ui_renderer_oom_real(void);
int main(void) {
  struct ui_renderer *renderer = NULL;
  ui_error_t rc;

  printf("Running ui_renderer tests...\n");
  test_ui_renderer_oom_fallback();
  test_ui_renderer_oom_fallback_error();
  test_ui_renderer_oom_real();
#ifdef _WIN32
  if (getenv("WINELOADER") != NULL) {
    printf("Skipping ui_renderer tests under Wine CI to avoid pipe hangs.\n");
    return 0;
  }
#endif

  if (ui_renderer_create(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL out_renderer\n");
    return 1;
  }

  if (ui_renderer_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on NULL renderer to destroy\n");
    return 1;
  }

  rc = ui_renderer_create(&renderer);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create ui_renderer\n");
    /* It might fail if no backend is available, but tests usually run on
       environments where at least fallback can init. If GLES and Native fail,
       it returns UI_ERROR_UNKNOWN. Let's not fail the test completely if it
       returns unknown, because the test environment might genuinely lack OpenGL
       and Gdiplus (though unlikely on win32). Actually, let's assume one
       succeeds. */
    return 1;
  }

  if (renderer->vtable == NULL) {
    printf("Renderer vtable is NULL\n");
    return 1;
  }

  {
    struct ui_rect rect;
    struct ui_color color;
    struct ui_font *font =
        NULL; /* Can't easily mock real font here without font manager */
    struct ui_image *img = NULL;
    struct ui_css_image gradient;
    struct ui_path path;
    struct ui_path_cmd cmds[4];

    rect.x = 0;
    rect.y = 0;
    rect.width = 100;
    rect.height = 100;
    color.r = 1.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = 1.0f;

    /* Test basic rendering commands to ensure no crashes */
    rc = renderer->vtable->begin_frame(renderer->ctx, 800, 600);
    if (rc != 0) {
      printf("begin_frame failed\n");
      return 1;
    }

    renderer->vtable->draw_rect(renderer->ctx, &rect, &color);
    renderer->vtable->push_clip(renderer->ctx, &rect);
    renderer->vtable->pop_clip(renderer->ctx);

    /* Test path */
    cmds[0].type = UI_PATH_CMD_MOVE_TO;
    cmds[0].x1 = 0;
    cmds[0].y1 = 0;
    cmds[1].type = UI_PATH_CMD_LINE_TO;
    cmds[1].x1 = 100;
    cmds[1].y1 = 100;
    cmds[2].type = UI_PATH_CMD_BEZIER_TO;
    cmds[2].x1 = 50;
    cmds[2].y1 = 50;
    cmds[2].x2 = 60;
    cmds[2].y2 = 60;
    cmds[2].x3 = 70;
    cmds[2].y3 = 70;
    cmds[3].type = UI_PATH_CMD_CLOSE;
    path.cmds = cmds;
    path.cmd_count = 4;
    renderer->vtable->draw_path(renderer->ctx, &path, &color);

    /* Test gradient */
    gradient.type = UI_CSS_IMAGE_LINEAR_GRADIENT;
    gradient.data.linear_gradient.angle = 90.0f;
    gradient.data.linear_gradient.stop_count = 2;
    gradient.data.linear_gradient.stops[0].color.components[0] = 1.0f;
    gradient.data.linear_gradient.stops[0].color.components[1] = 0.0f;
    gradient.data.linear_gradient.stops[0].color.components[2] = 0.0f;
    gradient.data.linear_gradient.stops[0].color.components[3] = 1.0f;
    gradient.data.linear_gradient.stops[1].color.components[0] = 0.0f;
    gradient.data.linear_gradient.stops[1].color.components[1] = 1.0f;
    gradient.data.linear_gradient.stops[1].color.components[2] = 0.0f;
    gradient.data.linear_gradient.stops[1].color.components[3] = 1.0f;
    renderer->vtable->draw_gradient(renderer->ctx, &rect, &gradient);

    gradient.type = UI_CSS_IMAGE_RADIAL_GRADIENT;
    gradient.data.radial_gradient.stop_count = 2;
    gradient.data.radial_gradient.stops[0].color.components[0] = 1.0f;
    gradient.data.radial_gradient.stops[0].color.components[1] = 0.0f;
    gradient.data.radial_gradient.stops[0].color.components[2] = 0.0f;
    gradient.data.radial_gradient.stops[0].color.components[3] = 1.0f;
    gradient.data.radial_gradient.stops[1].color.components[0] = 0.0f;
    gradient.data.radial_gradient.stops[1].color.components[1] = 1.0f;
    gradient.data.radial_gradient.stops[1].color.components[2] = 0.0f;
    gradient.data.radial_gradient.stops[1].color.components[3] = 1.0f;
    renderer->vtable->draw_gradient(renderer->ctx, &rect, &gradient);

    gradient.type = UI_CSS_IMAGE_CONIC_GRADIENT;
    gradient.data.conic_gradient.stop_count = 2;
    gradient.data.conic_gradient.stops[0].color.components[0] = 1.0f;
    gradient.data.conic_gradient.stops[0].color.components[1] = 0.0f;
    gradient.data.conic_gradient.stops[0].color.components[2] = 0.0f;
    gradient.data.conic_gradient.stops[0].color.components[3] = 1.0f;
    gradient.data.conic_gradient.stops[1].color.components[0] = 0.0f;
    gradient.data.conic_gradient.stops[1].color.components[1] = 1.0f;
    gradient.data.conic_gradient.stops[1].color.components[2] = 0.0f;
    gradient.data.conic_gradient.stops[1].color.components[3] = 1.0f;
    renderer->vtable->draw_gradient(renderer->ctx, &rect, &gradient);

    gradient.type = 999; /* Unknown type */
    renderer->vtable->draw_gradient(renderer->ctx, &rect, &gradient);

    /* Stubs */
    renderer->vtable->draw_image(renderer->ctx, img, &rect);
    renderer->vtable->set_blend_mode(renderer->ctx, UI_CSS_BLEND_MODE_NORMAL);
    renderer->vtable->set_shadow(renderer->ctx, NULL);

    rc = renderer->vtable->end_frame(renderer->ctx);
    if (rc != 0) {
      printf("end_frame failed\n");
      return 1;
    }

    /* Test error handling with NULL args where applicable */
    renderer->vtable->begin_frame(NULL, 100, 100);
    renderer->vtable->draw_rect(NULL, &rect, &color);
    renderer->vtable->draw_rect(renderer->ctx, NULL, &color);
    renderer->vtable->draw_rect(renderer->ctx, &rect, NULL);
    renderer->vtable->draw_text(NULL, "test", font, &rect);
    renderer->vtable->draw_gradient(NULL, &rect, &gradient);
    renderer->vtable->draw_gradient(renderer->ctx, NULL, &gradient);
    renderer->vtable->draw_gradient(renderer->ctx, &rect, NULL);
    renderer->vtable->draw_path(NULL, &path, &color);
    renderer->vtable->draw_path(renderer->ctx, NULL, &color);
    renderer->vtable->draw_path(renderer->ctx, &path, NULL);
    renderer->vtable->push_clip(NULL, &rect);
    renderer->vtable->push_clip(renderer->ctx, NULL);
    renderer->vtable->pop_clip(NULL);
  }

  rc = ui_renderer_destroy(renderer);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy ui_renderer\n");
    return 1;
  }

  /* Test GLES fallback directly to ensure coverage of gles_fallback_vtable */
  {
    struct ui_renderer gles_renderer;
    struct ui_rect rect;
    struct ui_color color;
    struct ui_css_image gradient;
    struct ui_path path;

    rect.x = 0;
    rect.y = 0;
    rect.width = 10;
    rect.height = 10;
    color.r = 1;
    color.g = 1;
    color.b = 1;
    color.a = 1;

    if (ui_renderer_gles_fallback_init(NULL) != UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed fallback init NULL\n");
      return 1;
    }

    rc = ui_renderer_gles_fallback_init(&gles_renderer);
    if (rc == UI_ERROR_NONE) {
      gles_renderer.vtable->begin_frame(gles_renderer.ctx, 800, 600);
      gles_renderer.vtable->begin_frame(NULL, 800, 600);

      gles_renderer.vtable->draw_rect(gles_renderer.ctx, &rect, &color);
      gles_renderer.vtable->draw_rect(NULL, &rect, &color);

      gles_renderer.vtable->draw_text(gles_renderer.ctx, "test", NULL, &rect);
      gles_renderer.vtable->draw_image(gles_renderer.ctx, NULL, &rect);
      gles_renderer.vtable->draw_gradient(gles_renderer.ctx, &rect, &gradient);
      gles_renderer.vtable->draw_path(gles_renderer.ctx, &path, &color);
      gles_renderer.vtable->push_clip(gles_renderer.ctx, &rect);
      gles_renderer.vtable->pop_clip(gles_renderer.ctx);
      gles_renderer.vtable->set_blend_mode(gles_renderer.ctx,
                                           UI_CSS_BLEND_MODE_NORMAL);
      gles_renderer.vtable->set_shadow(gles_renderer.ctx, NULL);

      gles_renderer.vtable->end_frame(gles_renderer.ctx);
      gles_renderer.vtable->end_frame(NULL);

      gles_renderer.vtable->destroy(gles_renderer.ctx);
      gles_renderer.vtable->destroy(NULL);
    }
  }

  /* Test Native fallback directly to ensure coverage */
  {
    struct ui_renderer native_renderer;
    if (ui_renderer_native_init(NULL) != UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed native init NULL\n");
      return 1;
    }
    rc = ui_renderer_native_init(&native_renderer);
    if (rc == UI_ERROR_NONE) {
      if (native_renderer.vtable && native_renderer.vtable->destroy) {
        native_renderer.vtable->destroy(native_renderer.ctx);
      }
    }
  }

  {
    struct ui_renderer *r1 = NULL;
    struct ui_renderer *r2 = NULL;
    struct ui_renderer_vtable vtable_no_destroy = {0};

    ui_renderer_create(&r1);
    r1->vtable = NULL;
    ui_renderer_destroy(r1);

    ui_renderer_create(&r2);
    r2->vtable = &vtable_no_destroy;
    ui_renderer_destroy(r2);
  }
  printf("ui_renderer tests passed.\n");
  return 0;
}

void test_ui_renderer_oom_fallback(void) {
  struct ui_renderer *renderer;
  extern int g_native_init_fail;
  g_native_init_fail = 1;
  ui_renderer_create(&renderer);
  g_native_init_fail = 0;
}

void test_ui_renderer_oom_fallback_error(void) {
  struct ui_renderer *renderer;
  extern int g_native_init_fail;
  extern int g_gles_init_fail;
  g_native_init_fail = 1;
  g_gles_init_fail = 1;
  ui_renderer_create(&renderer);
  g_native_init_fail = 0;
  g_gles_init_fail = 0;
}
void test_ui_renderer_oom_real(void) {
  extern int g_malloc_fail_countdown;
  struct ui_renderer *renderer;
  g_malloc_fail_countdown = 0;
  ui_renderer_create(&renderer);
  g_malloc_fail_countdown = -1;
}
