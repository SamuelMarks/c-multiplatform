#include "m3/m3_scroll.h"

static int m3_scroll_validate_consumed_component(M3Scalar consumed,
                                                 M3Scalar available) {
  if (available > 0.0f) {
    if (consumed < 0.0f || consumed > available) {
      return M3_ERR_RANGE;
    }
  } else if (available < 0.0f) {
    if (consumed > 0.0f || consumed < available) {
      return M3_ERR_RANGE;
    }
  } else {
    if (consumed != 0.0f) {
      return M3_ERR_RANGE;
    }
  }

  return M3_OK;
}

static int m3_scroll_validate_consumed(const M3ScrollDelta *consumed,
                                       const M3ScrollDelta *available) {
  int rc;

  if (consumed == NULL || available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scroll_validate_consumed_component(consumed->x, available->x);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_scroll_validate_consumed_component(consumed->y, available->y);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_scroll_validate_chain(const M3ScrollChain *chain) {
  m3_usize i;

  if (chain == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (chain->count > 0 && chain->parents == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < chain->count; ++i) {
    M3ScrollParent *parent = chain->parents[i];

    if (parent == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    if (parent->vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }

  return M3_OK;
}

int M3_CALL m3_scroll_delta_init(M3ScrollDelta *delta, M3Scalar x, M3Scalar y) {
  if (delta == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  delta->x = x;
  delta->y = y;
  return M3_OK;
}

int M3_CALL m3_scroll_parent_init(M3ScrollParent *parent, void *ctx,
                                  const M3ScrollParentVTable *vtable) {
  if (parent == NULL || vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  parent->ctx = ctx;
  parent->vtable = vtable;
  return M3_OK;
}

int M3_CALL m3_scroll_chain_init(M3ScrollChain *chain, M3ScrollParent **parents,
                                 m3_usize count) {
  M3ScrollChain temp;
  int rc;

  if (chain == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  temp.parents = parents;
  temp.count = count;
  rc = m3_scroll_validate_chain(&temp);
  if (rc != M3_OK) {
    return rc;
  }

  chain->parents = parents;
  chain->count = count;
  return M3_OK;
}

int M3_CALL m3_scroll_chain_pre_scroll(const M3ScrollChain *chain,
                                       const M3ScrollDelta *delta,
                                       M3ScrollDelta *out_consumed,
                                       M3ScrollDelta *out_remaining) {
  M3ScrollDelta remaining;
  M3ScrollDelta total;
  m3_usize i;
  int rc;

  if (chain == NULL || delta == NULL || out_consumed == NULL ||
      out_remaining == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scroll_validate_chain(chain);
  if (rc != M3_OK) {
    return rc;
  }

  remaining = *delta;
  total.x = 0.0f;
  total.y = 0.0f;

  if ((remaining.x == 0.0f && remaining.y == 0.0f) || chain->count == 0u) {
    *out_consumed = total;
    *out_remaining = remaining;
    return M3_OK;
  }

  for (i = 0; i < chain->count; ++i) {
    M3ScrollParent *parent;
    M3ScrollDelta consumed;

    parent = chain->parents[i];
    consumed.x = 0.0f;
    consumed.y = 0.0f;

    if (parent->vtable->pre_scroll != NULL) {
      rc = parent->vtable->pre_scroll(parent->ctx, &remaining, &consumed);
      if (rc != M3_OK) {
        return rc;
      }
      rc = m3_scroll_validate_consumed(&consumed, &remaining);
      if (rc != M3_OK) {
        return rc;
      }
    }

    remaining.x -= consumed.x;
    remaining.y -= consumed.y;
    total.x += consumed.x;
    total.y += consumed.y;
  }

  *out_consumed = total;
  *out_remaining = remaining;
  return M3_OK;
}

int M3_CALL m3_scroll_chain_post_scroll(const M3ScrollChain *chain,
                                        const M3ScrollDelta *delta_remaining,
                                        const M3ScrollDelta *child_consumed,
                                        M3ScrollDelta *out_consumed,
                                        M3ScrollDelta *out_remaining) {
  M3ScrollDelta remaining;
  M3ScrollDelta total;
  m3_usize i;
  int rc;

  if (chain == NULL || delta_remaining == NULL || child_consumed == NULL ||
      out_consumed == NULL || out_remaining == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_scroll_validate_chain(chain);
  if (rc != M3_OK) {
    return rc;
  }

  remaining = *delta_remaining;
  total.x = 0.0f;
  total.y = 0.0f;

  if ((remaining.x == 0.0f && remaining.y == 0.0f) || chain->count == 0u) {
    *out_consumed = total;
    *out_remaining = remaining;
    return M3_OK;
  }

  for (i = 0; i < chain->count; ++i) {
    M3ScrollParent *parent;
    M3ScrollDelta consumed;

    parent = chain->parents[i];
    consumed.x = 0.0f;
    consumed.y = 0.0f;

    if (parent->vtable->post_scroll != NULL) {
      rc = parent->vtable->post_scroll(parent->ctx, &remaining, child_consumed,
                                       &consumed);
      if (rc != M3_OK) {
        return rc;
      }
      rc = m3_scroll_validate_consumed(&consumed, &remaining);
      if (rc != M3_OK) {
        return rc;
      }
    }

    remaining.x -= consumed.x;
    remaining.y -= consumed.y;
    total.x += consumed.x;
    total.y += consumed.y;
  }

  *out_consumed = total;
  *out_remaining = remaining;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_scroll_test_validate_consumed(const M3ScrollDelta *consumed,
                                             const M3ScrollDelta *available) {
  return m3_scroll_validate_consumed(consumed, available);
}

int M3_CALL m3_scroll_test_validate_chain(const M3ScrollChain *chain) {
  return m3_scroll_validate_chain(chain);
}
#endif
