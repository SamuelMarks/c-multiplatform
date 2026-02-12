#ifndef M3_SCROLL_H
#define M3_SCROLL_H

/**
 * @file m3_scroll.h
 * @brief Nested scroll coordination for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_math.h"

/**
 * @brief Scroll delta along horizontal and vertical axes.
 */
typedef struct M3ScrollDelta {
  M3Scalar x; /**< Horizontal scroll delta in pixels. */
  M3Scalar y; /**< Vertical scroll delta in pixels. */
} M3ScrollDelta;

/**
 * @brief Scroll parent virtual table.
 */
typedef struct M3ScrollParentVTable {
  /**
   * @brief Pre-scroll callback invoked before the child consumes the delta.
   * @param ctx Parent context pointer.
   * @param delta Remaining scroll delta.
   * @param out_consumed Receives the delta consumed by the parent.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *pre_scroll)(void *ctx, const M3ScrollDelta *delta,
                           M3ScrollDelta *out_consumed);
  /**
   * @brief Post-scroll callback invoked after the child consumes the delta.
   * @param ctx Parent context pointer.
   * @param delta Remaining scroll delta after the child scrolls.
   * @param child_consumed Delta consumed by the child.
   * @param out_consumed Receives the delta consumed by the parent.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *post_scroll)(void *ctx, const M3ScrollDelta *delta,
                            const M3ScrollDelta *child_consumed,
                            M3ScrollDelta *out_consumed);
} M3ScrollParentVTable;

/**
 * @brief Scroll parent interface.
 */
typedef struct M3ScrollParent {
  void *ctx;                          /**< Parent context pointer. */
  const M3ScrollParentVTable *vtable; /**< Parent virtual table. */
} M3ScrollParent;

/**
 * @brief Chain of scroll parents ordered from closest to farthest.
 */
typedef struct M3ScrollChain {
  M3ScrollParent **parents; /**< Parent array (may be NULL when count is 0). */
  m3_usize count;           /**< Number of parents in the chain. */
} M3ScrollChain;

/**
 * @brief Initialize a scroll delta.
 * @param delta Delta to initialize.
 * @param x Horizontal delta in pixels.
 * @param y Vertical delta in pixels.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scroll_delta_init(M3ScrollDelta *delta, M3Scalar x,
                                        M3Scalar y);

/**
 * @brief Initialize a scroll parent interface.
 * @param parent Parent interface to initialize.
 * @param ctx Parent context pointer.
 * @param vtable Parent virtual table.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scroll_parent_init(M3ScrollParent *parent, void *ctx,
                                         const M3ScrollParentVTable *vtable);

/**
 * @brief Initialize a scroll chain.
 * @param chain Scroll chain instance.
 * @param parents Parent array (may be NULL when count is 0).
 * @param count Number of parents in the chain.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scroll_chain_init(M3ScrollChain *chain,
                                        M3ScrollParent **parents,
                                        m3_usize count);

/**
 * @brief Dispatch a pre-scroll through the parent chain.
 * @param chain Scroll parent chain.
 * @param delta Scroll delta to distribute.
 * @param out_consumed Receives the total delta consumed by parents.
 * @param out_remaining Receives the remaining delta after parents.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scroll_chain_pre_scroll(const M3ScrollChain *chain,
                                              const M3ScrollDelta *delta,
                                              M3ScrollDelta *out_consumed,
                                              M3ScrollDelta *out_remaining);

/**
 * @brief Dispatch a post-scroll through the parent chain.
 * @param chain Scroll parent chain.
 * @param delta_remaining Remaining delta after the child scrolls.
 * @param child_consumed Delta consumed by the child.
 * @param out_consumed Receives the total delta consumed by parents.
 * @param out_remaining Receives the remaining delta after parents.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scroll_chain_post_scroll(
    const M3ScrollChain *chain, const M3ScrollDelta *delta_remaining,
    const M3ScrollDelta *child_consumed, M3ScrollDelta *out_consumed,
    M3ScrollDelta *out_remaining);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for scroll consumption validation.
 * @param consumed Consumed delta to validate.
 * @param available Available delta to validate against.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scroll_test_validate_consumed(
    const M3ScrollDelta *consumed, const M3ScrollDelta *available);

/**
 * @brief Test wrapper for scroll chain validation.
 * @param chain Scroll chain to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scroll_test_validate_chain(const M3ScrollChain *chain);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_SCROLL_H */
