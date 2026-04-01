/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_apple_pay_integrations(void) {
  cmp_payment_ctx_t *ctx = NULL;
  cmp_ui_node_t dummy_node;

  ASSERT_EQ(CMP_SUCCESS, cmp_payment_ctx_create(&ctx));

  /* HIG requires use of exact PKPaymentButton */
  ASSERT_EQ(CMP_SUCCESS, cmp_payment_mount_apple_pay_button(
                             ctx, &dummy_node, CMP_APPLE_PAY_BUTTON_BLACK,
                             CMP_APPLE_PAY_BUTTON_TYPE_BUY));

  /* Trigger flow */
  ASSERT_EQ(CMP_SUCCESS, cmp_payment_request_apple_pay(ctx, "merchant.com.app",
                                                       "USD", 9.99f));

  ASSERT_EQ(CMP_SUCCESS, cmp_payment_ctx_destroy(ctx));
  PASS();
}

TEST test_storekit_purchases(void) {
  cmp_storekit_ctx_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_storekit_ctx_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_storekit_purchase_product(ctx, "com.app.pro_subscription"));
  ASSERT_EQ(CMP_SUCCESS, cmp_storekit_restore_purchases(ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_storekit_ctx_destroy(ctx));
  PASS();
}

TEST test_cloud_sync_and_files(void) {
  cmp_cloud_sync_ctx_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_cloud_sync_ctx_create(&ctx));

  /* iCloud KVS */
  ASSERT_EQ(CMP_SUCCESS, cmp_cloud_sync_set_key_value(ctx, "theme", "dark"));

  /* CloudKit */
  ASSERT_EQ(CMP_SUCCESS, cmp_cloud_sync_upload_record(ctx, "SaveData", "save_1",
                                                      "\"{\"level\": 5}\""));

  /* Files App Integration */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_cloud_sync_expose_to_files_app(ctx, "Documents/Exports"));

  ASSERT_EQ(CMP_SUCCESS, cmp_cloud_sync_ctx_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_payment_ctx_t *pm = NULL;
  cmp_storekit_ctx_t *sk = NULL;
  cmp_cloud_sync_ctx_t *cl = NULL;
  cmp_ui_node_t node;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_payment_ctx_create(NULL));
  cmp_payment_ctx_create(&pm);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_payment_mount_apple_pay_button(
                                       NULL, &node, CMP_APPLE_PAY_BUTTON_BLACK,
                                       CMP_APPLE_PAY_BUTTON_TYPE_BUY));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_payment_mount_apple_pay_button(
                                       pm, NULL, CMP_APPLE_PAY_BUTTON_BLACK,
                                       CMP_APPLE_PAY_BUTTON_TYPE_BUY));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_payment_request_apple_pay(NULL, "a", "b", 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_payment_request_apple_pay(pm, NULL, "b", 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_payment_request_apple_pay(pm, "a", NULL, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_payment_request_apple_pay(pm, "a", "b",
                                          0.0f)); /* negative or zero amount */

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_storekit_ctx_create(NULL));
  cmp_storekit_ctx_create(&sk);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_storekit_purchase_product(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_storekit_purchase_product(sk, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_storekit_restore_purchases(NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cloud_sync_ctx_create(NULL));
  cmp_cloud_sync_ctx_create(&cl);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cloud_sync_set_key_value(NULL, "a", "b"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cloud_sync_set_key_value(cl, NULL, "b"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_cloud_sync_set_key_value(cl, "a", NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cloud_sync_upload_record(NULL, "a", "b", "c"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cloud_sync_upload_record(cl, NULL, "b", "c"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cloud_sync_upload_record(cl, "a", NULL, "c"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cloud_sync_upload_record(cl, "a", "b", NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cloud_sync_expose_to_files_app(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_cloud_sync_expose_to_files_app(cl, NULL));

  cmp_payment_ctx_destroy(pm);
  cmp_storekit_ctx_destroy(sk);
  cmp_cloud_sync_ctx_destroy(cl);
  PASS();
}

SUITE(transactions_hig_suite) {
  RUN_TEST(test_apple_pay_integrations);
  RUN_TEST(test_storekit_purchases);
  RUN_TEST(test_cloud_sync_and_files);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(transactions_hig_suite);
  GREATEST_MAIN_END();
}
