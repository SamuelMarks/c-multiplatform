/* clang-format off */
#include "../include/ui_renderer_gl1.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_gl1(void) {
  struct ui_renderer_backend *backend = NULL;
  struct ui_color col = {1, 1, 1, 1};
  void *tex = NULL;

#ifdef __EMSCRIPTEN__
  if (ui_renderer_gl1_create(&backend) != UI_ERROR_UNSUPPORTED)
    return 1;
  ui_renderer_gl1_destroy(NULL);
  return 0;
#else

  if (ui_renderer_gl1_create(&backend) != UI_ERROR_NONE)
    return 1;

  if (backend->init(backend, NULL, NULL) != UI_ERROR_NONE)
    return 1;

  backend->set_viewport(backend, 0, 0, 800, 600);
  backend->clear(backend, col);

  backend->draw_rect(backend, 0, 0, 10, 10, col);
  backend->draw_border(backend, 0, 0, 10, 10, 1, col);

  if (backend->create_texture(backend, 10, 10, &tex) != UI_ERROR_NONE)
    return 1;
  backend->set_render_target(backend, tex);
  backend->draw_texture(backend, tex, 0, 0, 10, 10, 1.0f);
  backend->flush(backend);
  backend->flush(backend); /* Hit the empty flush branch */

  {
    unsigned char buf[4];
    backend->read_pixels(backend, 1, 1, buf);
  }

  backend->destroy_texture(backend, tex);

  ui_renderer_gl1_destroy(backend);

  /* Null checks */
  ui_renderer_gl1_create(NULL);
  ui_renderer_gl1_destroy(NULL);

  g_malloc_fail_countdown = 0;
  if (ui_renderer_gl1_create(&backend) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_renderer_gl1_create(&backend);
  g_malloc_fail_countdown = 0;
  if (backend->init(backend, NULL, NULL) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_renderer_gl1_destroy(backend);

  /* texture malloc fail */
  ui_renderer_gl1_create(&backend);
  backend->init(backend, NULL, NULL);
  g_malloc_fail_countdown = 0;
  if (backend->create_texture(backend, 10, 10, &tex) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  /* batch flush */
  {
    int i;
    for (i = 0; i < 2100; i++) {
      backend->draw_rect(backend, 0, 0, 1, 1, col);
    }
  }

  /* trigger out of memory in batching */
  {
    struct ui_vertex v[8193];
    unsigned short idx[24577];
    backend->draw_triangles(backend, v, 8193, idx, 24577);
  }

  /* Null args to backend funcs */
  backend->flush(NULL);
  backend->draw_triangles(NULL, NULL, 0, NULL, 0);
  backend->init(NULL, NULL, NULL);
  backend->destroy(NULL);
  backend->create_texture(NULL, 0, 0, NULL);
  backend->destroy_texture(NULL, NULL);
  backend->set_render_target(NULL, NULL);
  backend->draw_texture(NULL, NULL, 0, 0, 0, 0, 0);
  backend->read_pixels(NULL, 0, 0, NULL);

  ui_renderer_gl1_destroy(backend);

  return 0;
#endif
}

int main(void) {
  int failed = 0;
  failed |= test_gl1();
  if (failed)
    return 1;
  return 0;
}
