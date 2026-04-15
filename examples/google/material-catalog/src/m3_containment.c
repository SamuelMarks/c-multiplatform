/* clang-format off */
#include "m3_containment.h"
#include "m3_color.h"
#include "m3_buttons.h"
#include <string.h>
#include <stdio.h>

static uint32_t color_to_hex(cmp_color_t color) {
    uint32_t r = (uint32_t)(color.r * 255.0f);
    uint32_t g = (uint32_t)(color.g * 255.0f);
    uint32_t b = (uint32_t)(color.b * 255.0f);
    uint32_t a = (uint32_t)(color.a * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

int m3_card_create(material_catalog_state_t* state, const m3_card_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* card;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&card) != 0) {
        return -1;
    }
    
    switch (config->type) {
        case M3_CARD_TYPE_ELEVATED:
            card->bg_color_ref = &state->sys_colors_hex.surface_container_low;
            /* Elevation shadow usually mapped via style property in real CMP */
            /* card->style->box_shadow_blur = dp_to_px(state, 1.0f); */
            break;
        case M3_CARD_TYPE_FILLED:
            card->bg_color_ref = &state->sys_colors_hex.surface_container_highest;
            /* 0dp elevation */
            break;
        case M3_CARD_TYPE_OUTLINED:
            card->bg_color_ref = &state->sys_colors_hex.surface;
            /* card->style->border_width = dp_to_px(state, 1.0f);
               card->style->border_color_ref = &state->sys_colors_hex.outline; */
            break;
    }

    /* M3 Medium shape for cards by default is 12dp */
    /* card->style->border_radius = dp_to_px(state, 12.0f); */
    
    card->layout->direction = CMP_FLEX_COLUMN;
    card->layout->padding[0] = dp_to_px(state, 16.0f); /* Top */
    card->layout->padding[1] = dp_to_px(state, 16.0f); /* Right */
    card->layout->padding[2] = dp_to_px(state, 16.0f); /* Bottom */
    card->layout->padding[3] = dp_to_px(state, 16.0f); /* Left */

    if (config->on_click) {
        cmp_ui_node_add_event_listener(card, CMP_EVENT_TYPE_MOUSE, 1 /* ACTION_CLICK mock */, config->on_click, config->user_data);
    }
    
    /* interactive states: hover lift, drag elevation increase would be attached as event listeners managing layout/style state */
    if (config->is_draggable) {
        /* attach pointer down/up/move for dragging, mutating elevation */
    }

    *out_node = card;
    return 0;
}

int m3_divider_create(material_catalog_state_t* state, const m3_divider_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* divider;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&divider) != 0) {
        return -1;
    }
    
    divider->bg_color_ref = &state->sys_colors_hex.outline_variant;

    if (config->is_vertical) {
        divider->layout->width = dp_to_px(state, 1.0f);
        divider->layout->height = -1.0f; /* Fill height */
        divider->layout->flex_grow = 1.0f;
        
        if (config->type != M3_DIVIDER_TYPE_FULL_WIDTH) {
            divider->layout->margin[0] = dp_to_px(state, config->inset_start_dp);
            divider->layout->margin[2] = dp_to_px(state, config->inset_end_dp);
        }
    } else {
        divider->layout->height = dp_to_px(state, 1.0f);
        divider->layout->width = -1.0f; /* Fill width */
        divider->layout->flex_grow = 1.0f;
        
        if (config->type != M3_DIVIDER_TYPE_FULL_WIDTH) {
            divider->layout->margin[3] = dp_to_px(state, config->inset_start_dp); /* Left */
            divider->layout->margin[1] = dp_to_px(state, config->inset_end_dp); /* Right */
        }
    }

    *out_node = divider;
    return 0;
}

int m3_dialog_create(material_catalog_state_t* state, const m3_dialog_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* dialog;
    cmp_ui_node_t* content_col = NULL;
    cmp_ui_node_t* actions_row = NULL;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&dialog) != 0) {
        return -1;
    }
    
    dialog->bg_color_ref = &state->sys_colors_hex.surface_container_high;
    /* 3rd level elevation, 28dp extra large shape */
    
    dialog->layout->direction = CMP_FLEX_COLUMN;
    dialog->layout->padding[0] = dp_to_px(state, 24.0f);
    dialog->layout->padding[1] = dp_to_px(state, 24.0f);
    dialog->layout->padding[2] = dp_to_px(state, 24.0f);
    dialog->layout->padding[3] = dp_to_px(state, 24.0f);

    if (config->type == M3_DIALOG_TYPE_FULL_SCREEN) {
        dialog->layout->width = state->window_width;
        dialog->layout->height = state->window_height;
        dialog->layout->position_type = CMP_POSITION_ABSOLUTE;
        dialog->layout->position[0] = 0;
        dialog->layout->position[3] = 0;
    } else {
        dialog->layout->min_width = dp_to_px(state, 280.0f);
        dialog->layout->max_width = dp_to_px(state, 560.0f);
        dialog->layout->align_self = CMP_FLEX_ALIGN_CENTER;
    }

    cmp_ui_box_create(&content_col);
    content_col->layout->direction = CMP_FLEX_COLUMN;
    content_col->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    
    if (config->icon) {
        cmp_ui_node_t* icon_node;
        if (cmp_ui_text_create(&icon_node, config->icon, (int)strlen(config->icon)) == 0) {
            icon_node->text_color_ref = &state->sys_colors_hex.secondary;
            icon_node->layout->margin[2] = dp_to_px(state, 16.0f); /* Bottom margin */
            cmp_ui_node_add_child(content_col, icon_node);
        }
    }

    if (config->headline) {
        cmp_ui_node_t* title_node;
        if (cmp_ui_text_create(&title_node, config->headline, (int)strlen(config->headline)) == 0) {
            title_node->text_color_ref = &state->sys_colors_hex.on_surface;
            title_node->font_size = dp_to_px(state, 24.0f); /* Headline Small */
            title_node->layout->margin[2] = dp_to_px(state, 16.0f);
            cmp_ui_node_add_child(content_col, title_node);
        }
    }

    if (config->supporting_text) {
        cmp_ui_node_t* text_node;
        if (cmp_ui_text_create(&text_node, config->supporting_text, (int)strlen(config->supporting_text)) == 0) {
            text_node->text_color_ref = &state->sys_colors_hex.on_surface_variant;
            text_node->font_size = dp_to_px(state, 14.0f); /* Body Medium */
            cmp_ui_node_add_child(content_col, text_node);
        }
    }
    
    cmp_ui_node_add_child(dialog, content_col);

    /* Actions row */
    cmp_ui_box_create(&actions_row);
    actions_row->layout->direction = CMP_FLEX_ROW;
    actions_row->layout->justify_content = CMP_FLEX_ALIGN_END; /* Actions right-aligned */
    actions_row->layout->margin[0] = dp_to_px(state, 24.0f); /* Top margin */
    actions_row->layout->width = -1.0f; /* 100% */
    
    if (config->dismiss_label) {
        cmp_ui_node_t* dismiss_btn;
        m3_button_config_t btn_cfg;
        memset(&btn_cfg, 0, sizeof(btn_cfg));
        btn_cfg.type = M3_BUTTON_TYPE_TEXT;
        btn_cfg.label = config->dismiss_label;
        btn_cfg.on_click = (m3_event_cb_t)config->on_dismiss;
        btn_cfg.user_data = config->user_data;
        if (m3_button_create(state, &btn_cfg, &dismiss_btn) == 0) {
            cmp_ui_node_add_child(actions_row, dismiss_btn);
        }
    }

    if (config->confirm_label) {
        cmp_ui_node_t* confirm_btn;
        m3_button_config_t btn_cfg;
        memset(&btn_cfg, 0, sizeof(btn_cfg));
        btn_cfg.type = M3_BUTTON_TYPE_TEXT;
        btn_cfg.label = config->confirm_label;
        btn_cfg.on_click = (m3_event_cb_t)config->on_confirm;
        btn_cfg.user_data = config->user_data;
        if (m3_button_create(state, &btn_cfg, &confirm_btn) == 0) {
            cmp_ui_node_add_child(actions_row, confirm_btn);
        }
    }
    
    cmp_ui_node_add_child(dialog, actions_row);

    *out_node = dialog;
    return 0;
}

int m3_bottom_sheet_create(material_catalog_state_t* state, const m3_bottom_sheet_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* sheet;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&sheet) != 0) {
        return -1;
    }
    
    sheet->bg_color_ref = &state->sys_colors_hex.surface_container_low;
    /* 28dp top corners, 0dp bottom */
    
    sheet->layout->direction = CMP_FLEX_COLUMN;
    sheet->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    sheet->layout->width = -1.0f; /* Full width generally on mobile, bounded on tablet */
    sheet->layout->position_type = CMP_POSITION_ABSOLUTE;
    sheet->layout->position[2] = 0.0f; /* Anchored to bottom */
    sheet->layout->position[3] = 0.0f; /* Left 0 */

    if (config->type == M3_BOTTOM_SHEET_TYPE_MODAL) {
        /* Scrim rendering and fling velocity to dismiss is managed outside by the engine/router, 
           but we create the container node here. Modal implies it sits in an overlay container. */
        sheet->bg_color_ref = &state->sys_colors_hex.surface_container_low;
    }

    if (config->show_drag_handle) {
        cmp_ui_node_t* handle;
        if (cmp_ui_box_create(&handle) == 0) {
            handle->bg_color_ref = &state->sys_colors_hex.on_surface_variant;
            handle->layout->width = dp_to_px(state, 32.0f);
            handle->layout->height = dp_to_px(state, 4.0f);
            handle->layout->margin[0] = dp_to_px(state, 22.0f); /* Top margin */
            handle->layout->margin[2] = dp_to_px(state, 22.0f); /* Bottom margin */
            /* Pill shape via style */
            cmp_ui_node_add_child(sheet, handle);
        }
    }

    *out_node = sheet;
    return 0;
}
/* clang-format on */
