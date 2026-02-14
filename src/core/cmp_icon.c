#include "cmpc/cmp_icon.h"
#include "cmpc/cmp_path.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

#ifdef CMP_TESTING
#define CMP_ICON_TEST_FAIL_NONE 0u
#define CMP_ICON_TEST_FAIL_PATH_INIT 1u
#define CMP_ICON_TEST_FAIL_PATH_APPEND 2u

static cmp_u32 g_cmp_icon_test_fail_point = CMP_ICON_TEST_FAIL_NONE;
static cmp_usize g_cmp_icon_test_cstr_limit = 0;

int CMP_CALL cmp_icon_test_set_fail_point(cmp_u32 fail_point) {
  g_cmp_icon_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL cmp_icon_test_clear_fail_points(void) {
  g_cmp_icon_test_fail_point = CMP_ICON_TEST_FAIL_NONE;
  return CMP_OK;
}

int CMP_CALL cmp_icon_test_set_cstr_limit(cmp_usize max_len) {
  g_cmp_icon_test_cstr_limit = max_len;
  return CMP_OK;
}

static int cmp_icon_test_fail_point_match(cmp_u32 point) {
  if (g_cmp_icon_test_fail_point != point) {
    return 0;
  }
  g_cmp_icon_test_fail_point = CMP_ICON_TEST_FAIL_NONE;
  return 1;
}
#endif /* GCOVR_EXCL_LINE */

typedef struct CMPIconSvgTransform {
  CMPScalar scale;
  CMPScalar offset_x;
  CMPScalar offset_y;
  CMPScalar viewbox_x;
  CMPScalar viewbox_y;
} CMPIconSvgTransform;

static int cmp_icon_validate_color(const CMPColor *color) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_icon_validate_style(const CMPIconStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return CMP_ERR_RANGE;
  }
  if (style->italic != CMP_FALSE && style->italic != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_icon_validate_color(&style->color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int cmp_icon_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_icon_validate_svg(const CMPIconSvg *svg) {
  if (svg == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (svg->utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (svg->viewbox_width <= 0.0f || svg->viewbox_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static cmp_usize cmp_icon_cstr_limit(void) {
  cmp_usize max_len;

  max_len = (cmp_usize) ~(cmp_usize)0;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (g_cmp_icon_test_cstr_limit != 0) {
    max_len = g_cmp_icon_test_cstr_limit;
  }
#endif /* GCOVR_EXCL_LINE */
  return max_len;
}

static int cmp_icon_cstrlen(const char *cstr, cmp_usize *out_len) {
  cmp_usize length;
  cmp_usize max_len;

  if (cstr == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  max_len = cmp_icon_cstr_limit();
  length = 0;
  while (cstr[length] != '\0') {
    if (length == max_len) {
      return CMP_ERR_OVERFLOW;
    }
    length += 1;
  }
  *out_len = length;
  return CMP_OK;
}

static int cmp_icon_validate_render_mode(cmp_u32 render_mode) {
  if (render_mode != CMP_ICON_RENDER_AUTO &&
      render_mode != CMP_ICON_RENDER_FONT &&
      render_mode != CMP_ICON_RENDER_SVG) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int cmp_icon_gfx_supports_font(const CMPGfx *gfx, CMPBool require_draw) {
  if (gfx == NULL || gfx->text_vtable == NULL) {
    return 0;
  }
  if (gfx->text_vtable->create_font == NULL ||
      gfx->text_vtable->measure_text == NULL ||
      gfx->text_vtable->destroy_font == NULL) {
    return 0;
  }
  if (require_draw == CMP_TRUE && gfx->text_vtable->draw_text == NULL) {
    return 0;
  }
  return 1;
}

static int cmp_icon_gfx_supports_svg(const CMPGfx *gfx) {
  if (gfx == NULL || gfx->vtable == NULL || gfx->vtable->draw_path == NULL) {
    return 0;
  }
  return 1;
}

static int cmp_icon_select_render_mode(const CMPGfx *gfx, const char *utf8_name,
                                       cmp_usize utf8_len,
                                       const CMPIconSvg *svg,
                                       cmp_u32 render_mode,
                                       CMPBool require_draw,
                                       cmp_u32 *out_mode) {
  int has_font;
  int has_svg;
  int rc;

  if (out_mode == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  has_font = cmp_icon_gfx_supports_font(gfx, require_draw);
  has_svg = cmp_icon_gfx_supports_svg(gfx);

  if (render_mode == CMP_ICON_RENDER_FONT) {
    if (!has_font) {
      return CMP_ERR_UNSUPPORTED;
    }
    if (utf8_name == NULL) {
      return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
    }
    *out_mode = CMP_ICON_RENDER_FONT;
    return CMP_OK;
  }

  if (render_mode == CMP_ICON_RENDER_SVG) {
    if (!has_svg) {
      return CMP_ERR_UNSUPPORTED;
    }
    rc = cmp_icon_validate_svg(svg);
    if (rc != CMP_OK) {
      return rc;
    }
    *out_mode = CMP_ICON_RENDER_SVG;
    return CMP_OK;
  }

  if (has_font && utf8_name != NULL) {
    *out_mode = CMP_ICON_RENDER_FONT;
    return CMP_OK;
  }
  if (has_svg && svg != NULL) {
    rc = cmp_icon_validate_svg(svg);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    *out_mode = CMP_ICON_RENDER_SVG;
    return CMP_OK;
  }
  return CMP_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
}

static void cmp_icon_svg_transform_point(const CMPIconSvgTransform *xf,
                                         CMPScalar x, CMPScalar y,
                                         CMPScalar *out_x, CMPScalar *out_y) {
  *out_x = (x - xf->viewbox_x) * xf->scale + xf->offset_x;
  *out_y = (y - xf->viewbox_y) * xf->scale + xf->offset_y;
}

static void cmp_icon_svg_skip_separators(const char **cursor) {
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

static int cmp_icon_svg_parse_number(const char **cursor,
                                     CMPScalar *out_value) {
  const char *p;
  char *endptr;
  double value;

  if (cursor == NULL || *cursor == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  p = *cursor;
  cmp_icon_svg_skip_separators(&p);
  if (*p == '\0') {
    *cursor = p;
    return CMP_ERR_NOT_FOUND;
  }

  errno = 0;
  value = strtod(p, &endptr);
  if (endptr == p) {
    *cursor = p;
    return CMP_ERR_NOT_FOUND;
  }
  if (errno == ERANGE) {
    *cursor = endptr;
    return CMP_ERR_RANGE;
  }
  if (value != value) {
    *cursor = endptr;
    return CMP_ERR_RANGE;
  }
  if (value > (double)FLT_MAX || value < (double)-FLT_MAX) {
    *cursor = endptr;     /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  *out_value = (CMPScalar)value;
  *cursor = endptr;
  return CMP_OK;
}

static int cmp_icon_svg_parse_pair(const char **cursor, CMPScalar *out_x,
                                   CMPScalar *out_y) {
  int rc;

  rc = cmp_icon_svg_parse_number(cursor, out_x);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_icon_svg_parse_number(cursor, out_y);
  if (rc == CMP_ERR_NOT_FOUND) {
    return CMP_ERR_CORRUPT;
  }
  return rc;
}

static int cmp_icon_map_path_result(int rc) {
  if (rc == CMP_ERR_STATE) {
    return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
  }
  return rc;
}

static int cmp_icon_path_move_to(CMPPath *path, const CMPIconSvgTransform *xf,
                                 CMPScalar x, CMPScalar y) {
  CMPScalar tx;
  CMPScalar ty;
  int rc;

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_icon_test_fail_point_match(CMP_ICON_TEST_FAIL_PATH_APPEND)) {
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

  cmp_icon_svg_transform_point(xf, x, y, &tx, &ty);
  rc = cmp_path_move_to(path, tx, ty);
  return cmp_icon_map_path_result(rc);
}

static int cmp_icon_path_line_to(CMPPath *path, const CMPIconSvgTransform *xf,
                                 CMPScalar x, CMPScalar y) {
  CMPScalar tx;
  CMPScalar ty;
  int rc;

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_icon_test_fail_point_match(CMP_ICON_TEST_FAIL_PATH_APPEND)) {
    return CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */

  cmp_icon_svg_transform_point(xf, x, y, &tx, &ty);
  rc = cmp_path_line_to(path, tx, ty);
  return cmp_icon_map_path_result(rc);
}

static int cmp_icon_path_quad_to(CMPPath *path, const CMPIconSvgTransform *xf,
                                 CMPScalar cx, CMPScalar cy, CMPScalar x,
                                 CMPScalar y) {
  CMPScalar tcx;
  CMPScalar tcy;
  CMPScalar tx;
  CMPScalar ty;
  int rc;

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_icon_test_fail_point_match(CMP_ICON_TEST_FAIL_PATH_APPEND)) {
    return CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */

  cmp_icon_svg_transform_point(xf, cx, cy, &tcx, &tcy);
  cmp_icon_svg_transform_point(xf, x, y, &tx, &ty);
  rc = cmp_path_quad_to(path, tcx, tcy, tx, ty);
  return cmp_icon_map_path_result(rc);
}

static int cmp_icon_path_cubic_to(CMPPath *path, const CMPIconSvgTransform *xf,
                                  CMPScalar cx1, CMPScalar cy1, CMPScalar cx2,
                                  CMPScalar cy2, CMPScalar x, CMPScalar y) {
  CMPScalar tcx1;
  CMPScalar tcy1;
  CMPScalar tcx2;
  CMPScalar tcy2;
  CMPScalar tx;
  CMPScalar ty;
  int rc;

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_icon_test_fail_point_match(CMP_ICON_TEST_FAIL_PATH_APPEND)) {
    return CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */

  cmp_icon_svg_transform_point(xf, cx1, cy1, &tcx1, &tcy1);
  cmp_icon_svg_transform_point(xf, cx2, cy2, &tcx2, &tcy2);
  cmp_icon_svg_transform_point(xf, x, y, &tx, &ty);
  rc = cmp_path_cubic_to(path, tcx1, tcy1, tcx2, tcy2, tx, ty);
  return cmp_icon_map_path_result(rc);
}

static int cmp_icon_path_close(CMPPath *path) {
  int rc;

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_icon_test_fail_point_match(CMP_ICON_TEST_FAIL_PATH_APPEND)) {
    return CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */

  rc = cmp_path_close(path);
  return cmp_icon_map_path_result(rc);
}

static int cmp_icon_svg_compute_transform(const CMPIconSvg *svg,
                                          const CMPRect *bounds,
                                          CMPIconSvgTransform *out_xf) {
  CMPScalar scale_x;
  CMPScalar scale_y;
  CMPScalar scale;
  CMPScalar scaled_w;
  CMPScalar scaled_h;

  if (svg == NULL || bounds == NULL || out_xf == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (svg->viewbox_width <= 0.0f || svg->viewbox_height <= 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
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
  return CMP_OK;
}

static int cmp_icon_svg_parse(const char *data, const CMPIconSvgTransform *xf,
                              CMPPath *path) {
  const char *p;
  CMPScalar cur_x;
  CMPScalar cur_y;
  CMPScalar start_x;
  CMPScalar start_y;
  CMPScalar last_cx;
  CMPScalar last_cy;
  CMPScalar last_qx;
  CMPScalar last_qy;
  CMPScalar x;
  CMPScalar y;
  CMPScalar x1;
  CMPScalar y1;
  CMPScalar x2;
  CMPScalar y2;
  int rc;
  int relative;
  int saw_command;
  int got_any;
  char cmd;
  char last_cmd;

  if (data == NULL || xf == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
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
    cmp_icon_svg_skip_separators(&p);
    if (*p == '\0') {
      break;
    }
    if (!isalpha((unsigned char)*p)) {
      return CMP_ERR_CORRUPT;
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
        rc = cmp_icon_svg_parse_pair(&p, &x, &y);
        if (rc == CMP_ERR_NOT_FOUND) {
          if (!got_any) {
            return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != CMP_OK) {
          return rc;
        }
        if (relative) {
          x += cur_x;
          y += cur_y;
        }
        if (!got_any) {
          rc = cmp_icon_path_move_to(path, xf, x, y);
          if (rc != CMP_OK) {
            return rc;
          }
          start_x = x;
          start_y = y;
          got_any = 1;
        } else {
          rc = cmp_icon_path_line_to(path, xf, x, y); /* GCOVR_EXCL_LINE */
          if (rc != CMP_OK) {                         /* GCOVR_EXCL_LINE */
            return rc;                                /* GCOVR_EXCL_LINE */
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
        rc = cmp_icon_svg_parse_pair(&p, &x, &y);
        if (rc == CMP_ERR_NOT_FOUND) {
          if (!got_any) {
            return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != CMP_OK) {
          return rc;
        }
        if (relative) {
          x += cur_x;
          y += cur_y;
        }
        rc = cmp_icon_path_line_to(path, xf, x, y);
        if (rc != CMP_OK) {
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
        rc = cmp_icon_svg_parse_number(&p, &x);
        if (rc == CMP_ERR_NOT_FOUND) {
          if (!got_any) {
            return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          x += cur_x;
        }
        y = cur_y;
        rc = cmp_icon_path_line_to(path, xf, x, y);
        if (rc != CMP_OK) {
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
        rc = cmp_icon_svg_parse_number(&p, &y);
        if (rc == CMP_ERR_NOT_FOUND) {
          if (!got_any) {
            return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          y += cur_y;
        }
        x = cur_x;
        rc = cmp_icon_path_line_to(path, xf, x, y);
        if (rc != CMP_OK) {
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
        rc = cmp_icon_svg_parse_pair(&p, &x1, &y1);
        if (rc == CMP_ERR_NOT_FOUND) {
          if (!got_any) {
            return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        rc = cmp_icon_svg_parse_pair(&p, &x2, &y2);
        if (rc == CMP_ERR_NOT_FOUND) {
          return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
        }
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        rc = cmp_icon_svg_parse_pair(&p, &x, &y);
        if (rc == CMP_ERR_NOT_FOUND) {
          return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
        }
        if (rc != CMP_OK) {
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
        rc = cmp_icon_path_cubic_to(path, xf, x1, y1, x2, y2, x, y);
        if (rc != CMP_OK) {
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
        rc = cmp_icon_svg_parse_pair(&p, &x2, &y2);
        if (rc == CMP_ERR_NOT_FOUND) {
          if (!got_any) {
            return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        rc = cmp_icon_svg_parse_pair(&p, &x, &y);
        if (rc == CMP_ERR_NOT_FOUND) {
          return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
        }
        if (rc != CMP_OK) {
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
        rc = cmp_icon_path_cubic_to(path, xf, x1, y1, x2, y2, x, y);
        if (rc != CMP_OK) {
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
        rc = cmp_icon_svg_parse_pair(&p, &x1, &y1);
        if (rc == CMP_ERR_NOT_FOUND) {
          if (!got_any) {
            return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        rc = cmp_icon_svg_parse_pair(&p, &x, &y);
        if (rc == CMP_ERR_NOT_FOUND) {
          return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
        }
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        if (relative) {
          x1 += cur_x;
          y1 += cur_y;
          x += cur_x;
          y += cur_y;
        }
        rc = cmp_icon_path_quad_to(path, xf, x1, y1, x, y);
        if (rc != CMP_OK) {
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
        rc = cmp_icon_svg_parse_pair(&p, &x, &y);
        if (rc == CMP_ERR_NOT_FOUND) {
          if (!got_any) {
            return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
          }
          break;
        }
        if (rc != CMP_OK) {
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
        rc = cmp_icon_path_quad_to(path, xf, x1, y1, x, y);
        if (rc != CMP_OK) {
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
      rc = cmp_icon_path_close(path);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      cur_x = start_x;
      cur_y = start_y;
      last_cmd = cmd;
      break;
    default:
      return CMP_ERR_UNSUPPORTED;
    }
  }

  if (!saw_command) {
    return CMP_ERR_CORRUPT; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int cmp_icon_svg_build_path(const CMPIconSvg *svg, const CMPRect *bounds,
                                   CMPPath *path) {
  CMPIconSvgTransform xf;
  int rc;
  int cleanup_rc;

  if (svg == NULL || bounds == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_validate_svg(svg);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_svg_compute_transform(svg, bounds, &xf);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_icon_test_fail_point_match(CMP_ICON_TEST_FAIL_PATH_INIT)) {
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

  path->commands = NULL;
  path->count = 0;
  path->capacity = 0;
  path->allocator.ctx = NULL;
  path->allocator.alloc = NULL;
  path->allocator.realloc = NULL;
  path->allocator.free = NULL;

  rc = cmp_path_init(path, NULL, 0);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_svg_parse(svg->utf8_path, &xf, path);
  if (rc != CMP_OK) {
    cleanup_rc = cmp_path_shutdown(path);
    if (cleanup_rc != CMP_OK) {
      return cleanup_rc; /* GCOVR_EXCL_LINE */
    }
    return rc;
  }

  return CMP_OK;
}

static int cmp_icon_measure_svg(const CMPIconStyle *style,
                                const CMPIconSvg *svg,
                                CMPIconMetrics *out_metrics) {
  CMPScalar size;
  CMPScalar w;
  CMPScalar h;

  if (style == NULL || svg == NULL || out_metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (svg->viewbox_width <= 0.0f || svg->viewbox_height <= 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  size = (CMPScalar)style->size_px;
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
  return CMP_OK;
}

static int cmp_icon_measure_font(const CMPGfx *gfx, const CMPIconStyle *style,
                                 const char *utf8_name, cmp_usize utf8_len,
                                 CMPIconMetrics *out_metrics) {
  CMPHandle font;
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  int rc;
  int cleanup_rc;

  if (gfx == NULL || style == NULL || out_metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (!cmp_icon_gfx_supports_font(gfx, CMP_FALSE)) {
    return CMP_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
  }

  font.id = 0u;
  font.generation = 0u;
  rc = gfx->text_vtable->create_font(gfx->ctx, style->utf8_family,
                                     style->size_px, style->weight,
                                     style->italic, &font);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = gfx->text_vtable->measure_text(gfx->ctx, font, utf8_name, utf8_len,
                                      &width, &height, &baseline);
  if (rc != CMP_OK) {
    cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
    if (cleanup_rc != CMP_OK) {
      return cleanup_rc; /* GCOVR_EXCL_LINE */
    }
    return rc;
  }

  if (width < 0.0f || height < 0.0f || baseline < 0.0f) {
    cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
    if (cleanup_rc != CMP_OK) {
      return cleanup_rc; /* GCOVR_EXCL_LINE */
    }
    return CMP_ERR_RANGE;
  }

  out_metrics->width = width;
  out_metrics->height = height;
  out_metrics->baseline = baseline;

  cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
  if (cleanup_rc != CMP_OK) {
    return cleanup_rc;
  }
  return CMP_OK;
}

static int cmp_icon_draw_font(const CMPGfx *gfx, const CMPRect *bounds,
                              const CMPIconStyle *style, const char *utf8_name,
                              cmp_usize utf8_len) {
  CMPHandle font;
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  CMPScalar x;
  CMPScalar y;
  int rc;
  int cleanup_rc;

  if (gfx == NULL || bounds == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (!cmp_icon_gfx_supports_font(gfx, CMP_TRUE)) {
    return CMP_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
  }

  font.id = 0u;
  font.generation = 0u;
  rc = gfx->text_vtable->create_font(gfx->ctx, style->utf8_family,
                                     style->size_px, style->weight,
                                     style->italic, &font);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = gfx->text_vtable->measure_text(gfx->ctx, font, utf8_name, utf8_len,
                                      &width, &height, &baseline);
  if (rc != CMP_OK) {
    cleanup_rc =
        gfx->text_vtable->destroy_font(gfx->ctx, font); /* GCOVR_EXCL_LINE */
    if (cleanup_rc != CMP_OK) {                         /* GCOVR_EXCL_LINE */
      return cleanup_rc;                                /* GCOVR_EXCL_LINE */
    }
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (width < 0.0f || height < 0.0f || baseline < 0.0f) {
    cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
    if (cleanup_rc != CMP_OK) {
      return cleanup_rc; /* GCOVR_EXCL_LINE */
    }
    return CMP_ERR_RANGE;
  }

  x = bounds->x + (bounds->width - width) * 0.5f;
  y = bounds->y + (bounds->height - height) * 0.5f + baseline;

  rc = gfx->text_vtable->draw_text(gfx->ctx, font, utf8_name, utf8_len, x, y,
                                   style->color);

  cleanup_rc = gfx->text_vtable->destroy_font(gfx->ctx, font);
  if (cleanup_rc != CMP_OK) {
    return cleanup_rc; /* GCOVR_EXCL_LINE */
  }
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int cmp_icon_draw_svg(const CMPGfx *gfx, const CMPRect *bounds,
                             const CMPIconStyle *style, const CMPIconSvg *svg) {
  CMPPath path;
  int rc;
  int cleanup_rc;

  if (gfx == NULL || bounds == NULL || style == NULL || svg == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (!cmp_icon_gfx_supports_svg(gfx)) {
    return CMP_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_svg_build_path(svg, bounds, &path);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = gfx->vtable->draw_path(gfx->ctx, &path, style->color);
  cleanup_rc = cmp_path_shutdown(&path);
  if (cleanup_rc != CMP_OK) {
    return cleanup_rc; /* GCOVR_EXCL_LINE */
  }
  return rc;
}

int CMP_CALL cmp_icon_style_init(CMPIconStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style->utf8_family = CMP_ICON_DEFAULT_FAMILY;
  style->size_px = CMP_ICON_DEFAULT_SIZE_PX;
  style->weight = CMP_ICON_DEFAULT_WEIGHT;
  style->italic = CMP_FALSE;
  style->color.r = 0.0f;
  style->color.g = 0.0f;
  style->color.b = 0.0f;
  style->color.a = 1.0f;
  return CMP_OK;
}

int CMP_CALL cmp_icon_svg_init(CMPIconSvg *svg) {
  if (svg == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  svg->utf8_path = NULL;
  svg->viewbox_x = 0.0f;
  svg->viewbox_y = 0.0f;
  svg->viewbox_width = 24.0f;
  svg->viewbox_height = 24.0f;
  return CMP_OK;
}

int CMP_CALL cmp_icon_measure_utf8(const CMPGfx *gfx, const CMPIconStyle *style,
                                   const char *utf8_name, cmp_usize utf8_len,
                                   const CMPIconSvg *svg, cmp_u32 render_mode,
                                   CMPIconMetrics *out_metrics) {
  int rc;
  cmp_u32 resolved_mode;

  if (gfx == NULL || style == NULL || out_metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_validate_style(style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = cmp_icon_validate_render_mode(render_mode);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_select_render_mode(gfx, utf8_name, utf8_len, svg, render_mode,
                                   CMP_FALSE, &resolved_mode);
  if (rc != CMP_OK) {
    return rc;
  }

  if (resolved_mode == CMP_ICON_RENDER_FONT) {
    return cmp_icon_measure_font(gfx, style, utf8_name, utf8_len, out_metrics);
  }
  return cmp_icon_measure_svg(style, svg, out_metrics);
}

int CMP_CALL cmp_icon_measure_cstr(const CMPGfx *gfx, const CMPIconStyle *style,
                                   const char *utf8_name, const CMPIconSvg *svg,
                                   cmp_u32 render_mode,
                                   CMPIconMetrics *out_metrics) {
  cmp_usize len;
  int rc;

  if (utf8_name == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_cstrlen(utf8_name, &len);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_icon_measure_utf8(gfx, style, utf8_name, len, svg, render_mode,
                               out_metrics);
}

int CMP_CALL cmp_icon_draw_utf8(const CMPGfx *gfx, const CMPRect *bounds,
                                const CMPIconStyle *style,
                                const char *utf8_name, cmp_usize utf8_len,
                                const CMPIconSvg *svg, cmp_u32 render_mode) {
  int rc;
  cmp_u32 resolved_mode;

  if (gfx == NULL || bounds == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_name == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_icon_validate_style(style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = cmp_icon_validate_rect(bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = cmp_icon_validate_render_mode(render_mode);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_icon_select_render_mode(gfx, utf8_name, utf8_len, svg, render_mode,
                                   CMP_TRUE, &resolved_mode);
  if (rc != CMP_OK) {
    return rc;
  }

  if (resolved_mode == CMP_ICON_RENDER_FONT) {
    return cmp_icon_draw_font(gfx, bounds, style, utf8_name, utf8_len);
  }
  return cmp_icon_draw_svg(gfx, bounds, style, svg);
}

int CMP_CALL cmp_icon_draw_cstr(const CMPGfx *gfx, const CMPRect *bounds,
                                const CMPIconStyle *style,
                                const char *utf8_name, const CMPIconSvg *svg,
                                cmp_u32 render_mode) {
  cmp_usize len;
  int rc;

  if (utf8_name == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_icon_cstrlen(utf8_name, &len);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_icon_draw_utf8(gfx, bounds, style, utf8_name, len, svg,
                            render_mode);
}

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
int CMP_CALL cmp_icon_test_cstrlen(const char *cstr, cmp_usize *out_len) {
  int (*fn)(const char *, cmp_usize *) = cmp_icon_cstrlen;
  return fn(cstr, out_len);
}

void CMP_CALL cmp_icon_test_svg_skip_separators(const char **cursor) {
  void (*fn)(const char **) = cmp_icon_svg_skip_separators;
  fn(cursor);
}

int CMP_CALL cmp_icon_test_svg_parse_number(const char **cursor,
                                            CMPScalar *out_value) {
  int (*fn)(const char **, CMPScalar *) = cmp_icon_svg_parse_number;
  return fn(cursor, out_value);
}

int CMP_CALL cmp_icon_test_map_path_result(int rc) {
  int (*fn)(int) = cmp_icon_map_path_result;
  return fn(rc);
}

int CMP_CALL cmp_icon_test_gfx_supports_font(const CMPGfx *gfx,
                                             CMPBool require_draw) {
  int (*fn)(const CMPGfx *, CMPBool) = cmp_icon_gfx_supports_font;
  return fn(gfx, require_draw);
}

int CMP_CALL cmp_icon_test_gfx_supports_svg(const CMPGfx *gfx) {
  int (*fn)(const CMPGfx *) = cmp_icon_gfx_supports_svg;
  return fn(gfx);
}

int CMP_CALL cmp_icon_test_select_render_mode(
    const CMPGfx *gfx, const char *utf8_name, cmp_usize utf8_len,
    const CMPIconSvg *svg, cmp_u32 render_mode, CMPBool require_draw,
    cmp_u32 *out_mode) {
  int (*fn)(const CMPGfx *, const char *, cmp_usize, const CMPIconSvg *,
            cmp_u32, CMPBool, cmp_u32 *) = cmp_icon_select_render_mode;
  return fn(gfx, utf8_name, utf8_len, svg, render_mode, require_draw, out_mode);
}

int CMP_CALL cmp_icon_test_svg_compute_transform(const CMPIconSvg *svg,
                                                 const CMPRect *bounds,
                                                 CMPScalar *out_scale,
                                                 CMPScalar *out_offset_x,
                                                 CMPScalar *out_offset_y) {
  CMPIconSvgTransform xf;
  CMPIconSvgTransform *out_xf = &xf;
  int rc;

  if (out_scale == NULL || out_offset_x == NULL || out_offset_y == NULL) {
    out_xf = NULL;
  }

  {
    int (*fn)(const CMPIconSvg *, const CMPRect *, CMPIconSvgTransform *) =
        cmp_icon_svg_compute_transform;
    rc = fn(svg, bounds, out_xf);
  }
  if (rc != CMP_OK) {
    return rc;
  }

  *out_scale = xf.scale;
  *out_offset_x = xf.offset_x;
  *out_offset_y = xf.offset_y;
  return CMP_OK;
}

int CMP_CALL cmp_icon_test_svg_parse(const char *data, CMPBool provide_xf,
                                     CMPBool provide_path) {
  CMPIconSvgTransform xf;
  CMPIconSvgTransform *xf_ptr;
  CMPPath path;
  CMPPath *path_ptr;
  int rc;

  xf_ptr = provide_xf ? &xf : NULL;
  path_ptr = provide_path ? &path : NULL;
  if (xf_ptr != NULL) {
    xf.scale = 1.0f;
    xf.offset_x = 0.0f;
    xf.offset_y = 0.0f;
    xf.viewbox_x = 0.0f;
    xf.viewbox_y = 0.0f;
  }

  if (path_ptr != NULL) {
    memset(&path, 0, sizeof(path));
    rc = cmp_path_init(&path, NULL, 0);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  {
    int (*fn)(const char *, const CMPIconSvgTransform *, CMPPath *) =
        cmp_icon_svg_parse;
    rc = fn(data, xf_ptr, path_ptr);
  }

  if (path_ptr != NULL) {
    int cleanup_rc = cmp_path_shutdown(&path);
    if (cleanup_rc != CMP_OK && rc == CMP_OK) {
      rc = cleanup_rc;
    }
  }

  return rc;
}

int CMP_CALL cmp_icon_test_validate_style(const CMPIconStyle *style) {
  return cmp_icon_validate_style(style);
}

int CMP_CALL cmp_icon_test_validate_color(const CMPColor *color) {
  return cmp_icon_validate_color(color);
}

int CMP_CALL cmp_icon_test_validate_rect(const CMPRect *rect) {
  return cmp_icon_validate_rect(rect);
}

int CMP_CALL cmp_icon_test_validate_svg(const CMPIconSvg *svg) {
  return cmp_icon_validate_svg(svg);
}
#endif /* GCOVR_EXCL_LINE */
