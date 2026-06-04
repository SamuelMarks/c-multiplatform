/* clang-format off */
#include "cmp_example_app.h"
#include "cmp.h"
#include "cmp_ui_card.h"
#include "cmp_ui_divider.h"
#include "cmp_ui_action_button.h"
#include "cmp_ui_fab.h"
#include "cmp_ui_segmented_button.h"
#include "cmp_ui_virtual_list.h"
#include "cmp_ui_snackbar.h"
#include "themes/cmp_f2_button.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static void on_btn_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  cmp_ui_snackbar_t *sb;
  cmp_ui_node_t *sb_node;
  const char *msg = (const char *)ctx;
  int rc;

  (void)node;

  if (evt && evt->action != CMP_ACTION_UP) {
    return;
  }

  /* Display snackbar */
  rc = cmp_ui_snackbar_create(&sb, msg ? msg : "Button Clicked", NULL);
  if (rc == CMP_SUCCESS) {
    (void)cmp_ui_snackbar_get_node(sb, &sb_node);
    /* In a real app we'd add this to an overlay layer */
    /* cmp_ui_overlay_add(sb_node); */
    printf("Snackbar: %s\n", msg ? msg : "Button Clicked");
  }
}

static int build_m3_buttons(cmp_ui_node_t *parent) {
  int rc;
  cmp_ui_card_t *card = NULL;
  cmp_ui_node_t *card_node = NULL;
  cmp_ui_node_t *card_box = NULL;
  cmp_ui_node_t *row = NULL;
  cmp_ui_action_button_t *ab = NULL;
  cmp_ui_node_t *ab_node = NULL;
  cmp_ui_divider_t *div = NULL;
  cmp_ui_node_t *div_node = NULL;

  /* Card wrapper */
  rc = cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_ELEVATED);
  if (rc != CMP_SUCCESS)
    return rc;
  (void)cmp_ui_card_get_node(card, &card_node);
  card_node->layout->margin[0] = 16.0f;
  card_node->layout->margin[1] = 16.0f;
  card_node->layout->margin[2] = 16.0f;
  card_node->layout->margin[3] = 16.0f;
  card_node->layout->padding[0] = 16.0f;
  card_node->layout->padding[1] = 16.0f;
  card_node->layout->padding[2] = 16.0f;
  card_node->layout->padding[3] = 16.0f;
  card_node->layout->width = 800.0f;
  card_node->layout->flex_shrink = 0.0f;

  (void)cmp_ui_box_create(&card_box);
  card_box->layout->direction = CMP_FLEX_COLUMN;
  card_box->layout->row_gap = 16.0f;
  card_box->layout->width = 768.0f;
  (void)cmp_ui_node_add_child(card_node, card_box);
  /* Title */
  {
    cmp_ui_node_t *title = NULL;
    (void)cmp_ui_text_create(&title, "Material 3 Buttons", -1);
    title->font_size = 20.0f;
    title->text_color = 0xFF000000;
    title->layout->width = 300.0f;
    title->layout->height = 24.0f;
    title->layout->flex_shrink = 0.0f;
    (void)cmp_ui_node_add_child(card_box, title);
  }

  /* Row 1 */
  /* Row */
  (void)cmp_ui_box_create(&row);
  row->layout->direction = CMP_FLEX_ROW;
  row->layout->column_gap = 16.0f;
  row->layout->width = 768.0f;
  row->layout->height = 48.0f;
  row->layout->flex_shrink = 0.0f;
  (void)cmp_ui_node_add_child(card_box, row);
  cmp_ui_action_button_create(&ab, "Elevated",
                              CMP_UI_ACTION_BUTTON_STYLE_ELEVATED);
  (void)cmp_ui_action_button_get_node(ab, &ab_node);
  cmp_ui_node_add_event_listener(ab_node, CMP_EVENT_TYPE_MOUSE, 0, on_btn_click,
                                 "M3 Elevated Clicked");
  (void)cmp_ui_node_add_child(row, ab_node);
  (void)cmp_ui_action_button_create(&ab, "Filled",
                                    CMP_UI_ACTION_BUTTON_STYLE_FILLED);
  (void)cmp_ui_action_button_get_node(ab, &ab_node);
  cmp_ui_node_add_event_listener(ab_node, CMP_EVENT_TYPE_MOUSE, 0, on_btn_click,
                                 "M3 Filled Clicked");
  (void)cmp_ui_node_add_child(row, ab_node);
  (void)cmp_ui_action_button_create(&ab, "Tonal",
                                    CMP_UI_ACTION_BUTTON_STYLE_TONAL);
  (void)cmp_ui_action_button_get_node(ab, &ab_node);
  cmp_ui_node_add_event_listener(ab_node, CMP_EVENT_TYPE_MOUSE, 0, on_btn_click,
                                 "M3 Tonal Clicked");
  (void)cmp_ui_node_add_child(row, ab_node);
  /* Divider */
  (void)cmp_ui_divider_create(&div);
  (void)cmp_ui_divider_get_node(div, &div_node);
  div_node->layout->height = 1.0f;
  div_node->layout->width = -1.0f;
  div_node->bg_color = 0xFF000000;
  (void)cmp_ui_node_add_child(card_box, div_node);
  /* Row 2 */
  /* Row */
  (void)cmp_ui_box_create(&row);
  row->layout->direction = CMP_FLEX_ROW;
  row->layout->column_gap = 16.0f;
  row->layout->width = 768.0f;
  row->layout->height = 48.0f;
  row->layout->flex_shrink = 0.0f;
  (void)cmp_ui_node_add_child(card_box, row);
  cmp_ui_action_button_create(&ab, "Outlined",
                              CMP_UI_ACTION_BUTTON_STYLE_OUTLINED);
  (void)cmp_ui_action_button_get_node(ab, &ab_node);
  cmp_ui_node_add_event_listener(ab_node, CMP_EVENT_TYPE_MOUSE, 0, on_btn_click,
                                 "M3 Outlined Clicked");
  (void)cmp_ui_node_add_child(row, ab_node);
  (void)cmp_ui_action_button_create(&ab, "Text",
                                    CMP_UI_ACTION_BUTTON_STYLE_TEXT);
  (void)cmp_ui_action_button_get_node(ab, &ab_node);
  cmp_ui_node_add_event_listener(ab_node, CMP_EVENT_TYPE_MOUSE, 0, on_btn_click,
                                 "M3 Text Clicked");
  (void)cmp_ui_node_add_child(row, ab_node);
  (void)cmp_ui_node_add_child(parent, card_node);
  return CMP_SUCCESS;
}

static int build_f2_buttons(cmp_ui_node_t *parent) {
  int rc;
  cmp_ui_card_t *card = NULL;
  cmp_ui_node_t *card_node = NULL;
  cmp_ui_node_t *card_box = NULL;
  cmp_ui_node_t *row = NULL;
  cmp_ui_node_t *btn_node = NULL;

  rc = cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_ELEVATED);
  if (rc != CMP_SUCCESS)
    return rc;
  (void)cmp_ui_card_get_node(card, &card_node);
  card_node->layout->margin[0] = 16.0f;
  card_node->layout->margin[1] = 16.0f;
  card_node->layout->margin[2] = 16.0f;
  card_node->layout->margin[3] = 16.0f;
  card_node->layout->padding[0] = 16.0f;
  card_node->layout->padding[1] = 16.0f;
  card_node->layout->padding[2] = 16.0f;
  card_node->layout->padding[3] = 16.0f;
  card_node->layout->width = 800.0f;
  card_node->layout->height = 200.0f;
  card_node->layout->flex_shrink = 0.0f;

  (void)cmp_ui_box_create(&card_box);
  card_box->layout->direction = CMP_FLEX_COLUMN;
  card_box->layout->row_gap = 16.0f;
  card_box->layout->width = 768.0f;
  card_box->layout->height = 168.0f;
  card_box->layout->align_items = CMP_FLEX_ALIGN_START;
  (void)cmp_ui_node_add_child(card_node, card_box);
  {
    cmp_ui_node_t *title = NULL;
    (void)cmp_ui_text_create(&title, "Fluent 2 Buttons", -1);
    title->font_size = 20.0f;
    title->text_color = 0xFF000000;
    title->layout->width = -1.0f;
    title->layout->height = 24.0f;
    title->layout->flex_shrink = 1.0f;
    (void)cmp_ui_node_add_child(card_box, title);
  }

  /* Row */
  (void)cmp_ui_box_create(&row);
  row->layout->direction = CMP_FLEX_ROW;
  row->layout->column_gap = 16.0f;
  row->layout->width = 768.0f;
  row->layout->height = 48.0f;
  row->layout->flex_shrink = 0.0f;
  (void)cmp_ui_node_add_child(card_box, row);
  (void)cmp_f2_button_create(&btn_node, "Secondary", NULL);
  (void)cmp_f2_button_set_variant(btn_node, CMP_F2_BUTTON_VARIANT_SECONDARY);
  cmp_ui_node_add_event_listener(btn_node, CMP_EVENT_TYPE_MOUSE, 0,
                                 on_btn_click, "F2 Secondary Clicked");
  (void)cmp_ui_node_add_child(row, btn_node);
  (void)cmp_f2_button_create(&btn_node, "Primary", NULL);
  (void)cmp_f2_button_set_variant(btn_node, CMP_F2_BUTTON_VARIANT_PRIMARY);
  cmp_ui_node_add_event_listener(btn_node, CMP_EVENT_TYPE_MOUSE, 0,
                                 on_btn_click, "F2 Primary Clicked");
  (void)cmp_ui_node_add_child(row, btn_node);
  (void)cmp_f2_button_create(&btn_node, "Subtle", NULL);
  (void)cmp_f2_button_set_variant(btn_node, CMP_F2_BUTTON_VARIANT_SUBTLE);
  cmp_ui_node_add_event_listener(btn_node, CMP_EVENT_TYPE_MOUSE, 0,
                                 on_btn_click, "F2 Subtle Clicked");
  (void)cmp_ui_node_add_child(row, btn_node);
  /* Row */
  (void)cmp_ui_box_create(&row);
  row->layout->direction = CMP_FLEX_ROW;
  row->layout->column_gap = 16.0f;
  row->layout->width = 768.0f;
  row->layout->height = 48.0f;
  row->layout->flex_shrink = 0.0f;
  (void)cmp_ui_node_add_child(card_box, row);
  (void)cmp_f2_button_create(&btn_node, "Outline", NULL);
  (void)cmp_f2_button_set_variant(btn_node, CMP_F2_BUTTON_VARIANT_OUTLINE);
  cmp_ui_node_add_event_listener(btn_node, CMP_EVENT_TYPE_MOUSE, 0,
                                 on_btn_click, "F2 Outline Clicked");
  (void)cmp_ui_node_add_child(row, btn_node);
  (void)cmp_f2_button_create(&btn_node, "Transparent", NULL);
  (void)cmp_f2_button_set_variant(btn_node, CMP_F2_BUTTON_VARIANT_TRANSPARENT);
  cmp_ui_node_add_event_listener(btn_node, CMP_EVENT_TYPE_MOUSE, 0,
                                 on_btn_click, "F2 Transparent Clicked");
  (void)cmp_ui_node_add_child(row, btn_node);
  (void)cmp_ui_node_add_child(parent, card_node);
  return CMP_SUCCESS;
}

static int build_cu_buttons(cmp_ui_node_t *parent) {
  int rc;
  cmp_ui_card_t *card = NULL;
  cmp_ui_node_t *card_node = NULL;
  cmp_ui_node_t *card_box = NULL;
  cmp_ui_node_t *row = NULL;
  cmp_ui_node_t *btn_node = NULL;

  rc = cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_ELEVATED);
  if (rc != CMP_SUCCESS)
    return rc;
  (void)cmp_ui_card_get_node(card, &card_node);
  card_node->layout->margin[0] = 16.0f;
  card_node->layout->margin[1] = 16.0f;
  card_node->layout->margin[2] = 16.0f;
  card_node->layout->margin[3] = 16.0f;
  card_node->layout->padding[0] = 16.0f;
  card_node->layout->padding[1] = 16.0f;
  card_node->layout->padding[2] = 16.0f;
  card_node->layout->padding[3] = 16.0f;
  card_node->layout->width = 800.0f;
  card_node->layout->height = 200.0f;
  card_node->layout->flex_shrink = 0.0f;

  (void)cmp_ui_box_create(&card_box);
  card_box->layout->direction = CMP_FLEX_COLUMN;
  card_box->layout->row_gap = 16.0f;
  card_box->layout->width = 768.0f;
  card_box->layout->height = 168.0f;
  card_box->layout->align_items = CMP_FLEX_ALIGN_START;
  (void)cmp_ui_node_add_child(card_node, card_box);
  {
    cmp_ui_node_t *title = NULL;
    (void)cmp_ui_text_create(&title, "Cupertino Buttons (Generic)", -1);
    title->font_size = 20.0f;
    title->text_color = 0xFF000000;
    title->layout->width = -1.0f;
    title->layout->height = 24.0f;
    title->layout->flex_shrink = 0.0f;
    (void)cmp_ui_node_add_child(card_box, title);
  }

  /* Row */
  (void)cmp_ui_box_create(&row);
  row->layout->direction = CMP_FLEX_ROW;
  row->layout->column_gap = 16.0f;
  row->layout->width = 768.0f;
  row->layout->height = 48.0f;
  row->layout->flex_shrink = 0.0f;
  (void)cmp_ui_node_add_child(card_box, row);
  (void)cmp_ui_button_create(&btn_node, "Plain Button", 0);
  {
    size_t len = strlen("Plain Button");
    btn_node->layout->width = (float)len * 7.0f + 32.0f;
    btn_node->layout->height = 40.0f;
    btn_node->bg_color = 0x00000000;
    btn_node->text_color = 0xFF007AFF;
    btn_node->border_radius = 8.0f;
    btn_node->design_language_override = 3;
    btn_node->hover_opacity = 1.0f;
    btn_node->press_opacity = 0.3f;
  }
  cmp_ui_node_add_event_listener(btn_node, CMP_EVENT_TYPE_MOUSE, 0,
                                 on_btn_click, "Cupertino Plain Clicked");
  (void)cmp_ui_node_add_child(row, btn_node);
  (void)cmp_ui_button_create(&btn_node, "Tinted Button", 0);
  {
    size_t len = strlen("Tinted Button");
    btn_node->layout->width = (float)len * 7.0f + 32.0f;
    btn_node->layout->height = 40.0f;
    btn_node->bg_color = 0x33007AFF;
    btn_node->text_color = 0xFF007AFF;
    btn_node->border_radius = 8.0f;
    btn_node->design_language_override = 3;
    btn_node->hover_opacity = 0.8f;
    btn_node->press_opacity = 0.6f;
  }
  cmp_ui_node_add_event_listener(btn_node, CMP_EVENT_TYPE_MOUSE, 0,
                                 on_btn_click, "Cupertino Tinted Clicked");
  (void)cmp_ui_node_add_child(row, btn_node);
  (void)cmp_ui_node_add_child(parent, card_node);
  return CMP_SUCCESS;
}

static int build_body_ui(cmp_example_design_system_t design_system,
                         cmp_ui_node_t *body_container) {
  int rc = 0;
  cmp_ui_node_t *scroll_box = NULL;

  /* Use a box as scroll view placeholder */
  (void)cmp_ui_box_create(&scroll_box);
  scroll_box->layout->flex_grow = 1.0f;
  scroll_box->layout->width = -1.0f;
  scroll_box->layout->height = -1.0f;
  scroll_box->layout->direction = CMP_FLEX_COLUMN;
  scroll_box->layout->align_items = CMP_FLEX_ALIGN_STRETCH;
  scroll_box->layout->padding[0] = 16.0f;
  scroll_box->layout->padding[1] = 16.0f;
  scroll_box->layout->padding[2] = 16.0f;
  scroll_box->layout->padding[3] = 16.0f;
  scroll_box->layout->row_gap = 24.0f;
  scroll_box->bg_color = 0xFFF3F3F3;

  if (design_system == CMP_EXAMPLE_DESIGN_MATERIAL3) {
    build_m3_buttons(scroll_box);
  } else if (design_system == CMP_EXAMPLE_DESIGN_FLUENT2) {
    build_f2_buttons(scroll_box);
  } else if (design_system == CMP_EXAMPLE_DESIGN_CUPERTINO) {
    build_cu_buttons(scroll_box);
  } else {
    build_m3_buttons(scroll_box);
    build_f2_buttons(scroll_box);
    build_cu_buttons(scroll_box);
  }

  (void)cmp_ui_node_add_child(body_container, scroll_box);
  if (rc != 0) {
    return rc;
  }
  return rc;
}

int main(int argc, char **argv) {
  cmp_example_app_callbacks_t callbacks;
  int rc;

  (void)argc;
  (void)argv;

  callbacks.build_body_ui = build_body_ui;

  rc = cmp_example_app_run("Multi-Design Buttons", &callbacks);
  if (rc != CMP_SUCCESS) {
    fprintf(stderr, "App failed with %d\n", rc);
    return 1;
  }
  return 0;
}
