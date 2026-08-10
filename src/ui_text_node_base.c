/* clang-format off */
#include "ui_text_node_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_font_fail;
static ui_error_t mock_ui_font_manager_find_font(struct ui_font_manager *m,
                                                 const char *f, int w, int i,
                                                 struct ui_font **out) {
  (void)m;
  (void)f;
  (void)w;
  (void)i;
  if (g_mock_font_fail == 1)
    return UI_ERROR_NOT_FOUND;
  *out = (struct ui_font *)1; /* Fake font */
  return UI_ERROR_NONE;
}
#define ui_font_manager_find_font mock_ui_font_manager_find_font

static ui_error_t mock_ui_text_layout_shape(struct ui_text_layout *layout,
                                            struct ui_font *font, float size,
                                            const char *text, float max_width,
                                            int dir) {
  (void)layout;
  (void)font;
  (void)size;
  (void)max_width;
  (void)dir;
  if (g_mock_font_fail == 2)
    return UI_ERROR_UNKNOWN;
  if (g_mock_font_fail == 4 && strstr(text, "..."))
    return UI_ERROR_UNKNOWN; /* Fail specifically during ellipsis truncation */
  return UI_ERROR_NONE;
}
#define ui_text_layout_shape mock_ui_text_layout_shape

static ui_error_t mock_ui_text_layout_get_bounds(struct ui_text_layout *layout,
                                                 float *w, float *h) {
  static int calls = 0;
  (void)layout;
  if (g_mock_font_fail == 3)
    return UI_ERROR_UNKNOWN;
  if (g_mock_font_fail == 5) {
    if (++calls == 2) {
      calls = 0;
      return UI_ERROR_UNKNOWN;
    }
  } else {
    calls = 0;
  }
  *w = 100.0f;
  *h = 50.0f; /* Make it large enough to trigger overflow (e.g. line height 10,
                 max lines 2 -> 20) */
  return UI_ERROR_NONE;
}
#define ui_text_layout_get_bounds mock_ui_text_layout_get_bounds

static ui_error_t mock_ui_font_get_vmetrics(struct ui_font *font, float size,
                                            float *ascent, float *descent,
                                            float *line_gap) {
  (void)font;
  (void)size;
  if (g_mock_font_fail == 6)
    return UI_ERROR_UNKNOWN;
  *ascent = 10.0f;
  *descent = 0.0f;
  *line_gap = 0.0f;
  return UI_ERROR_NONE;
}
#define ui_font_get_vmetrics mock_ui_font_get_vmetrics

#endif

/** \brief ui_text_node_base */
struct ui_text_node_base {
  struct ui_component *component;
  struct ui_text_layout *layout;
  struct ui_font_manager *font_manager;

  char *text;
  char *font_family;
  float font_size;
  float max_width;
  int max_lines;
  enum ui_text_node_overflow overflow;

  float computed_width;
  float computed_height;
  struct ui_signal *text_signal;
};

ui_error_t ui_text_node_base_create(struct ui_text_node_base **out_node) {
  ui_error_t rc;
  struct ui_text_node_base *node;
  struct ui_dom_node *dom_node = NULL;

  if (!out_node)
    return UI_ERROR_INVALID_ARGUMENT;

  node = (struct ui_text_node_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_text_node_base));
  if (!node)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(node, 0, sizeof(struct ui_text_node_base));
  node->font_size = 16.0f;
  node->max_width = 0.0f;
  node->max_lines = 0;
  node->overflow = UI_TEXT_NODE_OVERFLOW_CLIP;

  rc = ui_component_create(&node->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(node);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dom_node);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(node->component);
    C_MULTIPLATFORM_FREE(node);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(dom_node, "span");
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(dom_node);
    (void)ui_component_destroy(node->component);
    C_MULTIPLATFORM_FREE(node);
    return rc;
  }
  node->component->shadow_root = dom_node;

  rc = ui_text_layout_create(&node->layout);
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(dom_node);
    node->component->shadow_root = NULL;
    (void)ui_component_destroy(node->component);
    C_MULTIPLATFORM_FREE(node);
    return rc;
  }

  *out_node = node;
  return UI_ERROR_NONE;
}

ui_error_t ui_text_node_base_destroy(struct ui_text_node_base *node) {
  if (!node)
    return UI_ERROR_NONE;
  if (node->layout)
    (void)ui_text_layout_destroy(node->layout);
  if (node->text)
    C_MULTIPLATFORM_FREE(node->text);
  if (node->font_family)
    C_MULTIPLATFORM_FREE(node->font_family);
  if (node->component) {
    if (node->component->shadow_root) {
      (void)ui_dom_node_destroy(node->component->shadow_root);
      node->component->shadow_root = NULL;
    }
    (void)ui_component_destroy(node->component);
  }
  C_MULTIPLATFORM_FREE(node);
  return UI_ERROR_NONE;
}

ui_error_t ui_text_node_base_set_text(struct ui_text_node_base *node,
                                      const char *text) {
  if (!node)
    return UI_ERROR_INVALID_ARGUMENT;
  if (node->text) {
    C_MULTIPLATFORM_FREE(node->text);
    node->text = NULL;
  }
  if (!text)
    return UI_ERROR_NONE;
  node->text = C_MULTIPLATFORM_STRDUP(text);
  return (node->text ? UI_ERROR_NONE : UI_ERROR_OUT_OF_MEMORY);
}

ui_error_t ui_text_node_base_get_text(const struct ui_text_node_base *node,
                                      const char **out_text) {
  if (!node || !out_text)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_text = node->text;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_text_node_base_set_font_manager(struct ui_text_node_base *node,
                                   struct ui_font_manager *font_manager) {
  if (!node)
    return UI_ERROR_INVALID_ARGUMENT;
  node->font_manager = font_manager;
  return UI_ERROR_NONE;
}

ui_error_t ui_text_node_base_set_font_family(struct ui_text_node_base *node,
                                             const char *family) {
  if (!node)
    return UI_ERROR_INVALID_ARGUMENT;
  if (node->font_family) {
    C_MULTIPLATFORM_FREE(node->font_family);
    node->font_family = NULL;
  }
  if (!family)
    return UI_ERROR_NONE;
  node->font_family = C_MULTIPLATFORM_STRDUP(family);
  return (node->font_family ? UI_ERROR_NONE : UI_ERROR_OUT_OF_MEMORY);
}

ui_error_t ui_text_node_base_set_font_size(struct ui_text_node_base *node,
                                           float size) {
  if (!node)
    return UI_ERROR_INVALID_ARGUMENT;
  node->font_size = size;
  return UI_ERROR_NONE;
}

ui_error_t ui_text_node_base_set_max_width(struct ui_text_node_base *node,
                                           float max_width) {
  if (!node)
    return UI_ERROR_INVALID_ARGUMENT;
  node->max_width = max_width;
  return UI_ERROR_NONE;
}

ui_error_t ui_text_node_base_set_max_lines(struct ui_text_node_base *node,
                                           int max_lines) {
  if (!node)
    return UI_ERROR_INVALID_ARGUMENT;
  node->max_lines = max_lines;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_text_node_base_set_overflow(struct ui_text_node_base *node,
                                          enum ui_text_node_overflow overflow) {
  if (!node)
    return UI_ERROR_INVALID_ARGUMENT;
  node->overflow = overflow;
  return UI_ERROR_NONE;
}

ui_error_t ui_text_node_base_update_layout(struct ui_text_node_base *node) {
  struct ui_font *font = NULL;
  ui_error_t rc;

  if (!node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Structural font fallback */
  if (node->font_manager) {
    const char *primary_font =
        node->font_family ? node->font_family : "sans-serif";
#define UI_FONT_FIND_IGNORE_PRIM(mgr, fam, w, it, outf)                        \
  ui_font_manager_find_font((mgr), (fam), (w), (it), (outf))
    rc = UI_FONT_FIND_IGNORE_PRIM(node->font_manager, primary_font, 400, 0,
                                  &font);
    if (rc != UI_ERROR_NONE) {
      /* Fallback to system-ui */
#define UI_FONT_FIND_IGNORE(mgr, fam, w, it, outf)                             \
  ui_font_manager_find_font((mgr), (fam), (w), (it), (outf))
      (void)UI_FONT_FIND_IGNORE(node->font_manager, "system-ui", 400, 0, &font);
      /* If no font can be found, we just proceed with NULL and
       * ui_text_layout_shape will fail or mock it */
    }
  }

  if (!node->text || !font) {
    node->computed_width = 0.0f;
    node->computed_height = 0.0f;
    if (node->component && node->component->shadow_root) {
      rc = ui_dom_node_set_text_content(node->component->shadow_root, "");
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    return UI_ERROR_NONE; /* Not an error to be empty or unfonted */
  }

  rc = ui_text_layout_shape(node->layout, font, node->font_size, node->text,
                            node->max_width, UI_TEXT_DIRECTION_LTR);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_text_layout_get_bounds(node->layout, &node->computed_width,
                                 &node->computed_height);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Truncation / Line Clamp Logic */
  if (node->max_lines > 0) {
    float ascent = 0.0f, descent = 0.0f, line_gap = 0.0f;
    float line_height;
    float max_allowed_height;
    rc = ui_font_get_vmetrics(font, node->font_size, &ascent, &descent,
                              &line_gap);
    if (rc != UI_ERROR_NONE)
      return rc;
    line_height = ascent - descent + line_gap;
    max_allowed_height = line_height * (float)node->max_lines;

    /* Small epsilon for floating point inaccuracies */
    if (node->computed_height > max_allowed_height + 0.1f) {
      if (node->overflow == UI_TEXT_NODE_OVERFLOW_ELLIPSIS) {
        size_t len = strlen(node->text);
        size_t target_len =
            (size_t)((float)len * (max_allowed_height / node->computed_height));
        if (target_len > 3 && target_len < len) {
          char *trunc_str = (char *)C_MULTIPLATFORM_MALLOC(target_len + 4);
          if (trunc_str) {
            /* use strncpy_s or strncpy safely */
#if defined(_MSC_VER)
            strncpy_s(trunc_str, target_len + 4, node->text, target_len);
            strcpy_s(trunc_str + target_len, 4, "...");
#else
            strncpy(trunc_str, node->text, target_len);
            strcpy(trunc_str + target_len, "...");
#endif
            rc = ui_text_layout_shape(node->layout, font, node->font_size,
                                      trunc_str, node->max_width,
                                      UI_TEXT_DIRECTION_LTR);
            if (rc != UI_ERROR_NONE) {
              C_MULTIPLATFORM_FREE(trunc_str);
              return rc;
            }
            rc = ui_text_layout_get_bounds(node->layout, &node->computed_width,
                                           &node->computed_height);
            if (rc != UI_ERROR_NONE) {
              C_MULTIPLATFORM_FREE(trunc_str);
              return rc;
            }
            C_MULTIPLATFORM_FREE(trunc_str);
          }
        }
      }
      /* Hard clip */
      if (node->computed_height > max_allowed_height) {
        node->computed_height = max_allowed_height;
      }
    }
  }

  if (node->component && node->component->shadow_root) {
    rc = ui_dom_node_set_text_content(node->component->shadow_root, node->text);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_text_node_base_get_layout(struct ui_text_node_base *node,
                                        struct ui_text_layout **out_layout) {
  if (!node || !out_layout)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_layout = node->layout;
  return UI_ERROR_NONE;
}
/** \brief ui_error */
ui_error_t
ui_text_node_base_get_component(struct ui_text_node_base *node,
                                struct ui_component **out_component) {
  if (!node || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = node->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_text_node_base_bind_text(struct ui_text_node_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = signal;
  return UI_ERROR_NONE;
}
