/**
 * @file md3_shape_morph.c
 * @brief Dynamic shape morphing and expressive polygon implementation for
 * Material 3.
 */

/* clang-format off */
#include "material3/md3_shape_morph.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Helper to generate parametric radial shapes with modulated radius
 * r(theta).
 */
static void
generate_radial_shape(size_t vertex_count,
                      float (*radius_fn)(float angle_rad, void *ctx), void *ctx,
                      struct md3_shape_vertex *vertices) {
  size_t i;
  float step;

  step = (float)(2.0 * M_PI / (double)vertex_count);
  for (i = 0; i < vertex_count; i++) {
    float theta;
    float r;
    theta = (float)i * step;
    r = radius_fn(theta, ctx);
    vertices[i].x = r * (float)cos(theta);
    vertices[i].y = r * (float)sin(theta);
  }
}

static float circle_radius(float angle, void *ctx) {
  if (angle == 0.0f && ctx == NULL) {
    return 1.0f;
  }
  return 1.0f;
}

static float star_radius(float angle, void *ctx) {
  int points;
  float p;
  float mod;
  points = *(int *)ctx;
  p = (float)(M_PI / (double)points);
  mod = (float)fmod(angle, 2.0 * p);
  if (mod > p) {
    mod = (float)(2.0 * p) - mod;
  }
  return 0.45f + 0.55f * (1.0f - (mod / p));
}

static float scallop_radius(float angle, void *ctx) {
  int lobes;
  lobes = *(int *)ctx;
  return 0.8f + 0.2f * (float)cos(lobes * angle);
}

static float clover_radius(float angle, void *ctx) {
  int leaves;
  leaves = *(int *)ctx;
  return 0.7f + 0.3f * (float)cos(leaves * angle);
}

static float flower_radius(float angle, void *ctx) {
  int petals;
  petals = *(int *)ctx;
  return (float)(0.6f + 0.4f * fabs(cos(0.5 * petals * angle)));
}

static float burst_radius(float angle, void *ctx) {
  if (ctx != NULL) {
    return 0.75f + 0.25f * (float)sin(16.0f * angle);
  }
  return 0.75f + 0.25f * (float)sin(16.0f * angle);
}

static float sunny_radius(float angle, void *ctx) {
  if (ctx != NULL) {
    return 0.7f + 0.3f * (float)cos(8.0f * angle);
  }
  return 0.7f + 0.3f * (float)cos(8.0f * angle);
}

static float cookie_radius(float angle, void *ctx) {
  if (ctx != NULL) {
    return 0.85f + 0.15f * (float)sin(9.0f * angle);
  }
  return 0.85f + 0.15f * (float)sin(9.0f * angle);
}

static float puff_radius(float angle, void *ctx) {
  if (ctx != NULL) {
    return 0.8f + 0.2f * (float)cos(5.0f * angle);
  }
  return 0.8f + 0.2f * (float)cos(5.0f * angle);
}

static float wave_radius(float angle, void *ctx) {
  if (ctx != NULL) {
    return 0.85f + 0.15f * (float)sin(3.0f * angle);
  }
  return 0.85f + 0.15f * (float)sin(3.0f * angle);
}

static float regular_polygon_radius(float angle, void *ctx) {
  int sides;
  float alpha;
  float mod;
  sides = *(int *)ctx;
  alpha = (float)(M_PI / (double)sides);
  mod = (float)fmod(angle, 2.0 * alpha);
  mod -= alpha;
  return (float)(cos(alpha) / cos(mod));
}

/**
 * @brief Generates polygon vertices for a given expressive shape type.
 *
 * @param type The expressive shape type.
 * @param vertex_count Number of sample vertices to generate (minimum 4, maximum
 * MD3_SHAPE_MAX_VERTICES).
 * @param out_shape Pointer to destination shape struct.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_shape_create_expressive(enum md3_expressive_shape_type type,
                                       size_t vertex_count,
                                       struct md3_shape *out_shape) {
  size_t i;
  int param;

  if (!out_shape || vertex_count < 4 || vertex_count > MD3_SHAPE_MAX_VERTICES) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_shape->type = type;
  out_shape->vertex_count = vertex_count;

  switch (type) {
  case MD3_EXPRESSIVE_SHAPE_RECTANGLE:
    for (i = 0; i < vertex_count; i++) {
      float t;
      t = (float)i / (float)vertex_count;
      if (t < 0.25f) {
        out_shape->vertices[i].x = -1.0f + 8.0f * t;
        out_shape->vertices[i].y = -1.0f;
      } else if (t < 0.5f) {
        out_shape->vertices[i].x = 1.0f;
        out_shape->vertices[i].y = -1.0f + 8.0f * (t - 0.25f);
      } else if (t < 0.75f) {
        out_shape->vertices[i].x = 1.0f - 8.0f * (t - 0.5f);
        out_shape->vertices[i].y = 1.0f;
      } else {
        out_shape->vertices[i].x = -1.0f;
        out_shape->vertices[i].y = 1.0f - 8.0f * (t - 0.75f);
      }
    }
    break;

  case MD3_EXPRESSIVE_SHAPE_CIRCLE:
    generate_radial_shape(vertex_count, circle_radius, NULL,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_PILL:
  case MD3_EXPRESSIVE_SHAPE_OVAL:
    generate_radial_shape(vertex_count, circle_radius, NULL,
                          out_shape->vertices);
    for (i = 0; i < vertex_count; i++) {
      out_shape->vertices[i].y *= 0.55f;
    }
    break;

  case MD3_EXPRESSIVE_SHAPE_ARCH:
    generate_radial_shape(vertex_count, circle_radius, NULL,
                          out_shape->vertices);
    for (i = 0; i < vertex_count; i++) {
      if (out_shape->vertices[i].y > 0.0f) {
        out_shape->vertices[i].y = 0.8f;
      }
    }
    break;

  case MD3_EXPRESSIVE_SHAPE_CLOVER_4:
    param = 4;
    generate_radial_shape(vertex_count, clover_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_CLOVER_8:
    param = 8;
    generate_radial_shape(vertex_count, clover_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_SCALLOP_6:
    param = 6;
    generate_radial_shape(vertex_count, scallop_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_SCALLOP_12:
    param = 12;
    generate_radial_shape(vertex_count, scallop_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_STAR_4:
    param = 4;
    generate_radial_shape(vertex_count, star_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_STAR_8:
    param = 8;
    generate_radial_shape(vertex_count, star_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_STAR_12:
    param = 12;
    generate_radial_shape(vertex_count, star_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_BURST:
    generate_radial_shape(vertex_count, burst_radius, NULL,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_FLOWER_4:
    param = 4;
    generate_radial_shape(vertex_count, flower_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_FLOWER_8:
    param = 8;
    generate_radial_shape(vertex_count, flower_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_CUT_CORNER_RECTANGLE:
  case MD3_EXPRESSIVE_SHAPE_CUT_CORNER_TOP_RIGHT:
  case MD3_EXPRESSIVE_SHAPE_ASYMMETRIC_ROUNDED_1:
  case MD3_EXPRESSIVE_SHAPE_ASYMMETRIC_ROUNDED_2:
  case MD3_EXPRESSIVE_SHAPE_SLANTED:
    for (i = 0; i < vertex_count; i++) {
      float theta;
      theta = (float)i * (float)(2.0 * M_PI / (double)vertex_count);
      out_shape->vertices[i].x = (float)cos(theta);
      out_shape->vertices[i].y = (float)sin(theta);
      if (type == MD3_EXPRESSIVE_SHAPE_CUT_CORNER_RECTANGLE) {
        if (out_shape->vertices[i].x > 0.6f &&
            out_shape->vertices[i].y > 0.6f) {
          out_shape->vertices[i].x = 0.6f;
          out_shape->vertices[i].y = 0.6f;
        }
      } else if (type == MD3_EXPRESSIVE_SHAPE_SLANTED) {
        out_shape->vertices[i].x += 0.25f * out_shape->vertices[i].y;
      }
    }
    break;

  case MD3_EXPRESSIVE_SHAPE_OCTAGON:
    param = 8;
    generate_radial_shape(vertex_count, regular_polygon_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_HEXAGON:
    param = 6;
    generate_radial_shape(vertex_count, regular_polygon_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_PENTAGON:
    param = 5;
    generate_radial_shape(vertex_count, regular_polygon_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_DIAMOND:
    param = 4;
    generate_radial_shape(vertex_count, regular_polygon_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_TRIANGLE:
    param = 3;
    generate_radial_shape(vertex_count, regular_polygon_radius, &param,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_HEART:
    for (i = 0; i < vertex_count; i++) {
      double t;
      t = (double)i * 2.0 * M_PI / (double)vertex_count;
      out_shape->vertices[i].x = (float)(16.0 * pow(sin(t), 3.0) / 16.0);
      out_shape->vertices[i].y = (float)(-(13.0 * cos(t) - 5.0 * cos(2.0 * t) -
                                           2.0 * cos(3.0 * t) - cos(4.0 * t)) /
                                         16.0);
    }
    break;

  case MD3_EXPRESSIVE_SHAPE_DROP:
    for (i = 0; i < vertex_count; i++) {
      float theta;
      float r;
      theta = (float)i * (float)(2.0 * M_PI / (double)vertex_count);
      r = 0.5f * (1.0f + (float)sin(theta));
      out_shape->vertices[i].x = r * (float)cos(theta);
      out_shape->vertices[i].y = (float)sin(theta);
    }
    break;

  case MD3_EXPRESSIVE_SHAPE_BUNNY_EARS:
  case MD3_EXPRESSIVE_SHAPE_SUNNY:
    generate_radial_shape(vertex_count, sunny_radius, NULL,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_COOKIE:
    generate_radial_shape(vertex_count, cookie_radius, NULL,
                          out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_PUFF:
  case MD3_EXPRESSIVE_SHAPE_CLOUD:
    generate_radial_shape(vertex_count, puff_radius, NULL, out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_WAVE:
    generate_radial_shape(vertex_count, wave_radius, NULL, out_shape->vertices);
    break;

  case MD3_EXPRESSIVE_SHAPE_GEM:
  case MD3_EXPRESSIVE_SHAPE_SHIELD:
    param = 6;
    generate_radial_shape(vertex_count, regular_polygon_radius, &param,
                          out_shape->vertices);
    break;

  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Interpolates smoothly between two shapes at normalized progress
 * [0.0, 1.0].
 *
 * @param from Source shape.
 * @param to Target shape.
 * @param progress Morph progress in range [0.0, 1.0].
 * @param out_shape Destination shape struct receiving interpolated coordinates.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_shape_morph_interpolate(const struct md3_shape *from,
                                       const struct md3_shape *to,
                                       float progress,
                                       struct md3_shape *out_shape) {
  size_t i;
  size_t count;
  float p;
  float inv_p;

  if (!from || !to || !out_shape) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (progress < 0.0f) {
    progress = 0.0f;
  } else if (progress > 1.0f) {
    progress = 1.0f;
  }

  count = from->vertex_count < to->vertex_count ? from->vertex_count
                                                : to->vertex_count;
  if (count == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_shape->type = progress < 0.5f ? from->type : to->type;
  out_shape->vertex_count = count;

  p = progress;
  inv_p = 1.0f - p;

  for (i = 0; i < count; i++) {
    out_shape->vertices[i].x =
        inv_p * from->vertices[i].x + p * to->vertices[i].x;
    out_shape->vertices[i].y =
        inv_p * from->vertices[i].y + p * to->vertices[i].y;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Generates an SVG path string from a shape scaled to specified width
 * and height.
 *
 * @param shape The shape definition.
 * @param width Target width in dp/px.
 * @param height Target height in dp/px.
 * @param out_buffer Destination string buffer.
 * @param buffer_capacity Capacity of destination buffer in bytes.
 * @return UI_ERROR_NONE on success, UI_ERROR_BUFFER_TOO_SMALL, or
 * UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_shape_to_svg_path(const struct md3_shape *shape, float width,
                                 float height, char *out_buffer,
                                 size_t buffer_capacity) {
  size_t i;
  size_t offset;
  float half_w;
  float half_h;
  int written;

  if (!shape || !out_buffer || buffer_capacity == 0 || width <= 0.0f ||
      height <= 0.0f || shape->vertex_count < 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  half_w = width * 0.5f;
  half_h = height * 0.5f;
  offset = 0;

  for (i = 0; i < shape->vertex_count; i++) {
    float px;
    float py;
    char cmd;

    px = half_w + shape->vertices[i].x * half_w;
    py = half_h + shape->vertices[i].y * half_h;
    cmd = (i == 0) ? 'M' : 'L';

#if defined(_MSC_VER)
    written = _snprintf_s(out_buffer + offset, buffer_capacity - offset,
                          _TRUNCATE, "%c%.1f,%.1f ", cmd, px, py);
#else
    written = snprintf(out_buffer + offset, buffer_capacity - offset,
                       "%c%.1f,%.1f ", cmd, px, py);
#endif
    if ((size_t)written >= (buffer_capacity - offset)) {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
    offset += (size_t)written;
  }

  if (offset + 2 >= buffer_capacity) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  out_buffer[offset++] = 'Z';
  out_buffer[offset] = '\0';

  return UI_ERROR_NONE;
}
