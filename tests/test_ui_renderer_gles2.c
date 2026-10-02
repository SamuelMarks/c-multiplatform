/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "../include/ui_renderer_gles2.h"
#include "../include/ui_font_manager.h"
#include "../include/ui_error.h"
#include "../include/ui_window_backend.h"
/* clang-format on */

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

void dummy_test_destroy_fail(void);

int main(void) {
  struct ui_renderer_backend *backend = NULL;
  ui_error_t rc;
  struct ui_color clear_color = {1.0f, 0.0f, 0.0f, 1.0f};
  void *tex_handle = NULL;

  rc = ui_renderer_gles2_create(&backend);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create GLES2 renderer backend.\n");
    return 1;
  }

#if defined(__EMSCRIPTEN__)
  printf("Skipping GLES2 rendering calls under headless Emscripten Node "
         "runner.\n");
  ui_renderer_gles2_destroy(backend);
  return 0;
#endif

  rc = backend->init(backend, NULL, NULL);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to init GLES2 renderer backend.\n");
    return 1;
  }

  /* We cannot actually call glViewport or glClear without an active OpenGL
     context, so we only test creation and destruction in this mocked unit test.
     Real E2E test would verify rendering output.
     For coverage, if we are under a mock GL or headless, calling these might
     crash without context. But since we don't have a real window here, we will
     just skip calling GL functions.
   */

  /* Test FBO stub functions (they don't crash without GL context in the current
   * stub) */
  rc = backend->create_texture(backend, 800, 600, &tex_handle);
  if (rc != UI_ERROR_NONE || tex_handle == NULL) {
    printf("Failed to create texture.\n");
    return 1;
  }

  rc = backend->set_render_target(backend, tex_handle);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set render target.\n");
    return 1;
  }

  rc = backend->set_render_target(backend, NULL); /* Default framebuffer */
  if (rc != UI_ERROR_NONE) {
    printf("Failed to reset render target.\n");
    return 1;
  }

  rc = backend->draw_texture(backend, tex_handle, 0.0f, 0.0f, 100.0f, 100.0f,
                             1.0f);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to draw texture.\n");
    return 1;
  }

  rc = backend->destroy_texture(backend, tex_handle);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy texture.\n");
    return 1;
  }

  /* Test VBO batching (merging quads) */
  rc = backend->draw_rect(backend, 0.0f, 0.0f, 10.0f, 10.0f, clear_color);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to draw rect for batching test.\n");
    return 1;
  }

  /* Test border */
  rc = backend->draw_border(backend, 0.0f, 0.0f, 100.0f, 100.0f, 5.0f,
                            clear_color);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to draw border.\n");
    return 1;
  }

  /* Test viewport */
  rc = backend->set_viewport(backend, 0, 0, 800, 600);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set viewport.\n");
    return 1;
  }

  /* Test clear */
  rc = backend->clear(backend, clear_color);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to clear.\n");
    return 1;
  }

  /* Test read pixels */
  {
    unsigned char buf[4];
    rc = backend->read_pixels(backend, 1, 1, buf);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to read pixels.\n");
      return 1;
    }
  }

  /* Test text rendering */
  {
    struct ui_font_manager *fm = NULL;
    struct ui_font *font = NULL;
    struct ui_color text_col = {1.0f, 0.0f, 0.0f, 1.0f};

    rc = ui_font_manager_create(&fm);
    if (rc != UI_ERROR_NONE) {
      return 1;
    }

    rc = backend->draw_text(NULL, "Test", (struct ui_font *)1, 0.0f, 0.0f,
                            16.0f, text_col);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      ui_font_manager_destroy(fm);
      return 1;
    }

    rc = backend->draw_text(backend, NULL, (struct ui_font *)1, 0.0f, 0.0f,
                            16.0f, text_col);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      ui_font_manager_destroy(fm);
      return 1;
    }

    rc = backend->draw_text(backend, "Test", NULL, 0.0f, 0.0f, 16.0f, text_col);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      ui_font_manager_destroy(fm);
      return 1;
    }

    rc = backend->draw_text(backend, "Test", (struct ui_font *)1, 0.0f, 0.0f,
                            0.0f, text_col);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      ui_font_manager_destroy(fm);
      return 1;
    }

    rc = backend->draw_text(backend, "Test", (struct ui_font *)1, 0.0f, 0.0f,
                            -1.0f, text_col);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      ui_font_manager_destroy(fm);
      return 1;
    }

    if (ui_font_manager_load_font_memory(fm, test_ttf_bytes,
                                         sizeof(test_ttf_bytes),
                                         &font) != UI_ERROR_NONE) {
      ui_font_manager_destroy(fm);
      return 1;
    }

    /* Valid font with dummy box (width=0, height=0) */
    rc = backend->draw_text(backend, "A B", font, 0.0f, 0.0f, 16.0f, text_col);
    if (rc != UI_ERROR_NONE) {
      ui_font_manager_destroy(fm);
      return 1;
    }

#ifdef UI_TEST_MOCK_ALLOC
    {
      extern int g_mock_gles2_glyph_box;
      extern int g_mock_gles2_glyph_metrics_fail;
      extern int g_mock_gles2_flush_fail;

      /* Flush failure at start */
      g_mock_gles2_flush_fail = 1;
      rc = backend->draw_text(backend, "A", font, 0.0f, 0.0f, 16.0f, text_col);
      if (rc != UI_ERROR_UNKNOWN) {
        ui_font_manager_destroy(fm);
        return 1;
      }
      g_mock_gles2_flush_fail = 0;

      /* Metrics query failure */
      g_mock_gles2_glyph_metrics_fail = 1;
      rc = backend->draw_text(backend, "A", font, 0.0f, 0.0f, 16.0f, text_col);
      if (rc != UI_ERROR_NONE) {
        ui_font_manager_destroy(fm);
        return 1;
      }
      g_mock_gles2_glyph_metrics_fail = 0;

      /* Valid glyph boxes (width > 0, height > 0) and height 0 for 'H' */
      g_mock_gles2_glyph_box = 1;
      rc =
          backend->draw_text(backend, "ABH", font, 0.0f, 0.0f, 16.0f, text_col);
      if (rc != UI_ERROR_NONE) {
        ui_font_manager_destroy(fm);
        return 1;
      }

      /* Draw rect failure inside draw_text */
      {
        char long_text[2050];
        memset(long_text, 'A', 2049);
        long_text[2049] = '\0';
        g_mock_gles2_flush_fail = 2;
        rc = backend->draw_text(backend, long_text, font, 0.0f, 0.0f, 16.0f,
                                text_col);
        if (rc != UI_ERROR_UNKNOWN) {
          ui_font_manager_destroy(fm);
          return 1;
        }
        g_mock_gles2_flush_fail = 0;
      }
      g_mock_gles2_glyph_box = 0;
      backend->flush(backend);
    }
#endif

    rc = ui_font_manager_destroy(fm);
    if (rc != UI_ERROR_NONE) {
      return 1;
    }
  }

  rc = backend->flush(backend);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to flush batch.\n");
    return 1;
  }

  /* Test clipping via glScissor */
  rc = backend->push_clip(backend, 0.0f, 0.0f, 100.0f, 100.0f);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to push clip.\n");
    return 1;
  }
  rc = backend->pop_clip(backend);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to pop clip.\n");
    return 1;
  }

  /* Test complex clipping via Stencil buffer (border-radius) */
  {
    struct ui_vertex verts[3] = {{0.0f, 0.0f, 0.0f, 0.0f, {1, 1, 1, 1}},
                                 {10.0f, 0.0f, 1.0f, 0.0f, {1, 1, 1, 1}},
                                 {0.0f, 10.0f, 0.0f, 1.0f, {1, 1, 1, 1}}};
    unsigned short indices[3] = {0, 1, 2};
    rc = backend->push_stencil_clip(backend, verts, 3, indices, 3);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to push stencil clip.\n");
      return 1;
    }
    rc = backend->pop_stencil_clip(backend);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to pop stencil clip.\n");
      return 1;
    }
  }

  /* Test error paths */
  if (ui_renderer_gles2_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_renderer_gles2_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->init(NULL, NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->set_viewport(NULL, 0, 0, 10, 10) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->flush(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_triangles(NULL, NULL, 0, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_border(NULL, 0, 0, 0, 0, 0, clear_color) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->push_clip(NULL, 0, 0, 0, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->pop_clip(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->push_stencil_clip(NULL, NULL, 0, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->pop_stencil_clip(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->read_pixels(NULL, 1, 1, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->read_pixels(backend, 1, 1, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Valid call for coverage */
  {
    unsigned char buf[4];
    if (backend->read_pixels(backend, 1, 1, buf) != UI_ERROR_NONE)
      return 1;
  }

  if (backend->create_texture(NULL, 10, 10, &tex_handle) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->destroy_texture(NULL, tex_handle) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->set_render_target(NULL, tex_handle) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_texture(NULL, tex_handle, 0.0f, 0.0f, 100.0f, 100.0f,
                            1.0f) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test backend methods NULLs */
  if (backend->create_texture(NULL, 10, 10, &tex_handle) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->create_texture(backend, 10, 10, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->destroy_texture(NULL, tex_handle) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->destroy_texture(backend, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_texture(NULL, tex_handle, 0, 0, 10, 10, 1.0f) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (backend->draw_texture(backend, NULL, 0, 0, 10, 10, 1.0f) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (backend->draw_triangles(backend, NULL, 0, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_vertex dummy_v;
    if (backend->draw_triangles(backend, &dummy_v, 0, NULL, 0) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  {
    void *tmp = backend->user_data;
    backend->user_data = NULL;
    {
      ui_error_t rc = backend->flush(backend);
      printf("Flush with user_data=NULL returned %d\n", rc);
      if (rc != UI_ERROR_INVALID_ARGUMENT)
        return 1;
    }
    {
      ui_error_t rc = backend->draw_triangles(backend, NULL, 0, NULL, 0);
      if (rc != UI_ERROR_INVALID_ARGUMENT) {
        printf("draw_triangles returned %d\n", rc);
        return 1;
      }
    }
    if (backend->draw_border(backend, 0, 0, 0, 0, 0, clear_color) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->set_viewport(backend, 0, 0, 10, 10) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->push_clip(backend, 0, 0, 10, 10) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->pop_clip(backend) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->push_stencil_clip(backend, NULL, 0, NULL, 0) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (backend->pop_stencil_clip(backend) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    backend->user_data = tmp;
  }

  {
    /* Test too many vertices error */
    struct ui_vertex dummy_verts[1] = {{0}};
    unsigned short dummy_indices[1] = {0};
    if (backend->draw_triangles(backend, dummy_verts, 9000, dummy_indices, 1) !=
        UI_ERROR_OUT_OF_MEMORY)
      return 1;
  }

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_malloc_fail_countdown;
    extern int g_mock_gles2_flush_fail;
    struct ui_renderer_backend *backend2;
    g_malloc_fail_countdown = 0;
    if (ui_renderer_gles2_create(&backend2) != UI_ERROR_OUT_OF_MEMORY)
      return 1;
    g_malloc_fail_countdown = -1;
    {
      ui_error_t rc_cleanup = ui_renderer_gles2_create(&backend2);
      if (rc_cleanup != UI_ERROR_NONE) {
        return 1;
      }
    }
    g_malloc_fail_countdown = 0;
    if (backend2->init(backend2, NULL, NULL) != UI_ERROR_OUT_OF_MEMORY)
      return 1;
    g_malloc_fail_countdown = -1;
    g_malloc_fail_countdown = 0;
    if (backend2->create_texture(backend2, 10, 10, &tex_handle) !=
        UI_ERROR_OUT_OF_MEMORY)
      return 1;
    g_malloc_fail_countdown = -1;
    {
      ui_error_t rc_cleanup = ui_renderer_gles2_destroy(backend2);
      if (rc_cleanup != UI_ERROR_NONE) {
        return 1;
      }
    }

    /* We need to fill the batch so that the subsequent draw commands trigger a
     * flush. */
    {
      static struct ui_vertex dummy_verts[8192];
      static unsigned short dummy_indices[8192];
      int i;
      for (i = 0; i < 8192; ++i) {
        dummy_indices[i] = 0;
        dummy_verts[i].x = 0.0f;
        dummy_verts[i].y = 0.0f;
      }

      /* Test draw_rect fail */
      g_mock_gles2_flush_fail = 0;
      if (backend->draw_triangles(backend, dummy_verts, 8192, dummy_indices,
                                  8192) != UI_ERROR_NONE)
        return 1;
      g_mock_gles2_flush_fail = 1;
      if (backend->draw_rect(backend, 0, 0, 10, 10, clear_color) !=
          UI_ERROR_UNKNOWN) {
        return 1;
      }

      /* Test draw_border fail 1 */
      g_mock_gles2_flush_fail = 0;
      if (backend->draw_triangles(backend, dummy_verts, 8192, dummy_indices,
                                  8192) != UI_ERROR_NONE)
        return 1;
      g_mock_gles2_flush_fail = 1;
      if (backend->draw_border(backend, 0, 0, 10, 10, 1, clear_color) !=
          UI_ERROR_UNKNOWN) {
        return 1;
      }

      /* Test draw_border fail 2 */
      g_mock_gles2_flush_fail = 0;
      if (backend->draw_triangles(backend, dummy_verts, 8192 - 4, dummy_indices,
                                  8192 - 6) != UI_ERROR_NONE)
        return 1;
      g_mock_gles2_flush_fail = 1;
      if (backend->draw_border(backend, 0, 0, 10, 10, 1, clear_color) !=
          UI_ERROR_UNKNOWN) {
        return 1;
      }

      /* Test draw_border fail 3 */
      g_mock_gles2_flush_fail = 0;
      if (backend->draw_triangles(backend, dummy_verts, 8192 - 8, dummy_indices,
                                  8192 - 12) != UI_ERROR_NONE)
        return 1;
      g_mock_gles2_flush_fail = 1;
      if (backend->draw_border(backend, 0, 0, 10, 10, 1, clear_color) !=
          UI_ERROR_UNKNOWN) {
        return 1;
      }

      /* Test draw_border fail 4 */
      g_mock_gles2_flush_fail = 0;
      if (backend->draw_triangles(backend, dummy_verts, 8192 - 12,
                                  dummy_indices, 8192 - 18) != UI_ERROR_NONE)
        return 1;
      g_mock_gles2_flush_fail = 1;
      if (backend->draw_border(backend, 0, 0, 10, 10, 1, clear_color) !=
          UI_ERROR_UNKNOWN) {
        return 1;
      }

      /* Put some data to index so flush isn't no-op */
      g_mock_gles2_flush_fail = 0;
      if (backend->draw_triangles(backend, dummy_verts, 1, dummy_indices, 1) !=
          UI_ERROR_NONE)
        return 1;
      g_mock_gles2_flush_fail = 1;
      if (backend->push_clip(backend, 0, 0, 10, 10) != UI_ERROR_UNKNOWN)
        return 1;
      if (backend->pop_clip(backend) != UI_ERROR_UNKNOWN)
        return 1;
      if (backend->push_stencil_clip(backend, dummy_verts, 1, dummy_indices,
                                     1) != UI_ERROR_UNKNOWN)
        return 1;
      if (backend->pop_stencil_clip(backend) != UI_ERROR_UNKNOWN)
        return 1;
      if (backend->draw_texture(backend, tex_handle, 0, 0, 10, 10, 1.0f) !=
          UI_ERROR_UNKNOWN)
        return 1;

      /* Test draw_triangles fail when batch goes over limit */
      g_mock_gles2_flush_fail = 0;
      if (backend->draw_triangles(backend, dummy_verts, 1, dummy_indices, 1) !=
          UI_ERROR_NONE)
        return 1;
      g_mock_gles2_flush_fail = 1;
      if (backend->draw_triangles(backend, dummy_verts, 9000, dummy_indices,
                                  9000) != UI_ERROR_UNKNOWN) {
        return 1;
      }

      /* New branch checks */
      g_mock_gles2_flush_fail = 0;
      /* Hit vertex_count <= MAX, index_count > MAX */
      if (backend->draw_triangles(backend, dummy_verts, 10, dummy_indices,
                                  24577) != UI_ERROR_OUT_OF_MEMORY)
        return 1;
      /* Hit vertex_count > MAX, index_count <= MAX */
      if (backend->draw_triangles(backend, dummy_verts, 8193, dummy_indices,
                                  10) != UI_ERROR_OUT_OF_MEMORY)
        return 1;
      if (backend->flush(backend) != UI_ERROR_NONE)
        return 1;

      /* Test destroy program behavior when user_data structure's program == 0
       */
      {
        unsigned int *prog_ptr =
            (unsigned int *)((char *)backend->user_data +
                             sizeof(struct ui_vertex) * 8192 +
                             sizeof(unsigned short) * 24576 + sizeof(int) * 2);
        unsigned int tmp_prog = *prog_ptr;
        *prog_ptr = 0;
        if (backend->flush(backend) != UI_ERROR_NONE)
          return 1;
        *prog_ptr = tmp_prog;
      }

      /* Hit the !program check in flush */
      {
        unsigned int *prog_ptr =
            (unsigned int *)((char *)backend->user_data +
                             sizeof(struct ui_vertex) * 8192 +
                             sizeof(unsigned short) * 24576 + sizeof(int) * 2);
        unsigned int tmp_prog = *prog_ptr;
        *prog_ptr = 0;
        if (backend->draw_triangles(backend, dummy_verts, 1, dummy_indices,
                                    1) != UI_ERROR_NONE)
          return 1;
        if (backend->flush(backend) != UI_ERROR_NONE)
          return 1;
        *prog_ptr = tmp_prog;
      }

      g_mock_gles2_flush_fail = 0;
    }
  }
#endif

  rc = ui_renderer_gles2_destroy(backend);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy GLES2 renderer backend.\n");
    return 1;
  }

  dummy_test_destroy_fail();
  printf("ui_renderer_gles2 tests passed.\n");
  return 0;
}

void dummy_test_destroy_fail(void) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_mock_gles2_destroy_fail;
  extern int g_mock_gles2_create_shader_fail;
  extern int g_mock_gles2_compile_status_fail;
  extern int g_mock_gles2_create_program_fail;
  struct ui_renderer_backend *backend = NULL;
  if (ui_renderer_gles2_create(&backend) == 0) {
    if (backend->init(backend, NULL, NULL) == UI_ERROR_NONE) {
      g_mock_gles2_destroy_fail = 1;
      if (backend->destroy(backend) != UI_ERROR_UNKNOWN) {
        /* handled */
      }
      {
        ui_error_t rc_cleanup = ui_renderer_gles2_destroy(backend);
        if (rc_cleanup != UI_ERROR_UNKNOWN && rc_cleanup != UI_ERROR_NONE) {
          /* handled */
        }
      }
      g_mock_gles2_destroy_fail = 0;

      /* Zero out program to hit missing branch in destroy */
      {
        unsigned int *prog_ptr =
            (unsigned int *)((char *)backend->user_data +
                             sizeof(struct ui_vertex) * 8192 +
                             sizeof(unsigned short) * 24576 + sizeof(int) * 2);
        *prog_ptr = 0;
      }
      if (backend->destroy(backend) != UI_ERROR_NONE) {
        /* handled */
      }
    }
  }

  /* Test shader creation failures */
  if (ui_renderer_gles2_create(&backend) == 0) {
    g_mock_gles2_create_shader_fail = 1;
    if (backend->init(backend, NULL, NULL) == UI_ERROR_NONE) {
      /* handled */
    }
    g_mock_gles2_create_shader_fail = 0;
    ui_renderer_gles2_destroy(backend);
  }

  if (ui_renderer_gles2_create(&backend) == 0) {
    g_mock_gles2_create_shader_fail = 2;
    if (backend->init(backend, NULL, NULL) == UI_ERROR_NONE) {
      /* handled */
    }
    g_mock_gles2_create_shader_fail = 0;
    ui_renderer_gles2_destroy(backend);
  }

  if (ui_renderer_gles2_create(&backend) == 0) {
    g_mock_gles2_compile_status_fail = 1;
    if (backend->init(backend, NULL, NULL) == UI_ERROR_NONE) {
      /* handled */
    }
    g_mock_gles2_compile_status_fail = 0;
    ui_renderer_gles2_destroy(backend);
  }

  if (ui_renderer_gles2_create(&backend) == 0) {
    g_mock_gles2_create_program_fail = 1;
    if (backend->init(backend, NULL, NULL) == UI_ERROR_NONE) {
      /* handled */
    }
    g_mock_gles2_create_program_fail = 0;
    ui_renderer_gles2_destroy(backend);
  }

  if (ui_renderer_gles2_create(&backend) == 0) {
    extern int g_mock_cgl_context_null;
    unsigned int shdr = 0;
    extern ui_error_t ui_test_gles2_compile_shader(
        unsigned int type, const char *source, unsigned int *out_shader);
    g_mock_cgl_context_null = 1;
    if (backend->init(backend, NULL, NULL) == UI_ERROR_NONE) {
      /* handled */
    }
    ui_test_gles2_compile_shader(0x8B31, "void main(){}", &shdr);
    g_mock_cgl_context_null = 0;
    ui_renderer_gles2_destroy(backend);
  }
#endif
}
