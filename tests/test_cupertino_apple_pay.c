/**
 * @file test_cupertino_apple_pay.c
 * @brief Unit tests for Apple Pay and Sign in with Apple HIG buttons.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_apple_pay.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_apple_pay_suite);

TEST test_apple_pay_invalid_arguments(void) {
  struct cupertino_apple_pay_descriptor desc;
  struct cupertino_apple_pay_button *btn = NULL;
  struct ui_button_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *label = NULL;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid pointers */
  rc = cupertino_apple_pay_create(NULL, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_pay_create(dummy_engine, NULL, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_pay_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range type */
  desc.type = (enum cupertino_apple_pay_type) - 1;
  rc = cupertino_apple_pay_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.type = (enum cupertino_apple_pay_type)999;
  rc = cupertino_apple_pay_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range style */
  desc.type = CUPERTINO_APPLE_PAY_BUY;
  desc.style = (enum cupertino_apple_pay_style) - 1;
  rc = cupertino_apple_pay_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.style = (enum cupertino_apple_pay_style)3;
  rc = cupertino_apple_pay_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_apple_pay_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Press start / end invalid */
  rc = cupertino_apple_pay_press_start(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_pay_press_end(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create a valid button for method invalid tests */
  desc.style = CUPERTINO_APPLE_PAY_STYLE_WHITE;
  rc = cupertino_apple_pay_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(btn != NULL);

  /* Getters invalid */
  rc = cupertino_apple_pay_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_pay_get_dimensions(btn, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_pay_get_dimensions(btn, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_apple_pay_get_label(NULL, &label);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_pay_get_label(btn, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_apple_pay_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_pay_get_base(btn, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Clean up */
  rc = cupertino_apple_pay_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_apple_pay_styles_types_and_press(void) {
  struct cupertino_apple_pay_descriptor desc;
  struct cupertino_apple_pay_button *btn = NULL;
  struct ui_button_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *label = NULL;
  float w = 0.0f;
  float h = 0.0f;
  int i;
  ui_error_t rc;

  /* Test all Apple Pay types and styles */
  for (i = 0; i < (int)CUPERTINO_APPLE_PAY_TYPE_COUNT; i++) {
    memset(&desc, 0, sizeof(desc));
    desc.type = (enum cupertino_apple_pay_type)i;
    desc.style = (enum cupertino_apple_pay_style)(i % 3);
    desc.width = 100.0f;       /* Clamps to 140 */
    desc.height = 30.0f;       /* Clamps to 44 */
    desc.corner_radius = 0.0f; /* Defaults to 4.0 */

    rc = cupertino_apple_pay_create(dummy_engine, &desc, &btn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(btn != NULL);

    rc = cupertino_apple_pay_get_dimensions(btn, &w, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(CUPERTINO_APPLE_PAY_MIN_WIDTH, w);
    ASSERT_EQ(CUPERTINO_APPLE_PAY_MIN_HEIGHT, h);

    rc = cupertino_apple_pay_get_label(btn, &label);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(label != NULL);
    ASSERT(strlen(label) > 0);

    rc = cupertino_apple_pay_get_base(btn, &base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(base != NULL);

    /* Press interactive state */
    rc = cupertino_apple_pay_press_start(btn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(1, btn->is_pressed);
    ASSERT_EQ(0.96f, btn->scale);
    ASSERT_EQ(0.70f, btn->opacity);

    rc = cupertino_apple_pay_press_end(btn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(0, btn->is_pressed);
    ASSERT_EQ(1.0f, btn->scale);
    ASSERT_EQ(1.0f, btn->opacity);

    rc = cupertino_apple_pay_destroy(btn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern const char *test_cupertino_get_apple_pay_label(
        enum cupertino_apple_pay_type type);
    extern const char *test_cupertino_get_apple_id_label(
        enum cupertino_apple_id_button_type type);

    const char *fallback_lbl;
    fallback_lbl =
        test_cupertino_get_apple_pay_label((enum cupertino_apple_pay_type)999);
    ASSERT_STR_EQ("Apple Pay", fallback_lbl);

    fallback_lbl = test_cupertino_get_apple_id_label(
        (enum cupertino_apple_id_button_type)999);
    ASSERT_STR_EQ("Sign in with Apple", fallback_lbl);
  }
#endif

  /* Test with explicit width/height/corner_radius > default */
  memset(&desc, 0, sizeof(desc));
  desc.type = CUPERTINO_APPLE_PAY_PLAIN;
  desc.style = CUPERTINO_APPLE_PAY_STYLE_WHITE_OUTLINE;
  desc.width = 250.0f;
  desc.height = 60.0f;
  desc.corner_radius = 12.0f;

  rc = cupertino_apple_pay_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(btn != NULL);
  ASSERT_EQ(250.0f, btn->width);
  ASSERT_EQ(60.0f, btn->height);
  ASSERT_EQ(12.0f, btn->corner_radius);

  /* Destroy with btn->base == NULL branch */
  base = btn->base;
  btn->base = NULL;
  rc = cupertino_apple_pay_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_button_base_destroy(base);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_apple_id_invalid_and_lifecycle(void) {
  struct cupertino_apple_id_descriptor desc;
  struct cupertino_apple_id_button *btn = NULL;
  struct ui_button_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *label = NULL;
  float w = 0.0f;
  float h = 0.0f;
  int i;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Invalid calls */
  rc = cupertino_apple_id_button_create(NULL, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_id_button_create(dummy_engine, NULL, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_id_button_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range type */
  desc.type = (enum cupertino_apple_id_button_type) - 1;
  rc = cupertino_apple_id_button_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.type = (enum cupertino_apple_id_button_type)999;
  rc = cupertino_apple_id_button_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range style */
  desc.type = CUPERTINO_APPLE_ID_SIGN_IN;
  desc.style = (enum cupertino_apple_pay_style) - 1;
  rc = cupertino_apple_id_button_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.style = (enum cupertino_apple_pay_style)3;
  rc = cupertino_apple_id_button_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_apple_id_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Press start / end invalid */
  rc = cupertino_apple_id_button_press_start(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_id_button_press_end(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create valid Apple ID button */
  desc.style = CUPERTINO_APPLE_PAY_STYLE_BLACK;
  desc.width = 100.0f;       /* Clamps to 140 */
  desc.height = 30.0f;       /* Clamps to 44 */
  desc.corner_radius = 0.0f; /* Defaults to 4 */

  rc = cupertino_apple_id_button_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(btn != NULL);

  /* Method invalid checks */
  rc = cupertino_apple_id_button_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_id_button_get_dimensions(btn, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_id_button_get_dimensions(btn, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_apple_id_button_get_label(NULL, &label);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_id_button_get_label(btn, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_apple_id_button_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_apple_id_button_get_base(btn, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_apple_id_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test all 3 Apple ID types and styles */
  for (i = 0; i < (int)CUPERTINO_APPLE_ID_TYPE_COUNT; i++) {
    memset(&desc, 0, sizeof(desc));
    desc.type = (enum cupertino_apple_id_button_type)i;
    desc.style = (enum cupertino_apple_pay_style)(i % 3);
    desc.width = 220.0f;
    desc.height = 54.0f;
    desc.corner_radius = 8.0f;

    rc = cupertino_apple_id_button_create(dummy_engine, &desc, &btn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(btn != NULL);

    rc = cupertino_apple_id_button_get_label(btn, &label);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(label != NULL);
    ASSERT(strlen(label) > 0);

    rc = cupertino_apple_id_button_get_dimensions(btn, &w, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(220.0f, w);
    ASSERT_EQ(54.0f, h);

    rc = cupertino_apple_id_button_press_start(btn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(0.96f, btn->scale);

    rc = cupertino_apple_id_button_press_end(btn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(1.0f, btn->scale);

    rc = cupertino_apple_id_button_get_base(btn, &base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(base != NULL);

    rc = cupertino_apple_id_button_destroy(btn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test Apple ID destroy with btn->base == NULL */
  memset(&desc, 0, sizeof(desc));
  desc.type = CUPERTINO_APPLE_ID_CONTINUE;
  desc.style = CUPERTINO_APPLE_PAY_STYLE_WHITE;
  rc = cupertino_apple_id_button_create(dummy_engine, &desc, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  base = btn->base;
  btn->base = NULL;
  rc = cupertino_apple_id_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_button_base_destroy(base);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_apple_pay_mock_failures(void) {
  struct cupertino_apple_pay_descriptor desc_pay;
  struct cupertino_apple_pay_button *btn_pay = NULL;
  struct cupertino_apple_id_descriptor desc_id;
  struct cupertino_apple_id_button *btn_id = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc_pay, 0, sizeof(desc_pay));
  desc_pay.type = CUPERTINO_APPLE_PAY_BUY;

  memset(&desc_id, 0, sizeof(desc_id));
  desc_id.type = CUPERTINO_APPLE_ID_SIGN_IN;

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_apple_pay_mock_base_create_fail;
    extern int g_cupertino_apple_pay_mock_base_set_text_fail;
    extern int g_cupertino_apple_pay_mock_base_destroy_fail;
    extern struct ui_button_base *g_cupertino_apple_pay_last_created_base;

    /* Apple Pay: base_create fail */
    g_cupertino_apple_pay_mock_base_create_fail = 1;
    rc = cupertino_apple_pay_create(dummy_engine, &desc_pay, &btn_pay);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_cupertino_apple_pay_mock_base_create_fail = 0;

    /* Apple Pay: set_text fail with destroy success */
    g_cupertino_apple_pay_mock_base_set_text_fail = 1;
    g_cupertino_apple_pay_mock_base_destroy_fail = 0;
    rc = cupertino_apple_pay_create(dummy_engine, &desc_pay, &btn_pay);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    /* Apple Pay: set_text fail with destroy fail */
    g_cupertino_apple_pay_mock_base_set_text_fail = 1;
    g_cupertino_apple_pay_mock_base_destroy_fail = 1;
    rc = cupertino_apple_pay_create(dummy_engine, &desc_pay, &btn_pay);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_apple_pay_mock_base_set_text_fail = 0;
    g_cupertino_apple_pay_mock_base_destroy_fail = 0;
    if (g_cupertino_apple_pay_last_created_base) {
      rc = ui_button_base_destroy(g_cupertino_apple_pay_last_created_base);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_apple_pay_last_created_base = NULL;
    }

    /* Apple Pay: destroy fail */
    rc = cupertino_apple_pay_create(dummy_engine, &desc_pay, &btn_pay);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_cupertino_apple_pay_mock_base_destroy_fail = 1;
    rc = cupertino_apple_pay_destroy(btn_pay);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_apple_pay_mock_base_destroy_fail = 0;
    rc = cupertino_apple_pay_destroy(btn_pay);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Apple ID: base_create fail */
    g_cupertino_apple_pay_mock_base_create_fail = 1;
    rc = cupertino_apple_id_button_create(dummy_engine, &desc_id, &btn_id);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_cupertino_apple_pay_mock_base_create_fail = 0;

    /* Apple ID: set_text fail with destroy success */
    g_cupertino_apple_pay_mock_base_set_text_fail = 1;
    g_cupertino_apple_pay_mock_base_destroy_fail = 0;
    rc = cupertino_apple_id_button_create(dummy_engine, &desc_id, &btn_id);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    /* Apple ID: set_text fail with destroy fail */
    g_cupertino_apple_pay_mock_base_set_text_fail = 1;
    g_cupertino_apple_pay_mock_base_destroy_fail = 1;
    rc = cupertino_apple_id_button_create(dummy_engine, &desc_id, &btn_id);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_apple_pay_mock_base_set_text_fail = 0;
    g_cupertino_apple_pay_mock_base_destroy_fail = 0;
    if (g_cupertino_apple_pay_last_created_base) {
      rc = ui_button_base_destroy(g_cupertino_apple_pay_last_created_base);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_apple_pay_last_created_base = NULL;
    }

    /* Apple ID: destroy fail */
    rc = cupertino_apple_id_button_create(dummy_engine, &desc_id, &btn_id);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_cupertino_apple_pay_mock_base_destroy_fail = 1;
    rc = cupertino_apple_id_button_destroy(btn_id);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_apple_pay_mock_base_destroy_fail = 0;
    rc = cupertino_apple_id_button_destroy(btn_id);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
#endif

  PASS();
}

TEST test_apple_pay_oom_mock(void) {
  struct cupertino_apple_pay_descriptor desc_pay;
  struct cupertino_apple_pay_button *btn_pay = NULL;
  struct cupertino_apple_id_descriptor desc_id;
  struct cupertino_apple_id_button *btn_id = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc_pay, 0, sizeof(desc_pay));
  desc_pay.type = CUPERTINO_APPLE_PAY_BUY;

  memset(&desc_id, 0, sizeof(desc_id));
  desc_id.type = CUPERTINO_APPLE_ID_SIGN_IN;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_apple_pay_create(dummy_engine, &desc_pay, &btn_pay);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, btn_pay);

  g_malloc_fail_countdown = 1;
  rc = cupertino_apple_pay_create(dummy_engine, &desc_pay, &btn_pay);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, btn_pay);

  g_malloc_fail_countdown = 0;
  rc = cupertino_apple_id_button_create(dummy_engine, &desc_id, &btn_id);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, btn_id);

  g_malloc_fail_countdown = 1;
  rc = cupertino_apple_id_button_create(dummy_engine, &desc_id, &btn_id);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, btn_id);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_apple_pay_create(NULL, &desc_pay, &btn_pay);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_apple_pay_suite) {
  RUN_TEST(test_apple_pay_invalid_arguments);
  RUN_TEST(test_apple_pay_styles_types_and_press);
  RUN_TEST(test_apple_id_invalid_and_lifecycle);
  RUN_TEST(test_apple_pay_mock_failures);
  RUN_TEST(test_apple_pay_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_apple_pay_suite);
  GREATEST_MAIN_END();
}
