#ifndef M3_MENU_H
#define M3_MENU_H

/**
 * @file m3_menu.h
 * @brief Menu and popup widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"

/** @brief Anchor uses a rectangle (dropdown menus). */
#define M3_MENU_ANCHOR_RECT 0
/** @brief Anchor uses a point (context menus). */
#define M3_MENU_ANCHOR_POINT 1

/** @brief Preferred menu direction: below the anchor. */
#define M3_MENU_DIRECTION_DOWN 0
/** @brief Preferred menu direction: above the anchor. */
#define M3_MENU_DIRECTION_UP 1
/** @brief Preferred menu direction: to the right of the anchor. */
#define M3_MENU_DIRECTION_RIGHT 2
/** @brief Preferred menu direction: to the left of the anchor. */
#define M3_MENU_DIRECTION_LEFT 3

/** @brief Align menu start edge to the anchor start. */
#define M3_MENU_ALIGN_START 0
/** @brief Align menu center to the anchor center. */
#define M3_MENU_ALIGN_CENTER 1
/** @brief Align menu end edge to the anchor end. */
#define M3_MENU_ALIGN_END 2

/** @brief Menu action: item selected. */
#define M3_MENU_ACTION_SELECT 1u
/** @brief Menu action: dismissed (pointer outside). */
#define M3_MENU_ACTION_DISMISS 2u

/** @brief Invalid menu index sentinel. */
#define M3_MENU_INVALID_INDEX ((cmp_usize) ~(cmp_usize)0)

/** @brief Default menu item height in pixels. */
#define M3_MENU_DEFAULT_ITEM_HEIGHT 48.0f
/** @brief Default menu minimum width in pixels. */
#define M3_MENU_DEFAULT_MIN_WIDTH 112.0f
/** @brief Default menu maximum width in pixels. */
#define M3_MENU_DEFAULT_MAX_WIDTH 280.0f
/** @brief Default horizontal padding in pixels. */
#define M3_MENU_DEFAULT_PADDING_X 12.0f
/** @brief Default vertical padding in pixels. */
#define M3_MENU_DEFAULT_PADDING_Y 8.0f
/** @brief Default spacing between items in pixels. */
#define M3_MENU_DEFAULT_ITEM_SPACING 0.0f
/** @brief Default menu corner radius in pixels. */
#define M3_MENU_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default gap between anchor and menu in pixels. */
#define M3_MENU_DEFAULT_ANCHOR_GAP 4.0f

struct M3Menu;

/**
 * @brief Menu action callback signature.
 * @param ctx User callback context pointer.
 * @param menu Menu instance that triggered the action.
 * @param action Action identifier (CMP_MENU_ACTION_*).
 * @param index Selected index (M3_MENU_INVALID_INDEX for dismiss).
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPMenuOnAction)(void *ctx, struct M3Menu *menu,
                                     cmp_u32 action, cmp_usize index);

/**
 * @brief Menu item descriptor.
 */
typedef struct M3MenuItem {
  const char *utf8_label; /**< UTF-8 label pointer (may be NULL when
                             utf8_len is 0). */
  cmp_usize utf8_len;      /**< UTF-8 label length in bytes. */
  CMPBool enabled;         /**< CMP_TRUE when item is enabled. */
} M3MenuItem;

/**
 * @brief Menu anchor descriptor.
 */
typedef struct M3MenuAnchor {
  cmp_u32 type;  /**< Anchor type (CMP_MENU_ANCHOR_*). */
  CMPRect rect;  /**< Anchor rectangle (used for M3_MENU_ANCHOR_RECT). */
  CMPVec2 point; /**< Anchor point (used for M3_MENU_ANCHOR_POINT). */
} M3MenuAnchor;

/**
 * @brief Menu placement descriptor.
 */
typedef struct M3MenuPlacement {
  cmp_u32 direction; /**< Preferred direction (CMP_MENU_DIRECTION_*). */
  cmp_u32 align;     /**< Alignment along the cross axis (CMP_MENU_ALIGN_*). */
} M3MenuPlacement;

/**
 * @brief Menu style descriptor.
 */
typedef struct M3MenuStyle {
  CMPLayoutEdges padding;  /**< Padding around menu contents. */
  CMPScalar item_height;   /**< Menu item height in pixels (> 0). */
  CMPScalar item_spacing;  /**< Spacing between items in pixels (>= 0). */
  CMPScalar min_width;     /**< Minimum menu width in pixels (>= 0). */
  CMPScalar max_width;     /**< Maximum menu width in pixels (>= 0, 0 = none). */
  CMPScalar corner_radius; /**< Menu corner radius in pixels (>= 0). */
  CMPScalar anchor_gap;    /**< Gap between anchor and menu in pixels (>= 0). */
  CMPTextStyle text_style; /**< Label text style (requires valid family name). */
  CMPColor background_color;    /**< Menu background color. */
  CMPColor disabled_text_color; /**< Text color for disabled items. */
  CMPShadow shadow;             /**< Shadow descriptor. */
  CMPBool shadow_enabled;       /**< CMP_TRUE when shadow rendering is enabled. */
} M3MenuStyle;

/**
 * @brief Menu widget instance.
 */
typedef struct M3Menu {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend; /**< Text backend instance. */
  CMPHandle font;              /**< Font handle for menu labels. */
  CMPTextMetrics font_metrics; /**< Cached font metrics. */
  M3MenuStyle style;          /**< Current menu style. */
  const M3MenuItem *items;    /**< Menu items (not owned). */
  cmp_usize item_count;        /**< Number of menu items. */
  M3MenuAnchor anchor;        /**< Current anchor descriptor. */
  M3MenuPlacement placement;  /**< Preferred placement settings. */
  cmp_u32 resolved_direction;  /**< Resolved placement direction. */
  CMPRect overlay_bounds;      /**< Bounds of the overlay region. */
  CMPRect menu_bounds;         /**< Bounds of the menu panel. */
  cmp_usize pressed_index;   /**< Pressed item index or M3_MENU_INVALID_INDEX. */
  CMPBool owns_font;         /**< CMP_TRUE when widget owns the font. */
  CMPBool open;              /**< CMP_TRUE when menu is open/visible. */
  CMPMenuOnAction on_action; /**< Action callback (may be NULL). */
  void *on_action_ctx;      /**< Action callback context pointer. */
} M3Menu;

/**
 * @brief Initialize a menu style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_style_init(M3MenuStyle *style);

/**
 * @brief Initialize a menu widget.
 * @param menu Menu instance.
 * @param backend Text backend instance.
 * @param style Menu style descriptor.
 * @param items Menu item array (may be NULL when item_count is 0).
 * @param item_count Number of menu items.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_init(M3Menu *menu, const CMPTextBackend *backend,
                                const M3MenuStyle *style,
                                const M3MenuItem *items, cmp_usize item_count);

/**
 * @brief Update the menu items.
 * @param menu Menu instance.
 * @param items Menu item array (may be NULL when item_count is 0).
 * @param item_count Number of menu items.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_set_items(M3Menu *menu, const M3MenuItem *items,
                                     cmp_usize item_count);

/**
 * @brief Update the menu style.
 * @param menu Menu instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_set_style(M3Menu *menu, const M3MenuStyle *style);

/**
 * @brief Set the menu anchor rectangle (dropdown menus).
 * @param menu Menu instance.
 * @param rect Anchor rectangle (non-negative width/height).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_set_anchor_rect(M3Menu *menu, const CMPRect *rect);

/**
 * @brief Set the menu anchor point (context menus).
 * @param menu Menu instance.
 * @param x Anchor X position in pixels.
 * @param y Anchor Y position in pixels.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_set_anchor_point(M3Menu *menu, CMPScalar x,
                                            CMPScalar y);

/**
 * @brief Update the menu placement settings.
 * @param menu Menu instance.
 * @param placement Placement descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_set_placement(M3Menu *menu,
                                         const M3MenuPlacement *placement);

/**
 * @brief Open or close the menu (updates widget hidden flag).
 * @param menu Menu instance.
 * @param open CMP_TRUE to open, CMP_FALSE to close.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_set_open(M3Menu *menu, CMPBool open);

/**
 * @brief Retrieve the open state of the menu.
 * @param menu Menu instance.
 * @param out_open Receives CMP_TRUE when open.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_get_open(const M3Menu *menu, CMPBool *out_open);

/**
 * @brief Retrieve the current menu panel bounds.
 * @param menu Menu instance.
 * @param out_bounds Receives the menu bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_get_bounds(const M3Menu *menu, CMPRect *out_bounds);

/**
 * @brief Retrieve the current overlay bounds.
 * @param menu Menu instance.
 * @param out_bounds Receives the overlay bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_get_overlay_bounds(const M3Menu *menu,
                                              CMPRect *out_bounds);

/**
 * @brief Retrieve the resolved menu direction after collision handling.
 * @param menu Menu instance.
 * @param out_direction Receives the resolved direction.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_get_resolved_direction(const M3Menu *menu,
                                                  cmp_u32 *out_direction);

/**
 * @brief Retrieve the bounds for a menu item.
 * @param menu Menu instance.
 * @param index Item index.
 * @param out_bounds Receives the item bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_get_item_bounds(const M3Menu *menu, cmp_usize index,
                                           CMPRect *out_bounds);

/**
 * @brief Assign a menu action callback.
 * @param menu Menu instance.
 * @param on_action Action callback (may be NULL).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_set_on_action(M3Menu *menu, CMPMenuOnAction on_action,
                                         void *ctx);

#ifdef CMP_TESTING
/**
 * @brief Test hook to force negative panel size computation.
 * @param enable CMP_TRUE to force a negative size on the next computation.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_set_force_negative_panel(CMPBool enable);

/**
 * @brief Test hook to force overflow clamping in panel bounds.
 * @param enable CMP_TRUE to force bounds overflow on the next computation.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_set_force_bounds_overflow(CMPBool enable);

/**
 * @brief Test hook to force hit-test rectangle errors.
 * @param enable CMP_TRUE to inject a hit-test error on the next check.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_set_force_hit_test_error(CMPBool enable);

/**
 * @brief Test wrapper for menu style validation.
 * @param style Menu style to validate.
 * @param require_family Whether a family name is required.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_validate_style(const M3MenuStyle *style,
                                               CMPBool require_family);

/**
 * @brief Test wrapper for menu text metric updates.
 * @param menu Menu instance.
 * @param out_text_width Text width output.
 * @param out_has_label CMP_TRUE when any label exists.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_update_metrics(M3Menu *menu,
                                               CMPScalar *out_text_width,
                                               CMPBool *out_has_label);

/**
 * @brief Test wrapper for computing panel size.
 * @param menu Menu instance.
 * @param out_width Panel width output.
 * @param out_height Panel height output.
 * @param out_has_label CMP_TRUE when any label exists.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_compute_panel_size(M3Menu *menu,
                                                   CMPScalar *out_width,
                                                   CMPScalar *out_height,
                                                   CMPBool *out_has_label);

/**
 * @brief Test wrapper for computing panel bounds.
 * @param menu Menu instance.
 * @param overlay Overlay bounds.
 * @param panel_width Panel width.
 * @param panel_height Panel height.
 * @param out_bounds Computed panel bounds.
 * @param out_direction Resolved direction.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_compute_panel_bounds(
    const M3Menu *menu, const CMPRect *overlay, CMPScalar panel_width,
    CMPScalar panel_height, CMPRect *out_bounds, cmp_u32 *out_direction);

/**
 * @brief Test wrapper for hit testing.
 * @param menu Menu instance.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 * @param out_inside CMP_TRUE when the point is inside.
 * @param out_index Hit item index (or M3_MENU_INVALID_INDEX).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_hit_test(const M3Menu *menu, CMPScalar x,
                                         CMPScalar y, CMPBool *out_inside,
                                         cmp_usize *out_index);

/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_validate_color(const CMPColor *color);

/**
 * @brief Test wrapper for setting a color.
 * @param color Color to set.
 * @param r Red component (0..1).
 * @param g Green component (0..1).
 * @param b Blue component (0..1).
 * @param a Alpha component (0..1).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_color_set(CMPColor *color, CMPScalar r,
                                          CMPScalar g, CMPScalar b, CMPScalar a);

/**
 * @brief Test wrapper for edge validation.
 * @param edges Edge descriptor to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_validate_edges(const CMPLayoutEdges *edges);

/**
 * @brief Test wrapper for text style validation.
 * @param style Text style to validate.
 * @param require_family Whether a family name is required.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_validate_text_style(const CMPTextStyle *style,
                                                    CMPBool require_family);

/**
 * @brief Test wrapper for measure spec validation.
 * @param spec Measure spec to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_validate_measure_spec(CMPMeasureSpec spec);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_validate_rect(const CMPRect *rect);

/**
 * @brief Test wrapper for anchor validation.
 * @param anchor Anchor descriptor to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_validate_anchor(const M3MenuAnchor *anchor);

/**
 * @brief Test wrapper for placement validation.
 * @param placement Placement descriptor to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_menu_test_validate_placement(const M3MenuPlacement *placement);

/**
 * @brief Test wrapper for menu item validation.
 * @param items Item array to validate.
 * @param count Item count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_menu_test_validate_items(const M3MenuItem *items,
                                               cmp_usize count);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_MENU_H */
