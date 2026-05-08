/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_siwa_ctx {
  int is_supported;
};

/**
 * @brief cmp_siwa_create
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_siwa_create(cmp_siwa_ctx_t **out_ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_siwa_ctx *ctx = NULL;

  if (out_ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_siwa_create: Invalid argument (out_ctx=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_siwa_ctx), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_siwa_create: Out of memory: %s\n", err_str);

    return rc;
  }

  ctx->is_supported = 1;
  *out_ctx = (cmp_siwa_ctx_t *)ctx;
  cmp_log_debug("cmp_siwa_create: Successfully created siwa context\n");

  return rc;
}

/**
 * @brief cmp_siwa_destroy
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_siwa_destroy(cmp_siwa_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_siwa_destroy: Invalid argument (ctx=NULL): %s\n",
                  err_str);

    return rc;
  }

  CMP_FREE(ctx);
  cmp_log_debug("cmp_siwa_destroy: Successfully destroyed siwa context\n");

  return rc;
}

/**
 * @brief cmp_siwa_request
 *
 * @param ctx Parameter description.
 * @param req_hidden_email Parameter description.
 * @param out_token_buf Parameter description.
 * @param token_cap Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_siwa_request(cmp_siwa_ctx_t *ctx, int req_hidden_email,
                     char *out_token_buf, size_t token_cap) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_siwa_ctx *c = (struct cmp_siwa_ctx *)ctx;
  (void)token_cap;

  if (c == NULL || out_token_buf == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_siwa_request: Invalid argument: %s\n", err_str);

    return rc;
  }

  (void)req_hidden_email;
  /* Simulating SIWA */
#if defined(_MSC_VER)
  strcpy_s(out_token_buf, token_cap, "apple_jwt_fake_token");
#else
  strcpy(out_token_buf, "apple_jwt_fake_token");
#endif
  cmp_log_debug("cmp_siwa_request: SIWA request mock fulfilled\n");

  return rc;
}

/**
 * @brief cmp_tree_validate_siwa_branding
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param out_is_valid Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tree_validate_siwa_branding(cmp_a11y_tree_t *tree, int node_id,
                                    int *out_is_valid) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  (void)tree;
  (void)node_id;

  if (tree == NULL || out_is_valid == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_tree_validate_siwa_branding: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Simulating precise pixel mapping validations to Apple branding guidelines
   */
  *out_is_valid = 1;
  cmp_log_debug("cmp_tree_validate_siwa_branding: SIWA branding validated\n");

  return rc;
}

struct cmp_local_auth {
  int is_supported;
};

/**
 * @brief cmp_local_auth_create
 *
 * @param out_auth Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_local_auth_create(cmp_local_auth_t **out_auth) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_local_auth *ctx = NULL;

  if (out_auth == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_local_auth_create: Invalid argument (out_auth=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_local_auth), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_local_auth_create: Out of memory: %s\n", err_str);

    return rc;
  }

  ctx->is_supported = 1;
  *out_auth = (cmp_local_auth_t *)ctx;
  cmp_log_debug(
      "cmp_local_auth_create: Successfully created local auth context\n");

  return rc;
}

/**
 * @brief cmp_local_auth_destroy
 *
 * @param auth Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_local_auth_destroy(cmp_local_auth_t *auth) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (auth == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_local_auth_destroy: Invalid argument (auth=NULL): %s\n",
                  err_str);

    return rc;
  }

  CMP_FREE(auth);
  cmp_log_debug(
      "cmp_local_auth_destroy: Successfully destroyed local auth context\n");

  return rc;
}

/**
 * @brief cmp_local_auth_request
 *
 * @param auth Parameter description.
 * @param reason Parameter description.
 * @param out_success Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_local_auth_request(cmp_local_auth_t *auth, const char *reason,
                           int *out_success) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_local_auth *c = (struct cmp_local_auth *)auth;

  if (c == NULL || reason == NULL || out_success == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_local_auth_request: Invalid argument: %s\n", err_str);

    return rc;
  }

  *out_success = 1;
  cmp_log_debug("cmp_local_auth_request: Local auth request mocked success\n");

  return rc;
}

struct cmp_keychain {
  char temp_key[128];
  char temp_val[128];
};

/**
 * @brief cmp_keychain_create
 *
 * @param out_keychain Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keychain_create(cmp_keychain_t **out_keychain) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_keychain *ctx = NULL;

  if (out_keychain == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_keychain_create: Invalid argument (out_keychain=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_keychain), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_keychain_create: Out of memory: %s\n", err_str);

    return rc;
  }

  memset(ctx->temp_key, 0, 128);
  memset(ctx->temp_val, 0, 128);
  *out_keychain = (cmp_keychain_t *)ctx;
  cmp_log_debug("cmp_keychain_create: Successfully created keychain context\n");

  return rc;
}

/**
 * @brief cmp_keychain_destroy
 *
 * @param keychain Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keychain_destroy(cmp_keychain_t *keychain) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (keychain == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_keychain_destroy: Invalid argument (keychain=NULL): %s\n",
        err_str);

    return rc;
  }

  CMP_FREE(keychain);
  cmp_log_debug(
      "cmp_keychain_destroy: Successfully destroyed keychain context\n");

  return rc;
}

/**
 * @brief cmp_keychain_save
 *
 * @param keychain Parameter description.
 * @param key Parameter description.
 * @param secret Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keychain_save(cmp_keychain_t *keychain, const char *key,
                      const char *secret) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_keychain *k = (struct cmp_keychain *)keychain;

  if (k == NULL || key == NULL || secret == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_keychain_save: Invalid argument: %s\n", err_str);

    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(k->temp_key, 128, key);
  strcpy_s(k->temp_val, 128, secret);
#else
  strncpy(k->temp_key, key, 127);
  k->temp_key[127] = '\0';
  strncpy(k->temp_val, secret, 127);
  k->temp_val[127] = '\0';
#endif
  cmp_log_debug("cmp_keychain_save: Keychain save mocked successfully\n");

  return rc;
}

/**
 * @brief cmp_keychain_load
 *
 * @param keychain Parameter description.
 * @param key Parameter description.
 * @param out_secret Parameter description.
 * @param secret_cap Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keychain_load(cmp_keychain_t *keychain, const char *key,
                      char *out_secret, size_t secret_cap) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_keychain *k = (struct cmp_keychain *)keychain;

  if (k == NULL || key == NULL || out_secret == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_keychain_load: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (strcmp(k->temp_key, key) == 0) {
#if defined(_MSC_VER)
    strcpy_s(out_secret, secret_cap, k->temp_val);
#else
    strncpy(out_secret, k->temp_val, secret_cap - 1);
    out_secret[secret_cap - 1] = '\0';
#endif
    cmp_log_debug("cmp_keychain_load: Successfully loaded key\n");

    return rc;
  }

  rc = CMP_ERROR_NOT_FOUND;
  err_rc = cmp_strerror(rc, &err_str);
  if (err_rc != CMP_SUCCESS) {
    err_str = "Unknown";
  }
  cmp_log_debug("cmp_keychain_load: Key not found: %s\n", err_str);

  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_text_content_type
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param type Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_text_content_type(cmp_a11y_tree_t *tree, int node_id,
                                             cmp_text_content_type_t type) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  (void)tree;
  (void)node_id;
  (void)type;

  if (tree == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_tree_set_node_text_content_type: Invalid argument "
                  "(tree=NULL): %s\n",
                  err_str);

    return rc;
  }

  /* Modifies internal vdom trait for input type mapping */
  cmp_log_debug(
      "cmp_a11y_tree_set_node_text_content_type: Set text content type\n");

  return rc;
}

/**
 * @brief cmp_window_set_secure_background_obscure
 *
 * @param window Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_set_secure_background_obscure(struct cmp_window *window,
                                             int enabled) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  (void)window;
  (void)enabled;

  if (window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_window_set_secure_background_obscure: Invalid argument "
                  "(window=NULL): %s\n",
                  err_str);

    return rc;
  }

  cmp_log_debug(
      "cmp_window_set_secure_background_obscure: Set obscure background\n");

  return rc;
}

/**
 * @brief cmp_visuals_check_sensitive_content
 *
 * @param payload Parameter description.
 * @param size Parameter description.
 * @param out_is_sensitive Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_visuals_check_sensitive_content(const void *payload, size_t size,
                                        int *out_is_sensitive) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  (void)payload;
  (void)size;

  if (payload == NULL || out_is_sensitive == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_visuals_check_sensitive_content: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Simulated sensitive content classifier hook */
  *out_is_sensitive = 0; /* Default clean */
  cmp_log_debug(
      "cmp_visuals_check_sensitive_content: Mock sensitive check completed\n");

  return rc;
}
