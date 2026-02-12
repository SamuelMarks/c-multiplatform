#include "m3/m3_render.h"

#ifdef M3_TESTING
static M3Bool g_m3_render_force_reserve = M3_FALSE;
static M3Bool g_m3_render_force_intersect_fail = M3_FALSE;
#endif

#define M3_RENDER_LIST_DEFAULT_CAPACITY 64

typedef struct M3RenderRecorder {
  M3RenderList *list;
  M3Gfx gfx;
} M3RenderRecorder;

static m3_usize m3_usize_max_value(void) { return (m3_usize) ~(m3_usize)0; }

static int m3_render_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = m3_usize_max_value();
  if (a > max_value - b) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return M3_OK;
}

static int m3_render_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = m3_usize_max_value();
  if (a != 0 && b > max_value / a) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return M3_OK;
}

static int m3_render_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_render_list_reserve(M3RenderList *list, m3_usize additional) {
  m3_usize required;
  m3_usize new_capacity;
  m3_usize bytes;
  void *mem;
  int rc;

  if (list == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_render_add_overflow(list->count, additional, &required);
  if (rc != M3_OK) {
    return rc;
  }

#ifdef M3_TESTING
  if (!g_m3_render_force_reserve && required <= list->capacity) {
    return M3_OK;
  }
#else
  if (required <= list->capacity) {
    return M3_OK;
  }
#endif

  if (list->capacity == 0) {
    new_capacity = M3_RENDER_LIST_DEFAULT_CAPACITY;
  } else if (list->capacity > m3_usize_max_value() / 2) {
    if (list->capacity < required) {
      return M3_ERR_OVERFLOW;
    }
    new_capacity = list->capacity;
  } else {
    new_capacity = list->capacity * 2;
  }

  if (new_capacity < required) {
    new_capacity = required;
  }

  rc = m3_render_mul_overflow(new_capacity, (m3_usize)sizeof(M3RenderCmd),
                              &bytes);
  if (rc != M3_OK) {
    return rc;
  }

  rc =
      list->allocator.realloc(list->allocator.ctx, list->commands, bytes, &mem);
  if (rc != M3_OK) {
    return rc;
  }

  list->commands = (M3RenderCmd *)mem;
  list->capacity = new_capacity;
  return M3_OK;
}

static int m3_render_record_begin_frame(void *gfx, M3Handle window,
                                        m3_i32 width, m3_i32 height,
                                        M3Scalar dpi_scale) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    return M3_ERR_RANGE;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_BEGIN_FRAME;
  cmd.data.begin_frame.window = window;
  cmd.data.begin_frame.width = width;
  cmd.data.begin_frame.height = height;
  cmd.data.begin_frame.dpi_scale = dpi_scale;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_end_frame(void *gfx, M3Handle window) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_END_FRAME;
  cmd.data.end_frame.window = window;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_clear(void *gfx, M3Color color) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_CLEAR;
  cmd.data.clear.color = color;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_draw_rect(void *gfx, const M3Rect *rect,
                                      M3Color color, M3Scalar corner_radius) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_render_validate_rect(rect);
  if (rc != M3_OK) {
    return rc;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_DRAW_RECT;
  cmd.data.draw_rect.rect = *rect;
  cmd.data.draw_rect.color = color;
  cmd.data.draw_rect.corner_radius = corner_radius;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_draw_line(void *gfx, M3Scalar x0, M3Scalar y0,
                                      M3Scalar x1, M3Scalar y1, M3Color color,
                                      M3Scalar thickness) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return M3_ERR_RANGE;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_DRAW_LINE;
  cmd.data.draw_line.x0 = x0;
  cmd.data.draw_line.y0 = y0;
  cmd.data.draw_line.x1 = x1;
  cmd.data.draw_line.y1 = y1;
  cmd.data.draw_line.color = color;
  cmd.data.draw_line.thickness = thickness;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_draw_path(void *gfx, const M3Path *path,
                                      M3Color color) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;

  if (gfx == NULL || path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return M3_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return M3_ERR_STATE;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_DRAW_PATH;
  cmd.data.draw_path.path = path;
  cmd.data.draw_path.color = color;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_push_clip(void *gfx, const M3Rect *rect) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_render_validate_rect(rect);
  if (rc != M3_OK) {
    return rc;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_PUSH_CLIP;
  cmd.data.push_clip.rect = *rect;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_pop_clip(void *gfx) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_POP_CLIP;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_set_transform(void *gfx, const M3Mat3 *transform) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;

  if (gfx == NULL || transform == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_SET_TRANSFORM;
  cmd.data.set_transform.transform = *transform;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_create_texture(void *gfx, m3_i32 width,
                                           m3_i32 height, m3_u32 format,
                                           const void *pixels, m3_usize size,
                                           M3Handle *out_texture) {
  M3_UNUSED(width);
  M3_UNUSED(height);
  M3_UNUSED(format);
  M3_UNUSED(pixels);
  M3_UNUSED(size);

  if (gfx == NULL || out_texture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  return M3_ERR_UNSUPPORTED;
}

static int m3_render_record_update_texture(void *gfx, M3Handle texture,
                                           m3_i32 x, m3_i32 y, m3_i32 width,
                                           m3_i32 height, const void *pixels,
                                           m3_usize size) {
  M3_UNUSED(texture);
  M3_UNUSED(x);
  M3_UNUSED(y);
  M3_UNUSED(width);
  M3_UNUSED(height);
  M3_UNUSED(pixels);
  M3_UNUSED(size);

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  return M3_ERR_UNSUPPORTED;
}

static int m3_render_record_destroy_texture(void *gfx, M3Handle texture) {
  M3_UNUSED(texture);

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  return M3_ERR_UNSUPPORTED;
}

static int m3_render_record_draw_texture(void *gfx, M3Handle texture,
                                         const M3Rect *src, const M3Rect *dst,
                                         M3Scalar opacity) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_render_validate_rect(src);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_render_validate_rect(dst);
  if (rc != M3_OK) {
    return rc;
  }

  recorder = (M3RenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_DRAW_TEXTURE;
  cmd.data.draw_texture.texture = texture;
  cmd.data.draw_texture.src = *src;
  cmd.data.draw_texture.dst = *dst;
  cmd.data.draw_texture.opacity = opacity;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_draw_text(void *text, M3Handle font,
                                      const char *utf8, m3_usize utf8_len,
                                      M3Scalar x, M3Scalar y, M3Color color) {
  M3RenderRecorder *recorder;
  M3RenderCmd cmd;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len > 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  recorder = (M3RenderRecorder *)text;
  if (recorder->list == NULL) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_RENDER_CMD_DRAW_TEXT;
  cmd.data.draw_text.font = font;
  cmd.data.draw_text.utf8 = utf8;
  cmd.data.draw_text.utf8_len = utf8_len;
  cmd.data.draw_text.x = x;
  cmd.data.draw_text.y = y;
  cmd.data.draw_text.color = color;
  return m3_render_list_append(recorder->list, &cmd);
}

static int m3_render_record_create_font(void *text, const char *utf8_family,
                                        m3_i32 size_px, m3_i32 weight,
                                        M3Bool italic, M3Handle *out_font) {
  M3_UNUSED(utf8_family);
  M3_UNUSED(size_px);
  M3_UNUSED(weight);
  M3_UNUSED(italic);

  if (text == NULL || out_font == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  return M3_ERR_UNSUPPORTED;
}

static int m3_render_record_destroy_font(void *text, M3Handle font) {
  M3_UNUSED(font);

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  return M3_ERR_UNSUPPORTED;
}

static int m3_render_record_measure_text(void *text, M3Handle font,
                                         const char *utf8, m3_usize utf8_len,
                                         M3Scalar *out_width,
                                         M3Scalar *out_height,
                                         M3Scalar *out_baseline) {
  M3_UNUSED(font);
  M3_UNUSED(utf8);
  M3_UNUSED(utf8_len);

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  return M3_ERR_UNSUPPORTED;
}

static const M3GfxVTable g_m3_render_record_vtable = {
    m3_render_record_begin_frame,    m3_render_record_end_frame,
    m3_render_record_clear,          m3_render_record_draw_rect,
    m3_render_record_draw_line,      m3_render_record_draw_path,
    m3_render_record_push_clip,      m3_render_record_pop_clip,
    m3_render_record_set_transform,  m3_render_record_create_texture,
    m3_render_record_update_texture, m3_render_record_destroy_texture,
    m3_render_record_draw_texture};

static const M3TextVTable g_m3_render_record_text_vtable = {
    m3_render_record_create_font, m3_render_record_destroy_font,
    m3_render_record_measure_text, m3_render_record_draw_text};

static int m3_render_validate_node(const M3RenderNode *node) {
  m3_usize i;
  int rc;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (node->widget == NULL || node->widget->vtable == NULL ||
      node->widget->vtable->paint == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_render_validate_rect(&node->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  if (node->child_count > 0 && node->children == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < node->child_count; ++i) {
    if (node->children[i] == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }

  return M3_OK;
}

static int m3_render_build_node(const M3RenderNode *node,
                                const M3Rect *parent_clip, M3Scalar dpi_scale,
                                M3RenderRecorder *recorder) {
  M3Rect clip;
  M3Bool has_intersection;
  M3PaintContext ctx;
  m3_usize i;
  int rc;
  int first_error;

  rc = m3_render_validate_node(node);
  if (rc != M3_OK) {
    return rc;
  }

  if (node->widget->flags & M3_WIDGET_FLAG_HIDDEN) {
    return M3_OK;
  }

  if (parent_clip == NULL) {
    clip = node->bounds;
    has_intersection =
        (clip.width > 0.0f && clip.height > 0.0f) ? M3_TRUE : M3_FALSE;
  } else {
    rc =
        m3_rect_intersect(parent_clip, &node->bounds, &clip, &has_intersection);
#ifdef M3_TESTING
    if (g_m3_render_force_intersect_fail) {
      rc = M3_ERR_RANGE;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (!has_intersection) {
    return M3_OK;
  }

  rc = recorder->gfx.vtable->push_clip(recorder->gfx.ctx, &clip);
  if (rc != M3_OK) {
    return rc;
  }

  ctx.gfx = &recorder->gfx;
  ctx.clip = clip;
  ctx.dpi_scale = dpi_scale;

  first_error = node->widget->vtable->paint(node->widget->ctx, &ctx);
  if (first_error == M3_OK) {
    for (i = 0; i < node->child_count; ++i) {
      rc = m3_render_build_node(node->children[i], &clip, dpi_scale, recorder);
      if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
      }
    }
  }

  rc = recorder->gfx.vtable->pop_clip(recorder->gfx.ctx);
  if (first_error == M3_OK && rc != M3_OK) {
    first_error = rc;
  }

  return first_error;
}

int M3_CALL m3_render_list_init(M3RenderList *list,
                                const M3Allocator *allocator,
                                m3_usize initial_capacity) {
  M3Allocator chosen;
  m3_usize bytes;
  void *mem;
  int rc;

  if (list == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (list->commands != NULL || list->capacity != 0 || list->count != 0) {
    return M3_ERR_STATE;
  }

  if (allocator == NULL) {
    rc = m3_get_default_allocator(&chosen);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    chosen = *allocator;
  }

  if (chosen.alloc == NULL || chosen.realloc == NULL || chosen.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (initial_capacity == 0) {
    initial_capacity = M3_RENDER_LIST_DEFAULT_CAPACITY;
  }

  rc = m3_render_mul_overflow(initial_capacity, (m3_usize)sizeof(M3RenderCmd),
                              &bytes);
  if (rc != M3_OK) {
    return rc;
  }

  rc = chosen.alloc(chosen.ctx, bytes, &mem);
  if (rc != M3_OK) {
    return rc;
  }

  list->allocator = chosen;
  list->commands = (M3RenderCmd *)mem;
  list->count = 0;
  list->capacity = initial_capacity;
  return M3_OK;
}

int M3_CALL m3_render_list_reset(M3RenderList *list) {
  if (list == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (list->commands == NULL) {
    return M3_ERR_STATE;
  }

  list->count = 0;
  return M3_OK;
}

int M3_CALL m3_render_list_shutdown(M3RenderList *list) {
  int rc;

  if (list == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (list->commands == NULL) {
    return M3_ERR_STATE;
  }

  rc = list->allocator.free(list->allocator.ctx, list->commands);

  list->commands = NULL;
  list->count = 0;
  list->capacity = 0;
  list->allocator.ctx = NULL;
  list->allocator.alloc = NULL;
  list->allocator.realloc = NULL;
  list->allocator.free = NULL;

  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_render_list_append(M3RenderList *list, const M3RenderCmd *cmd) {
  int rc;

  if (list == NULL || cmd == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (list->commands == NULL) {
    return M3_ERR_STATE;
  }
  if (list->count > list->capacity) {
    return M3_ERR_STATE;
  }

  rc = m3_render_list_reserve(list, 1);
  if (rc != M3_OK) {
    return rc;
  }

  list->commands[list->count] = *cmd;
  list->count += 1;
  return M3_OK;
}

int M3_CALL m3_render_list_execute(const M3RenderList *list, M3Gfx *gfx) {
  m3_usize i;
  int rc;

  if (list == NULL || gfx == NULL || gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (list->commands == NULL) {
    return M3_ERR_STATE;
  }

  for (i = 0; i < list->count; ++i) {
    const M3RenderCmd *cmd = &list->commands[i];

    switch (cmd->type) {
    case M3_RENDER_CMD_BEGIN_FRAME:
      if (gfx->vtable->begin_frame == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->begin_frame(
          gfx->ctx, cmd->data.begin_frame.window, cmd->data.begin_frame.width,
          cmd->data.begin_frame.height, cmd->data.begin_frame.dpi_scale);
      break;
    case M3_RENDER_CMD_END_FRAME:
      if (gfx->vtable->end_frame == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->end_frame(gfx->ctx, cmd->data.end_frame.window);
      break;
    case M3_RENDER_CMD_CLEAR:
      if (gfx->vtable->clear == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->clear(gfx->ctx, cmd->data.clear.color);
      break;
    case M3_RENDER_CMD_DRAW_RECT:
      if (gfx->vtable->draw_rect == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->draw_rect(gfx->ctx, &cmd->data.draw_rect.rect,
                                  cmd->data.draw_rect.color,
                                  cmd->data.draw_rect.corner_radius);
      break;
    case M3_RENDER_CMD_DRAW_LINE:
      if (gfx->vtable->draw_line == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->draw_line(
          gfx->ctx, cmd->data.draw_line.x0, cmd->data.draw_line.y0,
          cmd->data.draw_line.x1, cmd->data.draw_line.y1,
          cmd->data.draw_line.color, cmd->data.draw_line.thickness);
      break;
    case M3_RENDER_CMD_DRAW_PATH:
      if (gfx->vtable->draw_path == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->draw_path(gfx->ctx, cmd->data.draw_path.path,
                                  cmd->data.draw_path.color);
      break;
    case M3_RENDER_CMD_PUSH_CLIP:
      if (gfx->vtable->push_clip == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->push_clip(gfx->ctx, &cmd->data.push_clip.rect);
      break;
    case M3_RENDER_CMD_POP_CLIP:
      if (gfx->vtable->pop_clip == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->pop_clip(gfx->ctx);
      break;
    case M3_RENDER_CMD_SET_TRANSFORM:
      if (gfx->vtable->set_transform == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->set_transform(gfx->ctx,
                                      &cmd->data.set_transform.transform);
      break;
    case M3_RENDER_CMD_DRAW_TEXTURE:
      if (gfx->vtable->draw_texture == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->draw_texture(
          gfx->ctx, cmd->data.draw_texture.texture, &cmd->data.draw_texture.src,
          &cmd->data.draw_texture.dst, cmd->data.draw_texture.opacity);
      break;
    case M3_RENDER_CMD_DRAW_TEXT:
      if (gfx->text_vtable == NULL || gfx->text_vtable->draw_text == NULL) {
        return M3_ERR_UNSUPPORTED;
      }
      rc = gfx->text_vtable->draw_text(
          gfx->ctx, cmd->data.draw_text.font, cmd->data.draw_text.utf8,
          cmd->data.draw_text.utf8_len, cmd->data.draw_text.x,
          cmd->data.draw_text.y, cmd->data.draw_text.color);
      break;
    default:
      return M3_ERR_INVALID_ARGUMENT;
    }

    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

int M3_CALL m3_render_node_init(M3RenderNode *node, M3Widget *widget,
                                const M3Rect *bounds) {
  int rc;

  if (node == NULL || widget == NULL || bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_render_validate_rect(bounds);
  if (rc != M3_OK) {
    return rc;
  }

  node->widget = widget;
  node->children = NULL;
  node->child_count = 0;
  node->bounds = *bounds;
  return M3_OK;
}

int M3_CALL m3_render_node_set_bounds(M3RenderNode *node,
                                      const M3Rect *bounds) {
  int rc;

  if (node == NULL || bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_render_validate_rect(bounds);
  if (rc != M3_OK) {
    return rc;
  }

  node->bounds = *bounds;
  return M3_OK;
}

int M3_CALL m3_render_node_set_children(M3RenderNode *node,
                                        M3RenderNode **children,
                                        m3_usize count) {
  m3_usize i;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (count > 0 && children == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < count; ++i) {
    if (children[i] == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }

  node->children = children;
  node->child_count = count;
  return M3_OK;
}

int M3_CALL m3_render_build(const M3RenderNode *root, M3RenderList *list,
                            M3Scalar dpi_scale) {
  M3RenderRecorder recorder;
  m3_usize start_count;
  int rc;

  if (root == NULL || list == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (dpi_scale <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (list->commands == NULL) {
    return M3_ERR_STATE;
  }

  recorder.list = list;
  recorder.gfx.ctx = &recorder;
  recorder.gfx.vtable = &g_m3_render_record_vtable;
  recorder.gfx.text_vtable = &g_m3_render_record_text_vtable;

  start_count = list->count;
  rc = m3_render_build_node(root, NULL, dpi_scale, &recorder);
  if (rc != M3_OK) {
    list->count = start_count;
  }
  return rc;
}

#ifdef M3_TESTING
int M3_CALL m3_render_test_add_overflow(m3_usize a, m3_usize b,
                                        m3_usize *out_value) {
  return m3_render_add_overflow(a, b, out_value);
}

int M3_CALL m3_render_test_mul_overflow(m3_usize a, m3_usize b,
                                        m3_usize *out_value) {
  return m3_render_mul_overflow(a, b, out_value);
}

int M3_CALL m3_render_test_list_reserve(M3RenderList *list,
                                        m3_usize additional) {
  return m3_render_list_reserve(list, additional);
}

int M3_CALL m3_render_test_validate_node(const M3RenderNode *node) {
  return m3_render_validate_node(node);
}

int M3_CALL m3_render_test_set_force_reserve(M3Bool enable) {
  g_m3_render_force_reserve = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_render_test_set_force_intersect_fail(M3Bool enable) {
  g_m3_render_force_intersect_fail = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif
