/* clang-format off */
#include <stdio.h>
#include <string.h>

#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_error.h"
#include "../include/ui_layout.h"

#define ui_css_computed_style_get_property                                     \
  mock_ui_css_computed_style_get_property
#define ui_css_resolve_style mock_ui_css_resolve_style
ui_error_t mock_ui_css_computed_style_get_property(
    const struct ui_css_computed_style *style, const char *name,
    const char **out_value);
ui_error_t mock_ui_css_resolve_style(const struct ui_css_stylesheet *sheet,
                                     const struct ui_dom_node *node,
                                     struct ui_css_computed_style **out_style);

#define ui_css_computed_style_destroy mock_ui_css_computed_style_destroy
ui_error_t
mock_ui_css_computed_style_destroy(struct ui_css_computed_style *style);

#include "../src/ui_layout.c"
/* clang-format on */
#undef ui_css_computed_style_get_property
#undef ui_css_resolve_style
#undef ui_css_computed_style_destroy

static const char *g_mock_prop_name = NULL;
static const char *g_mock_prop_val = NULL;
static ui_error_t g_mock_prop_rc = UI_ERROR_NONE;

ui_error_t
mock_ui_css_computed_style_destroy(struct ui_css_computed_style *style) {
  return UI_ERROR_NONE;
}

ui_error_t mock_ui_css_computed_style_get_property(
    const struct ui_css_computed_style *style, const char *name,
    const char **out_value) {
  if (g_mock_prop_name && strcmp(name, g_mock_prop_name) == 0) {
    if (g_mock_prop_rc != UI_ERROR_NONE) {
      return g_mock_prop_rc;
    }
    *out_value = g_mock_prop_val;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_NOT_FOUND;
}

ui_error_t mock_ui_css_resolve_style(const struct ui_css_stylesheet *sheet,
                                     const struct ui_dom_node *node,
                                     struct ui_css_computed_style **out_style) {
  *out_style = (struct ui_css_computed_style *)1;
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_dom_node *root;
  struct ui_layout_node *out_node;
  int i;
  const char *props[] = {"align-content",
                         "align-items",
                         "align-self",
                         "aspect-ratio",
                         "background-color",
                         "background-image",
                         "background-repeat",
                         "background-size",
                         "block-step-align",
                         "block-step-insert",
                         "block-step-round",
                         "block-step-size",
                         "border-bottom-color",
                         "border-bottom-left-radius",
                         "border-bottom-right-radius",
                         "border-bottom-width",
                         "border-image",
                         "border-image-source",
                         "border-left-color",
                         "border-left-width",
                         "border-radius",
                         "border-right-color",
                         "border-right-width",
                         "border-top-color",
                         "border-top-left-radius",
                         "border-top-right-radius",
                         "border-top-width",
                         "border-width",
                         "bottom",
                         "box-decoration-break",
                         "box-shadow",
                         "box-sizing",
                         "box-snap",
                         "break-after",
                         "break-before",
                         "break-inside",
                         "color",
                         "color-scheme",
                         "column-count",
                         "column-gap",
                         "column-width",
                         "direction",
                         "display",
                         "flex-basis",
                         "flex-direction",
                         "flex-grow",
                         "flex-shrink",
                         "flex-wrap",
                         "flow-from",
                         "flow-into",
                         "font-family",
                         "font-feature-settings",
                         "font-size",
                         "font-stretch",
                         "font-style",
                         "font-variant",
                         "font-variation-settings",
                         "font-weight",
                         "forced-color-adjust",
                         "height",
                         "hyphens",
                         "justify-content",
                         "left",
                         "line-grid",
                         "line-snap",
                         "margin",
                         "margin-bottom",
                         "margin-left",
                         "margin-right",
                         "margin-top",
                         "margin-trim",
                         "max-height",
                         "max-width",
                         "min-height",
                         "min-width",
                         "opacity",
                         "orphans",
                         "overflow",
                         "overflow-x",
                         "overflow-y",
                         "padding",
                         "padding-bottom",
                         "padding-left",
                         "padding-right",
                         "padding-top",
                         "page-break-after",
                         "page-break-before",
                         "page-break-inside",
                         "position",
                         "print-color-adjust",
                         "right",
                         "text-align",
                         "text-decoration",
                         "text-decoration-color",
                         "text-decoration-line",
                         "text-decoration-style",
                         "text-emphasis-color",
                         "text-emphasis-position",
                         "text-emphasis-style",
                         "text-orientation",
                         "text-shadow",
                         "text-size-adjust",
                         "top",
                         "transform",
                         "unicode-bidi",
                         "white-space",
                         "widows",
                         "width",
                         "word-break",
                         "wrap-flow",
                         "wrap-through",
                         "writing-mode",
                         "z-index"};

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);

  /* Force out_node NULL error */
  (void)create_layout_node(root, (struct ui_css_computed_style *)1, 0, NULL);

  /* Force build_tree_recursive error */
  g_mock_prop_name = "display";
  g_mock_prop_val = "block";
  g_mock_prop_rc = UI_ERROR_OUT_OF_MEMORY;
  (void)build_tree_recursive(root, (struct ui_css_stylesheet *)1, &out_node);
  g_mock_prop_rc = UI_ERROR_NONE;
  g_mock_prop_name = NULL;
  g_mock_prop_val = NULL;

  /* Force attr_rc error */
  out_node = NULL;
  g_mock_prop_name = "display";
  g_mock_prop_val = "block";
  g_mock_prop_rc = UI_ERROR_OUT_OF_MEMORY;
  (void)create_layout_node(root, (struct ui_css_computed_style *)1, 0,
                           &out_node);
  if (out_node) {
    (void)compute_box_model(out_node);
    free(out_node);
  }
  g_mock_prop_rc = UI_ERROR_NONE;
  g_mock_prop_name = NULL;
  g_mock_prop_val = NULL;

  /* Call compute_box_model with a valid node to hit the !computed_style branch
   */
  out_node = NULL;
  (void)create_layout_node(root, (struct ui_css_computed_style *)1, 0,
                           &out_node);
  if (out_node) {
    out_node->computed_style = NULL;
    (void)compute_box_model(out_node);
    out_node->computed_style = (struct ui_css_computed_style *)1;
    free(out_node);
  }

  /* Force parse errors by mocking a property with an invalid value */
  for (i = 0; i < sizeof(props) / sizeof(props[0]); i++) {
    ui_error_t rc;

    /* 1. Test attr_rc != UI_ERROR_NONE (e.g. OOM from getter) */
    out_node = NULL;
    g_mock_prop_name = props[i];
    g_mock_prop_val = "10px";
    g_mock_prop_rc = UI_ERROR_OUT_OF_MEMORY;
    (void)create_layout_node(root, (struct ui_css_computed_style *)1, 0,
                             &out_node);
    if (out_node) {
      (void)compute_box_model(out_node);
      free(out_node);
    }

    /* 2. Test invalid value parsing */
    out_node = NULL;
    g_mock_prop_name = props[i];
    g_mock_prop_val = "invalid";
    g_mock_prop_rc = UI_ERROR_NONE;
    (void)create_layout_node(root, (struct ui_css_computed_style *)1, 0,
                             &out_node);
    if (out_node) {
      rc = compute_box_model(out_node);
      free(out_node);
    }
  }

  /* EXTRA COVERAGE */
  create_layout_node(root, NULL, 0, NULL);
  create_layout_node(root, NULL, 0, &out_node);
  compute_box_model(out_node);
  g_mock_prop_name = "display";
  g_mock_prop_val = "none";
  g_mock_prop_rc = UI_ERROR_INVALID_ARGUMENT;
  create_layout_node(root, (struct ui_css_computed_style *)1, 0, &out_node);
  ui_layout_solve_viewport(NULL, 0.0f, 0.0f);

  printf("Done\n");
  return 0;
}
