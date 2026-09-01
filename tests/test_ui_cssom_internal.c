/* clang-format off */
#include "../src/ui_cssom.c"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

static void test_cascade_internal(void) {
  struct ui_css_computed_style *style;
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;

  /* Insert a dummy property to be the current one */
  (void)append_computed_declaration(style, "color", "red", 0, 0, 1, 2, 3, 10);

  /* Now test all the cascade branches against it! */
  /* new has higher important */
  (void)append_computed_declaration(style, "color", "red2", 1, 0, 1, 2, 3, 10);

  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 1, 0, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 1, 2, 3, 10);

  /* layer order */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 1, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 2, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 2, 3, 10);

  /* layer order important inverted */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 1, 1, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 1, 2, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red3", 1, 0, 1, 2, 3, 10);

  /* spec a */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 0, 2, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 3, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 2, 3, 10);

  /* spec b */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 0, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 1, 3, 3, 10);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 3, 10);

  /* spec c */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 0, 1, 1, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 1, 1, 4, 10);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 2, 10);

  /* source order */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 0, 1, 1, 1, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 1, 1, 1, 11);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 1, 9);

  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

int main(void) {
  test_cascade_internal();
  return 0;
}
