/* clang-format off */
#include "cmp_css_color.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int cmp_strdup_safe(const char *src, char **dst) {
  size_t len;
  if (!dst)
    return -1;
  if (!src) {
    *dst = NULL;
    return 0;
  }
  len = strlen(src);
  *dst = (char *)malloc(len + 1);
  if (!*dst)
    return -1;
#if defined(_MSC_VER)
  strcpy_s(*dst, len + 1, src);
#else
  strcpy(*dst, src);
#endif
  return 0;
}

int cmp_prop_color_init_currentcolor(cmp_prop_color_t *color) {
  if (!color)
    return -1;
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = CMP_PROP_COLOR_CURRENTCOLOR;
  return 0;
}

int cmp_prop_color_init_transparent(cmp_prop_color_t *color) {
  if (!color)
    return -1;
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = CMP_PROP_COLOR_TRANSPARENT;
  return 0;
}

int cmp_prop_color_init_rgba(cmp_prop_color_t *color, float r, float g, float b,
                             float a) {
  if (!color)
    return -1;
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = CMP_PROP_COLOR_LEGACY;
  color->value.legacy.r = r;
  color->value.legacy.g = g;
  color->value.legacy.b = b;
  color->value.legacy.a = a;
  color->value.legacy.is_hsl = 0;
  return 0;
}

int cmp_prop_color_init_hsla(cmp_prop_color_t *color, float h, float s, float l,
                             float a) {
  if (!color)
    return -1;
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = CMP_PROP_COLOR_LEGACY;
  color->value.legacy.r = h;
  color->value.legacy.g = s;
  color->value.legacy.b = l;
  color->value.legacy.a = a;
  color->value.legacy.is_hsl = 1;
  return 0;
}

int cmp_prop_color_init_modern(cmp_prop_color_t *color,
                               cmp_color_modern_type_t modern_type, float c1,
                               float c2, float c3, float a) {
  if (!color)
    return -1;
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = CMP_PROP_COLOR_MODERN;
  color->value.modern.type = modern_type;
  color->value.modern.c1 = c1;
  color->value.modern.c2 = c2;
  color->value.modern.c3 = c3;
  color->value.modern.alpha = a;
  return 0;
}

int cmp_prop_color_init_space(cmp_prop_color_t *color,
                              cmp_color_space_type_t space_type, float p1,
                              float p2, float p3, float a) {
  if (!color)
    return -1;
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = CMP_PROP_COLOR_SPACE;
  color->value.space.space = space_type;
  color->value.space.p1 = p1;
  color->value.space.p2 = p2;
  color->value.space.p3 = p3;
  color->value.space.alpha = a;
  return 0;
}

int cmp_prop_color_init_system(cmp_prop_color_t *color, const char *sys_color) {
  int rc;
  if (!color || !sys_color)
    return -1;
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = CMP_PROP_COLOR_SYSTEM;
  rc = cmp_strdup_safe(sys_color, &color->value.system_color);
  if (rc != 0)
    return rc;
  return 0;
}

int cmp_prop_color_init_complex(cmp_prop_color_t *color,
                                cmp_prop_color_type_t func_type,
                                const char *func_string) {
  int rc;
  if (!color || !func_string)
    return -1;
  if (func_type != CMP_PROP_COLOR_MIX && func_type != CMP_PROP_COLOR_CONTRAST &&
      func_type != CMP_PROP_COLOR_RELATIVE) {
    return -1;
  }
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = func_type;
  rc = cmp_strdup_safe(func_string, &color->value.complex_func);
  if (rc != 0)
    return rc;
  return 0;
}

int cmp_prop_color_init_cmyk(cmp_prop_color_t *color, float c, float m, float y,
                             float k, float a, const char *fallback) {
  int rc;
  if (!color)
    return -1;
  memset(color, 0, sizeof(cmp_prop_color_t));
  color->type = CMP_PROP_COLOR_CMYK;
  color->value.cmyk.c = c;
  color->value.cmyk.m = m;
  color->value.cmyk.y = y;
  color->value.cmyk.k = k;
  color->value.cmyk.alpha = a;
  if (fallback) {
    rc = cmp_strdup_safe(fallback, &color->value.cmyk.fallback_color);
    if (rc != 0)
      return rc;
  } else {
    color->value.cmyk.fallback_color = NULL;
  }
  return 0;
}

int cmp_prop_color_free(cmp_prop_color_t *color) {
  if (!color)
    return -1;

  if (color->type == CMP_PROP_COLOR_SYSTEM) {
    if (color->value.system_color) {
      free(color->value.system_color);
      color->value.system_color = NULL;
    }
  } else if (color->type == CMP_PROP_COLOR_MIX ||
             color->type == CMP_PROP_COLOR_CONTRAST ||
             color->type == CMP_PROP_COLOR_RELATIVE) {
    if (color->value.complex_func) {
      free(color->value.complex_func);
      color->value.complex_func = NULL;
    }
  } else if (color->type == CMP_PROP_COLOR_CMYK) {
    if (color->value.cmyk.fallback_color) {
      free(color->value.cmyk.fallback_color);
      color->value.cmyk.fallback_color = NULL;
    }
  }
  return 0;
}