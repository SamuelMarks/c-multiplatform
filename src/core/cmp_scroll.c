#include "cmpc/cmp_scroll.h"

static int cmp_scroll_validate_consumed_component(CMPScalar consumed,
                                                  CMPScalar available) {
  if (available > 0.0f) {
    if (consumed < 0.0f || consumed > available) {
      return CMP_ERR_RANGE;
    }
  } else if (available < 0.0f) {
    if (consumed > 0.0f || consumed < available) {
      return CMP_ERR_RANGE;
    }
  } else {
    if (consumed != 0.0f) {
      return CMP_ERR_RANGE;
    }
  }

  return CMP_OK;
}

static int cmp_scroll_validate_consumed(const CMPScrollDelta *consumed,
                                        const CMPScrollDelta *available) {
  int rc;

  if (consumed == NULL || available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_scroll_validate_consumed_component(consumed->x, available->x);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_scroll_validate_consumed_component(consumed->y, available->y);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int cmp_scroll_validate_chain(const CMPScrollChain *chain) {
  cmp_usize i;

  if (chain == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (chain->count > 0 && chain->parents == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < chain->count; ++i) {
    CMPScrollParent *parent = chain->parents[i];

    if (parent == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    if (parent->vtable == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }

  return CMP_OK;
}

int CMP_CALL cmp_scroll_delta_init(CMPScrollDelta *delta, CMPScalar x,
                                   CMPScalar y) {
  if (delta == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  delta->x = x;
  delta->y = y;
  return CMP_OK;
}

int CMP_CALL cmp_scroll_parent_init(CMPScrollParent *parent, void *ctx,
                                    const CMPScrollParentVTable *vtable) {
  if (parent == NULL || vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  parent->ctx = ctx;
  parent->vtable = vtable;
  return CMP_OK;
}

int CMP_CALL cmp_scroll_chain_init(CMPScrollChain *chain,
                                   CMPScrollParent **parents, cmp_usize count) {
  CMPScrollChain temp;
  int rc;

  if (chain == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  temp.parents = parents;
  temp.count = count;
  rc = cmp_scroll_validate_chain(&temp);
  if (rc != CMP_OK) {
    return rc;
  }

  chain->parents = parents;
  chain->count = count;
  return CMP_OK;
}

int CMP_CALL cmp_scroll_chain_pre_scroll(const CMPScrollChain *chain,
                                         const CMPScrollDelta *delta,
                                         CMPScrollDelta *out_consumed,
                                         CMPScrollDelta *out_remaining) {
  CMPScrollDelta remaining;
  CMPScrollDelta total;
  cmp_usize i;
  int rc;

  if (chain == NULL || delta == NULL || out_consumed == NULL ||
      out_remaining == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_scroll_validate_chain(chain);
  if (rc != CMP_OK) {
    return rc;
  }

  remaining = *delta;
  total.x = 0.0f;
  total.y = 0.0f;

  if ((remaining.x == 0.0f && remaining.y == 0.0f) || chain->count == 0u) {
    *out_consumed = total;
    *out_remaining = remaining;
    return CMP_OK;
  }

  for (i = 0; i < chain->count; ++i) {
    CMPScrollParent *parent;
    CMPScrollDelta consumed;

    parent = chain->parents[i];
    consumed.x = 0.0f;
    consumed.y = 0.0f;

    if (parent->vtable->pre_scroll != NULL) {
      rc = parent->vtable->pre_scroll(parent->ctx, &remaining, &consumed);
      if (rc != CMP_OK) {
        return rc;
      }
      rc = cmp_scroll_validate_consumed(&consumed, &remaining);
      if (rc != CMP_OK) {
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
  return CMP_OK;
}

int CMP_CALL cmp_scroll_chain_post_scroll(const CMPScrollChain *chain,
                                          const CMPScrollDelta *delta_remaining,
                                          const CMPScrollDelta *child_consumed,
                                          CMPScrollDelta *out_consumed,
                                          CMPScrollDelta *out_remaining) {
  CMPScrollDelta remaining;
  CMPScrollDelta total;
  cmp_usize i;
  int rc;

  if (chain == NULL || delta_remaining == NULL || child_consumed == NULL ||
      out_consumed == NULL || out_remaining == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_scroll_validate_chain(chain);
  if (rc != CMP_OK) {
    return rc;
  }

  remaining = *delta_remaining;
  total.x = 0.0f;
  total.y = 0.0f;

  if ((remaining.x == 0.0f && remaining.y == 0.0f) || chain->count == 0u) {
    *out_consumed = total;
    *out_remaining = remaining;
    return CMP_OK;
  }

  for (i = 0; i < chain->count; ++i) {
    CMPScrollParent *parent;
    CMPScrollDelta consumed;

    parent = chain->parents[i];
    consumed.x = 0.0f;
    consumed.y = 0.0f;

    if (parent->vtable->post_scroll != NULL) {
      rc = parent->vtable->post_scroll(parent->ctx, &remaining, child_consumed,
                                       &consumed);
      if (rc != CMP_OK) {
        return rc;
      }
      rc = cmp_scroll_validate_consumed(&consumed, &remaining);
      if (rc != CMP_OK) {
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
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_scroll_test_validate_consumed(
    const CMPScrollDelta *consumed, const CMPScrollDelta *available) {
  return cmp_scroll_validate_consumed(consumed, available);
}

int CMP_CALL cmp_scroll_test_validate_chain(const CMPScrollChain *chain) {
  return cmp_scroll_validate_chain(chain);
}
#endif
