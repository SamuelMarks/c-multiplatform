/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_dnd_suite);

TEST test_cmp_dnd_create_destroy(void) {
  cmp_dnd_t *dnd = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_create(&dnd));
  ASSERT_NEQ(NULL, dnd);

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_destroy(dnd));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_destroy(NULL));

  PASS();
}

TEST test_cmp_dnd_text_and_operation(void) {
  cmp_dnd_t *dnd = NULL;
  char *text = NULL;
  cmp_dnd_op_t op;

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_create(&dnd));

  /* Defaults */
  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_get_payload_text(dnd, &text));
  ASSERT_EQ(NULL, text);

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_get_operation(dnd, &op));
  ASSERT_EQ(CMP_DND_OP_NONE, op);

  /* Set text */
  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_set_payload_text(dnd, "file:///test.txt"));
  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_get_payload_text(dnd, &text));
  ASSERT_NEQ(NULL, text);
  ASSERT_STR_EQ("file:///test.txt", text);
  CMP_FREE(text);

  /* Set operation */
  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_set_operation(dnd, CMP_DND_OP_COPY));
  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_get_operation(dnd, &op));
  ASSERT_EQ(CMP_DND_OP_COPY, op);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_set_payload_text(NULL, "test"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_set_payload_text(dnd, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_get_payload_text(NULL, &text));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_get_payload_text(dnd, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dnd_set_operation(NULL, CMP_DND_OP_COPY));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_get_operation(NULL, &op));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_get_operation(dnd, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_destroy(dnd));
  PASS();
}

TEST test_dnd_lift_animation(void) {
  float scale, shadow;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_dnd_evaluate_lift_animation(0.0f, &scale, &shadow));
  ASSERT_EQ(1.0f, scale);
  ASSERT_EQ(0.0f, shadow);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_dnd_evaluate_lift_animation(0.5f, &scale, &shadow));
  ASSERT_EQ(1.025f, scale);
  ASSERT_EQ(0.2f, shadow);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_dnd_evaluate_lift_animation(1.0f, &scale, &shadow));
  ASSERT_EQ(1.05f, scale);
  ASSERT_EQ(0.4f, shadow);

  PASS();
}

TEST test_dnd_multi_item_stack(void) {
  cmp_dnd_t *ctx = NULL;
  size_t count;

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_get_stack_count(ctx, &count));
  ASSERT_EQ(0, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_add_item_to_stack(ctx, "File 1"));
  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_get_stack_count(ctx, &count));
  ASSERT_EQ(1, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_add_item_to_stack(ctx, "File 2"));
  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_get_stack_count(ctx, &count));
  ASSERT_EQ(2, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_destroy(ctx));
  PASS();
}

TEST test_dnd_drop_targets(void) {
  float scale, opacity;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_dnd_evaluate_drop_target_highlight(0, &scale, &opacity));
  ASSERT_EQ(1.0f, scale);
  ASSERT_EQ(0.0f, opacity);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_dnd_evaluate_drop_target_highlight(1, &scale, &opacity));
  ASSERT_EQ(1.02f, scale);
  ASSERT_EQ(0.15f, opacity);

  PASS();
}

TEST test_dnd_spring_loading(void) {
  int trigger;

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_evaluate_spring_loading(500.0f, &trigger));
  ASSERT_EQ(0, trigger);

  ASSERT_EQ(CMP_SUCCESS, cmp_dnd_evaluate_spring_loading(1200.0f, &trigger));
  ASSERT_EQ(1, trigger);

  PASS();
}

TEST test_dnd_hig_null_args(void) {
  cmp_dnd_t *dnd = NULL;
  float f;
  size_t count;

  cmp_dnd_create(&dnd);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dnd_evaluate_lift_animation(1.0f, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dnd_evaluate_lift_animation(1.0f, &f, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_add_item_to_stack(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_add_item_to_stack(dnd, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_get_stack_count(NULL, &count));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_get_stack_count(dnd, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dnd_evaluate_drop_target_highlight(1, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dnd_evaluate_drop_target_highlight(1, &f, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dnd_evaluate_spring_loading(1.0f, NULL));

  cmp_dnd_destroy(dnd);
  PASS();
}

SUITE(cmp_dnd_suite) {
  RUN_TEST(test_cmp_dnd_create_destroy);
  RUN_TEST(test_cmp_dnd_text_and_operation);
  RUN_TEST(test_dnd_lift_animation);
  RUN_TEST(test_dnd_multi_item_stack);
  RUN_TEST(test_dnd_drop_targets);
  RUN_TEST(test_dnd_spring_loading);
  RUN_TEST(test_dnd_hig_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_dnd_suite);
  GREATEST_MAIN_END();
}
