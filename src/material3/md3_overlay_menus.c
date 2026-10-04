/**
 * @file md3_overlay_menus.c
 * @brief Material 3 & Expressive Overlay, Dialog, Menu, and Picker Components
 * implementation.
 */

/* clang-format off */
#include "material3/md3_overlay_menus.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Global flag to simulate failures in mocked dependencies. */
int g_md3_overlay_menus_mock_fail = 0;

/**
 * @brief Mock for ui_date_range_picker_base_create.
 * @param out Output date range picker pointer.
 * @brief Mock for ui_hover_card_base_create.
 * @param out Output hover card pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_hover_card_base_create(struct ui_hover_card_base **out) {
  if (g_md3_overlay_menus_mock_fail == 7) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_hover_card_base_create(out);
}

/**
 * @brief Mock for ui_hover_card_base_destroy.
 * @param b Hover card pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_hover_card_base_destroy(struct ui_hover_card_base *b) {
  if (g_md3_overlay_menus_mock_fail == 8) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_hover_card_base_destroy(b);
}

/**
 * @brief Mock for ui_hover_card_base_on_mouse_enter.
 * @param b Hover card pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_hover_card_base_on_mouse_enter(struct ui_hover_card_base *b) {
  if (g_md3_overlay_menus_mock_fail == 9) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_hover_card_base_on_mouse_enter(b);
}

/**
 * @brief Mock for ui_hover_card_base_on_mouse_leave.
 * @param b Hover card pointer.
 * @param x Mouse X coordinate.
 * @param y Mouse Y coordinate.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_hover_card_base_on_mouse_leave(struct ui_hover_card_base *b, float x,
                                    float y) {
  if (g_md3_overlay_menus_mock_fail == 10) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_hover_card_base_on_mouse_leave(b, x, y);
}

/**
 * @brief Mock for ui_popover_base_create.
 * @param out Output popover pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_popover_base_create(struct ui_popover_base **out) {
  if (g_md3_overlay_menus_mock_fail == 11) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_popover_base_create(out);
}

/**
 * @brief Mock for ui_popover_base_destroy.
 * @param b Popover pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_popover_base_destroy(struct ui_popover_base *b) {
  if (g_md3_overlay_menus_mock_fail == 12) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_popover_base_destroy(b);
}

/**
 * @brief Mock for ui_popover_base_close.
 * @param b Popover pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_popover_base_close(struct ui_popover_base *b) {
  if (g_md3_overlay_menus_mock_fail == 13) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_popover_base_close(b);
}

/**
 * @brief Mock for ui_banner_base_create.
 * @param out Output banner pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_banner_base_create(struct ui_banner_base **out) {
  if (g_md3_overlay_menus_mock_fail == 14) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_banner_base_create(out);
}

/**
 * @brief Mock for ui_banner_base_destroy.
 * @param b Banner pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_banner_base_destroy(struct ui_banner_base *b) {
  if (g_md3_overlay_menus_mock_fail == 15) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_banner_base_destroy(b);
}

/**
 * @brief Mock for ui_banner_base_set_text.
 * @param b Banner pointer.
 * @param t Text string.
 * @return ui_error_t result code.
 */
static ui_error_t mock_banner_base_set_text(struct ui_banner_base *b,
                                            const char *t) {
  if (g_md3_overlay_menus_mock_fail == 16) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_banner_base_set_text(b, t);
}

/**
 * @brief Mock for ui_banner_base_set_open.
 * @param b Banner pointer.
 * @param o Open state.
 * @return ui_error_t result code.
 */
static ui_error_t mock_banner_base_set_open(struct ui_banner_base *b, int o) {
  if (g_md3_overlay_menus_mock_fail == 17) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_banner_base_set_open(b, o);
}

/**
 * @brief Mock for ui_alert_base_create.
 * @param out Output alert pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_alert_base_create(struct ui_alert_base **out) {
  if (g_md3_overlay_menus_mock_fail == 18) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_alert_base_create(out);
}
/**
 * @brief Mock for ui_alert_base_set_role.
 * @param b Alert pointer.
 * @param r Alert role.
 * @return ui_error_t result code.
 */
static ui_error_t mock_alert_base_set_role(struct ui_alert_base *b,
                                           enum ui_alert_role r) {
  if (g_md3_overlay_menus_mock_fail == 19) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_alert_base_set_role(b, r);
}

/**
 * @brief Mock for ui_alert_base_destroy.
 * @param b Alert pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_alert_base_destroy(struct ui_alert_base *b) {
  if (g_md3_overlay_menus_mock_fail == 20) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_alert_base_destroy(b);
}

/**
 * @brief Mock for ui_alert_base_set_dismissible.
 * @param b Alert pointer.
 * @param d Dismissible flag.
 * @return ui_error_t result code.
 */
static ui_error_t mock_alert_base_set_dismissible(struct ui_alert_base *b,
                                                  int d) {
  if (g_md3_overlay_menus_mock_fail == 21) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_alert_base_set_dismissible(b, d);
}

/**
 * @brief Mock for ui_menubar_base_create.
 * @param out Output menubar pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_menubar_base_create(struct ui_menubar_base **out) {
  if (g_md3_overlay_menus_mock_fail == 22) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_menubar_base_create(out);
}

/**
 * @brief Mock for ui_component_destroy.
 * @param c Component pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_component_destroy(struct ui_component *c) {
  if (g_md3_overlay_menus_mock_fail == 23) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(c);
}

/**
 * @brief Mock for ui_dom_node_create.
 * @param t Node type.
 * @param out Output DOM node pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_dom_node_create(enum ui_dom_node_type t,
                                       struct ui_dom_node **out) {
  if (g_md3_overlay_menus_mock_fail == 24) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_create(t, out);
}

/**
 * @brief Mock for ui_menubar_base_append_item.
 * @param b Menubar pointer.
 * @param i Item component.
 * @return ui_error_t result code.
 */
static ui_error_t mock_menubar_base_append_item(struct ui_menubar_base *b,
                                                struct ui_component *i) {
  if (g_md3_overlay_menus_mock_fail == 25) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_menubar_base_append_item(b, i);
}

/**
 * @brief Mock for ui_context_menu_base_create.
 * @param out Output context menu pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_context_menu_base_create(struct ui_context_menu_base **out) {
  if (g_md3_overlay_menus_mock_fail == 26) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_context_menu_base_create(out);
}

/**
 * @brief Mock for ui_context_menu_base_destroy.
 * @param b Context menu pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_context_menu_base_destroy(struct ui_context_menu_base *b) {
  if (g_md3_overlay_menus_mock_fail == 27) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_context_menu_base_destroy(b);
}

/**
 * @brief Mock for ui_action_sheet_base_create.
 * @param out Output action sheet pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_action_sheet_base_create(struct ui_action_sheet_base **out) {
  if (g_md3_overlay_menus_mock_fail == 28) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_action_sheet_base_create(out);
}

/**
 * @brief Mock for ui_action_sheet_base_destroy.
 * @param b Action sheet pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_action_sheet_base_destroy(struct ui_action_sheet_base *b) {
  if (g_md3_overlay_menus_mock_fail == 29) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_action_sheet_base_destroy(b);
}

/**
 * @brief Mock for ui_action_sheet_base_set_open.
 * @param b Action sheet pointer.
 * @param o Open state.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_action_sheet_base_set_open(struct ui_action_sheet_base *b, int o) {
  if (g_md3_overlay_menus_mock_fail == 30) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_action_sheet_base_set_open(b, o);
}

/**
 * @brief Mock for ui_color_picker_base_create.
 * @param out Output color picker pointer.
 * @param cva Control value accessor.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_color_picker_base_create(struct ui_color_picker_base **out,
                              struct ui_control_value_accessor *cva) {
  if (g_md3_overlay_menus_mock_fail == 31) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_color_picker_base_create(out, cva);
}

/**
 * @brief Mock for ui_color_picker_base_destroy.
 * @param b Color picker pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_color_picker_base_destroy(struct ui_color_picker_base *b) {
  if (g_md3_overlay_menus_mock_fail == 32) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_color_picker_base_destroy(b);
}

/**
 * @brief Mock for ui_color_picker_hsv_to_rgb.
 * @param hsv Input HSV color.
 * @param rgb Output RGB color.
 * @return ui_error_t result code.
 */
static ui_error_t mock_color_picker_hsv_to_rgb(const struct ui_color_hsv *hsv,
                                               struct ui_color_rgb *rgb) {
  if (g_md3_overlay_menus_mock_fail == 33) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_color_picker_hsv_to_rgb(hsv, rgb);
}

/**
 * @brief Mock for ui_color_picker_rgb_to_hex.
 * @param rgb Input RGB color.
 * @param out Output hex buffer.
 * @param sz Size of hex buffer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_color_picker_rgb_to_hex(const struct ui_color_rgb *rgb,
                                               char *out, size_t sz) {
  if (g_md3_overlay_menus_mock_fail == 34) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_color_picker_rgb_to_hex(rgb, out, sz);
}

/**
 * @brief Mock for ui_color_picker_rgb_to_hsv.
 * @param rgb Input RGB color.
 * @param hsv Output HSV color.
 * @return ui_error_t result code.
 */
static ui_error_t mock_color_picker_rgb_to_hsv(const struct ui_color_rgb *rgb,
                                               struct ui_color_hsv *hsv) {
  if (g_md3_overlay_menus_mock_fail == 35) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_color_picker_rgb_to_hsv(rgb, hsv);
}

/**
 * @brief Mock for ui_color_picker_hex_to_rgb.
 * @param hex Input hex string.
 * @param rgb Output RGB color.
 * @return ui_error_t result code.
 */
static ui_error_t mock_color_picker_hex_to_rgb(const char *hex,
                                               struct ui_color_rgb *rgb) {
  if (g_md3_overlay_menus_mock_fail == 36) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_color_picker_hex_to_rgb(hex, rgb);
}

#undef ui_date_range_picker_base_create
#define ui_date_range_picker_base_create mock_date_range_picker_base_create
#undef ui_date_range_picker_base_destroy
#define ui_date_range_picker_base_destroy mock_date_range_picker_base_destroy
#undef ui_date_range_picker_base_select_date
#define ui_date_range_picker_base_select_date                                  \
  mock_date_range_picker_base_select_date
#undef ui_date_range_picker_base_get_range
#define ui_date_range_picker_base_get_range                                    \
  mock_date_range_picker_base_get_range
#undef ui_date_range_picker_base_set_hover_date
#define ui_date_range_picker_base_set_hover_date                               \
  mock_date_range_picker_base_set_hover_date
#undef ui_date_range_picker_base_clear
#define ui_date_range_picker_base_clear mock_date_range_picker_base_clear
#undef ui_hover_card_base_create
#define ui_hover_card_base_create mock_hover_card_base_create
#undef ui_hover_card_base_destroy
#define ui_hover_card_base_destroy mock_hover_card_base_destroy
#undef ui_hover_card_base_on_mouse_enter
#define ui_hover_card_base_on_mouse_enter mock_hover_card_base_on_mouse_enter
#undef ui_hover_card_base_on_mouse_leave
#define ui_hover_card_base_on_mouse_leave mock_hover_card_base_on_mouse_leave
#undef ui_popover_base_create
#define ui_popover_base_create mock_popover_base_create
#undef ui_popover_base_destroy
#define ui_popover_base_destroy mock_popover_base_destroy
#undef ui_popover_base_close
#define ui_popover_base_close mock_popover_base_close
#undef ui_banner_base_create
#define ui_banner_base_create mock_banner_base_create
#undef ui_banner_base_destroy
#define ui_banner_base_destroy mock_banner_base_destroy
#undef ui_banner_base_set_text
#define ui_banner_base_set_text mock_banner_base_set_text
#undef ui_banner_base_set_open
#define ui_banner_base_set_open mock_banner_base_set_open
#undef ui_alert_base_create
#define ui_alert_base_create mock_alert_base_create
#undef ui_alert_base_set_role
#define ui_alert_base_set_role mock_alert_base_set_role
#undef ui_alert_base_destroy
#define ui_alert_base_destroy mock_alert_base_destroy
#undef ui_alert_base_set_dismissible
#define ui_alert_base_set_dismissible mock_alert_base_set_dismissible
#undef ui_menubar_base_create
#define ui_menubar_base_create mock_menubar_base_create
#undef ui_component_destroy
#define ui_component_destroy mock_component_destroy
#undef ui_dom_node_create
#define ui_dom_node_create mock_dom_node_create
#undef ui_menubar_base_append_item
#define ui_menubar_base_append_item mock_menubar_base_append_item
#undef ui_context_menu_base_create
#define ui_context_menu_base_create mock_context_menu_base_create
#undef ui_context_menu_base_destroy
#define ui_context_menu_base_destroy mock_context_menu_base_destroy
#undef ui_action_sheet_base_create
#define ui_action_sheet_base_create mock_action_sheet_base_create
#undef ui_action_sheet_base_destroy
#define ui_action_sheet_base_destroy mock_action_sheet_base_destroy
#undef ui_action_sheet_base_set_open
#define ui_action_sheet_base_set_open mock_action_sheet_base_set_open
#undef ui_color_picker_base_create
#define ui_color_picker_base_create mock_color_picker_base_create
#undef ui_color_picker_base_destroy
#define ui_color_picker_base_destroy mock_color_picker_base_destroy
#undef ui_color_picker_hsv_to_rgb
#define ui_color_picker_hsv_to_rgb mock_color_picker_hsv_to_rgb
#undef ui_color_picker_rgb_to_hex
#define ui_color_picker_rgb_to_hex mock_color_picker_rgb_to_hex
#undef ui_color_picker_rgb_to_hsv
#define ui_color_picker_rgb_to_hsv mock_color_picker_rgb_to_hsv
#undef ui_color_picker_hex_to_rgb
#define ui_color_picker_hex_to_rgb mock_color_picker_hex_to_rgb
#endif

/* Hover Card Implementation                                                 */
/* ========================================================================= */

ui_error_t md3_hover_card_create(struct ui_engine *engine,
                                 struct ui_dom_node *anchor_node,
                                 struct md3_hover_card **out_hover_card) {
  struct md3_hover_card *hc;
  ui_error_t rc;

  if (!engine || !anchor_node || !out_hover_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  hc = (struct md3_hover_card *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_hover_card));
  if (!hc) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(hc, 0, sizeof(struct md3_hover_card));
  hc->anchor_node = anchor_node;
  hc->is_visible = 0;
  hc->open_delay_ms = 300.0f;
  hc->close_delay_ms = 200.0f;

  rc = ui_hover_card_base_create(&hc->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(hc);
    return rc;
  }

  *out_hover_card = hc;
  return UI_ERROR_NONE;
}

ui_error_t md3_hover_card_destroy(struct md3_hover_card *hover_card) {
  ui_error_t rc;

  if (!hover_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (hover_card->base) {
    rc = ui_hover_card_base_destroy(hover_card->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(hover_card);
  return UI_ERROR_NONE;
}

ui_error_t md3_hover_card_show(struct md3_hover_card *hover_card) {
  ui_error_t rc;

  if (!hover_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  hover_card->is_visible = 1;
  rc = ui_hover_card_base_on_mouse_enter(hover_card->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_hover_card_hide(struct md3_hover_card *hover_card) {
  ui_error_t rc;

  if (!hover_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  hover_card->is_visible = 0;
  rc = ui_hover_card_base_on_mouse_leave(hover_card->base, 0.0f, 0.0f);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Popover Implementation                                                    */
/* ========================================================================= */

ui_error_t md3_popover_create(struct ui_engine *engine,
                              struct ui_dom_node *anchor_node,
                              struct md3_popover **out_popover) {
  struct md3_popover *pop;
  ui_error_t rc;

  if (!engine || !anchor_node || !out_popover) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pop =
      (struct md3_popover *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_popover));
  if (!pop) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(pop, 0, sizeof(struct md3_popover));
  pop->anchor_node = anchor_node;
  pop->is_open = 0;
  pop->elevation_level = 2;

  rc = ui_popover_base_create(&pop->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(pop);
    return rc;
  }

  *out_popover = pop;
  return UI_ERROR_NONE;
}

ui_error_t md3_popover_destroy(struct md3_popover *popover) {
  ui_error_t rc;

  if (!popover) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (popover->base) {
    rc = ui_popover_base_destroy(popover->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(popover);
  return UI_ERROR_NONE;
}

ui_error_t md3_popover_open(struct md3_popover *popover) {
  if (!popover) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  popover->is_open = 1;
  return UI_ERROR_NONE;
}

ui_error_t md3_popover_close(struct md3_popover *popover) {
  ui_error_t rc;

  if (!popover) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  popover->is_open = 0;
  rc = ui_popover_base_close(popover->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_popover_is_open(const struct md3_popover *popover,
                               int *out_is_open) {
  if (!popover || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = popover->is_open;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Banner Implementation                                                     */
/* ========================================================================= */

ui_error_t md3_banner_create(struct ui_engine *engine,
                             struct md3_banner **out_banner) {
  struct md3_banner *b;
  ui_error_t rc;

  if (!engine || !out_banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  b = (struct md3_banner *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_banner));
  if (!b) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(b, 0, sizeof(struct md3_banner));
  b->is_open = 1;
  b->is_dismissible = 1;

  rc = ui_banner_base_create(&b->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(b);
    return rc;
  }

  *out_banner = b;
  return UI_ERROR_NONE;
}

ui_error_t md3_banner_destroy(struct md3_banner *banner) {
  ui_error_t rc;

  if (!banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (banner->base) {
    rc = ui_banner_base_destroy(banner->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(banner);
  return UI_ERROR_NONE;
}

ui_error_t md3_banner_set_text(struct md3_banner *banner, const char *text) {
  ui_error_t rc;

  if (!banner || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(banner->message, sizeof(banner->message), text, _TRUNCATE);
#else
  strncpy(banner->message, text, sizeof(banner->message) - 1);
  banner->message[sizeof(banner->message) - 1] = '\0';
#endif

  rc = ui_banner_base_set_text(banner->base, text);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_banner_set_action(struct md3_banner *banner, int action_index,
                                 const char *label) {
  if (!banner || !label || (action_index != 0 && action_index != 1)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (action_index == 0) {
#if defined(_MSC_VER)
    strncpy_s(banner->action_primary, sizeof(banner->action_primary), label,
              _TRUNCATE);
#else
    strncpy(banner->action_primary, label, sizeof(banner->action_primary) - 1);
    banner->action_primary[sizeof(banner->action_primary) - 1] = '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strncpy_s(banner->action_secondary, sizeof(banner->action_secondary), label,
              _TRUNCATE);
#else
    strncpy(banner->action_secondary, label,
            sizeof(banner->action_secondary) - 1);
    banner->action_secondary[sizeof(banner->action_secondary) - 1] = '\0';
#endif
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_banner_set_open(struct md3_banner *banner, int is_open) {
  ui_error_t rc;

  if (!banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  banner->is_open = (is_open != 0);
  rc = ui_banner_base_set_open(banner->base, banner->is_open);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_banner_is_open(const struct md3_banner *banner,
                              int *out_is_open) {
  if (!banner || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = banner->is_open;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Inline Alert Implementation                                               */
/* ========================================================================= */

ui_error_t md3_inline_alert_create(struct ui_engine *engine,
                                   enum md3_alert_severity severity,
                                   struct md3_inline_alert **out_alert) {
  struct md3_inline_alert *alt;
  ui_error_t rc;

  if (!engine || !out_alert || (int)severity < (int)MD3_ALERT_SEVERITY_INFO ||
      severity > MD3_ALERT_SEVERITY_ERROR) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  alt = (struct md3_inline_alert *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_inline_alert));
  if (!alt) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(alt, 0, sizeof(struct md3_inline_alert));
  alt->severity = severity;
  alt->is_dismissible = 1;
  alt->is_dismissed = 0;

  rc = ui_alert_base_create(&alt->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(alt);
    return rc;
  }

  if (severity == MD3_ALERT_SEVERITY_ERROR ||
      severity == MD3_ALERT_SEVERITY_WARNING) {
    rc = ui_alert_base_set_role(alt->base, UI_ALERT_ROLE_ALERT);
  } else {
    rc = ui_alert_base_set_role(alt->base, UI_ALERT_ROLE_STATUS);
  }
  if (rc != UI_ERROR_NONE) {
    ui_alert_base_destroy(alt->base);
    C_MULTIPLATFORM_FREE(alt);
    return rc;
  }

  *out_alert = alt;
  return UI_ERROR_NONE;
}

ui_error_t md3_inline_alert_destroy(struct md3_inline_alert *alert) {
  ui_error_t rc;

  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (alert->base) {
    rc = ui_alert_base_destroy(alert->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(alert);
  return UI_ERROR_NONE;
}

ui_error_t md3_inline_alert_set_title(struct md3_inline_alert *alert,
                                      const char *title) {
  if (!alert || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(alert->title, sizeof(alert->title), title, _TRUNCATE);
#else
  strncpy(alert->title, title, sizeof(alert->title) - 1);
  alert->title[sizeof(alert->title) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_inline_alert_set_message(struct md3_inline_alert *alert,
                                        const char *message) {
  if (!alert || !message) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(alert->message, sizeof(alert->message), message, _TRUNCATE);
#else
  strncpy(alert->message, message, sizeof(alert->message) - 1);
  alert->message[sizeof(alert->message) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_inline_alert_set_dismissible(struct md3_inline_alert *alert,
                                            int dismissible) {
  ui_error_t rc;

  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  alert->is_dismissible = (dismissible != 0);
  rc = ui_alert_base_set_dismissible(alert->base, alert->is_dismissible);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Menubar Implementation                                                    */
/* ========================================================================= */

ui_error_t md3_menubar_create(struct ui_engine *engine,
                              struct md3_menubar **out_menubar) {
  struct md3_menubar *mb;
  ui_error_t rc;

  if (!engine || !out_menubar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mb = (struct md3_menubar *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_menubar));
  if (!mb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(mb, 0, sizeof(struct md3_menubar));

  rc = ui_menubar_base_create(&mb->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(mb);
    return rc;
  }

  *out_menubar = mb;
  return UI_ERROR_NONE;
}

ui_error_t md3_menubar_destroy(struct md3_menubar *menubar) {
  ui_error_t rc;

  if (!menubar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (menubar->base) {
    rc = ui_component_destroy((struct ui_component *)menubar->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(menubar);
  return UI_ERROR_NONE;
}

ui_error_t md3_menubar_append_item(struct md3_menubar *menubar,
                                   struct ui_component *item) {
  ui_error_t rc;

  if (!menubar || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!item->shadow_root) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &item->shadow_root);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  rc = ui_menubar_base_append_item(menubar->base, item);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  menubar->item_count++;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Context Menu Implementation                                               */
/* ========================================================================= */

ui_error_t md3_context_menu_create(struct ui_engine *engine,
                                   struct md3_context_menu **out_context_menu) {
  struct md3_context_menu *cm;
  ui_error_t rc;

  if (!engine || !out_context_menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cm = (struct md3_context_menu *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_context_menu));
  if (!cm) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(cm, 0, sizeof(struct md3_context_menu));

  rc = ui_context_menu_base_create(&cm->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(cm);
    return rc;
  }

  *out_context_menu = cm;
  return UI_ERROR_NONE;
}

ui_error_t md3_context_menu_destroy(struct md3_context_menu *menu) {
  ui_error_t rc;

  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (menu->base) {
    rc = ui_context_menu_base_destroy(menu->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(menu);
  return UI_ERROR_NONE;
}

ui_error_t md3_context_menu_open_at(struct md3_context_menu *menu, int x, int y,
                                    int viewport_w, int viewport_h) {
  if (!menu || x < 0 || y < 0 || viewport_w <= 0 || viewport_h <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  menu->current_x = (x + 200 > viewport_w) ? (viewport_w - 200) : x;
  menu->current_y = (y + 150 > viewport_h) ? (viewport_h - 150) : y;
  menu->is_open = 1;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Action Sheet Implementation                                               */
/* ========================================================================= */

ui_error_t md3_action_sheet_create(struct ui_engine *engine,
                                   struct md3_action_sheet **out_sheet) {
  struct md3_action_sheet *as;
  ui_error_t rc;

  if (!engine || !out_sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  as = (struct md3_action_sheet *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_action_sheet));
  if (!as) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(as, 0, sizeof(struct md3_action_sheet));
  as->action_count = 0;
  as->is_open = 0;

  rc = ui_action_sheet_base_create(&as->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(as);
    return rc;
  }

  *out_sheet = as;
  return UI_ERROR_NONE;
}

ui_error_t md3_action_sheet_destroy(struct md3_action_sheet *sheet) {
  ui_error_t rc;

  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->base) {
    rc = ui_action_sheet_base_destroy(sheet->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(sheet);
  return UI_ERROR_NONE;
}

ui_error_t md3_action_sheet_add_action(struct md3_action_sheet *sheet,
                                       const char *label, const char *icon) {
  struct md3_action_sheet_item *item;

  if (!sheet || !label || sheet->action_count >= 16) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = &sheet->actions[sheet->action_count];
  memset(item, 0, sizeof(struct md3_action_sheet_item));

#if defined(_MSC_VER)
  strncpy_s(item->label, sizeof(item->label), label, _TRUNCATE);
  if (icon) {
    strncpy_s(item->icon, sizeof(item->icon), icon, _TRUNCATE);
  }
#else
  strncpy(item->label, label, sizeof(item->label) - 1);
  item->label[sizeof(item->label) - 1] = '\0';
  if (icon) {
    strncpy(item->icon, icon, sizeof(item->icon) - 1);
    item->icon[sizeof(item->icon) - 1] = '\0';
  }
#endif

  sheet->action_count++;
  return UI_ERROR_NONE;
}

ui_error_t md3_action_sheet_set_cancel_action(struct md3_action_sheet *sheet,
                                              const char *cancel_label) {
  if (!sheet || !cancel_label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(sheet->cancel_label, sizeof(sheet->cancel_label), cancel_label,
            _TRUNCATE);
#else
  strncpy(sheet->cancel_label, cancel_label, sizeof(sheet->cancel_label) - 1);
  sheet->cancel_label[sizeof(sheet->cancel_label) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_action_sheet_set_open(struct md3_action_sheet *sheet,
                                     int is_open) {
  ui_error_t rc;

  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet->is_open = (is_open != 0);
  rc = ui_action_sheet_base_set_open(sheet->base, sheet->is_open);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_action_sheet_is_open(const struct md3_action_sheet *sheet,
                                    int *out_is_open) {
  if (!sheet || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = sheet->is_open;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Coachmark Implementation                                                  */
/* ========================================================================= */

ui_error_t md3_coachmark_create(struct ui_engine *engine,
                                struct ui_dom_node *target_node,
                                struct md3_coachmark **out_coachmark) {
  struct md3_coachmark *cm;

  if (!engine || !target_node || !out_coachmark) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cm = (struct md3_coachmark *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_coachmark));
  if (!cm) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(cm, 0, sizeof(struct md3_coachmark));
  cm->target_node = target_node;
  cm->current_step = 0;
  cm->total_steps = 1;
  cm->is_active = 0;

  *out_coachmark = cm;
  return UI_ERROR_NONE;
}

ui_error_t md3_coachmark_destroy(struct md3_coachmark *coachmark) {
  if (!coachmark) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(coachmark);
  return UI_ERROR_NONE;
}

ui_error_t md3_coachmark_set_content(struct md3_coachmark *coachmark,
                                     const char *title, const char *description,
                                     int step_index, int total_steps) {
  if (!coachmark || !title || !description || step_index < 0 ||
      total_steps <= 0 || step_index >= total_steps) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(coachmark->title, sizeof(coachmark->title), title, _TRUNCATE);
  strncpy_s(coachmark->description, sizeof(coachmark->description), description,
            _TRUNCATE);
#else
  strncpy(coachmark->title, title, sizeof(coachmark->title) - 1);
  coachmark->title[sizeof(coachmark->title) - 1] = '\0';
  strncpy(coachmark->description, description,
          sizeof(coachmark->description) - 1);
  coachmark->description[sizeof(coachmark->description) - 1] = '\0';
#endif

  coachmark->current_step = step_index;
  coachmark->total_steps = total_steps;
  return UI_ERROR_NONE;
}

ui_error_t md3_coachmark_start(struct md3_coachmark *coachmark) {
  if (!coachmark) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  coachmark->is_active = 1;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Color Picker Implementation                                               */
/* ========================================================================= */

static ui_error_t md3_cp_cva_write_value(void *component,
                                         union ui_signal_payload value) {
  struct md3_color_picker *picker;
  const struct ui_color_rgb *rgb;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct md3_color_picker *)component;
  rgb = (const struct ui_color_rgb *)value.ptr_val;
  if (rgb) {
    return md3_color_picker_set_rgb(picker, rgb->r, rgb->g, rgb->b);
  }
  return UI_ERROR_NONE;
}

static ui_error_t md3_cp_cva_set_disabled_state(void *component,
                                                ui_bool_t is_disabled) {
  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (is_disabled != UI_TRUE && is_disabled != UI_FALSE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_color_picker_create(struct ui_engine *engine,
                                   struct md3_color_picker **out_picker,
                                   struct ui_control_value_accessor **out_cva) {
  struct md3_color_picker *cp;
  ui_error_t rc;

  if (!engine || !out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cp = (struct md3_color_picker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_color_picker));
  if (!cp) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(cp, 0, sizeof(struct md3_color_picker));
  cp->hsv.h = 0.0;
  cp->hsv.s = 1.0;
  cp->hsv.v = 1.0;
  cp->rgb.r = 255;
  cp->rgb.g = 0;
  cp->rgb.b = 0;

#if defined(_MSC_VER)
  strncpy_s(cp->hex_string, sizeof(cp->hex_string), "#FF0000", _TRUNCATE);
#else
  strncpy(cp->hex_string, "#FF0000", sizeof(cp->hex_string) - 1);
  cp->hex_string[sizeof(cp->hex_string) - 1] = '\0';
#endif

  rc = ui_color_picker_base_create(&cp->base, &cp->cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(cp);
    return rc;
  }

  cp->cva.component = cp;
  cp->cva.write_value = md3_cp_cva_write_value;
  cp->cva.set_disabled_state = md3_cp_cva_set_disabled_state;
  cp->cva.register_on_change = NULL;
  cp->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &cp->cva;
  }

  *out_picker = cp;
  return UI_ERROR_NONE;
}

ui_error_t md3_color_picker_destroy(struct md3_color_picker *picker) {
  ui_error_t rc;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (picker->base) {
    rc = ui_color_picker_base_destroy(picker->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(picker);
  return UI_ERROR_NONE;
}

ui_error_t md3_color_picker_set_hsv(struct md3_color_picker *picker, double h,
                                    double s, double v) {
  ui_error_t rc;

  if (!picker || h < 0.0 || h >= 360.0 || s < 0.0 || s > 1.0 || v < 0.0 ||
      v > 1.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->hsv.h = h;
  picker->hsv.s = s;
  picker->hsv.v = v;

  rc = ui_color_picker_hsv_to_rgb(&picker->hsv, &picker->rgb);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_color_picker_rgb_to_hex(&picker->rgb, picker->hex_string,
                                  sizeof(picker->hex_string));
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_color_picker_get_hsv(const struct md3_color_picker *picker,
                                    double *out_h, double *out_s,
                                    double *out_v) {
  if (!picker || !out_h || !out_s || !out_v) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_h = picker->hsv.h;
  *out_s = picker->hsv.s;
  *out_v = picker->hsv.v;
  return UI_ERROR_NONE;
}

ui_error_t md3_color_picker_set_rgb(struct md3_color_picker *picker,
                                    unsigned char r, unsigned char g,
                                    unsigned char b) {
  ui_error_t rc;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->rgb.r = r;
  picker->rgb.g = g;
  picker->rgb.b = b;

  rc = ui_color_picker_rgb_to_hsv(&picker->rgb, &picker->hsv);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_color_picker_rgb_to_hex(&picker->rgb, picker->hex_string,
                                  sizeof(picker->hex_string));
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_color_picker_get_rgb(const struct md3_color_picker *picker,
                                    unsigned char *out_r, unsigned char *out_g,
                                    unsigned char *out_b) {
  if (!picker || !out_r || !out_g || !out_b) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_r = picker->rgb.r;
  *out_g = picker->rgb.g;
  *out_b = picker->rgb.b;
  return UI_ERROR_NONE;
}

ui_error_t md3_color_picker_set_hex(struct md3_color_picker *picker,
                                    const char *hex) {
  ui_error_t rc;

  if (!picker || !hex) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_color_picker_hex_to_rgb(hex, &picker->rgb);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_color_picker_rgb_to_hsv(&picker->rgb, &picker->hsv);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  strncpy_s(picker->hex_string, sizeof(picker->hex_string), hex, _TRUNCATE);
#else
  strncpy(picker->hex_string, hex, sizeof(picker->hex_string) - 1);
  picker->hex_string[sizeof(picker->hex_string) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_color_picker_get_hex(const struct md3_color_picker *picker,
                                    char *out_hex, size_t hex_size) {
  if (!picker || !out_hex || hex_size < 8) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(out_hex, hex_size, picker->hex_string, _TRUNCATE);
#else
  strncpy(out_hex, picker->hex_string, hex_size - 1);
  out_hex[hex_size - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Command Palette Implementation                                            */
/* ========================================================================= */

ui_error_t
md3_command_palette_create(struct ui_engine *engine,
                           struct md3_command_palette **out_palette) {
  struct md3_command_palette *cp;

  if (!engine || !out_palette) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cp = (struct md3_command_palette *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_command_palette));
  if (!cp) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(cp, 0, sizeof(struct md3_command_palette));
  cp->command_count = 0;
  cp->is_open = 0;

  *out_palette = cp;
  return UI_ERROR_NONE;
}

ui_error_t md3_command_palette_destroy(struct md3_command_palette *palette) {
  if (!palette) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(palette);
  return UI_ERROR_NONE;
}

ui_error_t md3_command_palette_add_action(struct md3_command_palette *palette,
                                          const char *category,
                                          const char *title,
                                          const char *shortcut) {
  struct md3_command_item *item;

  if (!palette || !title || palette->command_count >= 64) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = &palette->commands[palette->command_count];
  memset(item, 0, sizeof(struct md3_command_item));

  if (category) {
#if defined(_MSC_VER)
    strncpy_s(item->category, sizeof(item->category), category, _TRUNCATE);
#else
    strncpy(item->category, category, sizeof(item->category) - 1);
    item->category[sizeof(item->category) - 1] = '\0';
#endif
  }

#if defined(_MSC_VER)
  strncpy_s(item->title, sizeof(item->title), title, _TRUNCATE);
  if (shortcut) {
    strncpy_s(item->shortcut, sizeof(item->shortcut), shortcut, _TRUNCATE);
  }
#else
  strncpy(item->title, title, sizeof(item->title) - 1);
  item->title[sizeof(item->title) - 1] = '\0';
  if (shortcut) {
    strncpy(item->shortcut, shortcut, sizeof(item->shortcut) - 1);
    item->shortcut[sizeof(item->shortcut) - 1] = '\0';
  }
#endif

  palette->command_count++;
  return UI_ERROR_NONE;
}

ui_error_t md3_command_palette_open(struct md3_command_palette *palette) {
  if (!palette) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  palette->is_open = 1;
  return UI_ERROR_NONE;
}

ui_error_t md3_command_palette_close(struct md3_command_palette *palette) {
  if (!palette) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  palette->is_open = 0;
  return UI_ERROR_NONE;
}

ui_error_t
md3_command_palette_is_open(const struct md3_command_palette *palette,
                            int *out_is_open) {
  if (!palette || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = palette->is_open;
  return UI_ERROR_NONE;
}
