#include "f2/f2_data_components.h"
#include <string.h>

/* -------------------------------------------------------------------------- */
/* DataGrid / Table */

CMP_API int CMP_CALL f2_data_grid_style_init(F2DataGridStyle *style) {
    if (!style) return CMP_ERR_INVALID_ARGUMENT;
    memset(style, 0, sizeof(*style));
    
    style->row_height = F2_DATA_GRID_DEFAULT_ROW_HEIGHT;
    style->header_height = F2_DATA_GRID_DEFAULT_HEADER_HEIGHT;
    
    style->header_bg = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
    style->row_bg_normal = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
    style->row_bg_hover = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
    style->row_bg_selected = (CMPColor){0.90f, 0.95f, 1.0f, 1.0f};
    style->border_color = (CMPColor){0.9f, 0.9f, 0.9f, 1.0f};

    style->header_text.utf8_family = "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
    style->header_text.size_px = 14;
    style->header_text.weight = 600;
    
    style->cell_text.utf8_family = "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
    style->cell_text.size_px = 14;
    style->cell_text.weight = 400;

    return CMP_OK;
}

CMP_API int CMP_CALL f2_data_grid_init(F2DataGrid *grid, const F2DataGridStyle *style, cmp_usize columns, cmp_usize rows) {
    if (!grid || !style) return CMP_ERR_INVALID_ARGUMENT;
    memset(grid, 0, sizeof(*grid));
    grid->style = *style;
    grid->column_count = columns;
    grid->row_count = rows;
    grid->selected_row = (cmp_usize)-1;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_data_grid_set_selected(F2DataGrid *grid, cmp_usize row_index) {
    if (!grid) return CMP_ERR_INVALID_ARGUMENT;
    grid->selected_row = row_index;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_data_grid_set_on_select(F2DataGrid *grid, F2DataGridOnSelect on_select, void *ctx) {
    if (!grid) return CMP_ERR_INVALID_ARGUMENT;
    grid->on_select = on_select;
    grid->on_select_ctx = ctx;
    return CMP_OK;
}

/* -------------------------------------------------------------------------- */
/* ComboBox / Select */

CMP_API int CMP_CALL f2_combo_box_style_init(F2ComboBoxStyle *style) {
    if (!style) return CMP_ERR_INVALID_ARGUMENT;
    memset(style, 0, sizeof(*style));
    
    f2_text_field_style_init(&style->text_field);
    style->dropdown_bg = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
    style->item_hover_bg = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
    style->max_dropdown_height = 250.0f;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_combo_box_init(F2ComboBox *combo, const CMPTextBackend *backend, const F2ComboBoxStyle *style, CMPAllocator *alloc, cmp_usize items) {
    int res;
    if (!combo || !backend || !style || !alloc) return CMP_ERR_INVALID_ARGUMENT;
    memset(combo, 0, sizeof(*combo));
    combo->style = *style;
    combo->item_count = items;
    combo->selected_index = (cmp_usize)-1;
    combo->is_open = CMP_FALSE;

    res = f2_text_field_init(&combo->input_field, backend, &style->text_field, alloc);
    if (res != CMP_OK) return res;

    return CMP_OK;
}

CMP_API int CMP_CALL f2_combo_box_set_selected(F2ComboBox *combo, cmp_usize index) {
    if (!combo) return CMP_ERR_INVALID_ARGUMENT;
    combo->selected_index = index;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_combo_box_set_open(F2ComboBox *combo, CMPBool open) {
    if (!combo) return CMP_ERR_INVALID_ARGUMENT;
    combo->is_open = open;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_combo_box_set_on_select(F2ComboBox *combo, F2ComboBoxOnSelect on_select, void *ctx) {
    if (!combo) return CMP_ERR_INVALID_ARGUMENT;
    combo->on_select = on_select;
    combo->on_select_ctx = ctx;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_combo_box_cleanup(F2ComboBox *combo) {
    if (!combo) return CMP_ERR_INVALID_ARGUMENT;
    /* cmp_text_field cleanup does not exist */
    return CMP_OK;
}

/* -------------------------------------------------------------------------- */
/* Calendar / DatePicker */

CMP_API int CMP_CALL f2_date_picker_style_init(F2DatePickerStyle *style) {
    if (!style) return CMP_ERR_INVALID_ARGUMENT;
    memset(style, 0, sizeof(*style));
    
    style->header_bg = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
    style->day_bg_normal = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
    style->day_bg_hover = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
    style->day_bg_selected = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f};
    style->day_bg_today = (CMPColor){0.90f, 0.90f, 0.90f, 1.0f};
    
    style->header_text.utf8_family = "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
    style->header_text.size_px = 16;
    style->header_text.weight = 600;
    
    style->day_text.utf8_family = "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
    style->day_text.size_px = 14;
    style->day_text.weight = 400;
    
    style->cell_size = 40.0f;

    return CMP_OK;
}

CMP_API int CMP_CALL f2_date_picker_init(F2DatePicker *picker, const F2DatePickerStyle *style) {
    if (!picker || !style) return CMP_ERR_INVALID_ARGUMENT;
    memset(picker, 0, sizeof(*picker));
    picker->style = *style;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_date_picker_set_date(F2DatePicker *picker, cmp_u32 year, cmp_u8 month, cmp_u8 day) {
    if (!picker) return CMP_ERR_INVALID_ARGUMENT;
    picker->selected_year = year;
    picker->selected_month = month;
    picker->selected_day = day;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_date_picker_set_on_select(F2DatePicker *picker, F2DatePickerOnSelect on_select, void *ctx) {
    if (!picker) return CMP_ERR_INVALID_ARGUMENT;
    picker->on_select = on_select;
    picker->on_select_ctx = ctx;
    return CMP_OK;
}

/* -------------------------------------------------------------------------- */
/* TreeView */

CMP_API int CMP_CALL f2_tree_view_style_init(F2TreeViewStyle *style) {
    if (!style) return CMP_ERR_INVALID_ARGUMENT;
    memset(style, 0, sizeof(*style));
    
    style->item_bg_normal = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
    style->item_bg_hover = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
    style->item_bg_selected = (CMPColor){0.90f, 0.95f, 1.0f, 1.0f};
    
    style->text_style.utf8_family = "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
    style->text_style.size_px = 14;
    style->text_style.weight = 400;
    
    style->indent_width = 16.0f;
    style->item_height = 32.0f;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_tree_view_init(F2TreeView *tree, const F2TreeViewStyle *style) {
    if (!tree || !style) return CMP_ERR_INVALID_ARGUMENT;
    memset(tree, 0, sizeof(*tree));
    tree->style = *style;
    tree->selected_id = (cmp_usize)-1;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_tree_view_set_selected(F2TreeView *tree, cmp_usize item_id) {
    if (!tree) return CMP_ERR_INVALID_ARGUMENT;
    tree->selected_id = item_id;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_tree_view_set_callbacks(F2TreeView *tree,
                                                F2TreeViewOnSelect on_select, void *select_ctx,
                                                F2TreeViewOnExpand on_expand, void *expand_ctx) {
    if (!tree) return CMP_ERR_INVALID_ARGUMENT;
    tree->on_select = on_select;
    tree->on_select_ctx = select_ctx;
    tree->on_expand = on_expand;
    tree->on_expand_ctx = expand_ctx;
    return CMP_OK;
}
