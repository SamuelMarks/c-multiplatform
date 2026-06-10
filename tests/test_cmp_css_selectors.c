/* clang-format off */
#include "cmp_css_selectors.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

struct test_dom_node {
  const cmp_dom_node_vtable_t *vtable;
  const char *tag;
  const char *id;
  const char *classes;
  const char *attrs[10][2];
  const struct test_dom_node *parent;
  const struct test_dom_node *prev_sibling;
  const struct test_dom_node *first_child;
  const struct test_dom_node *next_sibling;
};

static int test_get_tag(const cmp_dom_node_t *node, const char **out_tag) {
  struct test_dom_node *n = (struct test_dom_node *)node;
  if (!n || !out_tag)
    return -1;
  *out_tag = n->tag;
  return 0;
}

static int test_get_id(const cmp_dom_node_t *node, const char **out_id) {
  struct test_dom_node *n = (struct test_dom_node *)node;
  if (!n || !out_id)
    return -1;
  *out_id = n->id;
  return 0;
}

static int test_get_classes(const cmp_dom_node_t *node,
                            const char **out_classes) {
  struct test_dom_node *n = (struct test_dom_node *)node;
  if (!n || !out_classes)
    return -1;
  *out_classes = n->classes;
  return 0;
}

static int test_get_attribute(const cmp_dom_node_t *node, const char *attr_name,
                              const char **out_val) {
  struct test_dom_node *n = (struct test_dom_node *)node;
  int i;
  if (!n || !attr_name || !out_val)
    return -1;
  *out_val = NULL;
  for (i = 0; i < 10 && n->attrs[i][0] != NULL; i++) {
    if (strcmp(n->attrs[i][0], attr_name) == 0) {
      *out_val = n->attrs[i][1];
      return 0;
    }
  }
  return 0;
}

static int test_get_parent(const cmp_dom_node_t *node,
                           const cmp_dom_node_t **out_parent) {
  struct test_dom_node *n = (struct test_dom_node *)node;
  if (!n || !out_parent)
    return -1;
  *out_parent = (const cmp_dom_node_t *)n->parent;
  return 0;
}

static int test_get_prev_sibling(const cmp_dom_node_t *node,
                                 const cmp_dom_node_t **out_prev) {
  struct test_dom_node *n = (struct test_dom_node *)node;
  if (!n || !out_prev)
    return -1;
  *out_prev = (const cmp_dom_node_t *)n->prev_sibling;
  return 0;
}

static int test_match_column(const cmp_dom_node_t *node,
                             const cmp_selector_t *col_sel, int *out_match) {
  /* For testing: assume node with class "in-col" matches if column selector
   * expects .col-target */
  struct test_dom_node *n = (struct test_dom_node *)node;
  int match = 0;

  if (!n || !col_sel || !out_match)
    return -1;

  /* Mock logic: if the cell has class "in-col", simulate matching the abstract
   * col_sel by wrapping it in a dummy node and passing it. */
  if (n->classes && strstr(n->classes, "in-col")) {
    struct test_dom_node mock_col = {NULL,           "col", NULL, "col-target",
                                     {{NULL, NULL}}, NULL,  NULL};
    mock_col.vtable = n->vtable;
    cmp_selector_match(col_sel, (const cmp_dom_node_t *)&mock_col, &match);
  }

  *out_match = match;
  return 0;
}

static int test_get_first_child(const cmp_dom_node_t *node,
                                const cmp_dom_node_t **out_child) {
  struct test_dom_node *n = (struct test_dom_node *)node;
  if (!n || !out_child)
    return -1;
  *out_child = (const cmp_dom_node_t *)n->first_child;
  return 0;
}

static int test_get_next_sibling(const cmp_dom_node_t *node,
                                 const cmp_dom_node_t **out_next) {
  struct test_dom_node *n = (struct test_dom_node *)node;
  if (!n || !out_next)
    return -1;
  *out_next = (const cmp_dom_node_t *)n->next_sibling;
  return 0;
}

static int test_get_dir(const cmp_dom_node_t *node, const char **out_dir) {
  return test_get_attribute(node, "dir", out_dir);
}
static int test_get_lang(const cmp_dom_node_t *node, const char **out_lang) {
  return test_get_attribute(node, "lang", out_lang);
}
static int test_get_link_state(const cmp_dom_node_t *node, int *out_is_any_link,
                               int *out_is_link, int *out_is_visited,
                               int *out_is_local_link) {
  const char *href = NULL;
  int rc = test_get_attribute(node, "href", &href);
  if (rc != 0)
    return rc;
  if (href) {
    *out_is_any_link = 1;
    *out_is_link = 1;
    *out_is_visited = 0;
    *out_is_local_link = 0;
  } else {
    *out_is_any_link = 0;
    *out_is_link = 0;
    *out_is_visited = 0;
    *out_is_local_link = 0;
  }
  return 0;
}
static int test_is_target(const cmp_dom_node_t *node, int *out_is_target) {
  const char *target = NULL;
  int rc = test_get_attribute(node, "target-active", &target);
  if (rc != 0)
    return rc;
  *out_is_target = (target != NULL) ? 1 : 0;
  return 0;
}
static int test_is_target_within(const cmp_dom_node_t *node,
                                 int *out_is_target_within) {
  (void)node;
  *out_is_target_within = 0;
  return 0;
}
static int test_is_scope(const cmp_dom_node_t *node, int *out_is_scope) {
  (void)node;
  *out_is_scope = 0;
  return 0;
}

static int test_get_user_action_state(const cmp_dom_node_t *node,
                                      int *out_is_hover, int *out_is_active,
                                      int *out_is_focus,
                                      int *out_is_focus_visible,
                                      int *out_is_focus_within) {
  const char *hover = NULL, *active = NULL, *focus = NULL, *focus_vis = NULL,
             *focus_within = NULL;
  int rc;
  rc = test_get_attribute(node, "test-hover", &hover);
  if (rc)
    return rc;
  rc = test_get_attribute(node, "test-active", &active);
  if (rc)
    return rc;
  rc = test_get_attribute(node, "test-focus", &focus);
  if (rc)
    return rc;
  rc = test_get_attribute(node, "test-focus-visible", &focus_vis);
  if (rc)
    return rc;
  rc = test_get_attribute(node, "test-focus-within", &focus_within);
  if (rc)
    return rc;
  *out_is_hover = hover ? 1 : 0;
  *out_is_active = active ? 1 : 0;
  *out_is_focus = focus ? 1 : 0;
  *out_is_focus_visible = focus_vis ? 1 : 0;
  *out_is_focus_within = focus_within ? 1 : 0;
  return 0;
}

static int test_get_time_state(const cmp_dom_node_t *node, int *out_is_current,
                               int *out_is_past, int *out_is_future) {
  const char *current = NULL, *past = NULL, *future = NULL;
  int rc;
  rc = test_get_attribute(node, "test-current", &current);
  if (rc)
    return rc;
  rc = test_get_attribute(node, "test-past", &past);
  if (rc)
    return rc;
  rc = test_get_attribute(node, "test-future", &future);
  if (rc)
    return rc;
  *out_is_current = current ? 1 : 0;
  *out_is_past = past ? 1 : 0;
  *out_is_future = future ? 1 : 0;
  return 0;
}

static int test_get_form_state(const cmp_dom_node_t *node,
                               cmp_form_state_t *out_state) {
  const char *attr = NULL;
  int rc;
  if (!out_state)
    return -1;
  memset(out_state, 0, sizeof(*out_state));

  rc = test_get_attribute(node, "test-disabled", &attr);
  if (rc)
    return rc;
  if (attr)
    out_state->is_disabled = 1;
  else
    out_state->is_enabled = 1;

  rc = test_get_attribute(node, "test-read-only", &attr);
  if (rc)
    return rc;
  if (attr)
    out_state->is_read_only = 1;
  else
    out_state->is_read_write = 1;

  rc = test_get_attribute(node, "test-checked", &attr);
  if (rc)
    return rc;
  if (attr)
    out_state->is_checked = 1;

  rc = test_get_attribute(node, "test-valid", &attr);
  if (rc)
    return rc;
  if (attr)
    out_state->is_valid = 1;

  rc = test_get_attribute(node, "test-invalid", &attr);
  if (rc)
    return rc;
  if (attr)
    out_state->is_invalid = 1;

  rc = test_get_attribute(node, "test-required", &attr);
  if (rc)
    return rc;
  if (attr)
    out_state->is_required = 1;
  else
    out_state->is_optional = 1;

  return 0;
}

static int test_get_dom_state(const cmp_dom_node_t *node, int *out_is_modal,
                              int *out_is_fullscreen, int *out_is_pip,
                              int *out_is_popover_open, int *out_is_defined) {
  const char *attr = NULL;
  int rc;

  rc = test_get_attribute(node, "test-modal", &attr);
  if (rc)
    return rc;
  *out_is_modal = attr ? 1 : 0;

  rc = test_get_attribute(node, "test-fullscreen", &attr);
  if (rc)
    return rc;
  *out_is_fullscreen = attr ? 1 : 0;

  rc = test_get_attribute(node, "test-pip", &attr);
  if (rc)
    return rc;
  *out_is_pip = attr ? 1 : 0;

  rc = test_get_attribute(node, "test-popover-open", &attr);
  if (rc)
    return rc;
  *out_is_popover_open = attr ? 1 : 0;

  rc = test_get_attribute(node, "test-defined", &attr);
  if (rc)
    return rc;
  *out_is_defined = attr ? 1 : 0;

  return 0;
}

static int test_get_shadow_host(const cmp_dom_node_t *node,
                                const cmp_dom_node_t **out_host) {
  const char *attr = NULL;
  int rc;

  rc = test_get_attribute(node, "test-is-shadow-host", &attr);
  if (rc)
    return rc;
  if (attr && strcmp(attr, "1") == 0) {
    *out_host = node;
    return 0;
  }

  *out_host = NULL;
  return 0;
}

static int test_get_media_state(const cmp_dom_node_t *node, int *out_is_playing,
                                int *out_is_paused, int *out_is_muted,
                                int *out_is_volume_locked) {
  const char *attr = NULL;
  int rc;
  rc = test_get_attribute(node, "test-playing", &attr);
  if (rc)
    return rc;
  *out_is_playing = attr ? 1 : 0;
  rc = test_get_attribute(node, "test-paused", &attr);
  if (rc)
    return rc;
  *out_is_paused = attr ? 1 : 0;
  rc = test_get_attribute(node, "test-muted", &attr);
  if (rc)
    return rc;
  *out_is_muted = attr ? 1 : 0;
  rc = test_get_attribute(node, "test-volume-locked", &attr);
  if (rc)
    return rc;
  *out_is_volume_locked = attr ? 1 : 0;
  return 0;
}

static const cmp_dom_node_vtable_t mock_vtable = {
    test_get_tag,         test_get_id,
    test_get_classes,     test_get_attribute,
    test_get_parent,      test_get_prev_sibling,
    test_get_first_child, test_get_next_sibling,
    test_match_column,    test_get_dir,
    test_get_lang,        test_get_link_state,
    test_is_target,       test_is_target_within,
    test_is_scope,        test_get_user_action_state,
    test_get_time_state,  test_get_form_state,
    test_get_dom_state,   test_get_shadow_host,
    test_get_media_state};

static int wrap_universal(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                          int *out_match) {
  return cmp_sel_universal_match((const cmp_sel_universal_t *)sel, node,
                                 out_match);
}
static int wrap_tag(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                    int *out_match) {
  return cmp_sel_tag_match((const cmp_sel_tag_t *)sel, node, out_match);
}
static int wrap_class(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  return cmp_sel_class_match((const cmp_sel_class_t *)sel, node, out_match);
}
static int wrap_id(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                   int *out_match) {
  return cmp_sel_id_match((const cmp_sel_id_t *)sel, node, out_match);
}
static int wrap_attr(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                     int *out_match) {
  return cmp_sel_attr_match((const cmp_sel_attr_t *)sel, node, out_match);
}
static int wrap_descendant(const cmp_selector_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  return cmp_sel_descendant_match((const cmp_sel_descendant_t *)sel, node,
                                  out_match);
}
static int wrap_child(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  return cmp_sel_child_match((const cmp_sel_child_t *)sel, node, out_match);
}
static int wrap_next_sibling(const cmp_selector_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  return cmp_sel_next_sibling_match((const cmp_sel_next_sibling_t *)sel, node,
                                    out_match);
}
static int wrap_subsequent_sibling(const cmp_selector_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  return cmp_sel_subsequent_sibling_match(
      (const cmp_sel_subsequent_sibling_t *)sel, node, out_match);
}
static int wrap_column(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                       int *out_match) {
  return cmp_sel_column_match((const cmp_sel_column_t *)sel, node, out_match);
}
static int wrap_is(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                   int *out_match) {
  return cmp_sel_is_match((const cmp_sel_is_t *)sel, node, out_match);
}
static int wrap_where(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  return cmp_sel_where_match((const cmp_sel_where_t *)sel, node, out_match);
}
static int wrap_has(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                    int *out_match) {
  return cmp_sel_has_match((const cmp_sel_has_t *)sel, node, out_match);
}
static int wrap_not(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                    int *out_match) {
  return cmp_sel_not_match((const cmp_sel_not_t *)sel, node, out_match);
}

TEST test_is_selector(void) {
  struct test_dom_node node = {&mock_vtable, "div",          "header",
                               "cls1 cls2",  {{NULL, NULL}}, NULL,
                               NULL,         NULL,           NULL};
  cmp_sel_tag_t sel1 = {{wrap_tag}, "span"};
  cmp_sel_id_t sel2 = {{wrap_id}, "header"};
  const cmp_selector_t *selectors[2];
  cmp_sel_is_t is_sel = {{wrap_is}, NULL, 2};
  int match = 0;

  selectors[0] = (cmp_selector_t *)&sel1;
  selectors[1] = (cmp_selector_t *)&sel2;
  is_sel.selectors = selectors;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&is_sel,
                                  (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match);

  sel2.id_name = "footer";
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&is_sel,
                                  (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(0, match);

  PASS();
}

TEST test_where_selector(void) {
  struct test_dom_node node = {&mock_vtable, "div",          "header",
                               "cls1 cls2",  {{NULL, NULL}}, NULL,
                               NULL,         NULL,           NULL};
  cmp_sel_tag_t sel1 = {{wrap_tag}, "span"};
  cmp_sel_id_t sel2 = {{wrap_id}, "header"};
  const cmp_selector_t *selectors[2];
  cmp_sel_where_t where_sel = {{wrap_where}, NULL, 2};
  int match = 0;

  selectors[0] = (cmp_selector_t *)&sel1;
  selectors[1] = (cmp_selector_t *)&sel2;
  where_sel.selectors = selectors;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&where_sel,
                                  (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match);

  sel2.id_name = "footer";
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&where_sel,
                                  (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(0, match);

  PASS();
}

TEST test_not_selector(void) {
  struct test_dom_node node = {&mock_vtable, "div",          "header",
                               "cls1 cls2",  {{NULL, NULL}}, NULL,
                               NULL,         NULL,           NULL};
  cmp_sel_tag_t sel1 = {{wrap_tag}, "span"};
  cmp_sel_id_t sel2 = {{wrap_id}, "footer"};
  const cmp_selector_t *selectors[2];
  cmp_sel_not_t not_sel = {{wrap_not}, NULL, 2};
  int match = 0;

  selectors[0] = (cmp_selector_t *)&sel1;
  selectors[1] = (cmp_selector_t *)&sel2;
  not_sel.selectors = selectors;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&not_sel,
                                  (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match); /* neither 'span' nor '#footer' match 'div#header', so
                          :not matches */

  sel2.id_name = "header";
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&not_sel,
                                  (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(0, match); /* '#header' matches, so :not fails */

  PASS();
}

TEST test_has_selector(void) {
  struct test_dom_node parent = {&mock_vtable, "div",          "main",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child1 = {&mock_vtable, "span",         "child1",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child2 = {
      &mock_vtable, "a",  "child2", "active", {{NULL, NULL}},
      NULL,         NULL, NULL,     NULL};

  cmp_sel_class_t sel_active = {{wrap_class}, "active"};
  cmp_sel_tag_t sel_p = {{wrap_tag}, "p"};
  const cmp_selector_t *selectors1[1];
  const cmp_selector_t *selectors2[1];
  cmp_sel_has_t has_sel1 = {{wrap_has}, NULL, 1};
  cmp_sel_has_t has_sel2 = {{wrap_has}, NULL, 1};
  int match = 0;

  /* Link up tree */
  parent.first_child = &child1;
  child1.parent = &parent;
  child1.next_sibling = &child2;
  child2.parent = &parent;
  child2.prev_sibling = &child1;

  selectors1[0] = (cmp_selector_t *)&sel_active;
  selectors2[0] = (cmp_selector_t *)&sel_p;
  has_sel1.selectors = selectors1;
  has_sel2.selectors = selectors2;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&has_sel1,
                                  (cmp_dom_node_t *)&parent, &match));
  ASSERT_EQ(1, match); /* parent has descendant 'child2' with class 'active' */

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&has_sel2,
                                  (cmp_dom_node_t *)&parent, &match));
  ASSERT_EQ(0, match); /* parent does not have descendant 'p' */

  PASS();
}

TEST test_universal_selector(void) {
  struct test_dom_node node = {&mock_vtable,   "div", "header", "cls1 cls2",
                               {{NULL, NULL}}, NULL,  NULL};
  cmp_sel_universal_t sel = {{wrap_universal}};
  int match = 0;

  ASSERT_EQ(0, cmp_sel_universal_match(&sel, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(-1, cmp_sel_universal_match(NULL, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(-1, cmp_sel_universal_match(&sel, NULL, &match));
  ASSERT_EQ(-1, cmp_sel_universal_match(&sel, (cmp_dom_node_t *)&node, NULL));

  PASS();
}

TEST test_tag_selector(void) {
  struct test_dom_node node = {&mock_vtable,   "div", NULL, NULL,
                               {{NULL, NULL}}, NULL,  NULL};
  cmp_sel_tag_t sel = {{wrap_tag}, "div"};
  cmp_sel_tag_t sel2 = {{wrap_tag}, "DIV"};
  cmp_sel_tag_t sel3 = {{wrap_tag}, "span"};
  int match = 0;

  ASSERT_EQ(0, cmp_sel_tag_match(&sel, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_sel_tag_match(&sel2, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match); /* Case insensitive */

  ASSERT_EQ(0, cmp_sel_tag_match(&sel3, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(-1, cmp_sel_tag_match(NULL, (cmp_dom_node_t *)&node, &match));

  PASS();
}

TEST test_class_selector(void) {
  struct test_dom_node node = {
      &mock_vtable,   "div", NULL, "container active btn-primary",
      {{NULL, NULL}}, NULL,  NULL};
  cmp_sel_class_t sel = {{wrap_class}, "active"};
  cmp_sel_class_t sel2 = {{wrap_class}, "btn"};
  cmp_sel_class_t sel3 = {{wrap_class}, "container"};
  cmp_sel_class_t sel4 = {{wrap_class}, "btn-primary"};
  int match = 0;

  ASSERT_EQ(0, cmp_sel_class_match(&sel, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_sel_class_match(&sel2, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(0, match); /* Not a full word match */

  ASSERT_EQ(0, cmp_sel_class_match(&sel3, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_sel_class_match(&sel4, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match);

  PASS();
}

TEST test_id_selector(void) {
  struct test_dom_node node = {&mock_vtable,   "div", "main-header", NULL,
                               {{NULL, NULL}}, NULL,  NULL};
  cmp_sel_id_t sel = {{wrap_id}, "main-header"};
  cmp_sel_id_t sel2 = {{wrap_id}, "MAIN-HEADER"};
  cmp_sel_id_t sel3 = {{wrap_id}, "header"};
  int match = 0;

  ASSERT_EQ(0, cmp_sel_id_match(&sel, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_sel_id_match(&sel2, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(0, match); /* Case sensitive */

  ASSERT_EQ(0, cmp_sel_id_match(&sel3, (cmp_dom_node_t *)&node, &match));
  ASSERT_EQ(0, match);

  PASS();
}

TEST test_attr_selector(void) {
  struct test_dom_node node = {&mock_vtable,
                               "input",
                               NULL,
                               NULL,
                               {{"type", "text"},
                                {"data-val", "abc-def"},
                                {"lang", "en-US"},
                                {NULL, NULL}},
                               NULL,
                               NULL};
  int match = 0;

  /* EXISTS */
  {
    cmp_sel_attr_t sel = {{wrap_attr},
                          "type",
                          NULL,
                          CMP_SEL_ATTR_OP_EXISTS,
                          CMP_SEL_ATTR_MOD_NONE};
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match);
    sel.attr_name = "disabled";
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(0, match);
  }

  /* EQUALS */
  {
    cmp_sel_attr_t sel = {{wrap_attr},
                          "type",
                          "text",
                          CMP_SEL_ATTR_OP_EQUALS,
                          CMP_SEL_ATTR_MOD_NONE};
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match);
    sel.attr_value = "TEXT";
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(0, match); /* Case sensitive by default */
    sel.modifier = CMP_SEL_ATTR_MOD_NOCASE;
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match); /* Case insensitive */
  }

  /* PREFIX */
  {
    cmp_sel_attr_t sel = {{wrap_attr},
                          "data-val",
                          "abc",
                          CMP_SEL_ATTR_OP_PREFIX,
                          CMP_SEL_ATTR_MOD_NONE};
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match);
    sel.attr_value = "xyz";
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(0, match);
  }

  /* SUFFIX */
  {
    cmp_sel_attr_t sel = {{wrap_attr},
                          "data-val",
                          "def",
                          CMP_SEL_ATTR_OP_SUFFIX,
                          CMP_SEL_ATTR_MOD_NONE};
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match);
    sel.attr_value = "de";
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(0, match);
  }

  /* CONTAINS */
  {
    cmp_sel_attr_t sel = {{wrap_attr},
                          "data-val",
                          "c-d",
                          CMP_SEL_ATTR_OP_CONTAINS,
                          CMP_SEL_ATTR_MOD_NONE};
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match);
    sel.modifier = CMP_SEL_ATTR_MOD_NOCASE;
    sel.attr_value = "C-D";
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match);
  }

  /* DASH */
  {
    cmp_sel_attr_t sel = {
        {wrap_attr}, "lang", "en", CMP_SEL_ATTR_OP_DASH, CMP_SEL_ATTR_MOD_NONE};
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match);
    sel.attr_value = "en-US";
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(1, match);
    sel.attr_value = "e";
    ASSERT_EQ(0, cmp_sel_attr_match(&sel, (cmp_dom_node_t *)&node, &match));
    ASSERT_EQ(0, match);
  }

  PASS();
}

TEST test_descendant_combinator(void) {
  struct test_dom_node parent = {&mock_vtable,   "div", "main", NULL,
                                 {{NULL, NULL}}, NULL,  NULL};
  struct test_dom_node child = {&mock_vtable,   "span", "child", NULL,
                                {{NULL, NULL}}, NULL,   NULL};
  struct test_dom_node grandchild = {&mock_vtable,   "a",  "gc", NULL,
                                     {{NULL, NULL}}, NULL, NULL};

  cmp_sel_id_t left = {{wrap_id}, "main"};
  cmp_sel_tag_t right = {{wrap_tag}, "a"};
  cmp_sel_descendant_t comb = {{wrap_descendant}, NULL, NULL};

  int match = 0;

  child.parent = &parent;
  grandchild.parent = &child;
  comb.left = (cmp_selector_t *)&left;
  comb.right = (cmp_selector_t *)&right;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&grandchild, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&child, &match));
  ASSERT_EQ(0, match);

  PASS();
}

TEST test_child_combinator(void) {
  struct test_dom_node parent = {&mock_vtable,   "div", "main", NULL,
                                 {{NULL, NULL}}, NULL,  NULL};
  struct test_dom_node child = {&mock_vtable,   "span", "child", NULL,
                                {{NULL, NULL}}, NULL,   NULL};
  struct test_dom_node grandchild = {&mock_vtable,   "a",  "gc", NULL,
                                     {{NULL, NULL}}, NULL, NULL};

  cmp_sel_id_t left = {{wrap_id}, "main"};
  cmp_sel_tag_t right = {{wrap_tag}, "span"};
  cmp_sel_child_t comb = {{wrap_child}, NULL, NULL};

  int match = 0;

  child.parent = &parent;
  grandchild.parent = &child;
  comb.left = (cmp_selector_t *)&left;
  comb.right = (cmp_selector_t *)&right;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&child, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&grandchild, &match));
  ASSERT_EQ(0, match);

  PASS();
}

TEST test_next_sibling_combinator(void) {
  struct test_dom_node sibling1 = {&mock_vtable,   "h1", "title", NULL,
                                   {{NULL, NULL}}, NULL, NULL};
  struct test_dom_node sibling2 = {&mock_vtable,   "p",  "desc", NULL,
                                   {{NULL, NULL}}, NULL, NULL};
  struct test_dom_node sibling3 = {&mock_vtable,   "p",  "desc2", NULL,
                                   {{NULL, NULL}}, NULL, NULL};

  cmp_sel_tag_t left = {{wrap_tag}, "h1"};
  cmp_sel_tag_t right = {{wrap_tag}, "p"};
  cmp_sel_next_sibling_t comb = {{wrap_next_sibling}, NULL, NULL};

  int match = 0;

  sibling2.prev_sibling = &sibling1;
  sibling3.prev_sibling = &sibling2;
  comb.left = (cmp_selector_t *)&left;
  comb.right = (cmp_selector_t *)&right;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&sibling2, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&sibling3, &match));
  ASSERT_EQ(0, match);

  PASS();
}

TEST test_subsequent_sibling_combinator(void) {
  struct test_dom_node sibling1 = {&mock_vtable,   "h1", "title", NULL,
                                   {{NULL, NULL}}, NULL, NULL};
  struct test_dom_node sibling2 = {&mock_vtable,   "div", "desc", NULL,
                                   {{NULL, NULL}}, NULL,  NULL};
  struct test_dom_node sibling3 = {&mock_vtable,   "p",  "desc2", NULL,
                                   {{NULL, NULL}}, NULL, NULL};

  cmp_sel_tag_t left = {{wrap_tag}, "h1"};
  cmp_sel_tag_t right = {{wrap_tag}, "p"};
  cmp_sel_subsequent_sibling_t comb = {{wrap_subsequent_sibling}, NULL, NULL};

  int match = 0;

  sibling2.prev_sibling = &sibling1;
  sibling3.prev_sibling = &sibling2;
  comb.left = (cmp_selector_t *)&left;
  comb.right = (cmp_selector_t *)&right;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&sibling3, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&sibling2, &match));
  ASSERT_EQ(0, match);

  PASS();
}

TEST test_column_combinator(void) {
  struct test_dom_node cell_in_col = {&mock_vtable,   "td", NULL, "in-col",
                                      {{NULL, NULL}}, NULL, NULL};
  struct test_dom_node cell_out_col = {&mock_vtable,   "td", NULL, "out-col",
                                       {{NULL, NULL}}, NULL, NULL};

  cmp_sel_class_t left = {{wrap_class}, "col-target"};
  cmp_sel_tag_t right = {{wrap_tag}, "td"};
  cmp_sel_column_t comb = {{wrap_column}, NULL, NULL};

  int match = 0;

  comb.left = (cmp_selector_t *)&left;
  comb.right = (cmp_selector_t *)&right;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&cell_in_col, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&comb,
                                  (cmp_dom_node_t *)&cell_out_col, &match));
  ASSERT_EQ(0, match);

  PASS();
}

static int wrap_empty(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  return cmp_pseudo_empty_match((const cmp_pseudo_empty_t *)sel, node,
                                out_match);
}
static int wrap_root(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                     int *out_match) {
  return cmp_pseudo_root_match((const cmp_pseudo_root_t *)sel, node, out_match);
}
static int wrap_nth_child(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                          int *out_match) {
  return cmp_pseudo_nth_child_match((const cmp_pseudo_nth_child_t *)sel, node,
                                    out_match);
}
static int wrap_nth_last_child(const cmp_selector_t *sel,
                               const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_nth_last_child_match(
      (const cmp_pseudo_nth_last_child_t *)sel, node, out_match);
}
static int wrap_first_child(const cmp_selector_t *sel,
                            const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_first_child_match((const cmp_pseudo_first_child_t *)sel,
                                      node, out_match);
}
static int wrap_last_child(const cmp_selector_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_last_child_match((const cmp_pseudo_last_child_t *)sel, node,
                                     out_match);
}
static int wrap_only_child(const cmp_selector_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_only_child_match((const cmp_pseudo_only_child_t *)sel, node,
                                     out_match);
}
static int wrap_nth_of_type(const cmp_selector_t *sel,
                            const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_nth_of_type_match((const cmp_pseudo_nth_of_type_t *)sel,
                                      node, out_match);
}
static int wrap_nth_last_of_type(const cmp_selector_t *sel,
                                 const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_nth_last_of_type_match(
      (const cmp_pseudo_nth_last_of_type_t *)sel, node, out_match);
}
static int wrap_first_of_type(const cmp_selector_t *sel,
                              const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_first_of_type_match((const cmp_pseudo_first_of_type_t *)sel,
                                        node, out_match);
}
static int wrap_last_of_type(const cmp_selector_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_last_of_type_match((const cmp_pseudo_last_of_type_t *)sel,
                                       node, out_match);
}
static int wrap_only_of_type(const cmp_selector_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_only_of_type_match((const cmp_pseudo_only_of_type_t *)sel,
                                       node, out_match);
}

TEST test_empty_selector(void) {
  struct test_dom_node parent = {&mock_vtable, "div",          "main",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child = {&mock_vtable, "span",         "child1",
                                NULL,         {{NULL, NULL}}, NULL,
                                NULL,         NULL,           NULL};
  cmp_pseudo_empty_t empty_sel = {{wrap_empty}};
  int match = 0;

  parent.first_child = &child;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&empty_sel,
                                  (cmp_dom_node_t *)&parent, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&empty_sel,
                                  (cmp_dom_node_t *)&child, &match));
  ASSERT_EQ(1, match);

  PASS();
}

TEST test_root_selector(void) {
  struct test_dom_node parent = {&mock_vtable, "div",          "main",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child = {&mock_vtable, "span",         "child1",
                                NULL,         {{NULL, NULL}}, NULL,
                                NULL,         NULL,           NULL};
  cmp_pseudo_root_t root_sel = {{wrap_root}};
  int match = 0;

  child.parent = &parent;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&root_sel,
                                  (cmp_dom_node_t *)&parent, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&root_sel,
                                  (cmp_dom_node_t *)&child, &match));
  ASSERT_EQ(0, match);

  PASS();
}

TEST test_nth_child_selector(void) {
  struct test_dom_node child1 = {&mock_vtable, "span",         "child1",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child2 = {&mock_vtable, "div",          "child2",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child3 = {
      &mock_vtable, "p",  "child3", NULL, {{NULL, NULL}},
      NULL,         NULL, NULL,     NULL};
  struct test_dom_node child4 = {
      &mock_vtable, "a",  "child4", NULL, {{NULL, NULL}},
      NULL,         NULL, NULL,     NULL};

  cmp_pseudo_nth_child_t nth_even = {{wrap_nth_child}, {2, 0}, NULL};
  cmp_pseudo_nth_child_t nth_odd = {{wrap_nth_child}, {2, 1}, NULL};
  cmp_pseudo_nth_child_t nth_3 = {{wrap_nth_child}, {0, 3}, NULL};
  int match = 0;

  child2.prev_sibling = &child1;
  child3.prev_sibling = &child2;
  child4.prev_sibling = &child3;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_even,
                                  (cmp_dom_node_t *)&child1, &match));
  ASSERT_EQ(0, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_even,
                                  (cmp_dom_node_t *)&child2, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_even,
                                  (cmp_dom_node_t *)&child3, &match));
  ASSERT_EQ(0, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_even,
                                  (cmp_dom_node_t *)&child4, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_odd,
                                  (cmp_dom_node_t *)&child1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_odd,
                                  (cmp_dom_node_t *)&child2, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_3,
                                  (cmp_dom_node_t *)&child2, &match));
  ASSERT_EQ(0, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_3,
                                  (cmp_dom_node_t *)&child3, &match));
  ASSERT_EQ(1, match);

  PASS();
}

TEST test_first_last_child(void) {
  struct test_dom_node child1 = {&mock_vtable, "span",         "child1",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child2 = {&mock_vtable, "div",          "child2",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child3 = {
      &mock_vtable, "p",  "child3", NULL, {{NULL, NULL}},
      NULL,         NULL, NULL,     NULL};

  cmp_pseudo_first_child_t first_sel = {{wrap_first_child}};
  cmp_pseudo_last_child_t last_sel = {{wrap_last_child}};
  cmp_pseudo_only_child_t only_sel = {{wrap_only_child}};
  int match = 0;

  child2.prev_sibling = &child1;
  child3.prev_sibling = &child2;
  child1.next_sibling = &child2;
  child2.next_sibling = &child3;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&first_sel,
                                  (cmp_dom_node_t *)&child1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&first_sel,
                                  (cmp_dom_node_t *)&child2, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&last_sel,
                                  (cmp_dom_node_t *)&child3, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&last_sel,
                                  (cmp_dom_node_t *)&child2, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&only_sel,
                                  (cmp_dom_node_t *)&child1, &match));
  ASSERT_EQ(0, match);

  child1.next_sibling = NULL; /* Make child1 the only child */
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&only_sel,
                                  (cmp_dom_node_t *)&child1, &match));
  ASSERT_EQ(1, match);

  PASS();
}

TEST test_nth_of_type(void) {
  struct test_dom_node child1 = {
      &mock_vtable, "p",  "child1", NULL, {{NULL, NULL}},
      NULL,         NULL, NULL,     NULL};
  struct test_dom_node child2 = {&mock_vtable, "div",          "child2",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child3 = {
      &mock_vtable, "p",  "child3", NULL, {{NULL, NULL}},
      NULL,         NULL, NULL,     NULL};
  struct test_dom_node child4 = {&mock_vtable, "span",         "child4",
                                 NULL,         {{NULL, NULL}}, NULL,
                                 NULL,         NULL,           NULL};
  struct test_dom_node child5 = {
      &mock_vtable, "p",  "child5", NULL, {{NULL, NULL}},
      NULL,         NULL, NULL,     NULL};

  cmp_pseudo_nth_of_type_t nth_2 = {{wrap_nth_of_type}, {0, 2}};
  cmp_pseudo_first_of_type_t first = {{wrap_first_of_type}};
  cmp_pseudo_last_of_type_t last = {{wrap_last_of_type}};
  cmp_pseudo_only_of_type_t only = {{wrap_only_of_type}};
  int match = 0;

  child2.prev_sibling = &child1;
  child3.prev_sibling = &child2;
  child4.prev_sibling = &child3;
  child5.prev_sibling = &child4;

  child1.next_sibling = &child2;
  child2.next_sibling = &child3;
  child3.next_sibling = &child4;
  child4.next_sibling = &child5;

  /* child3 is the 2nd 'p' element */
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_2,
                                  (cmp_dom_node_t *)&child1, &match));
  ASSERT_EQ(0, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_2,
                                  (cmp_dom_node_t *)&child3, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&nth_2,
                                  (cmp_dom_node_t *)&child5, &match));
  ASSERT_EQ(0, match);

  /* first/last/only */
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&first,
                                  (cmp_dom_node_t *)&child1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&last,
                                  (cmp_dom_node_t *)&child5, &match));
  ASSERT_EQ(1, match);

  /* child4 is the only 'span' */
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&only,
                                  (cmp_dom_node_t *)&child1, &match));
  ASSERT_EQ(0, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&only,
                                  (cmp_dom_node_t *)&child4, &match));
  ASSERT_EQ(1, match);

  PASS();
}

static int wrap_dir(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                    int *out_match) {
  return cmp_pseudo_dir_match((const cmp_pseudo_dir_t *)sel, node, out_match);
}
static int wrap_lang(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                     int *out_match) {
  return cmp_pseudo_lang_match((const cmp_pseudo_lang_t *)sel, node, out_match);
}
static int wrap_any_link(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                         int *out_match) {
  return cmp_pseudo_any_link_match((const cmp_pseudo_any_link_t *)sel, node,
                                   out_match);
}
static int wrap_target(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                       int *out_match) {
  return cmp_pseudo_target_match((const cmp_pseudo_target_t *)sel, node,
                                 out_match);
}

TEST test_linguistic_doc_selectors(void) {
  struct test_dom_node node1 = {
      &mock_vtable,
      "div",
      "d1",
      NULL,
      {{"dir", "rtl"}, {"lang", "fr-CA"}, {NULL, NULL}},
      NULL,
      NULL,
      NULL,
      NULL};
  struct test_dom_node node2 = {&mock_vtable,
                                "a",
                                "l1",
                                NULL,
                                {{"href", "https://example.com"}, {NULL, NULL}},
                                NULL,
                                NULL,
                                NULL,
                                NULL};
  struct test_dom_node node3 = {
      &mock_vtable, "div", "t1", NULL, {{"target-active", "1"}, {NULL, NULL}},
      NULL,         NULL,  NULL, NULL};

  cmp_pseudo_dir_t dir_rtl = {{wrap_dir}, "rtl"};
  cmp_pseudo_dir_t dir_ltr = {{wrap_dir}, "ltr"};

  const char *langs[] = {"fr"};
  cmp_pseudo_lang_t lang_fr = {{wrap_lang}, NULL, 1};

  cmp_pseudo_any_link_t any_link = {{wrap_any_link}};
  cmp_pseudo_target_t target = {{wrap_target}};
  int match = 0;

  lang_fr.langs = langs;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&dir_rtl,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&dir_ltr,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&lang_fr,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&any_link,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&any_link,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&target,
                                  (cmp_dom_node_t *)&node3, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&target,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(0, match);

  PASS();
}

static int wrap_hover(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  return cmp_pseudo_hover_match((const cmp_pseudo_hover_t *)sel, node,
                                out_match);
}
static int wrap_active(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                       int *out_match) {
  return cmp_pseudo_active_match((const cmp_pseudo_active_t *)sel, node,
                                 out_match);
}
static int wrap_focus(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  return cmp_pseudo_focus_match((const cmp_pseudo_focus_t *)sel, node,
                                out_match);
}
static int wrap_current(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                        int *out_match) {
  return cmp_pseudo_current_match((const cmp_pseudo_current_t *)sel, node,
                                  out_match);
}

TEST test_user_action_and_time_selectors(void) {
  struct test_dom_node node1 = {&mock_vtable,
                                "button",
                                "btn1",
                                NULL,
                                {{"test-hover", "1"},
                                 {"test-focus", "1"},
                                 {"test-current", "1"},
                                 {NULL, NULL}},
                                NULL,
                                NULL,
                                NULL,
                                NULL};

  struct test_dom_node node2 = {
      &mock_vtable,
      "button",
      "btn2",
      NULL,
      {{"test-active", "1"}, {"test-past", "1"}, {NULL, NULL}},
      NULL,
      NULL,
      NULL,
      NULL};

  cmp_pseudo_hover_t hover = {{wrap_hover}};
  cmp_pseudo_active_t active = {{wrap_active}};
  cmp_pseudo_focus_t focus = {{wrap_focus}};
  cmp_pseudo_current_t current = {{wrap_current}};
  int match = 0;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&hover,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&hover,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&active,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&active,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&focus,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&current,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);

  PASS();
}

static int wrap_enabled_disabled(const cmp_selector_t *sel,
                                 const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_enabled_disabled_match(
      (const cmp_pseudo_enabled_disabled_t *)sel, node, out_match);
}
static int wrap_checked(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                        int *out_match) {
  return cmp_pseudo_checked_match((const cmp_pseudo_checked_t *)sel, node,
                                  out_match);
}
static int wrap_valid_invalid(const cmp_selector_t *sel,
                              const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_valid_invalid_match((const cmp_pseudo_valid_invalid_t *)sel,
                                        node, out_match);
}

TEST test_input_form_selectors(void) {
  struct test_dom_node node1 = {&mock_vtable,
                                "input",
                                "input1",
                                NULL,
                                {{"test-disabled", "1"},
                                 {"test-checked", "1"},
                                 {"test-invalid", "1"},
                                 {NULL, NULL}},
                                NULL,
                                NULL,
                                NULL,
                                NULL};

  struct test_dom_node node2 = {&mock_vtable,
                                "input",
                                "input2",
                                NULL,
                                {{"test-valid", "1"}, {NULL, NULL}},
                                NULL,
                                NULL,
                                NULL,
                                NULL};

  cmp_pseudo_enabled_disabled_t disabled = {{wrap_enabled_disabled}, 1};
  cmp_pseudo_enabled_disabled_t enabled = {{wrap_enabled_disabled}, 0};
  cmp_pseudo_checked_t checked = {{wrap_checked}};
  cmp_pseudo_valid_invalid_t invalid = {{wrap_valid_invalid}, 1};
  cmp_pseudo_valid_invalid_t valid = {{wrap_valid_invalid}, 0};
  int match = 0;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&disabled,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&enabled,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(0, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&enabled,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&checked,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&checked,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&invalid,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&valid,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(0, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&valid,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(1, match);

  PASS();
}

static int wrap_host(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                     int *out_match) {
  return cmp_pseudo_host_match((const cmp_pseudo_host_t *)sel, node, out_match);
}
static int wrap_host_func(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                          int *out_match) {
  return cmp_pseudo_host_func_match((const cmp_pseudo_host_func_t *)sel, node,
                                    out_match);
}
static int wrap_host_context(const cmp_selector_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_host_context_match((const cmp_pseudo_host_context_t *)sel,
                                       node, out_match);
}
static int wrap_modal(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  return cmp_pseudo_modal_match((const cmp_pseudo_modal_t *)sel, node,
                                out_match);
}
static int wrap_fullscreen(const cmp_selector_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_fullscreen_match((const cmp_pseudo_fullscreen_t *)sel, node,
                                     out_match);
}

TEST test_dom_and_shadow_selectors(void) {
  struct test_dom_node host_node = {&mock_vtable,
                                    "my-element",
                                    "el1",
                                    NULL,
                                    {{"test-is-shadow-host", "1"},
                                     {"test-modal", "1"},
                                     {"test-fullscreen", "1"},
                                     {NULL, NULL}},
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL};
  struct test_dom_node normal_node = {
      &mock_vtable, "div", "el2", NULL, {{NULL, NULL}}, NULL, NULL, NULL, NULL};

  cmp_sel_tag_t my_el_sel = {{wrap_tag}, "my-element"};
  cmp_sel_tag_t div_sel = {{wrap_tag}, "div"};

  cmp_pseudo_host_t host_sel = {{wrap_host}};
  cmp_pseudo_host_func_t host_func_sel_my_el = {{wrap_host_func}, NULL};
  cmp_pseudo_host_func_t host_func_sel_div = {{wrap_host_func}, NULL};
  cmp_pseudo_host_context_t host_context_sel = {{wrap_host_context}, NULL};

  cmp_pseudo_modal_t modal_sel = {{wrap_modal}};
  cmp_pseudo_fullscreen_t fullscreen_sel = {{wrap_fullscreen}};

  int match = 0;

  host_func_sel_my_el.selector = (const cmp_selector_t *)&my_el_sel;
  host_func_sel_div.selector = (const cmp_selector_t *)&div_sel;
  host_context_sel.selector = (const cmp_selector_t *)&my_el_sel;

  host_node.parent = &normal_node;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&host_sel,
                                  (cmp_dom_node_t *)&host_node, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&host_sel,
                                  (cmp_dom_node_t *)&normal_node, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&host_func_sel_my_el,
                                  (cmp_dom_node_t *)&host_node, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&host_func_sel_div,
                                  (cmp_dom_node_t *)&host_node, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&host_context_sel,
                                  (cmp_dom_node_t *)&host_node, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&modal_sel,
                                  (cmp_dom_node_t *)&host_node, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&fullscreen_sel,
                                  (cmp_dom_node_t *)&host_node, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&modal_sel,
                                  (cmp_dom_node_t *)&normal_node, &match));
  ASSERT_EQ(0, match);

  PASS();
}

static int wrap_media_state(const cmp_selector_t *sel,
                            const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_media_state_match((const cmp_pseudo_media_state_t *)sel,
                                      node, out_match);
}
static int wrap_media_volume(const cmp_selector_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_media_volume_match((const cmp_pseudo_media_volume_t *)sel,
                                       node, out_match);
}

TEST test_media_selectors(void) {
  struct test_dom_node node1 = {
      &mock_vtable,
      "video",
      "vid1",
      NULL,
      {{"test-playing", "1"}, {"test-muted", "1"}, {NULL, NULL}},
      NULL,
      NULL,
      NULL,
      NULL};

  struct test_dom_node node2 = {
      &mock_vtable,
      "audio",
      "aud1",
      NULL,
      {{"test-paused", "1"}, {"test-volume-locked", "1"}, {NULL, NULL}},
      NULL,
      NULL,
      NULL,
      NULL};

  cmp_pseudo_media_state_t playing = {{wrap_media_state}, 0};
  cmp_pseudo_media_state_t paused = {{wrap_media_state}, 1};
  cmp_pseudo_media_volume_t muted = {{wrap_media_volume}, 0};
  cmp_pseudo_media_volume_t vol_locked = {{wrap_media_volume}, 1};
  int match = 0;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&playing,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);
  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&playing,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&paused,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&muted,
                                  (cmp_dom_node_t *)&node1, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&vol_locked,
                                  (cmp_dom_node_t *)&node2, &match));
  ASSERT_EQ(1, match);

  PASS();
}

static int wrap_pseudo_elem_before_after(const cmp_selector_t *sel,
                                         const cmp_dom_node_t *node,
                                         int *out_match) {
  return cmp_pseudo_elem_before_after_match(
      (const cmp_pseudo_elem_before_after_t *)sel, node, out_match);
}

static int wrap_pseudo_elem_slotted(const cmp_selector_t *sel,
                                    const cmp_dom_node_t *node,
                                    int *out_match) {
  return cmp_pseudo_elem_slotted_match((const cmp_pseudo_elem_slotted_t *)sel,
                                       node, out_match);
}

static int wrap_pseudo_elem_part(const cmp_selector_t *sel,
                                 const cmp_dom_node_t *node, int *out_match) {
  return cmp_pseudo_elem_part_match((const cmp_pseudo_elem_part_t *)sel, node,
                                    out_match);
}

static int stub_is_slotted_true(const cmp_dom_node_t *node,
                                int *out_is_slotted) {
  (void)node;
  *out_is_slotted = 1;
  return 0;
}

static int stub_is_slotted_false(const cmp_dom_node_t *node,
                                 int *out_is_slotted) {
  (void)node;
  *out_is_slotted = 0;
  return 0;
}

static int stub_has_part_true(const cmp_dom_node_t *node, const char *part_name,
                              int *out_has_part) {
  (void)node;
  (void)part_name;
  *out_has_part = 1;
  return 0;
}

static int stub_has_part_false(const cmp_dom_node_t *node,
                               const char *part_name, int *out_has_part) {
  (void)node;
  (void)part_name;
  *out_has_part = 0;
  return 0;
}

TEST test_pseudo_elements_selectors(void) {
  cmp_dom_node_vtable_t vt_slotted = mock_vtable;
  cmp_dom_node_vtable_t vt_not_slotted = mock_vtable;
  cmp_dom_node_vtable_t vt_part = mock_vtable;
  cmp_dom_node_vtable_t vt_not_part = mock_vtable;
  struct test_dom_node node_slotted = {
      NULL, "div", "id", "class", {{NULL, NULL}}, NULL, NULL, NULL, NULL};
  struct test_dom_node node_not_slotted = {
      NULL, "div", "id", "class", {{NULL, NULL}}, NULL, NULL, NULL, NULL};
  struct test_dom_node node_part = {NULL, "div", "id", "class", {{NULL, NULL}},
                                    NULL, NULL,  NULL, NULL};
  struct test_dom_node node_not_part = {
      NULL, "div", "id", "class", {{NULL, NULL}}, NULL, NULL, NULL, NULL};
  cmp_pseudo_elem_before_after_t before = {{wrap_pseudo_elem_before_after}, 0};
  cmp_pseudo_elem_slotted_t slotted = {{wrap_pseudo_elem_slotted}, NULL};
  const char *parts[] = {"my-part"};
  cmp_pseudo_elem_part_t part = {{wrap_pseudo_elem_part}, NULL, 1};
  int match = 0;

  node_slotted.vtable = &vt_slotted;
  node_not_slotted.vtable = &vt_not_slotted;
  node_part.vtable = &vt_part;
  node_not_part.vtable = &vt_not_part;

  part.parts = parts;

  vt_slotted.is_slotted = stub_is_slotted_true;
  vt_not_slotted.is_slotted = stub_is_slotted_false;
  vt_part.has_part = stub_has_part_true;
  vt_not_part.has_part = stub_has_part_false;

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&before,
                                  (cmp_dom_node_t *)&node_slotted, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&slotted,
                                  (cmp_dom_node_t *)&node_slotted, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&slotted,
                                  (cmp_dom_node_t *)&node_not_slotted, &match));
  ASSERT_EQ(0, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&part,
                                  (cmp_dom_node_t *)&node_part, &match));
  ASSERT_EQ(1, match);

  ASSERT_EQ(0, cmp_selector_match((cmp_selector_t *)&part,
                                  (cmp_dom_node_t *)&node_not_part, &match));
  ASSERT_EQ(0, match);

  PASS();
}

SUITE(css_selectors_suite) {
  RUN_TEST(test_universal_selector);
  RUN_TEST(test_tag_selector);
  RUN_TEST(test_class_selector);
  RUN_TEST(test_id_selector);
  RUN_TEST(test_attr_selector);
  RUN_TEST(test_descendant_combinator);
  RUN_TEST(test_child_combinator);
  RUN_TEST(test_next_sibling_combinator);
  RUN_TEST(test_subsequent_sibling_combinator);
  RUN_TEST(test_column_combinator);
  RUN_TEST(test_is_selector);
  RUN_TEST(test_where_selector);
  RUN_TEST(test_not_selector);
  RUN_TEST(test_has_selector);
  RUN_TEST(test_empty_selector);
  RUN_TEST(test_root_selector);
  RUN_TEST(test_nth_child_selector);
  RUN_TEST(test_first_last_child);
  RUN_TEST(test_nth_of_type);
  RUN_TEST(test_linguistic_doc_selectors);
  RUN_TEST(test_user_action_and_time_selectors);
  RUN_TEST(test_input_form_selectors);
  RUN_TEST(test_dom_and_shadow_selectors);
  RUN_TEST(test_media_selectors);
  RUN_TEST(test_pseudo_elements_selectors);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(css_selectors_suite);
  GREATEST_MAIN_END();
}
