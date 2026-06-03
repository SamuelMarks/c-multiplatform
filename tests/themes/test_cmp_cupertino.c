#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_cupertino.h"
/* clang-format on */

TEST test_cupertino_measure_button(void) {
  cmp_ui_node_t *node = NULL;
  float w = 0.0f, h = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_button_create(&node, "OK", -1));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_measure_button(node, &w, &h));
  ASSERT(w > 0.0f);
  ASSERT(h == 44.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_measure_button(NULL, &w, &h));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_measure_text_input(void) {
  cmp_ui_node_t *node = NULL;
  float w = 0.0f, h = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_measure_text_input(node, &w, &h));
  ASSERT(w == 200.0f);
  ASSERT(h == 44.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cupertino_measure_text_input(NULL, &w, &h));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_measure_slider(void) {
  cmp_ui_node_t *node = NULL;
  float w = 0.0f, h = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_measure_slider(node, &w, &h));
  ASSERT(w == 150.0f);
  ASSERT(h == 32.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_measure_slider(NULL, &w, &h));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_measure_toggle(void) {
  cmp_ui_node_t *node = NULL;
  float w = 0.0f, h = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_measure_toggle(node, &w, &h));
  ASSERT(w == 51.0f);
  ASSERT(h == 31.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_measure_toggle(NULL, &w, &h));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_measure_checkbox(void) {
  cmp_ui_node_t *node = NULL;
  float w = 0.0f, h = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_measure_checkbox(node, &w, &h));
  ASSERT(w == 20.0f);
  ASSERT(h == 20.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cupertino_measure_checkbox(NULL, &w, &h));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_measure_radio(void) {
  cmp_ui_node_t *node = NULL;
  float w = 0.0f, h = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_measure_radio(node, &w, &h));
  ASSERT(w == 20.0f);
  ASSERT(h == 20.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_measure_radio(NULL, &w, &h));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_measure_progress(void) {
  cmp_ui_node_t *node = NULL;
  float w = 0.0f, h = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_measure_progress(node, &w, &h));
  ASSERT(w == 150.0f);
  ASSERT(h == 4.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cupertino_measure_progress(NULL, &w, &h));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_measure_dropdown(void) {
  cmp_ui_node_t *node = NULL;
  float w = 0.0f, h = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_measure_dropdown(node, &w, &h));
  ASSERT(w == 120.0f);
  ASSERT(h == 32.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cupertino_measure_dropdown(NULL, &w, &h));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_layouts(void) {
  cmp_ui_node_t *node = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_layout_nav_bar(node));
  ASSERT(node->layout->direction == CMP_FLEX_ROW);
  ASSERT(node->layout->min_height == 44.0f);

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_layout_tab_bar(node));
  ASSERT(node->layout->min_height == 49.0f);

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_layout_dialog_content(node));
  ASSERT(node->layout->direction == CMP_FLEX_COLUMN);
  ASSERT(node->layout->min_width == 270.0f);

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_layout_sidebar(node));
  ASSERT(node->layout->min_width == 200.0f);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_layout_nav_bar(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_layout_tab_bar(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_layout_dialog_content(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_layout_sidebar(NULL));

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_draws(void) {
  cmp_ui_node_t *node = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_button(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_text_input(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_slider(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_toggle(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_checkbox(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_radio(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_progress(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_card(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_tooltip(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_menu(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_draw_focus_ring(node));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_button(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_text_input(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_slider(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_toggle(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_checkbox(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_radio(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_progress(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_card(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_tooltip(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_menu(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cupertino_draw_focus_ring(NULL));

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_configs(void) {
  cmp_ui_node_t *node = NULL;
  float a, b, c;
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&node));

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_get_spring_config(node, &a, &b, &c));
  ASSERT(a == 1.0f);
  ASSERT(b == 100.0f);
  ASSERT(c == 10.0f);

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_get_ripple_config(node, &a, &b));
  ASSERT(a == 0.5f);
  ASSERT(b == 0.5f);

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_get_state_layer_opacity(node, 0, &a));
  ASSERT(a == 1.0f);

  ASSERT_EQ(CMP_SUCCESS, cmp_cupertino_get_transition_duration(node, &a));
  ASSERT(a == 250.0f);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cupertino_get_spring_config(NULL, &a, &b, &c));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cupertino_get_ripple_config(NULL, &a, &b));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cupertino_get_state_layer_opacity(NULL, 0, &a));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cupertino_get_transition_duration(NULL, &a));
  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_cupertino_vtable(void) {
  const cmp_theme_vtable_t *vt = NULL;
  cmp_theme_get_cupertino_vtable(&vt);
  ASSERT(vt != NULL);
  ASSERT(vt->measure_button != NULL);
  PASS();
}

SUITE(cmp_cupertino_suite) {
  RUN_TEST(test_cupertino_measure_button);
  RUN_TEST(test_cupertino_measure_text_input);
  RUN_TEST(test_cupertino_measure_slider);
  RUN_TEST(test_cupertino_measure_toggle);
  RUN_TEST(test_cupertino_measure_checkbox);
  RUN_TEST(test_cupertino_measure_radio);
  RUN_TEST(test_cupertino_measure_progress);
  RUN_TEST(test_cupertino_measure_dropdown);
  RUN_TEST(test_cupertino_layouts);
  RUN_TEST(test_cupertino_draws);
  RUN_TEST(test_cupertino_configs);
  RUN_TEST(test_cupertino_vtable);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_cupertino_suite);
  GREATEST_MAIN_END();
}
