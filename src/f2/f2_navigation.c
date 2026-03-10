/* clang-format off */
#include "f2/f2_navigation.h"
#include "cmpc/cmp_core.h"
#include <string.h>
/* clang-format on */

/* -------------------------------------------------------------------------- */
/* NavigationView */

CMP_API int CMP_CALL f2_navigation_view_style_init(F2NavViewStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  style->mode = F2_NAV_VIEW_MODE_LEFT;
  style->item_height = 36.0f;
  style->compact_width = 48.0f;
  style->expanded_width = 240.0f;

  style->background_color = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
  style->item_bg_normal = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f};
  style->item_bg_hover = (CMPColor){0.90f, 0.90f, 0.90f, 1.0f};
  style->item_bg_pressed = (CMPColor){0.85f, 0.85f, 0.85f, 1.0f};
  style->item_bg_selected = (CMPColor){0.90f, 0.90f, 0.90f, 1.0f};

  style->text_normal = (CMPColor){0.1f, 0.1f, 0.1f, 1.0f};
  style->text_selected = (CMPColor){0.0f, 0.0f, 0.0f, 1.0f};
  style->indicator_color = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f};

  style->text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->text_style.size_px = 14;
  style->text_style.weight = 400;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_navigation_view_init(F2NavigationView *nav,
                                             const F2NavViewStyle *style,
                                             const F2NavItem *items,
                                             cmp_usize count) {
  if (!nav || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (count > 0 && !items) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(nav, 0, sizeof(*nav));
  nav->style = *style;
  nav->items = items;
  nav->item_count = count;
  nav->selected_index = F2_NAV_INVALID_INDEX;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_navigation_view_set_selected(F2NavigationView *nav,
                                                     cmp_usize index) {
  if (!nav)
    return CMP_ERR_INVALID_ARGUMENT;
  nav->selected_index = index;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_navigation_view_set_on_select(
    F2NavigationView *nav, F2NavViewOnSelect on_select, void *ctx) {
  if (!nav)
    return CMP_ERR_INVALID_ARGUMENT;
  nav->on_select = on_select;
  nav->on_select_ctx = ctx;
  return CMP_OK;
}

/* -------------------------------------------------------------------------- */
/* TabList */

CMP_API int CMP_CALL f2_tab_list_style_init(F2TabListStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  style->tab_height = 40.0f;
  style->indicator_height = 2.0f;
  style->spacing = 16.0f;

  style->text_normal = (CMPColor){0.4f, 0.4f, 0.4f, 1.0f};
  style->text_selected = (CMPColor){0.0f, 0.0f, 0.0f, 1.0f};
  style->indicator_color = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f};
  style->bg_hover = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
  style->bg_pressed = (CMPColor){0.90f, 0.90f, 0.90f, 1.0f};

  style->text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->text_style.size_px = 14;
  style->text_style.weight = 400;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_tab_list_init(F2TabList *tabs,
                                      const F2TabListStyle *style,
                                      const F2NavItem *items, cmp_usize count) {
  if (!tabs || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (count > 0 && !items) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(tabs, 0, sizeof(*tabs));
  tabs->style = *style;
  tabs->items = items;
  tabs->item_count = count;
  tabs->selected_index = count > 0 ? 0 : F2_NAV_INVALID_INDEX;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_tab_list_set_selected(F2TabList *tabs,
                                              cmp_usize index) {
  if (!tabs)
    return CMP_ERR_INVALID_ARGUMENT;
  tabs->selected_index = index;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_tab_list_set_on_select(F2TabList *tabs,
                                               F2TabListOnSelect on_select,
                                               void *ctx) {
  if (!tabs)
    return CMP_ERR_INVALID_ARGUMENT;
  tabs->on_select = on_select;
  tabs->on_select_ctx = ctx;
  return CMP_OK;
}

/* -------------------------------------------------------------------------- */
/* Breadcrumb */

CMP_API int CMP_CALL f2_breadcrumb_style_init(F2BreadcrumbStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  style->item_spacing = 8.0f;
  style->text_normal = (CMPColor){0.4f, 0.4f, 0.4f, 1.0f};
  style->text_current = (CMPColor){0.0f, 0.0f, 0.0f, 1.0f};
  style->chevron_color = (CMPColor){0.6f, 0.6f, 0.6f, 1.0f};
  style->bg_hover = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};

  style->text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->text_style.size_px = 14;
  style->text_style.weight = 400;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_breadcrumb_init(F2Breadcrumb *nav,
                                        const F2BreadcrumbStyle *style,
                                        const F2NavItem *items,
                                        cmp_usize count) {
  if (!nav || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (count > 0 && !items) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(nav, 0, sizeof(*nav));
  nav->style = *style;
  nav->items = items;
  nav->item_count = count;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_breadcrumb_set_on_select(F2Breadcrumb *nav,
                                                 F2BreadcrumbOnSelect on_select,
                                                 void *ctx) {
  if (!nav)
    return CMP_ERR_INVALID_ARGUMENT;
  nav->on_select = on_select;
  nav->on_select_ctx = ctx;
  return CMP_OK;
}
