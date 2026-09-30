/**
 * @file rasterizer.c
 * @brief High-precision 2D software rasterizer for cross-platform screenshot
 * generation.
 */

/* clang-format off */
#include "rasterizer.h"
#include "ui_test_visual.h"
#include "../../src/ui_internal_mem.h"

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)  ((u) ? C_MULTIPLATFORM_MALLOC(x) : C_MULTIPLATFORM_MALLOC(x))
#define STBTT_free(x,u)    do { if (u) {} C_MULTIPLATFORM_FREE(x); } while (0)
#include "stb/stb_truetype.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

/* Standard 8x16 Bitmap Font Data (ASCII 32 to 126) */
static const unsigned char g_font_8x16[95][16] = {
    /* 32: Space */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 33: ! */
    {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x3c, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18,
     0x00, 0x00, 0x00, 0x00},
    /* 34: " */
    {0x00, 0x66, 0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 35: # */
    {0x00, 0x00, 0x6c, 0x6c, 0xfe, 0x6c, 0x6c, 0x6c, 0xfe, 0x6c, 0x6c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 36: $ */
    {0x18, 0x18, 0x7c, 0xc6, 0xc2, 0xc0, 0x7c, 0x06, 0x06, 0x86, 0xc6, 0x7c,
     0x18, 0x18, 0x00, 0x00},
    /* 37: % */
    {0x00, 0x00, 0x00, 0xc6, 0xcc, 0x18, 0x30, 0x60, 0xc6, 0xcc, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 38: & */
    {0x00, 0x38, 0x6c, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0xcc, 0xdc, 0x76, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 39: ' */
    {0x00, 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 40: ( */
    {0x00, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 41: ) */
    {0x00, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 42: * */
    {0x00, 0x00, 0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 43: + */
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 44: , */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x08,
     0x10, 0x00, 0x00, 0x00},
    /* 45: - */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 46: . */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 47: / */
    {0x00, 0x02, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 48: 0 */
    {0x00, 0x3c, 0x66, 0xc3, 0xc3, 0xdb, 0xdb, 0xc3, 0xc3, 0x66, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 49: 1 */
    {0x00, 0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 50: 2 */
    {0x00, 0x7c, 0xc6, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0xc6, 0xfe, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 51: 3 */
    {0x00, 0x7c, 0xc6, 0x06, 0x06, 0x3c, 0x06, 0x06, 0x06, 0xc6, 0x7c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 52: 4 */
    {0x00, 0x0c, 0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x0c, 0x0c, 0x1e, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 53: 5 */
    {0x00, 0xfe, 0xc0, 0xc0, 0xc0, 0xfc, 0x06, 0x06, 0x06, 0xc6, 0x7c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 54: 6 */
    {0x00, 0x38, 0x60, 0xc0, 0xc0, 0xfc, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 55: 7 */
    {0x00, 0xfe, 0xc6, 0x06, 0x0c, 0x18, 0x30, 0x30, 0x60, 0x60, 0x60, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 56: 8 */
    {0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 57: 9 */
    {0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0x7e, 0x06, 0x06, 0x06, 0x0c, 0x78, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 58: : */
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 59: ; */
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x08,
     0x10, 0x00, 0x00, 0x00},
    /* 60: < */
    {0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 61: = */
    {0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 62: > */
    {0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 63: ? */
    {0x00, 0x7c, 0xc6, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 64: @ */
    {0x00, 0x7c, 0xc6, 0xde, 0xde, 0xde, 0xdc, 0xc0, 0xc0, 0x7e, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 65: A */
    {0x00, 0x18, 0x3c, 0x66, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 66: B */
    {0x00, 0xfc, 0x66, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x66, 0x66, 0xfc, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 67: C */
    {0x00, 0x3c, 0x66, 0xc2, 0xc0, 0xc0, 0xc0, 0xc0, 0xc2, 0x66, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 68: D */
    {0x00, 0xf8, 0x6c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 69: E */
    {0x00, 0xfe, 0x62, 0x62, 0x68, 0x78, 0x68, 0x60, 0x62, 0x62, 0xfe, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 70: F */
    {0x00, 0xfe, 0x62, 0x62, 0x68, 0x78, 0x68, 0x60, 0x60, 0x60, 0xf0, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 71: G */
    {0x00, 0x3c, 0x66, 0xc2, 0xc0, 0xc0, 0xce, 0xc6, 0xc6, 0x66, 0x3a, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 72: H */
    {0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 73: I */
    {0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 74: J */
    {0x00, 0x1e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xc6, 0xc6, 0x7c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 75: K */
    {0x00, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc6, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 76: L */
    {0x00, 0xf0, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 77: M */
    {0x00, 0xc3, 0xe7, 0xff, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 78: N */
    {0x00, 0xc3, 0xe3, 0xf3, 0xdb, 0xcf, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 79: O */
    {0x00, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 80: P */
    {0x00, 0xfc, 0x66, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x60, 0xf0, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 81: Q */
    {0x00, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0x6e, 0x3c, 0x0e,
     0x00, 0x00, 0x00, 0x00},
    /* 82: R */
    {0x00, 0xfc, 0x66, 0x66, 0x66, 0x7c, 0xd8, 0xcc, 0xc6, 0xc6, 0xc6, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 83: S */
    {0x00, 0x7c, 0xc6, 0xc6, 0x60, 0x38, 0x0c, 0x06, 0xc6, 0xc6, 0x7c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 84: T */
    {0x00, 0x7e, 0x7e, 0x5a, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 85: U */
    {0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 86: V */
    {0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 87: W */
    {0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xe7, 0xc3, 0xc3, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 88: X */
    {0x00, 0xc3, 0xc3, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 89: Y */
    {0x00, 0xc3, 0xc3, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 90: Z */
    {0x00, 0xff, 0xc3, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0xc3, 0xff, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 91: [ */
    {0x00, 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 92: \ */
    {0x00, 0x80, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 93: ] */
    {0x00, 0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 94: ^ */
    {0x18, 0x3c, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 95: _ */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0xff, 0x00, 0x00},
    /* 96: ` */
    {0x00, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 97: a */
    {0x00, 0x00, 0x00, 0x00, 0x7c, 0x06, 0x7e, 0xc6, 0xc6, 0xce, 0x76, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 98: b */
    {0x00, 0xe0, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x66, 0xfc, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 99: c */
    {0x00, 0x00, 0x00, 0x00, 0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 100: d */
    {0x00, 0x0e, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 101: e */
    {0x00, 0x00, 0x00, 0x00, 0x3c, 0x66, 0xc2, 0xfe, 0xc0, 0x66, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 102: f */
    {0x00, 0x1c, 0x36, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 103: g */
    {0x00, 0x00, 0x00, 0x00, 0x3e, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c,
     0x00, 0x00, 0x00, 0x00},
    /* 104: h */
    {0x00, 0xe0, 0x60, 0x60, 0x6c, 0x76, 0x66, 0x66, 0x66, 0x66, 0xe7, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 105: i */
    {0x00, 0x18, 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 106: j */
    {0x00, 0x06, 0x06, 0x00, 0x0e, 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3c,
     0x00, 0x00, 0x00, 0x00},
    /* 107: k */
    {0x00, 0xe0, 0x60, 0x60, 0x66, 0x6c, 0x78, 0x78, 0x6c, 0x66, 0xe7, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 108: l */
    {0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 109: m */
    {0x00, 0x00, 0x00, 0x00, 0xec, 0xfe, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 110: n */
    {0x00, 0x00, 0x00, 0x00, 0xdc, 0x66, 0x66, 0x66, 0x66, 0x66, 0xe7, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 111: o */
    {0x00, 0x00, 0x00, 0x00, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 112: p */
    {0x00, 0x00, 0x00, 0x00, 0xdc, 0x66, 0x66, 0x66, 0x7c, 0x60, 0xf0, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 113: q */
    {0x00, 0x00, 0x00, 0x00, 0x3b, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x0f, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 114: r */
    {0x00, 0x00, 0x00, 0x00, 0xdc, 0x76, 0x66, 0x60, 0x60, 0x60, 0xf0, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 115: s */
    {0x00, 0x00, 0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x06, 0xc6, 0x7c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 116: t */
    {0x00, 0x10, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x30, 0x36, 0x1c, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 117: u */
    {0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7e, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 118: v */
    {0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x66, 0x66, 0x3c, 0x18, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 119: w */
    {0x00, 0x00, 0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xdb, 0xdb, 0xff, 0x66, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 120: x */
    {0x00, 0x00, 0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0xc3, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 121: y */
    {0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0x3e, 0x06, 0x0c, 0xf8,
     0x00, 0x00, 0x00, 0x00},
    /* 122: z */
    {0x00, 0x00, 0x00, 0x00, 0xfe, 0xcc, 0x18, 0x30, 0x60, 0xc6, 0xfe, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 123: { */
    {0x00, 0x0e, 0x18, 0x18, 0x18, 0x70, 0x18, 0x18, 0x18, 0x18, 0x0e, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 124: | */
    {0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 125: } */
    {0x00, 0x70, 0x18, 0x18, 0x18, 0x0e, 0x18, 0x18, 0x18, 0x18, 0x70, 0x00,
     0x00, 0x00, 0x00, 0x00},
    /* 126: ~ */
    {0x00, 0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}};

static float g_rasterizer_dpi_scale = 1.0f;

ui_error_t rasterizer_set_dpi_scale(float dpi_scale) {
  if (dpi_scale <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  g_rasterizer_dpi_scale = dpi_scale;
  return UI_ERROR_NONE;
}

ui_error_t rasterizer_get_dpi_scale(float *out_dpi_scale) {
  if (!out_dpi_scale) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_dpi_scale = g_rasterizer_dpi_scale;
  return UI_ERROR_NONE;
}

ui_error_t canvas_create(int width, int height, struct canvas **out_canvas) {
  struct canvas *c;
  int scaled_w;
  int scaled_h;
  if (!out_canvas) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_canvas = NULL;
  if (width <= 0 || height <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaled_w = (int)((float)width * g_rasterizer_dpi_scale);
  scaled_h = (int)((float)height * g_rasterizer_dpi_scale);
  if (scaled_w <= 0 || scaled_h <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  c = (struct canvas *)C_MULTIPLATFORM_MALLOC(sizeof(struct canvas));
  if (!c) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  c->width = scaled_w;
  c->height = scaled_h;
  c->pixels = (unsigned char *)C_MULTIPLATFORM_MALLOC((size_t)scaled_w *
                                                      (size_t)scaled_h * 4);
  if (!c->pixels) {
    C_MULTIPLATFORM_FREE(c);
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(c->pixels, 0, (size_t)scaled_w * (size_t)scaled_h * 4);
  *out_canvas = c;
  return UI_ERROR_NONE;
}

ui_error_t canvas_destroy(struct canvas *c) {
  if (!c) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (c->pixels) {
    C_MULTIPLATFORM_FREE(c->pixels);
  }
  C_MULTIPLATFORM_FREE(c);
  return UI_ERROR_NONE;
}

ui_error_t canvas_clear(struct canvas *c, ui_color_t color) {
  int total;
  int i;
  unsigned char r, g, b, a;
  if (!c || !c->pixels) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  a = (unsigned char)UI_COLOR_ALPHA(color);
  r = (unsigned char)UI_COLOR_RED(color);
  g = (unsigned char)UI_COLOR_GREEN(color);
  b = (unsigned char)UI_COLOR_BLUE(color);

  total = c->width * c->height;
  for (i = 0; i < total; ++i) {
    c->pixels[i * 4 + 0] = r;
    c->pixels[i * 4 + 1] = g;
    c->pixels[i * 4 + 2] = b;
    c->pixels[i * 4 + 3] = a;
  }
  return UI_ERROR_NONE;
}

static void blend_pixel(struct canvas *c, int x, int y, unsigned char sr,
                        unsigned char sg, unsigned char sb, float cov,
                        float alpha_mult) {
  size_t idx;
  float sa;
  float inv_sa;
  float dr, dg, db, da;

  if (x < 0 || x >= c->width || y < 0 || y >= c->height || cov <= 0.0f)
    return;

  idx = ((size_t)y * (size_t)c->width + (size_t)x) * 4;
  sa = cov * alpha_mult;
  if (sa <= 0.0f)
    return;
  if (sa > 1.0f)
    sa = 1.0f;

  inv_sa = 1.0f - sa;
  dr = (float)c->pixels[idx + 0];
  dg = (float)c->pixels[idx + 1];
  db = (float)c->pixels[idx + 2];
  da = (float)c->pixels[idx + 3] / 255.0f;

  c->pixels[idx + 0] = (unsigned char)((float)sr * sa + dr * inv_sa);
  c->pixels[idx + 1] = (unsigned char)((float)sg * sa + dg * inv_sa);
  c->pixels[idx + 2] = (unsigned char)((float)sb * sa + db * inv_sa);
  c->pixels[idx + 3] = (unsigned char)((sa + da * inv_sa) * 255.0f);
}

static float rounded_box_sdf(float px, float py, float cx, float cy,
                             float half_w, float half_h, float r) {
  float qx, qy;
  if (r > half_w)
    r = half_w;
  if (r > half_h)
    r = half_h;
  if (r < 0.0f)
    r = 0.0f;

  qx = fabsf(px - cx) - (half_w - r);
  qy = fabsf(py - cy) - (half_h - r);

  if (qx > 0.0f && qy > 0.0f) {
    return sqrtf(qx * qx + qy * qy) - r;
  }
  return (qx > qy ? qx : qy) - r;
}

ui_error_t draw_rounded_rect(struct canvas *c, float x, float y, float w,
                             float h, float radius, ui_color_t color) {
  int min_x, max_x, min_y, max_y;
  int px, py;
  float cx, cy, half_w, half_h;
  unsigned char r, g, b;
  float alpha_mult;

  if (!c || !c->pixels) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (w <= 0.0f || h <= 0.0f) {
    return UI_ERROR_NONE;
  }

  alpha_mult = (float)UI_COLOR_ALPHA(color) / 255.0f;
  if (alpha_mult <= 0.0f) {
    return UI_ERROR_NONE;
  }
  r = (unsigned char)UI_COLOR_RED(color);
  g = (unsigned char)UI_COLOR_GREEN(color);
  b = (unsigned char)UI_COLOR_BLUE(color);

  cx = x + w * 0.5f;
  cy = y + h * 0.5f;
  half_w = w * 0.5f;
  half_h = h * 0.5f;

  min_x = (int)floorf(x - 1.0f);
  max_x = (int)ceilf(x + w + 1.0f);
  min_y = (int)floorf(y - 1.0f);
  max_y = (int)ceilf(y + h + 1.0f);

  if (min_x < 0)
    min_x = 0;
  if (max_x > c->width)
    max_x = c->width;
  if (min_y < 0)
    min_y = 0;
  if (max_y > c->height)
    max_y = c->height;

  for (py = min_y; py < max_y; ++py) {
    for (px = min_x; px < max_x; ++px) {
      float dist = rounded_box_sdf((float)px + 0.5f, (float)py + 0.5f, cx, cy,
                                   half_w, half_h, radius);
      float cov = 0.5f - dist;
      if (cov > 1.0f)
        cov = 1.0f;
      if (cov > 0.0f) {
        blend_pixel(c, px, py, r, g, b, cov, alpha_mult);
      }
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t draw_rounded_rect_stroke(struct canvas *c, float x, float y, float w,
                                    float h, float radius, float stroke_width,
                                    ui_color_t color) {
  int min_x, max_x, min_y, max_y;
  int px, py;
  float cx, cy, half_w, half_h;
  unsigned char r, g, b;
  float alpha_mult;

  if (!c || !c->pixels) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (w <= 0.0f || h <= 0.0f || stroke_width <= 0.0f) {
    return UI_ERROR_NONE;
  }

  alpha_mult = (float)UI_COLOR_ALPHA(color) / 255.0f;
  if (alpha_mult <= 0.0f) {
    return UI_ERROR_NONE;
  }
  r = (unsigned char)UI_COLOR_RED(color);
  g = (unsigned char)UI_COLOR_GREEN(color);
  b = (unsigned char)UI_COLOR_BLUE(color);

  cx = x + w * 0.5f;
  cy = y + h * 0.5f;
  half_w = w * 0.5f;
  half_h = h * 0.5f;

  min_x = (int)floorf(x - stroke_width - 1.0f);
  max_x = (int)ceilf(x + w + stroke_width + 1.0f);
  min_y = (int)floorf(y - stroke_width - 1.0f);
  max_y = (int)ceilf(y + h + stroke_width + 1.0f);

  if (min_x < 0)
    min_x = 0;
  if (max_x > c->width)
    max_x = c->width;
  if (min_y < 0)
    min_y = 0;
  if (max_y > c->height)
    max_y = c->height;

  for (py = min_y; py < max_y; ++py) {
    for (px = min_x; px < max_x; ++px) {
      float dist = rounded_box_sdf((float)px + 0.5f, (float)py + 0.5f, cx, cy,
                                   half_w, half_h, radius);
      float d_stroke = fabsf(dist) - stroke_width * 0.5f;
      float cov = 0.5f - d_stroke;
      if (cov > 1.0f)
        cov = 1.0f;
      if (cov > 0.0f) {
        blend_pixel(c, px, py, r, g, b, cov, alpha_mult);
      }
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t draw_shadow(struct canvas *c, float x, float y, float w, float h,
                       float radius, int elevation) {
  float dy, blur, shadow_alpha;
  float sigma, two_sigma_sq;
  float cx, cy, half_w, half_h;
  int min_x, max_x, min_y, max_y;
  int px, py;

  if (!c || !c->pixels) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (elevation <= 0) {
    return UI_ERROR_NONE;
  }

  switch (elevation) {
  case 1:
    dy = 1.0f;
    blur = 3.0f;
    shadow_alpha = 0.12f;
    break;
  case 2:
    dy = 2.0f;
    blur = 6.0f;
    shadow_alpha = 0.16f;
    break;
  case 3:
    dy = 4.0f;
    blur = 10.0f;
    shadow_alpha = 0.20f;
    break;
  case 4:
    dy = 6.0f;
    blur = 14.0f;
    shadow_alpha = 0.22f;
    break;
  case 5:
  default:
    dy = 8.0f;
    blur = 18.0f;
    shadow_alpha = 0.25f;
    break;
  }

  sigma = blur * 0.5f;
  two_sigma_sq = 2.0f * sigma * sigma;

  cx = x + w * 0.5f;
  cy = (y + dy) + h * 0.5f;
  half_w = w * 0.5f;
  half_h = h * 0.5f;

  min_x = (int)floorf(x - blur * 2.0f);
  max_x = (int)ceilf(x + w + blur * 2.0f);
  min_y = (int)floorf(y + dy - blur * 2.0f);
  max_y = (int)ceilf(y + h + dy + blur * 2.0f);

  if (min_x < 0)
    min_x = 0;
  if (max_x > c->width)
    max_x = c->width;
  if (min_y < 0)
    min_y = 0;
  if (max_y > c->height)
    max_y = c->height;

  for (py = min_y; py < max_y; ++py) {
    for (px = min_x; px < max_x; ++px) {
      float dist = rounded_box_sdf((float)px + 0.5f, (float)py + 0.5f, cx, cy,
                                   half_w, half_h, radius);
      float cov;
      if (dist <= 0.0f) {
        cov = 1.0f;
      } else {
        cov = expf(-(dist * dist) / two_sigma_sq);
      }
      if (cov > 0.005f) {
        blend_pixel(c, px, py, 0, 0, 0, cov, shadow_alpha);
      }
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t draw_circle(struct canvas *c, float cx, float cy, float radius,
                       ui_color_t color) {
  int min_x, max_x, min_y, max_y;
  int px, py;
  unsigned char r, g, b;
  float alpha_mult;

  if (!c || !c->pixels) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (radius <= 0.0f) {
    return UI_ERROR_NONE;
  }

  alpha_mult = (float)UI_COLOR_ALPHA(color) / 255.0f;
  if (alpha_mult <= 0.0f) {
    return UI_ERROR_NONE;
  }
  r = (unsigned char)UI_COLOR_RED(color);
  g = (unsigned char)UI_COLOR_GREEN(color);
  b = (unsigned char)UI_COLOR_BLUE(color);

  min_x = (int)floorf(cx - radius - 1.0f);
  max_x = (int)ceilf(cx + radius + 1.0f);
  min_y = (int)floorf(cy - radius - 1.0f);
  max_y = (int)ceilf(cy + radius + 1.0f);

  if (min_x < 0)
    min_x = 0;
  if (max_x > c->width)
    max_x = c->width;
  if (min_y < 0)
    min_y = 0;
  if (max_y > c->height)
    max_y = c->height;

  for (py = min_y; py < max_y; ++py) {
    for (px = min_x; px < max_x; ++px) {
      float dx = (float)px + 0.5f - cx;
      float dy = (float)py + 0.5f - cy;
      float dist = sqrtf(dx * dx + dy * dy) - radius;
      float cov = 0.5f - dist;
      if (cov > 1.0f)
        cov = 1.0f;
      if (cov > 0.0f) {
        blend_pixel(c, px, py, r, g, b, cov, alpha_mult);
      }
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t draw_circle_stroke(struct canvas *c, float cx, float cy,
                              float radius, float stroke_width,
                              ui_color_t color) {
  int min_x, max_x, min_y, max_y;
  int px, py;
  unsigned char r, g, b;
  float alpha_mult;

  if (!c || !c->pixels) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (radius <= 0.0f || stroke_width <= 0.0f) {
    return UI_ERROR_NONE;
  }

  alpha_mult = (float)UI_COLOR_ALPHA(color) / 255.0f;
  if (alpha_mult <= 0.0f) {
    return UI_ERROR_NONE;
  }
  r = (unsigned char)UI_COLOR_RED(color);
  g = (unsigned char)UI_COLOR_GREEN(color);
  b = (unsigned char)UI_COLOR_BLUE(color);

  min_x = (int)floorf(cx - radius - stroke_width - 1.0f);
  max_x = (int)ceilf(cx + radius + stroke_width + 1.0f);
  min_y = (int)floorf(cy - radius - stroke_width - 1.0f);
  max_y = (int)ceilf(cy + radius + stroke_width + 1.0f);

  if (min_x < 0)
    min_x = 0;
  if (max_x > c->width)
    max_x = c->width;
  if (min_y < 0)
    min_y = 0;
  if (max_y > c->height)
    max_y = c->height;

  for (py = min_y; py < max_y; ++py) {
    for (px = min_x; px < max_x; ++px) {
      float dx = (float)px + 0.5f - cx;
      float dy = (float)py + 0.5f - cy;
      float dist =
          fabsf(sqrtf(dx * dx + dy * dy) - radius) - stroke_width * 0.5f;
      float cov = 0.5f - dist;
      if (cov > 1.0f)
        cov = 1.0f;
      if (cov > 0.0f) {
        blend_pixel(c, px, py, r, g, b, cov, alpha_mult);
      }
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t draw_line(struct canvas *c, float x0, float y0, float x1, float y1,
                     float width, ui_color_t color) {
  int min_x, max_x, min_y, max_y;
  int px, py;
  float vx, vy, len_sq;
  unsigned char r, g, b;
  float alpha_mult;

  if (!c || !c->pixels) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (width <= 0.0f) {
    return UI_ERROR_NONE;
  }

  alpha_mult = (float)UI_COLOR_ALPHA(color) / 255.0f;
  if (alpha_mult <= 0.0f) {
    return UI_ERROR_NONE;
  }
  r = (unsigned char)UI_COLOR_RED(color);
  g = (unsigned char)UI_COLOR_GREEN(color);
  b = (unsigned char)UI_COLOR_BLUE(color);

  min_x = (int)floorf((x0 < x1 ? x0 : x1) - width - 1.0f);
  max_x = (int)ceilf((x0 > x1 ? x0 : x1) + width + 1.0f);
  min_y = (int)floorf((y0 < y1 ? y0 : y1) - width - 1.0f);
  max_y = (int)ceilf((y0 > y1 ? y0 : y1) + width + 1.0f);

  if (min_x < 0)
    min_x = 0;
  if (max_x > c->width)
    max_x = c->width;
  if (min_y < 0)
    min_y = 0;
  if (max_y > c->height)
    max_y = c->height;

  vx = x1 - x0;
  vy = y1 - y0;
  len_sq = vx * vx + vy * vy;

  for (py = min_y; py < max_y; ++py) {
    for (px = min_x; px < max_x; ++px) {
      float wx = (float)px + 0.5f - x0;
      float wy = (float)py + 0.5f - y0;
      float t, dist_sq;
      if (len_sq <= 0.0001f) {
        t = 0.0f;
      } else {
        t = (wx * vx + wy * vy) / len_sq;
        if (t < 0.0f)
          t = 0.0f;
        else if (t > 1.0f)
          t = 1.0f;
      }
      {
        float qx = x0 + t * vx - ((float)px + 0.5f);
        float qy = y0 + t * vy - ((float)py + 0.5f);
        dist_sq = qx * qx + qy * qy;
      }
      {
        float dist = sqrtf(dist_sq) - width * 0.5f;
        float cov = 0.5f - dist;
        if (cov > 1.0f)
          cov = 1.0f;
        if (cov > 0.0f) {
          blend_pixel(c, px, py, r, g, b, cov, alpha_mult);
        }
      }
    }
  }
  return UI_ERROR_NONE;
}

static unsigned char *g_ttf_buffer = NULL;
static stbtt_fontinfo g_stb_font;
static int g_has_ttf_font = 0;

ui_error_t rasterizer_load_font(const char *ttf_path) {
  FILE *f;
  long sz;
  unsigned char *buf;
  size_t rd;

  if (!ttf_path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#if defined(_MSC_VER)
  {
    errno_t err = fopen_s(&f, ttf_path, "rb");
    if (err != 0 || !f) {
      return UI_ERROR_NOT_FOUND;
    }
  }
#else
  f = fopen(ttf_path, "rb");
  if (!f) {
    return UI_ERROR_NOT_FOUND;
  }
#endif
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return UI_ERROR_IO_FAILED;
  }
  sz = ftell(f);
  if (sz <= 0) {
    fclose(f);
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (fseek(f, 0, SEEK_SET) != 0) {
    fclose(f);
    return UI_ERROR_IO_FAILED;
  }
  buf = (unsigned char *)C_MULTIPLATFORM_MALLOC((size_t)sz);
  if (!buf) {
    fclose(f);
    return UI_ERROR_OUT_OF_MEMORY;
  }
  rd = fread(buf, 1, (size_t)sz, f);
  fclose(f);
  if (rd != (size_t)sz) {
    C_MULTIPLATFORM_FREE(buf);
    return UI_ERROR_IO_FAILED;
  }
  if (!stbtt_InitFont(&g_stb_font, buf, 0)) {
    C_MULTIPLATFORM_FREE(buf);
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (g_ttf_buffer) {
    C_MULTIPLATFORM_FREE(g_ttf_buffer);
  }
  g_ttf_buffer = buf;
  g_has_ttf_font = 1;
  return UI_ERROR_NONE;
}

ui_error_t measure_text_width(const char *text, float scale, float *out_width) {
  if (!text || !out_width) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (g_has_ttf_font) {
    float pixel_height = scale * 16.0f;
    float f_scale = stbtt_ScaleForPixelHeight(&g_stb_font, pixel_height);
    float total_w = 0.0f;
    const char *p;
    for (p = text; *p; ++p) {
      int advance = 0, lsb = 0;
      int ch = (unsigned char)*p;
      stbtt_GetCodepointHMetrics(&g_stb_font, ch, &advance, &lsb);
      total_w += (float)advance * f_scale;
      if (*(p + 1)) {
        int kern = stbtt_GetCodepointKernAdvance(&g_stb_font, ch,
                                                 (int)(unsigned char)*(p + 1));
        total_w += (float)kern * f_scale;
      }
    }
    *out_width = total_w;
    return UI_ERROR_NONE;
  }
  *out_width = (float)strlen(text) * (float)FONT_WIDTH * scale;
  return UI_ERROR_NONE;
}

ui_error_t draw_text(struct canvas *c, float x, float y, const char *text,
                     float scale, ui_color_t color) {
  unsigned char r, g, b;
  float alpha_mult;
  float cur_x;
  const char *p;

  if (!c || !c->pixels || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (scale <= 0.0f) {
    return UI_ERROR_NONE;
  }

  alpha_mult = (float)UI_COLOR_ALPHA(color) / 255.0f;
  if (alpha_mult <= 0.0f) {
    return UI_ERROR_NONE;
  }
  r = (unsigned char)UI_COLOR_RED(color);
  g = (unsigned char)UI_COLOR_GREEN(color);
  b = (unsigned char)UI_COLOR_BLUE(color);

  if (g_has_ttf_font) {
    float pixel_height = scale * 16.0f;
    float f_scale = stbtt_ScaleForPixelHeight(&g_stb_font, pixel_height);
    int ascent = 0, descent = 0, line_gap = 0;
    float baseline_y;
    stbtt_GetFontVMetrics(&g_stb_font, &ascent, &descent, &line_gap);
    baseline_y = y + (float)ascent * f_scale;

    cur_x = x;
    for (p = text; *p; ++p) {
      int ch = (unsigned char)*p;
      int advance = 0, lsb = 0;
      int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
      int gw, gh;

      stbtt_GetCodepointHMetrics(&g_stb_font, ch, &advance, &lsb);
      stbtt_GetCodepointBitmapBoxSubpixel(&g_stb_font, ch, f_scale, f_scale,
                                          0.0f, 0.0f, &x0, &y0, &x1, &y1);
      gw = x1 - x0;
      gh = y1 - y0;
      if (gw > 0 && gh > 0) {
        unsigned char *bitmap =
            (unsigned char *)C_MULTIPLATFORM_MALLOC((size_t)(gw * gh));
        if (bitmap) {
          int row, col;
          stbtt_MakeCodepointBitmapSubpixel(&g_stb_font, bitmap, gw, gh, gw,
                                            f_scale, f_scale, 0.0f, 0.0f, ch);
          for (row = 0; row < gh; ++row) {
            for (col = 0; col < gw; ++col) {
              unsigned char alpha_b = bitmap[row * gw + col];
              if (alpha_b > 0) {
                float cov = (float)alpha_b / 255.0f;
                int dst_x = (int)floorf(cur_x + (float)x0 + (float)col);
                int dst_y = (int)floorf(baseline_y + (float)y0 + (float)row);
                blend_pixel(c, dst_x, dst_y, r, g, b, cov, alpha_mult);
              }
            }
          }
          C_MULTIPLATFORM_FREE(bitmap);
        }
      }
      cur_x += (float)advance * f_scale;
      if (*(p + 1)) {
        int kern = stbtt_GetCodepointKernAdvance(&g_stb_font, ch,
                                                 (int)(unsigned char)*(p + 1));
        cur_x += (float)kern * f_scale;
      }
    }
    return UI_ERROR_NONE;
  }

  cur_x = x;
  for (p = text; *p; ++p) {
    unsigned char ch = (unsigned char)*p;
    int glyph_idx;
    int row, col;

    if (ch < 32 || ch > 126) {
      cur_x += (float)FONT_WIDTH * scale;
      continue;
    }
    glyph_idx = ch - 32;

    for (row = 0; row < FONT_HEIGHT; ++row) {
      unsigned char row_bits = g_font_8x16[glyph_idx][row];
      for (col = 0; col < FONT_WIDTH; ++col) {
        if (row_bits & (0x80 >> col)) {
          float px = cur_x + (float)col * scale;
          float py = y + (float)row * scale;
          float px1 = px + scale;
          float py1 = py + scale;
          int min_px = (int)floorf(px);
          int max_px = (int)ceilf(px1);
          int min_py = (int)floorf(py);
          int max_py = (int)ceilf(py1);
          int ix, iy;

          for (iy = min_py; iy < max_py; ++iy) {
            float y0 = (float)iy;
            float y1 = y0 + 1.0f;
            float oy = (y1 < py1 ? y1 : py1) - (y0 > py ? y0 : py);
            if (oy <= 0.0f) {
              continue;
            }
            for (ix = min_px; ix < max_px; ++ix) {
              float x0 = (float)ix;
              float x1 = x0 + 1.0f;
              float ox = (x1 < px1 ? x1 : px1) - (x0 > px ? x0 : px);
              if (ox <= 0.0f) {
                continue;
              }
              {
                float area_cov = ox * oy;
                if (scale < 1.0f && scale > 0.0f) {
                  area_cov /= (scale * scale);
                }
                if (area_cov > 1.0f) {
                  area_cov = 1.0f;
                }
                blend_pixel(c, ix, iy, r, g, b, area_cov, alpha_mult);
              }
            }
          }
        }
      }
    }
    cur_x += (float)FONT_WIDTH * scale;
  }
  return UI_ERROR_NONE;
}

ui_error_t draw_text_centered(struct canvas *c, float cx, float y,
                              const char *text, float scale, ui_color_t color) {
  float w = 0.0f;
  ui_error_t rc;

  if (!c || !c->pixels || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  rc = measure_text_width(text, scale, &w);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return draw_text(c, cx - w * 0.5f, y, text, scale, color);
}

ui_error_t draw_icon(struct canvas *c, float cx, float cy, enum icon_type type,
                     float size, ui_color_t color) {
  float s, stroke;
  ui_error_t rc = UI_ERROR_NONE;

  if (!c || !c->pixels) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (size <= 0.0f) {
    return UI_ERROR_NONE;
  }

  s = size * 0.5f;
  stroke = size > 24.0f ? 2.5f : 2.0f;

  switch (type) {
  case ICON_CHECK:
    rc = draw_line(c, cx - s * 0.7f, cy, cx - s * 0.2f, cy + s * 0.6f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx - s * 0.2f, cy + s * 0.6f, cx + s * 0.8f,
                     cy - s * 0.6f, stroke, color);

  case ICON_CLOSE:
    rc = draw_line(c, cx - s * 0.6f, cy - s * 0.6f, cx + s * 0.6f,
                   cy + s * 0.6f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx + s * 0.6f, cy - s * 0.6f, cx - s * 0.6f,
                     cy + s * 0.6f, stroke, color);

  case ICON_SEARCH: {
    float r = s * 0.55f;
    float scx = cx - s * 0.2f;
    float scy = cy - s * 0.2f;
    rc = draw_circle_stroke(c, scx, scy, r, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, scx + r * 0.7f, scy + r * 0.7f, cx + s * 0.8f,
                     cy + s * 0.8f, stroke + 0.5f, color);
  }

  case ICON_CALENDAR:
    rc = draw_rounded_rect_stroke(c, cx - s * 0.8f, cy - s * 0.6f, s * 1.6f,
                                  s * 1.5f, 3.0f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.8f, cy - s * 0.1f, cx + s * 0.8f,
                   cy - s * 0.1f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.5f, cy - s * 0.9f, cx - s * 0.5f,
                   cy - s * 0.5f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx + s * 0.5f, cy - s * 0.9f, cx + s * 0.5f,
                   cy - s * 0.5f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_circle(c, cx - s * 0.3f, cy + s * 0.3f, 1.5f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_circle(c, cx + s * 0.3f, cy + s * 0.3f, 1.5f, color);

  case ICON_CLOCK:
    rc = draw_circle_stroke(c, cx, cy, s * 0.85f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx, cy, cx, cy - s * 0.5f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx, cy, cx + s * 0.45f, cy, stroke, color);

  case ICON_ARROW_DOWN:
    rc = draw_line(c, cx - s * 0.6f, cy - s * 0.3f, cx, cy + s * 0.3f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx, cy + s * 0.3f, cx + s * 0.6f, cy - s * 0.3f, stroke,
                     color);

  case ICON_ARROW_UP:
    rc = draw_line(c, cx - s * 0.6f, cy + s * 0.3f, cx, cy - s * 0.3f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx, cy - s * 0.3f, cx + s * 0.6f, cy + s * 0.3f, stroke,
                     color);

  case ICON_ARROW_LEFT:
    rc = draw_line(c, cx + s * 0.7f, cy, cx - s * 0.7f, cy, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.7f, cy, cx - s * 0.1f, cy - s * 0.6f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx - s * 0.7f, cy, cx - s * 0.1f, cy + s * 0.6f, stroke,
                     color);

  case ICON_ARROW_RIGHT:
    rc = draw_line(c, cx - s * 0.7f, cy, cx + s * 0.7f, cy, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx + s * 0.7f, cy, cx + s * 0.1f, cy - s * 0.6f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx + s * 0.7f, cy, cx + s * 0.1f, cy + s * 0.6f, stroke,
                     color);

  case ICON_ADD:
    rc = draw_line(c, cx - s * 0.7f, cy, cx + s * 0.7f, cy, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx, cy - s * 0.7f, cx, cy + s * 0.7f, stroke, color);

  case ICON_EDIT:
    rc = draw_line(c, cx - s * 0.5f, cy + s * 0.5f, cx + s * 0.5f,
                   cy - s * 0.5f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.7f, cy + s * 0.7f, cx - s * 0.4f,
                   cy + s * 0.7f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx - s * 0.7f, cy + s * 0.7f, cx - s * 0.7f,
                     cy + s * 0.4f, stroke, color);

  case ICON_MORE_VERT:
    rc = draw_circle(c, cx, cy - s * 0.6f, 2.0f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_circle(c, cx, cy, 2.0f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_circle(c, cx, cy + s * 0.6f, 2.0f, color);

  case ICON_MENU:
    rc = draw_line(c, cx - s * 0.7f, cy - s * 0.5f, cx + s * 0.7f,
                   cy - s * 0.5f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.7f, cy, cx + s * 0.7f, cy, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx - s * 0.7f, cy + s * 0.5f, cx + s * 0.7f,
                     cy + s * 0.5f, stroke, color);

  case ICON_STAR:
  case ICON_STAR_OUTLINE: {
    float r_out = s * 0.85f;
    float r_in = s * 0.38f;
    int k;
    for (k = 0; k < 5; ++k) {
      float a0 = -1.5707963f + (float)k * 1.256637f;
      float a1 = a0 + 0.6283185f;
      float a2 = a0 + 1.256637f;
      float x0 = cx + cosf(a0) * r_out;
      float y0 = cy + sinf(a0) * r_out;
      float x1 = cx + cosf(a1) * r_in;
      float y1 = cy + sinf(a1) * r_in;
      float x2 = cx + cosf(a2) * r_out;
      float y2 = cy + sinf(a2) * r_out;
      rc = draw_line(c, x0, y0, x1, y1, stroke, color);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = draw_line(c, x1, y1, x2, y2, stroke, color);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    if (type == ICON_STAR) {
      return draw_circle(c, cx, cy, r_in, color);
    }
    return UI_ERROR_NONE;
  }

  case ICON_HEART:
    rc = draw_circle(c, cx - s * 0.35f, cy - s * 0.2f, s * 0.42f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_circle(c, cx + s * 0.35f, cy - s * 0.2f, s * 0.42f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.7f, cy - s * 0.1f, cx, cy + s * 0.75f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx + s * 0.7f, cy - s * 0.1f, cx, cy + s * 0.75f,
                     stroke, color);

  case ICON_PERSON:
    rc = draw_circle(c, cx, cy - s * 0.35f, s * 0.38f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_rounded_rect(c, cx - s * 0.7f, cy + s * 0.15f, s * 1.4f,
                             s * 0.7f, s * 0.4f, color);

  case ICON_SETTINGS:
    rc = draw_circle_stroke(c, cx, cy, s * 0.65f, stroke + 1.0f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_circle(c, cx, cy, s * 0.3f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.85f, cy, cx + s * 0.85f, cy, stroke + 1.5f,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx, cy - s * 0.85f, cx, cy + s * 0.85f, stroke + 1.5f,
                     color);

  case ICON_HOME:
    rc = draw_line(c, cx - s * 0.8f, cy, cx, cy - s * 0.7f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx, cy - s * 0.7f, cx + s * 0.8f, cy, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_rounded_rect(c, cx - s * 0.55f, cy - s * 0.05f, s * 1.1f,
                             s * 0.75f, 2.0f, color);

  case ICON_BELL:
    rc = draw_rounded_rect(c, cx - s * 0.55f, cy - s * 0.5f, s * 1.1f, s * 0.8f,
                           s * 0.4f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.75f, cy + s * 0.35f, cx + s * 0.75f,
                   cy + s * 0.35f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_circle(c, cx, cy + s * 0.55f, 2.0f, color);

  case ICON_INFO:
    rc = draw_circle_stroke(c, cx, cy, s * 0.8f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_circle(c, cx, cy - s * 0.4f, 1.8f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx, cy - s * 0.1f, cx, cy + s * 0.45f, stroke, color);

  case ICON_WARNING:
    rc = draw_line(c, cx, cy - s * 0.8f, cx - s * 0.8f, cy + s * 0.7f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx, cy - s * 0.8f, cx + s * 0.8f, cy + s * 0.7f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.8f, cy + s * 0.7f, cx + s * 0.8f,
                   cy + s * 0.7f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx, cy - s * 0.2f, cx, cy + s * 0.25f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_circle(c, cx, cy + s * 0.45f, 1.5f, color);

  case ICON_DELETE:
    rc = draw_line(c, cx - s * 0.6f, cy - s * 0.5f, cx + s * 0.6f,
                   cy - s * 0.5f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_rounded_rect_stroke(c, cx - s * 0.45f, cy - s * 0.4f, s * 0.9f,
                                  s * 1.1f, 2.0f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.2f, cy - s * 0.2f, cx - s * 0.2f,
                   cy + s * 0.5f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx + s * 0.2f, cy - s * 0.2f, cx + s * 0.2f,
                     cy + s * 0.5f, stroke, color);

  case ICON_SHARE:
    rc = draw_circle(c, cx - s * 0.5f, cy, s * 0.25f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_circle(c, cx + s * 0.5f, cy - s * 0.5f, s * 0.25f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_circle(c, cx + s * 0.5f, cy + s * 0.5f, s * 0.25f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.4f, cy, cx + s * 0.4f, cy - s * 0.45f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx - s * 0.4f, cy, cx + s * 0.4f, cy + s * 0.45f,
                     stroke, color);

  case ICON_CLOUD_UPLOAD:
    rc = draw_circle(c, cx - s * 0.35f, cy - s * 0.1f, s * 0.38f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_circle(c, cx + s * 0.3f, cy - s * 0.2f, s * 0.48f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_rounded_rect(c, cx - s * 0.7f, cy + s * 0.1f, s * 1.4f, s * 0.45f,
                           s * 0.2f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx, cy + s * 0.35f, cx, cy - s * 0.3f, stroke,
                   UI_COLOR_ARGB(255, 255, 255, 255));
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx, cy - s * 0.3f, cx - s * 0.3f, cy, stroke,
                   UI_COLOR_ARGB(255, 255, 255, 255));
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx, cy - s * 0.3f, cx + s * 0.3f, cy, stroke,
                     UI_COLOR_ARGB(255, 255, 255, 255));

  case ICON_PLAY:
    rc = draw_line(c, cx - s * 0.5f, cy - s * 0.6f, cx - s * 0.5f,
                   cy + s * 0.6f, stroke + 1.0f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.5f, cy - s * 0.6f, cx + s * 0.6f, cy,
                   stroke + 1.0f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx - s * 0.5f, cy + s * 0.6f, cx + s * 0.6f, cy,
                     stroke + 1.0f, color);

  case ICON_PAUSE:
    rc = draw_line(c, cx - s * 0.35f, cy - s * 0.6f, cx - s * 0.35f,
                   cy + s * 0.6f, stroke + 1.0f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx + s * 0.35f, cy - s * 0.6f, cx + s * 0.35f,
                     cy + s * 0.6f, stroke + 1.0f, color);

  case ICON_DRAG_HANDLE:
    return draw_rounded_rect(c, cx - s * 0.7f, cy - s * 0.15f, s * 1.4f,
                             s * 0.3f, s * 0.15f, color);

  case ICON_FOLDER:
    rc = draw_rounded_rect(c, cx - s * 0.8f, cy - s * 0.4f, s * 1.6f, s * 1.1f,
                           3.0f, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_rounded_rect(c, cx - s * 0.8f, cy - s * 0.7f, s * 0.8f,
                             s * 0.4f, 2.0f, color);

  case ICON_FILTER:
    rc = draw_line(c, cx - s * 0.7f, cy - s * 0.5f, cx + s * 0.7f,
                   cy - s * 0.5f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx - s * 0.4f, cy, cx + s * 0.4f, cy, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx - s * 0.15f, cy + s * 0.5f, cx + s * 0.15f,
                     cy + s * 0.5f, stroke, color);

  case ICON_REFRESH:
    rc = draw_circle_stroke(c, cx, cy, s * 0.65f, stroke, color);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = draw_line(c, cx + s * 0.65f, cy, cx + s * 0.95f, cy - s * 0.3f, stroke,
                   color);
    if (rc != UI_ERROR_NONE)
      return rc;
    return draw_line(c, cx + s * 0.65f, cy, cx + s * 0.35f, cy - s * 0.3f,
                     stroke, color);

  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }
}

ui_error_t save_canvas_png(const struct canvas *c, const char *filepath) {
  if (!c || !c->pixels || !filepath) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_visual_write_heatmap_to_disk(filepath, c->pixels, c->width,
                                         c->height);
}
