#include "f2/f2_flyout.h"
#include "cmpc/cmp_core.h"
#include <string.h>

CMP_API int CMP_CALL f2_flyout_style_init(F2FlyoutStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(style, 0, sizeof(*style));
    
    style->padding = F2_FLYOUT_DEFAULT_PADDING;
    style->corner_radius = F2_FLYOUT_DEFAULT_CORNER_RADIUS;
    style->border_width = 1.0f;
    style->has_beak = CMP_TRUE;
    
    style->background_color = (CMPColor){1.0f, 1.0f, 1.0f, 0.95f}; /* Fluent uses acrylic/translucency here */
    style->border_color = (CMPColor){0.8f, 0.8f, 0.8f, 1.0f};
    
    return CMP_OK;
}

CMP_API int CMP_CALL f2_flyout_init(F2Flyout *flyout, const F2FlyoutStyle *style) {
    if (!flyout || !style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(flyout, 0, sizeof(*flyout));
    flyout->style = *style;
    flyout->is_open = CMP_FALSE;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_flyout_show(F2Flyout *flyout, CMPRect anchor_bounds) {
    if (!flyout) return CMP_ERR_INVALID_ARGUMENT;
    flyout->anchor_bounds = anchor_bounds;
    flyout->is_open = CMP_TRUE;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_flyout_hide(F2Flyout *flyout) {
    if (!flyout) return CMP_ERR_INVALID_ARGUMENT;
    flyout->is_open = CMP_FALSE;
    return CMP_OK;
}
