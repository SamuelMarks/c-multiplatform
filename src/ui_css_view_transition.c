/* clang-format off */
#include "ui_css_view_transition.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "strtok_posix.h"
/* clang-format on */

#if defined(_MSC_VER)
#define UI_STRTOK(str, delim, ctx) strtok_s((str), (delim), (ctx))
#else
#define UI_STRTOK(str, delim, ctx) strtok_r((str), (delim), (ctx))
#endif

static ui_error_t skip_whitespace(const char **p_str) {
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_css_parse_view_transition_name */
ui_error_t ui_css_parse_view_transition_name(
    const char *str, struct ui_css_view_transition_name *out_name) {
  if (!str || !out_name)
    return UI_ERROR_INVALID_ARGUMENT;

  { (void)skip_whitespace(&str); }

  out_name->is_none = 0;
  out_name->name[0] = '\0';

  if (strcmp(str, "none") == 0) {
    out_name->is_none = 1;
    return UI_ERROR_NONE;
  }

  /* Valid custom ident */
  {
    UI_STRNCPY(out_name->name, sizeof(out_name->name), str,
               sizeof(out_name->name) - 1);
  }

  return UI_ERROR_NONE;
}
/** \brief ui_css_view_transition_class_destroy */
ui_error_t ui_css_view_transition_class_destroy(
    struct ui_css_view_transition_class *vt_class) {
  struct ui_css_view_transition_class_name *current;
  struct ui_css_view_transition_class_name *next;

  if (!vt_class)
    return UI_ERROR_INVALID_ARGUMENT;

  current = vt_class->names;
  while (current) {
    next = current->next;
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }
  vt_class->names = NULL;

  return UI_ERROR_NONE;
}

/** \brief ui_css_parse_view_transition_class */
ui_error_t ui_css_parse_view_transition_class(
    const char *str, struct ui_css_view_transition_class *out_class) {
  char token_buf[1024];
  char *token;
  char *next_token = NULL;
  struct ui_css_view_transition_class_name *head = NULL;
  struct ui_css_view_transition_class_name *tail = NULL;

  if (!str || !out_class)
    return UI_ERROR_INVALID_ARGUMENT;

  out_class->is_none = 0;
  out_class->names = NULL;

  { (void)skip_whitespace(&str); }

  if (strcmp(str, "none") == 0) {
    out_class->is_none = 1;
    return UI_ERROR_NONE;
  }

  UI_STRNCPY(token_buf, sizeof(token_buf), str, sizeof(token_buf) - 1);
  token_buf[sizeof(token_buf) - 1] = '\0';

  token = UI_STRTOK(token_buf, " ", &next_token);
  while (token) {
    struct ui_css_view_transition_class_name *node =
        (struct ui_css_view_transition_class_name *)C_MULTIPLATFORM_MALLOC(
            sizeof(struct ui_css_view_transition_class_name));

    if (!node) {
      out_class->names = head;
      { (void)ui_css_view_transition_class_destroy(out_class); }
      return UI_ERROR_OUT_OF_MEMORY;
    }

    UI_STRNCPY(node->name, sizeof(node->name), token, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    node->next = NULL;

    if (tail) {
      tail->next = node;
    } else {
      head = node;
    }
    tail = node;

    token = UI_STRTOK(NULL, " ", &next_token);
  }

  out_class->names = head;
  return UI_ERROR_NONE;
}
