/**
 * @file test_material3_state_delegation.c
 * @brief Tests verifying zero duplicate state and direct base handle
 * delegation.
 */

/* clang-format off */
#include "greatest.h"
#include "material3/md3_button.h"
#include "material3/md3_card.h"
#include "material3/md3_checkbox.h"
#include "material3/md3_divider.h"
#include "material3/md3_fab.h"
#include "material3/md3_list.h"
#include "material3/md3_navigation.h"
#include "material3/md3_progress.h"
#include "material3/md3_slider.h"
#include "material3/md3_switch.h"
#include "ui_button_base.h"
#include "ui_card_base.h"
#include "ui_checkbox_base.h"
#include "ui_error.h"
#include <string.h>
/* clang-format on */

TEST test_md3_state_delegation_button(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_button *btn = NULL;
  struct ui_button_base *base = NULL;
  ui_error_t rc;

  rc = md3_button_create(dummy_engine, MD3_BUTTON_FILLED, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(btn != NULL);

  rc = md3_button_get_base(btn, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_state_delegation_checkbox(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_checkbox *cb = NULL;
  int checked = 0;
  ui_error_t rc;

  rc = md3_checkbox_create(dummy_engine, &cb, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cb != NULL);

  rc = md3_checkbox_set_checked(cb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_checkbox_get_checked(cb, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, checked);

  rc = md3_checkbox_destroy(cb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_state_delegation_card(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_card *card = NULL;
  ui_error_t rc;

  rc = md3_card_create(dummy_engine, MD3_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(card != NULL);
  ASSERT(card->base != NULL);

  rc = md3_card_set_title(card, "Card Title");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_card_destroy(card);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_state_delegation_list(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_list *list = NULL;
  struct md3_list_item *item = NULL;
  ui_error_t rc;

  rc = md3_list_create(dummy_engine, &list);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(list != NULL);
  ASSERT(list->base != NULL);

  rc = md3_list_item_create(dummy_engine, MD3_LIST_ITEM_ONE_LINE, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(item != NULL);
  ASSERT(item->base != NULL);

  rc = md3_list_append_item(list, item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_list_item_destroy(item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_destroy(list);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_state_delegation_switch(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_switch *sw = NULL;
  int checked = 0;
  ui_error_t rc;

  rc = md3_switch_create(dummy_engine, &sw, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sw != NULL);

  rc = md3_switch_set_checked(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_switch_get_checked(sw, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, checked);

  rc = md3_switch_destroy(sw);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_state_delegation_slider(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_slider *slider = NULL;
  float val = 0.0f;
  ui_error_t rc;

  rc = md3_slider_create(dummy_engine, MD3_SLIDER_CONTINUOUS, 0.0f, 100.0f,
                         &slider, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(slider != NULL);

  rc = md3_slider_set_value(slider, 42.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_slider_get_value(slider, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(42.0f, val);

  rc = md3_slider_destroy(slider);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

SUITE(md3_state_delegation_suite) {
  RUN_TEST(test_md3_state_delegation_button);
  RUN_TEST(test_md3_state_delegation_checkbox);
  RUN_TEST(test_md3_state_delegation_card);
  RUN_TEST(test_md3_state_delegation_list);
  RUN_TEST(test_md3_state_delegation_switch);
  RUN_TEST(test_md3_state_delegation_slider);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_state_delegation_suite);
  GREATEST_MAIN_END();
}
