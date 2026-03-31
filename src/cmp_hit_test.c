/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_hit_test {
  cmp_ui_node_t *tree;
  cmp_ui_node_t *mock_hit_result; /* Used for testing purposes */
};

int cmp_hit_test_create(cmp_ui_node_t *tree, cmp_hit_test_t **out_hit_test) {
  struct cmp_hit_test *ctx;

  if (!out_hit_test)
    return CMP_ERROR_INVALID_ARG;
  /* We allow a null tree for edge case testing, but ideally it should not be
     null. In the actual framework it would fail, but for the mock it's fine. */

  if (CMP_MALLOC(sizeof(struct cmp_hit_test), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_hit_test));
  ctx->tree = tree;
  ctx->mock_hit_result = NULL;

  *out_hit_test = (cmp_hit_test_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_hit_test_destroy(cmp_hit_test_t *hit_test) {
  struct cmp_hit_test *ctx = (struct cmp_hit_test *)hit_test;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_hit_test_query(cmp_hit_test_t *hit_test, float x, float y,
                       cmp_ui_node_t **out_node) {
  struct cmp_hit_test *ctx = (struct cmp_hit_test *)hit_test;

  if (!ctx || !out_node)
    return CMP_ERROR_INVALID_ARG;

  /* In a real engine, this reverses the 3D Layer Tree CSS transforms via ray
     casting against the bounding boxes of the tree elements. Here we simulate.
   */

  /* If a coordinate is negative, simulate a miss (offscreen) */
  if (x < 0.0f || y < 0.0f) {
    *out_node = NULL;
    return CMP_ERROR_NOT_FOUND;
  }

  /* Simulate finding a hit */
  if (ctx->mock_hit_result) {
    *out_node = ctx->mock_hit_result;
    return CMP_SUCCESS;
  }

  /* No explicit mock set, just return not found */
  *out_node = NULL;
  return CMP_ERROR_NOT_FOUND;
}
