#include "cmpc/cmp_render.h"

#ifdef CMP_TESTING
static CMPBool g_cmp_render_force_reserve = CMP_FALSE;
static CMPBool g_cmp_render_force_intersect_fail = CMP_FALSE;
#endif

#define CMP_RENDER_LIST_DEFAULT_CAPACITY 64

typedef struct CMPRenderRecorder {
  CMPRenderList *list;
  CMPGfx gfx;
} CMPRenderRecorder;

static cmp_usize cmp_usize_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static int cmp_render_add_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = cmp_usize_max_value();
  if (a > max_value - b) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return CMP_OK;
}

static int cmp_render_mul_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = cmp_usize_max_value();
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}

static int cmp_render_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_render_list_reserve(CMPRenderList *list, cmp_usize additional) {
  cmp_usize required;
  cmp_usize new_capacity;
  cmp_usize bytes;
  void *mem;
  int rc;

  if (list == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_render_add_overflow(list->count, additional, &required);
  if (rc != CMP_OK) {
    return rc;
  }

#ifdef CMP_TESTING
  if (!g_cmp_render_force_reserve && required <= list->capacity) {
    return CMP_OK;
  }
#else
  if (required <= list->capacity) {
    return CMP_OK;
  }
#endif

  if (list->capacity == 0) {
    new_capacity = CMP_RENDER_LIST_DEFAULT_CAPACITY;
  } else if (list->capacity > cmp_usize_max_value() / 2) {
    if (list->capacity < required) {
      return CMP_ERR_OVERFLOW;
    }
    new_capacity = list->capacity;
  } else {
    new_capacity = list->capacity * 2;
  }

  if (new_capacity < required) {
    new_capacity = required;
  }

  rc = cmp_render_mul_overflow(new_capacity, (cmp_usize)sizeof(CMPRenderCmd),
                              &bytes);
  if (rc != CMP_OK) {
    return rc;
  }

  rc =
      list->allocator.realloc(list->allocator.ctx, list->commands, bytes, &mem);
  if (rc != CMP_OK) {
    return rc;
  }

  list->commands = (CMPRenderCmd *)mem;
  list->capacity = new_capacity;
  return CMP_OK;
}

static int cmp_render_record_begin_frame(void *gfx, CMPHandle window,
                                        cmp_i32 width, cmp_i32 height,
                                        CMPScalar dpi_scale) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_BEGIN_FRAME;
  cmd.data.begin_frame.window = window;
  cmd.data.begin_frame.width = width;
  cmd.data.begin_frame.height = height;
  cmd.data.begin_frame.dpi_scale = dpi_scale;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_end_frame(void *gfx, CMPHandle window) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_END_FRAME;
  cmd.data.end_frame.window = window;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_clear(void *gfx, CMPColor color) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_CLEAR;
  cmd.data.clear.color = color;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_draw_rect(void *gfx, const CMPRect *rect,
                                      CMPColor color, CMPScalar corner_radius) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_render_validate_rect(rect);
  if (rc != CMP_OK) {
    return rc;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_DRAW_RECT;
  cmd.data.draw_rect.rect = *rect;
  cmd.data.draw_rect.color = color;
  cmd.data.draw_rect.corner_radius = corner_radius;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                                      CMPScalar x1, CMPScalar y1, CMPColor color,
                                      CMPScalar thickness) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return CMP_ERR_RANGE;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_DRAW_LINE;
  cmd.data.draw_line.x0 = x0;
  cmd.data.draw_line.y0 = y0;
  cmd.data.draw_line.x1 = x1;
  cmd.data.draw_line.y1 = y1;
  cmd.data.draw_line.color = color;
  cmd.data.draw_line.thickness = thickness;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_draw_path(void *gfx, const CMPPath *path,
                                      CMPColor color) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;

  if (gfx == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return CMP_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return CMP_ERR_STATE;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_DRAW_PATH;
  cmd.data.draw_path.path = path;
  cmd.data.draw_path.color = color;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_push_clip(void *gfx, const CMPRect *rect) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;
  int rc; /* GCOVR_EXCL_LINE */

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_render_validate_rect(rect);
  if (rc != CMP_OK) {
    return rc;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_PUSH_CLIP;
  cmd.data.push_clip.rect = *rect;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_pop_clip(void *gfx) {
  CMPRenderRecorder *recorder; /* GCOVR_EXCL_LINE */
  CMPRenderCmd cmd;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_POP_CLIP;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_set_transform(void *gfx, const CMPMat3 *transform) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd; /* GCOVR_EXCL_LINE */

  if (gfx == NULL || transform == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_SET_TRANSFORM;
  cmd.data.set_transform.transform = *transform;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_create_texture(void *gfx, cmp_i32 width,
                                           cmp_i32 height,
                                           cmp_u32 format, /* GCOVR_EXCL_LINE */
                                           const void *pixels, cmp_usize size,
                                           CMPHandle *out_texture) {
  CMP_UNUSED(width);
  CMP_UNUSED(height);
  CMP_UNUSED(format);
  CMP_UNUSED(pixels);
  CMP_UNUSED(size);

  if (gfx == NULL || out_texture == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_ERR_UNSUPPORTED;
}

static int cmp_render_record_update_texture(void *gfx, CMPHandle texture,
                                           cmp_i32 x, cmp_i32 y,
                                           cmp_i32 width, /* GCOVR_EXCL_LINE */
                                           cmp_i32 height, const void *pixels,
                                           cmp_usize size) {
  CMP_UNUSED(texture);
  CMP_UNUSED(x);
  CMP_UNUSED(y);
  CMP_UNUSED(width);
  CMP_UNUSED(height);
  CMP_UNUSED(pixels);
  CMP_UNUSED(size);

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_ERR_UNSUPPORTED;
}

static int cmp_render_record_destroy_texture(void *gfx, CMPHandle texture) {
  CMP_UNUSED(texture);

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_ERR_UNSUPPORTED;
}

static int
cmp_render_record_draw_texture(void *gfx, CMPHandle texture, const CMPRect *src,
                              const CMPRect *dst,
                              CMPScalar opacity) { /* GCOVR_EXCL_LINE */
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_render_validate_rect(src);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_render_validate_rect(dst);
  if (rc != CMP_OK) {
    return rc;
  }

  recorder = (CMPRenderRecorder *)gfx;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_DRAW_TEXTURE;
  cmd.data.draw_texture.texture = texture;
  cmd.data.draw_texture.src = *src;
  cmd.data.draw_texture.dst = *dst;
  cmd.data.draw_texture.opacity = opacity;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_draw_text(void *text, CMPHandle font,
                                      const char *utf8, cmp_usize utf8_len,
                                      CMPScalar x, CMPScalar y, CMPColor color) {
  CMPRenderRecorder *recorder;
  CMPRenderCmd cmd;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len > 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  recorder = (CMPRenderRecorder *)text;
  if (recorder->list == NULL) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_RENDER_CMD_DRAW_TEXT;
  cmd.data.draw_text.font = font;
  cmd.data.draw_text.utf8 = utf8;
  cmd.data.draw_text.utf8_len = utf8_len;
  cmd.data.draw_text.x = x;
  cmd.data.draw_text.y = y;
  cmd.data.draw_text.color = color;
  return cmp_render_list_append(recorder->list, &cmd);
}

static int cmp_render_record_create_font(void *text, const char *utf8_family,
                                        cmp_i32 size_px, cmp_i32 weight,
                                        CMPBool italic, CMPHandle *out_font) {
  CMP_UNUSED(utf8_family);
  CMP_UNUSED(size_px);
  CMP_UNUSED(weight);
  CMP_UNUSED(italic);

  if (text == NULL || out_font == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_ERR_UNSUPPORTED;
}

static int cmp_render_record_destroy_font(void *text, CMPHandle font) {
  CMP_UNUSED(font);

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_ERR_UNSUPPORTED;
}

static int
cmp_render_record_measure_text(void *text, CMPHandle font, const char *utf8,
                              cmp_usize utf8_len, CMPScalar *out_width,
                              CMPScalar *out_height, /* GCOVR_EXCL_LINE */
                              CMPScalar *out_baseline) {
  CMP_UNUSED(font);
  CMP_UNUSED(utf8);
  CMP_UNUSED(utf8_len);

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_ERR_UNSUPPORTED;
}

static const CMPGfxVTable g_cmp_render_record_vtable =
    {/* GCOVR_EXCL_LINE */
     cmp_render_record_begin_frame,
     cmp_render_record_end_frame,
     cmp_render_record_clear,
     cmp_render_record_draw_rect,
     cmp_render_record_draw_line,
     cmp_render_record_draw_path, /* GCOVR_EXCL_LINE */
     cmp_render_record_push_clip,
     cmp_render_record_pop_clip,
     cmp_render_record_set_transform,
     cmp_render_record_create_texture,
     cmp_render_record_update_texture,
     cmp_render_record_destroy_texture,
     cmp_render_record_draw_texture};

static const CMPTextVTable g_cmp_render_record_text_vtable =
    {/* GCOVR_EXCL_LINE */
     cmp_render_record_create_font, cmp_render_record_destroy_font,
     cmp_render_record_measure_text, cmp_render_record_draw_text};

static int cmp_render_validate_node(const CMPRenderNode *node) {
  cmp_usize i; /* GCOVR_EXCL_LINE */
  int rc;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (node->widget == NULL || node->widget->vtable == NULL ||
      node->widget->vtable->paint == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_render_validate_rect(&node->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  if (node->child_count > 0 && node->children == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < node->child_count; ++i) {
    if (node->children[i] == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }

  return CMP_OK;
}

static int
cmp_render_build_node(const CMPRenderNode *node, const CMPRect *parent_clip,
                     CMPScalar dpi_scale,
                     CMPRenderRecorder *recorder) { /* GCOVR_EXCL_LINE */
  CMPRect clip;
  CMPBool has_intersection;
  CMPPaintContext ctx;
  cmp_usize i;
  int rc;
  int first_error;

  rc = cmp_render_validate_node(node);
  if (rc != CMP_OK) {
    return rc;
  }

  if (node->widget->flags & CMP_WIDGET_FLAG_HIDDEN) {
    return CMP_OK;
  }

  if (parent_clip == NULL) {
    clip = node->bounds;
    has_intersection =
        (clip.width > 0.0f && clip.height > 0.0f) ? CMP_TRUE : CMP_FALSE;
  } else {
    rc =
        cmp_rect_intersect(parent_clip, &node->bounds, &clip, &has_intersection);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (g_cmp_render_force_intersect_fail) {
      rc = CMP_ERR_RANGE;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (!has_intersection) {
    return CMP_OK;
  }

  rc = recorder->gfx.vtable->push_clip(recorder->gfx.ctx, &clip);
  if (rc != CMP_OK) {
    return rc;
  }

  ctx.gfx = &recorder->gfx;
  ctx.clip = clip;
  ctx.dpi_scale = dpi_scale;

  first_error = node->widget->vtable->paint(node->widget->ctx, &ctx);
  if (first_error == CMP_OK) {
    for (i = 0; i < node->child_count; ++i) {
      rc = cmp_render_build_node(node->children[i], &clip, dpi_scale, recorder);
      if (rc != CMP_OK && first_error == CMP_OK) {
        first_error = rc;
      }
    }
  }

  rc = recorder->gfx.vtable->pop_clip(recorder->gfx.ctx);
  if (first_error == CMP_OK && rc != CMP_OK) {
    first_error = rc;
  }

  return first_error;
}

int CMP_CALL
cmp_render_list_init(CMPRenderList *list, const CMPAllocator *allocator,
                    cmp_usize initial_capacity) { /* GCOVR_EXCL_LINE */
  CMPAllocator chosen;
  cmp_usize bytes;
  void *mem;
  int rc; /* GCOVR_EXCL_LINE */

  if (list == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (list->commands != NULL || list->capacity != 0 || list->count != 0) {
    return CMP_ERR_STATE;
  }

  if (allocator == NULL) {
    rc = cmp_get_default_allocator(&chosen);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    chosen = *allocator;
  }

  if (chosen.alloc == NULL || chosen.realloc == NULL || chosen.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (initial_capacity == 0) {
    initial_capacity = CMP_RENDER_LIST_DEFAULT_CAPACITY;
  }

  rc = cmp_render_mul_overflow(initial_capacity, (cmp_usize)sizeof(CMPRenderCmd),
                              &bytes);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = chosen.alloc(chosen.ctx, bytes, &mem);
  if (rc != CMP_OK) {
    return rc;
  }

  list->allocator = chosen;
  list->commands = (CMPRenderCmd *)mem;
  list->count = 0;
  list->capacity = initial_capacity;
  return CMP_OK;
}

int CMP_CALL cmp_render_list_reset(CMPRenderList *list) {
  if (list == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (list->commands == NULL) {
    return CMP_ERR_STATE;
  }

  list->count = 0;
  return CMP_OK;
}

int CMP_CALL cmp_render_list_shutdown(CMPRenderList *list) {
  int rc;

  if (list == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (list->commands == NULL) {
    return CMP_ERR_STATE;
  }

  rc = list->allocator.free(list->allocator.ctx, list->commands);

  list->commands = NULL;
  list->count = 0;
  list->capacity = 0;
  list->allocator.ctx = NULL;
  list->allocator.alloc = NULL;
  list->allocator.realloc = NULL;
  list->allocator.free = NULL;

  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_render_list_append(CMPRenderList *list, const CMPRenderCmd *cmd) {
  int rc; /* GCOVR_EXCL_LINE */

  if (list == NULL || cmd == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (list->commands == NULL) {
    return CMP_ERR_STATE;
  }
  if (list->count > list->capacity) {
    return CMP_ERR_STATE;
  }

  rc = cmp_render_list_reserve(list, 1);
  if (rc != CMP_OK) {
    return rc;
  }

  list->commands[list->count] = *cmd;
  list->count += 1;
  return CMP_OK;
}

int CMP_CALL cmp_render_list_execute(const CMPRenderList *list, CMPGfx *gfx) {
  cmp_usize i;
  int rc;

  if (list == NULL || gfx == NULL || gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (list->commands == NULL) {
    return CMP_ERR_STATE;
  }

  for (i = 0; i < list->count; ++i) {
    const CMPRenderCmd *cmd = &list->commands[i];

    switch (cmd->type) {
    case CMP_RENDER_CMD_BEGIN_FRAME:
      if (gfx->vtable->begin_frame == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->begin_frame(
          gfx->ctx, cmd->data.begin_frame.window, cmd->data.begin_frame.width,
          cmd->data.begin_frame.height, cmd->data.begin_frame.dpi_scale);
      break;
    case CMP_RENDER_CMD_END_FRAME:
      if (gfx->vtable->end_frame == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->end_frame(gfx->ctx, cmd->data.end_frame.window);
      break;
    case CMP_RENDER_CMD_CLEAR: /* GCOVR_EXCL_LINE */
      if (gfx->vtable->clear == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->clear(gfx->ctx, cmd->data.clear.color);
      break;
    case CMP_RENDER_CMD_DRAW_RECT:
      if (gfx->vtable->draw_rect == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->draw_rect(gfx->ctx, &cmd->data.draw_rect.rect,
                                  cmd->data.draw_rect.color,
                                  cmd->data.draw_rect.corner_radius);
      break;
    case CMP_RENDER_CMD_DRAW_LINE:
      if (gfx->vtable->draw_line == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->draw_line(
          gfx->ctx, cmd->data.draw_line.x0, cmd->data.draw_line.y0,
          cmd->data.draw_line.x1, cmd->data.draw_line.y1,
          cmd->data.draw_line.color, cmd->data.draw_line.thickness);
      break;
    case CMP_RENDER_CMD_DRAW_PATH:
      if (gfx->vtable->draw_path == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->draw_path(gfx->ctx, cmd->data.draw_path.path,
                                  cmd->data.draw_path.color);
      break;
    case CMP_RENDER_CMD_PUSH_CLIP:
      if (gfx->vtable->push_clip == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->push_clip(gfx->ctx, &cmd->data.push_clip.rect);
      break;
    case CMP_RENDER_CMD_POP_CLIP: /* GCOVR_EXCL_LINE */
      if (gfx->vtable->pop_clip == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->pop_clip(gfx->ctx);
      break;
    case CMP_RENDER_CMD_SET_TRANSFORM: /* GCOVR_EXCL_LINE */
      if (gfx->vtable->set_transform == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->set_transform(gfx->ctx,
                                      &cmd->data.set_transform.transform);
      break;
    case CMP_RENDER_CMD_DRAW_TEXTURE: /* GCOVR_EXCL_LINE */
      if (gfx->vtable->draw_texture == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->vtable->draw_texture(
          gfx->ctx, cmd->data.draw_texture.texture, &cmd->data.draw_texture.src,
          &cmd->data.draw_texture.dst, cmd->data.draw_texture.opacity);
      break;
    case CMP_RENDER_CMD_DRAW_TEXT:
      if (gfx->text_vtable == NULL || gfx->text_vtable->draw_text == NULL) {
        return CMP_ERR_UNSUPPORTED;
      }
      rc = gfx->text_vtable->draw_text(
          gfx->ctx, cmd->data.draw_text.font, cmd->data.draw_text.utf8,
          cmd->data.draw_text.utf8_len, cmd->data.draw_text.x,
          cmd->data.draw_text.y, cmd->data.draw_text.color);
      break;
    default:
      return CMP_ERR_INVALID_ARGUMENT;
    }

    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

int CMP_CALL cmp_render_node_init(CMPRenderNode *node, CMPWidget *widget,
                                const CMPRect *bounds) {
  int rc;

  if (node == NULL || widget == NULL || bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_render_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  node->widget = widget;
  node->children = NULL;
  node->child_count = 0;
  node->bounds = *bounds;
  return CMP_OK;
}

int CMP_CALL cmp_render_node_set_bounds(
    CMPRenderNode *node, const CMPRect *bounds) { /* GCOVR_EXCL_LINE */
  int rc;

  if (node == NULL || bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_render_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  node->bounds = *bounds;
  return CMP_OK;
}

int CMP_CALL cmp_render_node_set_children(CMPRenderNode *node,
                                        CMPRenderNode **children,
                                        cmp_usize count) {
  cmp_usize i;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (count > 0 && children == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < count; ++i) {
    if (children[i] == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }

  node->children = children;
  node->child_count = count;
  return CMP_OK;
}

int CMP_CALL cmp_render_build(const CMPRenderNode *root, CMPRenderList *list,
                            CMPScalar dpi_scale) {
  CMPRenderRecorder recorder;
  cmp_usize start_count; /* GCOVR_EXCL_LINE */
  int rc;

  if (root == NULL || list == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dpi_scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (list->commands == NULL) {
    return CMP_ERR_STATE;
  }

  recorder.list = list;
  recorder.gfx.ctx = &recorder;
  recorder.gfx.vtable = &g_cmp_render_record_vtable;
  recorder.gfx.text_vtable = &g_cmp_render_record_text_vtable;

  start_count = list->count;
  rc = cmp_render_build_node(root, NULL, dpi_scale, &recorder);
  if (rc != CMP_OK) {
    list->count = start_count;
  }
  return rc;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_render_test_add_overflow(cmp_usize a, cmp_usize b,
                                        cmp_usize *out_value) {
  return cmp_render_add_overflow(a, b, out_value);
}

int CMP_CALL cmp_render_test_mul_overflow(cmp_usize a, cmp_usize b,
                                        cmp_usize *out_value) {
  return cmp_render_mul_overflow(a, b, out_value);
}

int CMP_CALL cmp_render_test_list_reserve(CMPRenderList *list,
                                        cmp_usize additional) {
  return cmp_render_list_reserve(list, additional);
}

int CMP_CALL cmp_render_test_validate_node(const CMPRenderNode *node) {
  return cmp_render_validate_node(node);
}

int CMP_CALL cmp_render_test_set_force_reserve(CMPBool enable) {
  g_cmp_render_force_reserve = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_render_test_set_force_intersect_fail(CMPBool enable) {
  g_cmp_render_force_intersect_fail = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif
