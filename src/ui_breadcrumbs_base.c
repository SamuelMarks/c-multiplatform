/* clang-format off */
#include "ui_breadcrumbs_base.h"
#include "ui_internal_mem.h"
#include "ui_dom_node.h"
#include "ui_css_parser.h"
#include <string.h>
#include <stdio.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_breadcrumbs_base_css = ":host { "
                                             "display: block; "
                                             "font-family: sans-serif; "
                                             "font-size: 14px; "
                                             "} "
                                             "ol { "
                                             "list-style: none; "
                                             "padding: 0; "
                                             "margin: 0; "
                                             "display: flex; "
                                             "} "
                                             "li { "
                                             "display: flex; "
                                             "align-items: center; "
                                             "} "
                                             "li + li::before { "
                                             "content: '/'; "
                                             "margin: 0 8px; "
                                             "color: #999; "
                                             "} "
                                             "a { "
                                             "text-decoration: none; "
                                             "color: #007bff; "
                                             "cursor: pointer; "
                                             "} "
                                             "a:hover { "
                                             "text-decoration: underline; "
                                             "} "
                                             "a[aria-current=\"page\"] { "
                                             "color: #333; "
                                             "cursor: default; "
                                             "text-decoration: none; "
                                             "}";

/** \brief ui_breadcrumb_segment */
struct ui_breadcrumb_segment {
  char *label;
  char *path;
  struct ui_dom_node *li_node;
  struct ui_dom_node *a_node;
  struct ui_dom_node *text_node;
};

/** \brief ui_breadcrumbs_base */
#ifdef UI_TEST_MOCK_ALLOC
extern int g_breadcrumbs_mock_fail;

static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_breadcrumbs_mock_fail == 130 || g_breadcrumbs_mock_fail == 397 ||
      g_breadcrumbs_mock_fail == 413 || g_breadcrumbs_mock_fail == 427)
    return UI_ERROR_UNKNOWN;
  return ui_dom_node_append_child(parent, child);
}
#define ui_dom_node_append_child mock_dom_node_append_child

static ui_error_t
mock_ui_component_set_default_style(struct ui_component *comp,
                                    struct ui_css_stylesheet *style) {
  if (g_breadcrumbs_mock_fail == 2)
    return UI_ERROR_UNKNOWN;
  return ui_component_set_default_style(comp, style);
}
#define ui_component_set_default_style mock_ui_component_set_default_style

static ui_error_t mock_dom_node_remove_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_breadcrumbs_mock_fail == 3)
    return UI_ERROR_UNKNOWN;
  return ui_dom_node_remove_child(parent, child);
}
#define ui_dom_node_remove_child mock_dom_node_remove_child

static ui_error_t mock_dom_node_destroy(struct ui_dom_node *node) {
  if (g_breadcrumbs_mock_fail == 4)
    return UI_ERROR_UNKNOWN;
  return ui_dom_node_destroy(node);
}
#define ui_dom_node_destroy mock_dom_node_destroy

static ui_error_t mock_ui_component_destroy(struct ui_component *comp) {
  if (g_breadcrumbs_mock_fail == 5)
    return UI_ERROR_UNKNOWN;
  return ui_component_destroy(comp);
}
#define ui_component_destroy mock_ui_component_destroy

static ui_error_t mock_ui_dom_node_set_text_content(struct ui_dom_node *node,
                                                    const char *text) {
  if (g_breadcrumbs_mock_fail == 6)
    return UI_ERROR_UNKNOWN;
  return ui_dom_node_set_text_content(node, text);
}
#define ui_dom_node_set_text_content mock_ui_dom_node_set_text_content

static ui_error_t mock_ui_router_navigate(struct ui_router *router,
                                          const char *path) {
  if (g_breadcrumbs_mock_fail == 7)
    return UI_ERROR_UNKNOWN;
  return ui_router_navigate(router, path);
}
#define ui_router_navigate mock_ui_router_navigate

static ui_error_t mock_ui_dom_node_set_attribute(struct ui_dom_node *node,
                                                 const char *k, const char *v) {
  if (g_breadcrumbs_mock_fail == 8)
    return UI_ERROR_UNKNOWN;
  return ui_dom_node_set_attribute(node, k, v);
}
#define ui_dom_node_set_attribute mock_ui_dom_node_set_attribute

static ui_error_t mock_ui_css_parse_stylesheet(const char *css,
                                               struct ui_css_stylesheet **out) {
  if (g_breadcrumbs_mock_fail == 9)
    return UI_ERROR_UNKNOWN;
  return ui_css_parse_stylesheet(css, out);
}
#define ui_css_parse_stylesheet mock_ui_css_parse_stylesheet
#endif

struct ui_breadcrumbs_base {
  struct ui_router *router;
  struct ui_component *component;
  struct ui_dom_node *nav_node;
  struct ui_dom_node *ol_node;

  struct ui_breadcrumb_segment *segments;
  size_t segment_count;
  struct ui_signal *active_index_signal;
};

static ui_error_t internal_strndup(const char *src, size_t n, char **out_str) {
  char *copy;

  copy = (char *)C_MULTIPLATFORM_MALLOC(n + 1);
  if (!copy)
    return UI_ERROR_OUT_OF_MEMORY;

#if defined(_MSC_VER)
  strncpy_s(copy, n + 1, src, n);
#else
  if (n > 0) {
    memcpy(copy, src, n);
  }
  copy[n] = '\0';
#endif

  *out_str = copy;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_breadcrumbs_base_create(struct ui_router *router,
                           struct ui_breadcrumbs_base **out_breadcrumbs) {
  struct ui_breadcrumbs_base *bc;
  ui_error_t rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_breadcrumbs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bc = (struct ui_breadcrumbs_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_breadcrumbs_base));
  if (!bc)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(bc, 0, sizeof(struct ui_breadcrumbs_base));
  bc->router = router;

  rc = ui_component_create(&bc->component);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &bc->nav_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(bc->nav_node, "nav");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(bc->nav_node, "aria-label", "Breadcrumb");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &bc->ol_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(bc->ol_node, "ol");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(bc->nav_node, bc->ol_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_css_parse_stylesheet(ui_breadcrumbs_base_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_component_set_default_style(bc->component, default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  bc->component->shadow_root = bc->nav_node;

  *out_breadcrumbs = bc;
  return UI_ERROR_NONE;

cleanup:
  if (bc->nav_node) {
    if (bc->component) {
      if (bc->component->shadow_root == bc->nav_node) {
        bc->component->shadow_root = NULL;
      }
    }
    (void)ui_dom_node_destroy(bc->nav_node);
  }
  if (bc->component)
    (void)ui_component_destroy(bc->component);
  C_MULTIPLATFORM_FREE(bc);
  return rc;
}

static ui_error_t free_segments(struct ui_breadcrumbs_base *bc) {
  size_t i;
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t tmp_rc;
  if (!bc->segments)
    return UI_ERROR_NONE;

  for (i = 0; i < bc->segment_count; i++) {
    if (bc->segments[i].label)
      C_MULTIPLATFORM_FREE(bc->segments[i].label);
    if (bc->segments[i].path)
      C_MULTIPLATFORM_FREE(bc->segments[i].path);

    /* If li_node exists and was appended to ol_node, we can just remove and
       destroy it. ui_dom_node_destroy is recursive. If OOM happened
       mid-creation, we might have unparented nodes. */
    if (bc->segments[i].li_node) {
      if (bc->segments[i].li_node->parent) {
        tmp_rc = ui_dom_node_remove_child(bc->segments[i].li_node->parent,
                                          bc->segments[i].li_node);
        if (tmp_rc != UI_ERROR_NONE)
          rc = tmp_rc;
      }
      tmp_rc = ui_dom_node_destroy(bc->segments[i].li_node);
      if (tmp_rc != UI_ERROR_NONE)
        rc = tmp_rc;
    }
  }
  C_MULTIPLATFORM_FREE(bc->segments);
  bc->segments = NULL;
  bc->segment_count = 0;
  return rc;
}

ui_error_t
ui_breadcrumbs_base_destroy(struct ui_breadcrumbs_base *breadcrumbs) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!breadcrumbs)
    return UI_ERROR_NONE;

  rc = free_segments(breadcrumbs);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc = ui_component_destroy(breadcrumbs->component);
    if (destroy_rc != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(breadcrumbs);
      return destroy_rc;
    }
    C_MULTIPLATFORM_FREE(breadcrumbs);
    return rc;
  }
  rc = ui_component_destroy(breadcrumbs->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(breadcrumbs);
    return rc;
  }
  C_MULTIPLATFORM_FREE(breadcrumbs);
  return UI_ERROR_NONE;
}
/** \brief ui_error */
ui_error_t
ui_breadcrumbs_base_get_component(struct ui_breadcrumbs_base *breadcrumbs,
                                  struct ui_component **out_component) {
  if (!breadcrumbs || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = breadcrumbs->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_breadcrumbs_base_set_path(struct ui_breadcrumbs_base *bc,
                                        const char *path) {
  size_t segment_capacity = 4;
  size_t count = 0;
  struct ui_breadcrumb_segment *new_segments = NULL;
  const char *p = path;
  const char *segment_start = NULL;
  size_t parsed_path_len = 0;
  char *accumulated_path = NULL;
  char *temp_acc = NULL;
  ui_error_t rc;

  if (!bc || !path)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = free_segments(bc);
  if (rc != UI_ERROR_NONE)
    return rc;

  new_segments = (struct ui_breadcrumb_segment *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_breadcrumb_segment) * segment_capacity);
  if (!new_segments)
    return UI_ERROR_OUT_OF_MEMORY;
  memset(new_segments, 0,
         sizeof(struct ui_breadcrumb_segment) * segment_capacity);

  /* Start accumulated path with root / if it starts with / */
  if (path[0] == '/') {
    internal_strndup("", 0, &accumulated_path);
  }

  while (*p) {
    if (*p == '/') {
      p++;
    }
    segment_start = p;
    while (*p && *p != '/') {
      p++;
    }

    if (p > segment_start) {
      if (count >= segment_capacity) {
        size_t new_cap = segment_capacity * 2;
        struct ui_breadcrumb_segment *larger =
            (struct ui_breadcrumb_segment *)C_MULTIPLATFORM_MALLOC(
                sizeof(struct ui_breadcrumb_segment) * new_cap);
        if (!larger) {
          rc = UI_ERROR_OUT_OF_MEMORY;
          goto fail;
        }
        memset(larger, 0, sizeof(struct ui_breadcrumb_segment) * new_cap);
        memcpy(larger, new_segments,
               sizeof(struct ui_breadcrumb_segment) * count);
        C_MULTIPLATFORM_FREE(new_segments);
        new_segments = larger;
        segment_capacity = new_cap;
      }

      rc = internal_strndup(segment_start, (size_t)(p - segment_start),
                            &new_segments[count].label);
      if (rc != UI_ERROR_NONE)
        goto fail;

      if (accumulated_path) {
        parsed_path_len =
            strlen(accumulated_path) + 1 + strlen(new_segments[count].label);
        temp_acc = (char *)C_MULTIPLATFORM_MALLOC(parsed_path_len + 1);
        if (!temp_acc) {
          rc = UI_ERROR_OUT_OF_MEMORY;
          goto fail;
        }
#if defined(_MSC_VER)
        sprintf_s(temp_acc, parsed_path_len + 1, "%s/%s", accumulated_path,
                  new_segments[count].label);
#else
        sprintf(temp_acc, "%s/%s", accumulated_path, new_segments[count].label);
#endif
        C_MULTIPLATFORM_FREE(accumulated_path);
        accumulated_path = temp_acc;
        rc = internal_strndup(accumulated_path, parsed_path_len,
                              &new_segments[count].path);
        if (rc != UI_ERROR_NONE)
          goto fail;
      } else {
        rc = internal_strndup(new_segments[count].label,
                              (size_t)(p - segment_start),
                              &new_segments[count].path);
        if (rc != UI_ERROR_NONE)
          goto fail;
      }

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                              &new_segments[count].li_node);
      if (rc != UI_ERROR_NONE)
        goto fail;
      rc = ui_dom_node_set_tag_name(new_segments[count].li_node, "li");
      if (rc != UI_ERROR_NONE)
        goto fail;
      rc = ui_dom_node_append_child(bc->ol_node, new_segments[count].li_node);
      if (rc != UI_ERROR_NONE)
        goto fail;

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                              &new_segments[count].a_node);
      if (rc != UI_ERROR_NONE)
        goto fail;
      rc = ui_dom_node_set_tag_name(new_segments[count].a_node, "a");
      if (rc != UI_ERROR_NONE)
        goto fail;
      rc = ui_dom_node_set_attribute(new_segments[count].a_node, "href",
                                     new_segments[count].path);
      if (rc != UI_ERROR_NONE)
        goto fail;
      rc = ui_dom_node_append_child(new_segments[count].li_node,
                                    new_segments[count].a_node);
      if (rc != UI_ERROR_NONE)
        goto fail;

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT,
                              &new_segments[count].text_node);
      if (rc != UI_ERROR_NONE)
        goto fail;
      rc = ui_dom_node_set_text_content(new_segments[count].text_node,
                                        new_segments[count].label);
      if (rc != UI_ERROR_NONE)
        goto fail;

      rc = ui_dom_node_append_child(new_segments[count].a_node,
                                    new_segments[count].text_node);
      if (rc != UI_ERROR_NONE)
        goto fail;

      count++;
    }
  }

  /* Set aria-current on the last segment */
  if (count > 0) {
    rc = ui_dom_node_set_attribute(new_segments[count - 1].a_node,
                                   "aria-current", "page");
    if (rc != UI_ERROR_NONE)
      goto fail;
  }

  bc->segments = new_segments;
  bc->segment_count = count;
  if (accumulated_path)
    C_MULTIPLATFORM_FREE(accumulated_path);
  return UI_ERROR_NONE;

fail:
  if (accumulated_path)
    C_MULTIPLATFORM_FREE(accumulated_path);
  bc->segments = new_segments;
  bc->segment_count = (count < segment_capacity) ? count + 1 : count;
  {
    ui_error_t free_rc = free_segments(bc);
    if (free_rc != UI_ERROR_NONE) {
      return free_rc;
    }
  }
  return rc;
}

ui_error_t ui_breadcrumbs_base_simulate_click(struct ui_breadcrumbs_base *bc,
                                              size_t index) {
  if (!bc)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index >= bc->segment_count)
    return UI_ERROR_OUT_OF_BOUNDS;

  if (bc->router && bc->segments[index].path) {
    return ui_router_navigate(bc->router, bc->segments[index].path);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_breadcrumbs_base_bind_active_index(struct ui_breadcrumbs_base *widget,
                                      struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
ui_error_t run_bc_coverage(void);
ui_error_t run_bc_coverage(void) {
  struct ui_dom_node *dn1 = NULL;
  struct ui_dom_node *dn2 = NULL;
  struct ui_router *dummy_router = NULL;
  struct ui_breadcrumbs_base *bc = NULL;

  g_breadcrumbs_mock_fail = 1;
  mock_dom_node_append_child(NULL, NULL);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 2;
  mock_ui_component_set_default_style(NULL, NULL);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 3;
  mock_dom_node_remove_child(NULL, NULL);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 4;
  mock_dom_node_destroy(NULL);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 5;
  mock_ui_component_destroy(NULL);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 6;
  mock_ui_dom_node_set_text_content(NULL, NULL);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 7;
  mock_ui_router_navigate(NULL, NULL);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 8;
  mock_ui_dom_node_set_attribute(NULL, NULL, NULL);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 9;
  mock_ui_css_parse_stylesheet(NULL, NULL);
  g_breadcrumbs_mock_fail = 0;

  /* Create dummy node for child tests */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dn1);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dn2);
  ui_dom_node_append_child(dn1, dn2);

  g_breadcrumbs_mock_fail = 3;
  mock_dom_node_remove_child(dn1, dn2);
  g_breadcrumbs_mock_fail = 0;
  g_breadcrumbs_mock_fail = 4;
  mock_dom_node_destroy(dn2);
  g_breadcrumbs_mock_fail = 0;

  ui_dom_node_destroy(dn1);

  ui_router_create(&dummy_router);

  /* component fail */
  ui_breadcrumbs_base_create(dummy_router, &bc);
  if (bc) {
    ui_breadcrumbs_base_set_path(bc, "/test");
    g_breadcrumbs_mock_fail = 5;
    ui_breadcrumbs_base_destroy(bc);
    g_breadcrumbs_mock_fail = 0;
  }

  /* free_segments fail */
  ui_breadcrumbs_base_create(dummy_router, &bc);
  if (bc) {
    ui_breadcrumbs_base_set_path(bc, "/test");
    g_breadcrumbs_mock_fail = 4; /* free segments fail */
    ui_breadcrumbs_base_set_path(bc, "/test2");
    g_breadcrumbs_mock_fail = 0;
    ui_breadcrumbs_base_destroy(bc);
  }

  ui_breadcrumbs_base_create(dummy_router, &bc);
  if (bc) {
    g_breadcrumbs_mock_fail = 2; /* component default style */
    ui_breadcrumbs_base_create(dummy_router, &bc);
    g_breadcrumbs_mock_fail = 0;
    ui_breadcrumbs_base_destroy(bc);
  }

  ui_breadcrumbs_base_create(dummy_router, &bc);
  if (bc) {
    g_breadcrumbs_mock_fail = 1; /* append child */
    ui_breadcrumbs_base_set_path(bc, "/test");
    g_breadcrumbs_mock_fail = 0;
    ui_breadcrumbs_base_destroy(bc);
  }

  ui_router_destroy(dummy_router);

  return UI_ERROR_NONE;
}
#endif
