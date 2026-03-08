#ifndef CUPERTINO_COLOR_H
#define CUPERTINO_COLOR_H

/**
 * @file cupertino_color.h
 * @brief Apple Cupertino semantic system colors for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_visuals.h"

/**
 * @brief Apple Cupertino semantic system color identifiers.
 */
typedef enum CupertinoSystemColor {
    CUPERTINO_COLOR_BLUE,
    CUPERTINO_COLOR_BROWN,
    CUPERTINO_COLOR_CYAN,
    CUPERTINO_COLOR_GREEN,
    CUPERTINO_COLOR_INDIGO,
    CUPERTINO_COLOR_MINT,
    CUPERTINO_COLOR_ORANGE,
    CUPERTINO_COLOR_PINK,
    CUPERTINO_COLOR_PURPLE,
    CUPERTINO_COLOR_RED,
    CUPERTINO_COLOR_TEAL,
    CUPERTINO_COLOR_YELLOW,
    CUPERTINO_COLOR_GRAY,
    CUPERTINO_COLOR_GRAY2,
    CUPERTINO_COLOR_GRAY3,
    CUPERTINO_COLOR_GRAY4,
    CUPERTINO_COLOR_GRAY5,
    CUPERTINO_COLOR_GRAY6
} CupertinoSystemColor;

/**
 * @brief Retrieves the semantic system color for a given identifier, adapting to light/dark mode.
 * @param color_id The system color identifier.
 * @param is_dark CMP_TRUE if dark mode is active, CMP_FALSE otherwise.
 * @param out_color Pointer to receive the resulting color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cupertino_color_get_system(CupertinoSystemColor color_id, CMPBool is_dark, CMPColor *out_color);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_COLOR_H */
