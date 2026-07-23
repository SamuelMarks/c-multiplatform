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
struct ui_breadcrumbs_base {
  struct ui_router *router;
  struct ui_component *component;
  struct ui_dom_node *nav_node;
  struct ui_dom_node *ol_node;

  struct ui_breadcrumb_segment *segments;
  size_t segment_count;
  struct ui_signal *active_index_signal;
};

static enum ui_error internal_strndup(const char *src, size_t n,
                                      char **out_str) {
  char *copy;

  copy = (char *)UI_MALLOC(n + 1);
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
enum ui_error
ui_breadcrumbs_base_create(struct ui_router *router,
                           struct ui_breadcrumbs_base **out_breadcrumbs) {
  struct ui_breadcrumbs_base *bc;
  enum ui_error rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_breadcrumbs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bc = (struct ui_breadcrumbs_base *)UI_MALLOC(
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
  ui_dom_node_set_tag_name(bc->nav_node, "nav");
  ui_dom_node_set_attribute(bc->nav_node, "aria-label", "Breadcrumb");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &bc->ol_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(bc->ol_node, "ol");
  ui_dom_node_append_child(bc->nav_node, bc->ol_node);

  rc = ui_css_parse_stylesheet(ui_breadcrumbs_base_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  ui_component_set_default_style(bc->component, default_style);

  bc->component->shadow_root = bc->nav_node;

  *out_breadcrumbs = bc;
  return UI_ERROR_NONE;

cleanup:
  if (bc->nav_node)
    ui_dom_node_destroy(bc->nav_node);
  if (bc->component)
    ui_component_destroy(bc->component);
  UI_FREE(bc);
  return rc;
}

static void free_segments(struct ui_breadcrumbs_base *bc) {
  size_t i;
  if (!bc->segments)
    return;

  for (i = 0; i < bc->segment_count; i++) {
    if (bc->segments[i].label)
      UI_FREE(bc->segments[i].label);
    if (bc->segments[i].path)
      UI_FREE(bc->segments[i].path);

    /* If li_node exists and was appended to ol_node, we can just remove and
       destroy it. ui_dom_node_destroy is recursive. If OOM happened
       mid-creation, we might have unparented nodes. */
    if (bc->segments[i].li_node) {
      if (bc->segments[i].li_node->parent == bc->ol_node) {
        ui_dom_node_remove_child(bc->ol_node, bc->segments[i].li_node);
      }
      ui_dom_node_destroy(bc->segments[i].li_node);
    }
  }
  UI_FREE(bc->segments);
  bc->segments = NULL;
  bc->segment_count = 0;
}

void ui_breadcrumbs_base_destroy(struct ui_breadcrumbs_base *breadcrumbs) {
  if (!breadcrumbs)
    return;

  free_segments(breadcrumbs);
  ui_component_destroy(breadcrumbs->component);
  UI_FREE(breadcrumbs);
}
/** \brief ui_error */
enum ui_error
ui_breadcrumbs_base_get_component(struct ui_breadcrumbs_base *breadcrumbs,
                                  struct ui_component **out_component) {
  if (!breadcrumbs || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = breadcrumbs->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_breadcrumbs_base_set_path(struct ui_breadcrumbs_base *bc,
                                           const char *path) {
  size_t segment_capacity = 4;
  size_t count = 0;
  struct ui_breadcrumb_segment *new_segments = NULL;
  const char *p = path;
  const char *segment_start = NULL;
  size_t parsed_path_len = 0;
  char *accumulated_path = NULL;
  char *temp_acc = NULL;
  enum ui_error rc;

  if (!bc || !path)
    return UI_ERROR_INVALID_ARGUMENT;

  free_segments(bc);

  new_segments = (struct ui_breadcrumb_segment *)UI_MALLOC(
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
            (struct ui_breadcrumb_segment *)UI_MALLOC(
                sizeof(struct ui_breadcrumb_segment) * new_cap);
        if (!larger) {
          rc = UI_ERROR_OUT_OF_MEMORY;
          goto fail;
        }
        memset(larger, 0, sizeof(struct ui_breadcrumb_segment) * new_cap);
        memcpy(larger, new_segments,
               sizeof(struct ui_breadcrumb_segment) * count);
        UI_FREE(new_segments);
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
        temp_acc = (char *)UI_MALLOC(parsed_path_len + 1);
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
        UI_FREE(accumulated_path);
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
      ui_dom_node_set_tag_name(new_segments[count].li_node, "li");
      ui_dom_node_append_child(bc->ol_node, new_segments[count].li_node);

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                              &new_segments[count].a_node);
      if (rc != UI_ERROR_NONE)
        goto fail;
      ui_dom_node_set_tag_name(new_segments[count].a_node, "a");
      ui_dom_node_set_attribute(new_segments[count].a_node, "href",
                                new_segments[count].path);
      ui_dom_node_append_child(new_segments[count].li_node,
                               new_segments[count].a_node);

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT,
                              &new_segments[count].text_node);
      if (rc != UI_ERROR_NONE)
        goto fail;
      ui_dom_node_set_text_content(new_segments[count].text_node,
                                   new_segments[count].label);

      ui_dom_node_append_child(new_segments[count].a_node,
                               new_segments[count].text_node);

      count++;
    }
  }

  /* Set aria-current on the last segment */
  if (count > 0) {
    ui_dom_node_set_attribute(new_segments[count - 1].a_node, "aria-current",
                              "page");
  }

  bc->segments = new_segments;
  bc->segment_count = count;
  if (accumulated_path)
    UI_FREE(accumulated_path);
  return UI_ERROR_NONE;

fail:
  if (accumulated_path)
    UI_FREE(accumulated_path);
  bc->segments = new_segments;
  bc->segment_count = (count < segment_capacity) ? count + 1 : count;
  free_segments(bc);
  return rc;
}

enum ui_error ui_breadcrumbs_base_simulate_click(struct ui_breadcrumbs_base *bc,
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
enum ui_error
ui_breadcrumbs_base_bind_active_index(struct ui_breadcrumbs_base *widget,
                                      struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
