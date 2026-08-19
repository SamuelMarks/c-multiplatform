/**
 * @file ui_typography_scale.h
 * @brief Standardized typography scales and metrics.
 */

#ifndef UI_TYPOGRAPHY_SCALE_H
#define UI_TYPOGRAPHY_SCALE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ui_typography Typography Scale
 * @brief Standardized typography scales and metrics.
 * @{
 */

/**
 * @brief Standardized typography scale categories.
 */
enum ui_typography_scale {
  UI_TYPOGRAPHY_SCALE_DISPLAY_LARGE,   /**< Display Large */
  UI_TYPOGRAPHY_SCALE_DISPLAY_MEDIUM,  /**< Display Medium */
  UI_TYPOGRAPHY_SCALE_DISPLAY_SMALL,   /**< Display Small */
  UI_TYPOGRAPHY_SCALE_HEADLINE_LARGE,  /**< Headline Large */
  UI_TYPOGRAPHY_SCALE_HEADLINE_MEDIUM, /**< Headline Medium */
  UI_TYPOGRAPHY_SCALE_HEADLINE_SMALL,  /**< Headline Small */
  UI_TYPOGRAPHY_SCALE_TITLE_LARGE,     /**< Title Large */
  UI_TYPOGRAPHY_SCALE_TITLE_MEDIUM,    /**< Title Medium */
  UI_TYPOGRAPHY_SCALE_TITLE_SMALL,     /**< Title Small */
  UI_TYPOGRAPHY_SCALE_LABEL_LARGE,     /**< Label Large */
  UI_TYPOGRAPHY_SCALE_LABEL_MEDIUM,    /**< Label Medium */
  UI_TYPOGRAPHY_SCALE_LABEL_SMALL,     /**< Label Small */
  UI_TYPOGRAPHY_SCALE_BODY_LARGE,      /**< Body Large */
  UI_TYPOGRAPHY_SCALE_BODY_MEDIUM,     /**< Body Medium */
  UI_TYPOGRAPHY_SCALE_BODY_SMALL       /**< Body Small */
};

/**
 * @brief Represents the metrics for a specific typography scale.
 */
struct ui_typography_metrics {
  float font_size;      /**< Font size in logical pixels */
  float line_height;    /**< Line height in logical pixels */
  float letter_spacing; /**< Tracking/Letter spacing in logical pixels */
  float
      font_weight; /**< Font weight (e.g., 400.0 for regular, 700.0 for bold) */
};

/**
 * @brief Retrieves the standard metrics for a given typography scale.
 *
 * @param scale The typography scale category.
 * @param out_metrics Pointer to store the resulting metrics.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t
ui_typography_scale_get_metrics(enum ui_typography_scale scale,
                                struct ui_typography_metrics *out_metrics);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TYPOGRAPHY_SCALE_H */
