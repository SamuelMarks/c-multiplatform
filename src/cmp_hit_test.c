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
  int rc = 0; /* CMP_SUCCESS */
  struct cmp_hit_test *ctx = NULL;

  if (!out_hit_test) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != 0) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_hit_test_create: %s\n", err_str);
    }
    return rc;
  }
  /* We allow a null tree for edge case testing, but ideally it should not be
     null. In the actual framework it would fail, but for the mock it's fine. */

  rc = CMP_MALLOC(sizeof(struct cmp_hit_test), (void **)&ctx);
  if (rc != 0) {
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_hit_test));
  ctx->tree = tree;
  ctx->mock_hit_result = NULL;

  *out_hit_test = (cmp_hit_test_t *)ctx;
  return 0;
}

/**
 * @brief Destroys a hit test context.
 *
 * @param hit_test Pointer to the hit test context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hit_test_destroy(cmp_hit_test_t *hit_test) {
  int rc = 0; /* CMP_SUCCESS */
  struct cmp_hit_test *ctx = (struct cmp_hit_test *)hit_test;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != 0) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_hit_test_destroy: %s\n", err_str);
    }
    return rc;
  }

  rc = CMP_FREE(ctx);
  if (rc != 0) {
    LOG_DEBUG("cmp_hit_test_destroy: CMP_FREE failed\n");
  }
  return rc;
}

/**
 * @brief hit_test_recursive
 *
 * @param node Parameter description.
 * @param x Parameter description.
 * @param y Parameter description.
 * @param out_hit Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int hit_test_recursive(cmp_ui_node_t *node, float x, float y, cmp_ui_node_t **out_hit) {
  size_t i;
  cmp_ui_node_t *child_hit = NULL;
  int rc = 0;

  if (!out_hit) {
    return CMP_ERROR_INVALID_ARG;
  }
  
  *out_hit = NULL;

  if (!node || !node->layout) {
    return 0;
  }

  /* Check if point is inside this node's bounds */
  if (x >= node->layout->computed_rect.x &&
      x <= (node->layout->computed_rect.x + node->layout->computed_rect.width) &&
      y >= node->layout->computed_rect.y &&
      y <= (node->layout->computed_rect.y + node->layout->computed_rect.height)) {

    /* It's a potential hit. Now check children in reverse order (top z-index first) */
    for (i = node->child_count; i > 0; i--) {
      rc = hit_test_recursive(node->children[i - 1], x, y, &child_hit);
      if (rc != 0) {
        return rc;
      }
      if (child_hit) {
        *out_hit = child_hit;
        return 0;
      }
    }

    /* If no children hit, but this node was hit, return this node */
    *out_hit = node;
  }

  return 0;
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
  int rc = 0; /* CMP_SUCCESS */
  struct cmp_hit_test *ctx = (struct cmp_hit_test *)hit_test;

  if (!ctx || !out_node) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != 0) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_hit_test_query: %s\n", err_str);
    }
    return rc;
  }

  /* If a coordinate is negative, simulate a miss (offscreen) */
  if (x < 0.0f || y < 0.0f) {
    *out_node = NULL;
    rc = CMP_ERROR_NOT_FOUND;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != 0) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_hit_test_query out of bounds: %s\n", err_str);
    }
    return rc;
  }

  /* Simulate finding a hit using the mock bypass if set */
  if (ctx->mock_hit_result) {
    *out_node = ctx->mock_hit_result;
    return 0;
  }

  /* Actual recursive hit testing */
  rc = hit_test_recursive(ctx->tree, x, y, out_node);
  if (rc != 0) {
    LOG_DEBUG("cmp_hit_test_query hit_test_recursive failed\n");
    return rc;
  }

  if (!*out_node) {
    rc = CMP_ERROR_NOT_FOUND;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != 0) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_hit_test_query miss: %s\n", err_str);
    }
  }

  return rc;
}
