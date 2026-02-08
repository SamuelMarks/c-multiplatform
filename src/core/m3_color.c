#include "m3/m3_color.h"

#include <math.h>

#define M3_COLOR_PI 3.14159265358979323846
#define M3_COLOR_WHITE_X 0.95047f
#define M3_COLOR_WHITE_Y 1.00000f
#define M3_COLOR_WHITE_Z 1.08883f

#ifdef M3_TESTING
static M3Bool g_color_fail_argb_to_xyz = M3_FALSE;
static M3Bool g_color_fail_xyz_to_lab = M3_FALSE;
static M3Bool g_color_fail_lab_to_xyz = M3_FALSE;
static M3Bool g_color_fail_xyz_to_argb = M3_FALSE;
static int g_color_fail_lch_to_argb_call = 0;
static int g_color_lch_to_argb_calls = 0;
static int g_color_fail_palette_init_call = 0;
static int g_color_palette_init_calls = 0;
static int g_color_fail_palette_tone_call = 0;
static int g_color_palette_tone_calls = 0;
#endif

static M3Scalar m3_scalar_clamp01(M3Scalar value)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

static M3Scalar m3_wrap_hue(M3Scalar hue)
{
    while (hue < 0.0f) {
        hue += 360.0f;
    }
    while (hue >= 360.0f) {
        hue -= 360.0f;
    }
    return hue;
}

static M3Scalar m3_srgb_to_linear(M3Scalar c)
{
    if (c <= 0.04045f) {
        return c / 12.92f;
    }
    return (M3Scalar)pow((c + 0.055f) / 1.055f, 2.4);
}

static M3Scalar m3_linear_to_srgb(M3Scalar c)
{
    if (c <= 0.0031308f) {
        return 12.92f * c;
    }
    return (M3Scalar)(1.055 * pow(c, 1.0 / 2.4) - 0.055);
}

static int m3_color_argb_to_xyz(m3_u32 argb, M3Scalar *out_x, M3Scalar *out_y, M3Scalar *out_z)
{
    m3_u8 r8;
    m3_u8 g8;
    m3_u8 b8;
    M3Scalar r;
    M3Scalar g;
    M3Scalar b;

    if (out_x == NULL || out_y == NULL || out_z == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#ifdef M3_TESTING
    if (g_color_fail_argb_to_xyz) {
        return M3_ERR_UNKNOWN;
    }
#endif

    r8 = (m3_u8)((argb >> 16) & 0xFFu);
    g8 = (m3_u8)((argb >> 8) & 0xFFu);
    b8 = (m3_u8)(argb & 0xFFu);

    r = m3_srgb_to_linear((M3Scalar)r8 / 255.0f);
    g = m3_srgb_to_linear((M3Scalar)g8 / 255.0f);
    b = m3_srgb_to_linear((M3Scalar)b8 / 255.0f);

    *out_x = 0.41233895f * r + 0.35762064f * g + 0.18051042f * b;
    *out_y = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    *out_z = 0.01932141f * r + 0.11916382f * g + 0.95034478f * b;
    return M3_OK;
}

static int m3_color_xyz_to_argb(M3Scalar x, M3Scalar y, M3Scalar z, m3_u32 *out_argb, M3Bool *out_in_gamut)
{
    M3Scalar r_lin;
    M3Scalar g_lin;
    M3Scalar b_lin;
    M3Scalar r;
    M3Scalar g;
    M3Scalar b;
    M3Bool in_gamut;

    if (out_argb == NULL || out_in_gamut == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#ifdef M3_TESTING
    if (g_color_fail_xyz_to_argb) {
        return M3_ERR_UNKNOWN;
    }
#endif

    r_lin = 3.2413775f * x - 1.5376652f * y - 0.4988537f * z;
    g_lin = -0.9691453f * x + 1.8758854f * y + 0.0415659f * z;
    b_lin = 0.05562094f * x - 0.20395524f * y + 1.0571799f * z;

    in_gamut = (r_lin >= 0.0f && r_lin <= 1.0f && g_lin >= 0.0f && g_lin <= 1.0f && b_lin >= 0.0f && b_lin <= 1.0f)
        ? M3_TRUE
        : M3_FALSE;

    r = m3_linear_to_srgb(m3_scalar_clamp01(r_lin));
    g = m3_linear_to_srgb(m3_scalar_clamp01(g_lin));
    b = m3_linear_to_srgb(m3_scalar_clamp01(b_lin));

    r = m3_scalar_clamp01(r);
    g = m3_scalar_clamp01(g);
    b = m3_scalar_clamp01(b);

    *out_argb = 0xFF000000u
        | ((m3_u32)(r * 255.0f + 0.5f) << 16)
        | ((m3_u32)(g * 255.0f + 0.5f) << 8)
        | (m3_u32)(b * 255.0f + 0.5f);
    *out_in_gamut = in_gamut;
    return M3_OK;
}

static M3Scalar m3_lab_f(M3Scalar t)
{
    const M3Scalar delta = 6.0f / 29.0f;
    const M3Scalar delta3 = delta * delta * delta;

    if (t > delta3) {
        return (M3Scalar)pow(t, 1.0 / 3.0);
    }

    return t / (3.0f * delta * delta) + 4.0f / 29.0f;
}

static M3Scalar m3_lab_f_inv(M3Scalar t)
{
    const M3Scalar delta = 6.0f / 29.0f;

    if (t > delta) {
        return t * t * t;
    }

    return 3.0f * delta * delta * (t - 4.0f / 29.0f);
}

static int m3_color_xyz_to_lab(M3Scalar x, M3Scalar y, M3Scalar z, M3Scalar *out_l, M3Scalar *out_a, M3Scalar *out_b)
{
    M3Scalar fx;
    M3Scalar fy;
    M3Scalar fz;

    if (out_l == NULL || out_a == NULL || out_b == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#ifdef M3_TESTING
    if (g_color_fail_xyz_to_lab) {
        return M3_ERR_UNKNOWN;
    }
#endif

    fx = m3_lab_f(x / M3_COLOR_WHITE_X);
    fy = m3_lab_f(y / M3_COLOR_WHITE_Y);
    fz = m3_lab_f(z / M3_COLOR_WHITE_Z);

    *out_l = 116.0f * fy - 16.0f;
    *out_a = 500.0f * (fx - fy);
    *out_b = 200.0f * (fy - fz);
    return M3_OK;
}

static int m3_color_lab_to_xyz(M3Scalar l, M3Scalar a, M3Scalar b, M3Scalar *out_x, M3Scalar *out_y, M3Scalar *out_z)
{
    M3Scalar fy;
    M3Scalar fx;
    M3Scalar fz;

    if (out_x == NULL || out_y == NULL || out_z == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#ifdef M3_TESTING
    if (g_color_fail_lab_to_xyz) {
        return M3_ERR_UNKNOWN;
    }
#endif

    fy = (l + 16.0f) / 116.0f;
    fx = fy + a / 500.0f;
    fz = fy - b / 200.0f;

    *out_x = M3_COLOR_WHITE_X * m3_lab_f_inv(fx);
    *out_y = M3_COLOR_WHITE_Y * m3_lab_f_inv(fy);
    *out_z = M3_COLOR_WHITE_Z * m3_lab_f_inv(fz);
    return M3_OK;
}

static int m3_color_lch_to_argb(M3Scalar hue, M3Scalar chroma, M3Scalar tone, m3_u32 *out_argb, M3Bool *out_in_gamut)
{
    M3Scalar rad;
    M3Scalar a;
    M3Scalar b;
    M3Scalar x;
    M3Scalar y;
    M3Scalar z;
    int rc;

    if (out_argb == NULL || out_in_gamut == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#ifdef M3_TESTING
    g_color_lch_to_argb_calls += 1;
    if (g_color_fail_lch_to_argb_call > 0 && g_color_lch_to_argb_calls == g_color_fail_lch_to_argb_call) {
        return M3_ERR_UNKNOWN;
    }
#endif

    rad = (M3Scalar)(hue * (M3Scalar)(M3_COLOR_PI / 180.0));
    a = chroma * (M3Scalar)cos((double)rad);
    b = chroma * (M3Scalar)sin((double)rad);

    rc = m3_color_lab_to_xyz(tone, a, b, &x, &y, &z);
    if (rc != M3_OK) {
        return rc;
    }

    return m3_color_xyz_to_argb(x, y, z, out_argb, out_in_gamut);
}

int M3_CALL m3_color_argb_from_rgba(m3_u8 r, m3_u8 g, m3_u8 b, m3_u8 a, m3_u32 *out_argb)
{
    if (out_argb == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_argb = ((m3_u32)a << 24)
        | ((m3_u32)r << 16)
        | ((m3_u32)g << 8)
        | (m3_u32)b;
    return M3_OK;
}

int M3_CALL m3_color_rgba_from_argb(m3_u32 argb, m3_u8 *out_r, m3_u8 *out_g, m3_u8 *out_b, m3_u8 *out_a)
{
    if (out_r == NULL || out_g == NULL || out_b == NULL || out_a == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_a = (m3_u8)((argb >> 24) & 0xFFu);
    *out_r = (m3_u8)((argb >> 16) & 0xFFu);
    *out_g = (m3_u8)((argb >> 8) & 0xFFu);
    *out_b = (m3_u8)(argb & 0xFFu);
    return M3_OK;
}

int M3_CALL m3_hct_from_argb(m3_u32 argb, M3ColorHct *out_hct)
{
    M3Scalar x;
    M3Scalar y;
    M3Scalar z;
    M3Scalar l;
    M3Scalar a;
    M3Scalar b;
    M3Scalar hue;
    M3Scalar chroma;
    int rc;

    if (out_hct == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_color_argb_to_xyz(argb, &x, &y, &z);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_color_xyz_to_lab(x, y, z, &l, &a, &b);
    if (rc != M3_OK) {
        return rc;
    }

    hue = (M3Scalar)(atan2((double)b, (double)a) * (180.0 / M3_COLOR_PI));
    hue = m3_wrap_hue(hue);
    chroma = (M3Scalar)sqrt((double)(a * a + b * b));

    out_hct->hue = hue;
    out_hct->chroma = chroma;
    out_hct->tone = l;
    return M3_OK;
}

int M3_CALL m3_hct_to_argb(const M3ColorHct *hct, m3_u32 *out_argb)
{
    M3Scalar hue;
    M3Scalar chroma;
    M3Scalar tone;
    M3Scalar low;
    M3Scalar high;
    M3Scalar mid;
    m3_u32 best_argb;
    m3_u32 candidate;
    M3Bool in_gamut;
    int rc;
    int i;

    if (hct == NULL || out_argb == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    hue = hct->hue;
    chroma = hct->chroma;
    tone = hct->tone;

    if (chroma < 0.0f || tone < 0.0f || tone > 100.0f) {
        return M3_ERR_RANGE;
    }

    if (tone <= 0.0f) {
        *out_argb = 0xFF000000u;
        return M3_OK;
    }
    if (tone >= 100.0f) {
        *out_argb = 0xFFFFFFFFu;
        return M3_OK;
    }

    hue = m3_wrap_hue(hue);

    rc = m3_color_lch_to_argb(hue, chroma, tone, &candidate, &in_gamut);
    if (rc != M3_OK) {
        return rc;
    }
    if (in_gamut == M3_TRUE) {
        *out_argb = candidate;
        return M3_OK;
    }

    rc = m3_color_lch_to_argb(hue, 0.0f, tone, &best_argb, &in_gamut);
    if (rc != M3_OK) {
        return rc;
    }

    low = 0.0f;
    high = chroma;
    for (i = 0; i < 24; ++i) {
        mid = (low + high) * 0.5f;
        rc = m3_color_lch_to_argb(hue, mid, tone, &candidate, &in_gamut);
        if (rc != M3_OK) {
            return rc;
        }
        if (in_gamut == M3_TRUE) {
            best_argb = candidate;
            low = mid;
        } else {
            high = mid;
        }
    }

    *out_argb = best_argb;
    return M3_OK;
}

int M3_CALL m3_tonal_palette_init(M3TonalPalette *palette, M3Scalar hue, M3Scalar chroma)
{
    if (palette == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (chroma < 0.0f) {
        return M3_ERR_RANGE;
    }
#ifdef M3_TESTING
    g_color_palette_init_calls += 1;
    if (g_color_fail_palette_init_call > 0 && g_color_palette_init_calls == g_color_fail_palette_init_call) {
        return M3_ERR_UNKNOWN;
    }
#endif

    palette->hue = m3_wrap_hue(hue);
    palette->chroma = chroma;
    return M3_OK;
}

int M3_CALL m3_tonal_palette_tone_argb(const M3TonalPalette *palette, M3Scalar tone, m3_u32 *out_argb)
{
    M3ColorHct hct;

    if (palette == NULL || out_argb == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (tone < 0.0f || tone > 100.0f) {
        return M3_ERR_RANGE;
    }
#ifdef M3_TESTING
    g_color_palette_tone_calls += 1;
    if (g_color_fail_palette_tone_call > 0 && g_color_palette_tone_calls == g_color_fail_palette_tone_call) {
        return M3_ERR_UNKNOWN;
    }
#endif

    hct.hue = palette->hue;
    hct.chroma = palette->chroma;
    hct.tone = tone;
    return m3_hct_to_argb(&hct, out_argb);
}

static M3Scalar m3_scheme_max_chroma(M3Scalar value, M3Scalar minimum)
{
    if (value < minimum) {
        return minimum;
    }
    return value;
}

int M3_CALL m3_scheme_generate(m3_u32 source_argb, M3Bool dark, M3Scheme *out_scheme)
{
    M3ColorHct source_hct;
    M3TonalPalette primary;
    M3TonalPalette secondary;
    M3TonalPalette tertiary;
    M3TonalPalette neutral;
    M3TonalPalette neutral_variant;
    M3TonalPalette error;
    M3Scalar hue;
    M3Scalar chroma;
    int rc;

    if (out_scheme == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_hct_from_argb(source_argb, &source_hct);
    if (rc != M3_OK) {
        return rc;
    }

    hue = source_hct.hue;
    chroma = source_hct.chroma;

    rc = m3_tonal_palette_init(&primary, hue, m3_scheme_max_chroma(chroma, 48.0f));
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_tonal_palette_init(&secondary, hue, m3_scheme_max_chroma(chroma * 0.33f, 16.0f));
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_tonal_palette_init(&tertiary, hue + 60.0f, m3_scheme_max_chroma(chroma * 0.5f, 24.0f));
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_tonal_palette_init(&neutral, hue, 4.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_tonal_palette_init(&neutral_variant, hue, 8.0f);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_tonal_palette_init(&error, 25.0f, 84.0f);
    if (rc != M3_OK) {
        return rc;
    }

    if (dark == M3_TRUE) {
        rc = m3_tonal_palette_tone_argb(&primary, 80.0f, &out_scheme->primary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&primary, 20.0f, &out_scheme->on_primary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&primary, 30.0f, &out_scheme->primary_container);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&primary, 90.0f, &out_scheme->on_primary_container);
        if (rc != M3_OK) {
            return rc;
        }

        rc = m3_tonal_palette_tone_argb(&secondary, 80.0f, &out_scheme->secondary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&secondary, 20.0f, &out_scheme->on_secondary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&secondary, 30.0f, &out_scheme->secondary_container);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&secondary, 90.0f, &out_scheme->on_secondary_container);
        if (rc != M3_OK) {
            return rc;
        }

        rc = m3_tonal_palette_tone_argb(&tertiary, 80.0f, &out_scheme->tertiary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&tertiary, 20.0f, &out_scheme->on_tertiary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&tertiary, 30.0f, &out_scheme->tertiary_container);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&tertiary, 90.0f, &out_scheme->on_tertiary_container);
        if (rc != M3_OK) {
            return rc;
        }

        rc = m3_tonal_palette_tone_argb(&neutral, 10.0f, &out_scheme->background);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral, 90.0f, &out_scheme->on_background);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral, 10.0f, &out_scheme->surface);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral, 90.0f, &out_scheme->on_surface);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral_variant, 30.0f, &out_scheme->surface_variant);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral_variant, 80.0f, &out_scheme->on_surface_variant);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral_variant, 60.0f, &out_scheme->outline);
        if (rc != M3_OK) {
            return rc;
        }

        rc = m3_tonal_palette_tone_argb(&error, 80.0f, &out_scheme->error);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&error, 20.0f, &out_scheme->on_error);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&error, 30.0f, &out_scheme->error_container);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&error, 80.0f, &out_scheme->on_error_container);
        if (rc != M3_OK) {
            return rc;
        }
    } else {
        rc = m3_tonal_palette_tone_argb(&primary, 40.0f, &out_scheme->primary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&primary, 100.0f, &out_scheme->on_primary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&primary, 90.0f, &out_scheme->primary_container);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&primary, 10.0f, &out_scheme->on_primary_container);
        if (rc != M3_OK) {
            return rc;
        }

        rc = m3_tonal_palette_tone_argb(&secondary, 40.0f, &out_scheme->secondary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&secondary, 100.0f, &out_scheme->on_secondary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&secondary, 90.0f, &out_scheme->secondary_container);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&secondary, 10.0f, &out_scheme->on_secondary_container);
        if (rc != M3_OK) {
            return rc;
        }

        rc = m3_tonal_palette_tone_argb(&tertiary, 40.0f, &out_scheme->tertiary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&tertiary, 100.0f, &out_scheme->on_tertiary);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&tertiary, 90.0f, &out_scheme->tertiary_container);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&tertiary, 10.0f, &out_scheme->on_tertiary_container);
        if (rc != M3_OK) {
            return rc;
        }

        rc = m3_tonal_palette_tone_argb(&neutral, 99.0f, &out_scheme->background);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral, 10.0f, &out_scheme->on_background);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral, 99.0f, &out_scheme->surface);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral, 10.0f, &out_scheme->on_surface);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral_variant, 90.0f, &out_scheme->surface_variant);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral_variant, 30.0f, &out_scheme->on_surface_variant);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&neutral_variant, 50.0f, &out_scheme->outline);
        if (rc != M3_OK) {
            return rc;
        }

        rc = m3_tonal_palette_tone_argb(&error, 40.0f, &out_scheme->error);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&error, 100.0f, &out_scheme->on_error);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&error, 90.0f, &out_scheme->error_container);
        if (rc != M3_OK) {
            return rc;
        }
        rc = m3_tonal_palette_tone_argb(&error, 10.0f, &out_scheme->on_error_container);
        if (rc != M3_OK) {
            return rc;
        }
    }

    return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_color_test_set_argb_to_xyz_fail(M3Bool fail)
{
    g_color_fail_argb_to_xyz = fail;
    return M3_OK;
}

int M3_CALL m3_color_test_set_xyz_to_lab_fail(M3Bool fail)
{
    g_color_fail_xyz_to_lab = fail;
    return M3_OK;
}

int M3_CALL m3_color_test_set_lab_to_xyz_fail(M3Bool fail)
{
    g_color_fail_lab_to_xyz = fail;
    return M3_OK;
}

int M3_CALL m3_color_test_set_xyz_to_argb_fail(M3Bool fail)
{
    g_color_fail_xyz_to_argb = fail;
    return M3_OK;
}

int M3_CALL m3_color_test_set_lch_to_argb_fail_call(int call_index)
{
    if (call_index < 0) {
        return M3_ERR_RANGE;
    }
    g_color_fail_lch_to_argb_call = call_index;
    g_color_lch_to_argb_calls = 0;
    return M3_OK;
}

int M3_CALL m3_color_test_set_tonal_palette_init_fail_call(int call_index)
{
    if (call_index < 0) {
        return M3_ERR_RANGE;
    }
    g_color_fail_palette_init_call = call_index;
    g_color_palette_init_calls = 0;
    return M3_OK;
}

int M3_CALL m3_color_test_set_tonal_palette_tone_fail_call(int call_index)
{
    if (call_index < 0) {
        return M3_ERR_RANGE;
    }
    g_color_fail_palette_tone_call = call_index;
    g_color_palette_tone_calls = 0;
    return M3_OK;
}

int M3_CALL m3_color_test_reset_failures(void)
{
    g_color_fail_argb_to_xyz = M3_FALSE;
    g_color_fail_xyz_to_lab = M3_FALSE;
    g_color_fail_lab_to_xyz = M3_FALSE;
    g_color_fail_xyz_to_argb = M3_FALSE;
    g_color_fail_lch_to_argb_call = 0;
    g_color_lch_to_argb_calls = 0;
    g_color_fail_palette_init_call = 0;
    g_color_palette_init_calls = 0;
    g_color_fail_palette_tone_call = 0;
    g_color_palette_tone_calls = 0;
    return M3_OK;
}

int M3_CALL m3_color_test_argb_to_xyz(m3_u32 argb, M3Scalar *out_x, M3Scalar *out_y, M3Scalar *out_z)
{
    return m3_color_argb_to_xyz(argb, out_x, out_y, out_z);
}

int M3_CALL m3_color_test_xyz_to_argb(M3Scalar x, M3Scalar y, M3Scalar z, m3_u32 *out_argb, M3Bool *out_in_gamut)
{
    return m3_color_xyz_to_argb(x, y, z, out_argb, out_in_gamut);
}

int M3_CALL m3_color_test_xyz_to_lab(M3Scalar x, M3Scalar y, M3Scalar z, M3Scalar *out_l, M3Scalar *out_a, M3Scalar *out_b)
{
    return m3_color_xyz_to_lab(x, y, z, out_l, out_a, out_b);
}

int M3_CALL m3_color_test_lab_to_xyz(M3Scalar l, M3Scalar a, M3Scalar b, M3Scalar *out_x, M3Scalar *out_y, M3Scalar *out_z)
{
    return m3_color_lab_to_xyz(l, a, b, out_x, out_y, out_z);
}

int M3_CALL m3_color_test_lch_to_argb(M3Scalar hue, M3Scalar chroma, M3Scalar tone, m3_u32 *out_argb, M3Bool *out_in_gamut)
{
    return m3_color_lch_to_argb(hue, chroma, tone, out_argb, out_in_gamut);
}
#endif
