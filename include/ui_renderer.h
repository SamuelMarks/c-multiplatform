#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

struct ui_window_backend;
struct ui_window;
struct ui_font;
struct ui_image;
struct ui_css_image;

/**
 * @brief Represents a basic rectangle used for UI layout and rendering.
 */
struct ui_rect {
  float x;
  float y;
  float width;
  float height;
};

/**
 * @brief Path command types for vector drawing.
 */
enum ui_path_cmd_type {
  UI_PATH_CMD_MOVE_TO,
  UI_PATH_CMD_LINE_TO,
  UI_PATH_CMD_BEZIER_TO,
  UI_PATH_CMD_CLOSE
};

/**
 * @brief Represents a single path command.
 */
struct ui_path_cmd {
  enum ui_path_cmd_type type;
  float x1;
  float y1;
  float x2;
  float y2;
  float x3;
  float y3;
};

/**
 * @brief Path structure for vector drawing.
 */
struct ui_path {
  struct ui_path_cmd *cmds;
  int cmd_count;
  int cmd_capacity;
};

/**
 * @brief RGBA color struct.
 */
struct ui_color {
  float r;
  float g;
  float b;
  float a;
};

/**
 * @brief VTable for the high-level Native-First rendering pipeline.
 */
struct ui_renderer_vtable {
  ui_error_t (*begin_frame)(void *ctx, int width, int height);
  ui_error_t (*end_frame)(void *ctx);
  ui_error_t (*draw_rect)(void *ctx, const struct ui_rect *r,
                          const struct ui_color *c);
  ui_error_t (*draw_text)(void *ctx, const char *text, const struct ui_font *f,
                          const struct ui_rect *r);
  ui_error_t (*draw_image)(void *ctx, const struct ui_image *img,
                           const struct ui_rect *r);
  ui_error_t (*draw_gradient)(void *ctx, const struct ui_rect *r,
                              const struct ui_css_image *gradient);
  ui_error_t (*draw_path)(void *ctx, const struct ui_path *p,
                          const struct ui_color *c);
  ui_error_t (*push_clip)(void *ctx, const struct ui_rect *r);
  ui_error_t (*pop_clip)(void *ctx);
  ui_error_t (*set_blend_mode)(void *ctx, enum ui_css_blend_mode mode);
  ui_error_t (*set_shadow)(void *ctx, const struct ui_css_shadow *shadow);
  ui_error_t (*read_pixels)(void *ctx, unsigned char *out_rgba_buffer);
  ui_error_t (*destroy)(void *ctx);
};

/**
 * @brief The High-Level Renderer Interface.
 */
struct ui_renderer {
  const struct ui_renderer_vtable *vtable;
  void *ctx;
};

/**
 * @brief Initializes the optimal renderer for the platform (Native first,
 * fallback to GLES2).
 */
ui_error_t ui_renderer_create(struct ui_renderer **out_renderer);

/**
 * @brief Initializes the native backend.
 * @return 0 on success, non-zero on failure (e.g. no native backend or init
 * failed).
 */
ui_error_t ui_renderer_native_init(struct ui_renderer *renderer);

/**
 * @brief Initializes the GLES 2.0 fallback backend.
 * @return 0 on success, non-zero on failure.
 */
ui_error_t ui_renderer_gles_fallback_init(struct ui_renderer *renderer);

/**
 * @brief Destroys the given renderer.
 */
ui_error_t ui_renderer_destroy(struct ui_renderer *renderer);

/**
 * @brief Geometry vertex for batching.
 */
struct ui_vertex {
  float x;
  float y;
  float u;
  float v;
  struct ui_color color;
};

/**
 * @brief Abstract vtable for rendering backend.
 */
struct ui_renderer_backend {
  /**
   * @brief Initializes the renderer for the given window.
   */
  ui_error_t (*init)(struct ui_renderer_backend *backend,
                     struct ui_window_backend *window_backend,
                     struct ui_window *window);

  /**
   * @brief Destroys the renderer and associated resources.
   */
  ui_error_t (*destroy)(struct ui_renderer_backend *backend);
  ui_error_t (*push_clip)(struct ui_renderer_backend *backend, float x, float y,
                          float w, float h);
  ui_error_t (*pop_clip)(struct ui_renderer_backend *backend);
  ui_error_t (*push_stencil_clip)(struct ui_renderer_backend *backend,
                                  const struct ui_vertex *vertices,
                                  int vertex_count,
                                  const unsigned short *indices,
                                  int index_count);
  ui_error_t (*pop_stencil_clip)(struct ui_renderer_backend *backend);

  /**
   * @brief Sets the rendering viewport.
   */
  ui_error_t (*set_viewport)(struct ui_renderer_backend *backend, int x, int y,
                             int width, int height);

  /**
   * @brief Clears the screen with the specified color.
   */
  ui_error_t (*clear)(struct ui_renderer_backend *backend,
                      struct ui_color color);

  /**
   * @brief Submits a UI rectangle to the geometry batch.
   */
  ui_error_t (*draw_rect)(struct ui_renderer_backend *backend, float x, float y,
                          float width, float height, struct ui_color color);

  /**
   * @brief Submits a UI border (hollow rectangle) to the geometry batch.
   */
  ui_error_t (*draw_border)(struct ui_renderer_backend *backend, float x,
                            float y, float width, float height, float thickness,
                            struct ui_color color);

  /**
   * @brief Submits arbitrary triangulated geometry (e.g. SVG paths) to the
   * batch.
   */
  ui_error_t (*draw_triangles)(struct ui_renderer_backend *backend,
                               const struct ui_vertex *vertices,
                               int vertex_count, const unsigned short *indices,
                               int index_count);

  /**
   * @brief Flushes the batched geometry to the GPU.
   */
  ui_error_t (*flush)(struct ui_renderer_backend *backend);

  /**
   * @brief Creates an offscreen texture (FBO) for rendering.
   */
  ui_error_t (*create_texture)(struct ui_renderer_backend *backend, int width,
                               int height, void **out_texture_handle);

  /**
   * @brief Destroys an offscreen texture (FBO).
   */
  ui_error_t (*destroy_texture)(struct ui_renderer_backend *backend,
                                void *texture_handle);

  /**
   * @brief Sets the render target to a specific texture (FBO). Use NULL to
   * render to the default screen.
   */
  ui_error_t (*set_render_target)(struct ui_renderer_backend *backend,
                                  void *texture_handle);

  /**
   * @brief Draws a previously rendered offscreen texture to the current render
   * target.
   */
  ui_error_t (*draw_texture)(struct ui_renderer_backend *backend,
                             void *texture_handle, float x, float y,
                             float width, float height, float opacity);

  /**
   * @brief Reads pixels from the current render target.
   */
  ui_error_t (*read_pixels)(struct ui_renderer_backend *backend, int width,
                            int height, unsigned char *out_rgba_buffer);

  void *user_data;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RENDERER_H */
