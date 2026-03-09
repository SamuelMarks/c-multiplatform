#ifndef F2_NAVIGATION_H
#define F2_NAVIGATION_H

/**
 * @file f2_navigation.h
 * @brief Microsoft Fluent 2 Navigation components (NavigationView, TabList,
 * Breadcrumb).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_color.h"

/** @brief Invalid selection index. */
#define F2_NAV_INVALID_INDEX ((cmp_usize) ~(cmp_usize)0)

/**
 * @brief Common navigation item.
 */
typedef struct F2NavItem {
  const char *utf8_label; /**< Item label. */
  cmp_usize utf8_len;     /**< Label length. */
} F2NavItem;

/* -------------------------------------------------------------------------- */
/* NavigationView */

/** @brief Display mode for NavigationView. */
typedef enum F2NavViewMode {
  F2_NAV_VIEW_MODE_LEFT = 0,     /**< Vertical, left-aligned. */
  F2_NAV_VIEW_MODE_LEFT_COMPACT, /**< Vertical, icons only. */
  F2_NAV_VIEW_MODE_TOP           /**< Horizontal, top-aligned. */
} F2NavViewMode;

struct F2NavigationView;

/** @brief NavigationView selection callback. */
typedef int(CMP_CALL *F2NavViewOnSelect)(void *ctx,
                                         struct F2NavigationView *nav,
                                         cmp_usize index);

/** @brief Fluent 2 NavigationView style descriptor. */
typedef struct F2NavViewStyle {
  F2NavViewMode mode;        /**< Display mode. */
  CMPColor background_color; /**< Container background. */
  CMPColor item_bg_normal;   /**< Normal item background. */
  CMPColor item_bg_hover;    /**< Hover item background. */
  CMPColor item_bg_pressed;  /**< Pressed item background. */
  CMPColor item_bg_selected; /**< Selected item background. */
  CMPColor text_normal;      /**< Normal text color. */
  CMPColor text_selected;    /**< Selected text color. */
  CMPColor indicator_color;  /**< Selection indicator bar color. */
  CMPTextStyle text_style;   /**< Label text style. */
  CMPScalar item_height;     /**< Height of each item. */
  CMPScalar compact_width;   /**< Width when in compact mode. */
  CMPScalar expanded_width;  /**< Width when expanded. */
} F2NavViewStyle;

/** @brief Fluent 2 NavigationView widget. */
typedef struct F2NavigationView {
  CMPWidget widget;            /**< Widget interface. */
  F2NavViewStyle style;        /**< Current style. */
  const F2NavItem *items;      /**< Array of items. */
  cmp_usize item_count;        /**< Item count. */
  cmp_usize selected_index;    /**< Selected item index. */
  CMPRect bounds;              /**< Layout bounds. */
  F2NavViewOnSelect on_select; /**< Selection callback. */
  void *on_select_ctx;         /**< Selection callback context. */
} F2NavigationView;

/**
 * @brief Initialize a default Fluent 2 NavigationView style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_navigation_view_style_init(F2NavViewStyle *style);

/**
 * @brief Initialize a Fluent 2 NavigationView widget.
 * @param nav NavigationView instance.
 * @param style Style descriptor.
 * @param items Array of navigation items.
 * @param count Number of items.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_navigation_view_init(F2NavigationView *nav,
                                             const F2NavViewStyle *style,
                                             const F2NavItem *items,
                                             cmp_usize count);

/**
 * @brief Set the selected index of the NavigationView.
 * @param nav NavigationView instance.
 * @param index Selected index.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_navigation_view_set_selected(F2NavigationView *nav,
                                                     cmp_usize index);

/**
 * @brief Set the selection callback of the NavigationView.
 * @param nav NavigationView instance.
 * @param on_select Selection callback.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_navigation_view_set_on_select(
    F2NavigationView *nav, F2NavViewOnSelect on_select, void *ctx);

/* -------------------------------------------------------------------------- */
/* TabList */

struct F2TabList;

/** @brief TabList selection callback. */
typedef int(CMP_CALL *F2TabListOnSelect)(void *ctx, struct F2TabList *tabs,
                                         cmp_usize index);

/** @brief Fluent 2 TabList style descriptor. */
typedef struct F2TabListStyle {
  CMPColor text_normal;       /**< Normal tab text. */
  CMPColor text_selected;     /**< Selected tab text. */
  CMPColor indicator_color;   /**< Animated bottom indicator color. */
  CMPColor bg_hover;          /**< Hover background. */
  CMPColor bg_pressed;        /**< Pressed background. */
  CMPTextStyle text_style;    /**< Tab text style. */
  CMPScalar tab_height;       /**< Height of the tab row. */
  CMPScalar indicator_height; /**< Thickness of the indicator line. */
  CMPScalar spacing;          /**< Spacing between tabs. */
} F2TabListStyle;

/** @brief Fluent 2 TabList widget. */
typedef struct F2TabList {
  CMPWidget widget;            /**< Widget interface. */
  F2TabListStyle style;        /**< Current style. */
  const F2NavItem *items;      /**< Array of tab items. */
  cmp_usize item_count;        /**< Tab count. */
  cmp_usize selected_index;    /**< Selected tab index. */
  CMPRect bounds;              /**< Layout bounds. */
  F2TabListOnSelect on_select; /**< Selection callback. */
  void *on_select_ctx;         /**< Selection callback context. */
} F2TabList;

/**
 * @brief Initialize a default Fluent 2 TabList style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tab_list_style_init(F2TabListStyle *style);

/**
 * @brief Initialize a Fluent 2 TabList widget.
 * @param tabs TabList instance.
 * @param style Style descriptor.
 * @param items Array of tab items.
 * @param count Number of items.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tab_list_init(F2TabList *tabs,
                                      const F2TabListStyle *style,
                                      const F2NavItem *items, cmp_usize count);

/**
 * @brief Set the selected tab index.
 * @param tabs TabList instance.
 * @param index Selected index.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tab_list_set_selected(F2TabList *tabs, cmp_usize index);

/**
 * @brief Set the selection callback of the TabList.
 * @param tabs TabList instance.
 * @param on_select Selection callback.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tab_list_set_on_select(F2TabList *tabs,
                                               F2TabListOnSelect on_select,
                                               void *ctx);

/* -------------------------------------------------------------------------- */
/* Breadcrumb */

struct F2Breadcrumb;

/** @brief Breadcrumb selection callback. */
typedef int(CMP_CALL *F2BreadcrumbOnSelect)(void *ctx, struct F2Breadcrumb *nav,
                                            cmp_usize index);

/** @brief Fluent 2 Breadcrumb style descriptor. */
typedef struct F2BreadcrumbStyle {
  CMPColor text_normal;    /**< Parent text color. */
  CMPColor text_current;   /**< Current (last) item text color. */
  CMPColor chevron_color;  /**< Separator icon color. */
  CMPColor bg_hover;       /**< Hover background for parents. */
  CMPTextStyle text_style; /**< Breadcrumb text style. */
  CMPScalar item_spacing;  /**< Space between text and chevron. */
} F2BreadcrumbStyle;

/** @brief Fluent 2 Breadcrumb widget. */
typedef struct F2Breadcrumb {
  CMPWidget widget;               /**< Widget interface. */
  F2BreadcrumbStyle style;        /**< Current style. */
  const F2NavItem *items;         /**< Array of items. */
  cmp_usize item_count;           /**< Item count. */
  CMPRect bounds;                 /**< Layout bounds. */
  F2BreadcrumbOnSelect on_select; /**< Selection callback. */
  void *on_select_ctx;            /**< Selection callback context. */
} F2Breadcrumb;

/**
 * @brief Initialize a default Fluent 2 Breadcrumb style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_breadcrumb_style_init(F2BreadcrumbStyle *style);

/**
 * @brief Initialize a Fluent 2 Breadcrumb widget.
 * @param nav Breadcrumb instance.
 * @param style Style descriptor.
 * @param items Array of breadcrumb items.
 * @param count Number of items.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_breadcrumb_init(F2Breadcrumb *nav,
                                        const F2BreadcrumbStyle *style,
                                        const F2NavItem *items,
                                        cmp_usize count);

/**
 * @brief Set the selection callback of the Breadcrumb.
 * @param nav Breadcrumb instance.
 * @param on_select Selection callback.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_breadcrumb_set_on_select(F2Breadcrumb *nav,
                                                 F2BreadcrumbOnSelect on_select,
                                                 void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_NAVIGATION_H */
