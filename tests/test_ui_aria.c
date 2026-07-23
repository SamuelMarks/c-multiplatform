/* clang-format off */
#include "ui_aria.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error test_ui_aria_role_from_string() {
  enum ui_aria_role role;
  assert(ui_aria_role_from_string("button", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_BUTTON);
  assert(ui_aria_role_from_string("checkbox", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_CHECKBOX);
  assert(ui_aria_role_from_string("dialog", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_DIALOG);
  assert(ui_aria_role_from_string("alert", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_ALERT);
  assert(ui_aria_role_from_string("slider", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_SLIDER);
  assert(ui_aria_role_from_string("progressbar", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_PROGRESSBAR);
  assert(ui_aria_role_from_string("heading", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_HEADING);
  assert(ui_aria_role_from_string("link", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_LINK);
  assert(ui_aria_role_from_string("textbox", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_TEXTBOX);
  assert(ui_aria_role_from_string("status", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_STATUS);
  assert(ui_aria_role_from_string("separator", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_SEPARATOR);
  assert(ui_aria_role_from_string("list", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_LIST);
  assert(ui_aria_role_from_string("listitem", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_LISTITEM);
  assert(ui_aria_role_from_string("radiogroup", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_RADIOGROUP);
  assert(ui_aria_role_from_string("tablist", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_TABLIST);
  assert(ui_aria_role_from_string("navigation", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_NAVIGATION);
  assert(ui_aria_role_from_string("group", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_GROUP);
  assert(ui_aria_role_from_string("unknown", &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_NONE);
  assert(ui_aria_role_from_string(NULL, &role) == UI_ERROR_NONE &&
         role == UI_ARIA_ROLE_NONE);
  assert(ui_aria_role_from_string("button", NULL) == UI_ERROR_INVALID_ARGUMENT);
  printf("test_ui_aria_role_from_string passed\n");
  return UI_ERROR_NONE;
}

static enum ui_error test_ui_aria_state_parse_empty() {
  struct ui_dom_node *node = NULL;
  struct ui_aria_state state;
  enum ui_error err;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");

  /* Test NULL out_state */
  err = ui_aria_state_parse(node, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  /* Test NULL node or non-element */
  err = ui_aria_state_parse(NULL, &state);
  assert(err == UI_ERROR_NONE);
  assert(state.role == UI_ARIA_ROLE_NONE);

  /* normal empty node */
  err = ui_aria_state_parse(node, &state);
  assert(err == UI_ERROR_NONE);
  assert(state.role == UI_ARIA_ROLE_NONE);
  assert(state.is_hidden == 0);
  assert(state.is_disabled == 0);
  assert(state.is_expanded == -1);
  assert(state.is_checked == -1);
  assert(state.label == NULL);
  assert(state.description == NULL);

  /* wrong type */
  node->type = UI_DOM_NODE_TYPE_TEXT;
  err = ui_aria_state_parse(node, &state);
  assert(err == UI_ERROR_NONE);
  node->type = UI_DOM_NODE_TYPE_ELEMENT;

  ui_aria_state_cleanup(&state);
  ui_dom_node_destroy(node);

  /* NULL cleanup test */
  ui_aria_state_cleanup(NULL);
  printf("test_ui_aria_state_parse_empty passed\n");
  return UI_ERROR_NONE;
}

static enum ui_error test_ui_aria_state_parse_implicit() {
  struct ui_dom_node *node;
  struct ui_aria_state state;
  const char *headings[] = {"h1", "h2", "h3", "h4", "h5", "h6"};
  int i;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  ui_dom_node_set_tag_name(node, "button");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_BUTTON);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "a");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_LINK);
  ui_aria_state_cleanup(&state);

  for (i = 0; i < 6; i++) {
    ui_dom_node_set_tag_name(node, headings[i]);
    ui_aria_state_parse(node, &state);
    assert(state.role == UI_ARIA_ROLE_HEADING);
    ui_aria_state_cleanup(&state);
  }

  ui_dom_node_set_tag_name(node, "input");
  ui_dom_node_remove_attribute(node, "type");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_TEXTBOX);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_attribute(node, "type", "");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_TEXTBOX);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "ul");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_LIST);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "ol");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_LIST);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "li");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_LISTITEM);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "nav");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_NAVIGATION);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "input");
  ui_dom_node_set_attribute(node, "type", "checkbox");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_CHECKBOX);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_attribute(node, "type", "range");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_SLIDER);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "input");
  ui_dom_node_set_attribute(node, "type", "text");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_TEXTBOX);
  ui_aria_state_cleanup(&state);

  ui_dom_node_set_tag_name(node, "div");
  ui_dom_node_set_attribute(node, "role", "invalid-role");
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_NONE);
  ui_aria_state_cleanup(&state);

  /* implicit checked */
  ui_dom_node_set_attribute(node, "checked", "");
  ui_aria_state_parse(node, &state);
  assert(state.is_checked == 1);
  ui_aria_state_cleanup(&state);

  /* implicit disabled */
  ui_dom_node_set_attribute(node, "disabled", "");
  ui_aria_state_parse(node, &state);
  assert(state.is_disabled == 1);
  ui_aria_state_cleanup(&state);

  ui_dom_node_destroy(node);
  printf("test_ui_aria_state_parse_implicit passed\n");
}

static enum ui_error test_ui_aria_state_parse_explicit() {
  struct ui_dom_node *node = NULL;
  struct ui_aria_state state;
  enum ui_error err;

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

  assert(state.role == UI_ARIA_ROLE_SLIDER);
  assert(state.is_hidden == 1);
  assert(state.is_disabled == 1);
  assert(state.is_expanded == 0);
  assert(state.is_checked == 2);
  assert(state.label != NULL && strcmp(state.label, "Volume") == 0);
  assert(state.description != NULL &&
         strcmp(state.description, "Controls volume level") == 0);

  ui_aria_state_cleanup(&state);

  /* Test aria-expanded true, aria-checked true */
  ui_dom_node_set_attribute(node, "aria-expanded", "true");
  ui_dom_node_set_attribute(node, "aria-checked", "true");
  ui_aria_state_parse(node, &state);
  assert(state.is_expanded == 1);
  assert(state.is_checked == 1);
  ui_aria_state_cleanup(&state);

  /* Test aria-checked false */
  ui_dom_node_set_attribute(node, "aria-checked", "false");
  ui_aria_state_parse(node, &state);
  assert(state.is_checked == 0);
  ui_aria_state_cleanup(&state);

  /* Test empty aria-label */
  ui_dom_node_set_attribute(node, "aria-label", "");
  ui_aria_state_parse(node, &state);
  assert(state.label == NULL || strcmp(state.label, "") == 0);
  ui_aria_state_cleanup(&state);

  /* Test OOM on aria-label */
  ui_dom_node_set_attribute(node, "aria-label", "label");
  g_malloc_fail_countdown = 0;
  err = ui_aria_state_parse(node, &state);
  assert(err == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  /* Test OOM on aria-description */
  ui_dom_node_set_attribute(node, "aria-description", "desc");
  g_malloc_fail_countdown = 1; /* label succeeds, desc fails */
  err = ui_aria_state_parse(node, &state);
  assert(err == UI_ERROR_OUT_OF_MEMORY);
  assert(state.label == NULL); /* Should be cleaned up by the error handler */
  g_malloc_fail_countdown = -1;

  /* False state test conditions */
  ui_dom_node_set_attribute(node, "aria-disabled", "false");
  ui_dom_node_set_attribute(node, "aria-hidden", "false");
  ui_dom_node_set_attribute(node, "aria-expanded", "false");
  ui_dom_node_set_attribute(node, "aria-checked", "false");
  ui_aria_state_parse(node, &state);
  assert(state.is_disabled == 0);
  assert(state.is_hidden == 0);
  assert(state.is_expanded == 0);
  assert(state.is_checked == 0);
  ui_aria_state_cleanup(&state);

  /* Fallback attributes branch conditions */
  ui_dom_node_remove_attribute(node, "aria-disabled");
  ui_dom_node_remove_attribute(node, "aria-hidden");
  ui_dom_node_remove_attribute(node, "aria-expanded");
  ui_dom_node_remove_attribute(node, "aria-checked");

  ui_dom_node_set_attribute(node, "disabled", "true");
  ui_dom_node_set_attribute(node, "checked", "true");
  ui_aria_state_parse(node, &state);
  assert(state.is_disabled == 1);
  assert(state.is_checked == 1);
  ui_aria_state_cleanup(&state);

  /* Fallback false branches */
  ui_dom_node_set_attribute(node, "disabled", "false");
  ui_dom_node_set_attribute(node, "checked", "false");
  ui_aria_state_parse(node, &state);
  /* disabled native is existence, doesn't matter false */
  assert(state.is_disabled == 1);
  assert(state.is_checked == 1);
  ui_aria_state_cleanup(&state);

  /* Other random explicit values */
  ui_dom_node_remove_attribute(node, "disabled");
  ui_dom_node_set_attribute(node, "aria-hidden", "random_string");
  ui_dom_node_set_attribute(node, "aria-disabled", "random_string");
  ui_dom_node_set_attribute(node, "aria-expanded", "random_string");
  ui_dom_node_set_attribute(node, "aria-checked", "random_string");
  ui_aria_state_parse(node, &state);
  assert(state.is_hidden == 0);
  assert(state.is_disabled == 0);
  assert(state.is_expanded == -1);
  assert(state.is_checked == -1);
  ui_aria_state_cleanup(&state);

  /* Fallback missing tag_name test */
  ui_dom_node_destroy(node);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  node->tag_name =
      NULL; /* forcefully remove string since constructor gives default */
  ui_aria_state_parse(node, &state);
  assert(state.role == UI_ARIA_ROLE_NONE);
  ui_aria_state_cleanup(&state);
  ui_dom_node_destroy(node);

  printf("test_ui_aria_state_parse_explicit passed\n");
  return UI_ERROR_NONE;
}

int main() {
  test_ui_aria_role_from_string();
  test_ui_aria_state_parse_empty();
  test_ui_aria_state_parse_implicit();
  test_ui_aria_state_parse_explicit();
  printf("All test_ui_aria passed\n");
  return 0;
}
