/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_tab_item {
  char *title;
  char *icon;
  char *route;
  int badge_value;
} cmp_tab_item_t;

struct cmp_tab_bar {
  cmp_tab_item_t *tabs;
  size_t count;
  size_t capacity;
};

int cmp_tab_bar_create(cmp_tab_bar_t **out_tab_bar) {
  struct cmp_tab_bar *ctx;
  if (!out_tab_bar)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_tab_bar), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->tabs = NULL;
  ctx->count = 0;
  ctx->capacity = 0;

  *out_tab_bar = (cmp_tab_bar_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_tab_bar_destroy(cmp_tab_bar_t *tab_bar) {
  struct cmp_tab_bar *ctx = (struct cmp_tab_bar *)tab_bar;
  size_t i;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->tabs) {
    for (i = 0; i < ctx->count; ++i) {
      if (ctx->tabs[i].title)
        CMP_FREE(ctx->tabs[i].title);
      if (ctx->tabs[i].icon)
        CMP_FREE(ctx->tabs[i].icon);
      if (ctx->tabs[i].route)
        CMP_FREE(ctx->tabs[i].route);
    }
    CMP_FREE(ctx->tabs);
  }
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_tab_bar_add_tab(cmp_tab_bar_t *tab_bar, const char *title,
                        const char *symbol_icon, const char *route_uri) {
  struct cmp_tab_bar *ctx = (struct cmp_tab_bar *)tab_bar;
  cmp_tab_item_t *new_tabs;
  size_t new_cap, len;

  if (!ctx || !title || !symbol_icon || !route_uri)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->count == ctx->capacity) {
    new_cap = ctx->capacity == 0 ? 4 : ctx->capacity * 2;
    if (CMP_MALLOC(new_cap * sizeof(cmp_tab_item_t), (void **)&new_tabs) !=
        CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (ctx->tabs) {
      memcpy(new_tabs, ctx->tabs, ctx->count * sizeof(cmp_tab_item_t));
      CMP_FREE(ctx->tabs);
    }
    ctx->tabs = new_tabs;
    ctx->capacity = new_cap;
  }

  len = strlen(title);
  if (CMP_MALLOC(len + 1, (void **)&ctx->tabs[ctx->count].title) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->tabs[ctx->count].title, len + 1, title);
#else
  strcpy(ctx->tabs[ctx->count].title, title);
#endif

  len = strlen(symbol_icon);
  if (CMP_MALLOC(len + 1, (void **)&ctx->tabs[ctx->count].icon) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->tabs[ctx->count].icon, len + 1, symbol_icon);
#else
  strcpy(ctx->tabs[ctx->count].icon, symbol_icon);
#endif

  len = strlen(route_uri);
  if (CMP_MALLOC(len + 1, (void **)&ctx->tabs[ctx->count].route) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->tabs[ctx->count].route, len + 1, route_uri);
#else
  strcpy(ctx->tabs[ctx->count].route, route_uri);
#endif

  ctx->tabs[ctx->count].badge_value = 0;
  ctx->count++;

  /* The actual HIG logic (limit to 5) would happen at the view construction
     boundary. If count > 5 on iPhone, the 5th tab automatically becomes a
     "More" navigation drill-down. */

  return CMP_SUCCESS;
}

int cmp_tab_bar_set_badge(cmp_tab_bar_t *tab_bar, size_t tab_index,
                          int badge_value) {
  struct cmp_tab_bar *ctx = (struct cmp_tab_bar *)tab_bar;
  if (!ctx || tab_index >= ctx->count)
    return CMP_ERROR_INVALID_ARG;

  ctx->tabs[tab_index].badge_value = badge_value;
  return CMP_SUCCESS;
}

int cmp_tab_bar_resolve_layout(cmp_tab_bar_t *tab_bar, float available_width,
                               cmp_tab_bar_placement_t *out_placement,
                               cmp_macos_material_t *out_material) {
  if (!tab_bar || !out_placement || !out_material)
    return CMP_ERROR_INVALID_ARG;

  if (available_width < 768.0f) {
    /* Compact Width (iPhone) */
    *out_placement = CMP_TAB_BAR_PLACEMENT_BOTTOM;
    *out_material =
        CMP_MACOS_MATERIAL_WINDOW_BACKGROUND; /* Maps to standard blur */
  } else {
    /* Regular Width (iPad/Mac) */
    *out_placement = CMP_TAB_BAR_PLACEMENT_LEADING; /* Sidebar */
    *out_material =
        CMP_MACOS_MATERIAL_BEHIND_WINDOW; /* Deep translucency on desktop */
  }
  return CMP_SUCCESS;
}

int cmp_toolbar_resolve_placement(float available_width,
                                  int *out_is_bottom_placed) {
  if (!out_is_bottom_placed)
    return CMP_ERROR_INVALID_ARG;

  /* iPhones (compact width) place toolbars at bottom edge.
     iPads/Macs integrate toolbars directly into the top Navigation Bar. */
  if (available_width < 768.0f) {
    *out_is_bottom_placed = 1;
  } else {
    *out_is_bottom_placed = 0;
  }
  return CMP_SUCCESS;
}
