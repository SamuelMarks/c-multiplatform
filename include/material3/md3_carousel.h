/**
 * @file md3_carousel.h
 * @brief Material 3 Expressive Carousel component wrapping ui_carousel_base.
 */

#ifndef MATERIAL3_MD3_CAROUSEL_H
#define MATERIAL3_MD3_CAROUSEL_H

/* clang-format off */
#include "ui_carousel_base.h"
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_carousel_layout_type
 * @brief Material 3 Carousel layout strategy variants.
 */
enum md3_carousel_layout_type {
  MD3_CAROUSEL_LAYOUT_MULTI_BROWSE =
      0,                           /**< Large, medium, and small item preview */
  MD3_CAROUSEL_LAYOUT_UNCONTAINED, /**< Free scroll flick with no end bounding
                                    */
  MD3_CAROUSEL_LAYOUT_HERO,        /**< Prominent large featured banner */
  MD3_CAROUSEL_LAYOUT_FULL_SCREEN, /**< Full viewport pager */
  MD3_CAROUSEL_LAYOUT_COUNT
};

/**
 * @struct md3_carousel
 * @brief Material 3 Carousel skin wrapping ui_carousel_base.
 */
struct md3_carousel {
  struct ui_carousel_base *base;
  enum md3_carousel_layout_type layout_type;
  float corner_radius;
  int dynamic_mask_morphing;
};

/**
 * @brief Creates a Material 3 Carousel component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param layout_type Carousel layout strategy (Multi-browse, Uncontained,
 * Hero).
 * @param config Pointer to base carousel configuration.
 * @param out_carousel Pointer to receive newly created carousel.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_carousel_create(
    struct ui_engine *engine, enum md3_carousel_layout_type layout_type,
    const struct ui_carousel_config *config,
    struct md3_carousel **out_carousel);

/**
 * @brief Destroys a Material 3 Carousel component.
 *
 * @param carousel Carousel to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_carousel_destroy(struct md3_carousel *carousel);

/**
 * @brief Enables or disables dynamic mask morphing on boundary edges.
 *
 * @param carousel The carousel.
 * @param enable Non-zero to enable dynamic mask morphing.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_carousel_set_mask_morphing(struct md3_carousel *carousel, int enable);

/**
 * @brief Sets the corner radius for carousel items.
 *
 * @param carousel The carousel.
 * @param radius Radius in dp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_carousel_set_corner_radius(struct md3_carousel *carousel, float radius);

/**
 * @brief Scrolls the carousel to the specified item index.
 *
 * @param carousel The carousel.
 * @param index Target item index.
 * @param smooth Non-zero to animate with spring physics, 0 for immediate jump.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_carousel_scroll_to(struct md3_carousel *carousel, size_t index, int smooth);

/**
 * @brief Gets the underlying component of the carousel.
 *
 * @param carousel The carousel.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_carousel_get_component(
    struct md3_carousel *carousel, struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_CAROUSEL_H */
