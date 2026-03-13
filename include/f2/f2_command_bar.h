#ifndef F2_COMMAND_BAR_H
#define F2_COMMAND_BAR_H

/**
 * @file f2_command_bar.h
 * @brief Microsoft Fluent 2 CommandBar widget.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "f2/f2_button.h"
#include "f2/f2_color.h"
/* clang-format on */

/** @brief Default CommandBar height. */
#define F2_COMMAND_BAR_DEFAULT_HEIGHT 44.0f
/** @brief Default padding around commands. */
#define F2_COMMAND_BAR_DEFAULT_PADDING 4.0f
/** @brief Maximum primary commands displayed before overflow. */
#define F2_COMMAND_BAR_MAX_PRIMARY_ITEMS 10
/** @brief Maximum secondary commands (overflow menu). */
#define F2_COMMAND_BAR_MAX_SECONDARY_ITEMS 20

/**
 * @brief CommandBar item descriptor.
 */
typedef struct F2CommandBarItem {
  cmp_u32 id;             /**< Command ID (user defined). */
  const char *utf8_label; /**< Text label. */
  const char *utf8_icon;  /**< Icon identifier. */
  CMPBool disabled;       /**< Whether the command is disabled. */
} F2CommandBarItem;

struct F2CommandBar;

/**
 * @brief Command action callback signature.
 * @param ctx User callback context.
 * @param bar CommandBar instance.
 * @param command_id The ID of the command clicked.
 * @return CMP_OK on success.
 */
typedef int(CMP_CALL *F2CommandBarOnAction)(void *ctx, struct F2CommandBar *bar,
                                            cmp_u32 command_id);

/**
 * @brief Fluent 2 CommandBar style descriptor.
 */
typedef struct F2CommandBarStyle {
  CMPColor background_color; /**< Bar background color. */
  CMPColor border_color;     /**< Bottom/divider border color. */
  CMPScalar height;          /**< Fixed height of the bar. */
  CMPScalar padding;         /**< Padding inside the bar. */
  F2ButtonStyle item_style;  /**< Style applied to primary buttons. */
} F2CommandBarStyle;

/**
 * @brief Fluent 2 CommandBar widget instance.
 */
typedef struct F2CommandBar {
  CMPWidget widget;                      /**< Widget interface. */
  F2CommandBarStyle style;               /**< Current style. */
  const F2CommandBarItem *primary_items; /**< Primary items array. */
  cmp_usize primary_count;               /**< Number of primary items. */
  const F2CommandBarItem
      *secondary_items;           /**< Secondary (overflow) items array. */
  cmp_usize secondary_count;      /**< Number of secondary items. */
  CMPRect bounds;                 /**< Layout bounds. */
  F2CommandBarOnAction on_action; /**< Action callback. */
  void *on_action_ctx;            /**< Callback context. */
} F2CommandBar;

/**
 * @brief Initialize a default Fluent 2 command bar style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_command_bar_style_init(F2CommandBarStyle *style);

/**
 * @brief Initialize a Fluent 2 command bar widget.
 * @param bar CommandBar instance.
 * @param style Style descriptor.
 * @param primary_items Array of primary command items.
 * @param primary_count Number of primary items.
 * @param secondary_items Array of secondary (overflow) items.
 * @param secondary_count Number of secondary items.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_command_bar_init(
    F2CommandBar *bar, const F2CommandBarStyle *style,
    const F2CommandBarItem *primary_items, cmp_usize primary_count,
    const F2CommandBarItem *secondary_items, cmp_usize secondary_count);

/**
 * @brief Set the action callback for the command bar.
 * @param bar CommandBar instance.
 * @param on_action Action callback.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_command_bar_set_on_action(
    F2CommandBar *bar, F2CommandBarOnAction on_action, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_COMMAND_BAR_H */
