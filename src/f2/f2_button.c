#include "f2/f2_button.h"
#include <string.h>

static int f2_button_style_init_base(F2ButtonStyle *style) {
    style->is_rtl = CMP_FALSE;
    cmp_text_style_init(&style->text_style);
    style->text_style.utf8_family = "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
    style->text_style.size_px = 14;
    style->text_style.weight = 600;

    style->outline_width = 1.0f;
    style->corner_radius = F2_BUTTON_DEFAULT_CORNER_RADIUS;
    style->padding_x = F2_BUTTON_DEFAULT_PADDING_X;
    style->padding_y = F2_BUTTON_DEFAULT_PADDING_Y;
    
#if defined(CMP_IOS_AVAILABLE)
    style->min_width = 44.0f;
    style->min_height = 44.0f;
#elif defined(CMP_ANDROID_AVAILABLE)
    style->min_width = 48.0f;
    style->min_height = 48.0f;
#else
    style->min_width = F2_BUTTON_DEFAULT_MIN_WIDTH;
    style->min_height = F2_BUTTON_DEFAULT_MIN_HEIGHT;
#endif

    style->shadow_enabled = CMP_FALSE;
    style->icon_spacing = 8.0f;
    style->icon_diameter = 16.0f;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_style_init_primary(F2ButtonStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    f2_button_style_init_base(style);
    style->variant = F2_BUTTON_VARIANT_PRIMARY;
    style->background_color = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f}; /* Blue */
    style->background_hover_color = (CMPColor){0.0f, 0.45f, 0.88f, 1.0f};
    style->background_pressed_color = (CMPColor){0.0f, 0.28f, 0.65f, 1.0f};
    style->text_style.color = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
    style->outline_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_style_init_standard(F2ButtonStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    f2_button_style_init_base(style);
    style->variant = F2_BUTTON_VARIANT_STANDARD;
    style->background_color = (CMPColor){0.99f, 0.99f, 0.99f, 1.0f};
    style->background_hover_color = (CMPColor){0.96f, 0.96f, 0.96f, 1.0f};
    style->background_pressed_color = (CMPColor){0.90f, 0.90f, 0.90f, 1.0f};
    style->text_style.color = (CMPColor){0.14f, 0.14f, 0.14f, 1.0f};
    style->outline_color = (CMPColor){0.82f, 0.82f, 0.82f, 1.0f};
    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_style_init_outline(F2ButtonStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    f2_button_style_init_base(style);
    style->variant = F2_BUTTON_VARIANT_OUTLINE;
    style->background_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
    style->background_hover_color = (CMPColor){0.96f, 0.96f, 0.96f, 1.0f};
    style->background_pressed_color = (CMPColor){0.90f, 0.90f, 0.90f, 1.0f};
    style->text_style.color = (CMPColor){0.14f, 0.14f, 0.14f, 1.0f};
    style->outline_color = (CMPColor){0.82f, 0.82f, 0.82f, 1.0f};
    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_style_init_subtle(F2ButtonStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    f2_button_style_init_base(style);
    style->variant = F2_BUTTON_VARIANT_SUBTLE;
    style->background_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
    style->background_hover_color = (CMPColor){0.96f, 0.96f, 0.96f, 1.0f};
    style->background_pressed_color = (CMPColor){0.90f, 0.90f, 0.90f, 1.0f};
    style->text_style.color = (CMPColor){0.14f, 0.14f, 0.14f, 1.0f};
    style->outline_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_style_init_transparent(F2ButtonStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    f2_button_style_init_base(style);
    style->variant = F2_BUTTON_VARIANT_TRANSPARENT;
    style->background_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
    style->background_hover_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
    style->background_pressed_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
    style->text_style.color = (CMPColor){0.14f, 0.14f, 0.14f, 1.0f};
    style->outline_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_init(F2Button *button,
                                    const CMPTextBackend *backend,
                                    const F2ButtonStyle *style,
                                    const char *utf8_label, cmp_usize utf8_len) {
    if (!button || !backend || !style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(button, 0, sizeof(F2Button));
    button->text_backend = *backend;
    button->style = *style;
    button->utf8_label = utf8_label;
    button->utf8_len = utf8_len;
    button->metrics_valid = CMP_FALSE;

    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_set_icon(F2Button *button, const char *utf8_icon,
                                        cmp_usize icon_len) {
    if (!button) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    button->utf8_icon = utf8_icon;
    button->icon_len = icon_len;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_set_label(F2Button *button,
                                         const char *utf8_label,
                                         cmp_usize utf8_len) {
    if (!button) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    button->utf8_label = utf8_label;
    button->utf8_len = utf8_len;
    button->metrics_valid = CMP_FALSE;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_button_set_on_click(F2Button *button,
                                            F2ButtonOnClick on_click,
                                            void *ctx) {
    if (!button) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    button->on_click = on_click;
    button->on_click_ctx = ctx;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_toggle_button_init(F2ToggleButton *toggle_button,
                                           const CMPTextBackend *backend,
                                           const F2ButtonStyle *style,
                                           const char *utf8_label,
                                           cmp_usize utf8_len,
                                           CMPBool is_toggled) {
    int res;
    if (!toggle_button || !backend || !style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    
    memset(toggle_button, 0, sizeof(*toggle_button));
    res = f2_button_init(&toggle_button->core, backend, style, utf8_label, utf8_len);
    if (res != CMP_OK) {
        return res;
    }
    
    toggle_button->is_toggled = is_toggled;
    toggle_button->toggled_bg_color = (CMPColor){0.9f, 0.9f, 0.9f, 1.0f};
    toggle_button->toggled_text_color = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f};
    
    return CMP_OK;
}

CMP_API int CMP_CALL f2_toggle_button_set_toggled(F2ToggleButton *toggle_button, CMPBool is_toggled) {
    if (!toggle_button) return CMP_ERR_INVALID_ARGUMENT;
    toggle_button->is_toggled = is_toggled;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_split_button_init(F2SplitButton *split_button,
                                          const CMPTextBackend *backend,
                                          const F2ButtonStyle *style,
                                          const char *utf8_label,
                                          cmp_usize utf8_len) {
    int res;
    if (!split_button || !backend || !style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(split_button, 0, sizeof(*split_button));
    
    res = f2_button_init(&split_button->primary_action, backend, style, utf8_label, utf8_len);
    if (res != CMP_OK) return res;
    
    res = f2_button_init(&split_button->dropdown_action, backend, style, "v", 1);
    if (res != CMP_OK) return res;
    
    split_button->dropdown_open = CMP_FALSE;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_split_button_set_dropdown_open(F2SplitButton *split_button, CMPBool is_open) {
    if (!split_button) return CMP_ERR_INVALID_ARGUMENT;
    split_button->dropdown_open = is_open;
    return CMP_OK;
}
