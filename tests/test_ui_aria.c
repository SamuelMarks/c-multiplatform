/* clang-format off */
#include "greatest.h"
#include "ui_aria.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_ui_aria_role_from_string(void) {
  enum ui_aria_role role;
  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("button", &role));
  ASSERT_EQ(UI_ARIA_ROLE_BUTTON, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("checkbox", &role));
  ASSERT_EQ(UI_ARIA_ROLE_CHECKBOX, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("dialog", &role));
  ASSERT_EQ(UI_ARIA_ROLE_DIALOG, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("alert", &role));
  ASSERT_EQ(UI_ARIA_ROLE_ALERT, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("slider", &role));
  ASSERT_EQ(UI_ARIA_ROLE_SLIDER, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("progressbar", &role));
  ASSERT_EQ(UI_ARIA_ROLE_PROGRESSBAR, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("heading", &role));
  ASSERT_EQ(UI_ARIA_ROLE_HEADING, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("link", &role));
  ASSERT_EQ(UI_ARIA_ROLE_LINK, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("textbox", &role));
  ASSERT_EQ(UI_ARIA_ROLE_TEXTBOX, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("status", &role));
  ASSERT_EQ(UI_ARIA_ROLE_STATUS, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("separator", &role));
  ASSERT_EQ(UI_ARIA_ROLE_SEPARATOR, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("list", &role));
  ASSERT_EQ(UI_ARIA_ROLE_LIST, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("listitem", &role));
  ASSERT_EQ(UI_ARIA_ROLE_LISTITEM, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("radiogroup", &role));
  ASSERT_EQ(UI_ARIA_ROLE_RADIOGROUP, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("tablist", &role));
  ASSERT_EQ(UI_ARIA_ROLE_TABLIST, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("navigation", &role));
  ASSERT_EQ(UI_ARIA_ROLE_NAVIGATION, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("group", &role));
  ASSERT_EQ(UI_ARIA_ROLE_GROUP, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string("unknown", &role));
  ASSERT_EQ(UI_ARIA_ROLE_NONE, role);

  ASSERT_EQ(UI_ERROR_NONE, ui_aria_role_from_string(NULL, &role));
  ASSERT_EQ(UI_ARIA_ROLE_NONE, role);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_aria_role_from_string("button", NULL));

  PASS();
}

TEST test_ui_aria_state_parse_empty(void) {
  struct ui_dom_node *node = NULL;
  struct ui_aria_state state;
  ui_error_t err;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");

  /* Test NULL out_state */
  err = ui_aria_state_parse(node, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, err);

  /* Test NULL node or non-element */
  err = ui_aria_state_parse(NULL, &state);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_EQ(UI_ARIA_ROLE_NONE, state.role);

  /* normal empty node */
  err = ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_EQ(UI_ARIA_ROLE_NONE, state.role);
  ASSERT_EQ(0, state.is_hidden);
  ASSERT_EQ(0, state.is_disabled);
  ASSERT_EQ(-1, state.is_expanded);
  ASSERT_EQ(-1, state.is_checked);
  ASSERT_EQ(NULL, state.label);
  ASSERT_EQ(NULL, state.description);

  /* wrong type */
  node->type = UI_DOM_NODE_TYPE_TEXT;
  err = ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ERROR_NONE, err);
  node->type = UI_DOM_NODE_TYPE_ELEMENT;

  ui_aria_state_cleanup(&state);
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* NULL cleanup test */
  ui_aria_state_cleanup(NULL);
  PASS();
}

TEST test_ui_aria_state_parse_implicit(void) {
  struct ui_dom_node *node;
  struct ui_aria_state state;
  const char *headings[] = {"h1", "h2", "h3", "h4", "h5", "h6"};
  int i;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  ui_dom_node_set_tag_name(node, "button");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_BUTTON, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "a");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_LINK, state.role);
  ui_aria_state_cleanup(&state);

  for (i = 0; i < 6; i++) {
    ui_dom_node_set_tag_name(node, headings[i]);
    ui_aria_state_parse(node, &state);
    ASSERT_EQ(UI_ARIA_ROLE_HEADING, state.role);
    ui_aria_state_cleanup(&state);
  }

  ui_dom_node_set_tag_name(node, "input");
  ui_dom_node_remove_attribute(node, "type");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_TEXTBOX, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_attribute(node, "type", "");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_TEXTBOX, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "ul");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_LIST, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "ol");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_LIST, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "li");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_LISTITEM, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "nav");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_NAVIGATION, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "input");
  ui_dom_node_set_attribute(node, "type", "checkbox");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_CHECKBOX, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_attribute(node, "type", "range");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_SLIDER, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "input");
  ui_dom_node_set_attribute(node, "type", "text");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_TEXTBOX, state.role);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "div");
  ui_dom_node_set_attribute(node, "role", "invalid-role");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_NONE, state.role);
  ui_aria_state_cleanup(&state);

  /* implicit checked */
  ui_dom_node_set_attribute(node, "checked", "");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(1, state.is_checked);
  ui_aria_state_cleanup(&state);

  /* implicit disabled */
  ui_dom_node_set_attribute(node, "disabled", "");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(1, state.is_disabled);
  ui_aria_state_cleanup(&state);

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  PASS();
}

TEST test_ui_aria_state_parse_explicit(void) {
  struct ui_dom_node *node = NULL;
  struct ui_aria_state state;
  ui_error_t err;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");

  ui_dom_node_set_attribute(node, "role", "slider");
  ui_dom_node_set_attribute(node, "aria-hidden", "true");
  ui_dom_node_set_attribute(node, "aria-disabled", "true");
  ui_dom_node_set_attribute(node, "aria-expanded", "false");
  ui_dom_node_set_attribute(node, "aria-checked", "mixed");
  ui_dom_node_set_attribute(node, "aria-label", "Volume");
  ui_dom_node_set_attribute(node, "aria-description", "Controls volume level");

  ui_aria_state_parse(node, &state);

  ASSERT_EQ(UI_ARIA_ROLE_SLIDER, state.role);
  ASSERT_EQ(1, state.is_hidden);
  ASSERT_EQ(1, state.is_disabled);
  ASSERT_EQ(0, state.is_expanded);
  ASSERT_EQ(2, state.is_checked);
  ASSERT(state.label != NULL);
  ASSERT_STR_EQ("Volume", state.label);
  ASSERT(state.description != NULL);
  ASSERT_STR_EQ("Controls volume level", state.description);

  ui_aria_state_cleanup(&state);

  /* Test aria-expanded true, aria-checked true */
  ui_dom_node_set_attribute(node, "aria-expanded", "true");
  ui_dom_node_set_attribute(node, "aria-checked", "true");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(1, state.is_expanded);
  ASSERT_EQ(1, state.is_checked);
  ui_aria_state_cleanup(&state);

  /* Test aria-checked false */
  ui_dom_node_set_attribute(node, "aria-checked", "false");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(0, state.is_checked);
  ui_aria_state_cleanup(&state);

  /* Test empty aria-label */
  ui_dom_node_set_attribute(node, "aria-label", "");
  ui_aria_state_parse(node, &state);
  ASSERT((state.label == NULL) || (strcmp(state.label, "") == 0));
  ui_aria_state_cleanup(&state);

  /* Test OOM on aria-label */
  ui_dom_node_set_attribute(node, "aria-label", "label");
  g_malloc_fail_countdown = 0;
  err = ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, err);
  g_malloc_fail_countdown = -1;

  /* Test OOM on aria-description */
  ui_dom_node_set_attribute(node, "aria-description", "desc");
  g_malloc_fail_countdown = 1; /* label succeeds, desc fails */
  err = ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, err);
  ASSERT_EQ(NULL, state.label); /* Should be cleaned up by the error handler */
  g_malloc_fail_countdown = -1;

  /* False state test conditions */
  ui_dom_node_set_attribute(node, "aria-disabled", "false");
  ui_dom_node_set_attribute(node, "aria-hidden", "false");
  ui_dom_node_set_attribute(node, "aria-expanded", "false");
  ui_dom_node_set_attribute(node, "aria-checked", "false");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(0, state.is_disabled);
  ASSERT_EQ(0, state.is_hidden);
  ASSERT_EQ(0, state.is_expanded);
  ASSERT_EQ(0, state.is_checked);
  ui_aria_state_cleanup(&state);

  /* Fallback attributes branch conditions */
  ui_dom_node_remove_attribute(node, "aria-disabled");
  ui_dom_node_remove_attribute(node, "aria-hidden");
  ui_dom_node_remove_attribute(node, "aria-expanded");
  ui_dom_node_remove_attribute(node, "aria-checked");

  ui_dom_node_set_attribute(node, "disabled", "true");
  ui_dom_node_set_attribute(node, "checked", "true");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(1, state.is_disabled);
  ASSERT_EQ(1, state.is_checked);
  ui_aria_state_cleanup(&state);

  /* Fallback false branches */
  ui_dom_node_set_attribute(node, "disabled", "false");
  ui_dom_node_set_attribute(node, "checked", "false");
  ui_aria_state_parse(node, &state);
  /* disabled native is existence, doesn't matter false */
  ASSERT_EQ(1, state.is_disabled);
  ASSERT_EQ(1, state.is_checked);
  ui_aria_state_cleanup(&state);

  /* Other random explicit values */
  ui_dom_node_remove_attribute(node, "disabled");
  ui_dom_node_set_attribute(node, "aria-hidden", "random_string");
  ui_dom_node_set_attribute(node, "aria-disabled", "random_string");
  ui_dom_node_set_attribute(node, "aria-expanded", "random_string");
  ui_dom_node_set_attribute(node, "aria-checked", "random_string");
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(0, state.is_hidden);
  ASSERT_EQ(0, state.is_disabled);
  ASSERT_EQ(-1, state.is_expanded);
  ASSERT_EQ(-1, state.is_checked);
  ui_aria_state_cleanup(&state);

  /* Fallback missing tag_name test */
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  node->tag_name =
      NULL; /* forcefully remove string since constructor gives default */
  ui_aria_state_parse(node, &state);
  ASSERT_EQ(UI_ARIA_ROLE_NONE, state.role);
  ui_aria_state_cleanup(&state);
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  PASS();
}

SUITE(ui_aria_suite) {
  RUN_TEST(test_ui_aria_role_from_string);
  RUN_TEST(test_ui_aria_state_parse_empty);
  RUN_TEST(test_ui_aria_state_parse_implicit);
  RUN_TEST(test_ui_aria_state_parse_explicit);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_aria_suite);

#ifdef UI_TEST_MOCK_ALLOC
  extern ui_error_t run_aria_coverage(void);
  run_aria_coverage();
#endif
  GREATEST_MAIN_END();
}
