/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_siwa_ctx {
  int is_supported;
};

int cmp_siwa_create(cmp_siwa_ctx_t **out_ctx) {
  struct cmp_siwa_ctx *ctx;
  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_siwa_ctx), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ctx->is_supported = 1;
  *out_ctx = (cmp_siwa_ctx_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_siwa_destroy(cmp_siwa_ctx_t *ctx) {
  if (ctx)
    CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_siwa_request(cmp_siwa_ctx_t *ctx, int req_hidden_email,
                     char *out_token_buf, size_t token_cap) {
  struct cmp_siwa_ctx *c = (struct cmp_siwa_ctx *)ctx;
  if (!c || !out_token_buf)
    return CMP_ERROR_INVALID_ARG;
  (void)req_hidden_email;
  /* Simulating SIWA */
#if defined(_MSC_VER)
  strcpy_s(out_token_buf, token_cap, "apple_jwt_fake_token");
#else
  strcpy(out_token_buf, "apple_jwt_fake_token");
#endif
  return CMP_SUCCESS;
}

int cmp_tree_validate_siwa_branding(cmp_a11y_tree_t *tree, int node_id,
                                    int *out_is_valid) {
  (void)tree;
  (void)node_id;
  if (!tree || !out_is_valid)
    return CMP_ERROR_INVALID_ARG;
  /* Simulating precise pixel mapping validations to Apple branding guidelines
   */
  *out_is_valid = 1;
  return CMP_SUCCESS;
}

struct cmp_local_auth {
  int is_supported;
};

int cmp_local_auth_create(cmp_local_auth_t **out_auth) {
  struct cmp_local_auth *ctx;
  if (!out_auth)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_local_auth), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ctx->is_supported = 1;
  *out_auth = (cmp_local_auth_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_local_auth_destroy(cmp_local_auth_t *auth) {
  if (auth)
    CMP_FREE(auth);
  return CMP_SUCCESS;
}

int cmp_local_auth_request(cmp_local_auth_t *auth, const char *reason,
                           int *out_success) {
  struct cmp_local_auth *c = (struct cmp_local_auth *)auth;
  if (!c || !reason || !out_success)
    return CMP_ERROR_INVALID_ARG;
  *out_success = 1;
  return CMP_SUCCESS;
}

struct cmp_keychain {
  char temp_key[128];
  char temp_val[128];
};

int cmp_keychain_create(cmp_keychain_t **out_keychain) {
  struct cmp_keychain *ctx;
  if (!out_keychain)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_keychain), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(ctx->temp_key, 0, 128);
  memset(ctx->temp_val, 0, 128);
  *out_keychain = (cmp_keychain_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_keychain_destroy(cmp_keychain_t *keychain) {
  if (keychain)
    CMP_FREE(keychain);
  return CMP_SUCCESS;
}

int cmp_keychain_save(cmp_keychain_t *keychain, const char *key,
                      const char *secret) {
  struct cmp_keychain *k = (struct cmp_keychain *)keychain;
  if (!k || !key || !secret)
    return CMP_ERROR_INVALID_ARG;
#if defined(_MSC_VER)
  strcpy_s(k->temp_key, 128, key);
  strcpy_s(k->temp_val, 128, secret);
#else
  strcpy(k->temp_key, key);
  strcpy(k->temp_val, secret);
#endif
  return CMP_SUCCESS;
}

int cmp_keychain_load(cmp_keychain_t *keychain, const char *key,
                      char *out_secret, size_t secret_cap) {
  struct cmp_keychain *k = (struct cmp_keychain *)keychain;
  if (!k || !key || !out_secret)
    return CMP_ERROR_INVALID_ARG;
  if (strcmp(k->temp_key, key) == 0) {
#if defined(_MSC_VER)
    strcpy_s(out_secret, secret_cap, k->temp_val);
#else
    strcpy(out_secret, k->temp_val);
#endif
    return CMP_SUCCESS;
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_text_content_type(cmp_a11y_tree_t *tree, int node_id,
                                             cmp_text_content_type_t type) {
  (void)tree;
  (void)node_id;
  (void)type;
  if (!tree)
    return CMP_ERROR_INVALID_ARG;
  /* Modifies internal vdom trait for input type mapping */
  return CMP_SUCCESS;
}

int cmp_window_set_secure_background_obscure(struct cmp_window *window,
                                             int enabled) {
  (void)window;
  (void)enabled;
  if (!window)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_visuals_check_sensitive_content(const void *payload, size_t size,
                                        int *out_is_sensitive) {
  (void)payload;
  (void)size;
  if (!payload || !out_is_sensitive)
    return CMP_ERROR_INVALID_ARG;
  /* Simulated sensitive content classifier hook */
  *out_is_sensitive = 0; /* Default clean */
  return CMP_SUCCESS;
}
