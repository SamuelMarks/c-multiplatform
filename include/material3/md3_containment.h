/**
 * @file md3_containment.h
 * @brief Material 3 & Expressive Containment, Layout, and Presentation
 * Components.
 *
 * Provides spec-compliant Material 3 implementations for:
 * - md3_accordion & md3_expansion_panel (wrapping ui_accordion_base &
 * ui_disclosure_base)
 * - md3_grid_list & md3_grid_tile (wrapping ui_grid_list_base)
 * - md3_surface (wrapping ui_surface_base)
 * - md3_avatar & md3_avatar_group (wrapping ui_avatar_base &
 * ui_avatar_group_base)
 * - md3_swipe_action (wrapping ui_swipe_action_base)
 * - md3_skeleton (wrapping ui_skeleton_base)
 * - md3_empty_state (wrapping ui_empty_state_base)
 * - md3_masonry_layout (wrapping ui_masonry_layout_base)
 * - md3_aspect_ratio (wrapping ui_aspect_ratio_base)
 */

#ifndef MATERIAL3_MD3_CONTAINMENT_H
#define MATERIAL3_MD3_CONTAINMENT_H

/* clang-format off */
#include "ui_accordion_base.h"
#include "ui_aspect_ratio_base.h"
#include "ui_avatar_base.h"
#include "ui_avatar_group_base.h"
#include "ui_disclosure_base.h"
#include "ui_empty_state_base.h"
#include "ui_error.h"
#include "ui_grid_list_base.h"
#include "ui_masonry_layout_base.h"
#include "ui_skeleton_base.h"
#include "ui_surface_base.h"
#include "ui_swipe_action_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/* ========================================================================= */
/* md3_accordion & md3_expansion_panel                                       */
/* ========================================================================= */

/**
 * @struct md3_expansion_panel
 * @brief Material 3 Expansion Panel component.
 */
struct md3_expansion_panel {
  struct ui_disclosure_base *base;
  int is_expanded;
  int is_disabled;
  char title[128];
  char subtitle[128];
  float elevation;
};

/**
 * @struct md3_accordion
 * @brief Material 3 Accordion container managing expansion panels.
 */
struct md3_accordion {
  struct ui_accordion_base *base;
  int multi_expand;
  struct md3_expansion_panel *panels[32];
  size_t panel_count;
};

/**
 * @brief Creates a Material 3 Accordion component.
 *
 * @param engine The UI engine context.
 * @param out_accordion Pointer to receive the allocated accordion.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_accordion_create(
    struct ui_engine *engine, struct md3_accordion **out_accordion);

/**
 * @brief Destroys a Material 3 Accordion component.
 *
 * @param accordion The accordion to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_accordion_destroy(struct md3_accordion *accordion);

/**
 * @brief Sets multi-expansion mode on the accordion.
 *
 * @param accordion The accordion.
 * @param multi_expand 1 to allow multiple panels open, 0 for mutex accordion.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_accordion_set_multi_expand(
    struct md3_accordion *accordion, int multi_expand);

/**
 * @brief Creates a Material 3 Expansion Panel.
 *
 * @param engine The UI engine context.
 * @param out_panel Pointer to receive the allocated expansion panel.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_expansion_panel_create(
    struct ui_engine *engine, struct md3_expansion_panel **out_panel);

/**
 * @brief Destroys a Material 3 Expansion Panel.
 *
 * @param panel The expansion panel to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_expansion_panel_destroy(struct md3_expansion_panel *panel);

/**
 * @brief Adds an expansion panel to the accordion.
 *
 * @param accordion The accordion.
 * @param panel The expansion panel to add.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_accordion_add_panel(
    struct md3_accordion *accordion, struct md3_expansion_panel *panel);

/**
 * @brief Sets the expanded state of an expansion panel.
 *
 * @param panel The expansion panel.
 * @param is_expanded 1 to expand, 0 to collapse.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_expansion_panel_set_expanded(
    struct md3_expansion_panel *panel, int is_expanded);

/**
 * @brief Sets the title and subtitle of an expansion panel.
 *
 * @param panel The expansion panel.
 * @param title Title string.
 * @param subtitle Optional subtitle string.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_expansion_panel_set_title(
    struct md3_expansion_panel *panel, const char *title, const char *subtitle);

/* ========================================================================= */
/* md3_grid_list & md3_grid_tile                                             */
/* ========================================================================= */

/**
 * @struct md3_grid_tile
 * @brief A single tile in a Material 3 Grid List.
 */
struct md3_grid_tile {
  int rowspan;
  int colspan;
  char title[64];
};

/**
 * @struct md3_grid_list
 * @brief Material 3 Grid List component.
 */
struct md3_grid_list {
  struct ui_grid_list_base *base;
  int columns;
  float gutter;
  struct md3_grid_tile tiles[64];
  size_t tile_count;
};

/**
 * @brief Creates a Material 3 Grid List component.
 *
 * @param engine The UI engine context.
 * @param columns Number of columns in the grid.
 * @param out_grid_list Pointer to receive the allocated grid list.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_grid_list_create(struct ui_engine *engine, int columns,
                     struct md3_grid_list **out_grid_list);

/**
 * @brief Destroys a Material 3 Grid List component.
 *
 * @param grid_list The grid list to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_grid_list_destroy(struct md3_grid_list *grid_list);

/**
 * @brief Adds a tile to the Material 3 Grid List.
 *
 * @param grid_list The grid list.
 * @param rowspan Row span of the tile (>= 1).
 * @param colspan Column span of the tile (>= 1).
 * @param title Tile title label.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_grid_list_add_tile(struct md3_grid_list *grid_list, int rowspan,
                       int colspan, const char *title);

/* ========================================================================= */
/* md3_surface                                                               */
/* ========================================================================= */

/**
 * @enum md3_surface_role
 * @brief Surface tonal container roles in Material 3.
 */
enum md3_surface_role {
  MD3_SURFACE_ROLE_SURFACE,
  MD3_SURFACE_ROLE_SURFACE_CONTAINER_LOWEST,
  MD3_SURFACE_ROLE_SURFACE_CONTAINER_LOW,
  MD3_SURFACE_ROLE_SURFACE_CONTAINER,
  MD3_SURFACE_ROLE_SURFACE_CONTAINER_HIGH,
  MD3_SURFACE_ROLE_SURFACE_CONTAINER_HIGHEST,
  MD3_SURFACE_ROLE_SURFACE_DIM,
  MD3_SURFACE_ROLE_SURFACE_BRIGHT
};

/**
 * @struct md3_surface
 * @brief Material 3 Surface / Paper canvas container.
 */
struct md3_surface {
  struct ui_surface_base *base;
  enum md3_surface_role role;
  int elevation_level;
  float corner_radius;
  int has_outline;
};

/**
 * @brief Creates a Material 3 Surface component.
 *
 * @param engine The UI engine context.
 * @param role Tonal container role.
 * @param elevation_level Elevation level (0-5).
 * @param out_surface Pointer to receive the allocated surface.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_surface_create(struct ui_engine *engine, enum md3_surface_role role,
                   int elevation_level, struct md3_surface **out_surface);

/**
 * @brief Destroys a Material 3 Surface component.
 *
 * @param surface The surface to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_surface_destroy(struct md3_surface *surface);

/**
 * @brief Sets the elevation level of the surface.
 *
 * @param surface The surface.
 * @param elevation_level New elevation level (0-5).
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_surface_set_elevation(struct md3_surface *surface, int elevation_level);

/* ========================================================================= */
/* md3_avatar & md3_avatar_group                                             */
/* ========================================================================= */

/**
 * @struct md3_avatar
 * @brief Material 3 Avatar component.
 */
struct md3_avatar {
  struct ui_avatar_base *base;
  enum ui_avatar_type type;
  float size_dp;
  char name[64];
  char initials[8];
  char image_url[256];
};

/**
 * @struct md3_avatar_group
 * @brief Material 3 Avatar Group container.
 */
struct md3_avatar_group {
  struct ui_avatar_group_base *base;
  int max_visible;
  struct md3_avatar *avatars[32];
  size_t avatar_count;
};

/**
 * @brief Creates a Material 3 Avatar component.
 *
 * @param engine The UI engine context.
 * @param type Initial avatar display type.
 * @param out_avatar Pointer to receive the allocated avatar.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_avatar_create(struct ui_engine *engine, enum ui_avatar_type type,
                  struct md3_avatar **out_avatar);

/**
 * @brief Destroys a Material 3 Avatar component.
 *
 * @param avatar The avatar to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_avatar_destroy(struct md3_avatar *avatar);

/**
 * @brief Sets person name on avatar to generate initials.
 *
 * @param avatar The avatar.
 * @param name Full person name.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_avatar_set_name(struct md3_avatar *avatar, const char *name);

/**
 * @brief Sets image URL on avatar.
 *
 * @param avatar The avatar.
 * @param image_url Image resource URL.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_avatar_set_image_url(struct md3_avatar *avatar, const char *image_url);

/**
 * @brief Creates a Material 3 Avatar Group.
 *
 * @param engine The UI engine context.
 * @param max_visible Max number of avatars before "+N" overflow badge.
 * @param out_group Pointer to receive the allocated avatar group.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_avatar_group_create(struct ui_engine *engine, int max_visible,
                        struct md3_avatar_group **out_group);

/**
 * @brief Destroys a Material 3 Avatar Group.
 *
 * @param group The avatar group to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_avatar_group_destroy(struct md3_avatar_group *group);

/**
 * @brief Appends an avatar into the avatar group.
 *
 * @param group The avatar group.
 * @param avatar The avatar to append.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_avatar_group_append(
    struct md3_avatar_group *group, struct md3_avatar *avatar);

/* ========================================================================= */
/* md3_swipe_action                                                          */
/* ========================================================================= */

/**
 * @struct md3_swipe_action
 * @brief Material 3 Swipe Action container for list items and cards.
 */
struct md3_swipe_action {
  struct ui_swipe_action_base base;
  float threshold_ratio;
  int is_dismissed;
};

/**
 * @brief Creates a Material 3 Swipe Action component.
 *
 * @param engine The UI engine context.
 * @param out_action Pointer to receive the allocated swipe action.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_swipe_action_create(
    struct ui_engine *engine, struct md3_swipe_action **out_action);

/**
 * @brief Destroys a Material 3 Swipe Action component.
 *
 * @param action The swipe action to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_swipe_action_destroy(struct md3_swipe_action *action);

/**
 * @brief Updates the swipe drag gesture delta.
 *
 * @param action The swipe action.
 * @param delta_x Pointer horizontal displacement.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_swipe_action_update(struct md3_swipe_action *action, float delta_x);

/**
 * @brief Commits the swipe gesture on release.
 *
 * @param action The swipe action.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_swipe_action_commit(struct md3_swipe_action *action);

/* ========================================================================= */
/* md3_skeleton                                                              */
/* ========================================================================= */

/**
 * @enum md3_skeleton_variant
 * @brief Wireframe skeleton geometry presets.
 */
enum md3_skeleton_variant {
  MD3_SKELETON_TEXT,
  MD3_SKELETON_CIRCULAR,
  MD3_SKELETON_RECTANGULAR,
  MD3_SKELETON_BUTTON_PILL
};

/**
 * @struct md3_skeleton
 * @brief Material 3 Shimmer Skeleton Placeholder component.
 */
struct md3_skeleton {
  struct ui_skeleton_base *base;
  enum md3_skeleton_variant variant;
  float width;
  float height;
  float shimmer_duration_ms;
  int is_reduced_motion;
};

/**
 * @brief Creates a Material 3 Skeleton component.
 *
 * @param engine The UI engine context.
 * @param variant The wireframe skeleton shape variant.
 * @param out_skeleton Pointer to receive the allocated skeleton.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_skeleton_create(struct ui_engine *engine, enum md3_skeleton_variant variant,
                    struct md3_skeleton **out_skeleton);

/**
 * @brief Destroys a Material 3 Skeleton component.
 *
 * @param skeleton The skeleton to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_skeleton_destroy(struct md3_skeleton *skeleton);

/**
 * @brief Sets skeleton dimensions.
 *
 * @param skeleton The skeleton.
 * @param width Width in dp.
 * @param height Height in dp.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_skeleton_set_size(struct md3_skeleton *skeleton, float width, float height);

/* ========================================================================= */
/* md3_empty_state                                                           */
/* ========================================================================= */

/**
 * @struct md3_empty_state
 * @brief Material 3 Empty State layout container.
 */
struct md3_empty_state {
  struct ui_empty_state_base *base;
  char title[128];
  char description[256];
  char primary_action_label[64];
};

/**
 * @brief Creates a Material 3 Empty State component.
 *
 * @param engine The UI engine context.
 * @param out_empty_state Pointer to receive the allocated empty state.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_empty_state_create(
    struct ui_engine *engine, struct md3_empty_state **out_empty_state);

/**
 * @brief Destroys a Material 3 Empty State component.
 *
 * @param empty_state The empty state to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_empty_state_destroy(struct md3_empty_state *empty_state);

/**
 * @brief Sets headline title and supporting description on empty state.
 *
 * @param empty_state The empty state.
 * @param title Headline title.
 * @param description Supporting text description.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_empty_state_set_content(struct md3_empty_state *empty_state,
                            const char *title, const char *description);

/* ========================================================================= */
/* md3_masonry_layout                                                        */
/* ========================================================================= */

/**
 * @struct md3_masonry_layout
 * @brief Material 3 Multi-column dynamic Masonry card layout.
 */
struct md3_masonry_layout {
  struct ui_masonry_layout_base *base;
  int columns;
  float gutter_dp;
};

/**
 * @brief Creates a Material 3 Masonry Layout component.
 *
 * @param engine The UI engine context.
 * @param columns Number of columns.
 * @param out_masonry Pointer to receive the allocated masonry layout.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_masonry_layout_create(struct ui_engine *engine, int columns,
                          struct md3_masonry_layout **out_masonry);

/**
 * @brief Destroys a Material 3 Masonry Layout component.
 *
 * @param masonry The masonry layout to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_masonry_layout_destroy(struct md3_masonry_layout *masonry);

/**
 * @brief Recalculates masonry column packing and reflows cards.
 *
 * @param masonry The masonry layout.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_masonry_layout_reflow(struct md3_masonry_layout *masonry);

/* ========================================================================= */
/* md3_aspect_ratio                                                          */
/* ========================================================================= */

/**
 * @struct md3_aspect_ratio
 * @brief Material 3 Strict Aspect Ratio container.
 */
struct md3_aspect_ratio {
  struct ui_aspect_ratio_base *base;
  float ratio;
};

/**
 * @brief Creates a Material 3 Aspect Ratio component.
 *
 * @param engine The UI engine context.
 * @param ratio Target aspect ratio (width / height, e.g. 16.0f / 9.0f).
 * @param out_aspect_ratio Pointer to receive the allocated aspect ratio handle.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_aspect_ratio_create(struct ui_engine *engine, float ratio,
                        struct md3_aspect_ratio **out_aspect_ratio);

/**
 * @brief Destroys a Material 3 Aspect Ratio component.
 *
 * @param aspect_ratio The aspect ratio component to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_aspect_ratio_destroy(struct md3_aspect_ratio *aspect_ratio);

/**
 * @brief Sets the aspect ratio.
 *
 * @param aspect_ratio The aspect ratio component.
 * @param ratio New ratio (width / height).
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_aspect_ratio_set_ratio(struct md3_aspect_ratio *aspect_ratio, float ratio);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_CONTAINMENT_H */
