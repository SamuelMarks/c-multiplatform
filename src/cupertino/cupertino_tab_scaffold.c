/**
 * @file cupertino_tab_scaffold.c
 * @brief Cupertino Tab Scaffold and Controller implementation conforming to
 * Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_tab_scaffold.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_tab_scaffold_mock_controller_destroy_fail = 0;
int g_cupertino_tab_scaffold_mock_set_selected_fail = 0;

static ui_error_t
mock_cupertino_tab_bar_set_selected(struct cupertino_tab_bar *tab_bar,
                                    size_t selected_index) {
  if (g_cupertino_tab_scaffold_mock_set_selected_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return cupertino_tab_bar_set_selected(tab_bar, selected_index);
}
#undef cupertino_tab_bar_set_selected
/** @cond */
#define cupertino_tab_bar_set_selected mock_cupertino_tab_bar_set_selected
/** @endcond */
#endif

ui_error_t cupertino_tab_controller_create(
    size_t initial_index, size_t tab_count,
    struct cupertino_tab_controller **out_controller) {
  struct cupertino_tab_controller *controller;

  if (!out_controller || tab_count == 0 ||
      tab_count > CUPERTINO_TAB_BAR_MAX_ITEMS || initial_index >= tab_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  controller = (struct cupertino_tab_controller *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_tab_controller));
  if (!controller) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  controller->selected_index = initial_index;
  controller->tab_count = tab_count;

  *out_controller = controller;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_controller_destroy(struct cupertino_tab_controller *controller) {
  if (!controller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_tab_scaffold_mock_controller_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  C_MULTIPLATFORM_FREE(controller);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_controller_set_index(struct cupertino_tab_controller *controller,
                                   size_t index) {
  if (!controller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (index >= controller->tab_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  controller->selected_index = index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_controller_get_index(
    const struct cupertino_tab_controller *controller, size_t *out_index) {
  if (!controller || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = controller->selected_index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_controller_get_tab_count(
    const struct cupertino_tab_controller *controller, size_t *out_count) {
  if (!controller || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = controller->tab_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_scaffold_create(
    struct ui_engine *engine,
    const struct cupertino_tab_scaffold_descriptor *desc,
    struct cupertino_tab_scaffold **out_scaffold) {
  struct cupertino_tab_scaffold *scaffold;
  ui_error_t rc;

  if (!engine || !desc || !out_scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (desc->initial_index >= CUPERTINO_TAB_BAR_MAX_ITEMS) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold = (struct cupertino_tab_scaffold *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_tab_scaffold));
  if (!scaffold) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(scaffold, 0, sizeof(*scaffold));
  scaffold->bar_hidden = desc->bar_hidden ? 1 : 0;
  scaffold->is_dark = desc->is_dark ? 1 : 0;

  rc = cupertino_tab_controller_create(
      desc->initial_index, CUPERTINO_TAB_BAR_MAX_ITEMS, &scaffold->controller);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  scaffold->owns_controller = 1;

  *out_scaffold = scaffold;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_scaffold_destroy(struct cupertino_tab_scaffold *scaffold) {
  ui_error_t rc;

  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (scaffold->owns_controller && scaffold->controller) {
    rc = cupertino_tab_controller_destroy(scaffold->controller);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    scaffold->controller = NULL;
  }

  C_MULTIPLATFORM_FREE(scaffold);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_scaffold_set_tab_bar(struct cupertino_tab_scaffold *scaffold,
                                   struct cupertino_tab_bar *tab_bar) {
  ui_error_t rc;

  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->tab_bar = tab_bar;
  if (tab_bar && scaffold->controller) {
    rc = cupertino_tab_bar_set_selected(tab_bar,
                                        scaffold->controller->selected_index);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_scaffold_get_tab_bar(
    const struct cupertino_tab_scaffold *scaffold,
    struct cupertino_tab_bar **out_tab_bar) {
  if (!scaffold || !out_tab_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_tab_bar = scaffold->tab_bar;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_scaffold_set_controller(
    struct cupertino_tab_scaffold *scaffold,
    struct cupertino_tab_controller *controller) {
  ui_error_t rc;

  if (!scaffold || !controller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (scaffold->owns_controller && scaffold->controller) {
    rc = cupertino_tab_controller_destroy(scaffold->controller);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    scaffold->owns_controller = 0;
  }

  scaffold->controller = controller;
  scaffold->owns_controller = 0;

  if (scaffold->tab_bar) {
    rc = cupertino_tab_bar_set_selected(scaffold->tab_bar,
                                        controller->selected_index);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_scaffold_get_controller(
    const struct cupertino_tab_scaffold *scaffold,
    struct cupertino_tab_controller **out_controller) {
  if (!scaffold || !out_controller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_controller = scaffold->controller;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_scaffold_set_tab_view(struct cupertino_tab_scaffold *scaffold,
                                    size_t tab_index,
                                    struct ui_component *view) {
  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tab_index >= CUPERTINO_TAB_BAR_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  scaffold->tab_views[tab_index] = view;
  scaffold->is_tab_instantiated[tab_index] = (view != NULL) ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_scaffold_get_tab_view(
    const struct cupertino_tab_scaffold *scaffold, size_t tab_index,
    struct ui_component **out_view) {
  if (!scaffold || !out_view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tab_index >= CUPERTINO_TAB_BAR_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  *out_view = scaffold->tab_views[tab_index];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_scaffold_get_active_view(
    const struct cupertino_tab_scaffold *scaffold,
    struct ui_component **out_view) {
  size_t idx;

  if (!scaffold || !out_view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!scaffold->controller) {
    *out_view = NULL;
    return UI_ERROR_NONE;
  }

  idx = scaffold->controller->selected_index;
  if (idx < CUPERTINO_TAB_BAR_MAX_ITEMS) {
    *out_view = scaffold->tab_views[idx];
  } else {
    *out_view = NULL;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_scaffold_set_bar_hidden(struct cupertino_tab_scaffold *scaffold,
                                      int hidden) {
  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  scaffold->bar_hidden = hidden ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_scaffold_get_bar_hidden(
    const struct cupertino_tab_scaffold *scaffold, int *out_hidden) {
  if (!scaffold || !out_hidden) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_hidden = scaffold->bar_hidden;
  return UI_ERROR_NONE;
}
