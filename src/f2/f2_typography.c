#include "f2/f2_typography.h"
#include <string.h>

static int set_style(CMPTextStyle *style, const char *family, cmp_i32 size, cmp_i32 weight, CMPScalar line_height) {
    cmp_text_style_init(style);
    style->utf8_family = family;
    style->size_px = size;
    style->weight = weight;
    style->line_height_px = line_height;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_typography_scale_init(F2TypographyScale *scale) {
    const char *f2_family = "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
    
    if (!scale) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    set_style(&scale->roles[F2_TYPOGRAPHY_CAPTION], f2_family, 12, 400, 16.0f);
    set_style(&scale->roles[F2_TYPOGRAPHY_BODY], f2_family, 14, 400, 20.0f);
    set_style(&scale->roles[F2_TYPOGRAPHY_BODY_STRONG], f2_family, 14, 600, 20.0f);
    set_style(&scale->roles[F2_TYPOGRAPHY_SUBTITLE], f2_family, 20, 600, 28.0f);
    set_style(&scale->roles[F2_TYPOGRAPHY_TITLE], f2_family, 24, 600, 32.0f);
    set_style(&scale->roles[F2_TYPOGRAPHY_TITLE_LARGE], f2_family, 28, 600, 36.0f);
    set_style(&scale->roles[F2_TYPOGRAPHY_DISPLAY], f2_family, 32, 600, 40.0f);
    set_style(&scale->roles[F2_TYPOGRAPHY_DISPLAY_LARGE], f2_family, 40, 600, 52.0f);
    set_style(&scale->roles[F2_TYPOGRAPHY_DISPLAY_EXTRA_LARGE], f2_family, 68, 600, 92.0f);

    return CMP_OK;
}

CMP_API int CMP_CALL f2_typography_get_style(const F2TypographyScale *scale,
                                             F2TypographyRole role,
                                             CMPTextStyle *out_style) {
    if (!scale || !out_style || role < 0 || role >= F2_TYPOGRAPHY_ROLE_COUNT) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    *out_style = scale->roles[role];
    return CMP_OK;
}

CMP_API int CMP_CALL f2_typography_scale_set_color(F2TypographyScale *scale,
                                                   CMPColor color) {
    int i;
    if (!scale) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    for (i = 0; i < F2_TYPOGRAPHY_ROLE_COUNT; i++) {
        scale->roles[i].color = color;
    }
    return CMP_OK;
}

CMP_API int CMP_CALL f2_typography_scale_set_family(F2TypographyScale *scale,
                                                    const char *utf8_family) {
    int i;
    if (!scale || !utf8_family) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    for (i = 0; i < F2_TYPOGRAPHY_ROLE_COUNT; i++) {
        scale->roles[i].utf8_family = utf8_family;
    }
    return CMP_OK;
}
