#include "f2/f2_slider.h"
#include "cmpc/cmp_core.h"
#include <string.h>

CMP_API int CMP_CALL f2_slider_style_init(F2SliderStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(style, 0, sizeof(*style));
    style->is_rtl = CMP_FALSE;
    style->track_height = F2_SLIDER_DEFAULT_TRACK_HEIGHT;
    style->thumb_radius = F2_SLIDER_DEFAULT_THUMB_RADIUS;
    style->thumb_inner_radius = F2_SLIDER_DEFAULT_THUMB_INNER_RADIUS;

    style->track_color = (CMPColor){0.9f, 0.9f, 0.9f, 1.0f};
    style->active_track_color = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f};
    style->thumb_color = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
    style->thumb_inner_color = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f};
    
    style->disabled_track_color = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
    style->disabled_thumb_color = (CMPColor){0.8f, 0.8f, 0.8f, 1.0f};
    
    return CMP_OK;
}

CMP_API int CMP_CALL f2_slider_init(F2Slider *slider,
                                    const F2SliderStyle *style,
                                    CMPScalar min_value,
                                    CMPScalar max_value,
                                    CMPScalar value) {
    if (!slider || !style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (min_value > max_value) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(slider, 0, sizeof(*slider));
    slider->style = *style;
    slider->min_value = min_value;
    slider->max_value = max_value;
    
    if (value < min_value) value = min_value;
    if (value > max_value) value = max_value;
    slider->value = value;
    
    return CMP_OK;
}

CMP_API int CMP_CALL f2_slider_set_value(F2Slider *slider, CMPScalar value) {
    if (!slider) return CMP_ERR_INVALID_ARGUMENT;
    if (value < slider->min_value) value = slider->min_value;
    if (value > slider->max_value) value = slider->max_value;
    slider->value = value;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_slider_set_step(F2Slider *slider, CMPScalar step) {
    if (!slider) return CMP_ERR_INVALID_ARGUMENT;
    slider->step = step;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_slider_set_disabled(F2Slider *slider, CMPBool disabled) {
    if (!slider) return CMP_ERR_INVALID_ARGUMENT;
    slider->disabled = disabled;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_slider_set_on_change(F2Slider *slider,
                                             F2SliderOnChange on_change,
                                             void *ctx) {
    if (!slider) return CMP_ERR_INVALID_ARGUMENT;
    slider->on_change = on_change;
    slider->on_change_ctx = ctx;
    return CMP_OK;
}
