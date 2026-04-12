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

static void set_button_colors(cmp_ui_node_t* node, cmp_color_t bg, cmp_color_t fg) {
    if (node) {
        node->bg_color = color_to_hex(bg);
        node->text_color = color_to_hex(fg);
    }
}

int m3_button_create(material_catalog_state_t* state, const m3_button_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* btn;
    cmp_color_t bg = {0}, fg = {0}, border = {0};
    m3_color_roles_t roles;
    int is_dark;
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

    /* Text */
    if (config->label) {
        cmp_ui_node_t* text_node;
        int len = (int)strlen(config->label);
        if (cmp_ui_text_create(&text_node, config->label, len) == 0) {
            cmp_ui_node_add_child(btn, text_node);
        }
    }

    /* Colors and border */
    is_dark = (state->current_theme == CATALOG_THEME_DARK) ? 1 : 0;
    {
        cmp_color_t seed = {0.4f, 0.2f, 0.8f, 1.0f, CMP_COLOR_SPACE_SRGB};
        m3_color_generate_roles(seed, is_dark, &roles);
    }

    if (config->is_disabled) {
        fg = roles.on_surface;
        fg.a = 0.38f;
        
        if (config->type == M3_BUTTON_TYPE_FILLED || config->type == M3_BUTTON_TYPE_ELEVATED || config->type == M3_BUTTON_TYPE_FILLED_TONAL) {
            bg = roles.on_surface;
            bg.a = 0.12f;
        } else {
            bg.a = 0.0f;
        }
    } else {
        switch (config->type) {
            case M3_BUTTON_TYPE_ELEVATED:
                bg = roles.surface_container_low;
                fg = roles.primary;
                btn->elevation = 1.0f; /* 1dp elevation */
                break;
            case M3_BUTTON_TYPE_FILLED:
                bg = roles.primary;
                fg = roles.on_primary;
                break;
            case M3_BUTTON_TYPE_FILLED_TONAL:
                bg = roles.secondary_container;
                fg = roles.on_secondary_container;
                break;
            case M3_BUTTON_TYPE_OUTLINED:
                bg.a = 0.0f;
                fg = roles.primary;
                border = roles.outline;
                btn->border_width = 1.0f;
                btn->border_color = color_to_hex(border);
                break;
            case M3_BUTTON_TYPE_TEXT:
                bg.a = 0.0f;
                fg = roles.primary;
                btn->layout->padding[3] = dp_to_px(state, 12.0f);
                btn->layout->padding[1] = dp_to_px(state, 12.0f);
                break;
        }
    }

    btn->border_radius = 20.0f; /* Fully rounded corners for standard buttons (40dp height / 2) */

    set_button_colors(btn, bg, fg);

    if (config->on_click && !config->is_disabled) {
        cmp_ui_node_add_event_listener(btn, CMP_EVENT_TYPE_MOUSE, 1 /* CMP_ACTION_CLICK substitute, CMP_ACTION_UP usually 2, let's just use 0 or something valid */, config->on_click, config->user_data);
    }

    *out_node = btn;
    return 0;
}

int m3_fab_create(material_catalog_state_t* state, const m3_fab_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* fab;
    float size_px;
    m3_color_roles_t roles;
    int is_dark;

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
    
    is_dark = (state->current_theme == CATALOG_THEME_DARK) ? 1 : 0;
    {
        cmp_color_t seed = {0.4f, 0.2f, 0.8f, 1.0f, CMP_COLOR_SPACE_SRGB};
        m3_color_generate_roles(seed, is_dark, &roles);
    }
    
    set_button_colors(fab, roles.primary_container, roles.on_primary_container);

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
        config.user_data = user_data; 
        
        if (m3_button_create(state, &config, &btn) == 0) {
            cmp_ui_node_add_child(group, btn);
        }
    }
    
    (void)is_multi_select;
    
    *out_node = group;
    return 0;
}
/* clang-format on */