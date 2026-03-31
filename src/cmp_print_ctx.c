/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_print_ctx {
  int is_printing;
  int current_page;
};

int cmp_print_ctx_create(cmp_print_ctx_t **out_ctx) {
  cmp_print_ctx_t *ctx;
  if (!out_ctx) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_print_ctx_t), (void **)&ctx) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(ctx, 0, sizeof(cmp_print_ctx_t));
  *out_ctx = ctx;
  return CMP_SUCCESS;
}

int cmp_print_ctx_destroy(cmp_print_ctx_t *ctx) {
  if (!ctx) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_print_ctx_begin_page(cmp_print_ctx_t *ctx) {
  if (!ctx) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (ctx->is_printing) {
    return CMP_ERROR_INVALID_STATE; /* Already in a page */
  }
  ctx->is_printing = 1;
  ctx->current_page++;
  return CMP_SUCCESS;
}

int cmp_print_ctx_end_page(cmp_print_ctx_t *ctx) {
  if (!ctx) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (!ctx->is_printing) {
    return CMP_ERROR_INVALID_STATE; /* Not currently in a page */
  }
  ctx->is_printing = 0;
  return CMP_SUCCESS;
}

int cmp_print_ctx_save_pdf(cmp_print_ctx_t *ctx, const char *file_path) {
  if (!ctx || !file_path) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (ctx->is_printing) {
    return CMP_ERROR_INVALID_STATE; /* Must end all pages before saving */
  }
  if (ctx->current_page == 0) {
    return CMP_ERROR_INVALID_STATE; /* Cannot save empty document */
  }
  /* Native PDF generation OS APIs would go here */
  return CMP_SUCCESS;
}
