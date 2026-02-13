#ifndef CMP_LAYOUT_H
#define CMP_LAYOUT_H

/**
 * @file cmp_layout.h
 * @brief Flex-style layout solver for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_math.h"

/** @brief Sentinel value for automatic sizing. */
#define CMP_LAYOUT_AUTO (-1.0f)

/** @brief Measurement constraint: no restriction. */
#define CMP_LAYOUT_MEASURE_UNSPECIFIED 0
/** @brief Measurement constraint: exact size. */
#define CMP_LAYOUT_MEASURE_EXACTLY 1
/** @brief Measurement constraint: maximum size. */
#define CMP_LAYOUT_MEASURE_AT_MOST 2

/** @brief Flow direction: left-to-right. */
#define CMP_DIRECTION_LTR 0
/** @brief Flow direction: right-to-left. */
#define CMP_DIRECTION_RTL 1

/** @brief Layout direction: horizontal row. */
#define CMP_LAYOUT_DIRECTION_ROW 0
/** @brief Layout direction: top-to-bottom column. */
#define CMP_LAYOUT_DIRECTION_COLUMN 1

/** @brief No wrapping. */
#define CMP_LAYOUT_WRAP_NO 0
/** @brief Wrap items onto additional lines. */
#define CMP_LAYOUT_WRAP_YES 1

/** @brief Alignment at the start edge. */
#define CMP_LAYOUT_ALIGN_START 0
/** @brief Alignment centered. */
#define CMP_LAYOUT_ALIGN_CENTER 1
/** @brief Alignment at the end edge. */
#define CMP_LAYOUT_ALIGN_END 2
/** @brief Space between items. */
#define CMP_LAYOUT_ALIGN_SPACE_BETWEEN 3
/** @brief Space around items. */
#define CMP_LAYOUT_ALIGN_SPACE_AROUND 4
/** @brief Space evenly between items and edges. */
#define CMP_LAYOUT_ALIGN_SPACE_EVENLY 5
/** @brief Stretch items along the cross axis. */
#define CMP_LAYOUT_ALIGN_STRETCH 6

/**
 * @brief Measurement constraint descriptor.
 */
typedef struct CMPLayoutMeasureSpec {
  cmp_u32 mode;   /**< Measurement mode (CMP_LAYOUT_MEASURE_*). */
  CMPScalar size; /**< Size constraint in pixels. */
} CMPLayoutMeasureSpec;

/**
 * @brief Edge sizes used for padding.
 */
typedef struct CMPLayoutEdges {
  CMPScalar left;   /**< Left edge. */
  CMPScalar top;    /**< Top edge. */
  CMPScalar right;  /**< Right edge. */
  CMPScalar bottom; /**< Bottom edge. */
} CMPLayoutEdges;

/**
 * @brief Direction context for resolving logical start/end edges.
 */
typedef struct CMPLayoutDirection {
  cmp_u32 flow; /**< Flow direction (CMP_DIRECTION_*). */
} CMPLayoutDirection;

/**
 * @brief Layout style attributes.
 */
typedef struct CMPLayoutStyle {
  cmp_u32 direction;      /**< Layout direction (CMP_LAYOUT_DIRECTION_*). */
  cmp_u32 wrap;           /**< Wrap mode (CMP_LAYOUT_WRAP_*). */
  cmp_u32 align_main;     /**< Main-axis alignment (CMP_LAYOUT_ALIGN_*). */
  cmp_u32 align_cross;    /**< Cross-axis alignment (CMP_LAYOUT_ALIGN_*). */
  CMPLayoutEdges padding; /**< Padding around children (left/right map to
                               start/end based on layout direction). */
  CMPScalar width;        /**< Explicit width or CMP_LAYOUT_AUTO. */
  CMPScalar height;       /**< Explicit height or CMP_LAYOUT_AUTO. */
} CMPLayoutStyle;

/**
 * @brief Measure callback for leaf nodes.
 * @param ctx Measure callback context.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPLayoutMeasureFn)(void *ctx, CMPLayoutMeasureSpec width,
                                        CMPLayoutMeasureSpec height,
                                        CMPSize *out_size);

/**
 * @brief Layout node definition.
 */
typedef struct CMPLayoutNode {
  CMPLayoutStyle style;            /**< Style attributes. */
  struct CMPLayoutNode **children; /**< Child node list. */
  cmp_usize child_count;           /**< Number of child nodes. */
  CMPLayoutMeasureFn measure;      /**< Measure callback for leaf nodes. */
  void *measure_ctx;              /**< Measure callback context. */
  CMPSize measured;                /**< Measured size before layout. */
  CMPRect layout;                  /**< Final layout rectangle. */
} CMPLayoutNode;

/**
 * @brief Initialize a measure spec.
 * @param spec Spec to initialize.
 * @param mode Measurement mode (CMP_LAYOUT_MEASURE_*).
 * @param size Size constraint.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_measure_spec_init(CMPLayoutMeasureSpec *spec,
                                               cmp_u32 mode, CMPScalar size);

/**
 * @brief Initialize padding edges.
 * @param edges Edge descriptor to initialize.
 * @param left Left padding.
 * @param top Top padding.
 * @param right Right padding.
 * @param bottom Bottom padding.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_edges_set(CMPLayoutEdges *edges, CMPScalar left,
                                       CMPScalar top, CMPScalar right,
                                       CMPScalar bottom);

/**
 * @brief Initialize a layout direction context.
 * @param direction Direction context to initialize.
 * @param flow Flow direction (CMP_DIRECTION_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_direction_init(CMPLayoutDirection *direction,
                                            cmp_u32 flow);

/**
 * @brief Initialize a layout style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_style_init(CMPLayoutStyle *style);

/**
 * @brief Initialize a layout node.
 * @param node Node to initialize.
 * @param style Style to copy; NULL uses default style.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_node_init(CMPLayoutNode *node,
                                       const CMPLayoutStyle *style);

/**
 * @brief Assign children to a node.
 * @param node Node to update.
 * @param children Child node array; may be NULL when count is 0.
 * @param count Number of children.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_node_set_children(CMPLayoutNode *node,
                                               CMPLayoutNode **children,
                                               cmp_usize count);

/**
 * @brief Assign a measure callback to a node.
 * @param node Node to update.
 * @param measure Measure callback (NULL clears).
 * @param ctx Measure callback context.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_node_set_measure(CMPLayoutNode *node,
                                              CMPLayoutMeasureFn measure,
                                              void *ctx);

/**
 * @brief Retrieve a node's measured size.
 * @param node Node to query.
 * @param out_size Receives the measured size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_node_get_measured(const CMPLayoutNode *node,
                                               CMPSize *out_size);

/**
 * @brief Retrieve a node's final layout rectangle.
 * @param node Node to query.
 * @param out_rect Receives the layout rectangle.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_node_get_layout(const CMPLayoutNode *node,
                                             CMPRect *out_rect);

/**
 * @brief Compute layout for a node tree.
 * @param root Root node.
 * @param direction Layout direction context (CMP_DIRECTION_*).
 * @param width Width constraint.
 * @param height Height constraint.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_compute(CMPLayoutNode *root,
                                     const CMPLayoutDirection *direction,
                                     CMPLayoutMeasureSpec width,
                                     CMPLayoutMeasureSpec height);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for validating measure specs.
 * @param spec Measure spec to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_layout_test_validate_measure_spec(const CMPLayoutMeasureSpec *spec);

/**
 * @brief Test wrapper for validating direction context.
 * @param direction Direction context to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_layout_test_validate_direction(const CMPLayoutDirection *direction);

/**
 * @brief Test wrapper for clamping values to non-negative range.
 * @param value Input value.
 * @param out_value Receives the clamped value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_clamp_non_negative(CMPScalar value,
                                                     CMPScalar *out_value);

/**
 * @brief Test wrapper for resolving available size.
 * @param style_size Style-provided size (or CMP_LAYOUT_AUTO).
 * @param spec Measure spec.
 * @param out_available Receives the resolved available size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_resolve_available(CMPScalar style_size,
                                                    CMPLayoutMeasureSpec spec,
                                                    CMPScalar *out_available);

/**
 * @brief Test wrapper for resolving horizontal padding.
 * @param direction Direction context.
 * @param padding Padding edges.
 * @param out_left Receives resolved left padding.
 * @param out_right Receives resolved right padding.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_resolve_horizontal_padding(
    const CMPLayoutDirection *direction, const CMPLayoutEdges *padding,
    CMPScalar *out_left, CMPScalar *out_right);

/**
 * @brief Test wrapper for validating styles.
 * @param style Style to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_validate_style(const CMPLayoutStyle *style);

/**
 * @brief Test wrapper for applying measure specs.
 * @param size Input size.
 * @param spec Measure spec.
 * @param out_size Receives the clamped size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_apply_spec(CMPScalar size,
                                             CMPLayoutMeasureSpec spec,
                                             CMPScalar *out_size);

/**
 * @brief Test wrapper for measuring leaf nodes.
 * @param node Node to measure.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_measure_leaf(CMPLayoutNode *node,
                                               CMPLayoutMeasureSpec width,
                                               CMPLayoutMeasureSpec height,
                                               CMPSize *out_size);

/**
 * @brief Test wrapper for measuring row containers.
 * @param node Node to measure.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_measure_row(CMPLayoutNode *node,
                                              CMPLayoutMeasureSpec width,
                                              CMPLayoutMeasureSpec height,
                                              CMPSize *out_size);

/**
 * @brief Test wrapper for measuring column containers.
 * @param node Node to measure.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_measure_column(CMPLayoutNode *node,
                                                 CMPLayoutMeasureSpec width,
                                                 CMPLayoutMeasureSpec height,
                                                 CMPSize *out_size);

/**
 * @brief Test wrapper for measuring row containers with a custom direction.
 * @param node Node to measure.
 * @param direction Direction context.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_measure_row_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction,
    CMPLayoutMeasureSpec width, CMPLayoutMeasureSpec height, CMPSize *out_size);

/**
 * @brief Test wrapper for measuring column containers with a custom direction.
 * @param node Node to measure.
 * @param direction Direction context.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_measure_column_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction,
    CMPLayoutMeasureSpec width, CMPLayoutMeasureSpec height, CMPSize *out_size);

/**
 * @brief Test wrapper for measuring nodes.
 * @param node Node to measure.
 * @param width Width constraint.
 * @param height Height constraint.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_measure_node(CMPLayoutNode *node,
                                               CMPLayoutMeasureSpec width,
                                               CMPLayoutMeasureSpec height);

/**
 * @brief Test wrapper for laying out nodes.
 * @param node Node to layout.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_layout_node(CMPLayoutNode *node, CMPScalar x,
                                              CMPScalar y, CMPScalar width,
                                              CMPScalar height);

/**
 * @brief Test wrapper for laying out row children.
 * @param node Node to layout.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_layout_children_row(CMPLayoutNode *node,
                                                      CMPScalar x, CMPScalar y,
                                                      CMPScalar width,
                                                      CMPScalar height);

/**
 * @brief Test wrapper for laying out column children.
 * @param node Node to layout.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_layout_children_column(CMPLayoutNode *node,
                                                         CMPScalar x, CMPScalar y,
                                                         CMPScalar width,
                                                         CMPScalar height);

/**
 * @brief Test wrapper for measuring nodes with a custom direction context.
 * @param node Node to measure.
 * @param direction Direction context.
 * @param width Width constraint.
 * @param height Height constraint.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_measure_node_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction,
    CMPLayoutMeasureSpec width, CMPLayoutMeasureSpec height);

/**
 * @brief Test wrapper for laying out nodes with a custom direction context.
 * @param node Node to layout.
 * @param direction Direction context.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_layout_node_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction, CMPScalar x,
    CMPScalar y, CMPScalar width, CMPScalar height);

/**
 * @brief Test wrapper for laying out row children with a custom direction.
 * @param node Node to layout.
 * @param direction Direction context.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_layout_children_row_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction, CMPScalar x,
    CMPScalar y, CMPScalar width, CMPScalar height);

/**
 * @brief Test wrapper for laying out column children with a custom direction.
 * @param node Node to layout.
 * @param direction Direction context.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_layout_children_column_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction, CMPScalar x,
    CMPScalar y, CMPScalar width, CMPScalar height);

/**
 * @brief Test hook to force style init failure.
 * @param enable Whether to force failure.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_set_style_init_fail(CMPBool enable);

/**
 * @brief Test hook to force direction validation failure.
 * @param enable Whether to force failure.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_layout_test_set_direction_fail(CMPBool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_LAYOUT_H */
