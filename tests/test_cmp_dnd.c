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

SUITE(cmp_dnd_suite) {
  RUN_TEST(test_cmp_dnd_create_destroy);
  RUN_TEST(test_cmp_dnd_text_and_operation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_dnd_suite);
  GREATEST_MAIN_END();
}
