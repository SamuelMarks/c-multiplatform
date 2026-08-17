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
 * @struct ui_rect
 * @brief Represents a basic rectangle used for UI layout and rendering.
 */
struct ui_rect {
  /** @brief X coordinate of the top-left corner. */
  float x;
  /** @brief Y coordinate of the top-left corner. */
  float y;
  /** @brief Width of the rectangle. */
  float width;
  /** @brief Height of the rectangle. */
  float height;
};

/**
 * @enum ui_path_cmd_type
 * @brief Path command types for vector drawing.
 */
enum ui_path_cmd_type {
  /** @brief Move to a new coordinate without drawing. */
  UI_PATH_CMD_MOVE_TO,
  /** @brief Draw a line to a new coordinate. */
  UI_PATH_CMD_LINE_TO,
  /** @brief Draw a cubic bezier curve to a new coordinate. */
  UI_PATH_CMD_BEZIER_TO,
  /** @brief Close the current path. */
  UI_PATH_CMD_CLOSE
};

/**
 * @struct ui_path_cmd
 * @brief Represents a single path command.
 */
struct ui_path_cmd {
  /** @brief The type of path command. */
  enum ui_path_cmd_type type;
  /** @brief First X coordinate parameter. */
  float x1;
  /** @brief First Y coordinate parameter. */
  float y1;
  /** @brief Second X coordinate parameter. */
  float x2;
  /** @brief Second Y coordinate parameter. */
  float y2;
  /** @brief Third X coordinate parameter. */
  float x3;
  /** @brief Third Y coordinate parameter. */
  float y3;
};

/**
 * @struct ui_path
 * @brief Path structure for vector drawing.
 */
struct ui_path {
  /** @brief Array of path commands. */
  struct ui_path_cmd *cmds;
  /** @brief Number of active commands in the array. */
  int cmd_count;
  /** @brief Total allocated capacity of the array. */
  int cmd_capacity;
};

/**
 * @struct ui_color
 * @brief RGBA color struct.
 */
struct ui_color {
  /** @brief Red channel (0.0 to 1.0). */
  float r;
  /** @brief Green channel (0.0 to 1.0). */
  float g;
  /** @brief Blue channel (0.0 to 1.0). */
  float b;
  /** @brief Alpha channel (0.0 to 1.0). */
  float a;
};

/**
 * @struct ui_renderer_vtable
 * @brief VTable for the high-level Native-First rendering pipeline.
 */
struct ui_renderer_vtable {
  /**
   * @brief Begins rendering a frame.
   *
   * @param ctx The renderer context.
   * @param width Frame width.
   * @param height Frame height.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*begin_frame)(void *ctx, int width, int height);

  /**
   * @brief Ends rendering a frame.
   *
   * @param ctx The renderer context.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*end_frame)(void *ctx);

  /**
   * @brief Draws a solid rectangle.
   *
   * @param ctx The renderer context.
   * @param r The rectangle coordinates.
   * @param c The fill color.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_rect)(void *ctx, const struct ui_rect *r,
                          const struct ui_color *c);

  /**
   * @brief Draws text.
   *
   * @param ctx The renderer context.
   * @param text The null-terminated text to draw.
   * @param f The font.
   * @param r The bounding rectangle.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_text)(void *ctx, const char *text, const struct ui_font *f,
                          const struct ui_rect *r);

  /**
   * @brief Draws an image.
   *
   * @param ctx The renderer context.
   * @param img The image handle.
   * @param r The bounding rectangle.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_image)(void *ctx, const struct ui_image *img,
                           const struct ui_rect *r);

  /**
   * @brief Draws a gradient.
   *
   * @param ctx The renderer context.
   * @param r The bounding rectangle.
   * @param gradient The CSS gradient definition.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_gradient)(void *ctx, const struct ui_rect *r,
                              const struct ui_css_image *gradient);

  /**
   * @brief Draws a vector path.
   *
   * @param ctx The renderer context.
   * @param p The path data.
   * @param c The fill color.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_path)(void *ctx, const struct ui_path *p,
                          const struct ui_color *c);

  /**
   * @brief Pushes a clipping rectangle onto the stack.
   *
   * @param ctx The renderer context.
   * @param r The clipping rectangle.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*push_clip)(void *ctx, const struct ui_rect *r);

  /**
   * @brief Pops a clipping rectangle from the stack.
   *
   * @param ctx The renderer context.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*pop_clip)(void *ctx);

  /**
   * @brief Sets the CSS blend mode for subsequent draw calls.
   *
   * @param ctx The renderer context.
   * @param mode The blend mode.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*set_blend_mode)(void *ctx, enum ui_css_blend_mode mode);

  /**
   * @brief Sets the CSS shadow configuration for subsequent draw calls.
   *
   * @param ctx The renderer context.
   * @param shadow The shadow definition.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*set_shadow)(void *ctx, const struct ui_css_shadow *shadow);

  /**
   * @brief Reads pixels from the current render target.
   *
   * @param ctx The renderer context.
   * @param out_rgba_buffer Buffer to receive RGBA pixel data.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*read_pixels)(void *ctx, unsigned char *out_rgba_buffer);

  /**
   * @brief Destroys the renderer backend.
   *
   * @param ctx The renderer context.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*destroy)(void *ctx);
};

/**
 * @struct ui_renderer
 * @brief The High-Level Renderer Interface.
 */
struct ui_renderer {
  /** @brief Pointer to the virtual function table for the renderer. */
  const struct ui_renderer_vtable *vtable;
  /** @brief Opaque pointer to renderer-specific context data. */
  void *ctx;
};

/**
 * @brief Initializes the optimal renderer for the platform (Native first,
 * fallback to GLES2).
 *
 * @param out_renderer Pointer to receive the allocated and initialized
 * renderer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_create(struct ui_renderer **out_renderer);

/**
 * @brief Initializes the native backend.
 *
 * @param renderer The renderer to initialize natively.
 * @return UI_ERROR_NONE on success, or an appropriate error code (e.g. no
 * native backend or init failed).
 */
ui_error_t ui_renderer_native_init(struct ui_renderer *renderer);

/**
 * @brief Initializes the GLES 2.0 fallback backend.
 *
 * @param renderer The renderer to initialize with GLES2.
 * @return UI_ERROR_NONE on success, or an appropriate error code on failure.
 */
ui_error_t ui_renderer_gles_fallback_init(struct ui_renderer *renderer);

/**
 * @brief Destroys the given renderer.
 *
 * @param renderer The renderer to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_destroy(struct ui_renderer *renderer);

/**
 * @struct ui_vertex
 * @brief Geometry vertex for batching.
 */
struct ui_vertex {
  /** @brief X coordinate of the vertex. */
  float x;
  /** @brief Y coordinate of the vertex. */
  float y;
  /** @brief U texture coordinate. */
  float u;
  /** @brief V texture coordinate. */
  float v;
  /** @brief Color of the vertex. */
  struct ui_color color;
};

/**
 * @struct ui_renderer_backend
 * @brief Abstract vtable for rendering backend.
 */
struct ui_renderer_backend {
  /**
   * @brief Initializes the renderer for the given window.
   *
   * @param backend The renderer backend.
   * @param window_backend The window backend.
   * @param window The window instance.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*init)(struct ui_renderer_backend *backend,
                     struct ui_window_backend *window_backend,
                     struct ui_window *window);

  /**
   * @brief Destroys the renderer and associated resources.
   *
   * @param backend The renderer backend.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*destroy)(struct ui_renderer_backend *backend);

  /**
   * @brief Pushes a clipping rect.
   *
   * @param backend The renderer backend.
   * @param x X origin.
   * @param y Y origin.
   * @param w Width.
   * @param h Height.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*push_clip)(struct ui_renderer_backend *backend, float x, float y,
                          float w, float h);

  /**
   * @brief Pops a clipping rect.
   *
   * @param backend The renderer backend.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*pop_clip)(struct ui_renderer_backend *backend);

  /**
   * @brief Pushes a stencil clip from triangulated geometry.
   *
   * @param backend The renderer backend.
   * @param vertices Array of vertices defining the stencil.
   * @param vertex_count Number of vertices.
   * @param indices Array of indices defining triangles.
   * @param index_count Number of indices.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*push_stencil_clip)(struct ui_renderer_backend *backend,
                                  const struct ui_vertex *vertices,
                                  int vertex_count,
                                  const unsigned short *indices,
                                  int index_count);

  /**
   * @brief Pops a stencil clip.
   *
   * @param backend The renderer backend.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*pop_stencil_clip)(struct ui_renderer_backend *backend);

  /**
   * @brief Sets the rendering viewport.
   *
   * @param backend The renderer backend.
   * @param x X origin.
   * @param y Y origin.
   * @param width Viewport width.
   * @param height Viewport height.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*set_viewport)(struct ui_renderer_backend *backend, int x, int y,
                             int width, int height);

  /**
   * @brief Clears the screen with the specified color.
   *
   * @param backend The renderer backend.
   * @param color The clear color.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*clear)(struct ui_renderer_backend *backend,
                      struct ui_color color);

  /**
   * @brief Submits a UI rectangle to the geometry batch.
   *
   * @param backend The renderer backend.
   * @param x X origin.
   * @param y Y origin.
   * @param width Rectangle width.
   * @param height Rectangle height.
   * @param color Fill color.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_rect)(struct ui_renderer_backend *backend, float x, float y,
                          float width, float height, struct ui_color color);

  /**
   * @brief Submits a UI border (hollow rectangle) to the geometry batch.
   *
   * @param backend The renderer backend.
   * @param x X origin.
   * @param y Y origin.
   * @param width Rectangle width.
   * @param height Rectangle height.
   * @param thickness Border thickness.
   * @param color Border color.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_border)(struct ui_renderer_backend *backend, float x,
                            float y, float width, float height, float thickness,
                            struct ui_color color);

  /**
   * @brief Submits arbitrary triangulated geometry (e.g. SVG paths) to the
   * batch.
   *
   * @param backend The renderer backend.
   * @param vertices Array of vertices.
   * @param vertex_count Number of vertices.
   * @param indices Array of indices defining triangles.
   * @param index_count Number of indices.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_triangles)(struct ui_renderer_backend *backend,
                               const struct ui_vertex *vertices,
                               int vertex_count, const unsigned short *indices,
                               int index_count);

  /**
   * @brief Flushes the batched geometry to the GPU.
   *
   * @param backend The renderer backend.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*flush)(struct ui_renderer_backend *backend);

  /**
   * @brief Creates an offscreen texture (FBO) for rendering.
   *
   * @param backend The renderer backend.
   * @param width Texture width.
   * @param height Texture height.
   * @param out_texture_handle Pointer to receive the texture handle.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*create_texture)(struct ui_renderer_backend *backend, int width,
                               int height, void **out_texture_handle);

  /**
   * @brief Destroys an offscreen texture (FBO).
   *
   * @param backend The renderer backend.
   * @param texture_handle The texture handle to destroy.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*destroy_texture)(struct ui_renderer_backend *backend,
                                void *texture_handle);

  /**
   * @brief Sets the render target to a specific texture (FBO). Use NULL to
   * render to the default screen.
   *
   * @param backend The renderer backend.
   * @param texture_handle The texture handle, or NULL for screen.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*set_render_target)(struct ui_renderer_backend *backend,
                                  void *texture_handle);

  /**
   * @brief Draws a previously rendered offscreen texture to the current render
   * target.
   *
   * @param backend The renderer backend.
   * @param texture_handle The texture to draw.
   * @param x Dest X origin.
   * @param y Dest Y origin.
   * @param width Dest width.
   * @param height Dest height.
   * @param opacity Alpha opacity modifier.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*draw_texture)(struct ui_renderer_backend *backend,
                             void *texture_handle, float x, float y,
                             float width, float height, float opacity);

  /**
   * @brief Reads pixels from the current render target.
   *
   * @param backend The renderer backend.
   * @param width Width of region to read.
   * @param height Height of region to read.
   * @param out_rgba_buffer Pre-allocated buffer to receive RGBA pixels.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*read_pixels)(struct ui_renderer_backend *backend, int width,
                            int height, unsigned char *out_rgba_buffer);

  /** @brief Opaque user data for backend internal use. */
  void *user_data;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RENDERER_H */
