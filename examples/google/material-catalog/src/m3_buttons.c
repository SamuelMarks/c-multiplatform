/* clang-format off */
#include "m3_buttons.h"
#include "m3_color.h"
#include <string.h>

static uint32_t color_to_hex(cmp_color_t color) {
    uint32_t r = (uint32_t)(color.r * 255.0f);
    uint32_t g = (uint32_t)(color.g * 255.0f);
    uint32_t b = (uint32_t)(color.b * 255.0f);
    uint32_t a = (uint32_t)(color.a * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static void set_button_colors(cmp_ui_node_t* node, const uint32_t *bg_ref, const uint32_t *fg_ref) {
    if (node) {
        node->bg_color_ref = bg_ref;
        node->text_color_ref = fg_ref;
    }
}

int m3_button_create(material_catalog_state_t* state, const m3_button_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* btn;
    const uint32_t *bg_ref = NULL; const uint32_t *fg_ref = NULL; const uint32_t *border_ref = NULL;
    int res;

    if (!state || !config || !out_node) {
        return -1;
    }

    res = cmp_ui_button_create(&btn, "", 0);
    if (res != 0) {
        return res;
    }

    /* Minimum touch target 48x48dp, height is usually 40dp for standard button */
    btn->layout->min_width = dp_to_px(state, 48.0f);
    btn->layout->min_height = dp_to_px(state, 48.0f);
    btn->layout->height = dp_to_px(state, 40.0f);
    
    /* Layout */
    btn->layout->padding[3] = dp_to_px(state, 24.0f); /* Left */
    btn->layout->padding[1] = dp_to_px(state, 24.0f); /* Right */

    /* Leading icon */
    if (config->leading_icon) {
        cmp_ui_node_t* icon_node;
        if (cmp_ui_image_view_create(&icon_node, config->leading_icon) == 0) {
            icon_node->layout->width = dp_to_px(state, 18.0f);
            icon_node->layout->height = dp_to_px(state, 18.0f);
            icon_node->layout->margin[1] = dp_to_px(state, 8.0f); /* Right margin */
            cmp_ui_node_add_child(btn, icon_node);
        }
        btn->layout->padding[3] = dp_to_px(state, 16.0f); /* Reduce left padding when leading icon is present */
    }

    /* Text */
    if (config->label) {
        cmp_ui_node_t* text_node;
        int len = (int)strlen(config->label);
        if (cmp_ui_text_create(&text_node, config->label, len) == 0) {
            cmp_ui_node_add_child(btn, text_node);
        }
    }

    /* Trailing icon */
    if (config->trailing_icon) {
        cmp_ui_node_t* icon_node;
        if (cmp_ui_image_view_create(&icon_node, config->trailing_icon) == 0) {
            icon_node->layout->width = dp_to_px(state, 18.0f);
            icon_node->layout->height = dp_to_px(state, 18.0f);
            icon_node->layout->margin[3] = dp_to_px(state, 8.0f); /* Left margin */
            cmp_ui_node_add_child(btn, icon_node);
        }
        btn->layout->padding[1] = dp_to_px(state, 16.0f); /* Reduce right padding when trailing icon is present */
    }

    /* Colors and border */

    
    if (config->is_disabled) {
        fg_ref = &state->sys_colors_hex.on_surface;
        btn->opacity = 0.38f;

        if (config->type == M3_BUTTON_TYPE_FILLED || config->type == M3_BUTTON_TYPE_ELEVATED || config->type == M3_BUTTON_TYPE_FILLED_TONAL) {
            bg_ref = &state->sys_colors_hex.on_surface;
            /* For background, ideally 12%. Opacity covers the whole node, so it is a compromise */
        } else {
            bg_ref = NULL;
        }
    } else {
        switch (config->type) {
            case M3_BUTTON_TYPE_ELEVATED:
                bg_ref = &state->sys_colors_hex.surface_container_low;
                fg_ref = &state->sys_colors_hex.primary;
                btn->elevation = 1.0f; /* 1dp elevation */
                break;
            case M3_BUTTON_TYPE_FILLED:
                bg_ref = &state->sys_colors_hex.primary;
                fg_ref = &state->sys_colors_hex.on_primary;
                break;
            case M3_BUTTON_TYPE_FILLED_TONAL:
                bg_ref = &state->sys_colors_hex.secondary_container;
                fg_ref = &state->sys_colors_hex.on_secondary_container;
                break;
            case M3_BUTTON_TYPE_OUTLINED:
                bg_ref = NULL;
                fg_ref = &state->sys_colors_hex.primary;
                border_ref = &state->sys_colors_hex.outline;
                btn->border_width = 1.0f;
                btn->border_color_ref = border_ref;
                break;
            case M3_BUTTON_TYPE_TEXT:
                bg_ref = NULL;
                fg_ref = &state->sys_colors_hex.primary;
                btn->layout->padding[3] = dp_to_px(state, 12.0f);
                btn->layout->padding[1] = dp_to_px(state, 12.0f);
                break;
        }
    }
btn->border_radius = 20.0f; /* Fully rounded corners for standard buttons (40dp height / 2) */

    set_button_colors(btn, bg_ref, fg_ref);

    if (config->on_click && !config->is_disabled) {
        cmp_ui_node_add_event_listener(btn, CMP_EVENT_TYPE_MOUSE, 1 /* CMP_ACTION_CLICK substitute, CMP_ACTION_UP usually 2, let's just use 0 or something valid */, config->on_click, config->user_data);
    }

    *out_node = btn;
    return 0;
}

int m3_fab_create(material_catalog_state_t* state, const m3_fab_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* fab;
    float size_px;

    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_button_create(&fab, "", 0) != 0) {
        return -1;
    }

    switch (config->size) {
        case M3_FAB_SIZE_SMALL:
            size_px = dp_to_px(state, 40.0f);
            fab->border_radius = 12.0f;
            break;
        case M3_FAB_SIZE_LARGE:
            size_px = dp_to_px(state, 96.0f);
            fab->border_radius = 28.0f;
            break;
        case M3_FAB_SIZE_STANDARD:
        default:
            size_px = dp_to_px(state, 56.0f);
            fab->border_radius = 16.0f;
            break;
    }

    fab->layout->width = config->label ? -1.0f /* auto */ : size_px;
    fab->layout->height = size_px;
    fab->layout->min_width = size_px;
    fab->layout->min_height = size_px;
    fab->elevation = 3.0f; /* 3dp resting elevation */
    
    set_button_colors(fab, &state->sys_colors_hex.primary_container, &state->sys_colors_hex.on_primary_container);

    if (config->label) {
        cmp_ui_node_t* text_node;
        int len = (int)strlen(config->label);
        if (cmp_ui_text_create(&text_node, config->label, len) == 0) {
            cmp_ui_node_add_child(fab, text_node);
        }
        fab->layout->padding[3] = dp_to_px(state, 16.0f);
        fab->layout->padding[1] = dp_to_px(state, 20.0f);
    }

    if (config->on_click) {
        cmp_ui_node_add_event_listener(fab, CMP_EVENT_TYPE_MOUSE, 1, config->on_click, config->user_data);
    }

    *out_node = fab;
    return 0;
}

int m3_icon_button_create(material_catalog_state_t* state, m3_button_type_t type, const char* icon, int is_disabled, m3_event_cb_t on_click, void* user_data, cmp_ui_node_t** out_node) {
    m3_button_config_t config;
    int res;
    
    memset(&config, 0, sizeof(config));
    config.type = type;
    config.leading_icon = icon;
    config.is_disabled = is_disabled;
    config.on_click = on_click;
    config.user_data = user_data;
    
    res = m3_button_create(state, &config, out_node);
    if (res == 0) {
        (*out_node)->layout->padding[3] = dp_to_px(state, 8.0f);
        (*out_node)->layout->padding[1] = dp_to_px(state, 8.0f);
        (*out_node)->layout->width = dp_to_px(state, 40.0f);
        (*out_node)->layout->height = dp_to_px(state, 40.0f);
    }
    return res;
}

int m3_segmented_button_create(material_catalog_state_t* state, const char** segments, int segment_count, int is_multi_select, m3_event_cb_t on_segment_changed, void* user_data, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* group;
    int i;
    
    if (!state || !segments || segment_count <= 0 || !out_node) {
        return -1;
    }
    
    if (cmp_ui_box_create(&group) != 0) {
        return -1;
    }
    
    for (i = 0; i < segment_count; ++i) {
        cmp_ui_node_t* btn;
        m3_button_config_t config;
        memset(&config, 0, sizeof(config));
        config.type = M3_BUTTON_TYPE_TEXT;
        config.label = segments[i];
        config.on_click = on_segment_changed;
        config.user_data = user_data ? user_data : (void*)(uintptr_t)i; 
        
        if (m3_button_create(state, &config, &btn) == 0) {
            cmp_ui_node_add_child(group, btn);
        }
    }
    
    (void)is_multi_select;
    
    *out_node = group;
    return 0;
}
/* clang-format on */