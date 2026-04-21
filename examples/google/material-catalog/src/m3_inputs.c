/* clang-format off */
#include "m3_inputs.h"
#include "m3_color.h"
#include <string.h>

static uint32_t color_to_hex(cmp_color_t color) {
    uint32_t r = (uint32_t)(color.r * 255.0f);
    uint32_t g = (uint32_t)(color.g * 255.0f);
    uint32_t b = (uint32_t)(color.b * 255.0f);
    uint32_t a = (uint32_t)(color.a * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

int m3_checkbox_create(material_catalog_state_t* state, const m3_checkbox_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* box;
    cmp_ui_node_t* check_icon;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&box) != 0) {
        return -1;
    }
    
    /* Min touch target 48x48, visual is 18x18 */
    box->layout->width = dp_to_px(state, 18.0f);
    box->layout->height = dp_to_px(state, 18.0f);
    box->layout->margin[0] = dp_to_px(state, 15.0f);
    box->layout->margin[1] = dp_to_px(state, 15.0f);
    box->layout->margin[2] = dp_to_px(state, 15.0f);
    box->layout->margin[3] = dp_to_px(state, 15.0f);
    box->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    box->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
    box->border_radius = 2.0f; /* 2dp corner radius for checkboxes */

    if (config->is_disabled) {
        box->bg_color_ref = &state->sys_colors_hex.on_surface; /* Actually 38% outline, but mock */
        if (config->state == M3_CHECKBOX_STATE_CHECKED || config->state == M3_CHECKBOX_STATE_INDETERMINATE) {
            box->bg_color_ref = &state->sys_colors_hex.on_surface; /* 38% filled */
            box->opacity = 0.38f;
            box->border_width = 0.0f;
        } else {
            box->bg_color = 0; /* transparent */
            box->border_width = 2.0f;
            box->border_color_ref = &state->sys_colors_hex.on_surface;
            box->opacity = 0.38f;
        }
    } else if (config->is_error) {
        if (config->state == M3_CHECKBOX_STATE_UNCHECKED) {
            box->bg_color = 0;
            box->border_width = 2.0f;
            box->border_color_ref = &state->sys_colors_hex.error;
        } else {
            box->bg_color_ref = &state->sys_colors_hex.error;
            box->border_width = 0.0f;
        }
    } else {
        if (config->state == M3_CHECKBOX_STATE_UNCHECKED) {
            box->bg_color = 0;
            box->border_width = 2.0f;
            box->border_color_ref = &state->sys_colors_hex.on_surface_variant;
        } else {
            box->bg_color_ref = &state->sys_colors_hex.primary;
            box->border_width = 0.0f;
        }
    }

    if (config->state != M3_CHECKBOX_STATE_UNCHECKED) {
        /* Use SVG checkmark instead of 'v' text */
        if (cmp_ui_image_view_create(&check_icon, config->state == M3_CHECKBOX_STATE_CHECKED ? "vfs://assets/ic_check.svg" : "vfs://assets/ic_minus.svg") == 0) {
            check_icon->layout->width = dp_to_px(state, 14.0f);
            check_icon->layout->height = dp_to_px(state, 14.0f);
            
            if (config->is_disabled) {
                check_icon->text_color_ref = &state->sys_colors_hex.surface;
            } else if (config->is_error) {
                check_icon->text_color_ref = &state->sys_colors_hex.on_error;
            } else {
                check_icon->text_color_ref = &state->sys_colors_hex.on_primary;
            }
            cmp_ui_node_add_child(box, check_icon);
        }
    }

    /* Wrap checkbox in a 48x48 touch target for ripples */
    {
      cmp_ui_node_t *touch_target;
      if (cmp_ui_box_create(&touch_target) == 0) {
        touch_target->layout->width = dp_to_px(state, 48.0f);
        touch_target->layout->height = dp_to_px(state, 48.0f);
        touch_target->layout->align_items = CMP_FLEX_ALIGN_CENTER;
        touch_target->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
        touch_target->border_radius = 24.0f; /* Circular ripple */
        box->layout->margin[0] = 0; box->layout->margin[1] = 0;
        box->layout->margin[2] = 0; box->layout->margin[3] = 0;
        
        cmp_ui_node_add_child(touch_target, box);
        
        if (config->on_toggle && !config->is_disabled) {
            cmp_ui_node_add_event_listener(touch_target, CMP_EVENT_TYPE_MOUSE, 1, config->on_toggle, config->user_data);
            material_catalog_apply_ripple(state, touch_target);
        }
        *out_node = touch_target;
        return 0;
      }
    }

    if (config->on_toggle && !config->is_disabled) {
        cmp_ui_node_add_event_listener(box, CMP_EVENT_TYPE_MOUSE, 1, config->on_toggle, config->user_data);
    }

    *out_node = box;
    return 0;
}

int m3_radio_button_create(material_catalog_state_t* state, const m3_radio_button_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* rb;
    cmp_ui_node_t* inner;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&rb) != 0) {
        return -1;
    }
    
    rb->layout->width = dp_to_px(state, 20.0f);
    rb->layout->height = dp_to_px(state, 20.0f);
    rb->layout->margin[0] = dp_to_px(state, 14.0f);
    rb->layout->margin[1] = dp_to_px(state, 14.0f);
    rb->layout->margin[2] = dp_to_px(state, 14.0f);
    rb->layout->margin[3] = dp_to_px(state, 14.0f);
    rb->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    rb->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
    rb->bg_color = 0;

    if (config->is_selected) {
        if (cmp_ui_box_create(&inner) == 0) {
            inner->layout->width = dp_to_px(state, 10.0f);
            inner->layout->height = dp_to_px(state, 10.0f);
            if (config->is_disabled) {
                inner->bg_color_ref = &state->sys_colors_hex.on_surface; /* 38% */
            } else {
                inner->bg_color_ref = &state->sys_colors_hex.primary;
            }
            cmp_ui_node_add_child(rb, inner);
        }
    }

    if (config->on_select && !config->is_disabled) {
        cmp_ui_node_add_event_listener(rb, CMP_EVENT_TYPE_MOUSE, 1, config->on_select, config->user_data);
    }

    *out_node = rb;
    return 0;
}

int m3_switch_create(material_catalog_state_t* state, const m3_switch_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* track;
    cmp_ui_node_t* thumb;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&track) != 0) {
        return -1;
    }
    
    track->layout->width = dp_to_px(state, 52.0f);
    track->layout->height = dp_to_px(state, 32.0f);
    track->layout->direction = CMP_FLEX_ROW;
    track->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    /* justify content varies if on or off */
    track->layout->justify_content = config->is_on ? CMP_FLEX_ALIGN_END : CMP_FLEX_ALIGN_START;
    track->border_radius = 16.0f; /* Circular ends (32 / 2) */

    if (config->is_disabled) {
        track->bg_color_ref = &state->sys_colors_hex.surface_container_highest; /* 12% alpha */
        track->opacity = 0.12f;
        track->border_width = config->is_on ? 0.0f : 2.0f;
        track->border_color_ref = &state->sys_colors_hex.on_surface; /* 12% alpha */
    } else {
        track->bg_color_ref = config->is_on ? &state->sys_colors_hex.primary : &state->sys_colors_hex.surface_container_highest;
        track->border_width = config->is_on ? 0.0f : 2.0f;
        track->border_color_ref = &state->sys_colors_hex.outline;
    }

    if (cmp_ui_box_create(&thumb) == 0) {
        float size = config->is_on ? 24.0f : (config->show_icon ? 24.0f : 16.0f);
        thumb->layout->width = dp_to_px(state, size);
        thumb->layout->height = dp_to_px(state, size);
        thumb->border_radius = size * 0.5f; /* Circular thumb */
        thumb->layout->margin[0] = 0;
        thumb->layout->margin[2] = 0;

        /* The margins for the switch track */
        if (config->is_on) {
          thumb->layout->margin[1] = dp_to_px(state, 4.0f); /* Right margin */
          thumb->layout->margin[3] = dp_to_px(state, 0.0f);
        } else {
          thumb->layout->margin[3] = dp_to_px(state, 8.0f); /* Left margin, larger for unselected to clear border */
          thumb->layout->margin[1] = dp_to_px(state, 0.0f);
        }

        if (config->is_disabled) {
            thumb->bg_color_ref = config->is_on ? &state->sys_colors_hex.surface : &state->sys_colors_hex.on_surface; /* 38% alpha */
            thumb->opacity = config->is_on ? 1.0f : 0.38f;
        } else {
            thumb->bg_color_ref = config->is_on ? &state->sys_colors_hex.on_primary : &state->sys_colors_hex.outline;
        }

        if (config->show_icon && config->is_on) {
            cmp_ui_node_t* icon;
            if (cmp_ui_image_view_create(&icon, "vfs://assets/ic_check.svg") == 0) {
                icon->text_color_ref = config->is_disabled ? &state->sys_colors_hex.on_surface : &state->sys_colors_hex.on_primary_container;
                icon->layout->width = dp_to_px(state, 16.0f);
                icon->layout->height = dp_to_px(state, 16.0f);
                thumb->layout->align_items = CMP_FLEX_ALIGN_CENTER;
                thumb->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
                cmp_ui_node_add_child(thumb, icon);
            }
        }

        cmp_ui_node_add_child(track, thumb);
    }

    /* Wrap in touch target */
    {
      cmp_ui_node_t *touch_target;
      if (cmp_ui_box_create(&touch_target) == 0) {
        touch_target->layout->width = dp_to_px(state, 52.0f); /* Actually touch target should be 48x48 min */
        if (touch_target->layout->width < dp_to_px(state, 48.0f)) touch_target->layout->width = dp_to_px(state, 48.0f);
        touch_target->layout->height = dp_to_px(state, 48.0f);
        touch_target->layout->align_items = CMP_FLEX_ALIGN_CENTER;
        touch_target->layout->justify_content = CMP_FLEX_ALIGN_CENTER;

        /* The ripple should be circular and centered on the thumb */
        /* Since we can't easily position the ripple explicitly right now, we will apply it to the whole container */
        touch_target->border_radius = dp_to_px(state, 24.0f);

        cmp_ui_node_add_child(touch_target, track);

        if (config->on_toggle && !config->is_disabled) {
            cmp_ui_node_add_event_listener(touch_target, CMP_EVENT_TYPE_MOUSE, 1, config->on_toggle, config->user_data);
            material_catalog_apply_ripple(state, touch_target);
        }
        *out_node = touch_target;
        return 0;
      }
    }

    if (config->on_toggle && !config->is_disabled) {
        cmp_ui_node_add_event_listener(track, CMP_EVENT_TYPE_MOUSE, 1, config->on_toggle, config->user_data);
    }

    *out_node = track;
    return 0;
    }
int m3_chip_create(material_catalog_state_t* state, const m3_chip_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* chip;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&chip) != 0) {
        return -1;
    }
    
    chip->layout->height = dp_to_px(state, 32.0f);
    chip->layout->direction = CMP_FLEX_ROW;
    chip->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    
    if (config->is_elevated) {
        chip->bg_color_ref = &state->sys_colors_hex.surface_container_low;
    } else {
        chip->bg_color_ref = config->is_selected ? &state->sys_colors_hex.secondary_container : &state->sys_colors_hex.surface;
    }

    /* Padding */
    chip->layout->padding[3] = dp_to_px(state, config->leading_icon || (config->type == M3_CHIP_TYPE_FILTER && config->is_selected) ? 8.0f : 16.0f);
    chip->layout->padding[1] = dp_to_px(state, config->trailing_icon ? 8.0f : 16.0f);

    /* Leading Icon or Checkmark */
    if (config->leading_icon || (config->type == M3_CHIP_TYPE_FILTER && config->is_selected)) {
        cmp_ui_node_t* lead;
        const char* str = config->leading_icon ? config->leading_icon : "v";
        if (cmp_ui_text_create(&lead, str, (int)strlen(str)) == 0) {
            lead->text_color_ref = config->is_selected ? &state->sys_colors_hex.on_secondary_container : &state->sys_colors_hex.primary;
            lead->layout->margin[1] = dp_to_px(state, 8.0f); /* Right margin */
            cmp_ui_node_add_child(chip, lead);
        }
    }

    /* Label */
    if (config->label) {
        cmp_ui_node_t* text;
        if (cmp_ui_text_create(&text, config->label, (int)strlen(config->label)) == 0) {
            text->text_color_ref = config->is_selected ? &state->sys_colors_hex.on_secondary_container : &state->sys_colors_hex.on_surface;
            text->font_size = dp_to_px(state, 14.0f);
            cmp_ui_node_add_child(chip, text);
        }
    }

    /* Trailing Icon */
    if (config->trailing_icon) {
        cmp_ui_node_t* trail;
        if (cmp_ui_text_create(&trail, config->trailing_icon, (int)strlen(config->trailing_icon)) == 0) {
            trail->text_color_ref = &state->sys_colors_hex.on_surface_variant;
            trail->layout->margin[3] = dp_to_px(state, 8.0f); /* Left margin */
            if (config->on_trailing_click && !config->is_disabled) {
                cmp_ui_node_add_event_listener(trail, CMP_EVENT_TYPE_MOUSE, 1, config->on_trailing_click, config->user_data);
            }
            cmp_ui_node_add_child(chip, trail);
        }
    }

    if (config->on_click && !config->is_disabled) {
        cmp_ui_node_add_event_listener(chip, CMP_EVENT_TYPE_MOUSE, 1, config->on_click, config->user_data);
    }

    *out_node = chip;
    return 0;
}

int m3_slider_create(material_catalog_state_t* state, const m3_slider_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* slider;
    cmp_ui_node_t* track;
    cmp_ui_node_t* thumb;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&slider) != 0) {
        return -1;
    }
    
    slider->layout->height = dp_to_px(state, 48.0f); /* Touch target */
    slider->layout->width = -1.0f;
    slider->layout->direction = CMP_FLEX_ROW;
    slider->layout->align_items = CMP_FLEX_ALIGN_CENTER;

    if (cmp_ui_box_create(&track) == 0) {
        track->layout->height = dp_to_px(state, 16.0f); /* Inactive track height */
        track->layout->width = -1.0f;
        track->bg_color_ref = &state->sys_colors_hex.surface_container_highest;
        
        if (cmp_ui_box_create(&thumb) == 0) {
            thumb->layout->width = dp_to_px(state, 20.0f);
            thumb->layout->height = dp_to_px(state, 20.0f);
            thumb->bg_color_ref = &state->sys_colors_hex.primary;
            /* Positioning thumb via margins or absolute is complex without full Flexbox support active.
               We'll inject it into the track. */
            cmp_ui_node_add_child(track, thumb);
        }
        
        cmp_ui_node_add_child(slider, track);
    }

    *out_node = slider;
    return 0;
}

int m3_range_slider_create(material_catalog_state_t* state, const m3_range_slider_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* slider;
    cmp_ui_node_t* track;
    cmp_ui_node_t* thumb1;
    cmp_ui_node_t* thumb2;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&slider) != 0) {
        return -1;
    }
    
    slider->layout->height = dp_to_px(state, 48.0f); /* Touch target */
    slider->layout->width = -1.0f;
    slider->layout->direction = CMP_FLEX_ROW;
    slider->layout->align_items = CMP_FLEX_ALIGN_CENTER;

    if (cmp_ui_box_create(&track) == 0) {
        track->layout->height = dp_to_px(state, 16.0f);
        track->layout->width = -1.0f;
        track->bg_color_ref = &state->sys_colors_hex.surface_container_highest;
        
        if (cmp_ui_box_create(&thumb1) == 0) {
            thumb1->layout->width = dp_to_px(state, 20.0f);
            thumb1->layout->height = dp_to_px(state, 20.0f);
            thumb1->bg_color_ref = &state->sys_colors_hex.primary;
            cmp_ui_node_add_child(track, thumb1);
        }
        
        if (cmp_ui_box_create(&thumb2) == 0) {
            thumb2->layout->width = dp_to_px(state, 20.0f);
            thumb2->layout->height = dp_to_px(state, 20.0f);
            thumb2->bg_color_ref = &state->sys_colors_hex.primary;
            cmp_ui_node_add_child(track, thumb2);
        }
        
        cmp_ui_node_add_child(slider, track);
    }

    *out_node = slider;
    return 0;
}

int m3_text_field_create(material_catalog_state_t* state, const m3_text_field_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* field;
    cmp_ui_node_t* content_row = NULL;
    cmp_ui_node_t* text_col = NULL;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&field) != 0) {
        return -1;
    }
    
    field->layout->direction = CMP_FLEX_COLUMN;
    field->layout->width = -1.0f;

    cmp_ui_box_create(&content_row);
    content_row->layout->height = dp_to_px(state, 56.0f);
    content_row->layout->direction = CMP_FLEX_ROW;
    content_row->layout->align_items = CMP_FLEX_ALIGN_CENTER;

    if (config->type == M3_TEXT_FIELD_TYPE_FILLED) {
        content_row->bg_color_ref = &state->sys_colors_hex.surface_container_highest;
        content_row->layout->padding[3] = dp_to_px(state, 16.0f);
        content_row->layout->padding[1] = dp_to_px(state, 16.0f);
        /* Filled text field has rounded top corners */
        content_row->border_radius = dp_to_px(state, 4.0f); /* Approximated since we only have uniform border_radius atm */
        /* bottom border */
        content_row->border_width = dp_to_px(state, config->is_focused || config->is_error ? 2.0f : 1.0f);
        if (config->is_error) {
            content_row->border_color_ref = &state->sys_colors_hex.error;
        } else if (config->is_focused) {
            content_row->border_color_ref = &state->sys_colors_hex.primary;
        } else {
            content_row->border_color_ref = &state->sys_colors_hex.on_surface_variant;
        }
    } else {
        content_row->bg_color = 0;
        content_row->layout->padding[3] = dp_to_px(state, 16.0f);
        content_row->layout->padding[1] = dp_to_px(state, 16.0f);
        content_row->border_radius = dp_to_px(state, 4.0f);
        /* outline border */
        content_row->border_width = dp_to_px(state, config->is_focused || config->is_error ? 2.0f : 1.0f);
        if (config->is_error) {
            content_row->border_color_ref = &state->sys_colors_hex.error;
        } else if (config->is_focused) {
            content_row->border_color_ref = &state->sys_colors_hex.primary;
        } else {
            content_row->border_color_ref = &state->sys_colors_hex.outline;
        }
    }

    if (config->leading_icon) {
        cmp_ui_node_t* lead;
        if (cmp_ui_text_create(&lead, config->leading_icon, (int)strlen(config->leading_icon)) == 0) {
            lead->text_color_ref = config->is_error ? &state->sys_colors_hex.on_error_container : &state->sys_colors_hex.on_surface_variant;
            lead->layout->margin[1] = dp_to_px(state, 16.0f);
            cmp_ui_node_add_child(content_row, lead);
        }
    }

    cmp_ui_box_create(&text_col);
    text_col->layout->direction = CMP_FLEX_COLUMN;
    text_col->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
    text_col->layout->flex_grow = 1.0f;

    if (config->label && config->text) {
        /* Both present, label floats */
        cmp_ui_node_t* lbl;
        if (cmp_ui_text_create(&lbl, config->label, (int)strlen(config->label)) == 0) {
            lbl->text_color_ref = config->is_error ? &state->sys_colors_hex.error : (config->is_focused ? &state->sys_colors_hex.primary : &state->sys_colors_hex.on_surface_variant);
            lbl->font_size = dp_to_px(state, 12.0f);
            cmp_ui_node_add_child(text_col, lbl);
        }
    } else if (config->label) {
        /* Only label, acts as placeholder */
        cmp_ui_node_t* lbl;
        if (cmp_ui_text_create(&lbl, config->label, (int)strlen(config->label)) == 0) {
            lbl->text_color_ref = config->is_error ? &state->sys_colors_hex.error : &state->sys_colors_hex.on_surface_variant;
            lbl->font_size = dp_to_px(state, 16.0f);
            cmp_ui_node_add_child(text_col, lbl);
        }
    }

    if (config->text) {
        cmp_ui_node_t* txt;
        if (cmp_ui_text_create(&txt, config->text, (int)strlen(config->text)) == 0) {
            txt->text_color_ref = &state->sys_colors_hex.on_surface;
            txt->font_size = dp_to_px(state, 16.0f);
            cmp_ui_node_add_child(text_col, txt);
        }
    }

    cmp_ui_node_add_child(content_row, text_col);

    if (config->trailing_icon) {
        cmp_ui_node_t* trail;
        if (cmp_ui_text_create(&trail, config->trailing_icon, (int)strlen(config->trailing_icon)) == 0) {
            trail->text_color_ref = config->is_error ? &state->sys_colors_hex.error : &state->sys_colors_hex.on_surface_variant;
            trail->layout->margin[3] = dp_to_px(state, 16.0f);
            cmp_ui_node_add_child(content_row, trail);
        }
    }

    cmp_ui_node_add_child(field, content_row);

    if (config->supporting_text) {
        cmp_ui_node_t* supp;
        if (cmp_ui_text_create(&supp, config->supporting_text, (int)strlen(config->supporting_text)) == 0) {
            supp->text_color_ref = config->is_error ? &state->sys_colors_hex.error : &state->sys_colors_hex.on_surface_variant;
            supp->font_size = dp_to_px(state, 12.0f);
            supp->layout->padding[3] = dp_to_px(state, 16.0f);
            supp->layout->padding[1] = dp_to_px(state, 16.0f);
            supp->layout->padding[0] = dp_to_px(state, 4.0f);
            cmp_ui_node_add_child(field, supp);
        }
    }

    *out_node = field;
    return 0;
    }/* clang-format on */
