/**
 * @file test_material3_overlay_menus.c
 * @brief Comprehensive tests for Material 3 Overlay, Dialog, Menu, and Picker
 * Components.
 */

/* clang-format off */
#include "greatest.h"
#include "material3/md3_overlay_menus.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md3_overlay_menus_mock_fail;
extern int g_malloc_fail_countdown;
#endif

TEST test_md3_date_range_picker_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_date_range_picker *picker;
  struct ui_control_value_accessor *cva;
  struct ui_date d1;
  struct ui_date d2;
  struct ui_date_range range;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_date_range_picker_create(NULL, &picker, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_date_range_picker_create(dummy_engine, &picker, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(picker != NULL);
  ASSERT(cva != NULL);
  ASSERT_EQ(1, picker->is_dual_month);

  /* Select start date */
  d1.year = 2026;
  d1.month = 9;
  d1.day = 10;
  rc = md3_date_range_picker_select_date(picker, &d1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set hover date */
  d2.year = 2026;
  d2.month = 9;
  d2.day = 20;
  rc = md3_date_range_picker_set_hover_date(picker, &d2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(20, picker->hover_date.day);

  /* Select end date */
  rc = md3_date_range_picker_select_date(picker, &d2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Get range */
  rc = md3_date_range_picker_get_range(picker, &range);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, range.start_date.day);
  ASSERT_EQ(20, range.end_date.day);

  /* CVA read/write */
  {
    union ui_signal_payload payload;
    range.end_date.day = 25;
    payload.ptr_val = &range;
    rc = cva->write_value(picker, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(25, picker->range.end_date.day);
  }

  rc = cva->set_disabled_state(picker, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Clear */
  rc = md3_date_range_picker_clear(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy */
  rc = md3_date_range_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_hover_card_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct ui_dom_node *anchor;
  struct md3_hover_card *hc;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid args */
  rc = md3_hover_card_create(NULL, anchor, &hc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_hover_card_create(dummy_engine, NULL, &hc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_hover_card_create(dummy_engine, anchor, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_hover_card_create(dummy_engine, anchor, &hc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(hc != NULL);
  ASSERT_EQ(0, hc->is_visible);

  /* Show / Hide */
  rc = md3_hover_card_show(hc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, hc->is_visible);

  rc = md3_hover_card_hide(hc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, hc->is_visible);

  /* Destroy */
  rc = md3_hover_card_destroy(hc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_destroy(anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_popover_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct ui_dom_node *anchor;
  struct md3_popover *pop;
  int is_open;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid args */
  rc = md3_popover_create(NULL, anchor, &pop);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_popover_create(dummy_engine, anchor, &pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pop != NULL);
  ASSERT_EQ(2, pop->elevation_level);

  /* Open / Close */
  rc = md3_popover_open(pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_popover_is_open(pop, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = md3_popover_close(pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_popover_is_open(pop, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  /* Destroy */
  rc = md3_popover_destroy(pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_destroy(anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_banner_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_banner *banner;
  int is_open;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_banner_create(NULL, &banner);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_banner_create(dummy_engine, &banner);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(banner != NULL);

  /* Set text and actions */
  rc = md3_banner_set_text(banner, "Your connection is offline.");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Your connection is offline.", banner->message);

  rc = md3_banner_set_action(banner, 0, "Retry");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Retry", banner->action_primary);

  rc = md3_banner_set_action(banner, 1, "Dismiss");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Dismiss", banner->action_secondary);

  /* Open status */
  rc = md3_banner_is_open(banner, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = md3_banner_set_open(banner, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_banner_is_open(banner, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  /* Destroy */
  rc = md3_banner_destroy(banner);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_inline_alert_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_inline_alert *alert;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_inline_alert_create(NULL, MD3_ALERT_SEVERITY_INFO, &alert);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation across severities */
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_ERROR, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(alert != NULL);
  ASSERT_EQ(MD3_ALERT_SEVERITY_ERROR, alert->severity);

  rc = md3_inline_alert_set_title(alert, "Fatal Error");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Fatal Error", alert->title);

  rc = md3_inline_alert_set_message(alert, "Something went wrong.");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Something went wrong.", alert->message);

  rc = md3_inline_alert_set_dismissible(alert, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, alert->is_dismissible);

  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Info severity */
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_menubar_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_menubar *mb;
  struct ui_component *item;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_menubar_create(NULL, &mb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_menubar_create(dummy_engine, &mb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(mb != NULL);
  ASSERT_EQ(0, (int)mb->item_count);

  /* Append item */
  rc = ui_component_create(&item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_menubar_append_item(mb, item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)mb->item_count);

  /* Destroy */
  rc = md3_menubar_destroy(mb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  item->shadow_root = NULL;
  rc = ui_component_destroy(item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_context_menu_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_context_menu *cm;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_context_menu_create(NULL, &cm);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_context_menu_create(dummy_engine, &cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cm != NULL);
  ASSERT_EQ(0, cm->is_open);

  /* Open at coordinates */
  rc = md3_context_menu_open_at(cm, 50, 80, 1024, 768);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(50, cm->current_x);
  ASSERT_EQ(80, cm->current_y);
  ASSERT_EQ(1, cm->is_open);

  /* Boundary clamp */
  rc = md3_context_menu_open_at(cm, 950, 700, 1024, 768);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(824, cm->current_x);
  ASSERT_EQ(618, cm->current_y);

  /* Destroy */
  rc = md3_context_menu_destroy(cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_action_sheet_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_action_sheet *sheet;
  int is_open;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_action_sheet_create(NULL, &sheet);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_action_sheet_create(dummy_engine, &sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sheet != NULL);
  ASSERT_EQ(0, (int)sheet->action_count);

  /* Add actions */
  rc = md3_action_sheet_add_action(sheet, "Share via Mail", "mail");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_action_sheet_add_action(sheet, "Copy Link", "link");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)sheet->action_count);

  rc = md3_action_sheet_set_cancel_action(sheet, "Cancel");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Cancel", sheet->cancel_label);

  /* Open status */
  rc = md3_action_sheet_set_open(sheet, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_action_sheet_is_open(sheet, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = md3_action_sheet_set_open(sheet, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy */
  rc = md3_action_sheet_destroy(sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_coachmark_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct ui_dom_node *target;
  struct md3_coachmark *cm;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid args */
  rc = md3_coachmark_create(NULL, target, &cm);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_coachmark_create(dummy_engine, target, &cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cm != NULL);
  ASSERT_EQ(0, cm->is_active);

  /* Set content */
  rc = md3_coachmark_set_content(cm, "New Feature", "Click here to start.", 0,
                                 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("New Feature", cm->title);
  ASSERT_EQ(0, cm->current_step);
  ASSERT_EQ(3, cm->total_steps);

  /* Start */
  rc = md3_coachmark_start(cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, cm->is_active);

  /* Destroy */
  rc = md3_coachmark_destroy(cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_destroy(target);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_color_picker_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_color_picker *picker;
  struct ui_control_value_accessor *cva;
  double h, s, v;
  unsigned char r, g, b;
  char hex[16];
  struct ui_color_rgb rgb_val;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_color_picker_create(NULL, &picker, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_color_picker_create(dummy_engine, &picker, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(picker != NULL);
  ASSERT(cva != NULL);

  /* Set HSV */
  rc = md3_color_picker_set_hsv(picker, 120.0, 1.0, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_picker_get_hsv(picker, &h, &s, &v);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(120.0, h, "%f");

  /* Set RGB */
  rc = md3_color_picker_set_rgb(picker, 0, 0, 255);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_picker_get_rgb(picker, &r, &g, &b);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)r);
  ASSERT_EQ(0, (int)g);
  ASSERT_EQ(255, (int)b);

  /* Set HEX */
  rc = md3_color_picker_set_hex(picker, "#FFFF00");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_picker_get_hex(picker, hex, sizeof(hex));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("#FFFF00", hex);

  /* CVA write value */
  {
    union ui_signal_payload payload;
    rgb_val.r = 100;
    rgb_val.g = 150;
    rgb_val.b = 200;
    payload.ptr_val = &rgb_val;
    rc = cva->write_value(picker, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(100, (int)picker->rgb.r);
  }

  rc = cva->set_disabled_state(picker, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy */
  rc = md3_color_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_command_palette_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_command_palette *palette;
  int is_open;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_command_palette_create(NULL, &palette);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_command_palette_create(dummy_engine, &palette);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(palette != NULL);
  ASSERT_EQ(0, (int)palette->command_count);
  ASSERT_EQ(0, palette->is_open);

  /* Add actions */
  rc = md3_command_palette_add_action(palette, "Navigation", "Go to Dashboard",
                                      "Cmd+D");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_command_palette_add_action(palette, "File", "Save Workspace",
                                      "Cmd+S");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)palette->command_count);

  /* Open / Close */
  rc = md3_command_palette_open(palette);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_command_palette_is_open(palette, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = md3_command_palette_close(palette);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_command_palette_is_open(palette, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  /* Destroy */
  rc = md3_command_palette_destroy(palette);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_overlay_menus_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_date_range_picker *picker = NULL;
  struct md3_hover_card *hc = NULL;
  struct md3_popover *pop = NULL;
  struct md3_banner *banner = NULL;
  struct md3_inline_alert *alert = NULL;
  struct md3_menubar *mb = NULL;
  struct md3_context_menu *cm = NULL;
  struct md3_action_sheet *as = NULL;
  struct md3_coachmark *coachmark = NULL;
  struct md3_color_picker *cp = NULL;
  struct md3_command_palette *palette = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_dom_node *anchor = NULL;
  struct ui_component comp;
  struct ui_component comp_with_shadow;
  struct ui_date date;
  struct ui_date_range range;
  union ui_signal_payload payload;
  char hex_buf[32];
  double h, s, v;
  unsigned char r, g, b;
  int is_open = 0;
  int i;
  ui_error_t rc;

  memset(&comp, 0, sizeof(comp));
  memset(&comp_with_shadow, 0, sizeof(comp_with_shadow));
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                          &comp_with_shadow.shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 1. Date Range Picker */
  rc = md3_date_range_picker_create(dummy_engine, &picker, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cva write_value */
  payload.ptr_val = NULL;
  rc = picker->cva.write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = picker->cva.write_value(picker, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(&range, 0, sizeof(range));
  payload.ptr_val = &range;
  rc = picker->cva.write_value(picker, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cva set_disabled_state */
  rc = picker->cva.set_disabled_state(NULL, UI_TRUE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = picker->cva.set_disabled_state(picker, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = picker->cva.set_disabled_state(picker, UI_FALSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = picker->cva.set_disabled_state(picker, (ui_bool_t)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_date_range_picker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  picker->base = NULL;
  rc = md3_date_range_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_date_range_picker_create(dummy_engine, &picker, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(&date, 0, sizeof(date));
  rc = md3_date_range_picker_select_date(NULL, &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_select_date(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_set_hover_date(NULL, &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_set_hover_date(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_get_range(NULL, &range);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_get_range(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_clear(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 2. Hover Card */
  rc = md3_hover_card_create(NULL, anchor, &hc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_hover_card_create(dummy_engine, NULL, &hc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_hover_card_create(dummy_engine, anchor, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_hover_card_create(dummy_engine, anchor, &hc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_hover_card_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  hc->base = NULL;
  rc = md3_hover_card_destroy(hc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_hover_card_show(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_hover_card_hide(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* 3. Popover */
  rc = md3_popover_create(NULL, anchor, &pop);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_popover_create(dummy_engine, NULL, &pop);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_popover_create(dummy_engine, anchor, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_popover_create(dummy_engine, anchor, &pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_popover_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  pop->base = NULL;
  rc = md3_popover_destroy(pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_popover_open(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_popover_close(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_popover_is_open(NULL, &is_open);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_popover_create(dummy_engine, anchor, &pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_popover_is_open(pop, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_popover_destroy(pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 4. Banner */
  rc = md3_banner_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_create(dummy_engine, &banner);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_banner_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  banner->base = NULL;
  rc = md3_banner_destroy(banner);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_banner_create(dummy_engine, &banner);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_banner_set_text(NULL, "txt");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_set_text(banner, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_set_action(NULL, 0, "act");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_set_action(banner, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_set_action(banner, 2, "act");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_set_action(banner, -1, "act");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_set_action(banner, 0, "act0");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_banner_set_action(banner, 1, "act1");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_banner_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_set_open(banner, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_banner_is_open(NULL, &is_open);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_is_open(banner, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_banner_destroy(banner);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 5. Inline Alert */
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_inline_alert_create(dummy_engine, (enum md3_alert_severity) - 1,
                               &alert);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_inline_alert_create(dummy_engine, (enum md3_alert_severity)99,
                               &alert);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_ERROR, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc =
      md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_WARNING, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc =
      md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_SUCCESS, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_inline_alert_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  alert->base = NULL;
  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_inline_alert_set_title(NULL, "t");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_inline_alert_set_title(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_inline_alert_set_message(NULL, "m");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_inline_alert_set_message(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_inline_alert_set_dismissible(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_inline_alert_set_dismissible(alert, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 6. Menubar */
  rc = md3_menubar_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_menubar_create(dummy_engine, &mb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_menubar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  mb->base = NULL;
  rc = md3_menubar_destroy(mb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_menubar_create(dummy_engine, &mb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_menubar_append_item(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_menubar_append_item(mb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_menubar_append_item(mb, &comp_with_shadow);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_menubar_destroy(mb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 7. Context Menu */
  rc = md3_context_menu_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_context_menu_create(dummy_engine, &cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_context_menu_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  cm->base = NULL;
  rc = md3_context_menu_destroy(cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_context_menu_create(dummy_engine, &cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_context_menu_open_at(NULL, 0, 0, 100, 100);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_context_menu_open_at(cm, -1, 0, 100, 100);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_context_menu_open_at(cm, 0, -1, 100, 100);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_context_menu_open_at(cm, 0, 0, 0, 100);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_context_menu_open_at(cm, 0, 0, 100, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* Clamped positions */
  rc = md3_context_menu_open_at(cm, 900, 700, 1000, 800);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(800, cm->current_x);
  ASSERT_EQ(650, cm->current_y);
  /* Unclamped positions */
  rc = md3_context_menu_open_at(cm, 50, 50, 1000, 800);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(50, cm->current_x);
  ASSERT_EQ(50, cm->current_y);
  rc = md3_context_menu_destroy(cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 8. Action Sheet */
  rc = md3_action_sheet_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_action_sheet_create(dummy_engine, &as);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_action_sheet_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  as->base = NULL;
  rc = md3_action_sheet_destroy(as);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_action_sheet_create(dummy_engine, &as);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_action_sheet_add_action(NULL, "act", "icon");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_action_sheet_add_action(as, NULL, "icon");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_action_sheet_add_action(as, "no_icon", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  for (i = 1; i < 16; i++) {
    rc = md3_action_sheet_add_action(as, "a", "i");
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_action_sheet_add_action(as, "over", "i");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_action_sheet_set_cancel_action(NULL, "c");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_action_sheet_set_cancel_action(as, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_action_sheet_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_action_sheet_set_open(as, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_action_sheet_is_open(NULL, &is_open);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_action_sheet_is_open(as, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_action_sheet_destroy(as);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 9. Coachmark */
  rc = md3_coachmark_create(NULL, anchor, &coachmark);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_coachmark_create(dummy_engine, NULL, &coachmark);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_coachmark_create(dummy_engine, anchor, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_coachmark_create(dummy_engine, anchor, &coachmark);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_coachmark_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_coachmark_set_content(NULL, "t", "d", 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_coachmark_set_content(coachmark, NULL, "d", 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_coachmark_set_content(coachmark, "t", NULL, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_coachmark_set_content(coachmark, "t", "d", -1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_coachmark_set_content(coachmark, "t", "d", 0, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_coachmark_set_content(coachmark, "t", "d", 1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_coachmark_start(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_coachmark_destroy(coachmark);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 10. Color Picker */
  rc = md3_color_picker_create(NULL, &cp, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_create(dummy_engine, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_create(dummy_engine, &cp, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* cva write_value */
  payload.ptr_val = NULL;
  rc = cp->cva.write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cp->cva.write_value(cp, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  {
    struct ui_color_rgb rgb_val;
    rgb_val.r = 10;
    rgb_val.g = 20;
    rgb_val.b = 30;
    payload.ptr_val = &rgb_val;
    rc = cp->cva.write_value(cp, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* cva set_disabled_state */
  rc = cp->cva.set_disabled_state(NULL, UI_TRUE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cp->cva.set_disabled_state(cp, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cp->cva.set_disabled_state(cp, UI_FALSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cp->cva.set_disabled_state(cp, (ui_bool_t)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_color_picker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  cp->base = NULL;
  rc = md3_color_picker_destroy(cp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_color_picker_create(dummy_engine, &cp, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_hsv bounds */
  rc = md3_color_picker_set_hsv(NULL, 0.0, 1.0, 1.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_set_hsv(cp, -1.0, 1.0, 1.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_set_hsv(cp, 360.0, 1.0, 1.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_set_hsv(cp, 0.0, -0.1, 1.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_set_hsv(cp, 0.0, 1.1, 1.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_set_hsv(cp, 0.0, 1.0, -0.1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_set_hsv(cp, 0.0, 1.0, 1.1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* get_hsv NULL checks */
  rc = md3_color_picker_get_hsv(NULL, &h, &s, &v);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_hsv(cp, NULL, &s, &v);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_hsv(cp, &h, NULL, &v);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_hsv(cp, &h, &s, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* set_rgb / get_rgb */
  rc = md3_color_picker_set_rgb(NULL, 0, 0, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_rgb(NULL, &r, &g, &b);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_rgb(cp, NULL, &g, &b);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_rgb(cp, &r, NULL, &b);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_rgb(cp, &r, &g, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* set_hex / get_hex */
  rc = md3_color_picker_set_hex(NULL, "#000000");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_set_hex(cp, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_hex(NULL, hex_buf, sizeof(hex_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_hex(cp, NULL, sizeof(hex_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_picker_get_hex(cp, hex_buf, 7);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_color_picker_destroy(cp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 11. Command Palette */
  rc = md3_command_palette_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_command_palette_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_command_palette_create(dummy_engine, &palette);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_command_palette_add_action(NULL, "c", "t", "s");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_command_palette_add_action(palette, "c", NULL, "s");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_command_palette_add_action(palette, NULL, "t0", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  for (i = 1; i < 64; i++) {
    rc = md3_command_palette_add_action(palette, "c", "t", "s");
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_command_palette_add_action(palette, "c", "over", "s");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_command_palette_open(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_command_palette_close(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_command_palette_is_open(NULL, &is_open);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_command_palette_is_open(palette, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_command_palette_destroy(palette);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_destroy(anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(comp_with_shadow.shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_overlay_menus_mock_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_date_range_picker *picker = NULL;
  struct md3_hover_card *hc = NULL;
  struct md3_popover *pop = NULL;
  struct md3_banner *banner = NULL;
  struct md3_inline_alert *alert = NULL;
  struct md3_menubar *mb = NULL;
  struct md3_context_menu *cm = NULL;
  struct md3_action_sheet *as = NULL;
  struct md3_color_picker *cp = NULL;
  struct ui_dom_node *anchor = NULL;
  struct ui_component comp;
  struct ui_date date;
  ui_error_t rc;

  memset(&comp, 0, sizeof(comp));
  date.year = 2026;
  date.month = 9;
  date.day = 10;
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 1: drp base create fails */
  g_md3_overlay_menus_mock_fail = 1;
  rc = md3_date_range_picker_create(dummy_engine, &picker, NULL);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 2: drp base destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_date_range_picker_create(dummy_engine, &picker, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 2;
  rc = md3_date_range_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_date_range_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 3: drp select_date fails */
  rc = md3_date_range_picker_create(dummy_engine, &picker, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 3;
  rc = md3_date_range_picker_select_date(picker, &date);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 4: drp get_range fails inside select_date */
  g_md3_overlay_menus_mock_fail = 4;
  rc = md3_date_range_picker_select_date(picker, &date);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 5: drp set_hover_date fails */
  g_md3_overlay_menus_mock_fail = 5;
  rc = md3_date_range_picker_set_hover_date(picker, &date);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 6: drp clear fails */
  g_md3_overlay_menus_mock_fail = 6;
  rc = md3_date_range_picker_clear(picker);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_date_range_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 7: hover card create fails */
  g_md3_overlay_menus_mock_fail = 7;
  rc = md3_hover_card_create(dummy_engine, anchor, &hc);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 8: hover card destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_hover_card_create(dummy_engine, anchor, &hc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 8;
  rc = md3_hover_card_destroy(hc);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;

  /* Mock 9: hover card show fails */
  g_md3_overlay_menus_mock_fail = 9;
  rc = md3_hover_card_show(hc);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 10: hover card hide fails */
  g_md3_overlay_menus_mock_fail = 10;
  rc = md3_hover_card_hide(hc);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_hover_card_destroy(hc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 11: popover create fails */
  g_md3_overlay_menus_mock_fail = 11;
  rc = md3_popover_create(dummy_engine, anchor, &pop);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 12: popover destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_popover_create(dummy_engine, anchor, &pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 12;
  rc = md3_popover_destroy(pop);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;

  /* Mock 13: popover close fails */
  g_md3_overlay_menus_mock_fail = 13;
  rc = md3_popover_close(pop);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_popover_destroy(pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 14: banner create fails */
  g_md3_overlay_menus_mock_fail = 14;
  rc = md3_banner_create(dummy_engine, &banner);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 15: banner destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_banner_create(dummy_engine, &banner);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 15;
  rc = md3_banner_destroy(banner);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;

  /* Mock 16: banner set_text fails */
  g_md3_overlay_menus_mock_fail = 16;
  rc = md3_banner_set_text(banner, "msg");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 17: banner set_open fails */
  g_md3_overlay_menus_mock_fail = 17;
  rc = md3_banner_set_open(banner, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_banner_destroy(banner);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 18: alert create fails */
  g_md3_overlay_menus_mock_fail = 18;
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, &alert);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 19: alert set_role fails */
  g_md3_overlay_menus_mock_fail = 19;
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, &alert);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 20: alert destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 20;
  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;

  /* Mock 21: alert set_dismissible fails */
  g_md3_overlay_menus_mock_fail = 21;
  rc = md3_inline_alert_set_dismissible(alert, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_inline_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 22: menubar create fails */
  g_md3_overlay_menus_mock_fail = 22;
  rc = md3_menubar_create(dummy_engine, &mb);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 23: menubar destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_menubar_create(dummy_engine, &mb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 23;
  rc = md3_menubar_destroy(mb);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;

  /* Mock 24: menubar append_item dom_node_create fails */
  g_md3_overlay_menus_mock_fail = 24;
  rc = md3_menubar_append_item(mb, &comp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 25: menubar append_item fails */
  g_md3_overlay_menus_mock_fail = 25;
  rc = md3_menubar_append_item(mb, &comp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_menubar_destroy(mb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 26: context menu create fails */
  g_md3_overlay_menus_mock_fail = 26;
  rc = md3_context_menu_create(dummy_engine, &cm);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 27: context menu destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_context_menu_create(dummy_engine, &cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 27;
  rc = md3_context_menu_destroy(cm);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_context_menu_destroy(cm);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 28: action sheet create fails */
  g_md3_overlay_menus_mock_fail = 28;
  rc = md3_action_sheet_create(dummy_engine, &as);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 29: action sheet destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_action_sheet_create(dummy_engine, &as);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 29;
  rc = md3_action_sheet_destroy(as);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;

  /* Mock 30: action sheet set_open fails */
  g_md3_overlay_menus_mock_fail = 30;
  rc = md3_action_sheet_set_open(as, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_action_sheet_destroy(as);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 31: color picker create fails */
  g_md3_overlay_menus_mock_fail = 31;
  rc = md3_color_picker_create(dummy_engine, &cp, NULL);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 32: color picker destroy fails */
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_color_picker_create(dummy_engine, &cp, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_overlay_menus_mock_fail = 32;
  rc = md3_color_picker_destroy(cp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;

  /* Mock 33: color picker hsv_to_rgb fails */
  g_md3_overlay_menus_mock_fail = 33;
  rc = md3_color_picker_set_hsv(cp, 0.0, 1.0, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 34: color picker rgb_to_hex fails */
  g_md3_overlay_menus_mock_fail = 34;
  rc = md3_color_picker_set_hsv(cp, 0.0, 1.0, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  rc = md3_color_picker_set_rgb(cp, 255, 0, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 35: color picker rgb_to_hsv fails */
  g_md3_overlay_menus_mock_fail = 35;
  rc = md3_color_picker_set_rgb(cp, 255, 0, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  rc = md3_color_picker_set_hex(cp, "#FF0000");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 36: color picker hex_to_rgb fails */
  g_md3_overlay_menus_mock_fail = 36;
  rc = md3_color_picker_set_hex(cp, "#FF0000");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_overlay_menus_mock_fail = 0;
  rc = md3_color_picker_destroy(cp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_destroy(anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#endif
  PASS();
}

TEST test_md3_overlay_menus_oom_alloc(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_date_range_picker *picker = NULL;
  struct md3_hover_card *hc = NULL;
  struct md3_popover *pop = NULL;
  struct md3_banner *banner = NULL;
  struct md3_inline_alert *alert = NULL;
  struct md3_menubar *mb = NULL;
  struct md3_context_menu *cm = NULL;
  struct md3_action_sheet *as = NULL;
  struct md3_coachmark *coachmark = NULL;
  struct md3_color_picker *cp = NULL;
  struct md3_command_palette *palette = NULL;
  struct ui_dom_node *anchor = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_date_range_picker_create(dummy_engine, &picker, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_hover_card_create(dummy_engine, anchor, &hc);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_popover_create(dummy_engine, anchor, &pop);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_banner_create(dummy_engine, &banner);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, &alert);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_menubar_create(dummy_engine, &mb);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_context_menu_create(dummy_engine, &cm);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_action_sheet_create(dummy_engine, &as);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_coachmark_create(dummy_engine, anchor, &coachmark);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_color_picker_create(dummy_engine, &cp, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_command_palette_create(dummy_engine, &palette);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  rc = ui_dom_node_destroy(anchor);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif
  PASS();
}

SUITE(md3_overlay_menus_suite) {
  RUN_TEST(test_md3_date_range_picker_lifecycle);
  RUN_TEST(test_md3_hover_card_lifecycle);
  RUN_TEST(test_md3_popover_lifecycle);
  RUN_TEST(test_md3_banner_lifecycle);
  RUN_TEST(test_md3_inline_alert_lifecycle);
  RUN_TEST(test_md3_menubar_lifecycle);
  RUN_TEST(test_md3_context_menu_lifecycle);
  RUN_TEST(test_md3_action_sheet_lifecycle);
  RUN_TEST(test_md3_coachmark_lifecycle);
  RUN_TEST(test_md3_color_picker_lifecycle);
  RUN_TEST(test_md3_command_palette_lifecycle);
  RUN_TEST(test_md3_overlay_menus_branches);
  RUN_TEST(test_md3_overlay_menus_mock_failures);
  RUN_TEST(test_md3_overlay_menus_oom_alloc);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_overlay_menus_suite);
  GREATEST_MAIN_END();
}
