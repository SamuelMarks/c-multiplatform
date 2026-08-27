/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_css_gcpm.h"
#include "ui_test_mock_mem.h"
#include <stdlib.h>
#include "ui_cssom.h"
/* clang-format on */

/* Since ui_cssom provides ui_css_computed_style_get_property, we just need to
 * populate the struct ourselves since there is no
 * ui_css_computed_style_create/set */
static void add_prop(struct ui_css_computed_style *style, const char *name,
                     const char *val) {
  struct ui_css_computed_property *p =
      (struct ui_css_computed_property *)C_MULTIPLATFORM_MALLOC(sizeof(*p));
  memset(p, 0, sizeof(*p));
  p->property_name = (char *)C_MULTIPLATFORM_MALLOC(strlen(name) + 1);
  strcpy(p->property_name, name);
  p->property_value = (char *)C_MULTIPLATFORM_MALLOC(strlen(val) + 1);
  strcpy(p->property_value, val);
  p->next = style->properties;
  style->properties = p;
}

static void free_props(struct ui_css_computed_style *style) {
  struct ui_css_computed_property *p = style->properties;
  while (p) {
    struct ui_css_computed_property *next = p->next;
    C_MULTIPLATFORM_FREE(p->property_name);
    C_MULTIPLATFORM_FREE(p->property_value);
    C_MULTIPLATFORM_FREE(p);
    p = next;
  }
}

static int test_gcpm_parse(void) {
  struct ui_css_computed_style style;
  struct ui_css_gcpm_properties props;

  memset(&style, 0, sizeof(style));

  if (ui_css_gcpm_parse(NULL, &props) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_gcpm_parse(&style, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* empty style */
  if (ui_css_gcpm_parse(&style, &props) != UI_ERROR_NONE)
    return 1;
  if (props.string_set)
    return 1;
  if (props.bookmark_label)
    return 1;
  if (props.bookmark_level != 0)
    return 1;
  if (props.bookmark_state != 1)
    return 1;
  ui_css_gcpm_properties_cleanup(&props);

  /* none */
  add_prop(&style, "string-set", "none");
  add_prop(&style, "bookmark-label", "none");
  add_prop(&style, "bookmark-level", "none");
  add_prop(&style, "bookmark-state", "closed");

  if (ui_css_gcpm_parse(&style, &props) != UI_ERROR_NONE)
    return 1;
  if (props.string_set)
    return 1;
  if (props.bookmark_label)
    return 1;
  if (props.bookmark_level != 0)
    return 1;
  if (props.bookmark_state != 0)
    return 1;
  ui_css_gcpm_properties_cleanup(&props);
  free_props(&style);
  style.properties = NULL;

  /* valid empty string testing edge case where get_property succeeds but value
   * is empty */
  style.properties = NULL;
  add_prop(&style, "string-set", "");
  add_prop(&style, "bookmark-label", "");
  add_prop(&style, "bookmark-level", "");
  add_prop(&style, "bookmark-state", "");

  if (ui_css_gcpm_parse(&style, &props) != UI_ERROR_NONE)
    return 1;
  if (!props.string_set || strcmp(props.string_set, "") != 0)
    return 1;
  if (!props.bookmark_label || strcmp(props.bookmark_label, "") != 0)
    return 1;
  if (props.bookmark_level != 0)
    return 1;
  if (props.bookmark_state != 1)
    return 1;
  ui_css_gcpm_properties_cleanup(&props);
  free_props(&style);
  style.properties = NULL;

  /* active */
  add_prop(&style, "string-set", "title content(text)");
  add_prop(&style, "bookmark-label", "content(text)");
  add_prop(&style, "bookmark-level", "2");
  add_prop(&style, "bookmark-state", "open");

  if (ui_css_gcpm_parse(&style, &props) != UI_ERROR_NONE)
    return 1;
  if (strcmp(props.string_set, "title content(text)") != 0)
    return 1;
  if (strcmp(props.bookmark_label, "content(text)") != 0)
    return 1;
  if (props.bookmark_level != 2)
    return 1;
  if (props.bookmark_state != 1)
    return 1;
  ui_css_gcpm_properties_cleanup(&props);
  free_props(&style);

  return 0;
}

static int test_gcpm_oom(void) {
  struct ui_css_computed_style style;
  struct ui_css_gcpm_properties props;

  memset(&style, 0, sizeof(style));
  add_prop(&style, "string-set", "val");
  add_prop(&style, "bookmark-label", "val");

  g_malloc_fail_countdown = 0;
  if (ui_css_gcpm_parse(&style, &props) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  if (ui_css_gcpm_parse(&style, &props) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  free_props(&style);
  return 0;
}

static int test_gcpm_cleanup(void) {
  ui_css_gcpm_properties_cleanup(NULL);
  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_gcpm_parse();
  failures += test_gcpm_oom();
  failures += test_gcpm_cleanup();

  if (failures == 0) {
    printf("test_ui_css_gcpm passed\n");
  } else {
    printf("test_ui_css_gcpm failed with %d errors\n", failures);
  }

  return failures;
}
