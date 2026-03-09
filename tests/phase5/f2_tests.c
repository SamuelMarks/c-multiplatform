#include "cmpc/cmp_core.h"
#include "f2/f2_badge.h"
#include "f2/f2_button.h"
#include "f2/f2_card.h"
#include "f2/f2_color.h"
#include "f2/f2_command_bar.h"
#include "f2/f2_data_components.h"
#include "f2/f2_dialog.h"
#include "f2/f2_flyout.h"
#include "f2/f2_link.h"
#include "f2/f2_progress.h"
#include "f2/f2_searchbox.h"
#include "f2/f2_selection.h"
#include "f2/f2_slider.h"
#include "f2/f2_spin_button.h"
#include "f2/f2_text_field.h"
#include "f2/f2_tooltip.h"
#include "f2/f2_typography.h"
#include <stdio.h>

static int test_create_font(void *text, const char *utf8_family,
                            cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                            CMPHandle *out_font) {
  if (out_font) {
    out_font->id = 1;
    out_font->generation = 1;
  }
  return CMP_OK;
}
static int test_destroy_font(void *text, CMPHandle font) { return CMP_OK; }
static int test_measure_text(void *text, CMPHandle font, const char *utf8,
                             cmp_usize utf8_len, cmp_u32 base_direction,
                             CMPScalar *w, CMPScalar *h, CMPScalar *baseline) {
  if (w)
    *w = 10.0f;
  if (h)
    *h = 10.0f;
  if (baseline)
    *baseline = 8.0f;
  return CMP_OK;
}
static int test_draw_text(void *ctx, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, cmp_u32 base_direction,
                          CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}
static int test_shape_text(void *ctx, CMPHandle font, const char *utf8,
                           cmp_usize utf8_len, cmp_u32 base_direction,
                           CMPTextLayout *out_layout) {
  return CMP_OK;
}
static int test_free_layout(void *ctx, CMPTextLayout *layout) { return CMP_OK; }

static int test_f2_spin_button(void) {
  F2SpinButtonStyle style;
  F2SpinButton sb;
  CMPTextBackend backend;
  CMPTextVTable vtable;
  CMPAllocator allocator;

  vtable.create_font = test_create_font;
  vtable.destroy_font = test_destroy_font;
  vtable.measure_text = test_measure_text;
  vtable.draw_text = test_draw_text;
  vtable.shape_text = test_shape_text;
  vtable.free_layout = test_free_layout;
  vtable.draw_layout = NULL;

  backend.ctx = NULL;
  backend.vtable = &vtable;

  cmp_get_default_allocator(&allocator);

  if (f2_spin_button_style_init(&style) != CMP_OK)
    return 1;
  if (f2_spin_button_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_spin_button_init(&sb, &backend, &style, &allocator, 0, 100, 1, 50) !=
      CMP_OK)
    return 1;
  if (f2_spin_button_init(NULL, &backend, &style, &allocator, 0, 100, 1, 50) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_spin_button_init(&sb, NULL, &style, &allocator, 0, 100, 1, 50) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_spin_button_set_value(&sb, 75) != CMP_OK)
    return 1;
  if (f2_spin_button_set_value(NULL, 75) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_spin_button_set_on_change(&sb, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_spin_button_set_on_change(NULL, NULL, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_spin_button_cleanup(&sb) != CMP_OK)
    return 1;
  if (f2_spin_button_cleanup(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_searchbox(void) {
  F2SearchBoxStyle style;
  F2SearchBox sb;
  CMPTextBackend backend;
  CMPTextVTable vtable;
  CMPAllocator allocator;

  vtable.create_font = test_create_font;
  vtable.destroy_font = test_destroy_font;
  vtable.measure_text = test_measure_text;
  vtable.draw_text = test_draw_text;
  vtable.shape_text = test_shape_text;
  vtable.free_layout = test_free_layout;
  vtable.draw_layout = NULL;

  backend.ctx = NULL;
  backend.vtable = &vtable;

  cmp_get_default_allocator(&allocator);

  if (f2_search_box_style_init(&style) != CMP_OK)
    return 1;
  if (f2_search_box_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_search_box_init(&sb, &backend, &style, &allocator) != CMP_OK)
    return 1;
  if (f2_search_box_init(NULL, &backend, &style, &allocator) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_search_box_init(&sb, NULL, &style, &allocator) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_search_box_set_text(&sb, "query") != CMP_OK)
    return 1;
  if (f2_search_box_set_text(NULL, "query") != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_search_box_set_callbacks(&sb, NULL, NULL, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_search_box_set_callbacks(NULL, NULL, NULL, NULL, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_search_box_cleanup(&sb) != CMP_OK)
    return 1;
  if (f2_search_box_cleanup(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_tooltip(void) {
  F2TooltipStyle style;
  F2Tooltip tooltip;
  CMPRect bounds = {0};

  if (f2_tooltip_style_init(&style) != CMP_OK)
    return 1;
  if (f2_tooltip_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_tooltip_init(&tooltip, &style, "Test") != CMP_OK)
    return 1;
  if (f2_tooltip_init(NULL, &style, "Test") != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_tooltip_init(&tooltip, NULL, "Test") != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_tooltip_show(&tooltip, bounds) != CMP_OK)
    return 1;
  if (f2_tooltip_show(NULL, bounds) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_tooltip_hide(&tooltip) != CMP_OK)
    return 1;
  if (f2_tooltip_hide(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_card(void) {
  F2CardStyle style;
  F2Card card;

  if (f2_card_style_init(&style) != CMP_OK)
    return 1;
  if (f2_card_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_card_init(&card, &style) != CMP_OK)
    return 1;
  if (f2_card_init(NULL, &style) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_card_init(&card, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_card_set_on_click(&card, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_card_set_on_click(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_flyout(void) {
  F2FlyoutStyle style;
  F2Flyout flyout;
  CMPRect bounds = {0};

  if (f2_flyout_style_init(&style) != CMP_OK)
    return 1;
  if (f2_flyout_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_flyout_init(&flyout, &style) != CMP_OK)
    return 1;
  if (f2_flyout_init(NULL, &style) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_flyout_init(&flyout, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_flyout_show(&flyout, bounds) != CMP_OK)
    return 1;
  if (f2_flyout_show(NULL, bounds) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_flyout_hide(&flyout) != CMP_OK)
    return 1;
  if (f2_flyout_hide(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_dialog(void) {
  F2DialogStyle style;
  F2Dialog dialog;

  if (f2_dialog_style_init(&style) != CMP_OK)
    return 1;
  if (f2_dialog_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_dialog_init(&dialog, &style, "T", "B") != CMP_OK)
    return 1;
  if (f2_dialog_init(NULL, &style, "T", "B") != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_dialog_init(&dialog, NULL, "T", "B") != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_dialog_open(&dialog) != CMP_OK)
    return 1;
  if (f2_dialog_open(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_dialog_close(&dialog) != CMP_OK)
    return 1;
  if (f2_dialog_close(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_dialog_set_on_action(&dialog, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_dialog_set_on_action(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_slider(void) {
  F2SliderStyle style;
  F2Slider slider;

  if (f2_slider_style_init(&style) != CMP_OK)
    return 1;
  if (f2_slider_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_slider_init(&slider, &style, 0.0f, 100.0f, 50.0f) != CMP_OK)
    return 1;
  if (f2_slider_init(NULL, &style, 0.0f, 100.0f, 50.0f) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_slider_init(&slider, NULL, 0.0f, 100.0f, 50.0f) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_slider_init(&slider, &style, 100.0f, 0.0f, 50.0f) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_slider_set_value(&slider, 75.0f) != CMP_OK)
    return 1;
  if (f2_slider_set_value(NULL, 75.0f) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_slider_set_step(&slider, 10.0f) != CMP_OK)
    return 1;
  if (f2_slider_set_step(NULL, 10.0f) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_slider_set_disabled(&slider, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_slider_set_disabled(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_slider_set_on_change(&slider, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_slider_set_on_change(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_progress(void) {
  F2LinearProgressStyle lp_style;
  F2LinearProgress lp;
  F2CircularProgressStyle cp_style;
  F2CircularProgress cp;

  if (f2_linear_progress_style_init(&lp_style) != CMP_OK)
    return 1;
  if (f2_linear_progress_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_linear_progress_init(&lp, &lp_style, 0.5f) != CMP_OK)
    return 1;
  if (f2_linear_progress_init(NULL, &lp_style, 0.5f) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_linear_progress_init(&lp, NULL, 0.5f) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_linear_progress_set_value(&lp, 0.8f) != CMP_OK)
    return 1;
  if (f2_linear_progress_set_value(NULL, 0.8f) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_circular_progress_style_init(&cp_style) != CMP_OK)
    return 1;
  if (f2_circular_progress_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_circular_progress_init(&cp, &cp_style, 0.5f) != CMP_OK)
    return 1;
  if (f2_circular_progress_init(NULL, &cp_style, 0.5f) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_circular_progress_init(&cp, NULL, 0.5f) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_circular_progress_set_value(&cp, 0.8f) != CMP_OK)
    return 1;
  if (f2_circular_progress_set_value(NULL, 0.8f) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_badge(void) {
  F2BadgeStyle b_style;
  F2Badge badge;
  F2PresenceBadgeStyle p_style;
  F2PresenceBadge p_badge;

  if (f2_badge_style_init(&b_style) != CMP_OK)
    return 1;
  if (f2_badge_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_badge_init(&badge, &b_style, "1", 1) != CMP_OK)
    return 1;
  if (f2_badge_init(NULL, &b_style, "1", 1) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_badge_init(&badge, NULL, "1", 1) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_badge_set_label(&badge, "2", 1) != CMP_OK)
    return 1;
  if (f2_badge_set_label(NULL, "2", 1) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_presence_badge_style_init(&p_style) != CMP_OK)
    return 1;
  if (f2_presence_badge_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_presence_badge_init(&p_badge, &p_style, F2_PRESENCE_AVAILABLE) !=
      CMP_OK)
    return 1;
  if (f2_presence_badge_init(NULL, &p_style, F2_PRESENCE_AVAILABLE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_presence_badge_init(&p_badge, NULL, F2_PRESENCE_AVAILABLE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_presence_badge_set_state(&p_badge, F2_PRESENCE_BUSY) != CMP_OK)
    return 1;
  if (f2_presence_badge_set_state(NULL, F2_PRESENCE_BUSY) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_link(void) {
  F2LinkStyle style;
  F2Link link;

  if (f2_link_style_init(&style) != CMP_OK)
    return 1;
  if (f2_link_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_link_init(&link, &style, "Test", 4) != CMP_OK)
    return 1;
  if (f2_link_init(NULL, &style, "Test", 4) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_link_init(&link, NULL, "Test", 4) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_link_set_text(&link, "Hello", 5) != CMP_OK)
    return 1;
  if (f2_link_set_text(NULL, "Hello", 5) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_link_set_disabled(&link, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_link_set_disabled(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_link_set_on_click(&link, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_link_set_on_click(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_color(void) {
  F2ColorRamp ramp;
  F2Theme theme;
  int res;

  res = f2_color_ramp_generate(0xFF0000FF, &ramp);
  if (res != CMP_OK)
    return 1;

  res = f2_theme_generate(0xFF0000FF, CMP_FALSE, &theme);
  if (res != CMP_OK)
    return 1;

  res = f2_theme_generate(0xFF0000FF, CMP_TRUE, &theme);
  if (res != CMP_OK)
    return 1;

  if (f2_theme_apply_high_contrast(&theme, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_theme_apply_high_contrast(&theme, CMP_FALSE) != CMP_OK)
    return 1;
  if (f2_theme_apply_high_contrast(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_color_ramp_generate(0xFF0000FF, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_theme_generate(0xFF0000FF, CMP_FALSE, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_typography(void) {
  F2TypographyScale scale;
  CMPTextStyle style;

  if (f2_typography_scale_init(&scale) != CMP_OK)
    return 1;
  if (f2_typography_scale_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_typography_get_style(&scale, F2_TYPOGRAPHY_BODY, &style) != CMP_OK)
    return 1;
  if (f2_typography_get_style(NULL, F2_TYPOGRAPHY_BODY, &style) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_typography_get_style(&scale, F2_TYPOGRAPHY_ROLE_COUNT, &style) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_typography_scale_set_color(
          &scale, (CMPColor){1.0f, 0.0f, 0.0f, 1.0f}) != CMP_OK)
    return 1;
  if (f2_typography_scale_set_color(NULL, (CMPColor){1.0f, 0.0f, 0.0f, 1.0f}) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_typography_scale_set_family(&scale, "Arial") != CMP_OK)
    return 1;
  if (f2_typography_scale_set_family(NULL, "Arial") != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_typography_scale_set_family(&scale, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_button(void) {
  F2ButtonStyle style;
  F2Button button;
  CMPTextBackend backend;

  backend.ctx = NULL;
  backend.vtable = NULL;

  if (f2_button_style_init_primary(&style) != CMP_OK)
    return 1;
  if (f2_button_style_init_primary(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_button_style_init_standard(&style) != CMP_OK)
    return 1;
  if (f2_button_style_init_standard(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_button_style_init_outline(&style) != CMP_OK)
    return 1;
  if (f2_button_style_init_outline(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_button_style_init_subtle(&style) != CMP_OK)
    return 1;
  if (f2_button_style_init_subtle(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_button_style_init_transparent(&style) != CMP_OK)
    return 1;
  if (f2_button_style_init_transparent(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_button_init(&button, &backend, &style, "Test", 4) != CMP_OK)
    return 1;
  if (f2_button_init(NULL, &backend, &style, "Test", 4) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_button_init(&button, NULL, &style, "Test", 4) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_button_init(&button, &backend, NULL, "Test", 4) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_button_set_icon(&button, "icon", 4) != CMP_OK)
    return 1;
  if (f2_button_set_icon(NULL, "icon", 4) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_button_set_label(&button, "Label", 5) != CMP_OK)
    return 1;
  if (f2_button_set_label(NULL, "Label", 5) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_button_set_on_click(&button, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_button_set_on_click(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  F2ToggleButton toggle;
  if (f2_toggle_button_init(&toggle, &backend, &style, "Test", 4, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (f2_toggle_button_init(&toggle, NULL, &style, "Test", 4, CMP_FALSE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_toggle_button_set_toggled(&toggle, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_toggle_button_set_toggled(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  F2SplitButton split;
  if (f2_split_button_init(&split, &backend, &style, "Test", 4) != CMP_OK)
    return 1;
  if (f2_split_button_init(NULL, &backend, &style, "Test", 4) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_split_button_init(&split, NULL, &style, "Test", 4) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_split_button_set_dropdown_open(&split, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_split_button_set_dropdown_open(NULL, CMP_TRUE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_selection(void) {
  F2CheckboxStyle cb_style;
  F2Checkbox cb;
  F2SwitchStyle sw_style;
  F2Switch sw;
  F2RadioStyle r_style;
  F2Radio r;

  if (f2_checkbox_style_init(&cb_style) != CMP_OK)
    return 1;
  if (f2_checkbox_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_checkbox_init(&cb, &cb_style, CMP_FALSE) != CMP_OK)
    return 1;
  if (f2_checkbox_init(NULL, &cb_style, CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_checkbox_init(&cb, NULL, CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_checkbox_set_checked(&cb, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_checkbox_set_checked(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_checkbox_set_mixed_state(&cb, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_checkbox_set_mixed_state(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_checkbox_set_on_change(&cb, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_checkbox_set_on_change(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_switch_style_init(&sw_style) != CMP_OK)
    return 1;
  if (f2_switch_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_switch_init(&sw, &sw_style, CMP_FALSE) != CMP_OK)
    return 1;
  if (f2_switch_init(NULL, &sw_style, CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_switch_init(&sw, NULL, CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_switch_set_on(&sw, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_switch_set_on(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_switch_set_on_change(&sw, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_switch_set_on_change(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_radio_style_init(&r_style) != CMP_OK)
    return 1;
  if (f2_radio_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_radio_init(&r, &r_style, CMP_FALSE) != CMP_OK)
    return 1;
  if (f2_radio_init(NULL, &r_style, CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_radio_init(&r, NULL, CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_radio_set_selected(&r, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_radio_set_selected(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_radio_set_on_change(&r, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_radio_set_on_change(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_text_field(void) {
  F2TextFieldStyle style;
  F2TextField tf;
  CMPTextBackend backend;
  CMPTextVTable vtable;
  CMPAllocator allocator;
  int rc;

  vtable.create_font = test_create_font;
  vtable.destroy_font = test_destroy_font;
  vtable.measure_text = test_measure_text;
  vtable.draw_text = test_draw_text;
  vtable.shape_text = test_shape_text;
  vtable.free_layout = test_free_layout;
  vtable.draw_layout = NULL;

  backend.ctx = NULL;
  backend.vtable = &vtable;

  cmp_get_default_allocator(&allocator);

  if (f2_text_field_style_init(&style) != CMP_OK) {
    printf("fail 1\n");
    return 1;
  }
  if (f2_text_field_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 2\n");
    return 1;
  }

  rc = f2_text_field_init(&tf, &backend, &style, &allocator);
  if (rc != CMP_OK) {
    printf("fail 3: %d\n", rc);
    return 1;
  }
  if (f2_text_field_init(NULL, &backend, &style, &allocator) !=
      CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 4\n");
    return 1;
  }
  if (f2_text_field_init(&tf, NULL, &style, &allocator) !=
      CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 5\n");
    return 1;
  }
  if (f2_text_field_init(&tf, &backend, NULL, &allocator) !=
      CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 6\n");
    return 1;
  }
  if (f2_text_field_init(&tf, &backend, &style, NULL) !=
      CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 7\n");
    return 1;
  }

  if (f2_text_field_set_text(&tf, "hello") != CMP_OK) {
    printf("fail 8\n");
    return 1;
  }
  if (f2_text_field_set_text(NULL, "hello") != CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 9\n");
    return 1;
  }

  if (f2_text_field_set_error(&tf, CMP_TRUE) != CMP_OK) {
    printf("fail 10\n");
    return 1;
  }
  if (f2_text_field_set_error(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 11\n");
    return 1;
  }

  if (f2_text_field_set_on_change(&tf, NULL, NULL) != CMP_OK) {
    printf("fail 12\n");
    return 1;
  }
  if (f2_text_field_set_on_change(NULL, NULL, NULL) !=
      CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 13\n");
    return 1;
  }

  return 0;
}

#include "f2/f2_navigation.h"

static int test_f2_navigation(void) {
  F2NavViewStyle nv_style;
  F2NavigationView nv;
  F2TabListStyle t_style;
  F2TabList t;
  F2BreadcrumbStyle b_style;
  F2Breadcrumb b;
  F2NavItem items[2] = {{"A", 1}, {"B", 1}};

  if (f2_navigation_view_style_init(&nv_style) != CMP_OK)
    return 1;
  if (f2_navigation_view_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_navigation_view_init(&nv, &nv_style, items, 2) != CMP_OK)
    return 1;
  if (f2_navigation_view_init(NULL, &nv_style, items, 2) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_navigation_view_init(&nv, NULL, items, 2) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_navigation_view_set_selected(&nv, 1) != CMP_OK)
    return 1;
  if (f2_navigation_view_set_selected(NULL, 1) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_navigation_view_set_on_select(&nv, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_navigation_view_set_on_select(NULL, NULL, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_tab_list_style_init(&t_style) != CMP_OK)
    return 1;
  if (f2_tab_list_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_tab_list_init(&t, &t_style, items, 2) != CMP_OK)
    return 1;
  if (f2_tab_list_init(NULL, &t_style, items, 2) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_tab_list_init(&t, NULL, items, 2) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_tab_list_set_selected(&t, 1) != CMP_OK)
    return 1;
  if (f2_tab_list_set_selected(NULL, 1) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_tab_list_set_on_select(&t, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_tab_list_set_on_select(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_breadcrumb_style_init(&b_style) != CMP_OK)
    return 1;
  if (f2_breadcrumb_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_breadcrumb_init(&b, &b_style, items, 2) != CMP_OK)
    return 1;
  if (f2_breadcrumb_init(NULL, &b_style, items, 2) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_breadcrumb_init(&b, NULL, items, 2) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_breadcrumb_set_on_select(&b, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_breadcrumb_set_on_select(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_command_bar(void) {
  F2CommandBarStyle style;
  F2CommandBar bar;
  F2CommandBarItem items[1] = {{1, "A", "icon", CMP_FALSE}};

  if (f2_command_bar_style_init(&style) != CMP_OK)
    return 1;
  if (f2_command_bar_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_command_bar_init(&bar, &style, items, 1, NULL, 0) != CMP_OK)
    return 1;
  if (f2_command_bar_init(NULL, &style, items, 1, NULL, 0) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_command_bar_init(&bar, NULL, items, 1, NULL, 0) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_command_bar_init(&bar, &style, NULL, 1, NULL, 0) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_command_bar_init(&bar, &style, items, 1, NULL, 1) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_command_bar_set_on_action(&bar, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_command_bar_set_on_action(NULL, NULL, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_data_components(void) {
  F2DataGridStyle dg_style;
  F2DataGrid grid;
  F2ComboBoxStyle cb_style;
  F2ComboBox combo;
  F2DatePickerStyle dp_style;
  F2DatePicker picker;
  F2TreeViewStyle tv_style;
  F2TreeView tree;
  CMPTextBackend backend;
  CMPTextVTable vtable;
  CMPAllocator allocator;

  vtable.create_font = test_create_font;
  vtable.destroy_font = test_destroy_font;
  vtable.measure_text = test_measure_text;
  vtable.draw_text = test_draw_text;
  vtable.shape_text = test_shape_text;
  vtable.free_layout = test_free_layout;
  vtable.draw_layout = NULL;

  backend.ctx = NULL;
  backend.vtable = &vtable;

  cmp_get_default_allocator(&allocator);

  /* DataGrid */
  if (f2_data_grid_style_init(&dg_style) != CMP_OK)
    return 1;
  if (f2_data_grid_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_data_grid_init(&grid, &dg_style, 3, 5) != CMP_OK)
    return 1;
  if (f2_data_grid_init(NULL, &dg_style, 3, 5) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_data_grid_init(&grid, NULL, 3, 5) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_data_grid_set_selected(&grid, 1) != CMP_OK)
    return 1;
  if (f2_data_grid_set_selected(NULL, 1) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_data_grid_set_on_select(&grid, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_data_grid_set_on_select(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  /* ComboBox */
  if (f2_combo_box_style_init(&cb_style) != CMP_OK)
    return 1;
  if (f2_combo_box_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_combo_box_init(&combo, &backend, &cb_style, &allocator, 5) != CMP_OK)
    return 1;
  if (f2_combo_box_init(NULL, &backend, &cb_style, &allocator, 5) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_combo_box_init(&combo, NULL, &cb_style, &allocator, 5) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_combo_box_init(&combo, &backend, NULL, &allocator, 5) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_combo_box_init(&combo, &backend, &cb_style, NULL, 5) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_combo_box_set_selected(&combo, 1) != CMP_OK)
    return 1;
  if (f2_combo_box_set_selected(NULL, 1) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_combo_box_set_open(&combo, CMP_TRUE) != CMP_OK)
    return 1;
  if (f2_combo_box_set_open(NULL, CMP_TRUE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_combo_box_set_on_select(&combo, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_combo_box_set_on_select(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_combo_box_cleanup(&combo) != CMP_OK)
    return 1;
  if (f2_combo_box_cleanup(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  /* DatePicker */
  if (f2_date_picker_style_init(&dp_style) != CMP_OK)
    return 1;
  if (f2_date_picker_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_date_picker_init(&picker, &dp_style) != CMP_OK)
    return 1;
  if (f2_date_picker_init(NULL, &dp_style) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_date_picker_init(&picker, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_date_picker_set_date(&picker, 2024, 5, 20) != CMP_OK)
    return 1;
  if (f2_date_picker_set_date(NULL, 2024, 5, 20) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_date_picker_set_on_select(&picker, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_date_picker_set_on_select(NULL, NULL, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  /* TreeView */
  if (f2_tree_view_style_init(&tv_style) != CMP_OK)
    return 1;
  if (f2_tree_view_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_tree_view_init(&tree, &tv_style) != CMP_OK)
    return 1;
  if (f2_tree_view_init(NULL, &tv_style) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_tree_view_init(&tree, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_tree_view_set_selected(&tree, 1) != CMP_OK)
    return 1;
  if (f2_tree_view_set_selected(NULL, 1) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_tree_view_set_callbacks(&tree, NULL, NULL, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_tree_view_set_callbacks(NULL, NULL, NULL, NULL, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

#include "f2/f2_avatar.h"
#include "f2/f2_message_bar.h"

static int test_f2_message_bar(void) {
  F2Theme theme;
  F2MessageBarStyle style;
  F2MessageBar bar;

  f2_theme_generate(0xFF0000FF, CMP_FALSE, &theme);

  if (f2_message_bar_style_init(&style, F2_INTENT_INFO, &theme) != CMP_OK)
    return 1;
  if (f2_message_bar_style_init(NULL, F2_INTENT_INFO, &theme) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_message_bar_style_init(&style, F2_INTENT_INFO, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_message_bar_init(&bar, &style, "Test", 4) != CMP_OK)
    return 1;
  if (f2_message_bar_init(NULL, &style, "Test", 4) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_message_bar_init(&bar, NULL, "Test", 4) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_message_bar_show(&bar) != CMP_OK)
    return 1;
  if (f2_message_bar_show(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_message_bar_hide(&bar) != CMP_OK)
    return 1;
  if (f2_message_bar_hide(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_message_bar_set_action(&bar, "Act", 3, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_message_bar_set_action(NULL, "Act", 3, NULL, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_message_bar_set_dismiss(&bar, NULL, NULL) != CMP_OK)
    return 1;
  if (f2_message_bar_set_dismiss(NULL, NULL, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_f2_avatar(void) {
  F2AvatarStyle style;
  F2Avatar avatar;
  CMPHandle image = {1, 1};

  if (f2_avatar_style_init(&style) != CMP_OK)
    return 1;
  if (f2_avatar_style_init(NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_avatar_init_initials(&avatar, &style, "AB", 2) != CMP_OK)
    return 1;
  if (f2_avatar_init_initials(NULL, &style, "AB", 2) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_avatar_init_initials(&avatar, NULL, "AB", 2) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_avatar_init_image(&avatar, &style, image) != CMP_OK)
    return 1;
  if (f2_avatar_init_image(NULL, &style, image) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (f2_avatar_init_image(&avatar, NULL, image) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (f2_avatar_set_presence(&avatar, F2_PRESENCE_AVAILABLE, CMP_TRUE) !=
      CMP_OK)
    return 1;
  if (f2_avatar_set_presence(NULL, F2_PRESENCE_AVAILABLE, CMP_TRUE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

int main(void) {
  if (test_f2_color() != 0) {
    printf("f2_color tests failed\n");
    return 1;
  }
  if (test_f2_typography() != 0) {
    printf("f2_typography tests failed\n");
    return 1;
  }
  if (test_f2_button() != 0) {
    printf("f2_button tests failed\n");
    return 1;
  }
  if (test_f2_selection() != 0) {
    printf("f2_selection tests failed\n");
    return 1;
  }
  if (test_f2_text_field() != 0) {
    printf("f2_text_field tests failed\n");
    return 1;
  }
  if (test_f2_slider() != 0) {
    printf("f2_slider tests failed\n");
    return 1;
  }
  if (test_f2_progress() != 0) {
    printf("f2_progress tests failed\n");
    return 1;
  }
  if (test_f2_badge() != 0) {
    printf("f2_badge tests failed\n");
    return 1;
  }
  if (test_f2_link() != 0) {
    printf("f2_link tests failed\n");
    return 1;
  }
  if (test_f2_card() != 0) {
    printf("f2_card tests failed\n");
    return 1;
  }
  if (test_f2_flyout() != 0) {
    printf("f2_flyout tests failed\n");
    return 1;
  }
  if (test_f2_dialog() != 0) {
    printf("f2_dialog tests failed\n");
    return 1;
  }
  if (test_f2_tooltip() != 0) {
    printf("f2_tooltip tests failed\n");
    return 1;
  }
  if (test_f2_spin_button() != 0) {
    printf("f2_spin_button tests failed\n");
    return 1;
  }
  if (test_f2_searchbox() != 0) {
    printf("f2_searchbox tests failed\n");
    return 1;
  }
  if (test_f2_navigation() != 0) {
    printf("f2_navigation tests failed\n");
    return 1;
  }
  if (test_f2_command_bar() != 0) {
    printf("f2_command_bar tests failed\n");
    return 1;
  }
  if (test_f2_data_components() != 0) {
    printf("f2_data_components tests failed\n");
    return 1;
  }
  if (test_f2_message_bar() != 0) {
    printf("f2_message_bar tests failed\n");
    return 1;
  }
  if (test_f2_avatar() != 0) {
    printf("f2_avatar tests failed\n");
    return 1;
  }
  printf("All F2 tests passed!\n");
  return 0;
}
