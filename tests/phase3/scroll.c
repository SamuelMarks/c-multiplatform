#include "m3/m3_scroll.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestScrollParent {
  int pre_calls;
  int post_calls;
  int pre_fail;
  int post_fail;
  M3ScrollDelta pre_consumed;
  M3ScrollDelta post_consumed;
  M3ScrollDelta last_pre_delta;
  M3ScrollDelta last_post_delta;
  M3ScrollDelta last_child_consumed;
} TestScrollParent;

static int test_scroll_parent_reset(TestScrollParent *parent) {
  if (parent == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(parent, 0, sizeof(*parent));
  return M3_OK;
}

static int test_scroll_parent_pre(void *ctx, const M3ScrollDelta *delta,
                                  M3ScrollDelta *out_consumed) {
  TestScrollParent *parent;

  if (ctx == NULL || delta == NULL || out_consumed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  parent = (TestScrollParent *)ctx;
  parent->pre_calls += 1;
  parent->last_pre_delta = *delta;

  if (parent->pre_fail) {
    return M3_ERR_UNKNOWN;
  }

  *out_consumed = parent->pre_consumed;
  return M3_OK;
}

static int test_scroll_parent_post(void *ctx, const M3ScrollDelta *delta,
                                   const M3ScrollDelta *child_consumed,
                                   M3ScrollDelta *out_consumed) {
  TestScrollParent *parent;

  if (ctx == NULL || delta == NULL || child_consumed == NULL ||
      out_consumed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  parent = (TestScrollParent *)ctx;
  parent->post_calls += 1;
  parent->last_post_delta = *delta;
  parent->last_child_consumed = *child_consumed;

  if (parent->post_fail) {
    return M3_ERR_UNKNOWN;
  }

  *out_consumed = parent->post_consumed;
  return M3_OK;
}

int main(void) {
  {
    M3ScrollDelta delta;
    M3ScrollParent parent;
    M3ScrollParentVTable vtable;

    memset(&vtable, 0, sizeof(vtable));

    M3_TEST_EXPECT(m3_scroll_delta_init(NULL, 1.0f, 2.0f),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_scroll_delta_init(&delta, 1.0f, -2.0f));
    M3_TEST_ASSERT(delta.x == 1.0f);
    M3_TEST_ASSERT(delta.y == -2.0f);

    M3_TEST_EXPECT(m3_scroll_parent_init(NULL, NULL, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_scroll_parent_init(&parent, NULL, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_scroll_parent_init(&parent, &delta, &vtable));
    M3_TEST_ASSERT(parent.ctx == &delta);
    M3_TEST_ASSERT(parent.vtable == &vtable);
  }

  {
    M3ScrollChain chain;
    M3ScrollParent parent;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[1];

    memset(&chain, 0, sizeof(chain));
    memset(&vtable, 0, sizeof(vtable));

    M3_TEST_EXPECT(m3_scroll_chain_init(NULL, NULL, 0),
                   M3_ERR_INVALID_ARGUMENT);

    parents[0] = NULL;
    M3_TEST_EXPECT(m3_scroll_chain_init(&chain, parents, 1),
                   M3_ERR_INVALID_ARGUMENT);

    parent.ctx = NULL;
    parent.vtable = NULL;
    parents[0] = &parent;
    M3_TEST_EXPECT(m3_scroll_chain_init(&chain, parents, 1),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_scroll_parent_init(&parent, NULL, &vtable));
    parents[0] = &parent;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 1));
    M3_TEST_ASSERT(chain.count == 1);
    M3_TEST_ASSERT(chain.parents == parents);

    M3_TEST_OK(m3_scroll_chain_init(&chain, NULL, 0));
    M3_TEST_ASSERT(chain.count == 0);
    M3_TEST_ASSERT(chain.parents == NULL);
  }

  {
    M3ScrollDelta available;
    M3ScrollDelta consumed;

    M3_TEST_EXPECT(m3_scroll_test_validate_consumed(NULL, &available),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_scroll_test_validate_consumed(&consumed, NULL),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_scroll_delta_init(&available, 5.0f, -3.0f));
    M3_TEST_OK(m3_scroll_delta_init(&consumed, 2.0f, -1.0f));
    M3_TEST_OK(m3_scroll_test_validate_consumed(&consumed, &available));

    M3_TEST_OK(m3_scroll_delta_init(&consumed, -1.0f, -1.0f));
    M3_TEST_EXPECT(m3_scroll_test_validate_consumed(&consumed, &available),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_scroll_delta_init(&available, -2.0f, -2.0f));
    M3_TEST_OK(m3_scroll_delta_init(&consumed, 1.0f, 0.0f));
    M3_TEST_EXPECT(m3_scroll_test_validate_consumed(&consumed, &available),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_scroll_delta_init(&available, 0.0f, 0.0f));
    M3_TEST_OK(m3_scroll_delta_init(&consumed, 0.0f, 0.0f));
    M3_TEST_OK(m3_scroll_test_validate_consumed(&consumed, &available));

    M3_TEST_OK(m3_scroll_delta_init(&available, 0.0f, 0.0f));
    M3_TEST_OK(m3_scroll_delta_init(&consumed, 0.0f, 1.0f));
    M3_TEST_EXPECT(m3_scroll_test_validate_consumed(&consumed, &available),
                   M3_ERR_RANGE);
  }

  {
    M3ScrollChain chain;
    M3ScrollParent parent;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[1];

    memset(&chain, 0, sizeof(chain));
    memset(&vtable, 0, sizeof(vtable));

    M3_TEST_EXPECT(m3_scroll_test_validate_chain(NULL),
                   M3_ERR_INVALID_ARGUMENT);

    chain.parents = NULL;
    chain.count = 1;
    M3_TEST_EXPECT(m3_scroll_test_validate_chain(&chain),
                   M3_ERR_INVALID_ARGUMENT);

    parents[0] = NULL;
    chain.parents = parents;
    M3_TEST_EXPECT(m3_scroll_test_validate_chain(&chain),
                   M3_ERR_INVALID_ARGUMENT);

    parent.ctx = NULL;
    parent.vtable = NULL;
    parents[0] = &parent;
    chain.parents = parents;
    M3_TEST_EXPECT(m3_scroll_test_validate_chain(&chain),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_scroll_parent_init(&parent, NULL, &vtable));
    parents[0] = &parent;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 1));
    M3_TEST_OK(m3_scroll_test_validate_chain(&chain));
  }

  {
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    memset(&chain, 0, sizeof(chain));
    M3_TEST_OK(m3_scroll_delta_init(&delta, 1.0f, 0.0f));

    M3_TEST_EXPECT(
        m3_scroll_chain_pre_scroll(NULL, &delta, &consumed, &remaining),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_scroll_chain_pre_scroll(&chain, NULL, &consumed, &remaining),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_scroll_chain_pre_scroll(&chain, &delta, NULL, &remaining),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_scroll_chain_pre_scroll(&chain, &delta, &consumed, NULL),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    chain.parents = NULL;
    chain.count = 1;
    M3_TEST_OK(m3_scroll_delta_init(&delta, 1.0f, 0.0f));

    M3_TEST_EXPECT(
        m3_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining),
        M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    memset(&chain, 0, sizeof(chain));
    M3_TEST_OK(m3_scroll_delta_init(&delta, 3.0f, -2.0f));

    M3_TEST_OK(
        m3_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining));
    M3_TEST_ASSERT(consumed.x == 0.0f);
    M3_TEST_ASSERT(consumed.y == 0.0f);
    M3_TEST_ASSERT(remaining.x == delta.x);
    M3_TEST_ASSERT(remaining.y == delta.y);
  }

  {
    TestScrollParent state;
    M3ScrollParent parent;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[1];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&state));
    state.pre_consumed.x = 1.0f;
    state.pre_consumed.y = 1.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.pre_scroll = test_scroll_parent_pre;

    M3_TEST_OK(m3_scroll_parent_init(&parent, &state, &vtable));
    parents[0] = &parent;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 1));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 0.0f, 0.0f));
    M3_TEST_OK(
        m3_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining));

    M3_TEST_ASSERT(state.pre_calls == 0);
    M3_TEST_ASSERT(consumed.x == 0.0f);
    M3_TEST_ASSERT(consumed.y == 0.0f);
    M3_TEST_ASSERT(remaining.x == 0.0f);
    M3_TEST_ASSERT(remaining.y == 0.0f);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    M3ScrollParent scroll_parent_a;
    M3ScrollParent scroll_parent_b;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[2];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&parent_a));
    M3_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.pre_consumed.x = 2.0f;
    parent_a.pre_consumed.y = 0.0f;
    parent_b.pre_consumed.x = 1.0f;
    parent_b.pre_consumed.y = 0.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.pre_scroll = test_scroll_parent_pre;

    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 2));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 5.0f, 0.0f));
    M3_TEST_OK(
        m3_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining));

    M3_TEST_ASSERT(parent_a.pre_calls == 1);
    M3_TEST_ASSERT(parent_b.pre_calls == 1);
    M3_TEST_ASSERT(parent_a.last_pre_delta.x == 5.0f);
    M3_TEST_ASSERT(parent_b.last_pre_delta.x == 3.0f);
    M3_TEST_ASSERT(consumed.x == 3.0f);
    M3_TEST_ASSERT(consumed.y == 0.0f);
    M3_TEST_ASSERT(remaining.x == 2.0f);
    M3_TEST_ASSERT(remaining.y == 0.0f);
  }

  {
    TestScrollParent state;
    M3ScrollParent parent;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[1];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&state));

    memset(&vtable, 0, sizeof(vtable));

    M3_TEST_OK(m3_scroll_parent_init(&parent, &state, &vtable));
    parents[0] = &parent;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 1));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 4.0f, -2.0f));
    M3_TEST_OK(
        m3_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining));

    M3_TEST_ASSERT(state.pre_calls == 0);
    M3_TEST_ASSERT(consumed.x == 0.0f);
    M3_TEST_ASSERT(consumed.y == 0.0f);
    M3_TEST_ASSERT(remaining.x == 4.0f);
    M3_TEST_ASSERT(remaining.y == -2.0f);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    M3ScrollParent scroll_parent_a;
    M3ScrollParent scroll_parent_b;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[2];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&parent_a));
    M3_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.pre_fail = 1;

    memset(&vtable, 0, sizeof(vtable));
    vtable.pre_scroll = test_scroll_parent_pre;

    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 2));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 5.0f, 0.0f));
    M3_TEST_EXPECT(
        m3_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining),
        M3_ERR_UNKNOWN);

    M3_TEST_ASSERT(parent_a.pre_calls == 1);
    M3_TEST_ASSERT(parent_b.pre_calls == 0);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    M3ScrollParent scroll_parent_a;
    M3ScrollParent scroll_parent_b;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[2];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&parent_a));
    M3_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.pre_consumed.x = 6.0f;
    parent_a.pre_consumed.y = 0.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.pre_scroll = test_scroll_parent_pre;

    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 2));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 5.0f, 0.0f));
    M3_TEST_EXPECT(
        m3_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining),
        M3_ERR_RANGE);

    M3_TEST_ASSERT(parent_a.pre_calls == 1);
    M3_TEST_ASSERT(parent_b.pre_calls == 0);
  }

  {
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta child_consumed;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    memset(&chain, 0, sizeof(chain));
    M3_TEST_OK(m3_scroll_delta_init(&delta, 1.0f, 0.0f));
    M3_TEST_OK(m3_scroll_delta_init(&child_consumed, 0.0f, 0.0f));

    M3_TEST_EXPECT(m3_scroll_chain_post_scroll(NULL, &delta, &child_consumed,
                                               &consumed, &remaining),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_scroll_chain_post_scroll(&chain, NULL, &child_consumed,
                                               &consumed, &remaining),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_scroll_chain_post_scroll(&chain, &delta, NULL, &consumed,
                                               &remaining),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                               NULL, &remaining),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                               &consumed, NULL),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta child_consumed;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    chain.parents = NULL;
    chain.count = 1;
    M3_TEST_OK(m3_scroll_delta_init(&delta, 1.0f, 0.0f));
    M3_TEST_OK(m3_scroll_delta_init(&child_consumed, 0.0f, 0.0f));

    M3_TEST_EXPECT(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                               &consumed, &remaining),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta child_consumed;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    memset(&chain, 0, sizeof(chain));
    M3_TEST_OK(m3_scroll_delta_init(&delta, 3.0f, -2.0f));
    M3_TEST_OK(m3_scroll_delta_init(&child_consumed, 1.0f, -1.0f));

    M3_TEST_OK(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                           &consumed, &remaining));
    M3_TEST_ASSERT(consumed.x == 0.0f);
    M3_TEST_ASSERT(consumed.y == 0.0f);
    M3_TEST_ASSERT(remaining.x == delta.x);
    M3_TEST_ASSERT(remaining.y == delta.y);
  }

  {
    TestScrollParent state;
    M3ScrollParent parent;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[1];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta child_consumed;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&state));
    state.post_consumed.x = 1.0f;
    state.post_consumed.y = 1.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.post_scroll = test_scroll_parent_post;

    M3_TEST_OK(m3_scroll_parent_init(&parent, &state, &vtable));
    parents[0] = &parent;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 1));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 0.0f, 0.0f));
    M3_TEST_OK(m3_scroll_delta_init(&child_consumed, 0.0f, 0.0f));
    M3_TEST_OK(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                           &consumed, &remaining));

    M3_TEST_ASSERT(state.post_calls == 0);
    M3_TEST_ASSERT(consumed.x == 0.0f);
    M3_TEST_ASSERT(consumed.y == 0.0f);
    M3_TEST_ASSERT(remaining.x == 0.0f);
    M3_TEST_ASSERT(remaining.y == 0.0f);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    M3ScrollParent scroll_parent_a;
    M3ScrollParent scroll_parent_b;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[2];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta child_consumed;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&parent_a));
    M3_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.post_consumed.x = 2.0f;
    parent_a.post_consumed.y = -1.0f;
    parent_b.post_consumed.x = 1.0f;
    parent_b.post_consumed.y = 0.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.post_scroll = test_scroll_parent_post;

    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 2));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 4.0f, -2.0f));
    M3_TEST_OK(m3_scroll_delta_init(&child_consumed, 1.0f, -1.0f));
    M3_TEST_OK(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                           &consumed, &remaining));

    M3_TEST_ASSERT(parent_a.post_calls == 1);
    M3_TEST_ASSERT(parent_b.post_calls == 1);
    M3_TEST_ASSERT(parent_a.last_post_delta.x == 4.0f);
    M3_TEST_ASSERT(parent_a.last_post_delta.y == -2.0f);
    M3_TEST_ASSERT(parent_b.last_post_delta.x == 2.0f);
    M3_TEST_ASSERT(parent_b.last_post_delta.y == -1.0f);
    M3_TEST_ASSERT(parent_a.last_child_consumed.x == 1.0f);
    M3_TEST_ASSERT(parent_a.last_child_consumed.y == -1.0f);
    M3_TEST_ASSERT(parent_b.last_child_consumed.x == 1.0f);
    M3_TEST_ASSERT(parent_b.last_child_consumed.y == -1.0f);
    M3_TEST_ASSERT(consumed.x == 3.0f);
    M3_TEST_ASSERT(consumed.y == -1.0f);
    M3_TEST_ASSERT(remaining.x == 1.0f);
    M3_TEST_ASSERT(remaining.y == -1.0f);
  }

  {
    TestScrollParent state;
    M3ScrollParent parent;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[1];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta child_consumed;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&state));

    memset(&vtable, 0, sizeof(vtable));

    M3_TEST_OK(m3_scroll_parent_init(&parent, &state, &vtable));
    parents[0] = &parent;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 1));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 4.0f, -2.0f));
    M3_TEST_OK(m3_scroll_delta_init(&child_consumed, 1.0f, -1.0f));
    M3_TEST_OK(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                           &consumed, &remaining));

    M3_TEST_ASSERT(state.post_calls == 0);
    M3_TEST_ASSERT(consumed.x == 0.0f);
    M3_TEST_ASSERT(consumed.y == 0.0f);
    M3_TEST_ASSERT(remaining.x == 4.0f);
    M3_TEST_ASSERT(remaining.y == -2.0f);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    M3ScrollParent scroll_parent_a;
    M3ScrollParent scroll_parent_b;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[2];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta child_consumed;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&parent_a));
    M3_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.post_fail = 1;

    memset(&vtable, 0, sizeof(vtable));
    vtable.post_scroll = test_scroll_parent_post;

    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 2));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 4.0f, -2.0f));
    M3_TEST_OK(m3_scroll_delta_init(&child_consumed, 1.0f, -1.0f));
    M3_TEST_EXPECT(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                               &consumed, &remaining),
                   M3_ERR_UNKNOWN);

    M3_TEST_ASSERT(parent_a.post_calls == 1);
    M3_TEST_ASSERT(parent_b.post_calls == 0);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    M3ScrollParent scroll_parent_a;
    M3ScrollParent scroll_parent_b;
    M3ScrollParentVTable vtable;
    M3ScrollParent *parents[2];
    M3ScrollChain chain;
    M3ScrollDelta delta;
    M3ScrollDelta child_consumed;
    M3ScrollDelta consumed;
    M3ScrollDelta remaining;

    M3_TEST_OK(test_scroll_parent_reset(&parent_a));
    M3_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.post_consumed.x = 6.0f;
    parent_a.post_consumed.y = 0.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.post_scroll = test_scroll_parent_post;

    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    M3_TEST_OK(m3_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    M3_TEST_OK(m3_scroll_chain_init(&chain, parents, 2));

    M3_TEST_OK(m3_scroll_delta_init(&delta, 4.0f, -2.0f));
    M3_TEST_OK(m3_scroll_delta_init(&child_consumed, 1.0f, -1.0f));
    M3_TEST_EXPECT(m3_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                               &consumed, &remaining),
                   M3_ERR_RANGE);

    M3_TEST_ASSERT(parent_a.post_calls == 1);
    M3_TEST_ASSERT(parent_b.post_calls == 0);
  }

  return 0;
}
