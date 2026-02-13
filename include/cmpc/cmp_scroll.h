#ifndef CMP_SCROLL_H
#define CMP_SCROLL_H

/**
 * @file cmp_scroll.h
 * @brief Nested scroll coordination for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_math.h"

/**
 * @brief Scroll delta along horizontal and vertical axes.
 */
typedef struct CMPScrollDelta {
  CMPScalar x; /**< Horizontal scroll delta in pixels. */
  CMPScalar y; /**< Vertical scroll delta in pixels. */
} CMPScrollDelta;

/**
 * @brief Scroll parent virtual table.
 */
typedef struct CMPScrollParentVTable {
/**
 * @brief Pre-scroll callback invoked before the child consumes the delta.
 * @param ctx Parent context pointer.
 * @param delta Remaining scroll delta.
 * @param out_consumed Receives the delta consumed by the parent.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int pre_scroll(void *ctx, const CMPScrollDelta *delta,
                 CMPScrollDelta *out_consumed);
#else
  int(CMP_CALL *pre_scroll)(void *ctx, const CMPScrollDelta *delta,
                            CMPScrollDelta *out_consumed);
#endif
/**
 * @brief Post-scroll callback invoked after the child consumes the delta.
 * @param ctx Parent context pointer.
 * @param delta Remaining scroll delta after the child scrolls.
 * @param child_consumed Delta consumed by the child.
 * @param out_consumed Receives the delta consumed by the parent.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int post_scroll(void *ctx, const CMPScrollDelta *delta,
                  const CMPScrollDelta *child_consumed,
                  CMPScrollDelta *out_consumed);
#else
  int(CMP_CALL *post_scroll)(void *ctx, const CMPScrollDelta *delta,
                             const CMPScrollDelta *child_consumed,
                             CMPScrollDelta *out_consumed);
#endif
} CMPScrollParentVTable;

/**
 * @brief Scroll parent interface.
 */
typedef struct CMPScrollParent {
  void *ctx;                           /**< Parent context pointer. */
  const CMPScrollParentVTable *vtable; /**< Parent virtual table. */
} CMPScrollParent;

/**
 * @brief Chain of scroll parents ordered from closest to farthest.
 */
typedef struct CMPScrollChain {
  CMPScrollParent **parents; /**< Parent array (may be NULL when count is 0). */
  cmp_usize count;           /**< Number of parents in the chain. */
} CMPScrollChain;

/**
 * @brief Initialize a scroll delta.
 * @param delta Delta to initialize.
 * @param x Horizontal delta in pixels.
 * @param y Vertical delta in pixels.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_scroll_delta_init(CMPScrollDelta *delta, CMPScalar x,
                                           CMPScalar y);

/**
 * @brief Initialize a scroll parent interface.
 * @param parent Parent interface to initialize.
 * @param ctx Parent context pointer.
 * @param vtable Parent virtual table.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_scroll_parent_init(
    CMPScrollParent *parent, void *ctx, const CMPScrollParentVTable *vtable);

/**
 * @brief Initialize a scroll chain.
 * @param chain Scroll chain instance.
 * @param parents Parent array (may be NULL when count is 0).
 * @param count Number of parents in the chain.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_scroll_chain_init(CMPScrollChain *chain,
                                           CMPScrollParent **parents,
                                           cmp_usize count);

/**
 * @brief Dispatch a pre-scroll through the parent chain.
 * @param chain Scroll parent chain.
 * @param delta Scroll delta to distribute.
 * @param out_consumed Receives the total delta consumed by parents.
 * @param out_remaining Receives the remaining delta after parents.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_scroll_chain_pre_scroll(const CMPScrollChain *chain,
                                                 const CMPScrollDelta *delta,
                                                 CMPScrollDelta *out_consumed,
                                                 CMPScrollDelta *out_remaining);

/**
 * @brief Dispatch a post-scroll through the parent chain.
 * @param chain Scroll parent chain.
 * @param delta_remaining Remaining delta after the child scrolls.
 * @param child_consumed Delta consumed by the child.
 * @param out_consumed Receives the total delta consumed by parents.
 * @param out_remaining Receives the remaining delta after parents.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_scroll_chain_post_scroll(
    const CMPScrollChain *chain, const CMPScrollDelta *delta_remaining,
    const CMPScrollDelta *child_consumed, CMPScrollDelta *out_consumed,
    CMPScrollDelta *out_remaining);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for scroll consumption validation.
 * @param consumed Consumed delta to validate.
 * @param available Available delta to validate against.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_scroll_test_validate_consumed(
    const CMPScrollDelta *consumed, const CMPScrollDelta *available);

/**
 * @brief Test wrapper for scroll chain validation.
 * @param chain Scroll chain to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_scroll_test_validate_chain(const CMPScrollChain *chain);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_SCROLL_H */
