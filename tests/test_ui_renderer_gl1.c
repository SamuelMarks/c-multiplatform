/* clang-format off */
#include "../include/ui_renderer_gl1.h"
#include "../include/ui_error.h"
#include "../include/ui_font_manager.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static const unsigned char test_ttf_bytes[] = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x63, 0x6d, 0x61, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c,
    0x00, 0x00, 0x00, 0x14, 0x68, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x36, 0x68, 0x68, 0x65, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x24,
    0x68, 0x6d, 0x74, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xea,
    0x00, 0x00, 0x00, 0x08, 0x67, 0x6c, 0x79, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xf2, 0x00, 0x00, 0x00, 0x01, 0x6c, 0x6f, 0x63, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x00, 0x00, 0x00, 0x04,
    0x6d, 0x61, 0x78, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf7,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int test_gl1(void) {
  struct ui_renderer_backend *backend = NULL;
  struct ui_color col = {1, 1, 1, 1};
  void *tex = NULL;

#ifdef __EMSCRIPTEN__
  if (ui_renderer_gl1_create(&backend) != UI_ERROR_UNSUPPORTED)
    return 1;
  {
    ui_error_t rc_cleanup = ui_renderer_gl1_destroy(NULL);
    if (rc_cleanup != UI_ERROR_INVALID_ARGUMENT) {
      return 1;
    }
  }
  return 0;
#else

  if (ui_renderer_gl1_create(&backend) != UI_ERROR_NONE)
    return 1;

  if (backend->init(backend, NULL, NULL) != UI_ERROR_NONE)
    return 1;

  if (backend->set_viewport(backend, 0, 0, 800, 600) != UI_ERROR_NONE)
    return 1;
  if (backend->clear(backend, col) != UI_ERROR_NONE)
    return 1;

  if (backend->draw_rect(backend, 0, 0, 10, 10, col) != UI_ERROR_NONE)
    return 1;
  if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) != UI_ERROR_NONE)
    return 1;

  if (backend->create_texture(backend, 10, 10, &tex) != UI_ERROR_NONE)
    return 1;
  if (backend->set_render_target(backend, tex) != UI_ERROR_NONE)
    return 1;
  if (backend->draw_texture(backend, tex, 0, 0, 10, 10, 1.0f) != UI_ERROR_NONE)
    return 1;
  if (backend->flush(backend) != UI_ERROR_NONE)
    return 1;
  if (backend->flush(backend) != UI_ERROR_NONE) /* Hit the empty flush branch */
    return 1;

  {
    unsigned char buf[4];
    if (backend->read_pixels(backend, 1, 1, buf) != UI_ERROR_NONE)
      return 1;
  }

  if (backend->destroy_texture(backend, tex) != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_renderer_gl1_destroy(backend);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* Null checks */
  {
    ui_error_t rc_cleanup = ui_renderer_gl1_create(NULL);
    if (rc_cleanup != UI_ERROR_INVALID_ARGUMENT) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_renderer_gl1_destroy(NULL);
    if (rc_cleanup != UI_ERROR_INVALID_ARGUMENT) {
      return 1;
    }
  }

  g_malloc_fail_countdown = 0;
  if (ui_renderer_gl1_create(&backend) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_renderer_gl1_create(&backend);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  g_malloc_fail_countdown = 0;
  if (backend->init(backend, NULL, NULL) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_renderer_gl1_destroy(backend);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  /* texture malloc fail */
  {
    ui_error_t rc_cleanup = ui_renderer_gl1_create(&backend);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  if (backend->init(backend, NULL, NULL) != UI_ERROR_NONE)
    return 1;
  g_malloc_fail_countdown = 0;
  if (backend->create_texture(backend, 10, 10, &tex) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  /* batch flush */
  {
    int i;
    for (i = 0; i < 2100; i++) {
      if (backend->draw_rect(backend, 0, 0, 1, 1, col) != UI_ERROR_NONE)
        return 1;
    }
  }

  /* draw_text tests */
  {
    struct ui_font_manager *fm = NULL;
    struct ui_font *font = NULL;

    if (ui_font_manager_create(&fm) != UI_ERROR_NONE)
      return 1;

    if (ui_font_manager_load_font_memory(fm, test_ttf_bytes,
                                         sizeof(test_ttf_bytes),
                                         &font) != UI_ERROR_NONE) {
      ui_font_manager_destroy(fm);
      return 1;
    }

    /* Invalid arguments */
    if (backend->draw_text(NULL, "A", font, 0, 0, 16.0f, col) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->draw_text(backend, NULL, font, 0, 0, 16.0f, col) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->draw_text(backend, "A", NULL, 0, 0, 16.0f, col) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->draw_text(backend, "A", font, 0, 0, 0.0f, col) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->draw_text(backend, "A", font, 0, 0, -1.0f, col) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;

    /* Draw valid text and whitespace with dummy font (width=0, height=0) */
    if (backend->draw_text(backend, "A B", font, 0, 0, 16.0f, col) !=
        UI_ERROR_NONE)
      return 1;

#ifdef UI_TEST_MOCK_ALLOC
    {
      extern int g_mock_gl1_glyph_box;
      extern int g_mock_gl1_glyph_metrics_fail;
      extern int g_mock_gles2_flush_fail;

      /* Flush failure */
      g_mock_gles2_flush_fail = 1;
      if (backend->draw_text(backend, "A", font, 0, 0, 16.0f, col) !=
          UI_ERROR_UNKNOWN)
        return 1;
      g_mock_gles2_flush_fail = 0;

      /* Metrics query failure */
      g_mock_gl1_glyph_metrics_fail = 1;
      if (backend->draw_text(backend, "A", font, 0, 0, 16.0f, col) !=
          UI_ERROR_NONE)
        return 1;
      g_mock_gl1_glyph_metrics_fail = 0;

      /* Valid glyph boxes (width > 0, height > 0) and height 0 for 'H' */
      g_mock_gl1_glyph_box = 1;
      if (backend->draw_text(backend, "ABH", font, 0, 0, 16.0f, col) !=
          UI_ERROR_NONE)
        return 1;

      /* Draw rect failure inside draw_text */
      /* Text with > 2048 glyphs overflows vertex batch during draw_text and
       * flushes */
      {
        char long_text[2050];
        memset(long_text, 'A', 2049);
        long_text[2049] = '\0';
        g_mock_gles2_flush_fail = 2;
        if (backend->draw_text(backend, long_text, font, 0, 0, 16.0f, col) !=
            UI_ERROR_UNKNOWN)
          return 1;
        g_mock_gles2_flush_fail = 0;
      }
      g_mock_gl1_glyph_box = 0;
      backend->flush(backend);
    }
#endif

    ui_font_manager_destroy(fm);
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
    if (backend->draw_triangles(backend, dummy_verts, 8192, dummy_indices,
                                8192) != UI_ERROR_NONE) {
      return 1;
    }
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_rect(backend, 0, 0, 10, 10, col) != UI_ERROR_UNKNOWN) {
      return 1;
    }

    /* Test draw_border fail 1 */
    g_mock_gles2_flush_fail = 0;
    if (backend->draw_triangles(backend, dummy_verts, 8192, dummy_indices,
                                8192) != UI_ERROR_NONE) {
      return 1;
    }
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) !=
        UI_ERROR_UNKNOWN) {
      return 1;
    }

    /* Test draw_border fail 2 */
    g_mock_gles2_flush_fail = 0;
    if (backend->draw_triangles(backend, dummy_verts, 8192 - 4, dummy_indices,
                                8192 - 6) != UI_ERROR_NONE) {
      return 1;
    }
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) !=
        UI_ERROR_UNKNOWN) {
      return 1;
    }

    /* Test draw_border fail 3 */
    g_mock_gles2_flush_fail = 0;
    if (backend->draw_triangles(backend, dummy_verts, 8192 - 8, dummy_indices,
                                8192 - 12) != UI_ERROR_NONE) {
      return 1;
    }
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) !=
        UI_ERROR_UNKNOWN) {
      return 1;
    }

    /* Test draw_border fail 4 */
    g_mock_gles2_flush_fail = 0;
    if (backend->draw_triangles(backend, dummy_verts, 8192 - 12, dummy_indices,
                                8192 - 18) != UI_ERROR_NONE) {
      return 1;
    }
    g_mock_gles2_flush_fail = 1;
    if (backend->draw_border(backend, 0, 0, 10, 10, 1, col) !=
        UI_ERROR_UNKNOWN) {
      return 1;
    }

    g_mock_gles2_flush_fail = 1;
    if (backend->draw_texture(backend, tex, 0, 0, 10, 10, 1.0f) !=
        UI_ERROR_UNKNOWN) {
      return 1;
    }

    g_mock_gles2_flush_fail = 0;
  }

  {
    extern int g_mock_gles2_destroy_fail;
    g_mock_gles2_destroy_fail = 1;
    if (ui_renderer_gl1_destroy(backend) != UI_ERROR_UNKNOWN) {
      return 1;
    }
    g_mock_gles2_destroy_fail = 0;
  }
#endif

  /* trigger out of memory in batching */
  {
    struct ui_vertex v[8193];
    unsigned short idx[24577];
    if (backend->draw_triangles(backend, v, 8193, idx, 24577) !=
        UI_ERROR_OUT_OF_MEMORY)
      return 1;
    if (backend->draw_triangles(backend, v, 10, idx, 24577) !=
        UI_ERROR_OUT_OF_MEMORY)
      return 1;
    if (backend->flush(backend) != UI_ERROR_NONE)
      return 1;
    if (backend->draw_triangles(backend, v, 8100, idx, 10) != UI_ERROR_NONE)
      return 1;
    if (backend->draw_triangles(backend, v, 10, idx, 24500) != UI_ERROR_NONE)
      return 1;
  }

  /* Null args to backend funcs */
  if (backend->flush(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_triangles(NULL, NULL, 0, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_triangles(backend, NULL, 0, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_vertex v[1];
    unsigned short idx[1];
    if (backend->draw_triangles(backend, v, 0, NULL, 0) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;

    /* test user_data NULL for draw_triangles and flush */
    {
      void *tmp = backend->user_data;
      backend->user_data = NULL;
      if (backend->draw_triangles(backend, v, 0, idx, 0) !=
          UI_ERROR_INVALID_ARGUMENT)
        return 1;
      if (backend->flush(backend) != UI_ERROR_INVALID_ARGUMENT)
        return 1;
      backend->user_data = tmp;
    }
  }

  if (backend->create_texture(NULL, 10, 10, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->create_texture(backend, 10, 10, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (backend->destroy_texture(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->destroy_texture(backend, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (backend->read_pixels(NULL, 10, 10, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->read_pixels(backend, 10, 10, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->init(NULL, NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->set_render_target(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_texture(NULL, NULL, 0, 0, 0, 0, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_texture(backend, NULL, 0, 0, 0, 0, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->read_pixels(NULL, 0, 0, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    ui_error_t rc_cleanup = ui_renderer_gl1_destroy(backend);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

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
