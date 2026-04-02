/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_ui_box(void) {
  cmp_ui_node_t *box = NULL;
  int res;

  res = cmp_ui_box_create(&box);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(box != NULL);
  ASSERT_EQ_FMT(1, box->type, "%d");
  ASSERT(box->layout != NULL);

  res = cmp_ui_node_destroy(box);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_ui_text(void) {
  cmp_ui_node_t *text = NULL;
  int res;

  res = cmp_ui_text_create(&text, "Hello World", -1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(text != NULL);
  ASSERT_EQ_FMT(2, text->type, "%d");
  ASSERT(text->properties != NULL);
  ASSERT_STR_EQ("Hello World", (char *)text->properties);

  res = cmp_ui_node_destroy(text);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_ui_hierarchy(void) {
  cmp_ui_node_t *box = NULL, *text1 = NULL, *text2 = NULL;
  int res;

  cmp_ui_box_create(&box);
  cmp_ui_text_create(&text1, "Item 1", -1);
  cmp_ui_text_create(&text2, "Item 2", -1);

  res = cmp_ui_node_add_child(box, text1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_ui_node_add_child(box, text2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  ASSERT_EQ_FMT((size_t)2, box->child_count, "%zd");
  ASSERT(text1->parent == box);
  ASSERT(text2->parent == box);

  /* Verify layout tree mirroring */
  ASSERT_EQ_FMT((size_t)2, box->layout->child_count, "%zd");
  ASSERT(text1->layout->parent == box->layout);

  /* Recursively destroy everything */
  res = cmp_ui_node_destroy(box);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_ui_advanced_primitives(void) {
  cmp_ui_node_t *btn = NULL, *input = NULL, *chk = NULL, *rad = NULL,
                *img = NULL, *slider = NULL;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_button_create(&btn, "Click Me", -1), "%d");
  ASSERT_EQ_FMT(3, btn->type, "%d");
  ASSERT_STR_EQ("Click Me", (char *)btn->properties);
  cmp_ui_node_destroy(btn);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_text_input_create(&input), "%d");
  ASSERT_EQ_FMT(4, input->type, "%d");
  cmp_ui_node_destroy(input);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_checkbox_create(&chk, "Option"), "%d");
  ASSERT_EQ_FMT(5, chk->type, "%d");
  ASSERT_STR_EQ("Option", (char *)chk->properties);
  cmp_ui_node_destroy(chk);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_radio_create(&rad, 42), "%d");
  ASSERT_EQ_FMT(6, rad->type, "%d");
  ASSERT_EQ_FMT(42, *(int *)rad->properties, "%d");
  cmp_ui_node_destroy(rad);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_image_view_create(&img, "test.png"), "%d");
  ASSERT_EQ_FMT(7, img->type, "%d");
  ASSERT_STR_EQ("test.png", (char *)img->properties);
  cmp_ui_node_destroy(img);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_slider_create(&slider, 10.0f, 50.0f), "%d");
  ASSERT_EQ_FMT(8, slider->type, "%d");
  ASSERT_EQ_FMT(10.0f, ((float *)slider->properties)[0], "%f");
  ASSERT_EQ_FMT(50.0f, ((float *)slider->properties)[1], "%f");
  cmp_ui_node_destroy(slider);

  PASS();
}

TEST test_ui_phase17_components(void) {
  cmp_ui_node_t *lv = NULL, *gv = NULL, *drop = NULL, *mod = NULL, *cvs = NULL,
                *rt = NULL;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_list_view_create(&lv), "%d");
  ASSERT_EQ_FMT(9, lv->type, "%d");
  cmp_ui_node_destroy(lv);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_grid_view_create(&gv, 3), "%d");
  ASSERT_EQ_FMT(10, gv->type, "%d");
  ASSERT_EQ_FMT(3, *(int *)gv->properties, "%d");
  cmp_ui_node_destroy(gv);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dropdown_create(&drop), "%d");
  ASSERT_EQ_FMT(11, drop->type, "%d");
  cmp_ui_node_destroy(drop);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_modal_create(&mod), "%d");
  ASSERT_EQ_FMT(12, mod->type, "%d");
  cmp_ui_node_destroy(mod);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_canvas_create(&cvs), "%d");
  ASSERT_EQ_FMT(13, cvs->type, "%d");
  cmp_ui_node_destroy(cvs);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_rich_text_create(&rt), "%d");
  ASSERT_EQ_FMT(14, rt->type, "%d");
  cmp_ui_node_destroy(rt);

  PASS();
}

SUITE(ui_suite) {
  RUN_TEST(test_ui_box);
  RUN_TEST(test_ui_text);
  RUN_TEST(test_ui_hierarchy);
  RUN_TEST(test_ui_advanced_primitives);
  RUN_TEST(test_ui_phase17_components);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_suite);
  GREATEST_MAIN_END();
}
