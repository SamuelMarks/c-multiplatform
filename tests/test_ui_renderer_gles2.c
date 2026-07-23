/* clang-format off */
#include <stdio.h>
#include "../include/ui_renderer_gles2.h"
#include "../include/ui_error.h"
#include "../include/ui_window_backend.h"
/* clang-format on */

int main(void) {
  struct ui_renderer_backend *backend = NULL;
  enum ui_error rc;
  struct ui_color clear_color = {1.0f, 0.0f, 0.0f, 1.0f};
  void *tex_handle = NULL;

  rc = ui_renderer_gles2_create(&backend);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create GLES2 renderer backend.\n");
    return 1;
  }

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
    struct ui_renderer_backend *backend2;
    g_malloc_fail_countdown = 0;
    if (ui_renderer_gles2_create(&backend2) != UI_ERROR_OUT_OF_MEMORY)
      return 1;
    g_malloc_fail_countdown = -1;
    ui_renderer_gles2_create(&backend2);
    g_malloc_fail_countdown = 0;
    if (backend2->init(backend2, NULL, NULL) != UI_ERROR_OUT_OF_MEMORY)
      return 1;
    g_malloc_fail_countdown = -1;
    g_malloc_fail_countdown = 0;
    if (backend2->create_texture(backend2, 10, 10, &tex_handle) !=
        UI_ERROR_OUT_OF_MEMORY)
      return 1;
    g_malloc_fail_countdown = -1;
    ui_renderer_gles2_destroy(backend2);
  }
#endif

  rc = ui_renderer_gles2_destroy(backend);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy GLES2 renderer backend.\n");
    return 1;
  }

  printf("ui_renderer_gles2 tests passed.\n");
  return 0;
}
