/* clang-format off */
#include "ui_svg.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
#include <ctype.h>
#include <string.h>
#include <math.h>
/* clang-format on */

static enum ui_error skip_whitespace_and_commas(const char **ptr) {
  while (**ptr) {
    if (isspace((unsigned char)**ptr) || **ptr == ',') {
      (*ptr)++;
    } else {
      break;
    }
  }
  return UI_ERROR_NONE;
}

static enum ui_error parse_float(const char **ptr, float *out_val) {
  char *end;
  (void)skip_whitespace_and_commas(ptr);
  if (!**ptr) {
    return UI_ERROR_PARSE_FAILED;
  }
  *out_val = (float)strtod(*ptr, &end);
  if (*ptr == end) {
    return UI_ERROR_PARSE_FAILED;
  }
  *ptr = end;
  return UI_ERROR_NONE;
}

static enum ui_error ensure_capacity(struct ui_svg_path *path) {
  if (path->count >= path->capacity) {
    ui_uint32 new_cap = path->capacity == 0 ? 16 : path->capacity * 2;
    struct ui_svg_command *new_cmds = (struct ui_svg_command *)UI_REALLOC(
        path->commands, new_cap * sizeof(struct ui_svg_command));
    if (!new_cmds) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    path->commands = new_cmds;
    path->capacity = new_cap;
  }
  return UI_ERROR_NONE;
}

static enum ui_error reflect_point(struct ui_svg_point *out_p,
                                   const struct ui_svg_point *p,
                                   const struct ui_svg_point *ref) {
  out_p->x = ref->x * 2.0f - p->x;
  out_p->y = ref->y * 2.0f - p->y;
  return UI_ERROR_NONE;
}

enum ui_error ui_svg_path_init(struct ui_svg_path *path) {
  if (!path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  path->commands = NULL;
  path->count = 0;
  path->capacity = 0;
  return UI_ERROR_NONE;
}

void ui_svg_path_destroy(struct ui_svg_path *path) {
  if (path) {
    if (path->commands) {
      UI_FREE(path->commands);
    }
    path->commands = NULL;
    path->count = 0;
    path->capacity = 0;
  }
}

enum ui_error ui_svg_path_parse(struct ui_svg_path *path, const char *d_attr) {
  const char *ptr;
  char cmd;
  char last_cmd;
  float cx;
  float cy;
  float start_x;
  float start_y;
  struct ui_svg_point last_cp;
  enum ui_error rc = UI_ERROR_NONE;

  if (!path || !d_attr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ptr = d_attr;
  cmd = '\0';
  last_cmd = '\0';
  cx = 0.0f;
  cy = 0.0f;
  start_x = 0.0f;
  start_y = 0.0f;
  last_cp.x = 0.0f;
  last_cp.y = 0.0f;

  while (*ptr) {
    (void)skip_whitespace_and_commas(&ptr);
    if (!*ptr) {
      break;
    }

    if (isalpha((unsigned char)*ptr)) {
      cmd = *ptr++;
    } else {
      /* If no command is given, it repeats the last command.
       * But for 'M' or 'm', repeated commands are treated as 'L' or 'l'. */
      if (cmd == 'M') {
        cmd = 'L';
      } else if (cmd == 'm') {
        cmd = 'l';
      } else if (cmd == '\0') {
        rc = UI_ERROR_PARSE_FAILED;
        goto cleanup;
      }
    }

    {
      struct ui_svg_command new_cmd;
      float args[7];
      int i;
      int expected_args;
      int is_relative;
      char upper_cmd;

      memset(&new_cmd, 0, sizeof(new_cmd));
      is_relative = islower((unsigned char)cmd) ? 1 : 0;
      upper_cmd = (char)toupper((unsigned char)cmd);

      switch (upper_cmd) {
      case 'Z':
        expected_args = 0;
        break;
      case 'H':
      case 'V':
        expected_args = 1;
        break;
      case 'M':
      case 'L':
      case 'T':
        expected_args = 2;
        break;
      case 'S':
      case 'Q':
        expected_args = 4;
        break;
      case 'C':
        expected_args = 6;
        break;
      case 'A':
        expected_args = 7;
        break;
      default:
        rc = UI_ERROR_PARSE_FAILED;
        goto cleanup;
      }

      for (i = 0; i < expected_args; i++) {
        rc = parse_float(&ptr, &args[i]);
        if (rc != UI_ERROR_NONE) {
          goto cleanup;
        }
      }

      rc = ensure_capacity(path);
      if (rc != UI_ERROR_NONE) {
        goto cleanup;
      }

      switch (upper_cmd) {
      case 'M':
        new_cmd.type = UI_SVG_CMD_MOVE_TO;
        new_cmd.data.move_to.p.x = is_relative ? cx + args[0] : args[0];
        new_cmd.data.move_to.p.y = is_relative ? cy + args[1] : args[1];
        start_x = new_cmd.data.move_to.p.x;
        start_y = new_cmd.data.move_to.p.y;
        cx = start_x;
        cy = start_y;
        last_cp.x = cx;
        last_cp.y = cy;
        break;
      case 'L':
        new_cmd.type = UI_SVG_CMD_LINE_TO;
        new_cmd.data.line_to.p.x = is_relative ? cx + args[0] : args[0];
        new_cmd.data.line_to.p.y = is_relative ? cy + args[1] : args[1];
        cx = new_cmd.data.line_to.p.x;
        cy = new_cmd.data.line_to.p.y;
        last_cp.x = cx;
        last_cp.y = cy;
        break;
      case 'H':
        new_cmd.type = UI_SVG_CMD_LINE_TO;
        new_cmd.data.line_to.p.x = is_relative ? cx + args[0] : args[0];
        new_cmd.data.line_to.p.y = cy;
        cx = new_cmd.data.line_to.p.x;
        last_cp.x = cx;
        last_cp.y = cy;
        break;
      case 'V':
        new_cmd.type = UI_SVG_CMD_LINE_TO;
        new_cmd.data.line_to.p.x = cx;
        new_cmd.data.line_to.p.y = is_relative ? cy + args[0] : args[0];
        cy = new_cmd.data.line_to.p.y;
        last_cp.x = cx;
        last_cp.y = cy;
        break;
      case 'C':
        new_cmd.type = UI_SVG_CMD_CUBIC_BEZIER;
        new_cmd.data.cubic_bezier.cp1.x = is_relative ? cx + args[0] : args[0];
        new_cmd.data.cubic_bezier.cp1.y = is_relative ? cy + args[1] : args[1];
        new_cmd.data.cubic_bezier.cp2.x = is_relative ? cx + args[2] : args[2];
        new_cmd.data.cubic_bezier.cp2.y = is_relative ? cy + args[3] : args[3];
        new_cmd.data.cubic_bezier.p.x = is_relative ? cx + args[4] : args[4];
        new_cmd.data.cubic_bezier.p.y = is_relative ? cy + args[5] : args[5];
        cx = new_cmd.data.cubic_bezier.p.x;
        cy = new_cmd.data.cubic_bezier.p.y;
        last_cp = new_cmd.data.cubic_bezier.cp2;
        break;
      case 'S':
        new_cmd.type = UI_SVG_CMD_CUBIC_BEZIER;
        if (last_cmd == 'c' || last_cmd == 'C' || last_cmd == 's' ||
            last_cmd == 'S') {
          struct ui_svg_point current_p;
          current_p.x = cx;
          current_p.y = cy;
          (void)reflect_point(&new_cmd.data.cubic_bezier.cp1, &last_cp,
                              &current_p);
        } else {
          new_cmd.data.cubic_bezier.cp1.x = cx;
          new_cmd.data.cubic_bezier.cp1.y = cy;
        }
        new_cmd.data.cubic_bezier.cp2.x = is_relative ? cx + args[0] : args[0];
        new_cmd.data.cubic_bezier.cp2.y = is_relative ? cy + args[1] : args[1];
        new_cmd.data.cubic_bezier.p.x = is_relative ? cx + args[2] : args[2];
        new_cmd.data.cubic_bezier.p.y = is_relative ? cy + args[3] : args[3];
        cx = new_cmd.data.cubic_bezier.p.x;
        cy = new_cmd.data.cubic_bezier.p.y;
        last_cp = new_cmd.data.cubic_bezier.cp2;
        break;
      case 'Q':
        new_cmd.type = UI_SVG_CMD_QUADRATIC_BEZIER;
        new_cmd.data.quadratic_bezier.cp.x =
            is_relative ? cx + args[0] : args[0];
        new_cmd.data.quadratic_bezier.cp.y =
            is_relative ? cy + args[1] : args[1];
        new_cmd.data.quadratic_bezier.p.x =
            is_relative ? cx + args[2] : args[2];
        new_cmd.data.quadratic_bezier.p.y =
            is_relative ? cy + args[3] : args[3];
        cx = new_cmd.data.quadratic_bezier.p.x;
        cy = new_cmd.data.quadratic_bezier.p.y;
        last_cp = new_cmd.data.quadratic_bezier.cp;
        break;
      case 'T':
        new_cmd.type = UI_SVG_CMD_QUADRATIC_BEZIER;
        if (last_cmd == 'q' || last_cmd == 'Q' || last_cmd == 't' ||
            last_cmd == 'T') {
          struct ui_svg_point current_p;
          current_p.x = cx;
          current_p.y = cy;
          (void)reflect_point(&new_cmd.data.quadratic_bezier.cp, &last_cp,
                              &current_p);
        } else {
          new_cmd.data.quadratic_bezier.cp.x = cx;
          new_cmd.data.quadratic_bezier.cp.y = cy;
        }
        new_cmd.data.quadratic_bezier.p.x =
            is_relative ? cx + args[0] : args[0];
        new_cmd.data.quadratic_bezier.p.y =
            is_relative ? cy + args[1] : args[1];
        cx = new_cmd.data.quadratic_bezier.p.x;
        cy = new_cmd.data.quadratic_bezier.p.y;
        last_cp = new_cmd.data.quadratic_bezier.cp;
        break;
      case 'A':
        new_cmd.type = UI_SVG_CMD_ARC;
        new_cmd.data.arc.rx = args[0];
        new_cmd.data.arc.ry = args[1];
        new_cmd.data.arc.x_axis_rotation = args[2];
        new_cmd.data.arc.large_arc_flag = args[3] != 0.0f ? 1 : 0;
        new_cmd.data.arc.sweep_flag = args[4] != 0.0f ? 1 : 0;
        new_cmd.data.arc.p.x = is_relative ? cx + args[5] : args[5];
        new_cmd.data.arc.p.y = is_relative ? cy + args[6] : args[6];
        cx = new_cmd.data.arc.p.x;
        cy = new_cmd.data.arc.p.y;
        last_cp.x = cx;
        last_cp.y = cy;
        break;
      case 'Z':
        new_cmd.type = UI_SVG_CMD_CLOSE_PATH;
        cx = start_x;
        cy = start_y;
        last_cp.x = cx;
        last_cp.y = cy;
        break;
      }

      path->commands[path->count++] = new_cmd;
      last_cmd = cmd;
    }
  }

cleanup:
  if (rc != UI_ERROR_NONE) {
    if (path->commands) {
      UI_FREE(path->commands);
      path->commands = NULL;
    }
    path->count = 0;
    path->capacity = 0;
  }
  return rc;
}

enum ui_error ui_svg_flattened_path_init(struct ui_svg_flattened_path *path) {
  if (!path)
    return UI_ERROR_INVALID_ARGUMENT;
  path->subpaths = NULL;
  path->count = 0;
  path->capacity = 0;
  return UI_ERROR_NONE;
}

void ui_svg_flattened_path_destroy(struct ui_svg_flattened_path *path) {
  ui_uint32 i;
  if (!path)
    return;
  for (i = 0; i < path->count; i++) {
    if (path->subpaths[i].points) {
      UI_FREE(path->subpaths[i].points);
    }
  }
  if (path->subpaths) {
    UI_FREE(path->subpaths);
  }
  path->subpaths = NULL;
  path->count = 0;
  path->capacity = 0;
}

enum ui_error ui_svg_geometry_init(struct ui_svg_geometry *geom) {
  if (!geom)
    return UI_ERROR_INVALID_ARGUMENT;
  geom->vertices = NULL;
  geom->vertex_count = 0;
  geom->vertex_capacity = 0;
  geom->indices = NULL;
  geom->index_count = 0;
  geom->index_capacity = 0;
  return UI_ERROR_NONE;
}

void ui_svg_geometry_destroy(struct ui_svg_geometry *geom) {
  if (!geom)
    return;
  if (geom->vertices)
    UI_FREE(geom->vertices);
  if (geom->indices)
    UI_FREE(geom->indices);
  geom->vertices = NULL;
  geom->indices = NULL;
  geom->vertex_count = 0;
  geom->vertex_capacity = 0;
  geom->index_count = 0;
  geom->index_capacity = 0;
}

static enum ui_error append_subpath(struct ui_svg_flattened_path *path,
                                    struct ui_svg_subpath **out_subpath) {
  if (path->count >= path->capacity) {
    ui_uint32 new_cap = path->capacity == 0 ? 4 : path->capacity * 2;
    struct ui_svg_subpath *new_arr = (struct ui_svg_subpath *)UI_REALLOC(
        path->subpaths, new_cap * sizeof(struct ui_svg_subpath));
    if (!new_arr)
      return UI_ERROR_OUT_OF_MEMORY;
    path->subpaths = new_arr;
    path->capacity = new_cap;
  }
  path->subpaths[path->count].points = NULL;
  path->subpaths[path->count].count = 0;
  path->subpaths[path->count].capacity = 0;
  path->subpaths[path->count].closed = 0;
  *out_subpath = &path->subpaths[path->count];
  path->count++;
  return UI_ERROR_NONE;
}

static enum ui_error append_point(struct ui_svg_subpath *subpath, float x,
                                  float y) {
  if (subpath->count >= subpath->capacity) {
    ui_uint32 new_cap = subpath->capacity == 0 ? 16 : subpath->capacity * 2;
    struct ui_svg_point *new_arr = (struct ui_svg_point *)UI_REALLOC(
        subpath->points, new_cap * sizeof(struct ui_svg_point));
    if (!new_arr)
      return UI_ERROR_OUT_OF_MEMORY;
    subpath->points = new_arr;
    subpath->capacity = new_cap;
  }
  if (subpath->count > 0) {
    struct ui_svg_point last = subpath->points[subpath->count - 1];
    if (fabs(last.x - x) < 1e-5f && fabs(last.y - y) < 1e-5f) {
      return UI_ERROR_NONE; /* Skip duplicates */
    }
  }
  subpath->points[subpath->count].x = x;
  subpath->points[subpath->count].y = y;
  subpath->count++;
  return UI_ERROR_NONE;
}

static enum ui_error
flatten_cubic_recursive(struct ui_svg_subpath *subpath, struct ui_svg_point p0,
                        struct ui_svg_point p1, struct ui_svg_point p2,
                        struct ui_svg_point p3, float tolerance_sq, int depth) {
  float dx = p3.x - p0.x;
  float dy = p3.y - p0.y;
  float d1 = (float)fabs((p1.x - p3.x) * dy - (p1.y - p3.y) * dx);
  float d2 = (float)fabs((p2.x - p3.x) * dy - (p2.y - p3.y) * dx);
  enum ui_error rc;

  if (depth > 10 ||
      (d1 + d2) * (d1 + d2) < tolerance_sq * (dx * dx + dy * dy)) {
    return append_point(subpath, p3.x, p3.y);
  }

  {
    struct ui_svg_point p01, p12, p23, p012, p123, p0123;
    p01.x = (p0.x + p1.x) * 0.5f;
    p01.y = (p0.y + p1.y) * 0.5f;
    p12.x = (p1.x + p2.x) * 0.5f;
    p12.y = (p1.y + p2.y) * 0.5f;
    p23.x = (p2.x + p3.x) * 0.5f;
    p23.y = (p2.y + p3.y) * 0.5f;
    p012.x = (p01.x + p12.x) * 0.5f;
    p012.y = (p01.y + p12.y) * 0.5f;
    p123.x = (p12.x + p23.x) * 0.5f;
    p123.y = (p12.y + p23.y) * 0.5f;
    p0123.x = (p012.x + p123.x) * 0.5f;
    p0123.y = (p012.y + p123.y) * 0.5f;

    rc = flatten_cubic_recursive(subpath, p0, p01, p012, p0123, tolerance_sq,
                                 depth + 1);
    if (rc != UI_ERROR_NONE)
      return rc;
    return flatten_cubic_recursive(subpath, p0123, p123, p23, p3, tolerance_sq,
                                   depth + 1);
  }
}

static enum ui_error flatten_quadratic_recursive(struct ui_svg_subpath *subpath,
                                                 struct ui_svg_point p0,
                                                 struct ui_svg_point p1,
                                                 struct ui_svg_point p2,
                                                 float tolerance_sq,
                                                 int depth) {
  float dx = p2.x - p0.x;
  float dy = p2.y - p0.y;
  float d = (float)fabs((p1.x - p2.x) * dy - (p1.y - p2.y) * dx);
  enum ui_error rc;

  if (depth > 10 || d * d < tolerance_sq * (dx * dx + dy * dy)) {
    return append_point(subpath, p2.x, p2.y);
  }

  {
    struct ui_svg_point p01, p12, p012;
    p01.x = (p0.x + p1.x) * 0.5f;
    p01.y = (p0.y + p1.y) * 0.5f;
    p12.x = (p1.x + p2.x) * 0.5f;
    p12.y = (p1.y + p2.y) * 0.5f;
    p012.x = (p01.x + p12.x) * 0.5f;
    p012.y = (p01.y + p12.y) * 0.5f;

    rc = flatten_quadratic_recursive(subpath, p0, p01, p012, tolerance_sq,
                                     depth + 1);
    if (rc != UI_ERROR_NONE)
      return rc;
    return flatten_quadratic_recursive(subpath, p012, p12, p2, tolerance_sq,
                                       depth + 1);
  }
}

static enum ui_error arc_to_lines(struct ui_svg_subpath *subpath,
                                  struct ui_svg_point p0, float rx, float ry,
                                  float x_axis_rotation, int large_arc_flag,
                                  int sweep_flag, struct ui_svg_point p,
                                  float tolerance) {
  /* W3C Endpoint to Center parameterization */
  float pi = 3.14159265358979323846f;
  float rad = x_axis_rotation * pi / 180.0f;
  float cos_rad = (float)cos(rad);
  float sin_rad = (float)sin(rad);
  float dx2 = (p0.x - p.x) / 2.0f;
  float dy2 = (p0.y - p.y) / 2.0f;
  float x1p = cos_rad * dx2 + sin_rad * dy2;
  float y1p = -sin_rad * dx2 + cos_rad * dy2;
  float rx_sq, ry_sq, x1p_sq, y1p_sq, rad_chk;
  float sign, sq, coef, cxp, cyp, cx, cy;
  float ux, uy, vx, vy, start_angle, angle_extent;
  int segments, i;
  enum ui_error rc;

  (void)tolerance;

  rx = (float)fabs(rx);
  ry = (float)fabs(ry);

  if (rx == 0.0f || ry == 0.0f) {
    return append_point(subpath, p.x, p.y);
  }

  rx_sq = rx * rx;
  ry_sq = ry * ry;
  x1p_sq = x1p * x1p;
  y1p_sq = y1p * y1p;

  rad_chk = x1p_sq / rx_sq + y1p_sq / ry_sq;
  if (rad_chk > 1.0f) {
    rx *= (float)sqrt(rad_chk);
    ry *= (float)sqrt(rad_chk);
    rx_sq = rx * rx;
    ry_sq = ry * ry;
  }

  sign = (large_arc_flag == sweep_flag) ? -1.0f : 1.0f;
  sq = ((rx_sq * ry_sq) - (rx_sq * y1p_sq) - (ry_sq * x1p_sq)) /
       ((rx_sq * y1p_sq) + (ry_sq * x1p_sq));
  sq = (sq < 0.0f) ? 0.0f : sq;
  coef = sign * (float)sqrt(sq);
  cxp = coef * ((rx * y1p) / ry);
  cyp = coef * (-(ry * x1p) / rx);

  cx = cos_rad * cxp - sin_rad * cyp + (p0.x + p.x) / 2.0f;
  cy = sin_rad * cxp + cos_rad * cyp + (p0.y + p.y) / 2.0f;

  ux = (x1p - cxp) / rx;
  uy = (y1p - cyp) / ry;
  vx = (-x1p - cxp) / rx;
  vy = (-y1p - cyp) / ry;

  {
    float n = (float)sqrt(ux * ux + uy * uy);
    float p_val = ux;
    float angle = (float)acos(p_val / n);
    start_angle = (uy < 0.0f) ? -angle : angle;
  }

  {
    float n = (float)sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
    float p_val = ux * vx + uy * vy;
    float d = p_val / n;
    float angle;
    angle = (float)acos(d);
    angle_extent = (ux * vy - uy * vx < 0.0f) ? -angle : angle;
  }

  if (!sweep_flag && angle_extent > 0.0f) {
    angle_extent -= 2.0f * pi;
  } else if (sweep_flag && angle_extent < 0.0f) {
    angle_extent += 2.0f * pi;
  }

  segments = (int)ceil((float)fabs(angle_extent) /
                       (pi / 8.0f)); /* Simple segment limit based on angle */

  for (i = 1; i <= segments; i++) {
    float angle = start_angle + angle_extent * ((float)i / (float)segments);
    float x = cos_rad * rx * (float)cos(angle) -
              sin_rad * ry * (float)sin(angle) + cx;
    float y = sin_rad * rx * (float)cos(angle) +
              cos_rad * ry * (float)sin(angle) + cy;
    rc = append_point(subpath, x, y);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  /* Ensure final point is exact */
  return append_point(subpath, p.x, p.y);
}

enum ui_error ui_svg_path_flatten(struct ui_svg_flattened_path *flattened,
                                  const struct ui_svg_path *path,
                                  float tolerance) {
  ui_uint32 i;
  struct ui_svg_subpath *current_subpath = NULL;
  struct ui_svg_point current_point;
  struct ui_svg_point start_point;
  enum ui_error rc = UI_ERROR_NONE;
  float tolerance_sq = tolerance * tolerance;

  if (!flattened || !path)
    return UI_ERROR_INVALID_ARGUMENT;

  current_point.x = 0.0f;
  current_point.y = 0.0f;
  start_point.x = 0.0f;
  start_point.y = 0.0f;

  for (i = 0; i < path->count; i++) {
    const struct ui_svg_command *cmd = &path->commands[i];

    if (cmd->type == UI_SVG_CMD_MOVE_TO) {
      rc = append_subpath(flattened, &current_subpath);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_point(current_subpath, cmd->data.move_to.p.x,
                        cmd->data.move_to.p.y);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      current_point = cmd->data.move_to.p;
      start_point = current_point;
    } else if (current_subpath == NULL) {
      /* Path starts without a move_to. Implicitly move to 0,0. */
      rc = append_subpath(flattened, &current_subpath);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_point(current_subpath, 0.0f, 0.0f);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
    }

    if (cmd->type == UI_SVG_CMD_LINE_TO) {
      rc = append_point(current_subpath, cmd->data.line_to.p.x,
                        cmd->data.line_to.p.y);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      current_point = cmd->data.line_to.p;
    } else if (cmd->type == UI_SVG_CMD_CUBIC_BEZIER) {
      rc = flatten_cubic_recursive(current_subpath, current_point,
                                   cmd->data.cubic_bezier.cp1,
                                   cmd->data.cubic_bezier.cp2,
                                   cmd->data.cubic_bezier.p, tolerance_sq, 0);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      current_point = cmd->data.cubic_bezier.p;
    } else if (cmd->type == UI_SVG_CMD_QUADRATIC_BEZIER) {
      rc = flatten_quadratic_recursive(
          current_subpath, current_point, cmd->data.quadratic_bezier.cp,
          cmd->data.quadratic_bezier.p, tolerance_sq, 0);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      current_point = cmd->data.quadratic_bezier.p;
    } else if (cmd->type == UI_SVG_CMD_ARC) {
      rc = arc_to_lines(current_subpath, current_point, cmd->data.arc.rx,
                        cmd->data.arc.ry, cmd->data.arc.x_axis_rotation,
                        cmd->data.arc.large_arc_flag, cmd->data.arc.sweep_flag,
                        cmd->data.arc.p, tolerance);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      current_point = cmd->data.arc.p;
    } else if (cmd->type == UI_SVG_CMD_CLOSE_PATH) {
      rc = append_point(current_subpath, start_point.x, start_point.y);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      current_subpath->closed = 1;
      current_point = start_point;

      /* Close path ends the current subpath, next command should be a move_to
       * normally, but if it's a line/curve, it continues from the start point
       * in a new subpath in some renderers, or the same. For robustness, we
       * will let the next command dictate if a new subpath is needed, but SVG
       * spec says a new subpath begins if a line follows a Z. */
      current_subpath = NULL;
    }
  }

cleanup:
  if (rc != UI_ERROR_NONE) {
    ui_svg_flattened_path_destroy(flattened);
  }
  return rc;
}

static enum ui_error append_vertex(struct ui_svg_geometry *geom,
                                   struct ui_svg_point p,
                                   ui_uint32 *out_index) {
  if (geom->vertex_count >= geom->vertex_capacity) {
    ui_uint32 new_cap =
        geom->vertex_capacity == 0 ? 64 : geom->vertex_capacity * 2;
    struct ui_svg_point *new_arr = (struct ui_svg_point *)UI_REALLOC(
        geom->vertices, new_cap * sizeof(struct ui_svg_point));
    if (!new_arr)
      return UI_ERROR_OUT_OF_MEMORY;
    geom->vertices = new_arr;
    geom->vertex_capacity = new_cap;
  }
  geom->vertices[geom->vertex_count] = p;
  *out_index = geom->vertex_count;
  geom->vertex_count++;
  return UI_ERROR_NONE;
}

static enum ui_error append_index(struct ui_svg_geometry *geom,
                                  ui_uint32 index) {
  if (geom->index_count >= geom->index_capacity) {
    ui_uint32 new_cap =
        geom->index_capacity == 0 ? 128 : geom->index_capacity * 2;
    ui_uint32 *new_arr =
        (ui_uint32 *)UI_REALLOC(geom->indices, new_cap * sizeof(ui_uint32));
    if (!new_arr)
      return UI_ERROR_OUT_OF_MEMORY;
    geom->indices = new_arr;
    geom->index_capacity = new_cap;
  }
  geom->indices[geom->index_count] = index;
  geom->index_count++;
  return UI_ERROR_NONE;
}

static enum ui_error triangle_area(struct ui_svg_point a, struct ui_svg_point b,
                                   struct ui_svg_point c, float *out_area) {
  *out_area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  return UI_ERROR_NONE;
}

static enum ui_error is_point_in_triangle(struct ui_svg_point p,
                                          struct ui_svg_point a,
                                          struct ui_svg_point b,
                                          struct ui_svg_point c,
                                          int *out_match) {
  float area = 0.0f;
  float w1 = 0.0f, w2 = 0.0f, w3 = 0.0f;
  *out_match = 0;
  (void)triangle_area(a, b, c, &area);
  (void)triangle_area(b, c, p, &w1);
  w1 /= area;
  (void)triangle_area(c, a, p, &w2);
  w2 /= area;
  (void)triangle_area(a, b, p, &w3);
  w3 /= area;
  *out_match = (w1 >= 0.0f && w2 >= 0.0f && w3 >= 0.0f);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_svg_tessellate_fill(struct ui_svg_geometry *geom,
                       const struct ui_svg_flattened_path *flattened) {
  ui_uint32 i;
  enum ui_error rc = UI_ERROR_NONE;

  if (!geom || !flattened)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < flattened->count; i++) {
    struct ui_svg_subpath *subpath = &flattened->subpaths[i];
    ui_uint32 *linked = NULL;
    ui_uint32 n = subpath->count;
    ui_uint32 original_n;
    ui_uint32 j, prev, curr, next;
    int ears_found;

    if (n > 1 &&
        fabs(subpath->points[0].x - subpath->points[n - 1].x) < 1e-5f &&
        fabs(subpath->points[0].y - subpath->points[n - 1].y) < 1e-5f) {
      n--;
    }
    original_n = n;

    if (n < 3)
      continue;

    linked = (ui_uint32 *)UI_MALLOC(n * sizeof(ui_uint32));
    if (!linked) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }

    /* Ensure correct winding order (we want CCW for ear clipping, which means
     * positive area if Y is down) We will just check signed area and flip if
     * needed. */
    {
      float total_area = 0.0f;
      int reverse = 0;
      for (j = 0; j < n; j++) {
        struct ui_svg_point p1 = subpath->points[j];
        struct ui_svg_point p2 = subpath->points[(j + 1) % n];
        total_area += (p1.x * p2.y - p2.x * p1.y);
      }
      if (total_area < 0.0f) {
        reverse = 1;
      } else {
        reverse = 0;
      }

      if (reverse) {
        for (j = 0; j < n; j++) {
          linked[j] = (j == 0) ? n - 1 : j - 1;
        }
      } else {
        for (j = 0; j < n; j++) {
          linked[j] = (j + 1) % n;
        }
      }
    }

    curr = 0;
    ears_found = 1;
    while (n > 2 && ears_found) {
      ui_uint32 start = curr;
      ears_found = 0;
      do {
        /* Find prev */
        float area = 0.0f;
        for (prev = 0; prev < original_n; prev++) {
          if (linked[prev] == curr)
            break;
        }
        next = linked[curr];

        (void)triangle_area(subpath->points[prev], subpath->points[curr],
                            subpath->points[next], &area);
        if (area > 0.0f) {
          /* Convex, test if any other point is inside */
          int is_ear = 1;
          ui_uint32 test_pt = linked[next];
          while (test_pt != prev) {
            int is_in = 0;
            (void)is_point_in_triangle(
                subpath->points[test_pt], subpath->points[prev],
                subpath->points[curr], subpath->points[next], &is_in);
            if (is_in) {
              is_ear = 0;
              break;
            }
            test_pt = linked[test_pt];
          }

          if (is_ear) {
            ui_uint32 i1, i2, i3;
            rc = append_vertex(geom, subpath->points[prev], &i1);
            if (rc != UI_ERROR_NONE) {
              UI_FREE(linked);
              goto cleanup;
            }
            rc = append_vertex(geom, subpath->points[curr], &i2);
            if (rc != UI_ERROR_NONE) {
              UI_FREE(linked);
              goto cleanup;
            }
            rc = append_vertex(geom, subpath->points[next], &i3);
            if (rc != UI_ERROR_NONE) {
              UI_FREE(linked);
              goto cleanup;
            }

            rc = append_index(geom, i1);
            if (rc != UI_ERROR_NONE) {
              UI_FREE(linked);
              goto cleanup;
            }
            rc = append_index(geom, i2);
            if (rc != UI_ERROR_NONE) {
              UI_FREE(linked);
              goto cleanup;
            }
            rc = append_index(geom, i3);
            if (rc != UI_ERROR_NONE) {
              UI_FREE(linked);
              goto cleanup;
            }

            /* Remove curr */
            linked[prev] = next;
            linked[curr] = (ui_uint32)-1;
            curr = next;
            n--;
            ears_found = 1;
            break;
          }
        }
        curr = linked[curr];
      } while (curr != start);
    }
    UI_FREE(linked);
  }

cleanup:
  return rc;
}

/** \brief ui_error */
enum ui_error
ui_svg_tessellate_stroke(struct ui_svg_geometry *geom,
                         const struct ui_svg_flattened_path *flattened,
                         float stroke_width) {
  ui_uint32 i, j;
  enum ui_error rc = UI_ERROR_NONE;
  float half_width = stroke_width * 0.5f;

  if (!geom || !flattened)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < flattened->count; i++) {
    struct ui_svg_subpath *subpath = &flattened->subpaths[i];
    if (subpath->count < 2)
      continue;

    for (j = 0; j < subpath->count - 1; j++) {
      struct ui_svg_point p1 = subpath->points[j];
      struct ui_svg_point p2 = subpath->points[j + 1];
      float dx = p2.x - p1.x;
      float dy = p2.y - p1.y;
      float len = (float)sqrt(dx * dx + dy * dy);
      float nx, ny;
      ui_uint32 i1, i2, i3, i4;
      struct ui_svg_point v1, v2, v3, v4;

      if (len < 1e-5f)
        continue;

      nx = -dy / len * half_width;
      ny = dx / len * half_width;

      v1.x = p1.x + nx;
      v1.y = p1.y + ny;
      v2.x = p1.x - nx;
      v2.y = p1.y - ny;
      v3.x = p2.x + nx;
      v3.y = p2.y + ny;
      v4.x = p2.x - nx;
      v4.y = p2.y - ny;

      rc = append_vertex(geom, v1, &i1);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_vertex(geom, v2, &i2);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_vertex(geom, v3, &i3);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_vertex(geom, v4, &i4);
      if (rc != UI_ERROR_NONE)
        goto cleanup;

      rc = append_index(geom, i1);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_index(geom, i2);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_index(geom, i3);
      if (rc != UI_ERROR_NONE)
        goto cleanup;

      rc = append_index(geom, i2);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_index(geom, i4);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = append_index(geom, i3);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
    }
  }

cleanup:
  return rc;
}
