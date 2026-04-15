/* clang-format off */
#include "m3_navigation.h"
#include "m3_color.h"
#include <string.h>

static uint32_t color_to_hex(cmp_color_t color) {
    uint32_t r = (uint32_t)(color.r * 255.0f);
    uint32_t g = (uint32_t)(color.g * 255.0f);
    uint32_t b = (uint32_t)(color.b * 255.0f);
    uint32_t a = (uint32_t)(color.a * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

int m3_top_app_bar_create(material_catalog_state_t* state, const m3_top_app_bar_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* bar;
    cmp_ui_node_t* left_box = NULL;
    cmp_ui_node_t* right_box = NULL;
    cmp_ui_node_t* title_node;
    int i;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&bar) != 0) {
        return -1;
    }
    
    if (config->is_scrolled) {
        bar->bg_color_ref = &state->sys_colors_hex.surface_container;
    } else {
        bar->bg_color_ref = &state->sys_colors_hex.surface;
    }
    
    bar->layout->direction = CMP_FLEX_ROW;
    bar->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
    bar->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    bar->layout->width = -1.0f; /* 100% */
    
    switch (config->type) {
        case M3_TOP_APP_BAR_TYPE_SMALL:
        case M3_TOP_APP_BAR_TYPE_CENTER_ALIGNED:
            bar->layout->height = dp_to_px(state, 64.0f);
            break;
        case M3_TOP_APP_BAR_TYPE_MEDIUM:
            bar->layout->height = config->is_scrolled ? dp_to_px(state, 64.0f) : dp_to_px(state, 112.0f);
            break;
        case M3_TOP_APP_BAR_TYPE_LARGE:
            bar->layout->height = config->is_scrolled ? dp_to_px(state, 64.0f) : dp_to_px(state, 152.0f);
            break;
    }

    cmp_ui_box_create(&left_box);
    left_box->layout->direction = CMP_FLEX_ROW;
    left_box->layout->align_items = CMP_FLEX_ALIGN_CENTER;

    if (config->nav_icon) {
        cmp_ui_node_t* nav_node;
        if (cmp_ui_text_create(&nav_node, config->nav_icon, (int)strlen(config->nav_icon)) == 0) {
            nav_node->text_color_ref = &state->sys_colors_hex.on_surface;
            nav_node->layout->padding[3] = dp_to_px(state, 16.0f);
            nav_node->layout->padding[1] = dp_to_px(state, 16.0f);
            if (config->on_nav_click) {
                cmp_ui_node_add_event_listener(nav_node, CMP_EVENT_TYPE_MOUSE, 1, config->on_nav_click, config->user_data);
            }
            cmp_ui_node_add_child(left_box, nav_node);
        }
    }

    if (config->type != M3_TOP_APP_BAR_TYPE_CENTER_ALIGNED && config->title) {
        if (cmp_ui_text_create(&title_node, config->title, (int)strlen(config->title)) == 0) {
            title_node->text_color_ref = &state->sys_colors_hex.on_surface;
            title_node->font_size = dp_to_px(state, config->type == M3_TOP_APP_BAR_TYPE_SMALL || config->is_scrolled ? 22.0f : 28.0f);
            if (!config->nav_icon) title_node->layout->padding[3] = dp_to_px(state, 16.0f);
            cmp_ui_node_add_child(left_box, title_node);
        }
    }

    cmp_ui_node_add_child(bar, left_box);

    if (config->type == M3_TOP_APP_BAR_TYPE_CENTER_ALIGNED && config->title) {
        if (cmp_ui_text_create(&title_node, config->title, (int)strlen(config->title)) == 0) {
            title_node->text_color_ref = &state->sys_colors_hex.on_surface;
            title_node->font_size = dp_to_px(state, 22.0f);
            title_node->layout->position_type = CMP_POSITION_ABSOLUTE;
            /* Mock centering via layout engine */
            cmp_ui_node_add_child(bar, title_node);
        }
    }

    cmp_ui_box_create(&right_box);
    right_box->layout->direction = CMP_FLEX_ROW;
    right_box->layout->align_items = CMP_FLEX_ALIGN_CENTER;

    if (config->action_icons && config->action_count > 0) {
        for (i = 0; i < config->action_count; ++i) {
            cmp_ui_node_t* act_node;
            if (cmp_ui_text_create(&act_node, config->action_icons[i], (int)strlen(config->action_icons[i])) == 0) {
                act_node->text_color_ref = &state->sys_colors_hex.on_surface_variant;
                act_node->layout->padding[3] = dp_to_px(state, 12.0f);
                if (i == config->action_count - 1) {
                    act_node->layout->padding[1] = dp_to_px(state, 16.0f);
                }
                cmp_ui_node_add_child(right_box, act_node);
            }
        }
    }
    
    cmp_ui_node_add_child(bar, right_box);

    *out_node = bar;
    return 0;
}

int m3_bottom_app_bar_create(material_catalog_state_t* state, const m3_bottom_app_bar_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* bar;
    int i;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&bar) != 0) {
        return -1;
    }
    
    bar->bg_color_ref = &state->sys_colors_hex.surface_container;
    bar->layout->height = dp_to_px(state, 80.0f);
    bar->layout->width = -1.0f;
    bar->layout->direction = CMP_FLEX_ROW;
    bar->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    bar->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
    bar->layout->padding[3] = dp_to_px(state, 16.0f);
    bar->layout->padding[1] = dp_to_px(state, 16.0f);

    if (config->action_icons) {
        cmp_ui_node_t* act_box = NULL;
        cmp_ui_box_create(&act_box);
        act_box->layout->direction = CMP_FLEX_ROW;
        for (i = 0; i < config->action_count; ++i) {
            cmp_ui_node_t* icon;
            if (cmp_ui_text_create(&icon, config->action_icons[i], (int)strlen(config->action_icons[i])) == 0) {
                icon->text_color_ref = &state->sys_colors_hex.on_surface_variant;
                icon->layout->padding[1] = dp_to_px(state, 24.0f);
                cmp_ui_node_add_child(act_box, icon);
            }
        }
        cmp_ui_node_add_child(bar, act_box);
    }

    if (config->has_fab) {
        cmp_ui_node_t* fab;
        m3_fab_config_t fcfg;
        memset(&fcfg, 0, sizeof(fcfg));
        fcfg.size = M3_FAB_SIZE_STANDARD;
        fcfg.icon = "F";
        fcfg.is_lowered = 1;
        if (m3_fab_create(state, &fcfg, &fab) == 0) {
            cmp_ui_node_add_child(bar, fab);
        }
    }

    *out_node = bar;
    return 0;
}

int m3_navigation_bar_create(material_catalog_state_t* state, const m3_navigation_bar_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* bar;
    int i;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&bar) != 0) {
        return -1;
    }
    
    bar->bg_color_ref = &state->sys_colors_hex.surface_container;
    bar->layout->height = dp_to_px(state, 80.0f);
    bar->layout->width = -1.0f;
    bar->layout->direction = CMP_FLEX_ROW;
    bar->layout->justify_content = CMP_FLEX_ALIGN_SPACE_AROUND;
    bar->layout->align_items = CMP_FLEX_ALIGN_CENTER;

    for (i = 0; i < config->item_count; ++i) {
        cmp_ui_node_t* item_box = NULL;
        cmp_ui_node_t* icon_box = NULL;
        cmp_ui_node_t* label;
        int is_selected = (i == config->selected_index);
        
        cmp_ui_box_create(&item_box);
        item_box->layout->direction = CMP_FLEX_COLUMN;
        item_box->layout->align_items = CMP_FLEX_ALIGN_CENTER;
        item_box->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
        item_box->layout->flex_grow = 1.0f;

        cmp_ui_box_create(&icon_box);
        icon_box->layout->width = dp_to_px(state, 64.0f);
        icon_box->layout->height = dp_to_px(state, 32.0f);
        icon_box->layout->align_items = CMP_FLEX_ALIGN_CENTER;
        icon_box->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
        
        if (is_selected) {
            icon_box->bg_color_ref = &state->sys_colors_hex.secondary_container;
            /* radius 16dp */
        }
        
        if (config->items[i].icon) {
            cmp_ui_node_t* icon_txt;
            const char* ico_str = is_selected && config->items[i].active_icon ? config->items[i].active_icon : config->items[i].icon;
            if (cmp_ui_text_create(&icon_txt, ico_str, (int)strlen(ico_str)) == 0) {
                icon_txt->text_color_ref = is_selected ? &state->sys_colors_hex.on_secondary_container : &state->sys_colors_hex.on_surface_variant;
                cmp_ui_node_add_child(icon_box, icon_txt);
            }
        }
        
        cmp_ui_node_add_child(item_box, icon_box);

        if (!config->hide_labels) {
            if (cmp_ui_text_create(&label, config->items[i].label, (int)strlen(config->items[i].label)) == 0) {
                label->text_color_ref = is_selected ? &state->sys_colors_hex.on_surface : &state->sys_colors_hex.on_surface_variant;
                label->font_size = dp_to_px(state, 12.0f); /* Label medium */
                label->layout->margin[0] = dp_to_px(state, 4.0f); /* Top margin */
                cmp_ui_node_add_child(item_box, label);
            }
        }

        /* Mock click wrapper, no direct int passing in C89 callbacks cleanly without alloc, so we skip exact binding in mock */
        
        cmp_ui_node_add_child(bar, item_box);
    }

    *out_node = bar;
    return 0;
}

int m3_navigation_rail_create(material_catalog_state_t* state, const m3_navigation_rail_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* rail;
    int i;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&rail) != 0) {
        return -1;
    }
    
    rail->bg_color_ref = &state->sys_colors_hex.surface;
    rail->layout->width = dp_to_px(state, 80.0f);
    rail->layout->height = -1.0f;
    rail->layout->direction = CMP_FLEX_COLUMN;
    rail->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    
    if (config->alignment == M3_NAV_RAIL_ALIGN_CENTER) {
        rail->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
    } else if (config->alignment == M3_NAV_RAIL_ALIGN_BOTTOM) {
        rail->layout->justify_content = CMP_FLEX_ALIGN_END;
    } else {
        rail->layout->justify_content = CMP_FLEX_ALIGN_START;
    }

    if (config->has_menu) {
        cmp_ui_node_t* menu;
        if (cmp_ui_text_create(&menu, "M", 1) == 0) {
            menu->text_color_ref = &state->sys_colors_hex.on_surface_variant;
            menu->layout->margin[0] = dp_to_px(state, 24.0f);
            cmp_ui_node_add_child(rail, menu);
        }
    }

    if (config->has_fab) {
        cmp_ui_node_t* fab;
        m3_fab_config_t fcfg;
        memset(&fcfg, 0, sizeof(fcfg));
        fcfg.size = M3_FAB_SIZE_STANDARD;
        fcfg.icon = "+";
        if (m3_fab_create(state, &fcfg, &fab) == 0) {
            fab->layout->margin[0] = dp_to_px(state, 24.0f);
            cmp_ui_node_add_child(rail, fab);
        }
    }

    for (i = 0; i < config->item_count; ++i) {
        cmp_ui_node_t* item_box = NULL;
        int is_selected = (i == config->selected_index);
        
        cmp_ui_box_create(&item_box);
        item_box->layout->direction = CMP_FLEX_COLUMN;
        item_box->layout->align_items = CMP_FLEX_ALIGN_CENTER;
        item_box->layout->margin[0] = dp_to_px(state, 12.0f);

        /* Icon pill */
        if (config->items[i].icon) {
            cmp_ui_node_t* icon_box = NULL;
            cmp_ui_node_t* icon_txt;
            cmp_ui_box_create(&icon_box);
            icon_box->layout->width = dp_to_px(state, 56.0f);
            icon_box->layout->height = dp_to_px(state, 32.0f);
            icon_box->layout->align_items = CMP_FLEX_ALIGN_CENTER;
            icon_box->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
            
            if (is_selected) {
                icon_box->bg_color_ref = &state->sys_colors_hex.secondary_container;
            }
            
            if (cmp_ui_text_create(&icon_txt, config->items[i].icon, (int)strlen(config->items[i].icon)) == 0) {
                icon_txt->text_color_ref = is_selected ? &state->sys_colors_hex.on_secondary_container : &state->sys_colors_hex.on_surface_variant;
                cmp_ui_node_add_child(icon_box, icon_txt);
            }
            cmp_ui_node_add_child(item_box, icon_box);
        }

        if (config->items[i].label) {
            cmp_ui_node_t* label;
            if (cmp_ui_text_create(&label, config->items[i].label, (int)strlen(config->items[i].label)) == 0) {
                label->text_color_ref = is_selected ? &state->sys_colors_hex.on_surface : &state->sys_colors_hex.on_surface_variant;
                label->font_size = dp_to_px(state, 12.0f);
                label->layout->margin[0] = dp_to_px(state, 4.0f);
                cmp_ui_node_add_child(item_box, label);
            }
        }
        
        cmp_ui_node_add_child(rail, item_box);
    }

    *out_node = rail;
    return 0;
}

int m3_navigation_drawer_create(material_catalog_state_t* state, const m3_navigation_drawer_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* drawer;
    int i;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&drawer) != 0) {
        return -1;
    }
    
    drawer->bg_color_ref = &state->sys_colors_hex.surface_container_low;
    drawer->layout->width = dp_to_px(state, 360.0f);
    drawer->layout->height = -1.0f;
    drawer->layout->direction = CMP_FLEX_COLUMN;
    drawer->layout->padding[0] = dp_to_px(state, 24.0f); /* Top */
    drawer->layout->padding[3] = dp_to_px(state, 12.0f); /* Left */
    drawer->layout->padding[1] = dp_to_px(state, 12.0f); /* Right */

    if (config->headline) {
        cmp_ui_node_t* head;
        if (cmp_ui_text_create(&head, config->headline, (int)strlen(config->headline)) == 0) {
            head->text_color_ref = &state->sys_colors_hex.on_surface_variant;
            head->font_size = dp_to_px(state, 14.0f);
            head->layout->margin[3] = dp_to_px(state, 16.0f);
            head->layout->margin[2] = dp_to_px(state, 16.0f);
            cmp_ui_node_add_child(drawer, head);
        }
    }

    for (i = 0; i < config->item_count; ++i) {
        cmp_ui_node_t* item_box = NULL;
        int is_selected = (i == config->selected_index);
        
        cmp_ui_box_create(&item_box);
        item_box->layout->height = dp_to_px(state, 56.0f);
        item_box->layout->width = -1.0f;
        item_box->layout->direction = CMP_FLEX_ROW;
        item_box->layout->align_items = CMP_FLEX_ALIGN_CENTER;
        item_box->layout->padding[3] = dp_to_px(state, 16.0f);
        
        if (is_selected) {
            item_box->bg_color_ref = &state->sys_colors_hex.secondary_container;
            /* shape full (pill) */
        }
        
        if (config->items[i].icon) {
            cmp_ui_node_t* icon_txt;
            if (cmp_ui_text_create(&icon_txt, config->items[i].icon, (int)strlen(config->items[i].icon)) == 0) {
                icon_txt->text_color_ref = is_selected ? &state->sys_colors_hex.on_secondary_container : &state->sys_colors_hex.on_surface_variant;
                icon_txt->layout->margin[1] = dp_to_px(state, 12.0f);
                cmp_ui_node_add_child(item_box, icon_txt);
            }
        }
        
        if (config->items[i].label) {
            cmp_ui_node_t* label;
            if (cmp_ui_text_create(&label, config->items[i].label, (int)strlen(config->items[i].label)) == 0) {
                label->text_color_ref = is_selected ? &state->sys_colors_hex.on_surface : &state->sys_colors_hex.on_surface_variant;
                label->font_size = dp_to_px(state, 14.0f);
                cmp_ui_node_add_child(item_box, label);
            }
        }
        
        cmp_ui_node_add_child(drawer, item_box);
    }

    *out_node = drawer;
    return 0;
}

int m3_tabs_create(material_catalog_state_t* state, const m3_tabs_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* tabs;
    int i;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&tabs) != 0) {
        return -1;
    }
    
    tabs->bg_color_ref = &state->sys_colors_hex.surface;
    tabs->layout->direction = CMP_FLEX_ROW;
    tabs->layout->width = -1.0f;
    tabs->layout->height = dp_to_px(state, 48.0f);
    
    /* Lower border indicating track */
    /* style->border_bottom_width = dp_to_px(state, 1.0f); */
    
    for (i = 0; i < config->tab_count; ++i) {
        cmp_ui_node_t* tab = NULL;
        int is_selected = (i == config->selected_index);
        
        cmp_ui_box_create(&tab);
        tab->layout->direction = CMP_FLEX_COLUMN;
        tab->layout->align_items = CMP_FLEX_ALIGN_CENTER;
        tab->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
        
        if (config->is_scrollable) {
            tab->layout->padding[3] = dp_to_px(state, 16.0f);
            tab->layout->padding[1] = dp_to_px(state, 16.0f);
            tab->layout->min_width = dp_to_px(state, 90.0f);
        } else {
            tab->layout->flex_grow = 1.0f;
        }

        if (config->tab_icons && config->tab_icons[i]) {
            cmp_ui_node_t* icon;
            if (cmp_ui_text_create(&icon, config->tab_icons[i], (int)strlen(config->tab_icons[i])) == 0) {
                icon->text_color_ref = is_selected ? &state->sys_colors_hex.primary : &state->sys_colors_hex.on_surface_variant;
                cmp_ui_node_add_child(tab, icon);
            }
            tabs->layout->height = dp_to_px(state, 64.0f); /* Taller if icons present */
        }

        if (config->tab_labels && config->tab_labels[i]) {
            cmp_ui_node_t* label;
            if (cmp_ui_text_create(&label, config->tab_labels[i], (int)strlen(config->tab_labels[i])) == 0) {
                label->text_color_ref = is_selected ? &state->sys_colors_hex.primary : &state->sys_colors_hex.on_surface_variant;
                label->font_size = dp_to_px(state, 14.0f);
                cmp_ui_node_add_child(tab, label);
            }
        }
        
        if (is_selected) {
            /* Active indicator */
            cmp_ui_node_t* indicator;
            if (cmp_ui_box_create(&indicator) == 0) {
                indicator->bg_color_ref = &state->sys_colors_hex.primary;
                indicator->layout->height = dp_to_px(state, config->type == M3_TABS_TYPE_PRIMARY ? 3.0f : 2.0f);
                indicator->layout->position_type = CMP_POSITION_ABSOLUTE;
                indicator->layout->position[2] = 0.0f; /* Bottom */
                
                if (config->type == M3_TABS_TYPE_PRIMARY) {
                    indicator->layout->width = dp_to_px(state, 40.0f); /* Mock wrap-content width */
                } else {
                    indicator->layout->width = -1.0f; /* Full width */
                }
                cmp_ui_node_add_child(tab, indicator);
            }
        }

        cmp_ui_node_add_child(tabs, tab);
    }

    *out_node = tabs;
    return 0;
}
/* clang-format on */
