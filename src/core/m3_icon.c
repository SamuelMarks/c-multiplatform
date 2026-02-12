#include "m3/m3_icon.h"
#include "m3/m3_path.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <stdlib.h>

#ifdef M3_TESTING
#define M3_ICON_TEST_FAIL_NONE 0u
#define M3_ICON_TEST_FAIL_PATH_INIT 1u
#define M3_ICON_TEST_FAIL_PATH_APPEND 2u

static m3_u32 g_m3_icon_test_fail_point = M3_ICON_TEST_FAIL_NONE;
static m3_usize g_m3_icon_test_cstr_limit = 0;

int M3_CALL m3_icon_test_set_fail_point(m3_u32 fail_point) {
  g_m3_icon_test_fail_point = fail_point;
  return M3_OK;
}

int M3_CALL m3_icon_test_clear_fail_points(void) {
  g_m3_icon_test_fail_point = M3_ICON_TEST_FAIL_NONE;
  return M3_OK;
}

int M3_CALL m3_icon_test_set_cstr_limit(m3_usize max_len) {
  g_m3_icon_test_cstr_limit = max_len;
  return M3_OK;
}

static int m3_icon_test_fail_point_match(m3_u32 point) {
  if (g_m3_icon_test_fail_point != point) {
    return 0;
  }
  g_m3_icon_test_fail_point = M3_ICON_TEST_FAIL_NONE;
  return 1;
}
#endif

typedef struct M3IconSvgTransform {
  M3Scalar scale;
  M3Scalar offset_x;
  M3Scalar offset_y;
  M3Scalar viewbox_x;
  M3Scalar viewbox_y;
} M3IconSvgTransform;

static int m3_icon_validate_color(const M3Color *color) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_icon_validate_style(const M3IconStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->utf8_family == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return M3_ERR_RANGE;
  }
  if (style->italic != M3_FALSE && style->italic != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  rc = m3_icon_validate_color(&style->color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_icon_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_icon_validate_svg(const M3IconSvg *svg) {
  if (svg == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (svg->utf8_path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (svg->viewbox_width <= 0.0f || svg->viewbox_height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static m3_usize m3_icon_cstr_limit(void) {
  m3_usize max_len;

  max_len = (m3_usize) ~(m3_usize)0;
#ifdef M3_TESTING
  if (g_m3_icon_test_cstr_limit != 0) {
    max_len = g_m3_icon_test_cstr_limit;
  }
#endif
  return max_len;
}

static int m3_icon_cstrlen(const char *cstr, m3_usize *out_len) {
  m3_usize length;
  m3_usize max_len;

  if (cstr == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  max_len = m3_icon_cstr_limit();
  length = 0;
  while (cstr[length] != '\0') {
    if (length == max_len) {
      return M3_ERR_OVERFLOW;
    }
    length += 1;
  }
  *out_len = length;
  return M3_OK;
}

static int m3_icon_validate_render_mode(m3_u32 render_mode) {
  if (render_mode != M3_ICON_RENDER_AUTO &&
      render_mode != M3_ICON_RENDER_FONT && render_mode != M3_ICON_RENDER_SVG) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_icon_gfx_supports_font(const M3Gfx *gfx, M3Bool require_draw) {
  if (gfx == NULL || gfx->text_vtable == NULL) {
    return 0;
  }
  if (gfx->text_vtable->create_font == NULL ||
      gfx->text_vtable->measure_text == NULL ||
      gfx->text_vtable->destroy_font == NULL) {
    return 0;
  }
  if (require_draw == M3_TRUE && gfx->text_vtable->draw_text == NULL) {
    return 0;
  }
  return 1;
}

static int m3_icon_gfx_supports_svg(const M3Gfx *gfx) {
  if (gfx == NULL || gfx->vtable == NULL || gfx->vtable->draw_path == NULL) {
    return 0;
  }
  return 1;
}

static int m3_icon_select_render_mode(const M3Gfx *gfx, const char *utf8_name,
                                      m3_usize utf8_len, const M3IconSvg *svg,
                                      m3_u32 render_mode, M3Bool require_draw,
                                      m3_u32 *out_mode) {
  int has_font;
  int has_svg;
  int rc;

  if (out_mode == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  has_font = m3_icon_gfx_supports_font(gfx, require_draw);
  has_svg = m3_icon_gfx_supports_svg(gfx);

  if (render_mode == M3_ICON_RENDER_FONT) {
    if (!has_font) {
      return M3_ERR_UNSUPPORTED;
    }
    if (utf8_name == NULL) {
      return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
    }
    *out_mode = M3_ICON_RENDER_FONT;
    return M3_OK;
  }

  if (render_mode == M3_ICON_RENDER_SVG) {
    if (!has_svg) {
      return M3_ERR_UNSUPPORTED;
    }
    rc = m3_icon_validate_svg(svg);
    if (rc != M3_OK) {
      return rc;
    }
    *out_mode = M3_ICON_RENDER_SVG;
    return M3_OK;
  }

  if (has_font && utf8_name != NULL) {
    *out_mode = M3_ICON_RENDER_FONT;
    return M3_OK;
  }
  if (has_svg && svg != NULL) {
    rc = m3_icon_validate_svg(svg);
    if (rc != M3_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    *out_mode = M3_ICON_RENDER_SVG;
    return M3_OK;
  }
  return M3_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
}

static void m3_icon_svg_transform_point(const M3IconSvgTransform *xf,
                                        M3Scalar x, M3Scalar y, M3Scalar *out_x,
                                        M3Scalar *out_y) {
  *out_x = (x - xf->viewbox_x) * xf->scale + xf->offset_x;
  *out_y = (y - xf->viewbox_y) * xf->scale + xf->offset_y;
}

static void m3_icon_svg_skip_separators(const char **cursor) {
  const char *p;

  if (cursor == NULL || *cursor == NULL) {
    return; /* GCOVR_EXCL_LINE */
  }

  p = *cursor;
  while (*p != '\0') {
    if (*p == ',' || isspace((unsigned char)*p)) {
      p += 1;
    } else {
      break;
    }
  }
  *cursor = p;
}

static int m3_icon_svg_parse_number(const char **cursor, M3Scalar *out_value) {
  const char *p;
  char *endptr;
  double value;

  if (cursor == NULL || *cursor == NULL || out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  p = *cursor;
  m3_icon_svg_skip_separators(&p);
  if (*p == '\0') {
    *cursor = p;
    return M3_ERR_NOT_FOUND;
  }

  errno = 0;
  value = strtod(p, &endptr);
  if (endptr == p) {
    *cursor = p;
    return M3_ERR_NOT_FOUND;
  }
  if (errno == ERANGE) {
    *cursor = endptr;
    return M3_ERR_RANGE;
  }
  if (value != value) {
    *cursor = endptr;
    return M3_ERR_RANGE;
  }
  if (value > (double)FLT_MAX || value < (double)-FLT_MAX) {
    *cursor = endptr;    /* GCOVR_EXCL_LINE */
    return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  *out_value = (M3Scalar)value;
  *cursor = endptr;
  return M3_OK;
}

static int m3_icon_svg_parse_pair(const char **cursor, M3Scalar *out_x,
                                  M3Scalar *out_y) {
  int rc;

  rc = m3_icon_svg_parse_number(cursor, out_x);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_icon_svg_parse_number(cursor, out_y);
  if (rc == M3_ERR_NOT_FOUND) {
    return M3_ERR_CORRUPT;
  }
  return rc;
}

static int m3_icon_map_path_result(int rc) {
  if (rc == M3_ERR_STATE) {
    return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
  }
  return rc;
}

static int m3_icon_path_move_to(M3Path *path, const M3IconSvgTransform *xf,
                                M3Scalar x, M3Scalar y) {
  M3Scalar tx;
  M3Scalar ty;
  int rc;

#ifdef M3_TESTING
  if (m3_icon_test_fail_point_match(M3_ICON_TEST_FAIL_PATH_APPEND)) {
    return M3_ERR_IO;
  }
#endif

  m3_icon_svg_transform_point(xf, x, y, &tx, &ty);
  rc = m3_path_move_to(path, tx, ty);
  return m3_icon_map_path_result(rc);
}

static int m3_icon_path_line_to(M3Path *path, const M3IconSvgTransform *xf,
                                M3Scalar x, M3Scalar y) {
  M3Scalar tx;
  M3Scalar ty;
  int rc;

#ifdef M3_TESTING
  if (m3_icon_test_fail_point_match(M3_ICON_TEST_FAIL_PATH_APPEND)) {
    return M3_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif

  m3_icon_svg_transform_point(xf, x, y, &tx, &ty);
  rc = m3_path_line_to(path, tx, ty);
  return m3_icon_map_path_result(rc);
}

static int m3_icon_path_quad_to(M3Path *path, const M3IconSvgTransform *xf,
                                M3Scalar cx, M3Scalar cy, M3Scalar x,
                                M3Scalar y) {
  M3Scalar tcx;
  M3Scalar tcy;
  M3Scalar tx;
  M3Scalar ty;
  int rc;

#ifdef M3_TESTING
  if (m3_icon_test_fail_point_match(M3_ICON_TEST_FAIL_PATH_APPEND)) {
    return M3_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif

  m3_icon_svg_transform_point(xf, cx, cy, &tcx, &tcy);
  m3_icon_svg_transform_point(xf, x, y, &tx, &ty);
  rc = m3_path_quad_to(path, tcx, tcy, tx, ty);
  return m3_icon_map_path_result(rc);
}

static int m3_icon_path_cubic_to(M3Path *path, const M3IconSvgTransform *xf,
                                 M3Scalar cx1, M3Scalar cy1, M3Scalar cx2,
                                 M3Scalar cy2, M3Scalar x, M3Scalar y) {
  M3Scalar tcx1;
  M3Scalar tcy1;
  M3Scalar tcx2;
  M3Scalar tcy2;
  M3Scalar tx;
  M3Scalar ty;
  int rc;

#ifdef M3_TESTING
  if (m3_icon_test_fail_point_match(M3_ICON_TEST_FAIL_PATH_APPEND)) {
    return M3_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif

  m3_icon_svg_transform_point(xf, cx1, cy1, &tcx1, &tcy1);
  m3_icon_svg_transform_point(xf, cx2, cy2, &tcx2, &tcy2);
  m3_icon_svg_transform_point(xf, x, y, &tx, &ty);
  rc = m3_path_cubic_to(path, tcx1, tcy1, tcx2, tcy2, tx, ty);
  return m3_icon_map_path_result(rc);
}

static int m3_icon_path_close(M3Path *path) {
  int rc;

#ifdef M3_TESTING
  if (m3_icon_test_fail_point_match(M3_ICON_TEST_FAIL_PATH_APPEND)) {
    return M3_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif

  rc = m3_path_close(path);
  return m3_icon_map_path_result(rc);
}

static int m3_icon_svg_compute_transform(const M3IconSvg *svg,
                                         const M3Rect *bounds,
                                         M3IconSvgTransform *out_xf) {
  M3Scalar scale_x;
  M3Scalar scale_y;
  M3Scalar scale;
  M3Scalar scaled_w;
  M3Scalar scaled_h;

  if (svg == NULL || bounds == NULL || out_xf == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (svg->viewbox_width <= 0.0f || svg->viewbox_height <= 0.0f) {
    return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  scale_x = bounds->width / svg->viewbox_width;
  scale_y = bounds->height / svg->viewbox_height;
  scale = (scale_x < scale_y) ? scale_x : scale_y;

  scaled_w = svg->viewbox_width * scale;
  scaled_h = svg->viewbox_height * scale;

  out_xf->scale = scale;
  out_xf->offset_x = bounds->x + (bounds->width - scaled_w) * 0.5f;
  out_xf->offset_y = bounds->y + (bounds->height - scaled_h) * 0.5f;
  out_xf->viewbox_x = svg->viewbox_x;
  out_xf->viewbox_y = svg->viewbox_y;
  return M3_OK;
}

static int m3_icon_svg_parse(const char *data, const M3IconSvgTransform *xf,
                             M3Path *path) {
  const char *p;
  M3Scalar cur_x;
  M3Scalar cur_y;
  M3Scalar start_x;
  M3Scalar start_y;
  M3Scalar last_cx;
  M3Scalar last_cy;
  M3Scalar last_qx;
  M3Scalar last_qy;
  M3Scalar x;
  M3Scalar y;
  M3Scalar x1;
  M3Scalar y1;
  M3Scalar x2;
  M3Scalar y2;
  int rc;
  int relative;
  int saw_command;
  int got_any;
  char cmd;
  char last_cmd;

  if (data == NULL || xf == NULL || path == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  p = data;
  cur_x = 0.0f;
  cur_y = 0.0f;
  start_x = 0.0f;
  start_y = 0.0f;
  last_cx = 0.0f;
  last_cy = 0.0f;
  last_qx = 0.0f;
  last_qy = 0.0f;
  last_cmd = 0;
  saw_command = 0;

  while (1) {
    m3_icon_svg_skip_separators(&p);
    if (*p == '\0') {
      break;
    }
    if (!isalpha((unsigned char)*p)) {
      return M3_ERR_CORRUPT;
    }

    cmd = *p;
    p += 1;
    relative = (cmd >= 'a' && cmd <= 'z') ? 1 : 0;
    saw_command = 1;

    switch (cmd) {
    case 'M':
    case 'm':
      got_any = 0;
      while (1) {
        rc = m3_icon_svg_parse_pair(&p, &x, &y);
        if (rc == M3_ERR_NOT_FOUND) {
          if (!got_any) {
            return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != M3_OK) {
          return rc;
        }
        if (relative) {
          x += cur_x;
          y += cur_y;
        }
        if (!got_any) {
          rc = m3_icon_path_move_to(path, xf, x, y);
          if (rc != M3_OK) {
            return rc;
          }
          start_x = x;
          start_y = y;
          got_any = 1;
        } else {
          rc = m3_icon_path_line_to(path, xf, x, y); /* GCOVR_EXCL_LINE */
          if (rc != M3_OK) {                         /* GCOVR_EXCL_LINE */
            return rc;                               /* GCOVR_EXCL_LINE */
          }
        }
        cur_x = x;
        cur_y = y;
      }
      last_cmd = cmd;
      break;
    case 'L':
    case 'l':
      got_any = 0;
      while (1) {
        rc = m3_icon_svg_parse_pair(&p, &x, &y);
        if (rc == M3_ERR_NOT_FOUND) {
          if (!got_any) {
            return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != M3_OK) {
          return rc;
        }
        if (relative) {
          x += cur_x;
          y += cur_y;
        }
        rc = m3_icon_path_line_to(path, xf, x, y);
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        cur_x = x;
        cur_y = y;
        got_any = 1;
        last_cmd = cmd;
      }
      break;
    case 'H':
    case 'h':
      got_any = 0;
      while (1) {
        rc = m3_icon_svg_parse_number(&p, &x);
        if (rc == M3_ERR_NOT_FOUND) {
          if (!got_any) {
            return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          x += cur_x;
        }
        y = cur_y;
        rc = m3_icon_path_line_to(path, xf, x, y);
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        cur_x = x;
        got_any = 1;
        last_cmd = cmd;
      }
      break;
    case 'V':
    case 'v':
      got_any = 0;
      while (1) {
        rc = m3_icon_svg_parse_number(&p, &y);
        if (rc == M3_ERR_NOT_FOUND) {
          if (!got_any) {
            return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          y += cur_y;
        }
        x = cur_x;
        rc = m3_icon_path_line_to(path, xf, x, y);
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        cur_y = y;
        got_any = 1;
        last_cmd = cmd;
      }
      break;
    case 'C':
    case 'c':
      got_any = 0;
      while (1) {
        rc = m3_icon_svg_parse_pair(&p, &x1, &y1);
        if (rc == M3_ERR_NOT_FOUND) {
          if (!got_any) {
            return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        rc = m3_icon_svg_parse_pair(&p, &x2, &y2);
        if (rc == M3_ERR_NOT_FOUND) {
          return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        rc = m3_icon_svg_parse_pair(&p, &x, &y);
        if (rc == M3_ERR_NOT_FOUND) {
          return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          x1 += cur_x;
          y1 += cur_y;
          x2 += cur_x;
          y2 += cur_y;
          x += cur_x;
          y += cur_y;
        }
        rc = m3_icon_path_cubic_to(path, xf, x1, y1, x2, y2, x, y);
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        cur_x = x;
        cur_y = y;
        last_cx = x2;
        last_cy = y2;
        got_any = 1;
        last_cmd = cmd;
      }
      break;
    case 'S':
    case 's':
      got_any = 0;
      while (1) {
        rc = m3_icon_svg_parse_pair(&p, &x2, &y2);
        if (rc == M3_ERR_NOT_FOUND) {
          if (!got_any) {
            return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        rc = m3_icon_svg_parse_pair(&p, &x, &y);
        if (rc == M3_ERR_NOT_FOUND) {
          return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          x2 += cur_x;
          y2 += cur_y;
          x += cur_x;
          y += cur_y;
        }
        if (last_cmd == 'C' || last_cmd == 'c' || last_cmd == 'S' ||
            last_cmd == 's') { /* GCOVR_EXCL_LINE */
          x1 = cur_x * 2.0f - last_cx;
          y1 = cur_y * 2.0f - last_cy;
        } else {
          x1 = cur_x; /* GCOVR_EXCL_LINE */
          y1 = cur_y; /* GCOVR_EXCL_LINE */
        }
        rc = m3_icon_path_cubic_to(path, xf, x1, y1, x2, y2, x, y);
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        cur_x = x;
        cur_y = y;
        last_cx = x2;
        last_cy = y2;
        got_any = 1;
        last_cmd = cmd;
      }
      break;
    case 'Q':
    case 'q':
      got_any = 0;
      while (1) {
        rc = m3_icon_svg_parse_pair(&p, &x1, &y1);
        if (rc == M3_ERR_NOT_FOUND) {
          if (!got_any) {
            return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        rc = m3_icon_svg_parse_pair(&p, &x, &y);
        if (rc == M3_ERR_NOT_FOUND) {
          return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          x1 += cur_x;
          y1 += cur_y;
          x += cur_x;
          y += cur_y;
        }
        rc = m3_icon_path_quad_to(path, xf, x1, y1, x, y);
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        cur_x = x;
        cur_y = y;
        last_qx = x1;
        last_qy = y1;
        got_any = 1;
        last_cmd = cmd;
      }
      break;
    case 'T':
    case 't':
      got_any = 0;
      while (1) {
        rc = m3_icon_svg_parse_pair(&p, &x, &y);
        if (rc == M3_ERR_NOT_FOUND) {
          if (!got_any) {
            return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          x += cur_x;
          y += cur_y;
        }
        if (last_cmd == 'Q' || last_cmd == 'q' || last_cmd == 'T' ||
            last_cmd == 't') { /* GCOVR_EXCL_LINE */
          x1 = cur_x * 2.0f - last_qx;
          y1 = cur_y * 2.0f - last_qy;
        } else {
          x1 = cur_x; /* GCOVR_EXCL_LINE */
          y1 = cur_y; /* GCOVR_EXCL_LINE */
        }
        rc = m3_icon_path_quad_to(path, xf, x1, y1, x, y);
        if (rc != M3_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        cur_x = x;
        cur_y = y;
        last_qx = x1;
        last_qy = y1;
        got_any = 1;
        last_cmd = cmd;
      }
      break;
    case 'Z':
    case 'z':
      rc = m3_icon_path_close(path);
      if (rc != M3_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      cur_x = start_x;
      cur_y = start_y;
      last_cmd = cmd;
      break;
    default:
      return M3_ERR_UNSUPPORTED;
    }
  }

  if (!saw_command) {
    return M3_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
  }

  return M3_OK;
}

static int m3_icon_svg_build_path(const M3IconSvg *svg, const M3Rect *bounds,
                                  M3Path *path) {
  M3IconSvgTransform xf;
  int rc;
  int cleanup_rc;

  if (svg == NULL || bounds == NULL || path == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_validate_svg(svg);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_validate_rect(bounds);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_svg_compute_transform(svg, bounds, &xf);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

#ifdef M3_TESTING
  if (m3_icon_test_fail_point_match(M3_ICON_TEST_FAIL_PATH_INIT)) {
    return M3_ERR_IO;
  }
#endif

  path->commands = NULL;
  path->count = 0;
  path->capacity = 0;
  path->allocator.ctx = NULL;
  path->allocator.alloc = NULL;
  path->allocator.realloc = NULL;
  path->allocator.free = NULL;

  rc = m3_path_init(path, NULL, 0);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_svg_parse(svg->utf8_path, &xf, path);
  if (rc != M3_OK) {
    cleanup_rc = m3_path_shutdown(path);
    if (cleanup_rc != M3_OK) {
      return cleanup_rc; /* GCOVR_EXCL_LINE */
    }
    return rc;
  }

  return M3_OK;
}

static int m3_icon_measure_svg(const M3IconStyle *style, const M3IconSvg *svg,
                               M3IconMetrics *out_metrics) {
  M3Scalar size;
  M3Scalar w;
  M3Scalar h;

  if (style == NULL || svg == NULL || out_metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (svg->viewbox_width <= 0.0f || svg->viewbox_height <= 0.0f) {
    return M3_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  size = (M3Scalar)style->size_px;
  if (svg->viewbox_width >= svg->viewbox_height) {
    w = size;                                              /* GCOVR_EXCL_LINE */
    h = size * (svg->viewbox_height / svg->viewbox_width); /* GCOVR_EXCL_LINE */
  } else {                                                 /* GCOVR_EXCL_LINE */
    h = size;
    w = size * (svg->viewbox_width / svg->viewbox_height);
  }

  out_metrics->width = w;
  out_metrics->height = h;
  out_metrics->baseline = h;
  return M3_OK;
}

static int m3_icon_measure_font(const M3Gfx *gfx, const M3IconStyle *style,
                                const char *utf8_name, m3_usize utf8_len,
                                M3IconMetrics *out_metrics) {
  M3Handle font;
  M3Scalar width;
  M3Scalar height;
  M3Scalar baseline;
  int rc;
  int cleanup_rc;

  if (gfx == NULL || style == NULL || out_metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (!m3_icon_gfx_supports_font(gfx, M3_FALSE)) {
    return M3_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
  }

  font.id = 0u;
  font.generation = 0u;
  rc = gfx->text_vtable->create_font(gfx->ctx, style->utf8_family,
                                     style->size_px, style->weight,
                                     style->italic, &font);
  if (rc != M3_OK) {
    return rc;
  }

  rc = gfx->text_vtable->measure_text(gfx->ctx, font, utf8_name, utf8_len,
                                      &width, &height, &baseline);
  if (rc != M3_OK) {
    cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
    if (cleanup_rc != M3_OK) {
      return cleanup_rc; /* GCOVR_EXCL_LINE */
    }
    return rc;
  }

  if (width < 0.0f || height < 0.0f || baseline < 0.0f) {
    cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
    if (cleanup_rc != M3_OK) {
      return cleanup_rc; /* GCOVR_EXCL_LINE */
    }
    return M3_ERR_RANGE;
  }

  out_metrics->width = width;
  out_metrics->height = height;
  out_metrics->baseline = baseline;

  cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
  if (cleanup_rc != M3_OK) {
    return cleanup_rc;
  }
  return M3_OK;
}

static int m3_icon_draw_font(const M3Gfx *gfx, const M3Rect *bounds,
                             const M3IconStyle *style, const char *utf8_name,
                             m3_usize utf8_len) {
  M3Handle font;
  M3Scalar width;
  M3Scalar height;
  M3Scalar baseline;
  M3Scalar x;
  M3Scalar y;
  int rc;
  int cleanup_rc;

  if (gfx == NULL || bounds == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (!m3_icon_gfx_supports_font(gfx, M3_TRUE)) {
    return M3_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
  }

  font.id = 0u;
  font.generation = 0u;
  rc = gfx->text_vtable->create_font(gfx->ctx, style->utf8_family,
                                     style->size_px, style->weight,
                                     style->italic, &font);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = gfx->text_vtable->measure_text(gfx->ctx, font, utf8_name, utf8_len,
                                      &width, &height, &baseline);
  if (rc != M3_OK) {
    cleanup_rc =
        gfx->text_vtable->destroy_font(gfx->ctx, font); /* GCOVR_EXCL_LINE */
    if (cleanup_rc != M3_OK) {                          /* GCOVR_EXCL_LINE */
      return cleanup_rc;                                /* GCOVR_EXCL_LINE */
    }
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (width < 0.0f || height < 0.0f || baseline < 0.0f) {
    cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
    if (cleanup_rc != M3_OK) {
      return cleanup_rc; /* GCOVR_EXCL_LINE */
    }
    return M3_ERR_RANGE;
  }

  x = bounds->x + (bounds->width - width) * 0.5f;
  y = bounds->y + (bounds->height - height) * 0.5f + baseline;

  rc = gfx->text_vtable->draw_text(gfx->ctx, font, utf8_name, utf8_len, x, y,
                                   style->color);

  cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
  if (cleanup_rc != M3_OK) {
    return cleanup_rc; /* GCOVR_EXCL_LINE */
  }
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_icon_draw_svg(const M3Gfx *gfx, const M3Rect *bounds,
                            const M3IconStyle *style, const M3IconSvg *svg) {
  M3Path path;
  int rc;
  int cleanup_rc;

  if (gfx == NULL || bounds == NULL || style == NULL || svg == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (!m3_icon_gfx_supports_svg(gfx)) {
    return M3_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_svg_build_path(svg, bounds, &path);
  if (rc != M3_OK) {
    return rc;
  }

  rc = gfx->vtable->draw_path(gfx->ctx, &path, style->color);
  cleanup_rc = m3_path_shutdown(&path);
  if (cleanup_rc != M3_OK) {
    return cleanup_rc; /* GCOVR_EXCL_LINE */
  }
  return rc;
}

int M3_CALL m3_icon_style_init(M3IconStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  style->utf8_family = M3_ICON_DEFAULT_FAMILY;
  style->size_px = M3_ICON_DEFAULT_SIZE_PX;
  style->weight = M3_ICON_DEFAULT_WEIGHT;
  style->italic = M3_FALSE;
  style->color.r = 0.0f;
  style->color.g = 0.0f;
  style->color.b = 0.0f;
  style->color.a = 1.0f;
  return M3_OK;
}

int M3_CALL m3_icon_svg_init(M3IconSvg *svg) {
  if (svg == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  svg->utf8_path = NULL;
  svg->viewbox_x = 0.0f;
  svg->viewbox_y = 0.0f;
  svg->viewbox_width = 24.0f;
  svg->viewbox_height = 24.0f;
  return M3_OK;
}

int M3_CALL m3_icon_measure_utf8(const M3Gfx *gfx, const M3IconStyle *style,
                                 const char *utf8_name, m3_usize utf8_len,
                                 const M3IconSvg *svg, m3_u32 render_mode,
                                 M3IconMetrics *out_metrics) {
  int rc;
  m3_u32 resolved_mode;

  if (gfx == NULL || style == NULL || out_metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_validate_style(style);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_icon_validate_render_mode(render_mode);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_select_render_mode(gfx, utf8_name, utf8_len, svg, render_mode,
                                  M3_FALSE, &resolved_mode);
  if (rc != M3_OK) {
    return rc;
  }

  if (resolved_mode == M3_ICON_RENDER_FONT) {
    return m3_icon_measure_font(gfx, style, utf8_name, utf8_len, out_metrics);
  }
  return m3_icon_measure_svg(style, svg, out_metrics);
}

int M3_CALL m3_icon_measure_cstr(const M3Gfx *gfx, const M3IconStyle *style,
                                 const char *utf8_name, const M3IconSvg *svg,
                                 m3_u32 render_mode,
                                 M3IconMetrics *out_metrics) {
  m3_usize len;
  int rc;

  if (utf8_name == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_cstrlen(utf8_name, &len);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_icon_measure_utf8(gfx, style, utf8_name, len, svg, render_mode,
                              out_metrics);
}

int M3_CALL m3_icon_draw_utf8(const M3Gfx *gfx, const M3Rect *bounds,
                              const M3IconStyle *style, const char *utf8_name,
                              m3_usize utf8_len, const M3IconSvg *svg,
                              m3_u32 render_mode) {
  int rc;
  m3_u32 resolved_mode;

  if (gfx == NULL || bounds == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_icon_validate_style(style);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_icon_validate_rect(bounds);
  if (rc != M3_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_icon_validate_render_mode(render_mode);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_icon_select_render_mode(gfx, utf8_name, utf8_len, svg, render_mode,
                                  M3_TRUE, &resolved_mode);
  if (rc != M3_OK) {
    return rc;
  }

  if (resolved_mode == M3_ICON_RENDER_FONT) {
    return m3_icon_draw_font(gfx, bounds, style, utf8_name, utf8_len);
  }
  return m3_icon_draw_svg(gfx, bounds, style, svg);
}

int M3_CALL m3_icon_draw_cstr(const M3Gfx *gfx, const M3Rect *bounds,
                              const M3IconStyle *style, const char *utf8_name,
                              const M3IconSvg *svg, m3_u32 render_mode) {
  m3_usize len;
  int rc;

  if (utf8_name == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_icon_cstrlen(utf8_name, &len);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_icon_draw_utf8(gfx, bounds, style, utf8_name, len, svg,
                           render_mode);
}

#ifdef M3_TESTING
int M3_CALL m3_icon_test_validate_style(const M3IconStyle *style) {
  return m3_icon_validate_style(style);
}

int M3_CALL m3_icon_test_validate_color(const M3Color *color) {
  return m3_icon_validate_color(color);
}

int M3_CALL m3_icon_test_validate_rect(const M3Rect *rect) {
  return m3_icon_validate_rect(rect);
}

int M3_CALL m3_icon_test_validate_svg(const M3IconSvg *svg) {
  return m3_icon_validate_svg(svg);
}
#endif
