#ifndef M3_LAYOUT_H
#define M3_LAYOUT_H

/**
 * @file m3_layout.h
 * @brief Flex-style layout solver for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_math.h"

/** @brief Sentinel value for automatic sizing. */
#define M3_LAYOUT_AUTO (-1.0f)

/** @brief Measurement constraint: no restriction. */
#define M3_LAYOUT_MEASURE_UNSPECIFIED 0
/** @brief Measurement constraint: exact size. */
#define M3_LAYOUT_MEASURE_EXACTLY 1
/** @brief Measurement constraint: maximum size. */
#define M3_LAYOUT_MEASURE_AT_MOST 2

/** @brief Layout direction: left-to-right row. */
#define M3_LAYOUT_DIRECTION_ROW 0
/** @brief Layout direction: top-to-bottom column. */
#define M3_LAYOUT_DIRECTION_COLUMN 1

/** @brief No wrapping. */
#define M3_LAYOUT_WRAP_NO 0
/** @brief Wrap items onto additional lines. */
#define M3_LAYOUT_WRAP_YES 1

/** @brief Alignment at the start edge. */
#define M3_LAYOUT_ALIGN_START 0
/** @brief Alignment centered. */
#define M3_LAYOUT_ALIGN_CENTER 1
/** @brief Alignment at the end edge. */
#define M3_LAYOUT_ALIGN_END 2
/** @brief Space between items. */
#define M3_LAYOUT_ALIGN_SPACE_BETWEEN 3
/** @brief Space around items. */
#define M3_LAYOUT_ALIGN_SPACE_AROUND 4
/** @brief Space evenly between items and edges. */
#define M3_LAYOUT_ALIGN_SPACE_EVENLY 5
/** @brief Stretch items along the cross axis. */
#define M3_LAYOUT_ALIGN_STRETCH 6

/**
 * @brief Measurement constraint descriptor.
 */
typedef struct M3LayoutMeasureSpec {
    m3_u32 mode; /**< Measurement mode (M3_LAYOUT_MEASURE_*). */
    M3Scalar size; /**< Size constraint in pixels. */
} M3LayoutMeasureSpec;

/**
 * @brief Edge sizes used for padding.
 */
typedef struct M3LayoutEdges {
    M3Scalar left; /**< Left edge. */
    M3Scalar top; /**< Top edge. */
    M3Scalar right; /**< Right edge. */
    M3Scalar bottom; /**< Bottom edge. */
} M3LayoutEdges;

/**
 * @brief Layout style attributes.
 */
typedef struct M3LayoutStyle {
    m3_u32 direction; /**< Layout direction (M3_LAYOUT_DIRECTION_*). */
    m3_u32 wrap; /**< Wrap mode (M3_LAYOUT_WRAP_*). */
    m3_u32 align_main; /**< Main-axis alignment (M3_LAYOUT_ALIGN_*). */
    m3_u32 align_cross; /**< Cross-axis alignment (M3_LAYOUT_ALIGN_*). */
    M3LayoutEdges padding; /**< Padding around children. */
    M3Scalar width; /**< Explicit width or M3_LAYOUT_AUTO. */
    M3Scalar height; /**< Explicit height or M3_LAYOUT_AUTO. */
} M3LayoutStyle;

/**
 * @brief Measure callback for leaf nodes.
 * @param ctx Measure callback context.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return M3_OK on success or a failure code.
 */
typedef int (M3_CALL *M3LayoutMeasureFn)(void *ctx, M3LayoutMeasureSpec width, M3LayoutMeasureSpec height, M3Size *out_size);

/**
 * @brief Layout node definition.
 */
typedef struct M3LayoutNode {
    M3LayoutStyle style; /**< Style attributes. */
    struct M3LayoutNode **children; /**< Child node list. */
    m3_usize child_count; /**< Number of child nodes. */
    M3LayoutMeasureFn measure; /**< Measure callback for leaf nodes. */
    void *measure_ctx; /**< Measure callback context. */
    M3Size measured; /**< Measured size before layout. */
    M3Rect layout; /**< Final layout rectangle. */
} M3LayoutNode;

/**
 * @brief Initialize a measure spec.
 * @param spec Spec to initialize.
 * @param mode Measurement mode (M3_LAYOUT_MEASURE_*).
 * @param size Size constraint.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_measure_spec_init(M3LayoutMeasureSpec *spec, m3_u32 mode, M3Scalar size);

/**
 * @brief Initialize padding edges.
 * @param edges Edge descriptor to initialize.
 * @param left Left padding.
 * @param top Top padding.
 * @param right Right padding.
 * @param bottom Bottom padding.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_edges_set(M3LayoutEdges *edges, M3Scalar left, M3Scalar top, M3Scalar right, M3Scalar bottom);

/**
 * @brief Initialize a layout style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_style_init(M3LayoutStyle *style);

/**
 * @brief Initialize a layout node.
 * @param node Node to initialize.
 * @param style Style to copy; NULL uses default style.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_node_init(M3LayoutNode *node, const M3LayoutStyle *style);

/**
 * @brief Assign children to a node.
 * @param node Node to update.
 * @param children Child node array; may be NULL when count is 0.
 * @param count Number of children.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_node_set_children(M3LayoutNode *node, M3LayoutNode **children, m3_usize count);

/**
 * @brief Assign a measure callback to a node.
 * @param node Node to update.
 * @param measure Measure callback (NULL clears).
 * @param ctx Measure callback context.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_node_set_measure(M3LayoutNode *node, M3LayoutMeasureFn measure, void *ctx);

/**
 * @brief Retrieve a node's measured size.
 * @param node Node to query.
 * @param out_size Receives the measured size.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_node_get_measured(const M3LayoutNode *node, M3Size *out_size);

/**
 * @brief Retrieve a node's final layout rectangle.
 * @param node Node to query.
 * @param out_rect Receives the layout rectangle.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_node_get_layout(const M3LayoutNode *node, M3Rect *out_rect);

/**
 * @brief Compute layout for a node tree.
 * @param root Root node.
 * @param width Width constraint.
 * @param height Height constraint.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_compute(M3LayoutNode *root, M3LayoutMeasureSpec width, M3LayoutMeasureSpec height);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for validating measure specs.
 * @param spec Measure spec to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_validate_measure_spec(const M3LayoutMeasureSpec *spec);

/**
 * @brief Test wrapper for validating styles.
 * @param style Style to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_validate_style(const M3LayoutStyle *style);

/**
 * @brief Test wrapper for applying measure specs.
 * @param size Input size.
 * @param spec Measure spec.
 * @param out_size Receives the clamped size.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_apply_spec(M3Scalar size, M3LayoutMeasureSpec spec, M3Scalar *out_size);

/**
 * @brief Test wrapper for measuring leaf nodes.
 * @param node Node to measure.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_measure_leaf(M3LayoutNode *node, M3LayoutMeasureSpec width, M3LayoutMeasureSpec height, M3Size *out_size);

/**
 * @brief Test wrapper for measuring row containers.
 * @param node Node to measure.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_measure_row(M3LayoutNode *node, M3LayoutMeasureSpec width, M3LayoutMeasureSpec height, M3Size *out_size);

/**
 * @brief Test wrapper for measuring column containers.
 * @param node Node to measure.
 * @param width Width constraint.
 * @param height Height constraint.
 * @param out_size Receives the measured size.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_measure_column(M3LayoutNode *node, M3LayoutMeasureSpec width, M3LayoutMeasureSpec height, M3Size *out_size);

/**
 * @brief Test wrapper for measuring nodes.
 * @param node Node to measure.
 * @param width Width constraint.
 * @param height Height constraint.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_measure_node(M3LayoutNode *node, M3LayoutMeasureSpec width, M3LayoutMeasureSpec height);

/**
 * @brief Test wrapper for laying out nodes.
 * @param node Node to layout.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_layout_node(M3LayoutNode *node, M3Scalar x, M3Scalar y, M3Scalar width, M3Scalar height);

/**
 * @brief Test wrapper for laying out row children.
 * @param node Node to layout.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_layout_children_row(M3LayoutNode *node, M3Scalar x, M3Scalar y, M3Scalar width, M3Scalar height);

/**
 * @brief Test wrapper for laying out column children.
 * @param node Node to layout.
 * @param x Origin X.
 * @param y Origin Y.
 * @param width Assigned width.
 * @param height Assigned height.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_layout_test_layout_children_column(M3LayoutNode *node, M3Scalar x, M3Scalar y, M3Scalar width, M3Scalar height);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_LAYOUT_H */
