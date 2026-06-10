/* clang-format off */
#ifndef CMP_CSS_DEVICE_ADAPTATION_H
#define CMP_CSS_DEVICE_ADAPTATION_H

#include "cmp_css_box_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents viewport width properties.
 */
typedef struct cmp_viewport_width {
  cmp_prop_size_t min_width;
  cmp_prop_size_t max_width;
  cmp_prop_size_t width;
} cmp_viewport_width_t;

/**
 * @brief Represents viewport height properties.
 */
typedef struct cmp_viewport_height {
  cmp_prop_size_t min_height;
  cmp_prop_size_t max_height;
  cmp_prop_size_t height;
} cmp_viewport_height_t;

/**
 * @brief Represents viewport user-zoom values.
 */
typedef enum cmp_viewport_user_zoom {
  CMP_VIEWPORT_USER_ZOOM_ZOOM = 0,
  CMP_VIEWPORT_USER_ZOOM_FIXED
} cmp_viewport_user_zoom_t;

/**
 * @brief Represents viewport zoom properties.
 */
typedef struct cmp_viewport_zoom {
  float zoom;         /**< Current zoom factor (e.g. 1.0 = 100%) */
  float min_zoom;     /**< Minimum zoom factor */
  float max_zoom;     /**< Maximum zoom factor */
  cmp_viewport_user_zoom_t user_zoom; /**< user-zoom property */
} cmp_viewport_zoom_t;

/**
 * @brief Represents viewport orientation property.
 */
typedef enum cmp_viewport_orientation {
  CMP_VIEWPORT_ORIENTATION_AUTO = 0,
  CMP_VIEWPORT_ORIENTATION_PORTRAIT,
  CMP_VIEWPORT_ORIENTATION_LANDSCAPE
} cmp_viewport_orientation_t;

/**
 * @brief Initializes viewport width properties to auto.
 * @param vw Viewport width object.
 * @return 0 on success.
 */
int cmp_viewport_width_init(cmp_viewport_width_t *vw);

/**
 * @brief Initializes viewport height properties to auto.
 * @param vh Viewport height object.
 * @return 0 on success.
 */
int cmp_viewport_height_init(cmp_viewport_height_t *vh);

/**
 * @brief Initializes viewport zoom properties to default values.
 * @param zoom Viewport zoom object.
 * @return 0 on success.
 */
int cmp_viewport_zoom_init(cmp_viewport_zoom_t *zoom);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_DEVICE_ADAPTATION_H */
/* clang-format on */
