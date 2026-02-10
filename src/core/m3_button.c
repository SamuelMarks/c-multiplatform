#include "m3/m3_button.h"

#include <string.h>

#ifdef M3_TESTING
#define M3_BUTTON_TEST_FAIL_NONE 0u
#define M3_BUTTON_TEST_FAIL_TEXT_STYLE_INIT 1u
#define M3_BUTTON_TEST_FAIL_SHADOW_INIT 2u
#define M3_BUTTON_TEST_FAIL_RIPPLE_INIT 3u
#define M3_BUTTON_TEST_FAIL_RIPPLE_RADIUS 4u
#define M3_BUTTON_TEST_FAIL_RIPPLE_START 5u
#define M3_BUTTON_TEST_FAIL_SHADOW_SET 6u
#define M3_BUTTON_TEST_FAIL_RESOLVE_COLORS 7u
#define M3_BUTTON_TEST_FAIL_RESOLVE_CORNER 8u
#define M3_BUTTON_TEST_FAIL_OUTLINE_WIDTH 9u
#define M3_BUTTON_TEST_FAIL_RIPPLE_RELEASE 10u

static m3_u32 g_m3_button_test_fail_point = M3_BUTTON_TEST_FAIL_NONE;
static m3_u32 g_m3_button_test_color_fail_after = 0u;

int M3_CALL m3_button_test_set_fail_point(m3_u32 fail_point)
{
    g_m3_button_test_fail_point = fail_point;
    return M3_OK;
}

int M3_CALL m3_button_test_set_color_fail_after(m3_u32 call_count)
{
    g_m3_button_test_color_fail_after = call_count;
    return M3_OK;
}

int M3_CALL m3_button_test_clear_fail_points(void)
{
    g_m3_button_test_fail_point = M3_BUTTON_TEST_FAIL_NONE;
    g_m3_button_test_color_fail_after = 0u;
    return M3_OK;
}

static int m3_button_test_color_should_fail(void)
{
    if (g_m3_button_test_color_fail_after == 0u) {
        return 0;
    }
    g_m3_button_test_color_fail_after -= 1u;
    return (g_m3_button_test_color_fail_after == 0u) ? 1 : 0;
}

static int m3_button_test_fail_point_match(m3_u32 point)
{
    if (g_m3_button_test_fail_point != point) {
        return 0;
    }
    g_m3_button_test_fail_point = M3_BUTTON_TEST_FAIL_NONE;
    return 1;
}
#endif

static int m3_button_validate_color(const M3Color *color)
{
    if (color == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!(color->r >= 0.0f && color->r <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(color->g >= 0.0f && color->g <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(color->b >= 0.0f && color->b <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(color->a >= 0.0f && color->a <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_button_color_set(M3Color *color, M3Scalar r, M3Scalar g, M3Scalar b, M3Scalar a)
{
    if (color == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!(r >= 0.0f && r <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(g >= 0.0f && g <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(b >= 0.0f && b <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(a >= 0.0f && a <= 1.0f)) {
        return M3_ERR_RANGE;
    }
#ifdef M3_TESTING
    if (m3_button_test_color_should_fail()) {
        return M3_ERR_IO;
    }
#endif
    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;
    return M3_OK;
}

static int m3_button_color_with_alpha(const M3Color *base, M3Scalar alpha, M3Color *out_color)
{
    int rc;

    if (base == NULL || out_color == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!(alpha >= 0.0f && alpha <= 1.0f)) {
        return M3_ERR_RANGE;
    }

    rc = m3_button_validate_color(base);
    if (rc != M3_OK) {
        return rc;
    }
#ifdef M3_TESTING
    if (m3_button_test_color_should_fail()) {
        return M3_ERR_IO;
    }
#endif

    *out_color = *base;
    out_color->a = out_color->a * alpha;
    return M3_OK;
}

static int m3_button_validate_text_style(const M3TextStyle *style, M3Bool require_family)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (require_family == M3_TRUE && style->utf8_family == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (style->size_px <= 0) {
        return M3_ERR_RANGE;
    }
    if (style->weight < 100 || style->weight > 900) {
        return M3_ERR_RANGE;
    }
    if (style->italic != M3_FALSE && style->italic != M3_TRUE) {
        return M3_ERR_RANGE;
    }

    rc = m3_button_validate_color(&style->color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_button_validate_style(const M3ButtonStyle *style, M3Bool require_family)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    switch (style->variant) {
        case M3_BUTTON_VARIANT_FILLED:
        case M3_BUTTON_VARIANT_TONAL:
        case M3_BUTTON_VARIANT_OUTLINED:
        case M3_BUTTON_VARIANT_TEXT:
        case M3_BUTTON_VARIANT_ELEVATED:
        case M3_BUTTON_VARIANT_FAB:
            break;
        default:
            return M3_ERR_RANGE;
    }

    if (style->padding_x < 0.0f || style->padding_y < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->min_width < 0.0f || style->min_height < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->outline_width < 0.0f || style->corner_radius < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->fab_diameter < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->variant == M3_BUTTON_VARIANT_FAB && style->fab_diameter <= 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->ripple_expand_duration < 0.0f || style->ripple_fade_duration < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->shadow_enabled != M3_FALSE && style->shadow_enabled != M3_TRUE) {
        return M3_ERR_RANGE;
    }

    rc = m3_button_validate_text_style(&style->text_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_validate_color(&style->background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_color(&style->outline_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_color(&style->ripple_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_color(&style->disabled_background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_color(&style->disabled_text_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_color(&style->disabled_outline_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_button_validate_measure_spec(M3MeasureSpec spec)
{
    if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY && spec.mode != M3_MEASURE_AT_MOST) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_button_validate_rect(const M3Rect *rect)
{
    if (rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_button_validate_backend(const M3TextBackend *backend)
{
    if (backend == NULL || backend->vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_OK;
}

static int m3_button_metrics_update(M3Button *button)
{
    int rc;

    if (button == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (button->metrics_valid == M3_TRUE) {
        return M3_OK;
    }

    rc = m3_text_measure_utf8(&button->text_backend, button->font, button->utf8_label, button->utf8_len, &button->metrics);
    if (rc != M3_OK) {
        return rc;
    }

    button->metrics_valid = M3_TRUE;
    return M3_OK;
}

static int m3_button_resolve_colors(const M3Button *button, M3Color *out_background, M3Color *out_text, M3Color *out_outline,
    M3Color *out_ripple)
{
    int rc;

    if (button == NULL || out_background == NULL || out_text == NULL || out_outline == NULL || out_ripple == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (button->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        *out_background = button->style.disabled_background_color;
        *out_text = button->style.disabled_text_color;
        *out_outline = button->style.disabled_outline_color;
        *out_ripple = button->style.disabled_text_color;
    } else {
        *out_background = button->style.background_color;
        *out_text = button->style.text_style.color;
        *out_outline = button->style.outline_color;
        *out_ripple = button->style.ripple_color;
    }

    rc = m3_button_validate_color(out_background);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_color(out_text);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_color(out_outline);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_color(out_ripple);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_button_resolve_corner(const M3Button *button, M3Scalar *out_corner)
{
    M3Scalar corner;
    M3Scalar min_side;
    int rc;

    if (button == NULL || out_corner == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_button_validate_rect(&button->bounds);
    if (rc != M3_OK) {
        return rc;
    }

    if (button->style.variant == M3_BUTTON_VARIANT_FAB) {
        min_side = button->bounds.width;
        if (button->bounds.height < min_side) {
            min_side = button->bounds.height;
        }
        corner = min_side * 0.5f;
    } else {
        corner = button->style.corner_radius;
    }

    if (corner < 0.0f) {
        return M3_ERR_RANGE;
    }

    *out_corner = corner;
    return M3_OK;
}

static int m3_button_style_init_base(M3ButtonStyle *style, m3_u32 variant)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(style, 0, sizeof(*style));

    rc = m3_text_style_init(&style->text_style);
#ifdef M3_TESTING
    if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_TEXT_STYLE_INIT)) {
        rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_shadow_init(&style->shadow);
#ifdef M3_TESTING
    if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_SHADOW_INIT)) {
        rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
        return rc;
    }

    style->variant = variant;
    style->padding_x = M3_BUTTON_DEFAULT_PADDING_X;
    style->padding_y = M3_BUTTON_DEFAULT_PADDING_Y;
    style->min_width = M3_BUTTON_DEFAULT_MIN_WIDTH;
    style->min_height = M3_BUTTON_DEFAULT_MIN_HEIGHT;
    style->corner_radius = M3_BUTTON_DEFAULT_CORNER_RADIUS;
    style->outline_width = M3_BUTTON_DEFAULT_OUTLINE_WIDTH;
    style->fab_diameter = M3_BUTTON_DEFAULT_FAB_DIAMETER;
    style->ripple_expand_duration = M3_BUTTON_DEFAULT_RIPPLE_EXPAND;
    style->ripple_fade_duration = M3_BUTTON_DEFAULT_RIPPLE_FADE;
    style->shadow_enabled = M3_FALSE;

    rc = m3_button_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->disabled_background_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->disabled_text_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->disabled_outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

int M3_CALL m3_button_style_init_filled(M3ButtonStyle *style)
{
    int rc;

    rc = m3_button_style_init_base(style, M3_BUTTON_VARIANT_FILLED);
    if (rc != M3_OK) {
        return rc;
    }

    style->outline_width = 0.0f;

    rc = m3_button_color_set(&style->background_color, 0.26f, 0.52f, 0.96f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->text_style.color, 1.0f, 1.0f, 1.0f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->ripple_color, 1.0f, 1.0f, 1.0f, 0.2f);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_with_alpha(&style->background_color, 0.12f, &style->disabled_background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->text_style.color, 0.38f, &style->disabled_text_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->outline_color, 0.12f, &style->disabled_outline_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

int M3_CALL m3_button_style_init_tonal(M3ButtonStyle *style)
{
    int rc;

    rc = m3_button_style_init_base(style, M3_BUTTON_VARIANT_TONAL);
    if (rc != M3_OK) {
        return rc;
    }

    style->outline_width = 0.0f;

    rc = m3_button_color_set(&style->background_color, 0.73f, 0.75f, 0.86f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->text_style.color, 0.12f, 0.12f, 0.18f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->ripple_color, 0.12f, 0.12f, 0.18f, 0.18f);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_with_alpha(&style->background_color, 0.12f, &style->disabled_background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->text_style.color, 0.38f, &style->disabled_text_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->outline_color, 0.12f, &style->disabled_outline_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

int M3_CALL m3_button_style_init_outlined(M3ButtonStyle *style)
{
    int rc;

    rc = m3_button_style_init_base(style, M3_BUTTON_VARIANT_OUTLINED);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->text_style.color, 0.26f, 0.52f, 0.96f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->outline_color, 0.44f, 0.44f, 0.44f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->ripple_color, 0.26f, 0.52f, 0.96f, 0.16f);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_with_alpha(&style->background_color, 0.12f, &style->disabled_background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->text_style.color, 0.38f, &style->disabled_text_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->outline_color, 0.12f, &style->disabled_outline_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

int M3_CALL m3_button_style_init_text(M3ButtonStyle *style)
{
    int rc;

    rc = m3_button_style_init_base(style, M3_BUTTON_VARIANT_TEXT);
    if (rc != M3_OK) {
        return rc;
    }

    style->outline_width = 0.0f;

    rc = m3_button_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->text_style.color, 0.26f, 0.52f, 0.96f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->ripple_color, 0.26f, 0.52f, 0.96f, 0.16f);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_with_alpha(&style->background_color, 0.12f, &style->disabled_background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->text_style.color, 0.38f, &style->disabled_text_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->outline_color, 0.12f, &style->disabled_outline_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

int M3_CALL m3_button_style_init_elevated(M3ButtonStyle *style)
{
    M3Color shadow_color;
    int rc;

    rc = m3_button_style_init_base(style, M3_BUTTON_VARIANT_ELEVATED);
    if (rc != M3_OK) {
        return rc;
    }

    style->outline_width = 0.0f;

    rc = m3_button_color_set(&style->background_color, 0.95f, 0.95f, 0.95f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->text_style.color, 0.12f, 0.12f, 0.12f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->ripple_color, 0.12f, 0.12f, 0.12f, 0.16f);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_with_alpha(&style->background_color, 0.12f, &style->disabled_background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->text_style.color, 0.38f, &style->disabled_text_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->outline_color, 0.12f, &style->disabled_outline_color);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_set(&shadow_color, 0.0f, 0.0f, 0.0f, 0.28f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_shadow_set(&style->shadow, 0.0f, 2.0f, 6.0f, 0.0f, style->corner_radius, 3, shadow_color);
#ifdef M3_TESTING
    if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_SHADOW_SET)) {
        rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
        return rc;
    }
    style->shadow_enabled = M3_TRUE;

    return M3_OK;
}

int M3_CALL m3_button_style_init_fab(M3ButtonStyle *style)
{
    M3Color shadow_color;
    int rc;

    rc = m3_button_style_init_base(style, M3_BUTTON_VARIANT_FAB);
    if (rc != M3_OK) {
        return rc;
    }

    style->padding_x = 0.0f;
    style->padding_y = 0.0f;
    style->min_width = style->fab_diameter;
    style->min_height = style->fab_diameter;
    style->corner_radius = style->fab_diameter * 0.5f;
    style->outline_width = 0.0f;

    rc = m3_button_color_set(&style->background_color, 0.26f, 0.52f, 0.96f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->text_style.color, 1.0f, 1.0f, 1.0f, 1.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_set(&style->ripple_color, 1.0f, 1.0f, 1.0f, 0.2f);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_with_alpha(&style->background_color, 0.12f, &style->disabled_background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->text_style.color, 0.38f, &style->disabled_text_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_color_with_alpha(&style->outline_color, 0.12f, &style->disabled_outline_color);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_color_set(&shadow_color, 0.0f, 0.0f, 0.0f, 0.32f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_shadow_set(&style->shadow, 0.0f, 4.0f, 10.0f, 0.0f, style->corner_radius, 4, shadow_color);
#ifdef M3_TESTING
    if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_SHADOW_SET)) {
        rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
        return rc;
    }
    style->shadow_enabled = M3_TRUE;

    return M3_OK;
}

static int m3_button_widget_measure(void *widget, M3MeasureSpec width, M3MeasureSpec height, M3Size *out_size)
{
    M3Button *button;
    M3TextMetrics metrics;
    M3Scalar desired_width;
    M3Scalar desired_height;
    int rc;

    if (widget == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_button_validate_measure_spec(width);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_button_validate_measure_spec(height);
    if (rc != M3_OK) {
        return rc;
    }

    button = (M3Button *)widget;
    rc = m3_button_validate_style(&button->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_metrics_update(button);
    if (rc != M3_OK) {
        return rc;
    }

    metrics = button->metrics;
    desired_width = metrics.width + button->style.padding_x * 2.0f;
    desired_height = metrics.height + button->style.padding_y * 2.0f;

    if (desired_width < button->style.min_width) {
        desired_width = button->style.min_width;
    }
    if (desired_height < button->style.min_height) {
        desired_height = button->style.min_height;
    }

    if (button->style.variant == M3_BUTTON_VARIANT_FAB) {
        if (desired_width < button->style.fab_diameter) {
            desired_width = button->style.fab_diameter;
        }
        if (desired_height < button->style.fab_diameter) {
            desired_height = button->style.fab_diameter;
        }
    }

    if (width.mode == M3_MEASURE_EXACTLY) {
        out_size->width = width.size;
    } else if (width.mode == M3_MEASURE_AT_MOST) {
        out_size->width = (desired_width > width.size) ? width.size : desired_width;
    } else {
        out_size->width = desired_width;
    }

    if (height.mode == M3_MEASURE_EXACTLY) {
        out_size->height = height.size;
    } else if (height.mode == M3_MEASURE_AT_MOST) {
        out_size->height = (desired_height > height.size) ? height.size : desired_height;
    } else {
        out_size->height = desired_height;
    }

    return M3_OK;
}

static int m3_button_widget_layout(void *widget, M3Rect bounds)
{
    M3Button *button;
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_button_validate_rect(&bounds);
    if (rc != M3_OK) {
        return rc;
    }

    button = (M3Button *)widget;
    button->bounds = bounds;
    return M3_OK;
}

static int m3_button_widget_paint(void *widget, M3PaintContext *ctx)
{
    M3Button *button;
    M3Rect bounds;
    M3Rect inner;
    M3TextMetrics metrics;
    M3Color background;
    M3Color text_color;
    M3Color outline;
    M3Color ripple_color;
    M3Shadow shadow;
    M3Scalar corner_radius;
    M3Scalar inner_corner;
    M3Scalar outline_width;
    M3Scalar available_width;
    M3Scalar available_height;
    M3Scalar text_x;
    M3Scalar text_y;
    M3Bool ripple_active;
    int rc;

    if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (ctx->gfx->vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (ctx->gfx->vtable->draw_rect == NULL) {
        return M3_ERR_UNSUPPORTED;
    }
    if (ctx->gfx->text_vtable == NULL || ctx->gfx->text_vtable->draw_text == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    button = (M3Button *)widget;

    rc = m3_button_validate_style(&button->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_validate_rect(&button->bounds);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_metrics_update(button);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_resolve_colors(button, &background, &text_color, &outline, &ripple_color);
#ifdef M3_TESTING
    if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_RESOLVE_COLORS)) {
        rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_button_resolve_corner(button, &corner_radius);
#ifdef M3_TESTING
    if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_RESOLVE_CORNER)) {
        rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
        return rc;
    }

    bounds = button->bounds;
    outline_width = button->style.outline_width;
#ifdef M3_TESTING
    if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_OUTLINE_WIDTH)) {
        outline_width = -1.0f;
    }
#endif
    if (outline_width < 0.0f) {
        return M3_ERR_RANGE;
    }

    if (button->style.shadow_enabled == M3_TRUE) {
        shadow = button->style.shadow;
        shadow.corner_radius = corner_radius;
        rc = m3_shadow_paint(&shadow, ctx->gfx, &bounds, &ctx->clip);
        if (rc != M3_OK) {
            return rc;
        }
    }

    if (outline_width > 0.0f) {
        inner.x = bounds.x + outline_width;
        inner.y = bounds.y + outline_width;
        inner.width = bounds.width - outline_width * 2.0f;
        inner.height = bounds.height - outline_width * 2.0f;
        if (inner.width < 0.0f || inner.height < 0.0f) {
            return M3_ERR_RANGE;
        }

        rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, outline, corner_radius);
        if (rc != M3_OK) {
            return rc;
        }

        inner_corner = corner_radius - outline_width;
        if (inner_corner < 0.0f) {
            inner_corner = 0.0f;
        }
    } else {
        inner = bounds;
        inner_corner = corner_radius;
    }

    if (background.a > 0.0f) {
        rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner, background, inner_corner);
        if (rc != M3_OK) {
            return rc;
        }
    }

    if ((button->widget.flags & M3_WIDGET_FLAG_DISABLED) == 0) {
        rc = m3_ripple_is_active(&button->ripple, &ripple_active);
        if (rc != M3_OK) {
            return rc;
        }
        if (ripple_active == M3_TRUE) {
            rc = m3_ripple_paint(&button->ripple, ctx->gfx, &ctx->clip, inner_corner);
            if (rc != M3_OK) {
                return rc;
            }
        }
    }

    if (button->utf8_label == NULL && button->utf8_len == 0) {
        return M3_OK;
    }

    metrics = button->metrics;
    available_width = bounds.width - button->style.padding_x * 2.0f;
    available_height = bounds.height - button->style.padding_y * 2.0f;
    if (available_width < 0.0f) {
        available_width = 0.0f;
    }
    if (available_height < 0.0f) {
        available_height = 0.0f;
    }

    if (available_width <= metrics.width) {
        text_x = bounds.x + button->style.padding_x;
    } else {
        text_x = bounds.x + button->style.padding_x + (available_width - metrics.width) * 0.5f;
    }

    if (available_height <= metrics.height) {
        text_y = bounds.y + button->style.padding_y + metrics.baseline;
    } else {
        text_y = bounds.y + button->style.padding_y + (available_height - metrics.height) * 0.5f + metrics.baseline;
    }

    return ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, button->font, button->utf8_label, button->utf8_len, text_x,
        text_y, text_color);
}

static int m3_button_widget_event(void *widget, const M3InputEvent *event, M3Bool *out_handled)
{
    M3Button *button;
    M3Scalar center_x;
    M3Scalar center_y;
    M3Scalar max_radius;
    int rc;

    if (widget == NULL || event == NULL || out_handled == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_handled = M3_FALSE;

    button = (M3Button *)widget;

    if (button->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        return M3_OK;
    }

    switch (event->type) {
        case M3_INPUT_POINTER_DOWN:
            if (button->pressed == M3_TRUE) {
                return M3_ERR_STATE;
            }
            if (button->style.ripple_expand_duration < 0.0f) {
                return M3_ERR_RANGE;
            }
            rc = m3_button_validate_color(&button->style.ripple_color);
            if (rc != M3_OK) {
                return rc;
            }
            rc = m3_button_validate_rect(&button->bounds);
            if (rc != M3_OK) {
                return rc;
            }
            center_x = (M3Scalar)event->data.pointer.x;
            center_y = (M3Scalar)event->data.pointer.y;
            rc = m3_ripple_compute_max_radius(&button->bounds, center_x, center_y, &max_radius);
#ifdef M3_TESTING
            if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_RIPPLE_RADIUS)) {
                rc = M3_ERR_IO;
            }
#endif
            if (rc != M3_OK) {
                return rc;
            }
            rc = m3_ripple_start(&button->ripple, center_x, center_y, max_radius, button->style.ripple_expand_duration,
                button->style.ripple_color);
#ifdef M3_TESTING
            if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_RIPPLE_START)) {
                rc = M3_ERR_IO;
            }
#endif
            if (rc != M3_OK) {
                return rc;
            }
            button->pressed = M3_TRUE;
            *out_handled = M3_TRUE;
            return M3_OK;
        case M3_INPUT_POINTER_UP:
            if (button->pressed == M3_FALSE) {
                return M3_OK;
            }
            if (button->style.ripple_fade_duration < 0.0f) {
                return M3_ERR_RANGE;
            }
            button->pressed = M3_FALSE;
            rc = m3_ripple_release(&button->ripple, button->style.ripple_fade_duration);
#ifdef M3_TESTING
            if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_RIPPLE_RELEASE)) {
                rc = M3_ERR_IO;
            }
#endif
            if (rc != M3_OK && rc != M3_ERR_STATE) {
                return rc;
            }
            if (button->on_click != NULL) {
                rc = button->on_click(button->on_click_ctx, button);
                if (rc != M3_OK) {
                    return rc;
                }
            }
            *out_handled = M3_TRUE;
            return M3_OK;
        default:
            return M3_OK;
    }
}

static int m3_button_widget_get_semantics(void *widget, M3Semantics *out_semantics)
{
    M3Button *button;

    if (widget == NULL || out_semantics == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    button = (M3Button *)widget;
    out_semantics->role = M3_SEMANTIC_BUTTON;
    out_semantics->flags = 0;
    if (button->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
    }
    if (button->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
    }
    out_semantics->utf8_label = button->utf8_label;
    out_semantics->utf8_hint = NULL;
    out_semantics->utf8_value = NULL;
    return M3_OK;
}

static int m3_button_widget_destroy(void *widget)
{
    M3Button *button;
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    button = (M3Button *)widget;
    rc = M3_OK;
    if (button->owns_font == M3_TRUE && (button->font.id != 0u || button->font.generation != 0u)) {
        if (button->text_backend.vtable != NULL && button->text_backend.vtable->destroy_font != NULL) {
            rc = button->text_backend.vtable->destroy_font(button->text_backend.ctx, button->font);
        } else {
            rc = M3_ERR_UNSUPPORTED;
        }
    }

    button->font.id = 0u;
    button->font.generation = 0u;
    button->utf8_label = NULL;
    button->utf8_len = 0;
    button->metrics_valid = M3_FALSE;
    button->owns_font = M3_FALSE;
    button->pressed = M3_FALSE;
    button->text_backend.ctx = NULL;
    button->text_backend.vtable = NULL;
    button->widget.ctx = NULL;
    button->widget.vtable = NULL;
    button->on_click = NULL;
    button->on_click_ctx = NULL;
    return rc;
}

static const M3WidgetVTable g_m3_button_widget_vtable = {
    m3_button_widget_measure,
    m3_button_widget_layout,
    m3_button_widget_paint,
    m3_button_widget_event,
    m3_button_widget_get_semantics,
    m3_button_widget_destroy
};

int M3_CALL m3_button_init(M3Button *button, const M3TextBackend *backend, const M3ButtonStyle *style,
    const char *utf8_label, m3_usize utf8_len)
{
    int rc;

    if (button == NULL || backend == NULL || style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_label == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_button_validate_backend(backend);
    if (rc != M3_OK) {
        return rc;
    }
    if (backend->vtable->create_font == NULL || backend->vtable->destroy_font == NULL || backend->vtable->measure_text == NULL
        || backend->vtable->draw_text == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    rc = m3_button_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    memset(button, 0, sizeof(*button));
    button->text_backend = *backend;
    button->style = *style;
    button->utf8_label = utf8_label;
    button->utf8_len = utf8_len;
    button->metrics_valid = M3_FALSE;
    button->pressed = M3_FALSE;
    button->owns_font = M3_TRUE;

    rc = m3_ripple_init(&button->ripple);
#ifdef M3_TESTING
    if (m3_button_test_fail_point_match(M3_BUTTON_TEST_FAIL_RIPPLE_INIT)) {
        rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_text_font_create(backend, &style->text_style, &button->font);
    if (rc != M3_OK) {
        return rc;
    }

    button->widget.ctx = button;
    button->widget.vtable = &g_m3_button_widget_vtable;
    button->widget.handle.id = 0u;
    button->widget.handle.generation = 0u;
    button->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
    return M3_OK;
}

int M3_CALL m3_button_set_label(M3Button *button, const char *utf8_label, m3_usize utf8_len)
{
    if (button == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_label == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    button->utf8_label = utf8_label;
    button->utf8_len = utf8_len;
    button->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_button_set_style(M3Button *button, const M3ButtonStyle *style)
{
    M3Handle new_font;
    int rc;

    if (button == NULL || style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_button_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_text_font_create(&button->text_backend, &style->text_style, &new_font);
    if (rc != M3_OK) {
        return rc;
    }

    if (button->owns_font == M3_TRUE) {
        rc = m3_text_font_destroy(&button->text_backend, button->font);
        if (rc != M3_OK) {
            m3_text_font_destroy(&button->text_backend, new_font);
            return rc;
        }
    }

    button->style = *style;
    button->font = new_font;
    button->owns_font = M3_TRUE;
    button->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_button_set_on_click(M3Button *button, M3ButtonOnClick on_click, void *ctx)
{
    if (button == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    button->on_click = on_click;
    button->on_click_ctx = ctx;
    return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_button_test_validate_color(const M3Color *color)
{
    return m3_button_validate_color(color);
}

int M3_CALL m3_button_test_color_set(M3Color *color, M3Scalar r, M3Scalar g, M3Scalar b, M3Scalar a)
{
    return m3_button_color_set(color, r, g, b, a);
}

int M3_CALL m3_button_test_color_with_alpha(const M3Color *base, M3Scalar alpha, M3Color *out_color)
{
    return m3_button_color_with_alpha(base, alpha, out_color);
}

int M3_CALL m3_button_test_validate_text_style(const M3TextStyle *style, M3Bool require_family)
{
    return m3_button_validate_text_style(style, require_family);
}

int M3_CALL m3_button_test_validate_style(const M3ButtonStyle *style, M3Bool require_family)
{
    return m3_button_validate_style(style, require_family);
}

int M3_CALL m3_button_test_validate_measure_spec(M3MeasureSpec spec)
{
    return m3_button_validate_measure_spec(spec);
}

int M3_CALL m3_button_test_validate_rect(const M3Rect *rect)
{
    return m3_button_validate_rect(rect);
}

int M3_CALL m3_button_test_validate_backend(const M3TextBackend *backend)
{
    return m3_button_validate_backend(backend);
}

int M3_CALL m3_button_test_metrics_update(M3Button *button)
{
    return m3_button_metrics_update(button);
}

int M3_CALL m3_button_test_resolve_colors(const M3Button *button, M3Color *out_background, M3Color *out_text,
    M3Color *out_outline, M3Color *out_ripple)
{
    return m3_button_resolve_colors(button, out_background, out_text, out_outline, out_ripple);
}

int M3_CALL m3_button_test_resolve_corner(const M3Button *button, M3Scalar *out_corner)
{
    return m3_button_resolve_corner(button, out_corner);
}
#endif
