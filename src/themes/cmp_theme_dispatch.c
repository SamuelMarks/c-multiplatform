/* clang-format off */
#include "cmp.h"
#include "themes/cmp_theme_core.h"
#include "themes/cmp_cupertino.h"
#include "themes/cmp_fluent2.h"
#include "themes/cmp_material3.h"
#include "themes/cmp_unstyled.h"
/* clang-format on */

/**
 * @brief Resolves the appropriate VTable for a given UI node.
 * @param node The UI node to evaluate.
 * @return A pointer to the correct theme VTable.
 */
static const cmp_theme_vtable_t *g_default_theme_vtable = NULL;

/**
 * @brief cmp_theme_set_default_vtable
 *
 * @param vtable Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_EXEMPT(CMP_API void cmp_theme_set_default_vtable(
    const cmp_theme_vtable_t *vtable)) {
  g_default_theme_vtable = vtable;
}
#ifndef CMP_THEME_MODE_SINGLE_STATIC
/**
 * @brief cmp_resolve_vtable
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_resolve_vtable(const cmp_ui_node_t *node,
                       const cmp_theme_vtable_t **out_vtable) {
  int rc;
  rc = 0;
  if (!out_vtable)
    return CMP_ERROR_INVALID_ARG;

  if (!node) {
    if (g_default_theme_vtable) {
      *out_vtable = g_default_theme_vtable;
      return 0;
    }
    rc = cmp_theme_get_unstyled_vtable(out_vtable); /* Fallback */
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Check local widget override first */
  switch (node->design_language_override) {
  case 1:
    return cmp_theme_get_material3_vtable(out_vtable);
  case 2:
    return cmp_theme_get_fluent2_vtable(out_vtable);
  case 3:
    return cmp_theme_get_cupertino_vtable(out_vtable);
  case 4:
    return cmp_theme_get_unstyled_vtable(out_vtable);
  case 0: /* Inherit: traverse up the tree or check global context */
  default:
    break;
  }

  /* Traverse up the tree to find an inherited override */
  {
    const cmp_ui_node_t *current = node->parent;
    while (current) {
      switch (current->design_language_override) {
      case 1:
        return cmp_theme_get_material3_vtable(out_vtable);
      case 2:
        return cmp_theme_get_fluent2_vtable(out_vtable);
      case 3:
        return cmp_theme_get_cupertino_vtable(out_vtable);
      case 4:
        return cmp_theme_get_unstyled_vtable(out_vtable);
      case 0:
      default:
        current = current->parent;
        break;
      }
    }
  }

  if (g_default_theme_vtable) {
    *out_vtable = g_default_theme_vtable;
    return 0;
  }

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return cmp_theme_get_unstyled_vtable(out_vtable);
}

/**
 * @brief cmp_theme_create
 *
 * @param out_theme Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_theme_create(cmp_theme_t **out_theme) {
  int rc;
  rc = 0;
  cmp_theme_t *theme;
  if (!out_theme)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_theme_t), (void **)&theme) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(theme, 0, sizeof(cmp_theme_t));
  *out_theme = theme;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_theme_destroy
 *
 * @param theme Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_theme_destroy(cmp_theme_t *theme) {
  int rc;
  rc = 0;
  if (!theme)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(theme);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ffi_get_material3_vtable
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API
cmp_ffi_get_material3_vtable(const cmp_theme_vtable_t **out_vtable) {
  int rc;
  rc = 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return cmp_theme_get_material3_vtable(out_vtable);
}

/**
 * @brief cmp_ffi_get_fluent2_vtable
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_ffi_get_fluent2_vtable(const cmp_theme_vtable_t **out_vtable) {
  int rc;
  rc = 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return cmp_theme_get_fluent2_vtable(out_vtable);
}

/**
 * @brief cmp_ffi_get_cupertino_vtable
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API
cmp_ffi_get_cupertino_vtable(const cmp_theme_vtable_t **out_vtable) {
  int rc;
  rc = 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return cmp_theme_get_cupertino_vtable(out_vtable);
}

/**
 * @brief cmp_ffi_get_unstyled_vtable
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_ffi_get_unstyled_vtable(const cmp_theme_vtable_t **out_vtable) {
  int rc;
  rc = 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return cmp_theme_get_unstyled_vtable(out_vtable);
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */
