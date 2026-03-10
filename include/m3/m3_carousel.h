#ifndef M3_CAROUSEL_H
#define M3_CAROUSEL_H

/**
 * @file m3_carousel.h
 * @brief Material 3 Carousel widgets.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_scroll.h"
/* clang-format on */

/**
 * @brief Material 3 Carousel style descriptor.
 */
/**
 * @brief Carousel variant type.
 */
typedef enum M3CarouselVariant {
  M3_CAROUSEL_VARIANT_STANDARD = 0,
  M3_CAROUSEL_VARIANT_HERO = 1,
  M3_CAROUSEL_VARIANT_MULTI_BROWSE = 2
} M3CarouselVariant;

/** @brief Carousel style configuration. */
typedef struct M3CarouselStyle {
  CMPScalar item_spacing;    /**< Spacing between items. */
  M3CarouselVariant variant; /**< The variant of the carousel. */
  CMPScalar item_width;      /**< Width of items (ignored if <= 0). */
  CMPScalar hero_width;      /**< Hero item width for hero variant. */
  CMPScalar min_item_width;  /**< Minimum item width for multi-browse. */
  CMPScalar snap_velocity;   /**< Velocity threshold for snapping. */
  CMPColor background_color; /**< Background color. */
} M3CarouselStyle;

struct M3Carousel;

/**
 * @brief Carousel selection callback.
 * @param ctx User context.
 * @param carousel The carousel instance.
 * @param index The selected index.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *M3CarouselOnSelect)(void *ctx,
                                          struct M3Carousel *carousel,
                                          cmp_usize index);

/**
 * @brief Material 3 Carousel widget.
 */
typedef struct M3Carousel {
  CMPWidget widget;             /**< Widget interface. */
  M3CarouselStyle style;        /**< Carousel styling. */
  CMPWidget **items;            /**< Array of child item widgets. */
  cmp_usize item_count;         /**< Number of items. */
  cmp_usize capacity;           /**< Capacity of items array. */
  CMPScalar scroll_offset;      /**< Current scroll offset. */
  CMPScalar content_extent;     /**< Total content width. */
  CMPBool dragging;             /**< Whether the user is dragging. */
  cmp_i32 last_pointer_x;       /**< Last pointer X position. */
  cmp_u32 last_time_ms;         /**< Last time in milliseconds. */
  CMPScalar velocity;           /**< Scroll physics state. */
  CMPRect bounds;               /**< Layout bounds. */
  cmp_usize selected_index;     /**< Currently selected or snapped index. */
  M3CarouselOnSelect on_select; /**< Selection callback. */
  void *on_select_ctx;          /**< Callback context. */
} M3Carousel;

/**
 * @brief Initialize a carousel style with defaults.
 * @param style Style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_carousel_style_init(M3CarouselStyle *style);
/**
 * @brief Initialize a hero carousel style.
 * @param style The style to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_carousel_style_init_hero(M3CarouselStyle *style);

/**
 * @brief Initialize a multi-browse carousel style.
 * @param style The style to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_carousel_style_init_multi_browse(M3CarouselStyle *style);

/**
 * @brief Initialize a Material 3 carousel.
 * @param carousel Carousel instance.
 * @param style Style descriptor.
 * @param items Initial items array (may be NULL).
 * @param item_count Number of initial items.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_carousel_init(M3Carousel *carousel,
                                      const M3CarouselStyle *style,
                                      CMPWidget **items, cmp_usize item_count);

/**
 * @brief Add an item to the carousel.
 * @param carousel Carousel instance.
 * @param item Widget item to add.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_carousel_add_item(M3Carousel *carousel,
                                          CMPWidget *item);

/**
 * @brief Set the selection callback.
 * @param carousel Carousel instance.
 * @param on_select Callback function.
 * @param ctx Callback context.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_carousel_set_on_select(M3Carousel *carousel,
                                               M3CarouselOnSelect on_select,
                                               void *ctx);

/**
 * @return Test result.\n * @brief Test wrapper helper.
 */
CMP_API int CMP_CALL m3_carousel_test_helper(void);

#ifdef __cplusplus
}
#endif

#endif /* M3_CAROUSEL_H */
