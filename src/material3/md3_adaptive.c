/**
 * @file md3_adaptive.c
 * @brief Implementation of Material 3 Adaptive Scaffolds.
 */

/* clang-format off */
#include "material3/md3_adaptive.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct md3_list_detail_pane_scaffold {
  struct ui_adaptive_pane_scaffold_base *base;
};

struct md3_supporting_pane_scaffold {
  struct ui_adaptive_pane_scaffold_base *base;
};

struct md3_navigation_suite_scaffold {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  enum ui_adaptive_window_width_class width_class;
};

/* ========================================================================= */
/* List-Detail Pane Scaffold Implementation                                  */
/* ========================================================================= */

ui_error_t md3_list_detail_pane_scaffold_create(
    struct ui_engine *engine,
    struct md3_list_detail_pane_scaffold **out_scaffold) {
  struct md3_list_detail_pane_scaffold *s = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  if (engine == NULL || out_scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  s = (struct md3_list_detail_pane_scaffold *)C_MULTIPLATFORM_MALLOC(
      sizeof(*s));
  if (s == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(s, 0, sizeof(*s));

  rc = ui_adaptive_pane_scaffold_base_create(UI_ADAPTIVE_SCAFFOLD_LIST_DETAIL,
                                             &s->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(s);
    return rc;
  }

  rc = ui_adaptive_pane_scaffold_base_get_component(s->base, &comp);
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_md3_adaptive_mock_comp_null;
  if (g_md3_adaptive_mock_comp_null == 1)
    rc = UI_ERROR_UNKNOWN;
  if (g_md3_adaptive_mock_comp_null == 2)
    comp = NULL;
  if (g_md3_adaptive_mock_comp_null == 3) {
    comp->shadow_root = NULL;
  }
#endif
  if (rc == UI_ERROR_NONE) {
    if (comp != NULL) {
      if (comp->shadow_root != NULL) {
        rc = ui_dom_node_set_attribute(comp->shadow_root, "class",
                                       "md3-list-detail-pane-scaffold");
        if (rc != UI_ERROR_NONE) {
          ui_adaptive_pane_scaffold_base_destroy(s->base);
          C_MULTIPLATFORM_FREE(s);
          return rc;
        }
      }
    }
  }

  *out_scaffold = s;
  return UI_ERROR_NONE;
}

ui_error_t md3_list_detail_pane_scaffold_destroy(
    struct md3_list_detail_pane_scaffold *scaffold) {
  ui_error_t rc;

  if (scaffold == NULL) {
    return UI_ERROR_NONE;
  }

  rc = ui_adaptive_pane_scaffold_base_destroy(scaffold->base);
  scaffold->base = NULL;

  C_MULTIPLATFORM_FREE(scaffold);
  return rc;
}

ui_error_t md3_list_detail_pane_scaffold_get_base(
    struct md3_list_detail_pane_scaffold *scaffold,
    struct ui_adaptive_pane_scaffold_base **out_base) {
  if (scaffold == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = scaffold->base;
  return UI_ERROR_NONE;
}

ui_error_t md3_list_detail_pane_scaffold_set_list_pane(
    struct md3_list_detail_pane_scaffold *scaffold,
    struct ui_component *list_component) {
  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_adaptive_pane_scaffold_base_set_pane(
      scaffold->base, UI_PANE_ROLE_PRIMARY, list_component);
}

ui_error_t md3_list_detail_pane_scaffold_set_detail_pane(
    struct md3_list_detail_pane_scaffold *scaffold,
    struct ui_component *detail_component) {
  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_adaptive_pane_scaffold_base_set_pane(
      scaffold->base, UI_PANE_ROLE_SECONDARY, detail_component);
}

ui_error_t md3_list_detail_pane_scaffold_set_extra_pane(
    struct md3_list_detail_pane_scaffold *scaffold,
    struct ui_component *extra_component) {
  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_adaptive_pane_scaffold_base_set_pane(
      scaffold->base, UI_PANE_ROLE_SUPPORTING, extra_component);
}

/* ========================================================================= */
/* Supporting Pane Scaffold Implementation                                   */
/* ========================================================================= */

ui_error_t md3_supporting_pane_scaffold_create(
    struct ui_engine *engine,
    struct md3_supporting_pane_scaffold **out_scaffold) {
  struct md3_supporting_pane_scaffold *s = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  if (engine == NULL || out_scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  s = (struct md3_supporting_pane_scaffold *)C_MULTIPLATFORM_MALLOC(sizeof(*s));
  if (s == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(s, 0, sizeof(*s));

  rc = ui_adaptive_pane_scaffold_base_create(
      UI_ADAPTIVE_SCAFFOLD_SUPPORTING_PANE, &s->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(s);
    return rc;
  }

  rc = ui_adaptive_pane_scaffold_base_get_component(s->base, &comp);
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_md3_adaptive_mock_comp_null;
  if (g_md3_adaptive_mock_comp_null == 1)
    rc = UI_ERROR_UNKNOWN;
  if (g_md3_adaptive_mock_comp_null == 2)
    comp = NULL;
  if (g_md3_adaptive_mock_comp_null == 3) {
    comp->shadow_root = NULL;
  }
#endif
  if (rc == UI_ERROR_NONE) {
    if (comp != NULL) {
      if (comp->shadow_root != NULL) {
        rc = ui_dom_node_set_attribute(comp->shadow_root, "class",
                                       "md3-supporting-pane-scaffold");
        if (rc != UI_ERROR_NONE) {
          ui_adaptive_pane_scaffold_base_destroy(s->base);
          C_MULTIPLATFORM_FREE(s);
          return rc;
        }
      }
    }
  }

  *out_scaffold = s;
  return UI_ERROR_NONE;
}

ui_error_t md3_supporting_pane_scaffold_destroy(
    struct md3_supporting_pane_scaffold *scaffold) {
  ui_error_t rc;

  if (scaffold == NULL) {
    return UI_ERROR_NONE;
  }

  rc = ui_adaptive_pane_scaffold_base_destroy(scaffold->base);
  scaffold->base = NULL;

  C_MULTIPLATFORM_FREE(scaffold);
  return rc;
}

ui_error_t md3_supporting_pane_scaffold_get_base(
    struct md3_supporting_pane_scaffold *scaffold,
    struct ui_adaptive_pane_scaffold_base **out_base) {
  if (scaffold == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = scaffold->base;
  return UI_ERROR_NONE;
}

ui_error_t md3_supporting_pane_scaffold_set_main_pane(
    struct md3_supporting_pane_scaffold *scaffold,
    struct ui_component *main_component) {
  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_adaptive_pane_scaffold_base_set_pane(
      scaffold->base, UI_PANE_ROLE_PRIMARY, main_component);
}

ui_error_t md3_supporting_pane_scaffold_set_supporting_pane(
    struct md3_supporting_pane_scaffold *scaffold,
    struct ui_component *supporting_component) {
  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_adaptive_pane_scaffold_base_set_pane(
      scaffold->base, UI_PANE_ROLE_SUPPORTING, supporting_component);
}

/* ========================================================================= */
/* Navigation Suite Scaffold Implementation                                  */
/* ========================================================================= */

ui_error_t md3_navigation_suite_scaffold_create(
    struct ui_engine *engine,
    struct md3_navigation_suite_scaffold **out_scaffold) {
  struct md3_navigation_suite_scaffold *s = NULL;
  ui_error_t rc;

  if (engine == NULL || out_scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  s = (struct md3_navigation_suite_scaffold *)C_MULTIPLATFORM_MALLOC(
      sizeof(*s));
  if (s == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(s, 0, sizeof(*s));

  s->width_class = UI_WINDOW_WIDTH_COMPACT;

  rc = ui_component_create(&s->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(s);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &s->root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(s->component);
    C_MULTIPLATFORM_FREE(s);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(s->root_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(s->root_node);
    ui_component_destroy(s->component);
    C_MULTIPLATFORM_FREE(s);
    return rc;
  }

  rc = ui_dom_node_set_attribute(s->root_node, "class",
                                 "md3-navigation-suite-scaffold");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(s->root_node);
    ui_component_destroy(s->component);
    C_MULTIPLATFORM_FREE(s);
    return rc;
  }

  rc = ui_dom_node_set_attribute(s->root_node, "data-nav-layout",
                                 "navigation-bar");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(s->root_node);
    ui_component_destroy(s->component);
    C_MULTIPLATFORM_FREE(s);
    return rc;
  }

  s->component->shadow_root = s->root_node;
  *out_scaffold = s;
  return UI_ERROR_NONE;
}

ui_error_t md3_navigation_suite_scaffold_destroy(
    struct md3_navigation_suite_scaffold *scaffold) {
  if (scaffold == NULL) {
    return UI_ERROR_NONE;
  }

  if (scaffold->component != NULL) {
    ui_component_destroy(scaffold->component);
    scaffold->component = NULL;
  }

  C_MULTIPLATFORM_FREE(scaffold);
  return UI_ERROR_NONE;
}

ui_error_t md3_navigation_suite_scaffold_set_window_width_class(
    struct md3_navigation_suite_scaffold *scaffold,
    enum ui_adaptive_window_width_class width_class) {
  const char *layout_str;
  ui_error_t rc;

  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->width_class = width_class;
  switch (width_class) {
  case UI_WINDOW_WIDTH_COMPACT:
    layout_str = "navigation-bar";
    break;
  case UI_WINDOW_WIDTH_MEDIUM:
    layout_str = "navigation-rail";
    break;
  case UI_WINDOW_WIDTH_EXPANDED:
  default:
    layout_str = "navigation-drawer";
    break;
  }

  rc = ui_dom_node_set_attribute(scaffold->root_node, "data-nav-layout",
                                 layout_str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_navigation_suite_scaffold_get_component(
    struct md3_navigation_suite_scaffold *scaffold,
    struct ui_component **out_component) {
  if (scaffold == NULL || out_component == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_component = scaffold->component;
  return UI_ERROR_NONE;
}
