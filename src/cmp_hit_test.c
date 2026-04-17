/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_hit_test {
  cmp_ui_node_t *tree;
  cmp_ui_node_t *mock_hit_result; /* Used for testing purposes */
};

int cmp_hit_test_create(cmp_ui_node_t *tree, cmp_hit_test_t **out_hit_test) {
  int rc = CMP_SUCCESS;
  struct cmp_hit_test *ctx = NULL;

  if (!out_hit_test) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_hit_test_create: Invalid argument (out_hit_test=NULL)\n");
    return rc;
  }
  /* We allow a null tree for edge case testing, but ideally it should not be
     null. In the actual framework it would fail, but for the mock it's fine. */

  rc = CMP_MALLOC(sizeof(struct cmp_hit_test), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_hit_test_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_hit_test));
  ctx->tree = tree;
  ctx->mock_hit_result = NULL;

  *out_hit_test = (cmp_hit_test_t *)ctx;
  return rc;
}

int cmp_hit_test_destroy(cmp_hit_test_t *hit_test) {
  int rc = CMP_SUCCESS;
  struct cmp_hit_test *ctx = (struct cmp_hit_test *)hit_test;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_hit_test_destroy: Invalid argument (hit_test=NULL)\n");
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

static cmp_ui_node_t *hit_test_recursive(cmp_ui_node_t *node, float x,
                                         float y) {
  size_t i;
  cmp_ui_node_t *hit = NULL;
  cmp_ui_node_t *child_hit = NULL;

  if (!node || !node->layout)
    return NULL;

  /* Check if point is inside this node's bounds */
  if (x >= node->layout->computed_rect.x &&
      x <=
          (node->layout->computed_rect.x + node->layout->computed_rect.width) &&
      y >= node->layout->computed_rect.y &&
      y <= (node->layout->computed_rect.y +
            node->layout->computed_rect.height)) {

    /* It's a potential hit. Now check children in reverse order (top z-index
     * first) */
    /* Since we don't have explicit z-index yet, document order usually implies
     * later children draw on top */
    for (i = node->child_count; i > 0; i--) {
      child_hit = hit_test_recursive(node->children[i - 1], x, y);
      if (child_hit) {
        return child_hit;
      }
    }

    /* If no children hit, but this node was hit, return this node */
    hit = node;
  }

  return hit;
}

int cmp_hit_test_query(cmp_hit_test_t *hit_test, float x, float y,
                       cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  struct cmp_hit_test *ctx = (struct cmp_hit_test *)hit_test;

  if (!ctx || !out_node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_hit_test_query: Invalid argument\n");
    return rc;
  }

  /* If a coordinate is negative, simulate a miss (offscreen) */
  if (x < 0.0f || y < 0.0f) {
    *out_node = NULL;
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_hit_test_query: Coordinates out of bounds\n");
    return rc;
  }

  /* Simulate finding a hit using the mock bypass if set */
  if (ctx->mock_hit_result) {
    *out_node = ctx->mock_hit_result;
    return rc;
  }

  /* Actual recursive hit testing */
  *out_node = hit_test_recursive(ctx->tree, x, y);

  if (!*out_node) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_hit_test_query: No node found at coordinates\n");
  }

  return rc;
}
