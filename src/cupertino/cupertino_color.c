#include "cupertino/cupertino_color.h"
#include <stddef.h>

/* Helper to convert hex ARGB to CMPColor components */
static void cupertino_color_from_hex(cmp_u32 argb, CMPColor *out_color) {
    if (out_color) {
        out_color->r = (CMPScalar)((argb >> 16) & 0xFF) / 255.0f;
        out_color->g = (CMPScalar)((argb >> 8) & 0xFF) / 255.0f;
        out_color->b = (CMPScalar)(argb & 0xFF) / 255.0f;
        out_color->a = (CMPScalar)((argb >> 24) & 0xFF) / 255.0f;
    }
}

CMP_API int CMP_CALL cupertino_color_get_system(CupertinoSystemColor color_id, CMPBool is_dark, CMPColor *out_color) {
    cmp_u32 hex_color = 0xFF000000; /* fallback black */

    if (!out_color) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    switch (color_id) {
        case CUPERTINO_COLOR_BLUE:
            hex_color = is_dark ? 0xFF0A84FF : 0xFF007AFF;
            break;
        case CUPERTINO_COLOR_BROWN:
            hex_color = is_dark ? 0xFFAC8E68 : 0xFFA2845E;
            break;
        case CUPERTINO_COLOR_CYAN:
            hex_color = is_dark ? 0xFF64D2FF : 0xFF32ADE6;
            break;
        case CUPERTINO_COLOR_GREEN:
            hex_color = is_dark ? 0xFF30D158 : 0xFF34C759;
            break;
        case CUPERTINO_COLOR_INDIGO:
            hex_color = is_dark ? 0xFF5E5CE6 : 0xFF5856D6;
            break;
        case CUPERTINO_COLOR_MINT:
            hex_color = is_dark ? 0xFF66D4CF : 0xFF00C7BE;
            break;
        case CUPERTINO_COLOR_ORANGE:
            hex_color = is_dark ? 0xFFFF9F0A : 0xFFFF9500;
            break;
        case CUPERTINO_COLOR_PINK:
            hex_color = is_dark ? 0xFFFF375F : 0xFFFF2D55;
            break;
        case CUPERTINO_COLOR_PURPLE:
            hex_color = is_dark ? 0xFFBF5AF2 : 0xFFAF52DE;
            break;
        case CUPERTINO_COLOR_RED:
            hex_color = is_dark ? 0xFFFF453A : 0xFFFF3B30;
            break;
        case CUPERTINO_COLOR_TEAL:
            hex_color = is_dark ? 0xFF40C8E0 : 0xFF30B0C7;
            break;
        case CUPERTINO_COLOR_YELLOW:
            hex_color = is_dark ? 0xFFFFD60A : 0xFFFFCC00;
            break;
        case CUPERTINO_COLOR_GRAY:
            hex_color = is_dark ? 0xFF8E8E93 : 0xFF8E8E93;
            break;
        case CUPERTINO_COLOR_GRAY2:
            hex_color = is_dark ? 0xFF636366 : 0xFFAEAEB2;
            break;
        case CUPERTINO_COLOR_GRAY3:
            hex_color = is_dark ? 0xFF48484A : 0xFFC7C7CC;
            break;
        case CUPERTINO_COLOR_GRAY4:
            hex_color = is_dark ? 0xFF3A3A3C : 0xFFD1D1D6;
            break;
        case CUPERTINO_COLOR_GRAY5:
            hex_color = is_dark ? 0xFF2C2C2E : 0xFFE5E5EA;
            break;
        case CUPERTINO_COLOR_GRAY6:
            hex_color = is_dark ? 0xFF1C1C1E : 0xFFF2F2F7;
            break;
        default:
            return CMP_ERR_INVALID_ARGUMENT;
    }

    cupertino_color_from_hex(hex_color, out_color);
    return CMP_OK;
}
