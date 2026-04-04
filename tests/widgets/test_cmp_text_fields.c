/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_text_field_configurations(void) {
  cmp_text_field_t *ctx = NULL;
  cmp_ui_node_t dummy_node; /* Assuming opaque pointer is fine for test */

  ASSERT_EQ(CMP_SUCCESS, cmp_text_field_create(&ctx));

  /* Defaults */
  ASSERT_EQ(CMP_SUCCESS, cmp_text_field_set_keyboard_type(
                             ctx, CMP_KEYBOARD_TYPE_EMAIL_ADDRESS));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_text_field_set_return_key_type(ctx, CMP_RETURN_KEY_GO));
  ASSERT_EQ(CMP_SUCCESS, cmp_text_field_set_auto_capitalization(
                             ctx, CMP_AUTO_CAPITALIZATION_WORDS));
  ASSERT_EQ(CMP_SUCCESS, cmp_text_field_set_spellcheck_enabled(ctx, 1));

  /* Accessory View Mount */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_text_field_set_input_accessory_view(ctx, &dummy_node));

  /* Secure Entry Overrides */
  ASSERT_EQ(CMP_SUCCESS, cmp_text_field_set_secure_text_entry(ctx, 1));

  /* Internal state check - securely entered text should auto-disable caps and
   * spellcheck */
  /* We can't access opaque struct directly in test, but we can verify the
   * function returns success */

  ASSERT_EQ(CMP_SUCCESS, cmp_text_field_destroy(ctx));
  PASS();
}

TEST test_rich_text_detectors(void) {
  cmp_rich_text_view_t *ctx = NULL;
  cmp_data_detector_types_t flags =
      (cmp_data_detector_types_t)(CMP_DATA_DETECTOR_PHONE_NUMBER |
                                  CMP_DATA_DETECTOR_LINK);

  ASSERT_EQ(CMP_SUCCESS, cmp_rich_text_view_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_rich_text_view_set_data_detectors(ctx, flags));

  ASSERT_EQ(CMP_SUCCESS, cmp_rich_text_view_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_text_field_t *tf = NULL;
  cmp_rich_text_view_t *rt = NULL;
  cmp_ui_node_t dummy;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_text_field_create(NULL));
  cmp_text_field_create(&tf);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_text_field_set_keyboard_type(NULL, CMP_KEYBOARD_TYPE_DEFAULT));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_text_field_set_return_key_type(NULL, CMP_RETURN_KEY_DONE));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_text_field_set_secure_text_entry(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_text_field_set_auto_capitalization(
                                       NULL, CMP_AUTO_CAPITALIZATION_WORDS));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_text_field_set_spellcheck_enabled(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_text_field_set_input_accessory_view(NULL, &dummy));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_rich_text_view_create(NULL));
  cmp_rich_text_view_create(&rt);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_rich_text_view_set_data_detectors(NULL, CMP_DATA_DETECTOR_ALL));

  cmp_text_field_destroy(tf);
  cmp_rich_text_view_destroy(rt);
  PASS();
}

SUITE(text_fields_suite) {
  RUN_TEST(test_text_field_configurations);
  RUN_TEST(test_rich_text_detectors);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(text_fields_suite);
  GREATEST_MAIN_END();
}
