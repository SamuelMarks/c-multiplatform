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
  (void)ui_renderer_gl1_destroy(NULL);
  return 0;
#else

  if (ui_renderer_gl1_create(&backend) != UI_ERROR_NONE)
    return 1;

  if (backend->init(backend, NULL, NULL) != UI_ERROR_NONE)
    return 1;

  (void)backend->set_viewport(backend, 0, 0, 800, 600);
  (void)backend->clear(backend, col);

  (void)backend->draw_rect(backend, 0, 0, 10, 10, col);
  (void)backend->draw_border(backend, 0, 0, 10, 10, 1, col);

  if (backend->create_texture(backend, 10, 10, &tex) != UI_ERROR_NONE)
    return 1;
  (void)backend->set_render_target(backend, tex);
  (void)backend->draw_texture(backend, tex, 0, 0, 10, 10, 1.0f);
  (void)backend->flush(backend);
  (void)backend->flush(backend); /* Hit the empty flush branch */

  {
    unsigned char buf[4];
    (void)backend->read_pixels(backend, 1, 1, buf);
  }

  (void)backend->destroy_texture(backend, tex);

  (void)ui_renderer_gl1_destroy(backend);

  /* Null checks */
  (void)ui_renderer_gl1_create(NULL);
  (void)ui_renderer_gl1_destroy(NULL);

  g_malloc_fail_countdown = 0;
  if (ui_renderer_gl1_create(&backend) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  (void)ui_renderer_gl1_create(&backend);
  g_malloc_fail_countdown = 0;
  if (backend->init(backend, NULL, NULL) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  (void)ui_renderer_gl1_destroy(backend);

  /* texture malloc fail */
  (void)ui_renderer_gl1_create(&backend);
  (void)backend->init(backend, NULL, NULL);
  g_malloc_fail_countdown = 0;
  if (backend->create_texture(backend, 10, 10, &tex) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  /* batch flush */
  {
    int i;
    for (i = 0; i < 2100; i++) {
      (void)backend->draw_rect(backend, 0, 0, 1, 1, col);
    }
  }

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_mock_gles2_flush_fail;
    struct ui_vertex dummy_verts[8192];
    unsigned short dummy_indices[8192];
    int i;
    for (i = 0; i < 8192; ++i) {
      dummy_indices[i] = 0;
      dummy_verts[i].x = 0.0f;
      dummy_verts[i].y = 0.0f;
    }

    /* Test draw_rect fail */
    g_mock_gles2_flush_fail = 0;
    (void)backend->draw_triangles(backend, dummy_verts, 8192, dummy_indices,
                                  8192);
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_rect(backend, 0, 0, 10, 10, col) != UI_ERROR_UNKNOWN) {
    }

    /* Test draw_border fail 1 */
    g_mock_gles2_flush_fail = 0;
    (void)backend->draw_triangles(backend, dummy_verts, 8192, dummy_indices,
                                  8192);
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) !=
        UI_ERROR_UNKNOWN) {
    }

    /* Test draw_border fail 2 */
    g_mock_gles2_flush_fail = 0;
    (void)backend->draw_triangles(backend, dummy_verts, 8192 - 4, dummy_indices,
                                  8192 - 6);
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) !=
        UI_ERROR_UNKNOWN) {
    }

    /* Test draw_border fail 3 */
    g_mock_gles2_flush_fail = 0;
    (void)backend->draw_triangles(backend, dummy_verts, 8192 - 8, dummy_indices,
                                  8192 - 12);
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) !=
        UI_ERROR_UNKNOWN) {
    }

    /* Test draw_border fail 4 */
    g_mock_gles2_flush_fail = 0;
    (void)backend->draw_triangles(backend, dummy_verts, 8192 - 12,
                                  dummy_indices, 8192 - 18);
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) !=
        UI_ERROR_UNKNOWN) {
    }

    g_mock_gles2_flush_fail = 1;
    if (backend->draw_texture(backend, tex, 0, 0, 10, 10, 1.0f) !=
        UI_ERROR_UNKNOWN) {
    }

    g_mock_gles2_flush_fail = 0;
  }

  {
    extern int g_mock_gles2_destroy_fail;
    g_mock_gles2_destroy_fail = 1;
    if (ui_renderer_gl1_destroy(backend) != UI_ERROR_UNKNOWN) {
    }
    g_mock_gles2_destroy_fail = 0;
  }
#endif

  /* trigger out of memory in batching */
  {
    struct ui_vertex v[8193];
    unsigned short idx[24577];
    (void)backend->draw_triangles(backend, v, 8193, idx, 24577);
    (void)backend->draw_triangles(backend, v, 10, idx, 24577);
    (void)backend->flush(backend);
    (void)backend->draw_triangles(backend, v, 8100, idx, 10);
    (void)backend->draw_triangles(backend, v, 10, idx, 24500);
  }

  /* Null args to backend funcs */
  (void)backend->flush(NULL);
  (void)backend->draw_triangles(NULL, NULL, 0, NULL, 0);
  (void)backend->draw_triangles(backend, NULL, 0, NULL, 0);
  {
    struct ui_vertex v[1];
    unsigned short idx[1];
    (void)backend->draw_triangles(backend, v, 0, NULL, 0);

    /* test user_data NULL for draw_triangles and flush */
    {
      void *tmp = backend->user_data;
      backend->user_data = NULL;
      (void)backend->draw_triangles(backend, v, 0, idx, 0);
      (void)backend->flush(backend);
      backend->user_data = tmp;
    }
  }

  (void)backend->create_texture(NULL, 10, 10, NULL);
  (void)backend->create_texture(backend, 10, 10, NULL);

  (void)backend->destroy_texture(NULL, NULL);
  (void)backend->destroy_texture(backend, NULL);

  (void)backend->read_pixels(NULL, 10, 10, NULL);
  (void)backend->read_pixels(backend, 10, 10, NULL);
  (void)backend->init(NULL, NULL, NULL);
  (void)backend->destroy(NULL);
  (void)backend->set_render_target(NULL, NULL);
  (void)backend->draw_texture(NULL, NULL, 0, 0, 0, 0, 0);
  (void)backend->draw_texture(backend, NULL, 0, 0, 0, 0, 0);
  (void)backend->read_pixels(NULL, 0, 0, NULL);

  (void)ui_renderer_gl1_destroy(backend);

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
