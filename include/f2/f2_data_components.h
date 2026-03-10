#ifndef F2_DATA_COMPONENTS_H
#define F2_DATA_COMPONENTS_H

/**
 * @file f2_data_components.h
 * @brief Microsoft Fluent 2 DataGrid, ComboBox, DatePicker, and TreeView
 * widgets.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_color.h"
#include "f2/f2_text_field.h"
/* clang-format on */

/* -------------------------------------------------------------------------- */
/* DataGrid / Table */

/** @brief Default DataGrid row height. */
#define F2_DATA_GRID_DEFAULT_ROW_HEIGHT 44.0f
/** @brief Default DataGrid header height. */
#define F2_DATA_GRID_DEFAULT_HEADER_HEIGHT 44.0f

/** @brief Fluent 2 DataGrid style. */
typedef struct F2DataGridStyle {
  CMPColor header_bg;       /**< Header row background color. */
  CMPColor row_bg_normal;   /**< Normal row background color. */
  CMPColor row_bg_hover;    /**< Hover row background color. */
  CMPColor row_bg_selected; /**< Selected row background color. */
  CMPColor border_color;    /**< Grid border / divider color. */
  CMPTextStyle header_text; /**< Header text style. */
  CMPTextStyle cell_text;   /**< Cell text style. */
  CMPScalar row_height;     /**< Height of each data row. */
  CMPScalar header_height;  /**< Height of the header row. */
} F2DataGridStyle;

struct F2DataGrid;

/** @brief DataGrid row selection callback. */
typedef int(CMP_CALL *F2DataGridOnSelect)(void *ctx, struct F2DataGrid *grid,
                                          cmp_usize row_index);

/** @brief Fluent 2 DataGrid widget. */
typedef struct F2DataGrid {
  CMPWidget widget;             /**< Widget interface. */
  F2DataGridStyle style;        /**< Current style. */
  cmp_usize column_count;       /**< Number of columns. */
  cmp_usize row_count;          /**< Total number of data rows. */
  cmp_usize selected_row;       /**< Currently selected row. */
  CMPRect bounds;               /**< Layout bounds. */
  F2DataGridOnSelect on_select; /**< Row selection callback. */
  void *on_select_ctx;          /**< Context for callback. */
} F2DataGrid;

/**
 * @brief Initialize a default Fluent 2 DataGrid style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_data_grid_style_init(F2DataGridStyle *style);

/**
 * @brief Initialize a Fluent 2 DataGrid widget.
 * @param grid DataGrid instance.
 * @param style Style descriptor.
 * @param columns Number of columns.
 * @param rows Number of rows.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_data_grid_init(F2DataGrid *grid,
                                       const F2DataGridStyle *style,
                                       cmp_usize columns, cmp_usize rows);

/**
 * @brief Set the selected row index.
 * @param grid DataGrid instance.
 * @param row_index Index of the selected row.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_data_grid_set_selected(F2DataGrid *grid,
                                               cmp_usize row_index);

/**
 * @brief Set the row selection callback.
 * @param grid DataGrid instance.
 * @param on_select Callback function.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_data_grid_set_on_select(F2DataGrid *grid,
                                                F2DataGridOnSelect on_select,
                                                void *ctx);

/* -------------------------------------------------------------------------- */
/* ComboBox / Select */

/** @brief Fluent 2 ComboBox style. */
typedef struct F2ComboBoxStyle {
  F2TextFieldStyle text_field;   /**< Input field styling. */
  CMPColor dropdown_bg;          /**< Background for the dropdown list. */
  CMPColor item_hover_bg;        /**< Hover background for list items. */
  CMPScalar max_dropdown_height; /**< Max height before scrolling. */
} F2ComboBoxStyle;

struct F2ComboBox;

/** @brief ComboBox selection callback. */
typedef int(CMP_CALL *F2ComboBoxOnSelect)(void *ctx, struct F2ComboBox *combo,
                                          cmp_usize index);

/** @brief Fluent 2 ComboBox widget. */
typedef struct F2ComboBox {
  CMPWidget widget;             /**< Widget interface. */
  F2ComboBoxStyle style;        /**< Current style. */
  F2TextField input_field;      /**< Inner text field. */
  cmp_usize item_count;         /**< Number of items in list. */
  cmp_usize selected_index;     /**< Current selected index. */
  CMPBool is_open;              /**< True if dropdown is visible. */
  CMPRect bounds;               /**< Layout bounds. */
  F2ComboBoxOnSelect on_select; /**< Selection callback. */
  void *on_select_ctx;          /**< Callback context. */
} F2ComboBox;

/**
 * @brief Initialize a default Fluent 2 ComboBox style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_combo_box_style_init(F2ComboBoxStyle *style);

/**
 * @brief Initialize a Fluent 2 ComboBox widget.
 * @param combo ComboBox instance.
 * @param backend Text backend.
 * @param style Style descriptor.
 * @param alloc Memory allocator.
 * @param items Number of items.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_combo_box_init(F2ComboBox *combo,
                                       const CMPTextBackend *backend,
                                       const F2ComboBoxStyle *style,
                                       CMPAllocator *alloc, cmp_usize items);

/**
 * @brief Set the selected item index.
 * @param combo ComboBox instance.
 * @param index Index of the selected item.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_combo_box_set_selected(F2ComboBox *combo,
                                               cmp_usize index);

/**
 * @brief Set the open state of the dropdown.
 * @param combo ComboBox instance.
 * @param open Open state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_combo_box_set_open(F2ComboBox *combo, CMPBool open);

/**
 * @brief Set the selection callback.
 * @param combo ComboBox instance.
 * @param on_select Callback function.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_combo_box_set_on_select(F2ComboBox *combo,
                                                F2ComboBoxOnSelect on_select,
                                                void *ctx);

/**
 * @brief Clean up the ComboBox widget.
 * @param combo ComboBox instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_combo_box_cleanup(F2ComboBox *combo);

/* -------------------------------------------------------------------------- */
/* Calendar / DatePicker */

/** @brief Fluent 2 DatePicker style. */
typedef struct F2DatePickerStyle {
  CMPColor header_bg;       /**< Background of the month/year header. */
  CMPColor day_bg_normal;   /**< Normal day background. */
  CMPColor day_bg_hover;    /**< Hover day background. */
  CMPColor day_bg_selected; /**< Selected day background. */
  CMPColor day_bg_today;    /**< Current day background. */
  CMPTextStyle header_text; /**< Header text style. */
  CMPTextStyle day_text;    /**< Day cell text style. */
  CMPScalar cell_size;      /**< Size of a day cell (width & height). */
} F2DatePickerStyle;

struct F2DatePicker;

/** @brief DatePicker selection callback. */
typedef int(CMP_CALL *F2DatePickerOnSelect)(void *ctx,
                                            struct F2DatePicker *picker,
                                            cmp_u32 year, cmp_u8 month,
                                            cmp_u8 day);

/** @brief Fluent 2 DatePicker widget. */
typedef struct F2DatePicker {
  CMPWidget widget;               /**< Widget interface. */
  F2DatePickerStyle style;        /**< Current style. */
  cmp_u32 selected_year;          /**< Selected year. */
  cmp_u8 selected_month;          /**< Selected month (1-12). */
  cmp_u8 selected_day;            /**< Selected day (1-31). */
  CMPRect bounds;                 /**< Layout bounds. */
  F2DatePickerOnSelect on_select; /**< Selection callback. */
  void *on_select_ctx;            /**< Callback context. */
} F2DatePicker;

/**
 * @brief Initialize a default Fluent 2 DatePicker style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_date_picker_style_init(F2DatePickerStyle *style);

/**
 * @brief Initialize a Fluent 2 DatePicker widget.
 * @param picker DatePicker instance.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_date_picker_init(F2DatePicker *picker,
                                         const F2DatePickerStyle *style);

/**
 * @brief Set the selected date.
 * @param picker DatePicker instance.
 * @param year Year.
 * @param month Month.
 * @param day Day.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_date_picker_set_date(F2DatePicker *picker, cmp_u32 year,
                                             cmp_u8 month, cmp_u8 day);

/**
 * @brief Set the selection callback.
 * @param picker DatePicker instance.
 * @param on_select Callback function.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_date_picker_set_on_select(
    F2DatePicker *picker, F2DatePickerOnSelect on_select, void *ctx);

/* -------------------------------------------------------------------------- */
/* TreeView */

/** @brief Fluent 2 TreeView style. */
typedef struct F2TreeViewStyle {
  CMPColor item_bg_normal;   /**< Normal item background. */
  CMPColor item_bg_hover;    /**< Hover item background. */
  CMPColor item_bg_selected; /**< Selected item background. */
  CMPTextStyle text_style;   /**< Item text style. */
  CMPScalar indent_width;    /**< Pixels to indent per depth level. */
  CMPScalar item_height;     /**< Height of each tree item. */
} F2TreeViewStyle;

struct F2TreeView;

/** @brief TreeView selection callback. */
typedef int(CMP_CALL *F2TreeViewOnSelect)(void *ctx, struct F2TreeView *tree,
                                          cmp_usize item_id);
/** @brief TreeView expand/collapse callback. */
typedef int(CMP_CALL *F2TreeViewOnExpand)(void *ctx, struct F2TreeView *tree,
                                          cmp_usize item_id, CMPBool expanded);

/** @brief Fluent 2 TreeView widget. */
typedef struct F2TreeView {
  CMPWidget widget;             /**< Widget interface. */
  F2TreeViewStyle style;        /**< Current style. */
  cmp_usize selected_id;        /**< Currently selected item ID. */
  CMPRect bounds;               /**< Layout bounds. */
  F2TreeViewOnSelect on_select; /**< Selection callback. */
  void *on_select_ctx;          /**< Selection context. */
  F2TreeViewOnExpand on_expand; /**< Expand/collapse callback. */
  void *on_expand_ctx;          /**< Expand/collapse context. */
} F2TreeView;

/**
 * @brief Initialize a default Fluent 2 TreeView style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tree_view_style_init(F2TreeViewStyle *style);

/**
 * @brief Initialize a Fluent 2 TreeView widget.
 * @param tree TreeView instance.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tree_view_init(F2TreeView *tree,
                                       const F2TreeViewStyle *style);

/**
 * @brief Set the selected item ID.
 * @param tree TreeView instance.
 * @param item_id Item ID.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tree_view_set_selected(F2TreeView *tree,
                                               cmp_usize item_id);

/**
 * @brief Set the tree view callbacks.
 * @param tree TreeView instance.
 * @param on_select Selection callback.
 * @param select_ctx Selection context.
 * @param on_expand Expand/collapse callback.
 * @param expand_ctx Expand/collapse context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tree_view_set_callbacks(F2TreeView *tree,
                                                F2TreeViewOnSelect on_select,
                                                void *select_ctx,
                                                F2TreeViewOnExpand on_expand,
                                                void *expand_ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_DATA_COMPONENTS_H */
