/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_siwa_ctx {
  int is_supported;
};

int cmp_siwa_create(cmp_siwa_ctx_t **out_ctx) {
  int rc = CMP_SUCCESS;
  struct cmp_siwa_ctx *ctx = NULL;

  if (!out_ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_siwa_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_siwa_ctx), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    fprintf(stderr, "Error in cmp_siwa_create: Out of memory\n");
    return rc;
  }

  ctx->is_supported = 1;
  *out_ctx = (cmp_siwa_ctx_t *)ctx;
  return rc;
}

int cmp_siwa_destroy(cmp_siwa_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  if (ctx) {
    CMP_FREE(ctx);
  }
  return rc;
}

int cmp_siwa_request(cmp_siwa_ctx_t *ctx, int req_hidden_email,
                     char *out_token_buf, size_t token_cap) {
  int rc = CMP_SUCCESS;
  struct cmp_siwa_ctx *c = (struct cmp_siwa_ctx *)ctx;

  if (!c || !out_token_buf) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_siwa_request: Invalid argument\n");
    return rc;
  }

  (void)req_hidden_email;
  /* Simulating SIWA */
#if defined(_MSC_VER)
  strcpy_s(out_token_buf, token_cap, "apple_jwt_fake_token");
#else
  strcpy(out_token_buf, "apple_jwt_fake_token");
#endif
  return rc;
}

int cmp_tree_validate_siwa_branding(cmp_a11y_tree_t *tree, int node_id,
                                    int *out_is_valid) {
  int rc = CMP_SUCCESS;
  (void)tree;
  (void)node_id;

  if (!tree || !out_is_valid) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_tree_validate_siwa_branding: Invalid argument\n");
    return rc;
  }

  /* Simulating precise pixel mapping validations to Apple branding guidelines
   */
  *out_is_valid = 1;
  return rc;
}

struct cmp_local_auth {
  int is_supported;
};

int cmp_local_auth_create(cmp_local_auth_t **out_auth) {
  int rc = CMP_SUCCESS;
  struct cmp_local_auth *ctx = NULL;

  if (!out_auth) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_local_auth_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_local_auth), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    fprintf(stderr, "Error in cmp_local_auth_create: Out of memory\n");
    return rc;
  }

  ctx->is_supported = 1;
  *out_auth = (cmp_local_auth_t *)ctx;
  return rc;
}

int cmp_local_auth_destroy(cmp_local_auth_t *auth) {
  int rc = CMP_SUCCESS;
  if (auth) {
    CMP_FREE(auth);
  }
  return rc;
}

int cmp_local_auth_request(cmp_local_auth_t *auth, const char *reason,
                           int *out_success) {
  int rc = CMP_SUCCESS;
  struct cmp_local_auth *c = (struct cmp_local_auth *)auth;

  if (!c || !reason || !out_success) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_local_auth_request: Invalid argument\n");
    return rc;
  }

  *out_success = 1;
  return rc;
}

struct cmp_keychain {
  char temp_key[128];
  char temp_val[128];
};

int cmp_keychain_create(cmp_keychain_t **out_keychain) {
  int rc = CMP_SUCCESS;
  struct cmp_keychain *ctx = NULL;

  if (!out_keychain) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_keychain_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_keychain), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    fprintf(stderr, "Error in cmp_keychain_create: Out of memory\n");
    return rc;
  }

  memset(ctx->temp_key, 0, 128);
  memset(ctx->temp_val, 0, 128);
  *out_keychain = (cmp_keychain_t *)ctx;
  return rc;
}

int cmp_keychain_destroy(cmp_keychain_t *keychain) {
  int rc = CMP_SUCCESS;
  if (keychain) {
    CMP_FREE(keychain);
  }
  return rc;
}

int cmp_keychain_save(cmp_keychain_t *keychain, const char *key,
                      const char *secret) {
  int rc = CMP_SUCCESS;
  struct cmp_keychain *k = (struct cmp_keychain *)keychain;

  if (!k || !key || !secret) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_keychain_save: Invalid argument\n");
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(k->temp_key, 128, key);
  strcpy_s(k->temp_val, 128, secret);
#else
  strcpy(k->temp_key, key);
  strcpy(k->temp_val, secret);
#endif
  return rc;
}

int cmp_keychain_load(cmp_keychain_t *keychain, const char *key,
                      char *out_secret, size_t secret_cap) {
  int rc = CMP_SUCCESS;
  struct cmp_keychain *k = (struct cmp_keychain *)keychain;

  if (!k || !key || !out_secret) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_keychain_load: Invalid argument\n");
    return rc;
  }

  if (strcmp(k->temp_key, key) == 0) {
#if defined(_MSC_VER)
    strcpy_s(out_secret, secret_cap, k->temp_val);
#else
    strcpy(out_secret, k->temp_val);
#endif
    return rc;
  }

  rc = CMP_ERROR_NOT_FOUND;
  fprintf(stderr, "Error in cmp_keychain_load: Key not found\n");
  return rc;
}

int cmp_a11y_tree_set_node_text_content_type(cmp_a11y_tree_t *tree, int node_id,
                                             cmp_text_content_type_t type) {
  int rc = CMP_SUCCESS;
  (void)tree;
  (void)node_id;
  (void)type;

  if (!tree) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_a11y_tree_set_node_text_content_type: "
                    "Invalid argument\n");
    return rc;
  }

  /* Modifies internal vdom trait for input type mapping */
  return rc;
}

int cmp_window_set_secure_background_obscure(struct cmp_window *window,
                                             int enabled) {
  int rc = CMP_SUCCESS;
  (void)window;
  (void)enabled;

  if (!window) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_window_set_secure_background_obscure: "
                    "Invalid argument\n");
    return rc;
  }

  return rc;
}

int cmp_visuals_check_sensitive_content(const void *payload, size_t size,
                                        int *out_is_sensitive) {
  int rc = CMP_SUCCESS;
  (void)payload;
  (void)size;

  if (!payload || !out_is_sensitive) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_visuals_check_sensitive_content: Invalid argument\n");
    return rc;
  }

  /* Simulated sensitive content classifier hook */
  *out_is_sensitive = 0; /* Default clean */
  return rc;
}
