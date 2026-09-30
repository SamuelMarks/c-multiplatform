/**
 * @file md3_containment.c
 * @brief Material 3 Containment, Layout, and Presentation Implementation.
 */

/* clang-format off */
#include "material3/md3_containment.h"
#include "ui_engine.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_md3_containment_mock_fail = 0;

static ui_error_t mock_accordion_base_create(struct ui_accordion_base **out) {
  if (g_md3_containment_mock_fail == 1)
    return UI_ERROR_UNKNOWN;
  return ui_accordion_base_create(out);
}
static ui_error_t mock_accordion_base_destroy(struct ui_accordion_base *b) {
  if (g_md3_containment_mock_fail == 2)
    return UI_ERROR_UNKNOWN;
  return ui_accordion_base_destroy(b);
}
static ui_error_t mock_disclosure_base_create(struct ui_disclosure_base **out) {
  if (g_md3_containment_mock_fail == 3)
    return UI_ERROR_UNKNOWN;
  return ui_disclosure_base_create(out);
}
static ui_error_t mock_disclosure_base_destroy(struct ui_disclosure_base *b) {
  if (g_md3_containment_mock_fail == 4)
    return UI_ERROR_UNKNOWN;
  return ui_disclosure_base_destroy(b);
}
static ui_error_t
mock_accordion_base_add_disclosure(struct ui_accordion_base *a,
                                   struct ui_disclosure_base *d) {
  if (g_md3_containment_mock_fail == 5)
    return UI_ERROR_UNKNOWN;
  return ui_accordion_base_add_disclosure(a, d);
}
static ui_error_t
mock_disclosure_base_set_expanded(struct ui_disclosure_base *d, int e) {
  if (g_md3_containment_mock_fail == 6)
    return UI_ERROR_UNKNOWN;
  return ui_disclosure_base_set_expanded(d, e);
}
static ui_error_t mock_grid_list_base_create(struct ui_grid_list_base **out,
                                             int c) {
  if (g_md3_containment_mock_fail == 7)
    return UI_ERROR_UNKNOWN;
  return ui_grid_list_base_create(out, c);
}
static ui_error_t mock_grid_list_base_destroy(struct ui_grid_list_base *b) {
  if (g_md3_containment_mock_fail == 8)
    return UI_ERROR_UNKNOWN;
  return ui_grid_list_base_destroy(b);
}
static ui_error_t mock_grid_list_base_add_item(struct ui_grid_list_base *b,
                                               int r, int c) {
  if (g_md3_containment_mock_fail == 9)
    return UI_ERROR_UNKNOWN;
  return ui_grid_list_base_add_item(b, r, c);
}
static ui_error_t mock_surface_base_create(struct ui_surface_base **out) {
  if (g_md3_containment_mock_fail == 10)
    return UI_ERROR_UNKNOWN;
  return ui_surface_base_create(out);
}
static ui_error_t mock_avatar_base_create(struct ui_avatar_base **out) {
  if (g_md3_containment_mock_fail == 11)
    return UI_ERROR_UNKNOWN;
  return ui_avatar_base_create(out);
}
static ui_error_t mock_avatar_base_destroy(struct ui_avatar_base *b) {
  if (g_md3_containment_mock_fail == 12)
    return UI_ERROR_UNKNOWN;
  return ui_avatar_base_destroy(b);
}
static ui_error_t mock_avatar_base_set_name(struct ui_avatar_base *b,
                                            const char *n) {
  if (g_md3_containment_mock_fail == 13)
    return UI_ERROR_UNKNOWN;
  return ui_avatar_base_set_name(b, n);
}
static ui_error_t mock_avatar_base_get_initials(const struct ui_avatar_base *b,
                                                const char **i) {
  if (g_md3_containment_mock_fail == 32)
    return UI_ERROR_UNKNOWN;
  return ui_avatar_base_get_initials(b, i);
}
static ui_error_t mock_avatar_base_set_image_url(struct ui_avatar_base *b,
                                                 const char *u) {
  if (g_md3_containment_mock_fail == 14)
    return UI_ERROR_UNKNOWN;
  return ui_avatar_base_set_image_url(b, u);
}
static ui_error_t
mock_avatar_group_base_create(struct ui_avatar_group_base **out) {
  if (g_md3_containment_mock_fail == 15)
    return UI_ERROR_UNKNOWN;
  return ui_avatar_group_base_create(out);
}
static ui_error_t
mock_avatar_group_base_destroy(struct ui_avatar_group_base *b) {
  if (g_md3_containment_mock_fail == 16)
    return UI_ERROR_UNKNOWN;
  return ui_avatar_group_base_destroy(b);
}
static ui_error_t
mock_avatar_group_base_append_avatar(struct ui_avatar_group_base *g,
                                     struct ui_avatar_base *a) {
  if (g_md3_containment_mock_fail == 17)
    return UI_ERROR_UNKNOWN;
  return ui_avatar_group_base_append_avatar(g, a);
}
static ui_error_t mock_swipe_action_base_update(struct ui_swipe_action_base *b,
                                                float d) {
  if (g_md3_containment_mock_fail == 18)
    return UI_ERROR_UNKNOWN;
  return ui_swipe_action_base_update(b, d);
}
static ui_error_t
mock_swipe_action_base_commit(struct ui_swipe_action_base *b) {
  if (g_md3_containment_mock_fail == 19)
    return UI_ERROR_UNKNOWN;
  return ui_swipe_action_base_commit(b);
}
static ui_error_t mock_skeleton_base_create(struct ui_skeleton_base **out) {
  if (g_md3_containment_mock_fail == 20)
    return UI_ERROR_UNKNOWN;
  return ui_skeleton_base_create(out);
}
static ui_error_t mock_skeleton_base_destroy(struct ui_skeleton_base *b) {
  if (g_md3_containment_mock_fail == 21)
    return UI_ERROR_UNKNOWN;
  return ui_skeleton_base_destroy(b);
}
static ui_error_t mock_skeleton_base_set_dimensions(struct ui_skeleton_base *b,
                                                    int w, int h) {
  if (g_md3_containment_mock_fail == 22)
    return UI_ERROR_UNKNOWN;
  return ui_skeleton_base_set_dimensions(b, w, h);
}
static ui_error_t
mock_empty_state_base_create(struct ui_empty_state_base **out) {
  if (g_md3_containment_mock_fail == 23)
    return UI_ERROR_UNKNOWN;
  return ui_empty_state_base_create(out);
}
static ui_error_t mock_empty_state_base_set_title(struct ui_empty_state_base *b,
                                                  const char *t) {
  if (g_md3_containment_mock_fail == 24)
    return UI_ERROR_UNKNOWN;
  return ui_empty_state_base_set_title(b, t);
}
static ui_error_t
mock_empty_state_base_set_description(struct ui_empty_state_base *b,
                                      const char *d) {
  if (g_md3_containment_mock_fail == 25)
    return UI_ERROR_UNKNOWN;
  return ui_empty_state_base_set_description(b, d);
}
static ui_error_t
mock_masonry_layout_base_create(struct ui_masonry_layout_base **out) {
  if (g_md3_containment_mock_fail == 26)
    return UI_ERROR_UNKNOWN;
  return ui_masonry_layout_base_create(out);
}
static ui_error_t
mock_masonry_layout_base_destroy(struct ui_masonry_layout_base *b) {
  if (g_md3_containment_mock_fail == 27)
    return UI_ERROR_UNKNOWN;
  return ui_masonry_layout_base_destroy(b);
}
static ui_error_t
mock_masonry_layout_base_reflow(struct ui_masonry_layout_base *b) {
  if (g_md3_containment_mock_fail == 28)
    return UI_ERROR_UNKNOWN;
  return ui_masonry_layout_base_reflow(b);
}
static ui_error_t
mock_aspect_ratio_base_create(struct ui_aspect_ratio_base **out) {
  if (g_md3_containment_mock_fail == 29)
    return UI_ERROR_UNKNOWN;
  return ui_aspect_ratio_base_create(out);
}
static ui_error_t
mock_aspect_ratio_base_destroy(struct ui_aspect_ratio_base *b) {
  if (g_md3_containment_mock_fail == 30)
    return UI_ERROR_UNKNOWN;
  return ui_aspect_ratio_base_destroy(b);
}
static ui_error_t
mock_aspect_ratio_base_set_ratio(struct ui_aspect_ratio_base *b, float r) {
  if (g_md3_containment_mock_fail == 31)
    return UI_ERROR_UNKNOWN;
  return ui_aspect_ratio_base_set_ratio(b, r);
}

#undef ui_accordion_base_create
#define ui_accordion_base_create mock_accordion_base_create
#undef ui_accordion_base_destroy
#define ui_accordion_base_destroy mock_accordion_base_destroy
#undef ui_disclosure_base_create
#define ui_disclosure_base_create mock_disclosure_base_create
#undef ui_disclosure_base_destroy
#define ui_disclosure_base_destroy mock_disclosure_base_destroy
#undef ui_accordion_base_add_disclosure
#define ui_accordion_base_add_disclosure mock_accordion_base_add_disclosure
#undef ui_disclosure_base_set_expanded
#define ui_disclosure_base_set_expanded mock_disclosure_base_set_expanded
#undef ui_grid_list_base_create
#define ui_grid_list_base_create mock_grid_list_base_create
#undef ui_grid_list_base_destroy
#define ui_grid_list_base_destroy mock_grid_list_base_destroy
#undef ui_grid_list_base_add_item
#define ui_grid_list_base_add_item mock_grid_list_base_add_item
#undef ui_surface_base_create
#define ui_surface_base_create mock_surface_base_create
#undef ui_avatar_base_create
#define ui_avatar_base_create mock_avatar_base_create
#undef ui_avatar_base_destroy
#define ui_avatar_base_destroy mock_avatar_base_destroy
#undef ui_avatar_base_set_name
#define ui_avatar_base_set_name mock_avatar_base_set_name
#undef ui_avatar_base_get_initials
#define ui_avatar_base_get_initials mock_avatar_base_get_initials
#undef ui_avatar_base_set_image_url
#define ui_avatar_base_set_image_url mock_avatar_base_set_image_url
#undef ui_avatar_group_base_create
#define ui_avatar_group_base_create mock_avatar_group_base_create
#undef ui_avatar_group_base_destroy
#define ui_avatar_group_base_destroy mock_avatar_group_base_destroy
#undef ui_avatar_group_base_append_avatar
#define ui_avatar_group_base_append_avatar mock_avatar_group_base_append_avatar
#undef ui_swipe_action_base_update
#define ui_swipe_action_base_update mock_swipe_action_base_update
#undef ui_swipe_action_base_commit
#define ui_swipe_action_base_commit mock_swipe_action_base_commit
#undef ui_skeleton_base_create
#define ui_skeleton_base_create mock_skeleton_base_create
#undef ui_skeleton_base_destroy
#define ui_skeleton_base_destroy mock_skeleton_base_destroy
#undef ui_skeleton_base_set_dimensions
#define ui_skeleton_base_set_dimensions mock_skeleton_base_set_dimensions
#undef ui_empty_state_base_create
#define ui_empty_state_base_create mock_empty_state_base_create
#undef ui_empty_state_base_set_title
#define ui_empty_state_base_set_title mock_empty_state_base_set_title
#undef ui_empty_state_base_set_description
#define ui_empty_state_base_set_description                                    \
  mock_empty_state_base_set_description
#undef ui_masonry_layout_base_create
#define ui_masonry_layout_base_create mock_masonry_layout_base_create
#undef ui_masonry_layout_base_destroy
#define ui_masonry_layout_base_destroy mock_masonry_layout_base_destroy
#undef ui_masonry_layout_base_reflow
#define ui_masonry_layout_base_reflow mock_masonry_layout_base_reflow
#undef ui_aspect_ratio_base_create
#define ui_aspect_ratio_base_create mock_aspect_ratio_base_create
#undef ui_aspect_ratio_base_destroy
#define ui_aspect_ratio_base_destroy mock_aspect_ratio_base_destroy
#undef ui_aspect_ratio_base_set_ratio
#define ui_aspect_ratio_base_set_ratio mock_aspect_ratio_base_set_ratio
#endif

/* ========================================================================= */
/* md3_accordion & md3_expansion_panel                                       */
/* ========================================================================= */

ui_error_t md3_accordion_create(struct ui_engine *engine,
                                struct md3_accordion **out_accordion) {
  struct md3_accordion *acc;
  ui_error_t rc;

  if (!engine || !out_accordion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  acc = (struct md3_accordion *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_accordion));
  if (!acc) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(acc, 0, sizeof(struct md3_accordion));

  rc = ui_accordion_base_create(&acc->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(acc);
    return rc;
  }

  acc->multi_expand = 0;
  acc->panel_count = 0;

  *out_accordion = acc;
  return UI_ERROR_NONE;
}

ui_error_t md3_accordion_destroy(struct md3_accordion *accordion) {
  ui_error_t rc;

  if (!accordion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_accordion_base_destroy(accordion->base);
  C_MULTIPLATFORM_FREE(accordion);
  return rc;
}

ui_error_t md3_accordion_set_multi_expand(struct md3_accordion *accordion,
                                          int multi_expand) {
  if (!accordion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  accordion->multi_expand = multi_expand ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_expansion_panel_create(struct ui_engine *engine,
                                      struct md3_expansion_panel **out_panel) {
  struct md3_expansion_panel *panel;
  ui_error_t rc;

  if (!engine || !out_panel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  panel = (struct md3_expansion_panel *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_expansion_panel));
  if (!panel) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(panel, 0, sizeof(struct md3_expansion_panel));

  rc = ui_disclosure_base_create(&panel->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(panel);
    return rc;
  }

  panel->is_expanded = 0;
  panel->is_disabled = 0;
  panel->title[0] = '\0';
  panel->subtitle[0] = '\0';
  panel->elevation = 0.0f;

  *out_panel = panel;
  return UI_ERROR_NONE;
}

ui_error_t md3_expansion_panel_destroy(struct md3_expansion_panel *panel) {
  ui_error_t rc;

  if (!panel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_disclosure_base_destroy(panel->base);
  C_MULTIPLATFORM_FREE(panel);
  return rc;
}

ui_error_t md3_accordion_add_panel(struct md3_accordion *accordion,
                                   struct md3_expansion_panel *panel) {
  ui_error_t rc;

  if (!accordion || !panel || !panel->base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (accordion->panel_count >=
      sizeof(accordion->panels) / sizeof(accordion->panels[0])) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  accordion->panels[accordion->panel_count++] = panel;

  rc = ui_accordion_base_add_disclosure(accordion->base, panel->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_expansion_panel_set_expanded(struct md3_expansion_panel *panel,
                                            int is_expanded) {
  ui_error_t rc;

  if (!panel || !panel->base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  panel->is_expanded = is_expanded ? 1 : 0;
  panel->elevation = panel->is_expanded ? 1.0f : 0.0f;

  rc = ui_disclosure_base_set_expanded(panel->base, panel->is_expanded);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_expansion_panel_set_title(struct md3_expansion_panel *panel,
                                         const char *title,
                                         const char *subtitle) {
  if (!panel || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(panel->title, sizeof(panel->title), title, _TRUNCATE);
  if (subtitle) {
    strncpy_s(panel->subtitle, sizeof(panel->subtitle), subtitle, _TRUNCATE);
  } else {
    panel->subtitle[0] = '\0';
  }
#else
  strncpy(panel->title, title, sizeof(panel->title) - 1);
  panel->title[sizeof(panel->title) - 1] = '\0';
  if (subtitle) {
    strncpy(panel->subtitle, subtitle, sizeof(panel->subtitle) - 1);
    panel->subtitle[sizeof(panel->subtitle) - 1] = '\0';
  } else {
    panel->subtitle[0] = '\0';
  }
#endif

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_grid_list & md3_grid_tile                                             */
/* ========================================================================= */

ui_error_t md3_grid_list_create(struct ui_engine *engine, int columns,
                                struct md3_grid_list **out_grid_list) {
  struct md3_grid_list *gl;
  ui_error_t rc;

  if (!engine || !out_grid_list || columns <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  gl = (struct md3_grid_list *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_grid_list));
  if (!gl) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(gl, 0, sizeof(struct md3_grid_list));

  rc = ui_grid_list_base_create(&gl->base, columns);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(gl);
    return rc;
  }

  gl->columns = columns;
  gl->gutter = 8.0f;
  gl->tile_count = 0;

  *out_grid_list = gl;
  return UI_ERROR_NONE;
}

ui_error_t md3_grid_list_destroy(struct md3_grid_list *grid_list) {
  ui_error_t rc;

  if (!grid_list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_grid_list_base_destroy(grid_list->base);
  C_MULTIPLATFORM_FREE(grid_list);
  return rc;
}

ui_error_t md3_grid_list_add_tile(struct md3_grid_list *grid_list, int rowspan,
                                  int colspan, const char *title) {
  size_t idx;
  ui_error_t rc;

  if (!grid_list || rowspan <= 0 || colspan <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (grid_list->tile_count >=
      sizeof(grid_list->tiles) / sizeof(grid_list->tiles[0])) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = grid_list->tile_count;
  grid_list->tiles[idx].rowspan = rowspan;
  grid_list->tiles[idx].colspan = colspan;
  if (title) {
#if defined(_MSC_VER)
    strncpy_s(grid_list->tiles[idx].title, sizeof(grid_list->tiles[idx].title),
              title, _TRUNCATE);
#else
    strncpy(grid_list->tiles[idx].title, title,
            sizeof(grid_list->tiles[idx].title) - 1);
    grid_list->tiles[idx].title[sizeof(grid_list->tiles[idx].title) - 1] = '\0';
#endif
  } else {
    grid_list->tiles[idx].title[0] = '\0';
  }
  grid_list->tile_count++;

  rc = ui_grid_list_base_add_item(grid_list->base, rowspan, colspan);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_surface                                                               */
/* ========================================================================= */

ui_error_t md3_surface_create(struct ui_engine *engine,
                              enum md3_surface_role role, int elevation_level,
                              struct md3_surface **out_surface) {
  struct md3_surface *surf;
  ui_error_t rc;

  if (!engine || !out_surface || elevation_level < 0 || elevation_level > 5) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  surf =
      (struct md3_surface *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_surface));
  if (!surf) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(surf, 0, sizeof(struct md3_surface));

  rc = ui_surface_base_create(&surf->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(surf);
    return rc;
  }

  surf->role = role;
  surf->elevation_level = elevation_level;
  surf->corner_radius = 12.0f;
  surf->has_outline = 0;

  *out_surface = surf;
  return ui_surface_base_set_elevation(
      surf->base, (enum ui_elevation_level)elevation_level);
}

ui_error_t md3_surface_destroy(struct md3_surface *surface) {
  if (!surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(surface);
  return UI_ERROR_NONE;
}

ui_error_t md3_surface_set_elevation(struct md3_surface *surface,
                                     int elevation_level) {
  if (!surface || elevation_level < 0 || elevation_level > 5) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  surface->elevation_level = elevation_level;
  return ui_surface_base_set_elevation(
      surface->base, (enum ui_elevation_level)elevation_level);
}

/* ========================================================================= */
/* md3_avatar & md3_avatar_group                                             */
/* ========================================================================= */

ui_error_t md3_avatar_create(struct ui_engine *engine, enum ui_avatar_type type,
                             struct md3_avatar **out_avatar) {
  struct md3_avatar *av;
  ui_error_t rc;

  if (!engine || !out_avatar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  av = (struct md3_avatar *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_avatar));
  if (!av) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(av, 0, sizeof(struct md3_avatar));

  rc = ui_avatar_base_create(&av->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(av);
    return rc;
  }

  av->type = type;
  av->size_dp = 40.0f;
  av->name[0] = '\0';
  av->initials[0] = '\0';
  av->image_url[0] = '\0';

  *out_avatar = av;
  return UI_ERROR_NONE;
}

ui_error_t md3_avatar_destroy(struct md3_avatar *avatar) {
  ui_error_t rc;

  if (!avatar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_avatar_base_destroy(avatar->base);
  C_MULTIPLATFORM_FREE(avatar);
  return rc;
}

ui_error_t md3_avatar_set_name(struct md3_avatar *avatar, const char *name) {
  const char *inits = NULL;
  ui_error_t rc;

  if (!avatar || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(avatar->name, sizeof(avatar->name), name, _TRUNCATE);
#else
  strncpy(avatar->name, name, sizeof(avatar->name) - 1);
  avatar->name[sizeof(avatar->name) - 1] = '\0';
#endif

  rc = ui_avatar_base_set_name(avatar->base, name);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_avatar_base_get_initials(avatar->base, &inits);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
#if defined(_MSC_VER)
  strncpy_s(avatar->initials, sizeof(avatar->initials), inits, _TRUNCATE);
#else
  strncpy(avatar->initials, inits, sizeof(avatar->initials) - 1);
  avatar->initials[sizeof(avatar->initials) - 1] = 0;
#endif
  return UI_ERROR_NONE;
}

ui_error_t md3_avatar_set_image_url(struct md3_avatar *avatar,
                                    const char *image_url) {
  ui_error_t rc;

  if (!avatar || !image_url) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(avatar->image_url, sizeof(avatar->image_url), image_url, _TRUNCATE);
#else
  strncpy(avatar->image_url, image_url, sizeof(avatar->image_url) - 1);
  avatar->image_url[sizeof(avatar->image_url) - 1] = '\0';
#endif
  avatar->type = UI_AVATAR_TYPE_IMAGE;

  rc = ui_avatar_base_set_image_url(avatar->base, image_url);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_avatar_group_create(struct ui_engine *engine, int max_visible,
                                   struct md3_avatar_group **out_group) {
  struct md3_avatar_group *grp;
  ui_error_t rc;

  if (!engine || !out_group || max_visible < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  grp = (struct md3_avatar_group *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_avatar_group));
  if (!grp) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(grp, 0, sizeof(struct md3_avatar_group));

  rc = ui_avatar_group_base_create(&grp->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(grp);
    return rc;
  }

  grp->max_visible = max_visible;
  grp->avatar_count = 0;

  *out_group = grp;
  return ui_avatar_group_base_set_max_avatars(grp->base,
                                              (unsigned int)max_visible);
}

ui_error_t md3_avatar_group_destroy(struct md3_avatar_group *group) {
  ui_error_t rc;

  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_avatar_group_base_destroy(group->base);
  C_MULTIPLATFORM_FREE(group);
  return rc;
}

ui_error_t md3_avatar_group_append(struct md3_avatar_group *group,
                                   struct md3_avatar *avatar) {
  ui_error_t rc;

  if (!group || !avatar || !avatar->base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (group->avatar_count >=
      sizeof(group->avatars) / sizeof(group->avatars[0])) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  group->avatars[group->avatar_count++] = avatar;

  rc = ui_avatar_group_base_append_avatar(group->base, avatar->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_swipe_action                                                          */
/* ========================================================================= */

ui_error_t md3_swipe_action_create(struct ui_engine *engine,
                                   struct md3_swipe_action **out_action) {
  struct md3_swipe_action *act;

  if (!engine || !out_action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  act = (struct md3_swipe_action *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_swipe_action));
  if (!act) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(act, 0, sizeof(struct md3_swipe_action));

  act->threshold_ratio = 0.40f;
  act->is_dismissed = 0;

  *out_action = act;
  return ui_swipe_action_base_init(&act->base, (struct ui_component *)0x1234);
}

ui_error_t md3_swipe_action_destroy(struct md3_swipe_action *action) {
  if (!action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(action);
  return UI_ERROR_NONE;
}

ui_error_t md3_swipe_action_update(struct md3_swipe_action *action,
                                   float delta_x) {
  ui_error_t rc;

  if (!action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_swipe_action_base_update(&action->base, delta_x);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_swipe_action_commit(struct md3_swipe_action *action) {
  ui_error_t rc;

  if (!action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_swipe_action_base_commit(&action->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (action->base.state == UI_SWIPE_ACTION_REVEALED_LEFT ||
      action->base.state == UI_SWIPE_ACTION_REVEALED_RIGHT) {
    action->is_dismissed = 1;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_skeleton                                                              */
/* ========================================================================= */

ui_error_t md3_skeleton_create(struct ui_engine *engine,
                               enum md3_skeleton_variant variant,
                               struct md3_skeleton **out_skeleton) {
  struct md3_skeleton *skel;
  ui_error_t rc;
  enum ui_skeleton_shape shape;

  if (!engine || !out_skeleton) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  skel = (struct md3_skeleton *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_skeleton));
  if (!skel) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(skel, 0, sizeof(struct md3_skeleton));

  rc = ui_skeleton_base_create(&skel->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(skel);
    return rc;
  }

  skel->variant = variant;
  skel->width = (variant == MD3_SKELETON_CIRCULAR) ? 40.0f : 120.0f;
  skel->height = (variant == MD3_SKELETON_CIRCULAR) ? 40.0f : 20.0f;
  skel->shimmer_duration_ms = 1500.0f;
  skel->is_reduced_motion = 0;

  shape = (variant == MD3_SKELETON_CIRCULAR) ? UI_SKELETON_SHAPE_CIRCLE
          : (variant == MD3_SKELETON_BUTTON_PILL)
              ? UI_SKELETON_SHAPE_ROUNDED_RECTANGLE
              : UI_SKELETON_SHAPE_RECTANGLE;

  *out_skeleton = skel;
  return ui_skeleton_base_set_shape(skel->base, shape);
}

ui_error_t md3_skeleton_destroy(struct md3_skeleton *skeleton) {
  ui_error_t rc;

  if (!skeleton) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_skeleton_base_destroy(skeleton->base);
  C_MULTIPLATFORM_FREE(skeleton);
  return rc;
}

ui_error_t md3_skeleton_set_size(struct md3_skeleton *skeleton, float width,
                                 float height) {
  ui_error_t rc;

  if (!skeleton || width <= 0.0f || height <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  skeleton->width = width;
  skeleton->height = height;

  rc = ui_skeleton_base_set_dimensions(skeleton->base, (int)width, (int)height);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_empty_state                                                           */
/* ========================================================================= */

ui_error_t md3_empty_state_create(struct ui_engine *engine,
                                  struct md3_empty_state **out_empty_state) {
  struct md3_empty_state *es;
  ui_error_t rc;

  if (!engine || !out_empty_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  es = (struct md3_empty_state *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_empty_state));
  if (!es) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(es, 0, sizeof(struct md3_empty_state));

  rc = ui_empty_state_base_create(&es->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(es);
    return rc;
  }

  es->title[0] = '\0';
  es->description[0] = '\0';
  es->primary_action_label[0] = '\0';

  *out_empty_state = es;
  return UI_ERROR_NONE;
}

ui_error_t md3_empty_state_destroy(struct md3_empty_state *empty_state) {
  if (!empty_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(empty_state);
  return UI_ERROR_NONE;
}

ui_error_t md3_empty_state_set_content(struct md3_empty_state *empty_state,
                                       const char *title,
                                       const char *description) {
  ui_error_t rc;

  if (!empty_state || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(empty_state->title, sizeof(empty_state->title), title, _TRUNCATE);
  if (description) {
    strncpy_s(empty_state->description, sizeof(empty_state->description),
              description, _TRUNCATE);
  } else {
    empty_state->description[0] = '\0';
  }
#else
  strncpy(empty_state->title, title, sizeof(empty_state->title) - 1);
  empty_state->title[sizeof(empty_state->title) - 1] = '\0';
  if (description) {
    strncpy(empty_state->description, description,
            sizeof(empty_state->description) - 1);
    empty_state->description[sizeof(empty_state->description) - 1] = '\0';
  } else {
    empty_state->description[0] = '\0';
  }
#endif

  rc = ui_empty_state_base_set_title(empty_state->base, title);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (description) {
    rc = ui_empty_state_base_set_description(empty_state->base, description);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_masonry_layout                                                        */
/* ========================================================================= */

ui_error_t md3_masonry_layout_create(struct ui_engine *engine, int columns,
                                     struct md3_masonry_layout **out_masonry) {
  struct md3_masonry_layout *ml;
  ui_error_t rc;

  if (!engine || !out_masonry || columns <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ml = (struct md3_masonry_layout *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_masonry_layout));
  if (!ml) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(ml, 0, sizeof(struct md3_masonry_layout));

  rc = ui_masonry_layout_base_create(&ml->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ml);
    return rc;
  }

  ml->columns = columns;
  ml->gutter_dp = 12.0f;

  *out_masonry = ml;
  return UI_ERROR_NONE;
}

ui_error_t md3_masonry_layout_destroy(struct md3_masonry_layout *masonry) {
  ui_error_t rc;

  if (!masonry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_masonry_layout_base_destroy(masonry->base);
  C_MULTIPLATFORM_FREE(masonry);
  return rc;
}

ui_error_t md3_masonry_layout_reflow(struct md3_masonry_layout *masonry) {
  ui_error_t rc;

  if (!masonry || !masonry->base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_masonry_layout_base_reflow(masonry->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_aspect_ratio                                                          */
/* ========================================================================= */

ui_error_t md3_aspect_ratio_create(struct ui_engine *engine, float ratio,
                                   struct md3_aspect_ratio **out_aspect_ratio) {
  struct md3_aspect_ratio *ar;
  ui_error_t rc;

  if (!engine || !out_aspect_ratio || ratio <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ar = (struct md3_aspect_ratio *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_aspect_ratio));
  if (!ar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(ar, 0, sizeof(struct md3_aspect_ratio));

  rc = ui_aspect_ratio_base_create(&ar->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ar);
    return rc;
  }

  ar->ratio = ratio;
  *out_aspect_ratio = ar;
  return ui_aspect_ratio_base_set_ratio(ar->base, ratio);
}

ui_error_t md3_aspect_ratio_destroy(struct md3_aspect_ratio *aspect_ratio) {
  ui_error_t rc;

  if (!aspect_ratio) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_aspect_ratio_base_destroy(aspect_ratio->base);
  C_MULTIPLATFORM_FREE(aspect_ratio);
  return rc;
}

ui_error_t md3_aspect_ratio_set_ratio(struct md3_aspect_ratio *aspect_ratio,
                                      float ratio) {
  ui_error_t rc;

  if (!aspect_ratio || !aspect_ratio->base || ratio <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  aspect_ratio->ratio = ratio;
  rc = ui_aspect_ratio_base_set_ratio(aspect_ratio->base, ratio);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}
