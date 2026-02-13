#ifndef CMP_API_UI_H
#define CMP_API_UI_H

/**
 * @file cmp_api_ui.h
 * @brief UI widget contract ABI for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ws.h"

/** @brief No measurement constraint. */
#define CMP_MEASURE_UNSPECIFIED 0
/** @brief Exact measurement constraint. */
#define CMP_MEASURE_EXACTLY 1
/** @brief Maximum measurement constraint. */
#define CMP_MEASURE_AT_MOST 2

/** @brief Widget can receive focus. */
#define CMP_WIDGET_FLAG_FOCUSABLE 0x01
/** @brief Widget is disabled. */
#define CMP_WIDGET_FLAG_DISABLED 0x02
/** @brief Widget is hidden. */
#define CMP_WIDGET_FLAG_HIDDEN 0x04

/** @brief No semantic role. */
#define CMP_SEMANTIC_NONE 0
/** @brief Button semantic role. */
#define CMP_SEMANTIC_BUTTON 1
/** @brief Text semantic role. */
#define CMP_SEMANTIC_TEXT 2
/** @brief Image semantic role. */
#define CMP_SEMANTIC_IMAGE 3
/** @brief Slider semantic role. */
#define CMP_SEMANTIC_SLIDER 4
/** @brief Checkbox semantic role. */
#define CMP_SEMANTIC_CHECKBOX 5
/** @brief Switch semantic role. */
#define CMP_SEMANTIC_SWITCH 6
/** @brief Radio button semantic role. */
#define CMP_SEMANTIC_RADIO 7
/** @brief Text field semantic role. */
#define CMP_SEMANTIC_TEXT_FIELD 8

/** @brief Semantics node is disabled. */
#define CMP_SEMANTIC_FLAG_DISABLED 0x01
/** @brief Semantics node is selected. */
#define CMP_SEMANTIC_FLAG_SELECTED 0x02
/** @brief Semantics node is focusable. */
#define CMP_SEMANTIC_FLAG_FOCUSABLE 0x04
/** @brief Semantics node is focused. */
#define CMP_SEMANTIC_FLAG_FOCUSED 0x08

/**
 * @brief Layout constraint for a single axis.
 */
typedef struct CMPMeasureSpec {
  cmp_u32 mode;   /**< Measurement mode (CMP_MEASURE_*). */
  CMPScalar size; /**< Size constraint in pixels. */
} CMPMeasureSpec;

/**
 * @brief Painting context passed to widgets.
 */
typedef struct CMPPaintContext {
  CMPGfx *gfx;         /**< Graphics interface. */
  CMPRect clip;        /**< Current clip rectangle. */
  CMPScalar dpi_scale; /**< DPI scale factor. */
} CMPPaintContext;

/**
 * @brief Accessibility semantics description.
 */
typedef struct CMPSemantics {
  cmp_u32 role;           /**< Semantic role (CMP_SEMANTIC_*). */
  cmp_u32 flags;          /**< Semantic flags (CMP_SEMANTIC_FLAG_*). */
  const char *utf8_label; /**< Primary label text in UTF-8. */
  const char *utf8_hint;  /**< Hint text in UTF-8. */
  const char *utf8_value; /**< Value text in UTF-8. */
} CMPSemantics;

/**
 * @brief Widget virtual table.
 */
typedef struct CMPWidgetVTable {
/**
 * @brief Measure the widget.
 * @param widget Widget instance.
 * @param width Width measurement constraint.
 * @param height Height measurement constraint.
 * @param out_size Receives the desired size.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int measure(void *widget, CMPMeasureSpec width, CMPMeasureSpec height,
              CMPSize *out_size);
#else
  int(CMP_CALL *measure)(void *widget, CMPMeasureSpec width,
                         CMPMeasureSpec height, CMPSize *out_size);
#endif
/**
 * @brief Layout the widget within bounds.
 * @param widget Widget instance.
 * @param bounds Assigned bounds in pixels.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int layout(void *widget, CMPRect bounds);
#else
  int(CMP_CALL *layout)(void *widget, CMPRect bounds);
#endif
/**
 * @brief Paint the widget contents.
 * @param widget Widget instance.
 * @param ctx Painting context.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int paint(void *widget, CMPPaintContext *ctx);
#else
  int(CMP_CALL *paint)(void *widget, CMPPaintContext *ctx);
#endif
/**
 * @brief Handle an input event.
 * @param widget Widget instance.
 * @param event Input event.
 * @param out_handled Receives CMP_TRUE if the event was handled.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int event(void *widget, const CMPInputEvent *event, CMPBool *out_handled);
#else
  int(CMP_CALL *event)(void *widget, const CMPInputEvent *event,
                       CMPBool *out_handled);
#endif
/**
 * @brief Retrieve accessibility semantics for the widget.
 * @param widget Widget instance.
 * @param out_semantics Receives semantics information.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int get_semantics(void *widget, CMPSemantics *out_semantics);
#else
  int(CMP_CALL *get_semantics)(void *widget, CMPSemantics *out_semantics);
#endif
/**
 * @brief Destroy the widget instance.
 * @param widget Widget instance.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int destroy(void *widget);
#else
  int(CMP_CALL *destroy)(void *widget);
#endif
} CMPWidgetVTable;

/**
 * @brief Widget interface.
 */
typedef struct CMPWidget {
  void *ctx;                     /**< Widget implementation context. */
  const CMPWidgetVTable *vtable; /**< Widget virtual table. */
  CMPHandle handle;              /**< Handle for the widget object. */
  cmp_u32 flags;                 /**< Widget flags (CMP_WIDGET_FLAG_*). */
} CMPWidget;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_API_UI_H */
