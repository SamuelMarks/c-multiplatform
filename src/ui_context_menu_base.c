/* clang-format off */
#include "ui_context_menu_base.h"
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_context_menu_base {
  struct ui_menu_base *menu;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

/** \brief ui_error */
enum ui_error
ui_context_menu_base_create(struct ui_context_menu_base **out_menu) {
  struct ui_context_menu_base *ctx_menu;
  enum ui_error rc;

  if (!out_menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctx_menu = (struct ui_context_menu_base *)UI_MALLOC(
      sizeof(struct ui_context_menu_base));
  if (!ctx_menu) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_menu_base_create(&ctx_menu->menu);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(ctx_menu);
    return rc;
  }

  *out_menu = ctx_menu;
  return UI_ERROR_NONE;
}

enum ui_error ui_context_menu_base_destroy(struct ui_context_menu_base *menu) {
  if (menu) {
    ui_menu_base_destroy(menu->menu);
    UI_FREE(menu);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_context_menu_base_get_menu(struct ui_context_menu_base *menu,
                                            struct ui_menu_base **out_menu) {
  if (!menu || !out_menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_menu = menu->menu;
  return UI_ERROR_NONE;
}

enum ui_error ui_context_menu_base_open_at(struct ui_context_menu_base *menu,
                                           struct ui_overlay_director *director,
                                           int target_x, int target_y,
                                           int menu_width, int menu_height,
                                           int viewport_width,
                                           int viewport_height) {
  int clamped_x = target_x;
  int clamped_y = target_y;

  if (!menu || !director) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Clamp X */
  if (clamped_x + menu_width > viewport_width) {
    clamped_x = viewport_width - menu_width;
  }
  if (clamped_x < 0) {
    clamped_x = 0;
  }

  /* Clamp Y */
  if (clamped_y + menu_height > viewport_height) {
    clamped_y = viewport_height - menu_height;
  }
  if (clamped_y < 0) {
    clamped_y = 0;
  }

  return ui_menu_base_open_at(menu->menu, director, clamped_x, clamped_y);
}

/** \brief ui_error */
enum ui_error
ui_context_menu_base_bind_open(struct ui_context_menu_base *widget,
                               struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_context_menu_base_get_animating_signal(struct ui_context_menu_base *widget,
                                          struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
