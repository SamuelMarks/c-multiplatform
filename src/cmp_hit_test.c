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

/**
 * @brief Creates a hit test context.
 *
 * @param tree Pointer to the root UI node of the tree to hit test against.
 * @param out_hit_test Pointer to a variable where the new hit test context will
 * be stored.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hit_test_create(cmp_ui_node_t *tree, cmp_hit_test_t **out_hit_test) {
  int rc = CMP_SUCCESS; /* CMP_SUCCESS */
  struct cmp_hit_test *ctx = NULL;

  if (out_hit_test == NULL) {
    LOG_DEBUG("cmp_hit_test_create: out_hit_test is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* We allow a null tree for edge case testing, but ideally it should not be
     null. In the actual framework it would fail, but for the mock it's fine. */

  rc = CMP_MALLOC(sizeof(struct cmp_hit_test), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_hit_test_create: CMP_MALLOC failed\n");
    return rc;
  }
  if (ctx == NULL) {
    return CMP_ERROR_GENERAL;
  }

  memset(ctx, 0, sizeof(struct cmp_hit_test));
  ctx->tree = tree;
  ctx->mock_hit_result = NULL;

  *out_hit_test = (cmp_hit_test_t *)ctx;
  return rc;
}

/**
 * @brief Destroys a hit test context.
 *
 * @param hit_test Pointer to the hit test context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hit_test_destroy(cmp_hit_test_t *hit_test) {
  int rc = CMP_SUCCESS; /* CMP_SUCCESS */
  struct cmp_hit_test *ctx = (struct cmp_hit_test *)hit_test;

  if (ctx == NULL) {
    LOG_DEBUG("cmp_hit_test_destroy: hit_test is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_hit_test_destroy: CMP_FREE failed\n");
  }
  return rc;
}

/**
 * @brief Recursive helper to find hit node.
 *
 * @param node UI node to test.
 * @param x x coordinate.
 * @param y y coordinate.
 * @param out_hit Pointer to store hit node.
 * @return Returns 0 on success, or an error code on failure.
 */
static int hit_test_recursive(cmp_ui_node_t *node, float x, float y,
                              cmp_ui_node_t **out_hit) {
  size_t i;
  cmp_ui_node_t *child_hit = NULL;
  int rc = CMP_SUCCESS;

  if (out_hit == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_hit = NULL;

  if (node == NULL || node->layout == NULL) {
    return rc;
  }

  /* Check if point is inside this node's bounds */
  if (x >= node->layout->computed_rect.x &&
      x <=
          (node->layout->computed_rect.x + node->layout->computed_rect.width) &&
      y >= node->layout->computed_rect.y &&
      y <= (node->layout->computed_rect.y +
            node->layout->computed_rect.height)) {

    /* It's a potential hit. Now check children in reverse order (top z-index
     * first) */
    for (i = node->child_count; i > 0; i--) {
      rc = hit_test_recursive(node->children[i - 1], x, y, &child_hit);

      if (child_hit != NULL) {
        *out_hit = child_hit;
        return rc;
      }
    }

    /* If no children hit, but this node was hit, return this node */
    *out_hit = node;
  }

  return rc;
}

/**
 * @brief Queries the hit test context for the node at the given coordinates.
 *
 * @param hit_test Pointer to the hit test context.
 * @param x The x-coordinate to test.
 * @param y The y-coordinate to test.
 * @param out_node Pointer to a variable where the hit node will be stored.
 * @return Returns 0 on success, or an error code on failure (e.g.
 * CMP_ERROR_NOT_FOUND).
 */
int cmp_hit_test_query(cmp_hit_test_t *hit_test, float x, float y,
                       cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS; /* CMP_SUCCESS */
  struct cmp_hit_test *ctx = (struct cmp_hit_test *)hit_test;

  if (ctx == NULL || out_node == NULL) {
    LOG_DEBUG("cmp_hit_test_query: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* If a coordinate is negative, simulate a miss (offscreen) */
  if (x < 0.0f || y < 0.0f) {
    *out_node = NULL;
    LOG_DEBUG("cmp_hit_test_query out of bounds\n");
    return CMP_ERROR_NOT_FOUND;
  }

  /* Simulate finding a hit using the mock bypass if set */
  if (ctx->mock_hit_result != NULL) {
    *out_node = ctx->mock_hit_result;
    return rc;
  }

  /* Actual recursive hit testing */
  rc = hit_test_recursive(ctx->tree, x, y, out_node);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_hit_test_query hit_test_recursive failed\n");
    return rc;
  }

  if (*out_node == NULL) {
    LOG_DEBUG("cmp_hit_test_query miss\n");
    return CMP_ERROR_NOT_FOUND;
  }

  return rc;
}
