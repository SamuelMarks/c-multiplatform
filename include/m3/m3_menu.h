#ifndef M3_MENU_H
#define M3_MENU_H

/**
 * @file m3_menu.h
 * @brief Menu and popup widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_layout.h"
#include "m3_text.h"
#include "m3_visuals.h"

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
#define M3_MENU_INVALID_INDEX ((m3_usize) ~(m3_usize)0)

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
 * @param action Action identifier (M3_MENU_ACTION_*).
 * @param index Selected index (M3_MENU_INVALID_INDEX for dismiss).
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3MenuOnAction)(void *ctx, struct M3Menu *menu,
                                     m3_u32 action, m3_usize index);

/**
 * @brief Menu item descriptor.
 */
typedef struct M3MenuItem {
  const char *utf8_label; /**< UTF-8 label pointer (may be NULL when
                             utf8_len is 0). */
  m3_usize utf8_len;      /**< UTF-8 label length in bytes. */
  M3Bool enabled;         /**< M3_TRUE when item is enabled. */
} M3MenuItem;

/**
 * @brief Menu anchor descriptor.
 */
typedef struct M3MenuAnchor {
  m3_u32 type;  /**< Anchor type (M3_MENU_ANCHOR_*). */
  M3Rect rect;  /**< Anchor rectangle (used for M3_MENU_ANCHOR_RECT). */
  M3Vec2 point; /**< Anchor point (used for M3_MENU_ANCHOR_POINT). */
} M3MenuAnchor;

/**
 * @brief Menu placement descriptor.
 */
typedef struct M3MenuPlacement {
  m3_u32 direction; /**< Preferred direction (M3_MENU_DIRECTION_*). */
  m3_u32 align;     /**< Alignment along the cross axis (M3_MENU_ALIGN_*). */
} M3MenuPlacement;

/**
 * @brief Menu style descriptor.
 */
typedef struct M3MenuStyle {
  M3LayoutEdges padding;  /**< Padding around menu contents. */
  M3Scalar item_height;   /**< Menu item height in pixels (> 0). */
  M3Scalar item_spacing;  /**< Spacing between items in pixels (>= 0). */
  M3Scalar min_width;     /**< Minimum menu width in pixels (>= 0). */
  M3Scalar max_width;     /**< Maximum menu width in pixels (>= 0, 0 = none). */
  M3Scalar corner_radius; /**< Menu corner radius in pixels (>= 0). */
  M3Scalar anchor_gap;    /**< Gap between anchor and menu in pixels (>= 0). */
  M3TextStyle text_style; /**< Label text style (requires valid family name). */
  M3Color background_color;    /**< Menu background color. */
  M3Color disabled_text_color; /**< Text color for disabled items. */
  M3Shadow shadow;             /**< Shadow descriptor. */
  M3Bool shadow_enabled;       /**< M3_TRUE when shadow rendering is enabled. */
} M3MenuStyle;

/**
 * @brief Menu widget instance.
 */
typedef struct M3Menu {
  M3Widget widget; /**< Widget interface (points to this instance). */
  M3TextBackend text_backend; /**< Text backend instance. */
  M3Handle font;              /**< Font handle for menu labels. */
  M3TextMetrics font_metrics; /**< Cached font metrics. */
  M3MenuStyle style;          /**< Current menu style. */
  const M3MenuItem *items;    /**< Menu items (not owned). */
  m3_usize item_count;        /**< Number of menu items. */
  M3MenuAnchor anchor;        /**< Current anchor descriptor. */
  M3MenuPlacement placement;  /**< Preferred placement settings. */
  m3_u32 resolved_direction;  /**< Resolved placement direction. */
  M3Rect overlay_bounds;      /**< Bounds of the overlay region. */
  M3Rect menu_bounds;         /**< Bounds of the menu panel. */
  m3_usize pressed_index;   /**< Pressed item index or M3_MENU_INVALID_INDEX. */
  M3Bool owns_font;         /**< M3_TRUE when widget owns the font. */
  M3Bool open;              /**< M3_TRUE when menu is open/visible. */
  M3MenuOnAction on_action; /**< Action callback (may be NULL). */
  void *on_action_ctx;      /**< Action callback context pointer. */
} M3Menu;

/**
 * @brief Initialize a menu style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_style_init(M3MenuStyle *style);

/**
 * @brief Initialize a menu widget.
 * @param menu Menu instance.
 * @param backend Text backend instance.
 * @param style Menu style descriptor.
 * @param items Menu item array (may be NULL when item_count is 0).
 * @param item_count Number of menu items.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_init(M3Menu *menu, const M3TextBackend *backend,
                                const M3MenuStyle *style,
                                const M3MenuItem *items, m3_usize item_count);

/**
 * @brief Update the menu items.
 * @param menu Menu instance.
 * @param items Menu item array (may be NULL when item_count is 0).
 * @param item_count Number of menu items.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_set_items(M3Menu *menu, const M3MenuItem *items,
                                     m3_usize item_count);

/**
 * @brief Update the menu style.
 * @param menu Menu instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_set_style(M3Menu *menu, const M3MenuStyle *style);

/**
 * @brief Set the menu anchor rectangle (dropdown menus).
 * @param menu Menu instance.
 * @param rect Anchor rectangle (non-negative width/height).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_set_anchor_rect(M3Menu *menu, const M3Rect *rect);

/**
 * @brief Set the menu anchor point (context menus).
 * @param menu Menu instance.
 * @param x Anchor X position in pixels.
 * @param y Anchor Y position in pixels.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_set_anchor_point(M3Menu *menu, M3Scalar x,
                                            M3Scalar y);

/**
 * @brief Update the menu placement settings.
 * @param menu Menu instance.
 * @param placement Placement descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_set_placement(M3Menu *menu,
                                         const M3MenuPlacement *placement);

/**
 * @brief Open or close the menu (updates widget hidden flag).
 * @param menu Menu instance.
 * @param open M3_TRUE to open, M3_FALSE to close.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_set_open(M3Menu *menu, M3Bool open);

/**
 * @brief Retrieve the open state of the menu.
 * @param menu Menu instance.
 * @param out_open Receives M3_TRUE when open.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_get_open(const M3Menu *menu, M3Bool *out_open);

/**
 * @brief Retrieve the current menu panel bounds.
 * @param menu Menu instance.
 * @param out_bounds Receives the menu bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_get_bounds(const M3Menu *menu, M3Rect *out_bounds);

/**
 * @brief Retrieve the current overlay bounds.
 * @param menu Menu instance.
 * @param out_bounds Receives the overlay bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_get_overlay_bounds(const M3Menu *menu,
                                              M3Rect *out_bounds);

/**
 * @brief Retrieve the resolved menu direction after collision handling.
 * @param menu Menu instance.
 * @param out_direction Receives the resolved direction.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_get_resolved_direction(const M3Menu *menu,
                                                  m3_u32 *out_direction);

/**
 * @brief Retrieve the bounds for a menu item.
 * @param menu Menu instance.
 * @param index Item index.
 * @param out_bounds Receives the item bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_get_item_bounds(const M3Menu *menu, m3_usize index,
                                           M3Rect *out_bounds);

/**
 * @brief Assign a menu action callback.
 * @param menu Menu instance.
 * @param on_action Action callback (may be NULL).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_set_on_action(M3Menu *menu, M3MenuOnAction on_action,
                                         void *ctx);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_test_validate_color(const M3Color *color);

/**
 * @brief Test wrapper for setting a color.
 * @param color Color to set.
 * @param r Red component (0..1).
 * @param g Green component (0..1).
 * @param b Blue component (0..1).
 * @param a Alpha component (0..1).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_test_color_set(M3Color *color, M3Scalar r,
                                          M3Scalar g, M3Scalar b, M3Scalar a);

/**
 * @brief Test wrapper for edge validation.
 * @param edges Edge descriptor to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_test_validate_edges(const M3LayoutEdges *edges);

/**
 * @brief Test wrapper for text style validation.
 * @param style Text style to validate.
 * @param require_family Whether a family name is required.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_test_validate_text_style(const M3TextStyle *style,
                                                    M3Bool require_family);

/**
 * @brief Test wrapper for measure spec validation.
 * @param spec Measure spec to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_test_validate_measure_spec(M3MeasureSpec spec);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_test_validate_rect(const M3Rect *rect);

/**
 * @brief Test wrapper for anchor validation.
 * @param anchor Anchor descriptor to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_test_validate_anchor(const M3MenuAnchor *anchor);

/**
 * @brief Test wrapper for placement validation.
 * @param placement Placement descriptor to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_menu_test_validate_placement(const M3MenuPlacement *placement);

/**
 * @brief Test wrapper for menu item validation.
 * @param items Item array to validate.
 * @param count Item count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_menu_test_validate_items(const M3MenuItem *items,
                                               m3_usize count);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_MENU_H */
