/**
 * @file test_material3_media_workspace.c
 * @brief Unit tests for Material 3 Iconography, Media, Workflow, and Desktop
 * Workspace Shells.
 */

/* clang-format off */
#include "material3/md3_media_workspace.h"
#include "ui_engine.h"
#include "ui_test_mock_mem.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md3_media_workspace_mock_fail;
extern int g_malloc_fail_countdown;
#endif

TEST test_md3_icon_lifecycle_and_features(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_icon *icon = NULL;
  ui_error_t rc;

  /* Null checks */
  rc = md3_icon_create(NULL, &icon);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_create(dummy_engine, &icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(icon != NULL);

  /* Style */
  rc = md3_icon_set_style(NULL, MD3_ICON_FILLED);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_style(icon, MD3_ICON_FILLED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_ICON_FILLED, icon->style);

  /* Glyph and SVG path */
  rc = md3_icon_set_glyph(NULL, "settings");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_glyph(icon, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_glyph(icon, "settings");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("settings", icon->current_glyph);

  rc = md3_icon_set_svg_path(NULL, "M0 0h24v24H0z");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_svg_path(icon, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_svg_path(icon, "M0 0h24v24H0z");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Font axes */
  rc = md3_icon_set_font_axes(NULL, 500, 1, 0, 24.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 50, 1, 0, 24.0f); /* Out of bounds */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 500, 1, 0, 24.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(500, icon->font_weight);

  /* Size */
  rc = md3_icon_set_size(NULL, 32.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_size(icon, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_size(icon, 32.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(32.0f, icon->size_dp);

  /* RTL mirroring */
  rc = md3_icon_set_rtl_mirroring(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_rtl_mirroring(icon, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, icon->rtl_mirroring);

  /* Semantic & Decorative */
  rc = md3_icon_set_semantic(NULL, 1, "Settings");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_semantic(icon, 1, NULL); /* missing aria-label */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_semantic(icon, 1, "Settings Icon");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, icon->is_semantic);
  ASSERT_STR_EQ("Settings Icon", icon->aria_label);

  rc = md3_icon_set_semantic(icon, 0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, icon->is_semantic);

  /* Dynamic morphing */
  rc = md3_icon_morph_to(NULL, "M10 10", 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_morph_to(icon, NULL, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_morph_to(icon, "M10 10", 1.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_morph_to(icon, "M10 10", -0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_morph_to(icon, "M10 10", 0.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, icon->is_morphing);

  /* Clean up */
  rc = md3_icon_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_destroy(icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_chat_bubble_lifecycle_and_features(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_chat_bubble *bubble = NULL;
  ui_error_t rc;

  /* Null checks */
  rc = md3_chat_bubble_create(NULL, MD3_CHAT_INCOMING, &bubble);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_create(dummy_engine, MD3_CHAT_INCOMING, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chat_bubble_create(dummy_engine, MD3_CHAT_INCOMING, &bubble);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bubble != NULL);
  ASSERT_EQ(MD3_CHAT_INCOMING, bubble->direction);

  /* Message content */
  rc = md3_chat_bubble_set_message(NULL, "Hello!", "10:30 AM", MD3_MSG_SENT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_set_message(bubble, "Hello world", "10:30 AM",
                                   MD3_MSG_DELIVERED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Hello world", bubble->message_text);
  ASSERT_STR_EQ("10:30 AM", bubble->timestamp);
  ASSERT_EQ(MD3_MSG_DELIVERED, bubble->status);

  /* Reactions */
  rc = md3_chat_bubble_add_reaction(NULL, "👍", 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_add_reaction(bubble, "👍", 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)bubble->reaction_count);
  ASSERT_STR_EQ("👍", bubble->reactions[0].emoji);
  ASSERT_EQ(2, bubble->reactions[0].count);

  /* Expressive */
  rc = md3_chat_bubble_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_set_expressive(bubble, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, bubble->is_expressive);

  /* Clean up */
  rc = md3_chat_bubble_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_destroy(bubble);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_timeline_lifecycle_and_nodes(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_timeline *timeline = NULL;
  ui_error_t rc;

  /* Null checks */
  rc = md3_timeline_create(NULL, MD3_TIMELINE_VERTICAL, &timeline);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timeline_create(dummy_engine, MD3_TIMELINE_VERTICAL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_timeline_create(dummy_engine, MD3_TIMELINE_VERTICAL, &timeline);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(timeline != NULL);

  /* Add node */
  rc = md3_timeline_add_node(NULL, "Order Placed", "Confirmed",
                             MD3_MARKER_FILLED, MD3_LINE_SOLID);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timeline_add_node(timeline, "Order Placed", "Confirmed",
                             MD3_MARKER_FILLED, MD3_LINE_SOLID);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)timeline->item_count);
  ASSERT_STR_EQ("Order Placed", timeline->items[0].title);

  /* Opposing and Expressive */
  rc = md3_timeline_set_opposing(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timeline_set_opposing(timeline, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, timeline->is_opposing);

  rc = md3_timeline_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timeline_set_expressive(timeline, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, timeline->is_expressive);

  /* Clean up */
  rc = md3_timeline_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timeline_destroy(timeline);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_file_uploader_lifecycle_and_progress(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_file_uploader *uploader = NULL;
  ui_error_t rc;

  /* Null checks */
  rc = md3_file_uploader_create(NULL, &uploader);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_file_uploader_create(dummy_engine, &uploader);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(uploader != NULL);

  /* Max files & Formats */
  rc = md3_file_uploader_set_max_files(NULL, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_max_files(uploader, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_max_files(uploader, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, uploader->max_files);

  rc = md3_file_uploader_set_allowed_formats(NULL, ".png,.jpg");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_allowed_formats(uploader, ".png,.jpg");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ(".png,.jpg", uploader->allowed_formats);

  /* Add file & progress */
  rc = md3_file_uploader_add_file(NULL, "photo.png", 1024);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_add_file(uploader, "photo.png", 1024);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)uploader->file_count);
  ASSERT_STR_EQ("photo.png", uploader->files[0].file_name);

  rc = md3_file_uploader_set_file_progress(NULL, 0, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_file_progress(uploader, 0, 1.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_file_progress(uploader, 0, 0.75f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.75f, uploader->files[0].progress);

  /* Drag over & Expressive */
  rc = md3_file_uploader_set_drag_over(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_drag_over(uploader, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, uploader->is_drag_over);

  rc = md3_file_uploader_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_expressive(uploader, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, uploader->is_expressive);

  /* Clean up */
  rc = md3_file_uploader_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_destroy(uploader);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_video_player_lifecycle_and_controls(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_video_player *player = NULL;
  int is_ltr = 0;
  ui_error_t rc;

  /* Null checks */
  rc = md3_video_player_create(NULL, &player);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_video_player_create(dummy_engine, &player);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(player != NULL);

  /* Playback & Volume */
  rc = md3_video_player_play(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_play(player);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, player->base.is_playing);

  rc = md3_video_player_pause(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_pause(player);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, player->base.is_playing);

  rc = md3_video_player_seek(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_seek(player, -2.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_seek(player, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_video_player_set_volume(NULL, 0.8f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_set_volume(player, 1.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_set_volume(player, 0.8f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_video_player_set_playback_speed(NULL, 1.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_set_playback_speed(player, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_set_playback_speed(player, 1.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.5f, player->playback_speed);

  /* Fullscreen & LTR Invariant */
  rc = md3_video_player_set_fullscreen(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_set_fullscreen(player, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, player->base.is_fullscreen);

  rc = md3_video_player_get_playback_direction(NULL, &is_ltr);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_get_playback_direction(player, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_get_playback_direction(player, &is_ltr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_ltr);

  /* Expressive */
  rc = md3_video_player_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_set_expressive(player, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, player->is_expressive);

  /* Clean up */
  rc = md3_video_player_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_destroy(player);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_rich_text_editor_lifecycle_and_features(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_rich_text_editor *editor = NULL;
  size_t words = 0;
  size_t chars = 0;
  ui_error_t rc;

  /* Null checks */
  rc = md3_rich_text_editor_create(NULL, &editor);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_rich_text_editor_create(dummy_engine, &editor);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(editor != NULL);

  /* Text & Formatting */
  rc = md3_rich_text_editor_set_text(NULL, "Hello world");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_set_text(
      editor, "The quick brown fox jumps over the lazy dog");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_rich_text_editor_apply_format(NULL, "bold");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_apply_format(editor, "bold");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Word & char count */
  rc = md3_rich_text_editor_get_word_count(NULL, &words, &chars);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_get_word_count(editor, &words, &chars);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(9, (int)words);
  ASSERT_EQ(43, (int)chars);

  /* Undo / Redo */
  rc = md3_rich_text_editor_undo(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_undo(editor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_rich_text_editor_redo(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_redo(editor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Expressive */
  rc = md3_rich_text_editor_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_set_expressive(editor, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, editor->is_expressive);

  /* Clean up */
  rc = md3_rich_text_editor_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_destroy(editor);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_section_index_lifecycle_and_features(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_section_index *index = NULL;
  const char *sections[3] = {"A", "B", "C"};
  int active = -1;
  ui_error_t rc;

  /* Null checks */
  rc = md3_section_index_create(NULL, &index);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_section_index_create(dummy_engine, &index);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(index != NULL);

  /* Sections */
  rc = md3_section_index_set_sections(NULL, sections, 3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_set_sections(index, sections, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)index->section_count);

  /* Active section */
  rc = md3_section_index_set_active_section(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_set_active_section(index, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_section_index_get_active_section(NULL, &active);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_get_active_section(index, &active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, active);

  /* Magnifier & RTL */
  rc = md3_section_index_set_magnifier(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_set_magnifier(index, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, index->show_magnifier);

  rc = md3_section_index_set_rtl(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_set_rtl(index, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, index->is_rtl);

  /* Expressive */
  rc = md3_section_index_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_set_expressive(index, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, index->is_expressive);

  /* Clean up */
  rc = md3_section_index_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_destroy(index);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_dockable_layout_lifecycle_and_features(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_dockable_layout *layout = NULL;
  char buf[256];
  ui_error_t rc;

  /* Null checks */
  rc = md3_dockable_layout_create(NULL, &layout);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dockable_layout_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_dockable_layout_create(dummy_engine, &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(layout != NULL);

  /* Dock and remove panel */
  rc = md3_dockable_layout_dock_panel(NULL, 1, 0, UI_DOCK_EDGE_LEFT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dockable_layout_dock_panel(layout, 1, 0, UI_DOCK_EDGE_LEFT);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_dockable_layout_serialize(NULL, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dockable_layout_serialize(layout, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_dockable_layout_remove_panel(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dockable_layout_remove_panel(layout, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Expressive */
  rc = md3_dockable_layout_set_expressive(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dockable_layout_set_expressive(layout, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, layout->is_expressive);

  /* Clean up */
  rc = md3_dockable_layout_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dockable_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_media_workspace_branch_coverage(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_icon *icon = NULL;
  struct md3_chat_bubble *bubble = NULL;
  struct md3_timeline *tl = NULL;
  struct md3_file_uploader *uploader = NULL;
  struct md3_video_player *player = NULL;
  struct md3_rich_text_editor *rte = NULL;
  struct md3_section_index *si = NULL;
  struct md3_dockable_layout *layout = NULL;
  int i;
  size_t words = 0;
  size_t chars = 0;
  char buf[64];
  ui_error_t rc;

  /* 1. md3_icon branches */
  rc = md3_icon_create(dummy_engine, &icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* md3_icon_set_font_axes bounds */
  rc = md3_icon_set_font_axes(icon, 99, 0, 0, 24.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 901, 0, 0, 24.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 400, -1, 0, 24.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 400, 2, 0, 24.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 400, 0, -51, 24.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 400, 0, 201, 24.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 400, 0, 0, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_set_font_axes(icon, 400, 0, 0, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* md3_icon_set_rtl_mirroring 0 */
  rc = md3_icon_set_rtl_mirroring(icon, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, icon->rtl_mirroring);

  /* md3_icon_set_semantic with 0 and non-null */
  rc = md3_icon_set_semantic(icon, 0, "Ignored");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, icon->is_semantic);
  ASSERT_EQ('\0', icon->aria_label[0]);

  /* md3_icon_morph_to with 0.0f and 1.0f */
  rc = md3_icon_morph_to(icon, "M0 0", 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, icon->is_morphing);
  rc = md3_icon_morph_to(icon, "M0 0", 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, icon->is_morphing);

  /* md3_icon_set_glyph and svg_path with NULL icon->base */
  {
    struct ui_icon_base *saved_base = icon->base;
    icon->base = NULL;
    rc = md3_icon_set_glyph(icon, "test");
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_icon_set_svg_path(icon, "M0 0");
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    /* md3_icon_destroy with NULL base */
    rc = md3_icon_destroy(icon);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* restore and destroy base manually */
    rc = ui_icon_base_destroy(saved_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 2. md3_chat_bubble branches */
  rc = md3_chat_bubble_create(dummy_engine, MD3_CHAT_INCOMING, &bubble);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_CHAT_INCOMING, bubble->direction);

  /* NULL checks in set_message */
  rc = md3_chat_bubble_set_message(NULL, "hi", "now", MD3_MSG_SENT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_set_message(bubble, NULL, "now", MD3_MSG_SENT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_set_message(bubble, "hi", NULL, MD3_MSG_SENT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* add_reaction bounds */
  rc = md3_chat_bubble_add_reaction(NULL, "👍", 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_add_reaction(bubble, NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_add_reaction(bubble, "👍", 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chat_bubble_add_reaction(bubble, "👍", -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Fill up reactions to 8, then 9th fails */
  for (i = 0; i < 8; ++i) {
    rc = md3_chat_bubble_add_reaction(bubble, "👍", i + 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_chat_bubble_add_reaction(bubble, "👍", 9);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* md3_chat_bubble_set_expressive 0 */
  rc = md3_chat_bubble_set_expressive(bubble, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, bubble->is_expressive);

  /* md3_chat_bubble_destroy with NULL base and NULL arena */
  {
    struct ui_chat_bubble_base *saved_base = bubble->base;
    struct ui_arena *saved_arena = bubble->arena;
    bubble->base = NULL;
    bubble->arena = NULL;
    rc = md3_chat_bubble_destroy(bubble);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_chat_bubble_base_destroy(saved_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_arena_destroy(saved_arena);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 3. md3_timeline branches */
  rc = md3_timeline_create(dummy_engine, MD3_TIMELINE_VERTICAL, &tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_timeline_set_opposing(tl, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, tl->is_opposing);

  rc = md3_timeline_set_expressive(tl, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, tl->is_expressive);

  /* add_node NULL checks */
  rc = md3_timeline_add_node(NULL, "T", "D", MD3_MARKER_FILLED, MD3_LINE_SOLID);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timeline_add_node(tl, NULL, "D", MD3_MARKER_FILLED, MD3_LINE_SOLID);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timeline_add_node(tl, "T", NULL, MD3_MARKER_FILLED, MD3_LINE_SOLID);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Fill up items to 32 */
  for (i = 0; i < 32; ++i) {
    rc = md3_timeline_add_node(tl, "T", "D", MD3_MARKER_FILLED, MD3_LINE_SOLID);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* 33rd node is added to base, but item_count cap at 32 is hit */
  rc = md3_timeline_add_node(tl, "T", "D", MD3_MARKER_FILLED, MD3_LINE_SOLID);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(32, tl->item_count);

  /* destroy with NULL base */
  {
    struct ui_timeline_base *saved_base = tl->base;
    tl->base = NULL;
    rc = md3_timeline_add_node(tl, "T", "D", MD3_MARKER_FILLED, MD3_LINE_SOLID);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_timeline_destroy(tl);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_timeline_base_destroy(saved_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 4. md3_file_uploader branches */
  rc = md3_file_uploader_create(dummy_engine, &uploader);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_file_uploader_set_max_files(uploader, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_max_files(uploader, -5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_file_uploader_set_allowed_formats(uploader, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', uploader->allowed_formats[0]);

  rc = md3_file_uploader_set_allowed_formats(uploader, ".png,.jpg");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_file_uploader_set_max_files(uploader, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_file_uploader_add_file(uploader, NULL, 100);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_add_file(uploader, "f1.png", 100);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_file_uploader_add_file(uploader, "f2.png", 200);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* exceeds max_files */
  rc = md3_file_uploader_add_file(uploader, "f3.png", 300);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Test file count cap at 32 */
  rc = md3_file_uploader_set_max_files(uploader, 50);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  for (i = 2; i < 32; ++i) {
    rc = md3_file_uploader_add_file(uploader, "more.png", 100);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* 33rd file exceeds 32 limit even though max_files is 50 */
  rc = md3_file_uploader_add_file(uploader, "overflow.png", 100);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* progress bounds */
  rc = md3_file_uploader_set_file_progress(uploader, -1, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_file_progress(uploader, 50, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_file_progress(uploader, 0, -0.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_file_uploader_set_file_progress(uploader, 0, 1.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_file_uploader_set_drag_over(uploader, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, uploader->is_drag_over);

  rc = md3_file_uploader_set_expressive(uploader, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, uploader->is_expressive);

  rc = md3_file_uploader_destroy(uploader);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 5. md3_video_player branches */
  rc = md3_video_player_create(dummy_engine, &player);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_video_player_seek(player, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_video_player_set_volume(player, -0.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_set_volume(player, 1.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_video_player_set_playback_speed(player, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_video_player_set_playback_speed(player, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_video_player_set_fullscreen(player, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, player->base.is_fullscreen);

  rc = md3_video_player_set_expressive(player, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, player->is_expressive);

  rc = md3_video_player_get_playback_direction(player, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_video_player_destroy(player);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 6. md3_rich_text_editor branches */
  rc = md3_rich_text_editor_create(dummy_engine, &rte);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_rich_text_editor_set_text(rte, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', rte->text_buffer[0]);

  /* Word count with multiple words and spaces/tabs/newlines */
  rc = md3_rich_text_editor_set_text(rte, "  hello \t world \n test  ");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_rich_text_editor_get_word_count(rte, &words, &chars);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)words);

  rc = md3_rich_text_editor_get_word_count(NULL, &words, &chars);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_get_word_count(rte, NULL, &chars);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_get_word_count(rte, &words, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_rich_text_editor_apply_format(NULL, "b");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rich_text_editor_apply_format(rte, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_rich_text_editor_set_expressive(rte, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, rte->is_expressive);

  /* undo/redo with NULL base */
  {
    struct ui_rich_text_editor_base *saved_base = rte->base;
    rte->base = NULL;
    rc = md3_rich_text_editor_undo(rte);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_rich_text_editor_redo(rte);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_rich_text_editor_destroy(rte);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_rich_text_editor_base_destroy(saved_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 7. md3_section_index branches */
  rc = md3_section_index_create(dummy_engine, &si);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_section_index_set_sections(NULL, NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_section_index_set_sections(si, NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_section_index_get_active_section(si, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_section_index_set_magnifier(si, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, si->show_magnifier);

  rc = md3_section_index_set_rtl(si, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, si->is_rtl);

  rc = md3_section_index_set_expressive(si, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, si->is_expressive);

  /* NULL base */
  {
    struct ui_section_index_base *saved_base = si->base;
    const char *sec[] = {"A", "B"};
    si->base = NULL;
    rc = md3_section_index_set_sections(si, sec, 2);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_section_index_set_active_section(si, 0);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_section_index_destroy(si);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_section_index_base_destroy(saved_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 8. md3_dockable_layout branches */
  rc = md3_dockable_layout_create(dummy_engine, &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_dockable_layout_serialize(layout, NULL, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_dockable_layout_set_expressive(layout, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, layout->is_expressive);

  /* NULL base */
  {
    struct ui_dockable_layout_base *saved_base = layout->base;
    layout->base = NULL;
    rc = md3_dockable_layout_dock_panel(layout, 1, 0, UI_DOCK_EDGE_LEFT);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_dockable_layout_remove_panel(layout, 1);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_dockable_layout_serialize(layout, buf, sizeof(buf));
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_dockable_layout_destroy(layout);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dockable_layout_base_destroy(saved_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  PASS();
}

TEST test_md3_media_workspace_mock_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_icon *icon = NULL;
  struct md3_chat_bubble *bubble = NULL;
  struct md3_timeline *tl = NULL;
  struct md3_file_uploader *uploader = NULL;
  struct md3_video_player *player = NULL;
  struct md3_rich_text_editor *rte = NULL;
  struct md3_section_index *si = NULL;
  struct md3_dockable_layout *layout = NULL;
  const char *sec[] = {"A"};
  ui_error_t rc;

  /* Mock 1: icon base create fails */
  g_md3_media_workspace_mock_fail = 1;
  rc = md3_icon_create(dummy_engine, &icon);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 2: icon base destroy fails */
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_icon_create(dummy_engine, &icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 2;
  rc = md3_icon_destroy(icon);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_icon_destroy(icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 3: icon set glyph fails */
  rc = md3_icon_create(dummy_engine, &icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 3;
  rc = md3_icon_set_glyph(icon, "star");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_icon_destroy(icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 4: chat bubble arena create fails */
  g_md3_media_workspace_mock_fail = 4;
  rc = md3_chat_bubble_create(dummy_engine, MD3_CHAT_INCOMING, &bubble);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 5: chat bubble base create fails */
  g_md3_media_workspace_mock_fail = 5;
  rc = md3_chat_bubble_create(dummy_engine, MD3_CHAT_INCOMING, &bubble);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 6: chat bubble base destroy fails */
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_chat_bubble_create(dummy_engine, MD3_CHAT_INCOMING, &bubble);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 6;
  rc = md3_chat_bubble_destroy(bubble);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 7: chat bubble arena destroy fails */
  g_md3_media_workspace_mock_fail = 7;
  rc = md3_chat_bubble_destroy(bubble);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_chat_bubble_destroy(bubble);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 8: timeline base create fails */
  g_md3_media_workspace_mock_fail = 8;
  rc = md3_timeline_create(dummy_engine, MD3_TIMELINE_VERTICAL, &tl);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 9: timeline base destroy fails */
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_timeline_create(dummy_engine, MD3_TIMELINE_VERTICAL, &tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 9;
  rc = md3_timeline_destroy(tl);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_timeline_destroy(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 10: timeline add node fails */
  rc = md3_timeline_create(dummy_engine, MD3_TIMELINE_VERTICAL, &tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 10;
  rc = md3_timeline_add_node(tl, "T", "D", MD3_MARKER_FILLED, MD3_LINE_SOLID);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_timeline_destroy(tl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 11: file uploader init fails */
  g_md3_media_workspace_mock_fail = 11;
  rc = md3_file_uploader_create(dummy_engine, &uploader);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 12: file uploader destroy fails */
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_file_uploader_create(dummy_engine, &uploader);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 12;
  rc = md3_file_uploader_destroy(uploader);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_file_uploader_destroy(uploader);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 13: video player init fails */
  g_md3_media_workspace_mock_fail = 13;
  rc = md3_video_player_create(dummy_engine, &player);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 14: rich text editor base create fails */
  g_md3_media_workspace_mock_fail = 14;
  rc = md3_rich_text_editor_create(dummy_engine, &rte);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 15: rich text editor base destroy fails */
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_rich_text_editor_create(dummy_engine, &rte);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 15;
  rc = md3_rich_text_editor_destroy(rte);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_rich_text_editor_destroy(rte);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 16: section index base create fails */
  g_md3_media_workspace_mock_fail = 16;
  rc = md3_section_index_create(dummy_engine, &si);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 17: section index base destroy fails */
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_section_index_create(dummy_engine, &si);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 17;
  rc = md3_section_index_destroy(si);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_section_index_destroy(si);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 18: section index set sections fails */
  rc = md3_section_index_create(dummy_engine, &si);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 18;
  rc = md3_section_index_set_sections(si, sec, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;

  /* Mock 19: section index set active section fails */
  g_md3_media_workspace_mock_fail = 19;
  rc = md3_section_index_set_active_section(si, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_section_index_destroy(si);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 20: dockable layout create fails */
  g_md3_media_workspace_mock_fail = 20;
  rc = md3_dockable_layout_create(dummy_engine, &layout);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 21: dockable layout destroy fails */
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_dockable_layout_create(dummy_engine, &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_media_workspace_mock_fail = 21;
  rc = md3_dockable_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_media_workspace_mock_fail = 0;
  rc = md3_dockable_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#endif
  PASS();
}

TEST test_md3_media_workspace_oom_alloc(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_icon *icon = NULL;
  struct md3_chat_bubble *bubble = NULL;
  struct md3_timeline *tl = NULL;
  struct md3_file_uploader *uploader = NULL;
  struct md3_video_player *player = NULL;
  struct md3_rich_text_editor *rte = NULL;
  struct md3_section_index *si = NULL;
  struct md3_dockable_layout *layout = NULL;
  ui_error_t rc;

  /* Icon create OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_icon_create(dummy_engine, &icon);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Chat bubble create OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_chat_bubble_create(dummy_engine, MD3_CHAT_INCOMING, &bubble);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Timeline create OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_timeline_create(dummy_engine, MD3_TIMELINE_VERTICAL, &tl);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* File uploader create OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_file_uploader_create(dummy_engine, &uploader);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Video player create OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_video_player_create(dummy_engine, &player);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Rich text editor create OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_rich_text_editor_create(dummy_engine, &rte);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Section index create OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_section_index_create(dummy_engine, &si);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Dockable layout create OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_dockable_layout_create(dummy_engine, &layout);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif
  PASS();
}

SUITE(suite_material3_media_workspace) {
  RUN_TEST(test_md3_icon_lifecycle_and_features);
  RUN_TEST(test_md3_chat_bubble_lifecycle_and_features);
  RUN_TEST(test_md3_timeline_lifecycle_and_nodes);
  RUN_TEST(test_md3_file_uploader_lifecycle_and_progress);
  RUN_TEST(test_md3_video_player_lifecycle_and_controls);
  RUN_TEST(test_md3_rich_text_editor_lifecycle_and_features);
  RUN_TEST(test_md3_section_index_lifecycle_and_features);
  RUN_TEST(test_md3_dockable_layout_lifecycle_and_features);
  RUN_TEST(test_md3_media_workspace_branch_coverage);
  RUN_TEST(test_md3_media_workspace_mock_failures);
  RUN_TEST(test_md3_media_workspace_oom_alloc);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(suite_material3_media_workspace);
  GREATEST_MAIN_END();
}
