/**
 * @file md2_navigation_drawer.c
 * @brief Implementation of Material Design 2 Navigation Drawer component.
 */

/* clang-format off */
#include "material2/md2_navigation_drawer.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md2_nav_mock_set_on_close_fail;
int g_md2_nav_mock_set_on_close_fail = 0;
#endif

struct md2_navigation_drawer {
  struct ui_sidenav_base *base;
  enum md2_navigation_drawer_type type;
  md2_navigation_drawer_on_close_t on_close;
  void *user_data;
};

static ui_error_t on_base_close(struct ui_sidenav_base *sidenav,
                                void *user_data) {
  struct md2_navigation_drawer *drawer;

  drawer = (struct md2_navigation_drawer *)user_data;
  if (drawer->on_close != NULL) {
    return drawer->on_close(drawer, drawer->user_data);
  }

  return UI_ERROR_NONE;
}

ui_error_t
md2_navigation_drawer_create(enum md2_navigation_drawer_type type,
                             struct md2_navigation_drawer **out_drawer) {
  struct md2_navigation_drawer *drawer;
  ui_error_t rc;

  if (out_drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  drawer = (struct md2_navigation_drawer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md2_navigation_drawer));
  if (drawer == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  drawer->type = type;
  drawer->on_close = NULL;
  drawer->user_data = NULL;

  rc = ui_sidenav_base_create(&drawer->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(drawer);
    return rc;
  }

  if (type == MD2_NAVIGATION_DRAWER_STANDARD) {
    rc = ui_sidenav_base_set_mode(drawer->base, UI_SIDENAV_MODE_SIDE);
  } else {
    rc = ui_sidenav_base_set_mode(drawer->base, UI_SIDENAV_MODE_OVER);
  }
  if (rc != UI_ERROR_NONE) {
    ui_sidenav_base_destroy(drawer->base);
    C_MULTIPLATFORM_FREE(drawer);
    return rc;
  }

#ifdef UI_TEST_MOCK_ALLOC
  if (g_md2_nav_mock_set_on_close_fail) {
    rc = UI_ERROR_INVALID_ARGUMENT;
  } else {
#endif
    rc = ui_sidenav_base_set_on_close(drawer->base, on_base_close, drawer);
#ifdef UI_TEST_MOCK_ALLOC
  }
#endif

  if (rc != UI_ERROR_NONE) {
    ui_sidenav_base_destroy(drawer->base);
    C_MULTIPLATFORM_FREE(drawer);
    return rc;
  }

  *out_drawer = drawer;
  return UI_ERROR_NONE;
}

ui_error_t md2_navigation_drawer_destroy(struct md2_navigation_drawer *drawer) {
  if (drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_sidenav_base_destroy(drawer->base);
  C_MULTIPLATFORM_FREE(drawer);
  return UI_ERROR_NONE;
}

ui_error_t
md2_navigation_drawer_set_drawer_content(struct md2_navigation_drawer *drawer,
                                         struct ui_component *content) {
  if (drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_sidenav_base_set_drawer_content(drawer->base, content);
}

ui_error_t
md2_navigation_drawer_set_main_content(struct md2_navigation_drawer *drawer,
                                       struct ui_component *content) {
  if (drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_sidenav_base_set_main_content(drawer->base, content);
}

ui_error_t md2_navigation_drawer_set_open(struct md2_navigation_drawer *drawer,
                                          int is_open) {
  if (drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_sidenav_base_set_open(drawer->base, is_open);
}

ui_error_t
md2_navigation_drawer_is_open(const struct md2_navigation_drawer *drawer,
                              int *out_is_open) {
  if (drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_sidenav_base_is_open(drawer->base, out_is_open);
}

ui_error_t md2_navigation_drawer_set_overlay_director(
    struct md2_navigation_drawer *drawer,
    struct ui_overlay_director *director) {
  if (drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_sidenav_base_set_overlay_director(drawer->base, director);
}

ui_error_t
md2_navigation_drawer_set_on_close(struct md2_navigation_drawer *drawer,
                                   md2_navigation_drawer_on_close_t on_close,
                                   void *user_data) {
  if (drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  drawer->on_close = on_close;
  drawer->user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t
md2_navigation_drawer_get_component(struct md2_navigation_drawer *drawer,
                                    struct ui_component **out_component) {
  if (drawer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_sidenav_base_get_component(drawer->base, out_component);
}
