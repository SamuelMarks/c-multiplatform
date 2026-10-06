/**
 * @file test_material3_overlay_menus.c
 * @brief Unit tests for Material 3 Overlay Menus.
 */

/* clang-format off */
#include "material3/md3_overlay_menus.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
extern int g_md3_overlay_menus_mock_fail;
#endif

SUITE(md3_overlay_menus_suite);

static struct ui_engine *create_engine(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  if (ui_engine_create(&engine_cfg, &engine) != UI_ERROR_NONE) {
    return NULL;
  }
  return engine;
}

TEST test_md3_hover_card(void) {
  struct ui_engine *engine = create_engine();
  struct ui_dom_node node;
  struct md3_hover_card *hc = NULL;

  memset(&node, 0, sizeof(node));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_hover_card_create(NULL, &node, &hc));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_hover_card_create(engine, NULL, &hc));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_hover_card_create(engine, &node, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_hover_card_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_hover_card_show(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_hover_card_hide(NULL));

  ASSERT_EQ(UI_ERROR_NONE, md3_hover_card_create(engine, &node, &hc));
  ASSERT_EQ(UI_ERROR_NONE, md3_hover_card_show(hc));
  ASSERT_EQ(UI_ERROR_NONE, md3_hover_card_hide(hc));
  ASSERT_EQ(UI_ERROR_NONE, md3_hover_card_destroy(hc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_hover_card_create(engine, &node, &hc));
  g_malloc_fail_countdown = -1;

  g_md3_overlay_menus_mock_fail = 7;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_hover_card_create(engine, &node, &hc));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_hover_card_create(engine, &node, &hc));

  g_md3_overlay_menus_mock_fail = 9;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_hover_card_show(hc));
  g_md3_overlay_menus_mock_fail = 10;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_hover_card_hide(hc));
  g_md3_overlay_menus_mock_fail = 8;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_hover_card_destroy(hc));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_hover_card_destroy(hc));
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_popover(void) {
  struct ui_engine *engine = create_engine();
  struct ui_dom_node node;
  struct md3_popover *pop = NULL;
  int is_open = 0;

  memset(&node, 0, sizeof(node));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_popover_create(NULL, &node, &pop));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_popover_create(engine, NULL, &pop));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_popover_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_popover_open(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_popover_close(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_popover_is_open(NULL, &is_open));

  ASSERT_EQ(UI_ERROR_NONE, md3_popover_create(engine, &node, &pop));
  ASSERT_EQ(UI_ERROR_NONE, md3_popover_is_open(pop, &is_open));
  ASSERT_EQ(0, is_open);
  ASSERT_EQ(UI_ERROR_NONE, md3_popover_open(pop));
  ASSERT_EQ(UI_ERROR_NONE, md3_popover_is_open(pop, &is_open));
  ASSERT_EQ(1, is_open);
  ASSERT_EQ(UI_ERROR_NONE, md3_popover_close(pop));
  ASSERT_EQ(UI_ERROR_NONE, md3_popover_destroy(pop));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_popover_create(engine, &node, &pop));
  g_malloc_fail_countdown = -1;

  g_md3_overlay_menus_mock_fail = 11;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_popover_create(engine, &node, &pop));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_popover_create(engine, &node, &pop));

  g_md3_overlay_menus_mock_fail = 13;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_popover_close(pop));
  g_md3_overlay_menus_mock_fail = 12;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_popover_destroy(pop));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_popover_destroy(pop));
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_banner(void) {
  struct ui_engine *engine = create_engine();
  struct md3_banner *banner = NULL;
  int is_open = 0;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_create(NULL, &banner));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_set_text(NULL, "Text"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_banner_set_action(NULL, 0, "Action"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_banner_set_action(banner, 2, "Action"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_set_open(NULL, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_is_open(NULL, &is_open));

  ASSERT_EQ(UI_ERROR_NONE, md3_banner_create(engine, &banner));
  ASSERT_EQ(UI_ERROR_NONE, md3_banner_set_text(banner, "Hello"));
  ASSERT_EQ(UI_ERROR_NONE, md3_banner_set_action(banner, 0, "OK"));
  ASSERT_EQ(UI_ERROR_NONE, md3_banner_set_action(banner, 1, "Cancel"));
  ASSERT_EQ(UI_ERROR_NONE, md3_banner_set_open(banner, 0));
  ASSERT_EQ(UI_ERROR_NONE, md3_banner_is_open(banner, &is_open));
  ASSERT_EQ(0, is_open);
  ASSERT_EQ(UI_ERROR_NONE, md3_banner_destroy(banner));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_banner_create(engine, &banner));
  g_malloc_fail_countdown = -1;

  g_md3_overlay_menus_mock_fail = 14;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_banner_create(engine, &banner));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_banner_create(engine, &banner));

  g_md3_overlay_menus_mock_fail = 16;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_banner_set_text(banner, "Text"));
  g_md3_overlay_menus_mock_fail = 17;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_banner_set_open(banner, 1));
  g_md3_overlay_menus_mock_fail = 15;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_banner_destroy(banner));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_banner_destroy(banner));
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_inline_alert(void) {
  struct ui_engine *engine = create_engine();
  struct md3_inline_alert *alert = NULL;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_inline_alert_create(NULL, MD3_ALERT_SEVERITY_INFO, &alert));
  ASSERT_EQ(
      UI_ERROR_INVALID_ARGUMENT,
      md3_inline_alert_create(engine, (enum md3_alert_severity)99, &alert));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_inline_alert_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_inline_alert_set_title(NULL, "Title"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_inline_alert_set_message(NULL, "Msg"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_inline_alert_set_dismissible(NULL, 1));

  ASSERT_EQ(UI_ERROR_NONE,
            md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_INFO, &alert));
  ASSERT_EQ(UI_ERROR_NONE, md3_inline_alert_set_title(alert, "Title"));
  ASSERT_EQ(UI_ERROR_NONE, md3_inline_alert_set_message(alert, "Message"));
  ASSERT_EQ(UI_ERROR_NONE, md3_inline_alert_set_dismissible(alert, 1));
  ASSERT_EQ(UI_ERROR_NONE, md3_inline_alert_destroy(alert));

  ASSERT_EQ(UI_ERROR_NONE,
            md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_ERROR, &alert));
  ASSERT_EQ(UI_ERROR_NONE, md3_inline_alert_destroy(alert));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY,
            md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_INFO, &alert));
  g_malloc_fail_countdown = -1;

  g_md3_overlay_menus_mock_fail = 18;
  ASSERT_EQ(UI_ERROR_UNKNOWN,
            md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_INFO, &alert));
  g_md3_overlay_menus_mock_fail = 19;
  ASSERT_EQ(UI_ERROR_UNKNOWN,
            md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_INFO, &alert));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE,
            md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_INFO, &alert));

  g_md3_overlay_menus_mock_fail = 21;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_inline_alert_set_dismissible(alert, 1));
  g_md3_overlay_menus_mock_fail = 20;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_inline_alert_destroy(alert));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_inline_alert_destroy(alert));
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_menubar(void) {
  struct ui_engine *engine = create_engine();
  struct md3_menubar *mb = NULL;
  struct ui_component comp;

  memset(&comp, 0, sizeof(comp));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_menubar_create(NULL, &mb));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_menubar_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_menubar_append_item(NULL, &comp));

  ASSERT_EQ(UI_ERROR_NONE, md3_menubar_create(engine, &mb));
  ASSERT_EQ(UI_ERROR_NONE, md3_menubar_append_item(mb, &comp));
  md3_menubar_append_item(mb, &comp);
  ASSERT_EQ(UI_ERROR_NONE, md3_menubar_destroy(mb));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_menubar_create(engine, &mb));
  g_malloc_fail_countdown = -1;

  g_md3_overlay_menus_mock_fail = 22;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_menubar_create(engine, &mb));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_menubar_create(engine, &mb));

  memset(&comp, 0, sizeof(comp));
  g_md3_overlay_menus_mock_fail = 24;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_menubar_append_item(mb, &comp));

  memset(&comp, 0, sizeof(comp));
  g_md3_overlay_menus_mock_fail = 25;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_menubar_append_item(mb, &comp));
  g_md3_overlay_menus_mock_fail = 0;

  g_md3_overlay_menus_mock_fail = 23;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_menubar_destroy(mb));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_menubar_destroy(mb));
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_context_menu(void) {
  struct ui_engine *engine = create_engine();
  struct md3_context_menu *cm = NULL;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_context_menu_create(NULL, &cm));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_context_menu_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_context_menu_open_at(NULL, 0, 0, 100, 100));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_context_menu_open_at(cm, -1, 0, 100, 100));

  ASSERT_EQ(UI_ERROR_NONE, md3_context_menu_create(engine, &cm));
  ASSERT_EQ(UI_ERROR_NONE, md3_context_menu_open_at(cm, 10, 10, 800, 600));
  ASSERT_EQ(10, cm->current_x);
  ASSERT_EQ(10, cm->current_y);
  ASSERT_EQ(UI_ERROR_NONE, md3_context_menu_open_at(cm, 700, 500, 800, 600));
  ASSERT_EQ(600, cm->current_x);
  ASSERT_EQ(450, cm->current_y);
  ASSERT_EQ(UI_ERROR_NONE, md3_context_menu_destroy(cm));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_context_menu_create(engine, &cm));
  g_malloc_fail_countdown = -1;

  g_md3_overlay_menus_mock_fail = 26;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_context_menu_create(engine, &cm));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_context_menu_create(engine, &cm));

  g_md3_overlay_menus_mock_fail = 27;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_context_menu_destroy(cm));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_context_menu_destroy(cm));
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_action_sheet(void) {
  struct ui_engine *engine = create_engine();
  struct md3_action_sheet *as = NULL;
  int is_open = 0;
  int i;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_action_sheet_create(NULL, &as));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_action_sheet_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_action_sheet_add_action(NULL, "Lbl", NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_action_sheet_set_cancel_action(NULL, "Cancel"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_action_sheet_set_open(NULL, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_action_sheet_is_open(NULL, &is_open));

  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_create(engine, &as));

  for (i = 0; i < 16; i++) {
    ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_add_action(as, "Lbl", "Icon"));
  }
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_action_sheet_add_action(as, "Lbl", "Icon")); /* limit */

  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_set_cancel_action(as, "Cancel"));
  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_set_open(as, 1));
  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_is_open(as, &is_open));
  ASSERT_EQ(1, is_open);
  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_destroy(as));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_action_sheet_create(engine, &as));
  g_malloc_fail_countdown = -1;

  g_md3_overlay_menus_mock_fail = 28;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_action_sheet_create(engine, &as));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_create(engine, &as));

  g_md3_overlay_menus_mock_fail = 30;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_action_sheet_set_open(as, 1));
  g_md3_overlay_menus_mock_fail = 29;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_action_sheet_destroy(as));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_destroy(as));
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_coachmark(void) {
  struct ui_engine *engine = create_engine();
  struct ui_dom_node node;
  struct md3_coachmark *cm = NULL;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_coachmark_create(NULL, &node, &cm));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_coachmark_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(NULL, "T", "D", 0, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_coachmark_start(NULL));

  ASSERT_EQ(UI_ERROR_NONE, md3_coachmark_create(engine, &node, &cm));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(cm, "T", "D", -1, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(cm, "T", "D", 0, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(cm, "T", "D", 1, 1));
  ASSERT_EQ(UI_ERROR_NONE, md3_coachmark_set_content(cm, "T", "D", 0, 1));
  ASSERT_EQ(UI_ERROR_NONE, md3_coachmark_start(cm));
  ASSERT_EQ(UI_ERROR_NONE, md3_coachmark_destroy(cm));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_coachmark_create(engine, &node, &cm));
  g_malloc_fail_countdown = -1;
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_color_picker(void) {
  struct ui_engine *engine = create_engine();
  struct md3_color_picker *cp = NULL;
  struct ui_control_value_accessor *cva = NULL;
  double h, s, v;
  unsigned char r, g, b;
  char hex[16];
  struct ui_color_rgb rgb;
  union ui_signal_payload payload;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_create(NULL, &cp, &cva));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_color_picker_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_color_picker_set_hsv(NULL, 0, 0, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_hsv(NULL, &h, &s, &v));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_color_picker_set_rgb(NULL, 0, 0, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_rgb(NULL, &r, &g, &b));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_color_picker_set_hex(NULL, "#FFF"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_hex(NULL, hex, sizeof(hex)));

  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_create(engine, &cp, &cva));
  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_set_hsv(cp, 180, 0.5, 0.5));
  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_get_hsv(cp, &h, &s, &v));
  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_set_rgb(cp, 128, 64, 32));
  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_get_rgb(cp, &r, &g, &b));
  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_set_hex(cp, "#00FF00"));
  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_get_hex(cp, hex, sizeof(hex)));

  /* CVA methods */
  rgb.r = 255;
  rgb.g = 255;
  rgb.b = 255;
  payload.ptr_val = &rgb;
  ASSERT_EQ(UI_ERROR_NONE, cva->write_value(cp, payload));
  ASSERT_EQ(UI_ERROR_NONE, cva->set_disabled_state(cp, UI_TRUE));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva->write_value(NULL, payload));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva->set_disabled_state(NULL, UI_TRUE));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva->set_disabled_state(cp, 3));

  payload.ptr_val = NULL;
  ASSERT_EQ(
      UI_ERROR_NONE,
      cva->write_value(cp, payload)); /* Returns NONE on NULL payload usually */

  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_destroy(cp));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_color_picker_create(engine, &cp, &cva));
  g_malloc_fail_countdown = -1;

  g_md3_overlay_menus_mock_fail = 31;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_color_picker_create(engine, &cp, &cva));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_create(engine, &cp, &cva));

  g_md3_overlay_menus_mock_fail = 33;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_color_picker_set_hsv(cp, 180, 0.5, 0.5));
  g_md3_overlay_menus_mock_fail = 34;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_color_picker_set_hsv(cp, 180, 0.5, 0.5));
  g_md3_overlay_menus_mock_fail = 0;

  g_md3_overlay_menus_mock_fail = 35;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_color_picker_set_rgb(cp, 128, 64, 32));
  g_md3_overlay_menus_mock_fail = 34;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_color_picker_set_rgb(cp, 128, 64, 32));
  g_md3_overlay_menus_mock_fail = 0;

  g_md3_overlay_menus_mock_fail = 36;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_color_picker_set_hex(cp, "#FFFFFF"));
  g_md3_overlay_menus_mock_fail = 35;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_color_picker_set_hex(cp, "#FFFFFF"));
  g_md3_overlay_menus_mock_fail = 0;

  g_md3_overlay_menus_mock_fail = 32;
  ASSERT_EQ(UI_ERROR_UNKNOWN, md3_color_picker_destroy(cp));
  g_md3_overlay_menus_mock_fail = 0;

  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_destroy(cp));
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_command_palette(void) {
  struct ui_engine *engine = create_engine();
  struct md3_command_palette *cp = NULL;
  int is_open = 0;
  int i;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_command_palette_create(NULL, &cp));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_command_palette_destroy(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_command_palette_add_action(NULL, "C", "T", "S"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_command_palette_open(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_command_palette_close(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_command_palette_is_open(NULL, &is_open));

  ASSERT_EQ(UI_ERROR_NONE, md3_command_palette_create(engine, &cp));

  for (i = 0; i < 64; i++) {
    ASSERT_EQ(UI_ERROR_NONE,
              md3_command_palette_add_action(cp, "Cat", "Title", "Ctrl+T"));
  }
  ASSERT_EQ(
      UI_ERROR_INVALID_ARGUMENT,
      md3_command_palette_add_action(cp, "Cat", "Title", "Ctrl+T")); /* limit */

  ASSERT_EQ(UI_ERROR_NONE, md3_command_palette_open(cp));
  ASSERT_EQ(UI_ERROR_NONE, md3_command_palette_is_open(cp, &is_open));
  ASSERT_EQ(1, is_open);
  ASSERT_EQ(UI_ERROR_NONE, md3_command_palette_close(cp));
  ASSERT_EQ(UI_ERROR_NONE, md3_command_palette_destroy(cp));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, md3_command_palette_create(engine, &cp));
  g_malloc_fail_countdown = -1;
#endif

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_overlay_menus_extras(void) {
  struct ui_engine *engine = create_engine();
  struct md3_color_picker *cp = NULL;
  struct md3_command_palette *cmd = NULL;
  struct md3_action_sheet *as = NULL;

  /* Color picker create with NULL out_cva */
  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_create(engine, &cp, NULL));
  ASSERT_EQ(UI_ERROR_NONE, md3_color_picker_destroy(cp));

  /* Command palette action with NULL category and NULL shortcut */
  ASSERT_EQ(UI_ERROR_NONE, md3_command_palette_create(engine, &cmd));
  ASSERT_EQ(UI_ERROR_NONE,
            md3_command_palette_add_action(cmd, NULL, "Title", NULL));
  ASSERT_EQ(UI_ERROR_NONE, md3_command_palette_destroy(cmd));

  /* Action sheet add with NULL icon */
  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_create(engine, &as));
  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_add_action(as, "Title", NULL));
  ASSERT_EQ(UI_ERROR_NONE, md3_action_sheet_destroy(as));

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_overlay_menus_null_args(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  ui_error_t rc;
  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  struct ui_dom_node node;
  memset(&node, 0, sizeof(node));

  struct md3_popover *pop = NULL;
  rc = md3_popover_create(engine, &node, &pop);

  struct md3_banner *banner = NULL;
  rc = md3_banner_create(engine, &banner);

  struct md3_inline_alert *alert = NULL;
  rc = md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_INFO, &alert);

  struct md3_menubar *mb = NULL;
  rc = md3_menubar_create(engine, &mb);

  struct md3_context_menu *cm = NULL;
  rc = md3_context_menu_create(engine, &cm);

  struct md3_action_sheet *as = NULL;
  rc = md3_action_sheet_create(engine, &as);

  struct md3_coachmark *coachmark = NULL;
  rc = md3_coachmark_create(engine, &node, &coachmark);

  struct md3_color_picker *cp = NULL;
  struct ui_control_value_accessor *cva_tmp = NULL;
  rc = md3_color_picker_create(engine, &cp, &cva_tmp);

  struct md3_command_palette *palette = NULL;
  rc = md3_command_palette_create(engine, &palette);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_hover_card_create(engine, &node, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_popover_create(engine, &node, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_popover_is_open(pop, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_create(engine, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_set_text(banner, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_set_action(banner, 0, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_banner_set_action(banner, 2, "Action"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_banner_is_open(banner, NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_inline_alert_create(NULL, MD3_ALERT_SEVERITY_INFO, &alert));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_INFO, NULL));
  ASSERT_EQ(
      UI_ERROR_INVALID_ARGUMENT,
      md3_inline_alert_create(engine, (enum md3_alert_severity) - 1, &alert));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_create(NULL, &node, &coachmark));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_create(engine, NULL, &coachmark));
  ASSERT_EQ(UI_ERROR_NONE, cva_tmp->set_disabled_state(cp, UI_TRUE));
  ASSERT_EQ(UI_ERROR_NONE, cva_tmp->set_disabled_state(cp, UI_FALSE));

  ASSERT_EQ(
      UI_ERROR_INVALID_ARGUMENT,
      md3_inline_alert_create(engine, (enum md3_alert_severity)99, &alert));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_inline_alert_set_title(alert, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_inline_alert_set_message(alert, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_menubar_create(engine, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_menubar_append_item(mb, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_context_menu_create(engine, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_context_menu_open_at(cm, -1, 0, 10, 10));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_context_menu_open_at(cm, 0, -1, 10, 10));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_context_menu_open_at(cm, 0, 0, 0, 10));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_context_menu_open_at(cm, 0, 0, 10, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_action_sheet_create(engine, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_action_sheet_add_action(as, NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_action_sheet_set_cancel_action(as, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_action_sheet_is_open(as, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_create(engine, &node, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(coachmark, NULL, "D", 0, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(coachmark, "T", NULL, 0, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(coachmark, "T", "D", -1, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(coachmark, "T", "D", 0, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_coachmark_set_content(coachmark, "T", "D", 1, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_create(engine, NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_set_hsv(cp, -1.0, 0, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_set_hsv(cp, 360.0, 0, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_set_hsv(cp, 0, -0.1, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_color_picker_set_hsv(cp, 0, 1.1, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_set_hsv(cp, 0, 0, -0.1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_color_picker_set_hsv(cp, 0, 0, 1.1));
  double t_h, t_s, t_v;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_hsv(cp, NULL, &t_s, &t_v));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_hsv(cp, &t_h, NULL, &t_v));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_hsv(cp, &t_h, &t_s, NULL));
  unsigned char t_r, t_g, t_b;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_rgb(cp, NULL, &t_g, &t_b));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_rgb(cp, &t_r, NULL, &t_b));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_rgb(cp, &t_r, &t_g, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_color_picker_set_hex(cp, NULL));
  char hex_buf[8];
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, md3_color_picker_get_hex(cp, NULL, 8));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_color_picker_get_hex(cp, hex_buf, 7));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_command_palette_create(engine, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_command_palette_add_action(palette, NULL, NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            md3_command_palette_is_open(palette, NULL));

  md3_popover_destroy(pop);
  md3_banner_destroy(banner);
  md3_inline_alert_destroy(alert);
  md3_menubar_destroy(mb);
  md3_context_menu_destroy(cm);
  md3_action_sheet_destroy(as);
  md3_coachmark_destroy(coachmark);
  md3_color_picker_destroy(cp);
  md3_command_palette_destroy(palette);

  ui_engine_destroy(engine);
  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;

TEST test_md3_overlay_menus_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  ui_error_t rc;
  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  struct ui_dom_node node;
  memset(&node, 0, sizeof(node));
  struct ui_control_value_accessor *cva_tmp = NULL;

  int i;
  struct md3_hover_card *hc = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_hover_card_create(engine, &node, &hc);
    if (rc == UI_ERROR_NONE) {
      md3_hover_card_destroy(hc);
      break;
    }
  }

  struct md3_popover *pop = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_popover_create(engine, &node, &pop);
    if (rc == UI_ERROR_NONE) {
      md3_popover_destroy(pop);
      break;
    }
  }

  struct md3_banner *banner = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_banner_create(engine, &banner);
    if (rc == UI_ERROR_NONE) {
      md3_banner_destroy(banner);
      break;
    }
  }

  struct md3_inline_alert *alert = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_inline_alert_create(engine, MD3_ALERT_SEVERITY_WARNING, &alert);
    if (rc == UI_ERROR_NONE) {
      md3_inline_alert_destroy(alert);
      break;
    }
  }

  struct md3_menubar *mb = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_menubar_create(engine, &mb);
    if (rc == UI_ERROR_NONE) {
      md3_menubar_destroy(mb);
      break;
    }
  }

  struct md3_context_menu *cm = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_context_menu_create(engine, &cm);
    if (rc == UI_ERROR_NONE) {
      md3_context_menu_destroy(cm);
      break;
    }
  }

  struct md3_action_sheet *as = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_action_sheet_create(engine, &as);
    if (rc == UI_ERROR_NONE) {
      md3_action_sheet_destroy(as);
      break;
    }
  }

  struct md3_coachmark *coachmark = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_coachmark_create(engine, &node, &coachmark);
    if (rc == UI_ERROR_NONE) {
      md3_coachmark_destroy(coachmark);
      break;
    }
  }

  struct md3_color_picker *cp = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_color_picker_create(engine, &cp, &cva_tmp);
    if (rc == UI_ERROR_NONE) {
      md3_color_picker_destroy(cp);
      break;
    }
  }

  struct md3_command_palette *palette = NULL;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_command_palette_create(engine, &palette);
    if (rc == UI_ERROR_NONE) {
      md3_command_palette_destroy(palette);
      break;
    }
  }

  g_malloc_fail_countdown = -1;
  ui_engine_destroy(engine);
  PASS();
}
#endif

SUITE(md3_overlay_menus_suite) {
  RUN_TEST(test_md3_overlay_menus_null_args);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md3_overlay_menus_oom);
#endif

  RUN_TEST(test_md3_hover_card);
  RUN_TEST(test_md3_popover);
  RUN_TEST(test_md3_banner);
  RUN_TEST(test_md3_inline_alert);
  RUN_TEST(test_md3_menubar);
  RUN_TEST(test_md3_context_menu);
  RUN_TEST(test_md3_action_sheet);
  RUN_TEST(test_md3_coachmark);
  RUN_TEST(test_md3_color_picker);
  RUN_TEST(test_md3_command_palette);
  RUN_TEST(test_md3_overlay_menus_extras);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_overlay_menus_suite);
  GREATEST_MAIN_END();
}
