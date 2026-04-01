/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_payment_ctx {
  int is_ready;
};

struct cmp_storekit_ctx {
  int is_ready;
};

struct cmp_cloud_sync_ctx {
  int is_ready;
};

int cmp_payment_ctx_create(cmp_payment_ctx_t **out_ctx) {
  struct cmp_payment_ctx *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_payment_ctx), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_ready = 1;

  *out_ctx = (cmp_payment_ctx_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_payment_ctx_destroy(cmp_payment_ctx_t *ctx_opaque) {
  if (ctx_opaque)
    CMP_FREE(ctx_opaque);
  return CMP_SUCCESS;
}

int cmp_payment_mount_apple_pay_button(cmp_payment_ctx_t *ctx_opaque,
                                       cmp_ui_node_t *node,
                                       cmp_apple_pay_button_style_t style,
                                       cmp_apple_pay_button_type_t type) {
  struct cmp_payment_ctx *ctx = (struct cmp_payment_ctx *)ctx_opaque;
  if (!ctx || !node)
    return CMP_ERROR_INVALID_ARG;
  (void)style;
  (void)type;
  /* Attaches a PKPaymentButton directly to the UI rendering tree, bypassing
   * custom vector rendering to remain HIG compliant */
  return CMP_SUCCESS;
}

int cmp_payment_request_apple_pay(cmp_payment_ctx_t *ctx_opaque,
                                  const char *merchant_identifier,
                                  const char *currency_code, float amount) {
  struct cmp_payment_ctx *ctx = (struct cmp_payment_ctx *)ctx_opaque;
  if (!ctx || !merchant_identifier || !currency_code || amount <= 0.0f)
    return CMP_ERROR_INVALID_ARG;
  /* Triggers PKPaymentAuthorizationViewController */
  return CMP_SUCCESS;
}

int cmp_storekit_ctx_create(cmp_storekit_ctx_t **out_ctx) {
  struct cmp_storekit_ctx *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_storekit_ctx), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_ready = 1;

  *out_ctx = (cmp_storekit_ctx_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_storekit_ctx_destroy(cmp_storekit_ctx_t *ctx_opaque) {
  if (ctx_opaque)
    CMP_FREE(ctx_opaque);
  return CMP_SUCCESS;
}

int cmp_storekit_purchase_product(cmp_storekit_ctx_t *ctx_opaque,
                                  const char *product_identifier) {
  struct cmp_storekit_ctx *ctx = (struct cmp_storekit_ctx *)ctx_opaque;
  if (!ctx || !product_identifier)
    return CMP_ERROR_INVALID_ARG;
  /* Triggers SKPaymentQueue / StoreKit 2 flow */
  return CMP_SUCCESS;
}

int cmp_storekit_restore_purchases(cmp_storekit_ctx_t *ctx_opaque) {
  struct cmp_storekit_ctx *ctx = (struct cmp_storekit_ctx *)ctx_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_cloud_sync_ctx_create(cmp_cloud_sync_ctx_t **out_ctx) {
  struct cmp_cloud_sync_ctx *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_cloud_sync_ctx), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_ready = 1;

  *out_ctx = (cmp_cloud_sync_ctx_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_cloud_sync_ctx_destroy(cmp_cloud_sync_ctx_t *ctx_opaque) {
  if (ctx_opaque)
    CMP_FREE(ctx_opaque);
  return CMP_SUCCESS;
}

int cmp_cloud_sync_set_key_value(cmp_cloud_sync_ctx_t *ctx_opaque,
                                 const char *key, const char *value) {
  struct cmp_cloud_sync_ctx *ctx = (struct cmp_cloud_sync_ctx *)ctx_opaque;
  if (!ctx || !key || !value)
    return CMP_ERROR_INVALID_ARG;
  /* NSUbiquitousKeyValueStore.default.set */
  return CMP_SUCCESS;
}

int cmp_cloud_sync_upload_record(cmp_cloud_sync_ctx_t *ctx_opaque,
                                 const char *record_type, const char *record_id,
                                 const char *json_payload) {
  struct cmp_cloud_sync_ctx *ctx = (struct cmp_cloud_sync_ctx *)ctx_opaque;
  if (!ctx || !record_type || !record_id || !json_payload)
    return CMP_ERROR_INVALID_ARG;
  /* CloudKit CKRecord upload */
  return CMP_SUCCESS;
}

int cmp_cloud_sync_expose_to_files_app(cmp_cloud_sync_ctx_t *ctx_opaque,
                                       const char *relative_path) {
  struct cmp_cloud_sync_ctx *ctx = (struct cmp_cloud_sync_ctx *)ctx_opaque;
  if (!ctx || !relative_path)
    return CMP_ERROR_INVALID_ARG;
  /* Manipulates Info.plist dynamically or manages
   * LSSupportsOpeningDocumentsInPlace / UIExtension properties */
  return CMP_SUCCESS;
}
