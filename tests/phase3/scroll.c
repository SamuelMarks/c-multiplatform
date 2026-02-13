#include "cmpc/cmp_scroll.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestScrollParent {
  int pre_calls;
  int post_calls;
  int pre_fail;
  int post_fail;
  CMPScrollDelta pre_consumed;
  CMPScrollDelta post_consumed;
  CMPScrollDelta last_pre_delta;
  CMPScrollDelta last_post_delta;
  CMPScrollDelta last_child_consumed;
} TestScrollParent;

static int test_scroll_parent_reset(TestScrollParent *parent) {
  if (parent == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(parent, 0, sizeof(*parent));
  return CMP_OK;
}

static int test_scroll_parent_pre(void *ctx, const CMPScrollDelta *delta,
                                  CMPScrollDelta *out_consumed) {
  TestScrollParent *parent;

  if (ctx == NULL || delta == NULL || out_consumed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  parent = (TestScrollParent *)ctx;
  parent->pre_calls += 1;
  parent->last_pre_delta = *delta;

  if (parent->pre_fail) {
    return CMP_ERR_UNKNOWN;
  }

  *out_consumed = parent->pre_consumed;
  return CMP_OK;
}

static int test_scroll_parent_post(void *ctx, const CMPScrollDelta *delta,
                                   const CMPScrollDelta *child_consumed,
                                   CMPScrollDelta *out_consumed) {
  TestScrollParent *parent;

  if (ctx == NULL || delta == NULL || child_consumed == NULL ||
      out_consumed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  parent = (TestScrollParent *)ctx;
  parent->post_calls += 1;
  parent->last_post_delta = *delta;
  parent->last_child_consumed = *child_consumed;

  if (parent->post_fail) {
    return CMP_ERR_UNKNOWN;
  }

  *out_consumed = parent->post_consumed;
  return CMP_OK;
}

int main(void) {
  {
    CMPScrollDelta delta;
    CMPScrollParent parent;
    CMPScrollParentVTable vtable;

    memset(&vtable, 0, sizeof(vtable));

    CMP_TEST_EXPECT(cmp_scroll_delta_init(NULL, 1.0f, 2.0f),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 1.0f, -2.0f));
    CMP_TEST_ASSERT(delta.x == 1.0f);
    CMP_TEST_ASSERT(delta.y == -2.0f);

    CMP_TEST_EXPECT(cmp_scroll_parent_init(NULL, NULL, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_scroll_parent_init(&parent, NULL, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_scroll_parent_init(&parent, &delta, &vtable));
    CMP_TEST_ASSERT(parent.ctx == &delta);
    CMP_TEST_ASSERT(parent.vtable == &vtable);
  }

  {
    CMPScrollChain chain;
    CMPScrollParent parent;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[1];

    memset(&chain, 0, sizeof(chain));
    memset(&vtable, 0, sizeof(vtable));

    CMP_TEST_EXPECT(cmp_scroll_chain_init(NULL, NULL, 0),
                    CMP_ERR_INVALID_ARGUMENT);

    parents[0] = NULL;
    CMP_TEST_EXPECT(cmp_scroll_chain_init(&chain, parents, 1),
                    CMP_ERR_INVALID_ARGUMENT);

    parent.ctx = NULL;
    parent.vtable = NULL;
    parents[0] = &parent;
    CMP_TEST_EXPECT(cmp_scroll_chain_init(&chain, parents, 1),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_scroll_parent_init(&parent, NULL, &vtable));
    parents[0] = &parent;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 1));
    CMP_TEST_ASSERT(chain.count == 1);
    CMP_TEST_ASSERT(chain.parents == parents);

    CMP_TEST_OK(cmp_scroll_chain_init(&chain, NULL, 0));
    CMP_TEST_ASSERT(chain.count == 0);
    CMP_TEST_ASSERT(chain.parents == NULL);
  }

  {
    CMPScrollDelta available;
    CMPScrollDelta consumed;

    memset(&available, 0, sizeof(available));
    memset(&consumed, 0, sizeof(consumed));

    CMP_TEST_EXPECT(cmp_scroll_test_validate_consumed(NULL, &available),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_scroll_test_validate_consumed(&consumed, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_scroll_delta_init(&available, 5.0f, -3.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&consumed, 2.0f, -1.0f));
    CMP_TEST_OK(cmp_scroll_test_validate_consumed(&consumed, &available));

    CMP_TEST_OK(cmp_scroll_delta_init(&consumed, -1.0f, -1.0f));
    CMP_TEST_EXPECT(cmp_scroll_test_validate_consumed(&consumed, &available),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(cmp_scroll_delta_init(&available, -2.0f, -2.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&consumed, 1.0f, 0.0f));
    CMP_TEST_EXPECT(cmp_scroll_test_validate_consumed(&consumed, &available),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(cmp_scroll_delta_init(&available, 0.0f, 0.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&consumed, 0.0f, 0.0f));
    CMP_TEST_OK(cmp_scroll_test_validate_consumed(&consumed, &available));

    CMP_TEST_OK(cmp_scroll_delta_init(&available, 0.0f, 0.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&consumed, 0.0f, 1.0f));
    CMP_TEST_EXPECT(cmp_scroll_test_validate_consumed(&consumed, &available),
                    CMP_ERR_RANGE);
  }

  {
    CMPScrollChain chain;
    CMPScrollParent parent;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[1];

    memset(&chain, 0, sizeof(chain));
    memset(&vtable, 0, sizeof(vtable));

    CMP_TEST_EXPECT(cmp_scroll_test_validate_chain(NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    chain.parents = NULL;
    chain.count = 1;
    CMP_TEST_EXPECT(cmp_scroll_test_validate_chain(&chain),
                    CMP_ERR_INVALID_ARGUMENT);

    parents[0] = NULL;
    chain.parents = parents;
    CMP_TEST_EXPECT(cmp_scroll_test_validate_chain(&chain),
                    CMP_ERR_INVALID_ARGUMENT);

    parent.ctx = NULL;
    parent.vtable = NULL;
    parents[0] = &parent;
    chain.parents = parents;
    CMP_TEST_EXPECT(cmp_scroll_test_validate_chain(&chain),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_scroll_parent_init(&parent, NULL, &vtable));
    parents[0] = &parent;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 1));
    CMP_TEST_OK(cmp_scroll_test_validate_chain(&chain));
  }

  {
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    memset(&chain, 0, sizeof(chain));
    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 1.0f, 0.0f));

    CMP_TEST_EXPECT(
        cmp_scroll_chain_pre_scroll(NULL, &delta, &consumed, &remaining),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_scroll_chain_pre_scroll(&chain, NULL, &consumed, &remaining),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_scroll_chain_pre_scroll(&chain, &delta, NULL, &remaining),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_scroll_chain_pre_scroll(&chain, &delta, &consumed, NULL),
        CMP_ERR_INVALID_ARGUMENT);
  }

  {
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    chain.parents = NULL;
    chain.count = 1;
    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 1.0f, 0.0f));

    CMP_TEST_EXPECT(
        cmp_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining),
        CMP_ERR_INVALID_ARGUMENT);
  }

  {
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    memset(&chain, 0, sizeof(chain));
    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 3.0f, -2.0f));

    CMP_TEST_OK(
        cmp_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining));
    CMP_TEST_ASSERT(consumed.x == 0.0f);
    CMP_TEST_ASSERT(consumed.y == 0.0f);
    CMP_TEST_ASSERT(remaining.x == delta.x);
    CMP_TEST_ASSERT(remaining.y == delta.y);
  }

  {
    TestScrollParent state;
    CMPScrollParent parent;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[1];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&state));
    state.pre_consumed.x = 1.0f;
    state.pre_consumed.y = 1.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.pre_scroll = test_scroll_parent_pre;

    CMP_TEST_OK(cmp_scroll_parent_init(&parent, &state, &vtable));
    parents[0] = &parent;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 1));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 0.0f, 0.0f));
    CMP_TEST_OK(
        cmp_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining));

    CMP_TEST_ASSERT(state.pre_calls == 0);
    CMP_TEST_ASSERT(consumed.x == 0.0f);
    CMP_TEST_ASSERT(consumed.y == 0.0f);
    CMP_TEST_ASSERT(remaining.x == 0.0f);
    CMP_TEST_ASSERT(remaining.y == 0.0f);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    CMPScrollParent scroll_parent_a;
    CMPScrollParent scroll_parent_b;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[2];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&parent_a));
    CMP_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.pre_consumed.x = 2.0f;
    parent_a.pre_consumed.y = 0.0f;
    parent_b.pre_consumed.x = 1.0f;
    parent_b.pre_consumed.y = 0.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.pre_scroll = test_scroll_parent_pre;

    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 2));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 5.0f, 0.0f));
    CMP_TEST_OK(
        cmp_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining));

    CMP_TEST_ASSERT(parent_a.pre_calls == 1);
    CMP_TEST_ASSERT(parent_b.pre_calls == 1);
    CMP_TEST_ASSERT(parent_a.last_pre_delta.x == 5.0f);
    CMP_TEST_ASSERT(parent_b.last_pre_delta.x == 3.0f);
    CMP_TEST_ASSERT(consumed.x == 3.0f);
    CMP_TEST_ASSERT(consumed.y == 0.0f);
    CMP_TEST_ASSERT(remaining.x == 2.0f);
    CMP_TEST_ASSERT(remaining.y == 0.0f);
  }

  {
    TestScrollParent state;
    CMPScrollParent parent;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[1];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&state));

    memset(&vtable, 0, sizeof(vtable));

    CMP_TEST_OK(cmp_scroll_parent_init(&parent, &state, &vtable));
    parents[0] = &parent;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 1));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 4.0f, -2.0f));
    CMP_TEST_OK(
        cmp_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining));

    CMP_TEST_ASSERT(state.pre_calls == 0);
    CMP_TEST_ASSERT(consumed.x == 0.0f);
    CMP_TEST_ASSERT(consumed.y == 0.0f);
    CMP_TEST_ASSERT(remaining.x == 4.0f);
    CMP_TEST_ASSERT(remaining.y == -2.0f);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    CMPScrollParent scroll_parent_a;
    CMPScrollParent scroll_parent_b;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[2];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&parent_a));
    CMP_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.pre_fail = 1;

    memset(&vtable, 0, sizeof(vtable));
    vtable.pre_scroll = test_scroll_parent_pre;

    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 2));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 5.0f, 0.0f));
    CMP_TEST_EXPECT(
        cmp_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining),
        CMP_ERR_UNKNOWN);

    CMP_TEST_ASSERT(parent_a.pre_calls == 1);
    CMP_TEST_ASSERT(parent_b.pre_calls == 0);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    CMPScrollParent scroll_parent_a;
    CMPScrollParent scroll_parent_b;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[2];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&parent_a));
    CMP_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.pre_consumed.x = 6.0f;
    parent_a.pre_consumed.y = 0.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.pre_scroll = test_scroll_parent_pre;

    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 2));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 5.0f, 0.0f));
    CMP_TEST_EXPECT(
        cmp_scroll_chain_pre_scroll(&chain, &delta, &consumed, &remaining),
        CMP_ERR_RANGE);

    CMP_TEST_ASSERT(parent_a.pre_calls == 1);
    CMP_TEST_ASSERT(parent_b.pre_calls == 0);
  }

  {
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta child_consumed;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    memset(&chain, 0, sizeof(chain));
    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 1.0f, 0.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&child_consumed, 0.0f, 0.0f));

    CMP_TEST_EXPECT(cmp_scroll_chain_post_scroll(NULL, &delta, &child_consumed,
                                                 &consumed, &remaining),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_scroll_chain_post_scroll(&chain, NULL, &child_consumed,
                                                 &consumed, &remaining),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_scroll_chain_post_scroll(&chain, &delta, NULL,
                                                 &consumed, &remaining),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_scroll_chain_post_scroll(
                        &chain, &delta, &child_consumed, NULL, &remaining),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_scroll_chain_post_scroll(
                        &chain, &delta, &child_consumed, &consumed, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta child_consumed;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    chain.parents = NULL;
    chain.count = 1;
    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 1.0f, 0.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&child_consumed, 0.0f, 0.0f));

    CMP_TEST_EXPECT(cmp_scroll_chain_post_scroll(
                        &chain, &delta, &child_consumed, &consumed, &remaining),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta child_consumed;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    memset(&chain, 0, sizeof(chain));
    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 3.0f, -2.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&child_consumed, 1.0f, -1.0f));

    CMP_TEST_OK(cmp_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                             &consumed, &remaining));
    CMP_TEST_ASSERT(consumed.x == 0.0f);
    CMP_TEST_ASSERT(consumed.y == 0.0f);
    CMP_TEST_ASSERT(remaining.x == delta.x);
    CMP_TEST_ASSERT(remaining.y == delta.y);
  }

  {
    TestScrollParent state;
    CMPScrollParent parent;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[1];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta child_consumed;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&state));
    state.post_consumed.x = 1.0f;
    state.post_consumed.y = 1.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.post_scroll = test_scroll_parent_post;

    CMP_TEST_OK(cmp_scroll_parent_init(&parent, &state, &vtable));
    parents[0] = &parent;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 1));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 0.0f, 0.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&child_consumed, 0.0f, 0.0f));
    CMP_TEST_OK(cmp_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                             &consumed, &remaining));

    CMP_TEST_ASSERT(state.post_calls == 0);
    CMP_TEST_ASSERT(consumed.x == 0.0f);
    CMP_TEST_ASSERT(consumed.y == 0.0f);
    CMP_TEST_ASSERT(remaining.x == 0.0f);
    CMP_TEST_ASSERT(remaining.y == 0.0f);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    CMPScrollParent scroll_parent_a;
    CMPScrollParent scroll_parent_b;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[2];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta child_consumed;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&parent_a));
    CMP_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.post_consumed.x = 2.0f;
    parent_a.post_consumed.y = -1.0f;
    parent_b.post_consumed.x = 1.0f;
    parent_b.post_consumed.y = 0.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.post_scroll = test_scroll_parent_post;

    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 2));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 4.0f, -2.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&child_consumed, 1.0f, -1.0f));
    CMP_TEST_OK(cmp_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                             &consumed, &remaining));

    CMP_TEST_ASSERT(parent_a.post_calls == 1);
    CMP_TEST_ASSERT(parent_b.post_calls == 1);
    CMP_TEST_ASSERT(parent_a.last_post_delta.x == 4.0f);
    CMP_TEST_ASSERT(parent_a.last_post_delta.y == -2.0f);
    CMP_TEST_ASSERT(parent_b.last_post_delta.x == 2.0f);
    CMP_TEST_ASSERT(parent_b.last_post_delta.y == -1.0f);
    CMP_TEST_ASSERT(parent_a.last_child_consumed.x == 1.0f);
    CMP_TEST_ASSERT(parent_a.last_child_consumed.y == -1.0f);
    CMP_TEST_ASSERT(parent_b.last_child_consumed.x == 1.0f);
    CMP_TEST_ASSERT(parent_b.last_child_consumed.y == -1.0f);
    CMP_TEST_ASSERT(consumed.x == 3.0f);
    CMP_TEST_ASSERT(consumed.y == -1.0f);
    CMP_TEST_ASSERT(remaining.x == 1.0f);
    CMP_TEST_ASSERT(remaining.y == -1.0f);
  }

  {
    TestScrollParent state;
    CMPScrollParent parent;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[1];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta child_consumed;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&state));

    memset(&vtable, 0, sizeof(vtable));

    CMP_TEST_OK(cmp_scroll_parent_init(&parent, &state, &vtable));
    parents[0] = &parent;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 1));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 4.0f, -2.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&child_consumed, 1.0f, -1.0f));
    CMP_TEST_OK(cmp_scroll_chain_post_scroll(&chain, &delta, &child_consumed,
                                             &consumed, &remaining));

    CMP_TEST_ASSERT(state.post_calls == 0);
    CMP_TEST_ASSERT(consumed.x == 0.0f);
    CMP_TEST_ASSERT(consumed.y == 0.0f);
    CMP_TEST_ASSERT(remaining.x == 4.0f);
    CMP_TEST_ASSERT(remaining.y == -2.0f);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    CMPScrollParent scroll_parent_a;
    CMPScrollParent scroll_parent_b;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[2];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta child_consumed;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&parent_a));
    CMP_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.post_fail = 1;

    memset(&vtable, 0, sizeof(vtable));
    vtable.post_scroll = test_scroll_parent_post;

    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 2));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 4.0f, -2.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&child_consumed, 1.0f, -1.0f));
    CMP_TEST_EXPECT(cmp_scroll_chain_post_scroll(
                        &chain, &delta, &child_consumed, &consumed, &remaining),
                    CMP_ERR_UNKNOWN);

    CMP_TEST_ASSERT(parent_a.post_calls == 1);
    CMP_TEST_ASSERT(parent_b.post_calls == 0);
  }

  {
    TestScrollParent parent_a;
    TestScrollParent parent_b;
    CMPScrollParent scroll_parent_a;
    CMPScrollParent scroll_parent_b;
    CMPScrollParentVTable vtable;
    CMPScrollParent *parents[2];
    CMPScrollChain chain;
    CMPScrollDelta delta;
    CMPScrollDelta child_consumed;
    CMPScrollDelta consumed;
    CMPScrollDelta remaining;

    CMP_TEST_OK(test_scroll_parent_reset(&parent_a));
    CMP_TEST_OK(test_scroll_parent_reset(&parent_b));
    parent_a.post_consumed.x = 6.0f;
    parent_a.post_consumed.y = 0.0f;

    memset(&vtable, 0, sizeof(vtable));
    vtable.post_scroll = test_scroll_parent_post;

    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_a, &parent_a, &vtable));
    CMP_TEST_OK(cmp_scroll_parent_init(&scroll_parent_b, &parent_b, &vtable));
    parents[0] = &scroll_parent_a;
    parents[1] = &scroll_parent_b;
    CMP_TEST_OK(cmp_scroll_chain_init(&chain, parents, 2));

    CMP_TEST_OK(cmp_scroll_delta_init(&delta, 4.0f, -2.0f));
    CMP_TEST_OK(cmp_scroll_delta_init(&child_consumed, 1.0f, -1.0f));
    CMP_TEST_EXPECT(cmp_scroll_chain_post_scroll(
                        &chain, &delta, &child_consumed, &consumed, &remaining),
                    CMP_ERR_RANGE);

    CMP_TEST_ASSERT(parent_a.post_calls == 1);
    CMP_TEST_ASSERT(parent_b.post_calls == 0);
  }

  return 0;
}
