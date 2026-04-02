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
#ifndef CMP_THEME_MODE_SINGLE_STATIC
const cmp_theme_vtable_t *cmp_resolve_vtable(const cmp_ui_node_t *node) {
  if (!node) {
    return cmp_theme_get_unstyled_vtable(); /* Fallback */
  }

  /* Check local widget override first */
  switch (node->design_language_override) {
  case 1:
    return cmp_theme_get_material3_vtable();
  case 2:
    return cmp_theme_get_fluent2_vtable();
  case 3:
    return cmp_theme_get_cupertino_vtable();
  case 4:
    return cmp_theme_get_unstyled_vtable();
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
        return cmp_theme_get_material3_vtable();
      case 2:
        return cmp_theme_get_fluent2_vtable();
      case 3:
        return cmp_theme_get_cupertino_vtable();
      case 4:
        return cmp_theme_get_unstyled_vtable();
      case 0:
      default:
        current = current->parent;
        break;
      }
    }
  }

  /* If no override found, check global app context */
  /* TODO: We need a global context or window context to provide the default.
   * For now, default to Unstyled. */
  return cmp_theme_get_unstyled_vtable();
}

CMP_API int cmp_theme_create(cmp_theme_t **out_theme) {
  cmp_theme_t *theme;
  if (!out_theme)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_theme_t), (void **)&theme) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(theme, 0, sizeof(cmp_theme_t));
  *out_theme = theme;
  return CMP_SUCCESS;
}

CMP_API int cmp_theme_destroy(cmp_theme_t *theme) {
  if (!theme)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(theme);
  return CMP_SUCCESS;
}

CMP_API const cmp_theme_vtable_t *cmp_ffi_get_material3_vtable(void) {
  return cmp_theme_get_material3_vtable();
}

CMP_API const cmp_theme_vtable_t *cmp_ffi_get_fluent2_vtable(void) {
  return cmp_theme_get_fluent2_vtable();
}

CMP_API const cmp_theme_vtable_t *cmp_ffi_get_cupertino_vtable(void) {
  return cmp_theme_get_cupertino_vtable();
}

CMP_API const cmp_theme_vtable_t *cmp_ffi_get_unstyled_vtable(void) {
  return cmp_theme_get_unstyled_vtable();
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */
