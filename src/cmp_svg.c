/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

int cmp_svg_viewbox_evaluate(const cmp_svg_viewbox_t *viewbox,
                             float layout_width, float layout_height,
                             float *out_offset_x, float *out_offset_y,
                             float *out_scale_x, float *out_scale_y) {
  float scale_x, scale_y, scale, viewbox_scaled_w, viewbox_scaled_h, offset_x,
      offset_y;

  if (!viewbox || !out_offset_x || !out_offset_y || !out_scale_x ||
      !out_scale_y)
    return CMP_ERROR_INVALID_ARG;

  if (viewbox->width <= 0.0f || viewbox->height <= 0.0f) {
    *out_offset_x = 0.0f;
    *out_offset_y = 0.0f;
    *out_scale_x = 1.0f;
    *out_scale_y = 1.0f;
    return CMP_SUCCESS;
  }

  scale_x = layout_width / viewbox->width;
  scale_y = layout_height / viewbox->height;

  if (viewbox->aspect_ratio.align == CMP_SVG_ASPECT_RATIO_NONE) {
    *out_scale_x = scale_x;
    *out_scale_y = scale_y;
    *out_offset_x = -viewbox->x * scale_x;
    *out_offset_y = -viewbox->y * scale_y;
    return CMP_SUCCESS;
  }

  if (viewbox->aspect_ratio.meet_or_slice == CMP_SVG_ASPECT_RATIO_MEET) {
    scale = (scale_x < scale_y) ? scale_x : scale_y;
  } else { /* SLICE */
    scale = (scale_x > scale_y) ? scale_x : scale_y;
  }

  *out_scale_x = scale;
  *out_scale_y = scale;

  viewbox_scaled_w = viewbox->width * scale;
  viewbox_scaled_h = viewbox->height * scale;

  offset_x = 0.0f;
  offset_y = 0.0f;

  switch (viewbox->aspect_ratio.align) {
  case CMP_SVG_ASPECT_RATIO_XMIN_YMIN:
  case CMP_SVG_ASPECT_RATIO_XMIN_YMID:
  case CMP_SVG_ASPECT_RATIO_XMIN_YMAX:
    offset_x = 0.0f;
    break;
  case CMP_SVG_ASPECT_RATIO_XMID_YMIN:
  case CMP_SVG_ASPECT_RATIO_XMID_YMID:
  case CMP_SVG_ASPECT_RATIO_XMID_YMAX:
    offset_x = (layout_width - viewbox_scaled_w) / 2.0f;
    break;
  case CMP_SVG_ASPECT_RATIO_XMAX_YMIN:
  case CMP_SVG_ASPECT_RATIO_XMAX_YMID:
  case CMP_SVG_ASPECT_RATIO_XMAX_YMAX:
    offset_x = layout_width - viewbox_scaled_w;
    break;
  default:
    break;
  }

  switch (viewbox->aspect_ratio.align) {
  case CMP_SVG_ASPECT_RATIO_XMIN_YMIN:
  case CMP_SVG_ASPECT_RATIO_XMID_YMIN:
  case CMP_SVG_ASPECT_RATIO_XMAX_YMIN:
    offset_y = 0.0f;
    break;
  case CMP_SVG_ASPECT_RATIO_XMIN_YMID:
  case CMP_SVG_ASPECT_RATIO_XMID_YMID:
  case CMP_SVG_ASPECT_RATIO_XMAX_YMID:
    offset_y = (layout_height - viewbox_scaled_h) / 2.0f;
    break;
  case CMP_SVG_ASPECT_RATIO_XMIN_YMAX:
  case CMP_SVG_ASPECT_RATIO_XMID_YMAX:
  case CMP_SVG_ASPECT_RATIO_XMAX_YMAX:
    offset_y = layout_height - viewbox_scaled_h;
    break;
  default:
    break;
  }

  *out_offset_x = offset_x - (viewbox->x * scale);
  *out_offset_y = offset_y - (viewbox->y * scale);

  return CMP_SUCCESS;
}

/* Helper for appending a vertex */
static int renderer_append_vertex(cmp_svg_renderer_t *r, float x, float y) {
  if (r->vertex_count + 2 > r->vertex_capacity) {
    size_t new_cap = r->vertex_capacity == 0 ? 32 : r->vertex_capacity * 2;
    float *new_verts;
    if (CMP_MALLOC(new_cap * sizeof(float), (void **)&new_verts) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (r->vertices) {
      memcpy(new_verts, r->vertices, r->vertex_count * sizeof(float));
      CMP_FREE(r->vertices);
    }
    r->vertices = new_verts;
    r->vertex_capacity = new_cap;
  }
  r->vertices[r->vertex_count++] = x;
  r->vertices[r->vertex_count++] = y;
  return CMP_SUCCESS;
}

int cmp_svg_renderer_create(cmp_svg_renderer_t **out_renderer,
                            float tolerance) {
  cmp_svg_renderer_t *r;
  if (!out_renderer)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_svg_renderer_t), (void **)&r) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(r, 0, sizeof(cmp_svg_renderer_t));
  r->tolerance = tolerance > 0.0f ? tolerance : 0.25f;
  *out_renderer = r;
  return CMP_SUCCESS;
}

int cmp_svg_renderer_destroy(cmp_svg_renderer_t *renderer) {
  if (!renderer)
    return CMP_ERROR_INVALID_ARG;
  if (renderer->vertices)
    CMP_FREE(renderer->vertices);
  CMP_FREE(renderer);
  return CMP_SUCCESS;
}

int cmp_svg_renderer_move_to(cmp_svg_renderer_t *renderer, float x, float y) {
  if (!renderer)
    return CMP_ERROR_INVALID_ARG;
  renderer->current_x = x;
  renderer->current_y = y;
  renderer->start_x = x;
  renderer->start_y = y;
  return renderer_append_vertex(renderer, x, y);
}

int cmp_svg_renderer_line_to(cmp_svg_renderer_t *renderer, float x, float y) {
  if (!renderer)
    return CMP_ERROR_INVALID_ARG;
  renderer->current_x = x;
  renderer->current_y = y;
  return renderer_append_vertex(renderer, x, y);
}

int cmp_svg_renderer_quad_to(cmp_svg_renderer_t *renderer, float cx, float cy,
                             float x, float y) {
  int i, steps;
  float dt, t;
  float dx, dy, dist;
  if (!renderer)
    return CMP_ERROR_INVALID_ARG;

  dx = x - renderer->current_x;
  dy = y - renderer->current_y;
  dist = (float)sqrt((double)(dx * dx + dy * dy));

  steps = (int)(dist / renderer->tolerance);
  if (steps < 4)
    steps = 4;
  if (steps > 100)
    steps = 100;

  dt = 1.0f / (float)steps;
  for (i = 1; i <= steps; i++) {
    float px, py;
    t = (float)i * dt;
    px = (1.0f - t) * (1.0f - t) * renderer->current_x +
         2.0f * (1.0f - t) * t * cx + t * t * x;
    py = (1.0f - t) * (1.0f - t) * renderer->current_y +
         2.0f * (1.0f - t) * t * cy + t * t * y;
    if (renderer_append_vertex(renderer, px, py) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
  }
  renderer->current_x = x;
  renderer->current_y = y;
  return CMP_SUCCESS;
}

int cmp_svg_renderer_cubic_to(cmp_svg_renderer_t *renderer, float cx1,
                              float cy1, float cx2, float cy2, float x,
                              float y) {
  int i, steps;
  float dt, t;
  float dx, dy, dist;
  if (!renderer)
    return CMP_ERROR_INVALID_ARG;

  dx = x - renderer->current_x;
  dy = y - renderer->current_y;
  dist = (float)sqrt((double)(dx * dx + dy * dy));

  steps = (int)(dist / renderer->tolerance);
  if (steps < 8)
    steps = 8;
  if (steps > 200)
    steps = 200;

  dt = 1.0f / (float)steps;
  for (i = 1; i <= steps; i++) {
    float px, py, mt;
    t = (float)i * dt;
    mt = 1.0f - t;
    px = mt * mt * mt * renderer->current_x + 3.0f * mt * mt * t * cx1 +
         3.0f * mt * t * t * cx2 + t * t * t * x;
    py = mt * mt * mt * renderer->current_y + 3.0f * mt * mt * t * cy1 +
         3.0f * mt * t * t * cy2 + t * t * t * y;
    if (renderer_append_vertex(renderer, px, py) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
  }
  renderer->current_x = x;
  renderer->current_y = y;
  return CMP_SUCCESS;
}

static void arc_to_center_param(float x1, float y1, float rx, float ry,
                                float angle, int large_arc, int sweep, float x2,
                                float y2, float *cx, float *cy, float *theta1,
                                float *dtheta) {
  /* Simplified / unrotated arc calculation */
  float dx2 = (x1 - x2) / 2.0f;
  float dy2 = (y1 - y2) / 2.0f;
  float rx_sq = rx * rx;
  float ry_sq = ry * ry;
  float dx2_sq = dx2 * dx2;
  float dy2_sq = dy2 * dy2;
  float rad_check = (dx2_sq / rx_sq) + (dy2_sq / ry_sq);
  float sign, sq, coef;
  float cx1, cy1;
  (void)angle; /* Ignore rotation for now */
  if (rad_check > 1.0f) {
    rx *= (float)sqrt((double)rad_check);
    ry *= (float)sqrt((double)rad_check);
    rx_sq = rx * rx;
    ry_sq = ry * ry;
  }
  sign = (large_arc == sweep) ? -1.0f : 1.0f;
  sq = ((rx_sq * ry_sq) - (rx_sq * dy2_sq) - (ry_sq * dx2_sq)) /
       ((rx_sq * dy2_sq) + (ry_sq * dx2_sq));
  sq = (sq < 0.0f) ? 0.0f : sq;
  coef = sign * (float)sqrt((double)sq);
  cx1 = coef * ((rx * dy2) / ry);
  cy1 = coef * -((ry * dx2) / rx);
  *cx = cx1 + (x1 + x2) / 2.0f;
  *cy = cy1 + (y1 + y2) / 2.0f;
  *theta1 = (float)atan2((double)((y1 - *cy) / ry), (double)((x1 - *cx) / rx));
  {
    float theta2 =
        (float)atan2((double)((y2 - *cy) / ry), (double)((x2 - *cx) / rx));
    *dtheta = theta2 - *theta1;
    if (sweep == 0 && *dtheta > 0.0f)
      *dtheta -= 2.0f * 3.1415926535f;
    else if (sweep == 1 && *dtheta < 0.0f)
      *dtheta += 2.0f * 3.1415926535f;
  }
}

int cmp_svg_renderer_arc_to(cmp_svg_renderer_t *renderer, float rx, float ry,
                            float x_axis_rotation, int large_arc_flag,
                            int sweep_flag, float x, float y) {
  float cx, cy, theta1, dtheta;
  int i, steps;
  float dt;
  if (!renderer)
    return CMP_ERROR_INVALID_ARG;

  rx = (float)fabs((double)rx);
  ry = (float)fabs((double)ry);
  if (rx == 0.0f || ry == 0.0f)
    return cmp_svg_renderer_line_to(renderer, x, y);

  arc_to_center_param(renderer->current_x, renderer->current_y, rx, ry,
                      x_axis_rotation, large_arc_flag, sweep_flag, x, y, &cx,
                      &cy, &theta1, &dtheta);

  steps = (int)((fabs((double)dtheta) * rx) / renderer->tolerance);
  if (steps < 4)
    steps = 4;
  if (steps > 100)
    steps = 100;

  dt = dtheta / (float)steps;
  for (i = 1; i <= steps; i++) {
    float angle = theta1 + (float)i * dt;
    float px = cx + rx * (float)cos((double)angle);
    float py = cy + ry * (float)sin((double)angle);
    if (renderer_append_vertex(renderer, px, py) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
  }
  renderer->current_x = x;
  renderer->current_y = y;
  return CMP_SUCCESS;
}

int cmp_svg_renderer_close(cmp_svg_renderer_t *renderer) {
  if (!renderer)
    return CMP_ERROR_INVALID_ARG;
  return cmp_svg_renderer_line_to(renderer, renderer->start_x,
                                  renderer->start_y);
}

int cmp_svg_path_tessellate(cmp_svg_path_type_t path_type, const float *data,
                            size_t data_len, float **out_vertices,
                            size_t *out_vertex_count) {
  cmp_svg_renderer_t *r;
  int err;

  if (!data || data_len == 0 || !out_vertices || !out_vertex_count)
    return CMP_ERROR_INVALID_ARG;

  if ((err = cmp_svg_renderer_create(&r, 0.5f)) != CMP_SUCCESS)
    return err;

  if (path_type == CMP_SVG_PATH_POLYGON) {
    size_t i;
    for (i = 0; i < data_len; i += 2) {
      if (i == 0) {
        cmp_svg_renderer_move_to(r, data[i], data[i + 1]);
      } else {
        cmp_svg_renderer_line_to(r, data[i], data[i + 1]);
      }
    }
  } else if (path_type == CMP_SVG_PATH_BEZIER) {
    if (data_len >= 8) {
      cmp_svg_renderer_move_to(r, data[0], data[1]);
      cmp_svg_renderer_cubic_to(r, data[2], data[3], data[4], data[5], data[6],
                                data[7]);
    }
  } else if (path_type == CMP_SVG_PATH_ARC) {
    if (data_len >= 9) {
      cmp_svg_renderer_move_to(r, data[0], data[1]);
      cmp_svg_renderer_arc_to(r, data[2], data[3], data[4], (int)data[5],
                              (int)data[6], data[7], data[8]);
    }
  }

  if (r->vertex_count == 0) {
    cmp_svg_renderer_destroy(r);
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(r->vertex_count * sizeof(float), (void **)out_vertices) !=
      CMP_SUCCESS) {
    cmp_svg_renderer_destroy(r);
    return CMP_ERROR_OOM;
  }

  memcpy(*out_vertices, r->vertices, r->vertex_count * sizeof(float));
  *out_vertex_count = r->vertex_count / 2;

  cmp_svg_renderer_destroy(r);
  return CMP_SUCCESS;
}

static void normalize2(float *x, float *y) {
  float len = (float)sqrt((double)((*x) * (*x) + (*y) * (*y)));
  if (len > 0.0f) {
    *x /= len;
    *y /= len;
  }
}

int cmp_svg_dash_evaluate(const cmp_svg_dash_t *dash, const float *in_vertices,
                          size_t in_count, float ***out_subpaths,
                          size_t **out_subpath_counts,
                          size_t *out_subpath_count) {
  if (!dash || !in_vertices || in_count == 0 || !out_subpaths ||
      !out_subpath_counts || !out_subpath_count)
    return CMP_ERROR_INVALID_ARG;

  if (dash->count == 0 || !dash->array) {
    *out_subpath_count = 1;
    if (CMP_MALLOC(sizeof(float *), (void **)out_subpaths) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (CMP_MALLOC(sizeof(size_t), (void **)out_subpath_counts) !=
        CMP_SUCCESS) {
      CMP_FREE(*out_subpaths);
      return CMP_ERROR_OOM;
    }
    if (CMP_MALLOC(in_count * 2 * sizeof(float),
                   (void **)&((*out_subpaths)[0])) != CMP_SUCCESS) {
      CMP_FREE(*out_subpaths);
      CMP_FREE(*out_subpath_counts);
      return CMP_ERROR_OOM;
    }
    memcpy((*out_subpaths)[0], in_vertices, in_count * 2 * sizeof(float));
    (*out_subpath_counts)[0] = in_count;
    return CMP_SUCCESS;
  }

  /* Placeholder for proper dash walking. Right now just returns the whole path
   * as one dash */
  *out_subpath_count = 1;
  if (CMP_MALLOC(sizeof(float *), (void **)out_subpaths) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  if (CMP_MALLOC(sizeof(size_t), (void **)out_subpath_counts) != CMP_SUCCESS) {
    CMP_FREE(*out_subpaths);
    return CMP_ERROR_OOM;
  }
  if (CMP_MALLOC(in_count * 2 * sizeof(float),
                 (void **)&((*out_subpaths)[0])) != CMP_SUCCESS) {
    CMP_FREE(*out_subpaths);
    CMP_FREE(*out_subpath_counts);
    return CMP_ERROR_OOM;
  }
  memcpy((*out_subpaths)[0], in_vertices, in_count * 2 * sizeof(float));
  (*out_subpath_counts)[0] = in_count;
  return CMP_SUCCESS;
}

int cmp_svg_stroke_evaluate(const cmp_svg_stroke_t *stroke,
                            const float *in_vertices, size_t in_count,
                            float **out_stroke_vertices,
                            size_t *out_stroke_count) {
  cmp_svg_renderer_t *left, *right;
  float hw;
  size_t i;
  int err;

  if (!stroke || !in_vertices || in_count < 2 || !out_stroke_vertices ||
      !out_stroke_count)
    return CMP_ERROR_INVALID_ARG;

  hw = stroke->width * 0.5f;

  if ((err = cmp_svg_renderer_create(&left, 0.5f)) != CMP_SUCCESS)
    return err;
  if ((err = cmp_svg_renderer_create(&right, 0.5f)) != CMP_SUCCESS) {
    cmp_svg_renderer_destroy(left);
    return err;
  }

  for (i = 0; i < in_count; i++) {
    float x = in_vertices[i * 2];
    float y = in_vertices[i * 2 + 1];
    float dx = 0.0f, dy = 0.0f, nx, ny;

    if (i == 0) {
      dx = in_vertices[2] - x;
      dy = in_vertices[3] - y;
    } else if (i == in_count - 1) {
      dx = x - in_vertices[(i - 1) * 2];
      dy = y - in_vertices[(i - 1) * 2 + 1];
    } else {
      float dx1 = x - in_vertices[(i - 1) * 2];
      float dy1 = y - in_vertices[(i - 1) * 2 + 1];
      float dx2 = in_vertices[(i + 1) * 2] - x;
      float dy2 = in_vertices[(i + 1) * 2 + 1] - y;
      normalize2(&dx1, &dy1);
      normalize2(&dx2, &dy2);
      dx = dx1 + dx2;
      dy = dy1 + dy2;
    }

    normalize2(&dx, &dy);
    nx = -dy;
    ny = dx;

    if (i == 0 && stroke->linecap == 2) {
      x -= dx * hw;
      y -= dy * hw;
    } else if (i == in_count - 1 && stroke->linecap == 2) {
      x += dx * hw;
      y += dy * hw;
    }

    if (i == 0) {
      cmp_svg_renderer_move_to(left, x + nx * hw, y + ny * hw);
      cmp_svg_renderer_move_to(right, x - nx * hw, y - ny * hw);
    } else {
      cmp_svg_renderer_line_to(left, x + nx * hw, y + ny * hw);
      cmp_svg_renderer_line_to(right, x - nx * hw, y - ny * hw);
    }
  }

  *out_stroke_count = (left->vertex_count + right->vertex_count) / 2;
  if (CMP_MALLOC(*out_stroke_count * 2 * sizeof(float),
                 (void **)out_stroke_vertices) != CMP_SUCCESS) {
    cmp_svg_renderer_destroy(left);
    cmp_svg_renderer_destroy(right);
    return CMP_ERROR_OOM;
  }

  memcpy(*out_stroke_vertices, left->vertices,
         left->vertex_count * sizeof(float));
  for (i = 0; i < right->vertex_count; i += 2) {
    size_t rev_i = right->vertex_count - 2 - i;
    (*out_stroke_vertices)[left->vertex_count + i] = right->vertices[rev_i];
    (*out_stroke_vertices)[left->vertex_count + i + 1] =
        right->vertices[rev_i + 1];
  }

  cmp_svg_renderer_destroy(left);
  cmp_svg_renderer_destroy(right);

  if (stroke->dash.count > 0 && stroke->dash.array) {
    /* Dashed Strokes implementation placeholder */
  }

  return CMP_SUCCESS;
}

int cmp_svg_css_bind(cmp_svg_node_t *svg_node, void *css_style) {
  cmp_svg_css_t *css;
  if (!svg_node)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_svg_css_t), (void **)&css) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(css, 0, sizeof(cmp_svg_css_t));
  css->computed_style = css_style;
  css->element_name = "svg-node";

  svg_node->css = css;

  return CMP_SUCCESS;
}

int cmp_svg_use_instantiate(cmp_svg_node_t *source_node,
                            cmp_svg_node_t **out_cloned_node) {
  cmp_svg_node_t *clone;
  size_t i;
  int res;

  if (!source_node || !out_cloned_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_svg_node_create(&clone);
  if (res != CMP_SUCCESS)
    return res;

  clone->id = source_node->id;
  clone->is_foreign_object = source_node->is_foreign_object;
  clone->dom_mapping = source_node->dom_mapping;
  clone->stroke = source_node->stroke;
  clone->fill = source_node->fill;
  clone->filter = source_node->filter;
  clone->animations = source_node->animations;

  if (source_node->css) {
    if (CMP_MALLOC(sizeof(cmp_svg_css_t), (void **)&clone->css) ==
        CMP_SUCCESS) {
      memcpy(clone->css, source_node->css, sizeof(cmp_svg_css_t));
    }
  }

  for (i = 0; i < source_node->child_count; i++) {
    cmp_svg_node_t *child_clone = NULL;
    res = cmp_svg_use_instantiate(source_node->children[i], &child_clone);
    if (res == CMP_SUCCESS && child_clone != NULL) {
      cmp_svg_node_add_child(clone, child_clone);
    }
  }

  *out_cloned_node = clone;
  return CMP_SUCCESS;
}

int cmp_svg_smil_tick(cmp_svg_node_t *node, float dt_ms) {
  cmp_svg_smil_t *anim;
  size_t i;

  if (!node)
    return CMP_ERROR_INVALID_ARG;

  anim = node->animations;
  while (anim) {
    float progress;
    anim->current_time += dt_ms;

    if (anim->duration > 0.0f) {
      progress = anim->current_time / anim->duration;
      if (progress > 1.0f)
        progress = 1.0f;
    } else {
      progress = 1.0f;
    }

    if (anim->current_time > anim->duration) {
      anim->current_time = anim->duration;
    }

    if (anim->target_node) {
      float current_val =
          anim->start_value + (anim->end_value - anim->start_value) * progress;
      (void)current_val; /* Mock applying the value to target */
    }

    break; /* Simple single animation stub */
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_svg_smil_tick(node->children[i], dt_ms);
  }

  return CMP_SUCCESS;
}

int cmp_svg_foreign_bind(cmp_svg_node_t *svg_node,
                         cmp_layout_node_t *dom_node) {
  if (!svg_node || !dom_node)
    return CMP_ERROR_INVALID_ARG;

  svg_node->is_foreign_object = 1;
  svg_node->dom_mapping = dom_node;

  return CMP_SUCCESS;
}

int cmp_svg_filter_evaluate(const cmp_svg_filter_node_t *filter,
                            const unsigned char *in_pixels, int width,
                            int height, unsigned char **out_pixels) {
  unsigned char *out_buf;
  int i, total;

  if (!filter || !in_pixels || width <= 0 || height <= 0 || !out_pixels)
    return CMP_ERROR_INVALID_ARG;

  total = width * height * 4;
  if (CMP_MALLOC(total, (void **)&out_buf) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  if (filter->type == CMP_SVG_FE_COLOR_MATRIX) {
    for (i = 0; i < total; i += 4) {
      unsigned char r = in_pixels[i];
      unsigned char g = in_pixels[i + 1];
      unsigned char b = in_pixels[i + 2];
      unsigned char a = in_pixels[i + 3];
      unsigned char luma = (unsigned char)((r * 77 + g * 150 + b * 29) >> 8);
      out_buf[i] = luma;
      out_buf[i + 1] = luma;
      out_buf[i + 2] = luma;
      out_buf[i + 3] = a;
    }
  } else if (filter->type == CMP_SVG_FE_DISPLACEMENT_MAP) {
    /* Stub passthrough */
    memcpy(out_buf, in_pixels, total);
  } else {
    /* Default fallback passthrough */
    memcpy(out_buf, in_pixels, total);
  }

  *out_pixels = out_buf;
  return CMP_SUCCESS;
}

int cmp_svg_node_create(cmp_svg_node_t **out_node) {
  cmp_svg_node_t *node;
  if (!out_node)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_svg_node_t), (void **)&node) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(node, 0, sizeof(cmp_svg_node_t));
  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_svg_node_destroy(cmp_svg_node_t *node) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  if (node->css) {
    CMP_FREE(node->css);
  }
  if (node->children) {
    size_t i;
    for (i = 0; i < node->child_count; i++) {
      cmp_svg_node_destroy(node->children[i]);
    }
    CMP_FREE(node->children);
  }
  CMP_FREE(node);
  return CMP_SUCCESS;
}

int cmp_svg_node_add_child(cmp_svg_node_t *parent, cmp_svg_node_t *child) {
  cmp_svg_node_t **new_children;
  size_t new_cap;
  if (!parent || !child)
    return CMP_ERROR_INVALID_ARG;

  if (parent->child_count >= parent->child_capacity) {
    new_cap = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    if (CMP_MALLOC(new_cap * sizeof(cmp_svg_node_t *),
                   (void **)&new_children) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (parent->children) {
      memcpy(new_children, parent->children,
             parent->child_count * sizeof(cmp_svg_node_t *));
      CMP_FREE(parent->children);
    }
    parent->children = new_children;
    parent->child_capacity = new_cap;
  }

  parent->children[parent->child_count++] = child;
  child->parent = parent;
  return CMP_SUCCESS;
}
int cmp_svg_fill_evaluate(const cmp_svg_fill_t *fill, const float *in_vertices,
                          size_t in_count, float **out_fill_vertices,
                          size_t *out_fill_count) {
  size_t i, tri_count;
  float *out_buf;

  if (!fill || !in_vertices || in_count < 3 || !out_fill_vertices ||
      !out_fill_count)
    return CMP_ERROR_INVALID_ARG;

  /* Create a triangle fan from the first vertex (in_vertices[0],
   * in_vertices[1]) */
  tri_count = in_count - 2;
  if (CMP_MALLOC(tri_count * 3 * 2 * sizeof(float), (void **)&out_buf) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  for (i = 0; i < tri_count; i++) {
    out_buf[i * 6 + 0] = in_vertices[0];
    out_buf[i * 6 + 1] = in_vertices[1];
    out_buf[i * 6 + 2] = in_vertices[(i + 1) * 2 + 0];
    out_buf[i * 6 + 3] = in_vertices[(i + 1) * 2 + 1];
    out_buf[i * 6 + 4] = in_vertices[(i + 2) * 2 + 0];
    out_buf[i * 6 + 5] = in_vertices[(i + 2) * 2 + 1];
  }

  *out_fill_vertices = out_buf;
  *out_fill_count = tri_count * 3;

  return CMP_SUCCESS;
}
