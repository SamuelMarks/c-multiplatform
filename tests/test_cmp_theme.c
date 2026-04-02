#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_cupertino.h"
#include "themes/cmp_fluent2.h"
#include "themes/cmp_material3.h"
#include "themes/cmp_theme_core.h"
#include "themes/cmp_unstyled.h"
/* clang-format on */

#ifndef CMP_THEME_MODE_SINGLE_STATIC
TEST test_fuzz_dispatcher(void) {
  cmp_ui_node_t *node = NULL;
  int i;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));

  /* Rapidly fuzz the state and ensure resolve never traps or races with bad
   * state logic */
  for (i = 0; i < 1000; i++) {
    const cmp_theme_vtable_t *vt;
    node->design_language_override =
        (uint8_t)(rand() % 6); /* Include invalid high overrides > 4 */
    vt = cmp_resolve_vtable(node);
    ASSERT_NEQ(NULL, (void *)vt);
  }

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_theme_resolve(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *child = NULL;
  const cmp_theme_vtable_t *vt;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&child));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_add_child(node, child));

  /* Test NULL */
  vt = cmp_resolve_vtable(NULL);
  ASSERT_NEQ(NULL, (void *)vt); /* Unstyled */

  /* Test default inherit (falls back to unstyled) */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  /* Test local override */
  child->design_language_override = 1; /* Material 3 */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  child->design_language_override = 2; /* Fluent 2 */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  child->design_language_override = 3; /* Cupertino */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  child->design_language_override = 4; /* Unstyled */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  /* Test parent override inheritance */
  child->design_language_override = 0; /* Inherit */
  node->design_language_override = 1;  /* Parent Material 3 */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  node->design_language_override = 2; /* Parent Fluent 2 */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  node->design_language_override = 3; /* Parent Cupertino */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  node->design_language_override = 4; /* Parent Unstyled */
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  /* Default path through inherit */
  node->design_language_override = 0;
  vt = cmp_resolve_vtable(child);
  ASSERT_NEQ(NULL, (void *)vt);

  cmp_ui_node_destroy(node); /* Destroys child too */

  PASS();
}

TEST test_per_widget_override_scene(void) {
  cmp_ui_node_t *root = NULL;
  cmp_ui_node_t *btn_cuper = NULL;
  cmp_ui_node_t *btn_fluent = NULL;
  cmp_ui_node_t *btn_material = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&root));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_button_create(&btn_cuper, "Cupertino", -1));
  btn_cuper->design_language_override = 3;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_button_create(&btn_fluent, "Fluent 2", -1));
  btn_fluent->design_language_override = 2;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_button_create(&btn_material, "Material 3", -1));
  btn_material->design_language_override = 1;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_add_child(root, btn_cuper));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_add_child(root, btn_fluent));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_node_add_child(root, btn_material));

  /* Validate that runtime dispatch accurately provides 3 unique pointers for
   * the siblings */
  ASSERT_EQ(cmp_resolve_vtable(btn_cuper), cmp_ffi_get_cupertino_vtable());
  ASSERT_EQ(cmp_resolve_vtable(btn_fluent), cmp_ffi_get_fluent2_vtable());
  ASSERT_EQ(cmp_resolve_vtable(btn_material), cmp_ffi_get_material3_vtable());

  cmp_ui_node_destroy(root);
  PASS();
}
TEST test_global_enum_switch(void) {
  cmp_window_t *window = NULL;
  cmp_theme_t *theme_material = NULL;
  cmp_theme_t *theme_fluent = NULL;
  cmp_window_config_t config;

  memset(&config, 0, sizeof(cmp_window_config_t));
  config.width = 100;
  config.height = 100;
  config.title = "Test";

  /* Mock window and themes */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_create(&config, &window));
  ASSERT_EQ(CMP_SUCCESS, cmp_theme_create(&theme_material));
  ASSERT_EQ(CMP_SUCCESS, cmp_theme_create(&theme_fluent));

  /* Standard logic mapping for internal engine state representation */
  /* For this test, we validate the API returns success and updates pointer
   * references */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_set_theme(window, theme_material));
  /* In a real scenario, this invalidates the UI tree and triggers a full redraw
   * without leaking */

  ASSERT_EQ(CMP_SUCCESS, cmp_window_set_theme(window, theme_fluent));

  /* Clean up */
  ASSERT_EQ(CMP_SUCCESS, cmp_theme_destroy(theme_material));
  ASSERT_EQ(CMP_SUCCESS, cmp_theme_destroy(theme_fluent));
  cmp_window_destroy(window);

  PASS();
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */

SUITE(theme_suite) {
#ifndef CMP_THEME_MODE_SINGLE_STATIC
  RUN_TEST(test_fuzz_dispatcher);
  RUN_TEST(test_theme_resolve);
  RUN_TEST(test_per_widget_override_scene);
  RUN_TEST(test_global_enum_switch);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(theme_suite);
  GREATEST_MAIN_END();
}