/**
 * @file test_material3_containment.c
 * @brief Unit tests for Material 3 Containment & Layout Components.
 */

/* clang-format off */
#include "material3/md3_containment.h"
#include "ui_engine.h"
#include "ui_test_mock_mem.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md3_containment_mock_fail;
#endif

TEST test_md3_accordion_and_panel(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_accordion *acc = NULL;
  struct md3_expansion_panel *panel1 = NULL;
  struct md3_expansion_panel *panel2 = NULL;
  struct md3_expansion_panel *extra_panel = NULL;
  size_t i;
  ui_error_t rc;

  /* Null checks */
  rc = md3_accordion_create(NULL, &acc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_accordion_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_accordion_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_expansion_panel_create(NULL, &panel1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_expansion_panel_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_expansion_panel_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = md3_accordion_create(dummy_engine, &acc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(acc != NULL);

  rc = md3_expansion_panel_create(dummy_engine, &panel1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(panel1 != NULL);

  rc = md3_expansion_panel_create(dummy_engine, &panel2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(panel2 != NULL);

  /* Panel title with subtitle */
  rc = md3_expansion_panel_set_title(NULL, "Title", "Subtitle");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_expansion_panel_set_title(panel1, NULL, "Subtitle");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_expansion_panel_set_title(panel1, "Billing", "Payment details");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Billing", panel1->title);
  ASSERT_STR_EQ("Payment details", panel1->subtitle);

  /* Panel title without subtitle */
  rc = md3_expansion_panel_set_title(panel1, "Billing", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Billing", panel1->title);
  ASSERT_STR_EQ("", panel1->subtitle);

  /* Panel expand / collapse */
  rc = md3_expansion_panel_set_expanded(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_expansion_panel_set_expanded(panel1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, panel1->is_expanded);
  ASSERT_EQ(1.0f, panel1->elevation);

  rc = md3_expansion_panel_set_expanded(panel1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, panel1->is_expanded);
  ASSERT_EQ(0.0f, panel1->elevation);

  /* Add panel to accordion */
  rc = md3_accordion_add_panel(NULL, panel1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_accordion_add_panel(acc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  {
    struct md3_expansion_panel bad_panel;
    bad_panel.base = NULL;
    rc = md3_accordion_add_panel(acc, &bad_panel);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    rc = md3_expansion_panel_set_expanded(&bad_panel, 1);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }

  rc = md3_accordion_add_panel(acc, panel1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_accordion_add_panel(acc, panel2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)acc->panel_count);

  /* Fill up accordion to 32 panels limit */
  for (i = 2; i < 32; i++) {
    rc = md3_expansion_panel_create(dummy_engine, &extra_panel);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_accordion_add_panel(acc, extra_panel);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceed 32 panels limit -> UI_ERROR_OUT_OF_MEMORY */
  rc = md3_expansion_panel_create(dummy_engine, &extra_panel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_accordion_add_panel(acc, extra_panel);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  rc = md3_expansion_panel_destroy(extra_panel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Multi-expand toggle */
  rc = md3_accordion_set_multi_expand(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_accordion_set_multi_expand(acc, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, acc->multi_expand);
  rc = md3_accordion_set_multi_expand(acc, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, acc->multi_expand);

  /* Clean up */
  for (i = 0; i < acc->panel_count; i++) {
    rc = md3_expansion_panel_destroy(acc->panels[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_accordion_destroy(acc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_grid_list_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_grid_list *gl = NULL;
  size_t i;
  ui_error_t rc;

  /* Null and invalid args */
  rc = md3_grid_list_create(NULL, 4, &gl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_grid_list_create(dummy_engine, 0, &gl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_grid_list_create(dummy_engine, -2, &gl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_grid_list_create(dummy_engine, 4, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = md3_grid_list_create(dummy_engine, 4, &gl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(gl != NULL);
  ASSERT_EQ(4, gl->columns);

  /* Add tiles */
  rc = md3_grid_list_add_tile(NULL, 1, 1, "Tile 1");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_grid_list_add_tile(gl, 0, 1, "Tile 1");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_grid_list_add_tile(gl, -1, 1, "Tile 1");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_grid_list_add_tile(gl, 1, 0, "Tile 1");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_grid_list_add_tile(gl, 1, -1, "Tile 1");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_grid_list_add_tile(gl, 2, 2, "Featured");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_grid_list_add_tile(gl, 1, 1, "Standard");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Title NULL */
  rc = md3_grid_list_add_tile(gl, 1, 1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)gl->tile_count);
  ASSERT_STR_EQ("Featured", gl->tiles[0].title);
  ASSERT_EQ(2, gl->tiles[0].rowspan);
  ASSERT_EQ(2, gl->tiles[0].colspan);
  ASSERT_STR_EQ("", gl->tiles[2].title);

  /* Fill up to 64 tiles */
  for (i = 3; i < 64; i++) {
    rc = md3_grid_list_add_tile(gl, 1, 1, "Tile");
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceed 64 tiles -> UI_ERROR_OUT_OF_MEMORY */
  rc = md3_grid_list_add_tile(gl, 1, 1, "Over");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = md3_grid_list_destroy(gl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_grid_list_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_md3_surface_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_surface *surf = NULL;
  int lvl;
  ui_error_t rc;

  /* Null and invalid args */
  rc = md3_surface_create(NULL, MD3_SURFACE_ROLE_SURFACE, 1, &surf);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_surface_create(dummy_engine, MD3_SURFACE_ROLE_SURFACE, -1, &surf);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_surface_create(dummy_engine, MD3_SURFACE_ROLE_SURFACE, 6, &surf);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_surface_create(dummy_engine, MD3_SURFACE_ROLE_SURFACE, 1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Test all elevation levels 0 through 5 */
  for (lvl = 0; lvl <= 5; lvl++) {
    rc = md3_surface_create(dummy_engine, MD3_SURFACE_ROLE_SURFACE, lvl, &surf);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(surf != NULL);
    ASSERT_EQ(lvl, surf->elevation_level);
    rc = md3_surface_destroy(surf);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_surface_create(dummy_engine, MD3_SURFACE_ROLE_SURFACE_CONTAINER_HIGH,
                          2, &surf);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(surf != NULL);
  ASSERT_EQ(2, surf->elevation_level);
  ASSERT_EQ(MD3_SURFACE_ROLE_SURFACE_CONTAINER_HIGH, surf->role);

  /* Set elevation */
  rc = md3_surface_set_elevation(NULL, 3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_surface_set_elevation(surf, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_surface_set_elevation(surf, 7);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  for (lvl = 0; lvl <= 5; lvl++) {
    rc = md3_surface_set_elevation(surf, lvl);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(lvl, surf->elevation_level);
  }

  rc = md3_surface_destroy(surf);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_surface_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_md3_avatar_and_group(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_avatar *av1 = NULL;
  struct md3_avatar *av2 = NULL;
  struct md3_avatar *extra_av = NULL;
  struct md3_avatar_group *grp = NULL;
  size_t i;
  ui_error_t rc;

  /* Null checks */
  rc = md3_avatar_create(NULL, UI_AVATAR_TYPE_INITIALS, &av1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_avatar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &av1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(av1 != NULL);

  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_IMAGE, &av2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(av2 != NULL);

  /* Set avatar name and image */
  rc = md3_avatar_set_name(NULL, "John Doe");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_avatar_set_name(av1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_avatar_set_name(av1, "John Doe");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("John Doe", av1->name);

  rc = md3_avatar_set_image_url(NULL, "https://example.com/photo.jpg");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_avatar_set_image_url(av2, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_avatar_set_image_url(av2, "https://example.com/photo.jpg");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("https://example.com/photo.jpg", av2->image_url);
  ASSERT_EQ(UI_AVATAR_TYPE_IMAGE, av2->type);

  /* Avatar group */
  rc = md3_avatar_group_create(NULL, 3, &grp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_avatar_group_create(dummy_engine, -1, &grp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_avatar_group_create(dummy_engine, 3, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_avatar_group_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_avatar_group_create(dummy_engine, 3, &grp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(grp != NULL);

  rc = md3_avatar_group_append(NULL, av1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_avatar_group_append(grp, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Avatar with NULL base */
  {
    struct md3_avatar bad_av;
    bad_av.base = NULL;
    rc = md3_avatar_group_append(grp, &bad_av);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }

  rc = md3_avatar_group_append(grp, av1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_avatar_group_append(grp, av2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)grp->avatar_count);

  /* Fill up to 8 avatars */
  for (i = 2; i < 32; i++) {
    rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &extra_av);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_avatar_group_append(grp, extra_av);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceed 32 avatars -> UI_ERROR_OUT_OF_MEMORY */
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &extra_av);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_avatar_group_append(grp, extra_av);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  rc = md3_avatar_destroy(extra_av);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Clean up */
  for (i = 0; i < grp->avatar_count; i++) {
    rc = md3_avatar_destroy(grp->avatars[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_avatar_group_destroy(grp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_swipe_action_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_swipe_action *act = NULL;
  ui_error_t rc;

  rc = md3_swipe_action_create(NULL, &act);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_swipe_action_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_swipe_action_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_swipe_action_create(dummy_engine, &act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(act != NULL);
  ASSERT_EQ(0, act->is_dismissed);

  /* Swipe update */
  rc = md3_swipe_action_update(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_swipe_action_update(act, 150.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Swipe commit left reveal */
  act->base.threshold = 50.0f;
  rc = md3_swipe_action_commit(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_swipe_action_commit(act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, act->is_dismissed);

  /* Swipe commit right reveal */
  act->is_dismissed = 0;
  act->base.threshold = 50.0f;
  rc = md3_swipe_action_update(act, -300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_swipe_action_commit(act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, act->is_dismissed);

  /* Swipe commit idle (offset within threshold) */
  act->is_dismissed = 0;
  act->base.offset_x = 10.0f;
  act->base.threshold = 50.0f;
  rc = md3_swipe_action_commit(act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, act->is_dismissed);

  rc = md3_swipe_action_destroy(act);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_skeleton_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_skeleton *skel = NULL;
  ui_error_t rc;

  rc = md3_skeleton_create(NULL, MD3_SKELETON_CIRCULAR, &skel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_CIRCULAR, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_skeleton_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Circular skeleton */
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_CIRCULAR, &skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(skel != NULL);
  ASSERT_EQ(MD3_SKELETON_CIRCULAR, skel->variant);
  ASSERT_EQ(40.0f, skel->width);
  ASSERT_EQ(40.0f, skel->height);
  rc = md3_skeleton_destroy(skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Pill skeleton */
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_BUTTON_PILL, &skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(skel != NULL);
  ASSERT_EQ(MD3_SKELETON_BUTTON_PILL, skel->variant);
  ASSERT_EQ(120.0f, skel->width);
  ASSERT_EQ(20.0f, skel->height);
  rc = md3_skeleton_destroy(skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Text skeleton */
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_TEXT, &skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(skel != NULL);
  ASSERT_EQ(MD3_SKELETON_TEXT, skel->variant);
  rc = md3_skeleton_destroy(skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Rectangular skeleton */
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_RECTANGULAR, &skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(skel != NULL);
  ASSERT_EQ(MD3_SKELETON_RECTANGULAR, skel->variant);

  rc = md3_skeleton_set_size(NULL, 48.0f, 48.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_skeleton_set_size(skel, 0.0f, 48.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_skeleton_set_size(skel, -10.0f, 48.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_skeleton_set_size(skel, 48.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_skeleton_set_size(skel, 48.0f, -10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_skeleton_set_size(skel, 56.0f, 56.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(56.0f, skel->width);
  ASSERT_EQ(56.0f, skel->height);

  rc = md3_skeleton_destroy(skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_empty_state_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_empty_state *es = NULL;
  ui_error_t rc;

  rc = md3_empty_state_create(NULL, &es);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_empty_state_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_empty_state_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_empty_state_create(dummy_engine, &es);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(es != NULL);

  rc = md3_empty_state_set_content(NULL, "No Data", "Try refreshing");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_empty_state_set_content(es, NULL, "Try refreshing");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_empty_state_set_content(es, "No Orders",
                                   "You have not placed any orders.");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Orders", es->title);
  ASSERT_STR_EQ("You have not placed any orders.", es->description);

  /* Set content without description */
  rc = md3_empty_state_set_content(es, "No Orders", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Orders", es->title);
  ASSERT_STR_EQ("", es->description);

  rc = md3_empty_state_destroy(es);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_masonry_layout_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_masonry_layout *ml = NULL;
  ui_error_t rc;

  rc = md3_masonry_layout_create(NULL, 3, &ml);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_masonry_layout_create(dummy_engine, 0, &ml);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_masonry_layout_create(dummy_engine, -1, &ml);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_masonry_layout_create(dummy_engine, 3, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_masonry_layout_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_masonry_layout_create(dummy_engine, 3, &ml);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ml != NULL);
  ASSERT_EQ(3, ml->columns);

  rc = md3_masonry_layout_reflow(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  {
    struct md3_masonry_layout bad_ml;
    bad_ml.base = NULL;
    rc = md3_masonry_layout_reflow(&bad_ml);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }
  rc = md3_masonry_layout_reflow(ml);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_masonry_layout_destroy(ml);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_aspect_ratio_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_aspect_ratio *ar = NULL;
  ui_error_t rc;

  rc = md3_aspect_ratio_create(NULL, 16.0f / 9.0f, &ar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_aspect_ratio_create(dummy_engine, 0.0f, &ar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_aspect_ratio_create(dummy_engine, -1.0f, &ar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_aspect_ratio_create(dummy_engine, 16.0f / 9.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_aspect_ratio_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_aspect_ratio_create(dummy_engine, 16.0f / 9.0f, &ar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ar != NULL);

  rc = md3_aspect_ratio_set_ratio(NULL, 4.0f / 3.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  {
    struct md3_aspect_ratio bad_ar;
    bad_ar.base = NULL;
    rc = md3_aspect_ratio_set_ratio(&bad_ar, 1.5f);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }
  rc = md3_aspect_ratio_set_ratio(ar, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_aspect_ratio_set_ratio(ar, -2.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_aspect_ratio_set_ratio(ar, 4.0f / 3.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4.0f / 3.0f, ar->ratio);

  rc = md3_aspect_ratio_destroy(ar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_containment_mock_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_accordion *acc = NULL;
  struct md3_expansion_panel *panel = NULL;
  struct md3_grid_list *gl = NULL;
  struct md3_surface *surf = NULL;
  struct md3_avatar *av = NULL;
  struct md3_avatar_group *grp = NULL;
  struct md3_swipe_action *act = NULL;
  struct md3_skeleton *skel = NULL;
  struct md3_empty_state *es = NULL;
  struct md3_masonry_layout *ml = NULL;
  struct md3_aspect_ratio *ar = NULL;
  ui_error_t rc;

  /* Mock 1: accordion create fails */
  g_md3_containment_mock_fail = 1;
  rc = md3_accordion_create(dummy_engine, &acc);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 2: accordion destroy fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_accordion_create(dummy_engine, &acc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 2;
  rc = md3_accordion_destroy(acc);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 3: expansion panel create fails */
  g_md3_containment_mock_fail = 3;
  rc = md3_expansion_panel_create(dummy_engine, &panel);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 4: expansion panel destroy fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_expansion_panel_create(dummy_engine, &panel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 4;
  rc = md3_expansion_panel_destroy(panel);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 5: accordion add disclosure fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_accordion_create(dummy_engine, &acc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_expansion_panel_create(dummy_engine, &panel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 5;
  rc = md3_accordion_add_panel(acc, panel);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_expansion_panel_destroy(panel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_accordion_destroy(acc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 6: expansion panel set expanded fails */
  rc = md3_expansion_panel_create(dummy_engine, &panel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 6;
  rc = md3_expansion_panel_set_expanded(panel, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_expansion_panel_destroy(panel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 7: grid list create fails */
  g_md3_containment_mock_fail = 7;
  rc = md3_grid_list_create(dummy_engine, 3, &gl);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 8: grid list destroy fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_grid_list_create(dummy_engine, 3, &gl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 8;
  rc = md3_grid_list_destroy(gl);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 9: grid list add item fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_grid_list_create(dummy_engine, 3, &gl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 9;
  rc = md3_grid_list_add_tile(gl, 1, 1, "Tile");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_grid_list_destroy(gl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 10: surface create fails */
  g_md3_containment_mock_fail = 10;
  rc = md3_surface_create(dummy_engine, MD3_SURFACE_ROLE_SURFACE, 1, &surf);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 11: avatar create fails */
  g_md3_containment_mock_fail = 11;
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &av);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 12: avatar destroy fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &av);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 12;
  rc = md3_avatar_destroy(av);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 13: avatar set name fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &av);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 13;
  rc = md3_avatar_set_name(av, "Test");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_avatar_destroy(av);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 14: avatar set image url fails */
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_IMAGE, &av);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 14;
  rc = md3_avatar_set_image_url(av, "https://example.com/img.png");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_avatar_destroy(av);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 15: avatar group create fails */
  g_md3_containment_mock_fail = 15;
  rc = md3_avatar_group_create(dummy_engine, 3, &grp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 16: avatar group destroy fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_avatar_group_create(dummy_engine, 3, &grp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 16;
  rc = md3_avatar_group_destroy(grp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 17: avatar group append fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_avatar_group_create(dummy_engine, 3, &grp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &av);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 17;
  rc = md3_avatar_group_append(grp, av);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_avatar_destroy(av);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_avatar_group_destroy(grp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 18: swipe action update fails */
  rc = md3_swipe_action_create(dummy_engine, &act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 18;
  rc = md3_swipe_action_update(act, 10.0f);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 19: swipe action commit fails */
  g_md3_containment_mock_fail = 19;
  rc = md3_swipe_action_commit(act);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_swipe_action_destroy(act);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 20: skeleton create fails */
  g_md3_containment_mock_fail = 20;
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_CIRCULAR, &skel);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 21: skeleton destroy fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_CIRCULAR, &skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 21;
  rc = md3_skeleton_destroy(skel);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 22: skeleton set dimensions fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_CIRCULAR, &skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 22;
  rc = md3_skeleton_set_size(skel, 50.0f, 50.0f);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_skeleton_destroy(skel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 23: empty state create fails */
  g_md3_containment_mock_fail = 23;
  rc = md3_empty_state_create(dummy_engine, &es);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 24: empty state set title fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_empty_state_create(dummy_engine, &es);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 24;
  rc = md3_empty_state_set_content(es, "Title", "Desc");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 25: empty state set description fails */
  g_md3_containment_mock_fail = 25;
  rc = md3_empty_state_set_content(es, "Title", "Desc");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_empty_state_destroy(es);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 26: masonry create fails */
  g_md3_containment_mock_fail = 26;
  rc = md3_masonry_layout_create(dummy_engine, 3, &ml);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 27: masonry destroy fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_masonry_layout_create(dummy_engine, 3, &ml);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 27;
  rc = md3_masonry_layout_destroy(ml);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 28: masonry reflow fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_masonry_layout_create(dummy_engine, 3, &ml);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 28;
  rc = md3_masonry_layout_reflow(ml);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_masonry_layout_destroy(ml);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 29: aspect ratio create fails */
  g_md3_containment_mock_fail = 29;
  rc = md3_aspect_ratio_create(dummy_engine, 1.5f, &ar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 30: aspect ratio destroy fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_aspect_ratio_create(dummy_engine, 1.5f, &ar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 30;
  rc = md3_aspect_ratio_destroy(ar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 31: aspect ratio set ratio fails */
  g_md3_containment_mock_fail = 0;
  rc = md3_aspect_ratio_create(dummy_engine, 1.5f, &ar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 31;
  rc = md3_aspect_ratio_set_ratio(ar, 2.0f);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_aspect_ratio_destroy(ar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 32: avatar get initials fails */
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &av);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_containment_mock_fail = 32;
  rc = md3_avatar_set_name(av, "John Doe");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_containment_mock_fail = 0;
  rc = md3_avatar_destroy(av);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Reset mock fail */
  g_md3_containment_mock_fail = 0;
#endif

  PASS();
}

TEST test_md3_containment_oom_alloc(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_accordion *acc = NULL;
  struct md3_expansion_panel *panel = NULL;
  struct md3_grid_list *gl = NULL;
  struct md3_surface *surf = NULL;
  struct md3_avatar *av = NULL;
  struct md3_avatar_group *grp = NULL;
  struct md3_swipe_action *act = NULL;
  struct md3_skeleton *skel = NULL;
  struct md3_empty_state *es = NULL;
  struct md3_masonry_layout *ml = NULL;
  struct md3_aspect_ratio *ar = NULL;
  ui_error_t rc;

  g_malloc_fail_countdown = 0;
  rc = md3_accordion_create(dummy_engine, &acc);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_expansion_panel_create(dummy_engine, &panel);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_grid_list_create(dummy_engine, 3, &gl);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_surface_create(dummy_engine, MD3_SURFACE_ROLE_SURFACE, 1, &surf);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_avatar_create(dummy_engine, UI_AVATAR_TYPE_INITIALS, &av);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_avatar_group_create(dummy_engine, 3, &grp);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_swipe_action_create(dummy_engine, &act);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_skeleton_create(dummy_engine, MD3_SKELETON_CIRCULAR, &skel);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_empty_state_create(dummy_engine, &es);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_masonry_layout_create(dummy_engine, 3, &ml);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_aspect_ratio_create(dummy_engine, 1.5f, &ar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
#endif

  PASS();
}

SUITE(md3_containment_suite) {
  RUN_TEST(test_md3_accordion_and_panel);
  RUN_TEST(test_md3_grid_list_lifecycle);
  RUN_TEST(test_md3_surface_lifecycle);
  RUN_TEST(test_md3_avatar_and_group);
  RUN_TEST(test_md3_swipe_action_lifecycle);
  RUN_TEST(test_md3_skeleton_lifecycle);
  RUN_TEST(test_md3_empty_state_lifecycle);
  RUN_TEST(test_md3_masonry_layout_lifecycle);
  RUN_TEST(test_md3_aspect_ratio_lifecycle);
  RUN_TEST(test_md3_containment_mock_failures);
  RUN_TEST(test_md3_containment_oom_alloc);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  int result;
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_containment_suite);
  GREATEST_MAIN_END();
  return result;
}
