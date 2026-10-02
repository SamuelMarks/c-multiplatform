/**
 * @file cupertino_collection_view.h
 * @brief Cupertino Collection View & Compositional Layout (Item, Group,
 * Section, Layout).
 */

#ifndef CUPERTINO_CUPERTINO_COLLECTION_VIEW_H
#define CUPERTINO_CUPERTINO_COLLECTION_VIEW_H

/* clang-format off */
#include "ui_grid_list_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_COLLECTION_VIEW_MAX_SECTIONS 16
#define CUPERTINO_COLLECTION_VIEW_MAX_ITEMS_PER_GROUP 16
#define CUPERTINO_COLLECTION_VIEW_MAX_GROUPS_PER_SECTION 16

/**
 * @enum cupertino_collection_size_dimension
 * @brief Dimension measurement modes for compositional layout dimensions.
 */
enum cupertino_collection_size_dimension {
  CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_WIDTH =
      0, /**< Fraction of parent width (0.0 - 1.0). */
  CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_HEIGHT, /**< Fraction of parent
                                                       height (0.0 - 1.0). */
  CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE,          /**< Absolute point size. */
  CUPERTINO_COLLECTION_DIMENSION_ESTIMATED /**< Estimated point size with
                                              self-sizing. */
};

/**
 * @struct cupertino_collection_dimension
 * @brief Dimensional specification for width or height.
 */
struct cupertino_collection_dimension {
  enum cupertino_collection_size_dimension type; /**< Sizing mode. */
  float value; /**< Numeric value (fraction or points). */
};

/**
 * @struct cupertino_collection_size
 * @brief 2D size specification for compositional items and groups.
 */
struct cupertino_collection_size {
  struct cupertino_collection_dimension width;  /**< Width dimension. */
  struct cupertino_collection_dimension height; /**< Height dimension. */
};

/**
 * @struct cupertino_collection_insets
 * @brief Insets/margins around items or sections.
 */
struct cupertino_collection_insets {
  float top;      /**< Top inset in pt. */
  float leading;  /**< Leading inset in pt. */
  float bottom;   /**< Bottom inset in pt. */
  float trailing; /**< Trailing inset in pt. */
};

/**
 * @struct cupertino_collection_item
 * @brief Individual leaf element within a compositional layout group.
 */
struct cupertino_collection_item {
  struct cupertino_collection_size layout_size; /**< Layout size rule. */
  struct cupertino_collection_insets
      content_insets; /**< Inner content insets. */
  int item_index;     /**< Index identifier. */
  void *user_data;    /**< Associated user model data. */
};

/**
 * @enum cupertino_collection_group_direction
 * @brief Arranging direction within a group.
 */
enum cupertino_collection_group_direction {
  CUPERTINO_COLLECTION_GROUP_HORIZONTAL = 0, /**< Arrange items horizontally. */
  CUPERTINO_COLLECTION_GROUP_VERTICAL        /**< Arrange items vertically. */
};

/**
 * @struct cupertino_collection_group
 * @brief Group containing items arranged horizontally or vertically.
 */
struct cupertino_collection_group {
  struct cupertino_collection_size layout_size; /**< Group dimensions. */
  enum cupertino_collection_group_direction
      direction;     /**< Item arrangement direction. */
  size_t item_count; /**< Number of items in group. */
  struct cupertino_collection_item
      items[CUPERTINO_COLLECTION_VIEW_MAX_ITEMS_PER_GROUP]; /**< Items. */
  float inter_item_spacing; /**< Spacing between adjacent items. */
};

/**
 * @enum cupertino_collection_orthogonal_scroll_behavior
 * @brief Orthogonal scrolling mode for horizontally scrolling sections inside
 * vertical lists.
 */
enum cupertino_collection_orthogonal_scroll_behavior {
  CUPERTINO_COLLECTION_ORTHOGONAL_NONE = 0,   /**< No orthogonal scrolling. */
  CUPERTINO_COLLECTION_ORTHOGONAL_CONTINUOUS, /**< Standard continuous carousel
                                                 scroll. */
  CUPERTINO_COLLECTION_ORTHOGONAL_PAGING,     /**< Snapping page by page. */
  CUPERTINO_COLLECTION_ORTHOGONAL_GROUP_PAGING,         /**< Snapping to group
                                                           boundaries. */
  CUPERTINO_COLLECTION_ORTHOGONAL_GROUP_PAGING_CENTERED /**< Snapping to center
                                                           of group. */
};

/**
 * @struct cupertino_collection_section
 * @brief Section containing groups and orthogonal scrolling parameters.
 */
struct cupertino_collection_section {
  size_t group_count; /**< Number of groups. */
  struct cupertino_collection_group
      groups[CUPERTINO_COLLECTION_VIEW_MAX_GROUPS_PER_SECTION]; /**< Groups. */
  struct cupertino_collection_insets
      content_insets;        /**< Section margin insets. */
  float inter_group_spacing; /**< Spacing between groups in pt. */
  enum cupertino_collection_orthogonal_scroll_behavior
      orthogonal_behavior;        /**< Horizontal scrolling behavior. */
  float orthogonal_scroll_offset; /**< Current horizontal scroll position. */
  char header_title[64];          /**< Optional section header title. */
};

/**
 * @struct cupertino_collection_layout
 * @brief Compositional layout definition with adaptive breakpoint
 * configuration.
 */
struct cupertino_collection_layout {
  size_t section_count; /**< Number of sections in layout. */
  struct cupertino_collection_section
      sections[CUPERTINO_COLLECTION_VIEW_MAX_SECTIONS]; /**< Sections. */
  int compact_column_count; /**< Columns for compact widths (<600pt). */
  int regular_column_count; /**< Columns for regular widths (600-1024pt). */
  int wide_column_count;    /**< Columns for wide widths (>1024pt). */
  int active_column_count;  /**< Evaluated column count based on viewport. */
};

/**
 * @struct cupertino_collection_view_descriptor
 * @brief Initial configuration descriptor for collection view.
 */
struct cupertino_collection_view_descriptor {
  float initial_width;  /**< Viewport width in pt. */
  float initial_height; /**< Viewport height in pt. */
  int is_rtl;           /**< RTL layout flag (1 for RTL). */
};

/**
 * @struct cupertino_collection_view
 * @brief Cupertino Collection View instance wrapping ui_grid_list_base.
 */
struct cupertino_collection_view {
  struct ui_grid_list_base *base; /**< Underlying CDK grid list primitive. */
  struct cupertino_collection_layout layout; /**< Compositional layout state. */
  float viewport_width;                      /**< Viewport width in pt. */
  float viewport_height;                     /**< Viewport height in pt. */
  int is_rtl;                                /**< RTL direction. */
};

/**
 * @brief Creates a new Cupertino Collection View.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Creation descriptor.
 * @param out_view Pointer to receive collection view.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_collection_view_create(
    struct ui_engine *engine,
    const struct cupertino_collection_view_descriptor *desc,
    struct cupertino_collection_view **out_view);

/**
 * @brief Destroys a Cupertino Collection View instance.
 *
 * @param view Collection view to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_collection_view_destroy(struct cupertino_collection_view *view);

/**
 * @brief Configures responsive column breakpoints for the layout.
 *
 * @param view Collection view instance.
 * @param compact Columns when width < 600pt (must be >= 1).
 * @param regular Columns when width between 600-1024pt (must be >= 1).
 * @param wide Columns when width > 1024pt (must be >= 1).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_collection_view_set_breakpoints(
    struct cupertino_collection_view *view, int compact, int regular, int wide);

/**
 * @brief Updates viewport dimensions and triggers adaptive column count
 * recalculation.
 *
 * @param view Collection view instance.
 * @param width Viewport width in pt.
 * @param height Viewport height in pt.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_collection_view_update_viewport(
    struct cupertino_collection_view *view, float width, float height);

/**
 * @brief Adds a compositional section to the layout.
 *
 * @param view Collection view instance.
 * @param section Section configuration.
 * @param out_section_index Pointer to receive assigned section index.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if full, or
 * UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_collection_view_add_section(
    struct cupertino_collection_view *view,
    const struct cupertino_collection_section *section,
    size_t *out_section_index);

/**
 * @brief Appends a group of items to a section.
 *
 * @param view Collection view instance.
 * @param section_index Section index.
 * @param group Group configuration.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS on invalid
 * index/capacity, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_collection_view_add_group(
    struct cupertino_collection_view *view, size_t section_index,
    const struct cupertino_collection_group *group);

/**
 * @brief Updates horizontal scroll offset for an orthogonal scrolling section.
 *
 * @param view Collection view instance.
 * @param section_index Section index.
 * @param delta_x Horizontal pan offset delta in pt.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if section invalid,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_collection_view_scroll_orthogonal(
    struct cupertino_collection_view *view, size_t section_index,
    float delta_x);

/**
 * @brief Computes effective pixel/point bounds of an item within the
 * compositional layout.
 *
 * @param view Collection view instance.
 * @param section_index Section index.
 * @param group_index Group index within section.
 * @param item_index Item index within group.
 * @param out_x Pointer to receive X coordinate in pt.
 * @param out_y Pointer to receive Y coordinate in pt.
 * @param out_width Pointer to receive width in pt.
 * @param out_height Pointer to receive height in pt.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if indices invalid,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_collection_view_get_item_layout(
    const struct cupertino_collection_view *view, size_t section_index,
    size_t group_index, size_t item_index, float *out_x, float *out_y,
    float *out_width, float *out_height);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_CUPERTINO_COLLECTION_VIEW_H */
