#ifndef M3_API_UI_H
#define M3_API_UI_H

/**
 * @file m3_api_ui.h
 * @brief UI widget contract ABI for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_gfx.h"
#include "m3_api_ws.h"

/** @brief No measurement constraint. */
#define M3_MEASURE_UNSPECIFIED 0
/** @brief Exact measurement constraint. */
#define M3_MEASURE_EXACTLY 1
/** @brief Maximum measurement constraint. */
#define M3_MEASURE_AT_MOST 2

/** @brief Widget can receive focus. */
#define M3_WIDGET_FLAG_FOCUSABLE 0x01
/** @brief Widget is disabled. */
#define M3_WIDGET_FLAG_DISABLED 0x02
/** @brief Widget is hidden. */
#define M3_WIDGET_FLAG_HIDDEN 0x04

/** @brief No semantic role. */
#define M3_SEMANTIC_NONE 0
/** @brief Button semantic role. */
#define M3_SEMANTIC_BUTTON 1
/** @brief Text semantic role. */
#define M3_SEMANTIC_TEXT 2
/** @brief Image semantic role. */
#define M3_SEMANTIC_IMAGE 3
/** @brief Slider semantic role. */
#define M3_SEMANTIC_SLIDER 4
/** @brief Checkbox semantic role. */
#define M3_SEMANTIC_CHECKBOX 5
/** @brief Switch semantic role. */
#define M3_SEMANTIC_SWITCH 6
/** @brief Radio button semantic role. */
#define M3_SEMANTIC_RADIO 7
/** @brief Text field semantic role. */
#define M3_SEMANTIC_TEXT_FIELD 8

/** @brief Semantics node is disabled. */
#define M3_SEMANTIC_FLAG_DISABLED 0x01
/** @brief Semantics node is selected. */
#define M3_SEMANTIC_FLAG_SELECTED 0x02
/** @brief Semantics node is focusable. */
#define M3_SEMANTIC_FLAG_FOCUSABLE 0x04
/** @brief Semantics node is focused. */
#define M3_SEMANTIC_FLAG_FOCUSED 0x08

/**
 * @brief Layout constraint for a single axis.
 */
typedef struct M3MeasureSpec {
    m3_u32 mode; /**< Measurement mode (M3_MEASURE_*). */
    M3Scalar size; /**< Size constraint in pixels. */
} M3MeasureSpec;

/**
 * @brief Painting context passed to widgets.
 */
typedef struct M3PaintContext {
    M3Gfx *gfx; /**< Graphics interface. */
    M3Rect clip; /**< Current clip rectangle. */
    M3Scalar dpi_scale; /**< DPI scale factor. */
} M3PaintContext;

/**
 * @brief Accessibility semantics description.
 */
typedef struct M3Semantics {
    m3_u32 role; /**< Semantic role (M3_SEMANTIC_*). */
    m3_u32 flags; /**< Semantic flags (M3_SEMANTIC_FLAG_*). */
    const char *utf8_label; /**< Primary label text in UTF-8. */
    const char *utf8_hint; /**< Hint text in UTF-8. */
    const char *utf8_value; /**< Value text in UTF-8. */
} M3Semantics;

/**
 * @brief Widget virtual table.
 */
typedef struct M3WidgetVTable {
    /**
     * @brief Measure the widget.
     * @param widget Widget instance.
     * @param width Width measurement constraint.
     * @param height Height measurement constraint.
     * @param out_size Receives the desired size.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *measure)(void *widget, M3MeasureSpec width, M3MeasureSpec height, M3Size *out_size);
    /**
     * @brief Layout the widget within bounds.
     * @param widget Widget instance.
     * @param bounds Assigned bounds in pixels.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *layout)(void *widget, M3Rect bounds);
    /**
     * @brief Paint the widget contents.
     * @param widget Widget instance.
     * @param ctx Painting context.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *paint)(void *widget, M3PaintContext *ctx);
    /**
     * @brief Handle an input event.
     * @param widget Widget instance.
     * @param event Input event.
     * @param out_handled Receives M3_TRUE if the event was handled.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *event)(void *widget, const M3InputEvent *event, M3Bool *out_handled);
    /**
     * @brief Retrieve accessibility semantics for the widget.
     * @param widget Widget instance.
     * @param out_semantics Receives semantics information.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *get_semantics)(void *widget, M3Semantics *out_semantics);
    /**
     * @brief Destroy the widget instance.
     * @param widget Widget instance.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *destroy)(void *widget);
} M3WidgetVTable;

/**
 * @brief Widget interface.
 */
typedef struct M3Widget {
    void *ctx; /**< Widget implementation context. */
    const M3WidgetVTable *vtable; /**< Widget virtual table. */
    M3Handle handle; /**< Handle for the widget object. */
    m3_u32 flags; /**< Widget flags (M3_WIDGET_FLAG_*). */
} M3Widget;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_API_UI_H */
