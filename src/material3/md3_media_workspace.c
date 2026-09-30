/**
 * @file md3_media_workspace.c
 * @brief Implementation of Material 3 Iconography, Media, Workflow, and Desktop
 * Workspace Shells.
 */

/* clang-format off */
#include "material3/md3_media_workspace.h"
#include "ui_engine.h"
#include "ui_internal_mem.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Global flag to simulate failures in mocked dependencies. */
int g_md3_media_workspace_mock_fail = 0;

/**
 * @brief Mock for ui_icon_base_create.
 * @param out Output icon pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_icon_base_create(struct ui_icon_base **out) {
  if (g_md3_media_workspace_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_icon_base_create(out);
}

/**
 * @brief Mock for ui_icon_base_destroy.
 * @param b Icon pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_icon_base_destroy(struct ui_icon_base *b) {
  if (g_md3_media_workspace_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_icon_base_destroy(b);
}

/**
 * @brief Mock for ui_icon_base_set_font_glyph.
 * @param b Icon pointer.
 * @param f Font pointer.
 * @param g Glyph string.
 * @return ui_error_t result code.
 */
static ui_error_t mock_icon_base_set_font_glyph(struct ui_icon_base *b,
                                                struct ui_font *f,
                                                const char *g) {
  if (g_md3_media_workspace_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_icon_base_set_font_glyph(b, f, g);
}

/**
 * @brief Mock for ui_arena_create.
 * @param s Arena initial size.
 * @param out Output arena pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_arena_create(size_t s, struct ui_arena **out) {
  if (g_md3_media_workspace_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_arena_create(s, out);
}

/**
 * @brief Mock for ui_chat_bubble_base_create.
 * @param a Arena pointer.
 * @param c Configuration pointer.
 * @param out Output chat bubble pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_chat_bubble_base_create(struct ui_arena *a,
                             const struct ui_chat_bubble_config *c,
                             struct ui_chat_bubble_base **out) {
  if (g_md3_media_workspace_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_chat_bubble_base_create(a, c, out);
}

/**
 * @brief Mock for ui_chat_bubble_base_destroy.
 * @param b Chat bubble pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_chat_bubble_base_destroy(struct ui_chat_bubble_base *b) {
  if (g_md3_media_workspace_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_chat_bubble_base_destroy(b);
}

/**
 * @brief Mock for ui_arena_destroy.
 * @param a Arena pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_arena_destroy(struct ui_arena *a) {
  if (g_md3_media_workspace_mock_fail == 7) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_arena_destroy(a);
}

/**
 * @brief Mock for ui_timeline_base_create.
 * @param out Output timeline pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_timeline_base_create(struct ui_timeline_base **out) {
  if (g_md3_media_workspace_mock_fail == 8) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_timeline_base_create(out);
}

/**
 * @brief Mock for ui_timeline_base_destroy.
 * @param b Timeline pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_timeline_base_destroy(struct ui_timeline_base *b) {
  if (g_md3_media_workspace_mock_fail == 9) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_timeline_base_destroy(b);
}

/**
 * @brief Mock for ui_timeline_base_add_node.
 * @param b Timeline pointer.
 * @param t Title string.
 * @param d Description string.
 * @return ui_error_t result code.
 */
static ui_error_t mock_timeline_base_add_node(struct ui_timeline_base *b,
                                              const char *t, const char *d) {
  if (g_md3_media_workspace_mock_fail == 10) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_timeline_base_add_node(b, t, d);
}

/**
 * @brief Mock for ui_file_uploader_init.
 * @param b Uploader base pointer.
 * @param mf Max files count.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param w Width.
 * @param h Height.
 * @param c Component pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_file_uploader_init(struct ui_file_uploader_base *b,
                                          int mf, int x, int y, int w, int h,
                                          struct ui_control_value_accessor *c) {
  if (g_md3_media_workspace_mock_fail == 11) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_file_uploader_init(b, mf, x, y, w, h, c);
}

/**
 * @brief Mock for ui_file_uploader_destroy.
 * @param b Uploader base pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_file_uploader_destroy(struct ui_file_uploader_base *b) {
  if (g_md3_media_workspace_mock_fail == 12) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_file_uploader_destroy(b);
}

/**
 * @brief Mock for ui_video_player_base_init.
 * @param b Video player base pointer.
 * @param c Component pointer.
 * @param u URL or path string.
 * @return ui_error_t result code.
 */
static ui_error_t mock_video_player_base_init(struct ui_video_player_base *b,
                                              struct ui_component *c,
                                              struct ui_av_sync *u) {
  if (g_md3_media_workspace_mock_fail == 13) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_video_player_base_init(b, c, u);
}

/**
 * @brief Mock for ui_rich_text_editor_base_create.
 * @param out Output editor pointer.
 * @param c Component pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_rich_text_editor_base_create(struct ui_rich_text_editor_base **out,
                                  struct ui_control_value_accessor *c) {
  if (g_md3_media_workspace_mock_fail == 14) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_rich_text_editor_base_create(out, c);
}

/**
 * @brief Mock for ui_rich_text_editor_base_destroy.
 * @param b Editor pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_rich_text_editor_base_destroy(struct ui_rich_text_editor_base *b) {
  if (g_md3_media_workspace_mock_fail == 15) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_rich_text_editor_base_destroy(b);
}

/**
 * @brief Mock for ui_section_index_base_create.
 * @param out Output index pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_section_index_base_create(struct ui_section_index_base **out) {
  if (g_md3_media_workspace_mock_fail == 16) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_section_index_base_create(out);
}

/**
 * @brief Mock for ui_section_index_base_destroy.
 * @param b Index pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_section_index_base_destroy(struct ui_section_index_base *b) {
  if (g_md3_media_workspace_mock_fail == 17) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_section_index_base_destroy(b);
}

/**
 * @brief Mock for ui_section_index_base_set_sections.
 * @param b Index pointer.
 * @param s Sections array.
 * @param c Count.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_section_index_base_set_sections(struct ui_section_index_base *b,
                                     const char **s, size_t c) {
  if (g_md3_media_workspace_mock_fail == 18) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_section_index_base_set_sections(b, s, c);
}

/**
 * @brief Mock for ui_section_index_base_set_active_section.
 * @param b Index pointer.
 * @param a Active section index.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_section_index_base_set_active_section(struct ui_section_index_base *b,
                                           int a) {
  if (g_md3_media_workspace_mock_fail == 19) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_section_index_base_set_active_section(b, a);
}

/**
 * @brief Mock for ui_dockable_layout_base_create.
 * @param out Output layout pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_dockable_layout_base_create(struct ui_dockable_layout_base **out) {
  if (g_md3_media_workspace_mock_fail == 20) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dockable_layout_base_create(out);
}

/**
 * @brief Mock for ui_dockable_layout_base_destroy.
 * @param b Layout pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_dockable_layout_base_destroy(struct ui_dockable_layout_base *b) {
  if (g_md3_media_workspace_mock_fail == 21) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dockable_layout_base_destroy(b);
}

#undef ui_icon_base_create
#define ui_icon_base_create mock_icon_base_create
#undef ui_icon_base_destroy
#define ui_icon_base_destroy mock_icon_base_destroy
#undef ui_icon_base_set_font_glyph
#define ui_icon_base_set_font_glyph mock_icon_base_set_font_glyph
#undef ui_arena_create
#define ui_arena_create mock_arena_create
#undef ui_chat_bubble_base_create
#define ui_chat_bubble_base_create mock_chat_bubble_base_create
#undef ui_chat_bubble_base_destroy
#define ui_chat_bubble_base_destroy mock_chat_bubble_base_destroy
#undef ui_arena_destroy
#define ui_arena_destroy mock_arena_destroy
#undef ui_timeline_base_create
#define ui_timeline_base_create mock_timeline_base_create
#undef ui_timeline_base_destroy
#define ui_timeline_base_destroy mock_timeline_base_destroy
#undef ui_timeline_base_add_node
#define ui_timeline_base_add_node mock_timeline_base_add_node
#undef ui_file_uploader_init
#define ui_file_uploader_init mock_file_uploader_init
#undef ui_file_uploader_destroy
#define ui_file_uploader_destroy mock_file_uploader_destroy
#undef ui_video_player_base_init
#define ui_video_player_base_init mock_video_player_base_init
#undef ui_rich_text_editor_base_create
#define ui_rich_text_editor_base_create mock_rich_text_editor_base_create
#undef ui_rich_text_editor_base_destroy
#define ui_rich_text_editor_base_destroy mock_rich_text_editor_base_destroy
#undef ui_section_index_base_create
#define ui_section_index_base_create mock_section_index_base_create
#undef ui_section_index_base_destroy
#define ui_section_index_base_destroy mock_section_index_base_destroy
#undef ui_section_index_base_set_sections
#define ui_section_index_base_set_sections mock_section_index_base_set_sections
#undef ui_section_index_base_set_active_section
#define ui_section_index_base_set_active_section                               \
  mock_section_index_base_set_active_section
#undef ui_dockable_layout_base_create
#define ui_dockable_layout_base_create mock_dockable_layout_base_create
#undef ui_dockable_layout_base_destroy
#define ui_dockable_layout_base_destroy mock_dockable_layout_base_destroy
#endif

/* ========================================================================= */
/* md3_icon                                                                  */
/* ========================================================================= */

ui_error_t md3_icon_create(struct ui_engine *engine,
                           struct md3_icon **out_icon) {
  struct md3_icon *icon;
  ui_error_t rc;

  if (engine == NULL || out_icon == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  icon = (struct md3_icon *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_icon));
  if (icon == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(icon, 0, sizeof(struct md3_icon));

  rc = ui_icon_base_create(&icon->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(icon);
    return rc;
  }

  icon->style = MD3_ICON_OUTLINED;
  icon->size_dp = 24.0f;
  icon->font_weight = 400;
  icon->font_fill = 0;
  icon->font_grade = 0;
  icon->optical_size = 24.0f;
  icon->rtl_mirroring = 0;
  icon->is_semantic = 0;
  icon->aria_label[0] = '\0';
  icon->current_glyph[0] = '\0';
  icon->morph_target[0] = '\0';
  icon->morph_progress = 0.0f;
  icon->is_morphing = 0;

  *out_icon = icon;
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_destroy(struct md3_icon *icon) {
  ui_error_t rc;

  if (icon == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (icon->base != NULL) {
    rc = ui_icon_base_destroy(icon->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    icon->base = NULL;
  }

  C_MULTIPLATFORM_FREE(icon);
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_set_style(struct md3_icon *icon,
                              enum md3_icon_style style) {
  if (icon == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  icon->style = style;
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_set_glyph(struct md3_icon *icon, const char *glyph_name) {
  ui_error_t rc;

  if (icon == NULL || glyph_name == NULL || icon->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_icon_base_set_font_glyph(icon->base, (struct ui_font *)0x1,
                                   glyph_name);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(_MSC_VER)
  strncpy_s(icon->current_glyph, sizeof(icon->current_glyph), glyph_name,
            sizeof(icon->current_glyph) - 1);
#else
  strncpy(icon->current_glyph, glyph_name, sizeof(icon->current_glyph) - 1);
  icon->current_glyph[sizeof(icon->current_glyph) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_icon_set_svg_path(struct md3_icon *icon, const char *svg_path) {
  if (icon == NULL || svg_path == NULL || icon->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_icon_base_set_svg_path(icon->base, svg_path);
}

ui_error_t md3_icon_set_font_axes(struct md3_icon *icon, int weight, int fill,
                                  int grade, float optical_size) {
  if (icon == NULL || weight < 100 || weight > 900 || fill < 0 || fill > 1 ||
      grade < -50 || grade > 200 || optical_size <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  icon->font_weight = weight;
  icon->font_fill = fill;
  icon->font_grade = grade;
  icon->optical_size = optical_size;
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_set_size(struct md3_icon *icon, float size_dp) {
  if (icon == NULL || size_dp <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  icon->size_dp = size_dp;
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_set_rtl_mirroring(struct md3_icon *icon, int auto_mirror) {
  if (icon == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  icon->rtl_mirroring = auto_mirror ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_set_semantic(struct md3_icon *icon, int is_semantic,
                                 const char *aria_label) {
  if (icon == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_semantic && aria_label == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  icon->is_semantic = is_semantic ? 1 : 0;
  if (is_semantic) {
#if defined(_MSC_VER)
    strncpy_s(icon->aria_label, sizeof(icon->aria_label), aria_label,
              sizeof(icon->aria_label) - 1);
#else
    strncpy(icon->aria_label, aria_label, sizeof(icon->aria_label) - 1);
    icon->aria_label[sizeof(icon->aria_label) - 1] = '\0';
#endif
  } else {
    icon->aria_label[0] = '\0';
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_morph_to(struct md3_icon *icon, const char *target_svg_path,
                             float progress) {
  if (icon == NULL || target_svg_path == NULL || progress < 0.0f ||
      progress > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(icon->morph_target, sizeof(icon->morph_target), target_svg_path,
            sizeof(icon->morph_target) - 1);
#else
  strncpy(icon->morph_target, target_svg_path, sizeof(icon->morph_target) - 1);
  icon->morph_target[sizeof(icon->morph_target) - 1] = '\0';
#endif

  icon->morph_progress = progress;
  if (progress > 0.0f && progress < 1.0f) {
    icon->is_morphing = 1;
  } else {
    icon->is_morphing = 0;
  }
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_chat_bubble                                                           */
/* ========================================================================= */

ui_error_t md3_chat_bubble_create(struct ui_engine *engine,
                                  enum md3_chat_bubble_direction direction,
                                  struct md3_chat_bubble **out_bubble) {
  struct md3_chat_bubble *b;
  struct ui_chat_bubble_config cfg;
  ui_error_t rc;

  if (engine == NULL || out_bubble == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  b = (struct md3_chat_bubble *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_chat_bubble));
  if (b == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(b, 0, sizeof(struct md3_chat_bubble));

  cfg.tail_placement = (direction == MD3_CHAT_INCOMING)
                           ? UI_CHAT_BUBBLE_TAIL_BOTTOM_LEFT
                           : UI_CHAT_BUBBLE_TAIL_BOTTOM_RIGHT;
  cfg.group_position = UI_CHAT_BUBBLE_GROUP_SINGLE;

  rc = ui_arena_create(4096, &b->arena);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(b);
    return rc;
  }

  rc = ui_chat_bubble_base_create(b->arena, &cfg, &b->base);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(b->arena);
    C_MULTIPLATFORM_FREE(b);
    return rc;
  }

  b->direction = direction;
  b->message_text[0] = '\0';
  b->timestamp[0] = '\0';
  b->status = MD3_MSG_SENT;
  b->reaction_count = 0;
  b->is_expressive = 0;

  *out_bubble = b;
  return UI_ERROR_NONE;
}

ui_error_t md3_chat_bubble_destroy(struct md3_chat_bubble *bubble) {
  ui_error_t rc;

  if (bubble == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (bubble->base != NULL) {
    rc = ui_chat_bubble_base_destroy(bubble->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    bubble->base = NULL;
  }

  if (bubble->arena != NULL) {
    rc = ui_arena_destroy(bubble->arena);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    bubble->arena = NULL;
  }

  C_MULTIPLATFORM_FREE(bubble);
  return UI_ERROR_NONE;
}

ui_error_t md3_chat_bubble_set_message(struct md3_chat_bubble *bubble,
                                       const char *text, const char *timestamp,
                                       enum md3_message_status status) {
  if (bubble == NULL || text == NULL || timestamp == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(bubble->message_text, sizeof(bubble->message_text), text,
            sizeof(bubble->message_text) - 1);
  strncpy_s(bubble->timestamp, sizeof(bubble->timestamp), timestamp,
            sizeof(bubble->timestamp) - 1);
#else
  strncpy(bubble->message_text, text, sizeof(bubble->message_text) - 1);
  bubble->message_text[sizeof(bubble->message_text) - 1] = '\0';
  strncpy(bubble->timestamp, timestamp, sizeof(bubble->timestamp) - 1);
  bubble->timestamp[sizeof(bubble->timestamp) - 1] = '\0';
#endif

  bubble->status = status;
  return UI_ERROR_NONE;
}

ui_error_t md3_chat_bubble_add_reaction(struct md3_chat_bubble *bubble,
                                        const char *emoji, int count) {
  if (bubble == NULL || emoji == NULL || count <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (bubble->reaction_count < 8) {
#if defined(_MSC_VER)
    strncpy_s(bubble->reactions[bubble->reaction_count].emoji,
              sizeof(bubble->reactions[0].emoji), emoji,
              sizeof(bubble->reactions[0].emoji) - 1);
#else
    strncpy(bubble->reactions[bubble->reaction_count].emoji, emoji,
            sizeof(bubble->reactions[0].emoji) - 1);
    bubble->reactions[bubble->reaction_count]
        .emoji[sizeof(bubble->reactions[0].emoji) - 1] = '\0';
#endif
    bubble->reactions[bubble->reaction_count].count = count;
    bubble->reaction_count++;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_OUT_OF_MEMORY;
}

ui_error_t md3_chat_bubble_set_expressive(struct md3_chat_bubble *bubble,
                                          int enabled) {
  if (bubble == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  bubble->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_timeline                                                              */
/* ========================================================================= */

ui_error_t md3_timeline_create(struct ui_engine *engine,
                               enum md3_timeline_orientation orientation,
                               struct md3_timeline **out_timeline) {
  struct md3_timeline *tl;
  ui_error_t rc;

  if (engine == NULL || out_timeline == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tl = (struct md3_timeline *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_timeline));
  if (tl == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(tl, 0, sizeof(struct md3_timeline));

  rc = ui_timeline_base_create(&tl->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tl);
    return rc;
  }

  tl->orientation = orientation;
  tl->is_opposing = 0;
  tl->is_expressive = 0;
  tl->item_count = 0;

  *out_timeline = tl;
  return UI_ERROR_NONE;
}

ui_error_t md3_timeline_destroy(struct md3_timeline *timeline) {
  ui_error_t rc;

  if (timeline == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (timeline->base != NULL) {
    rc = ui_timeline_base_destroy(timeline->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    timeline->base = NULL;
  }

  C_MULTIPLATFORM_FREE(timeline);
  return UI_ERROR_NONE;
}

ui_error_t md3_timeline_add_node(struct md3_timeline *timeline,
                                 const char *title, const char *description,
                                 enum md3_timeline_marker marker,
                                 enum md3_timeline_line_style line_style) {
  ui_error_t rc;

  if (timeline == NULL || title == NULL || description == NULL ||
      timeline->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_timeline_base_add_node(timeline->base, title, description);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (timeline->item_count < 32) {
#if defined(_MSC_VER)
    strncpy_s(timeline->items[timeline->item_count].title,
              sizeof(timeline->items[0].title), title,
              sizeof(timeline->items[0].title) - 1);
    strncpy_s(timeline->items[timeline->item_count].description,
              sizeof(timeline->items[0].description), description,
              sizeof(timeline->items[0].description) - 1);
#else
    strncpy(timeline->items[timeline->item_count].title, title,
            sizeof(timeline->items[0].title) - 1);
    timeline->items[timeline->item_count]
        .title[sizeof(timeline->items[0].title) - 1] = '\0';
    strncpy(timeline->items[timeline->item_count].description, description,
            sizeof(timeline->items[0].description) - 1);
    timeline->items[timeline->item_count]
        .description[sizeof(timeline->items[0].description) - 1] = '\0';
#endif
    timeline->items[timeline->item_count].marker = marker;
    timeline->items[timeline->item_count].line_style = line_style;
    timeline->items[timeline->item_count].is_expanded = 0;
    timeline->item_count++;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_timeline_set_opposing(struct md3_timeline *timeline,
                                     int opposing) {
  if (timeline == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  timeline->is_opposing = opposing ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_timeline_set_expressive(struct md3_timeline *timeline,
                                       int enabled) {
  if (timeline == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  timeline->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_file_uploader                                                         */
/* ========================================================================= */

ui_error_t md3_file_uploader_create(struct ui_engine *engine,
                                    struct md3_file_uploader **out_uploader) {
  struct md3_file_uploader *u;
  ui_error_t rc;

  if (engine == NULL || out_uploader == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  u = (struct md3_file_uploader *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_file_uploader));
  if (u == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(u, 0, sizeof(struct md3_file_uploader));

  rc = ui_file_uploader_init(&u->base, 10, 0, 0, 300, 200, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(u);
    return rc;
  }

  u->max_files = 10;
  u->allowed_formats[0] = '\0';
  u->is_drag_over = 0;
  u->is_expressive = 0;
  u->file_count = 0;

  *out_uploader = u;
  return UI_ERROR_NONE;
}

ui_error_t md3_file_uploader_destroy(struct md3_file_uploader *uploader) {
  ui_error_t rc;

  if (uploader == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_file_uploader_destroy(&uploader->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  C_MULTIPLATFORM_FREE(uploader);
  return UI_ERROR_NONE;
}

ui_error_t md3_file_uploader_set_max_files(struct md3_file_uploader *uploader,
                                           int max_files) {
  if (uploader == NULL || max_files <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  uploader->max_files = max_files;
  uploader->base.max_files = max_files;
  return UI_ERROR_NONE;
}

ui_error_t
md3_file_uploader_set_allowed_formats(struct md3_file_uploader *uploader,
                                      const char *formats) {
  if (uploader == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (formats == NULL) {
    uploader->allowed_formats[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(uploader->allowed_formats, sizeof(uploader->allowed_formats),
              formats, sizeof(uploader->allowed_formats) - 1);
#else
    strncpy(uploader->allowed_formats, formats,
            sizeof(uploader->allowed_formats) - 1);
    uploader->allowed_formats[sizeof(uploader->allowed_formats) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

ui_error_t md3_file_uploader_add_file(struct md3_file_uploader *uploader,
                                      const char *file_name, size_t file_size) {
  if (uploader == NULL || file_name == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (uploader->file_count < 32 &&
      (int)uploader->file_count < uploader->max_files) {
#if defined(_MSC_VER)
    strncpy_s(uploader->files[uploader->file_count].file_name,
              sizeof(uploader->files[0].file_name), file_name,
              sizeof(uploader->files[0].file_name) - 1);
#else
    strncpy(uploader->files[uploader->file_count].file_name, file_name,
            sizeof(uploader->files[0].file_name) - 1);
    uploader->files[uploader->file_count]
        .file_name[sizeof(uploader->files[0].file_name) - 1] = '\0';
#endif
    uploader->files[uploader->file_count].file_size = file_size;
    uploader->files[uploader->file_count].progress = 0.0f;
    uploader->files[uploader->file_count].is_error = 0;
    uploader->file_count++;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_OUT_OF_MEMORY;
}

ui_error_t
md3_file_uploader_set_file_progress(struct md3_file_uploader *uploader,
                                    int file_index, float progress) {
  if (uploader == NULL || file_index < 0 ||
      (size_t)file_index >= uploader->file_count || progress < 0.0f ||
      progress > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  uploader->files[file_index].progress = progress;
  return UI_ERROR_NONE;
}

ui_error_t md3_file_uploader_set_drag_over(struct md3_file_uploader *uploader,
                                           int drag_over) {
  if (uploader == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  uploader->is_drag_over = drag_over ? 1 : 0;
  uploader->base.state = drag_over ? UI_FILE_UPLOADER_STATE_DRAG_OVER
                                   : UI_FILE_UPLOADER_STATE_IDLE;
  return UI_ERROR_NONE;
}

ui_error_t md3_file_uploader_set_expressive(struct md3_file_uploader *uploader,
                                            int enabled) {
  if (uploader == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  uploader->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_video_player                                                          */
/* ========================================================================= */

ui_error_t md3_video_player_create(struct ui_engine *engine,
                                   struct md3_video_player **out_player) {
  struct md3_video_player *vp;
  ui_error_t rc;

  if (engine == NULL || out_player == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  vp = (struct md3_video_player *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_video_player));
  if (vp == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(vp, 0, sizeof(struct md3_video_player));

  rc = ui_video_player_base_init(&vp->base, &vp->comp, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(vp);
    return rc;
  }

  vp->playback_speed = 1.0f;
  vp->is_expressive = 0;

  *out_player = vp;
  return UI_ERROR_NONE;
}

ui_error_t md3_video_player_destroy(struct md3_video_player *player) {
  if (player == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(player);
  return UI_ERROR_NONE;
}

ui_error_t md3_video_player_play(struct md3_video_player *player) {
  if (player == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  player->base.is_playing = 1;
  return UI_ERROR_NONE;
}

ui_error_t md3_video_player_pause(struct md3_video_player *player) {
  if (player == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  player->base.is_playing = 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_video_player_seek(struct md3_video_player *player,
                                 float time_seconds) {
  if (player == NULL || time_seconds < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_video_player_base_seek(&player->base, time_seconds);
}

ui_error_t md3_video_player_set_volume(struct md3_video_player *player,
                                       float volume) {
  if (player == NULL || volume < 0.0f || volume > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_video_player_base_set_volume(&player->base, volume);
}

ui_error_t md3_video_player_set_playback_speed(struct md3_video_player *player,
                                               float speed) {
  if (player == NULL || speed <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  player->playback_speed = speed;
  return UI_ERROR_NONE;
}

ui_error_t md3_video_player_set_fullscreen(struct md3_video_player *player,
                                           int fullscreen) {
  if (player == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  player->base.is_fullscreen = fullscreen ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
md3_video_player_get_playback_direction(const struct md3_video_player *player,
                                        int *out_is_ltr) {
  if (player == NULL || out_is_ltr == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_ltr = 1;
  return UI_ERROR_NONE;
}

ui_error_t md3_video_player_set_expressive(struct md3_video_player *player,
                                           int enabled) {
  if (player == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  player->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_rich_text_editor                                                      */
/* ========================================================================= */

ui_error_t
md3_rich_text_editor_create(struct ui_engine *engine,
                            struct md3_rich_text_editor **out_editor) {
  struct md3_rich_text_editor *rte;
  ui_error_t rc;

  if (engine == NULL || out_editor == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rte = (struct md3_rich_text_editor *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_rich_text_editor));
  if (rte == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(rte, 0, sizeof(struct md3_rich_text_editor));

  rc = ui_rich_text_editor_base_create(&rte->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(rte);
    return rc;
  }

  rte->text_buffer[0] = '\0';
  rte->is_expressive = 0;

  *out_editor = rte;
  return UI_ERROR_NONE;
}

ui_error_t md3_rich_text_editor_destroy(struct md3_rich_text_editor *editor) {
  ui_error_t rc;

  if (editor == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (editor->base != NULL) {
    rc = ui_rich_text_editor_base_destroy(editor->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    editor->base = NULL;
  }

  C_MULTIPLATFORM_FREE(editor);
  return UI_ERROR_NONE;
}

ui_error_t md3_rich_text_editor_set_text(struct md3_rich_text_editor *editor,
                                         const char *text) {
  if (editor == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (text == NULL) {
    editor->text_buffer[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(editor->text_buffer, sizeof(editor->text_buffer), text,
              sizeof(editor->text_buffer) - 1);
#else
    strncpy(editor->text_buffer, text, sizeof(editor->text_buffer) - 1);
    editor->text_buffer[sizeof(editor->text_buffer) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

ui_error_t
md3_rich_text_editor_apply_format(struct md3_rich_text_editor *editor,
                                  const char *format_tag) {
  if (editor == NULL || format_tag == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

ui_error_t
md3_rich_text_editor_get_word_count(const struct md3_rich_text_editor *editor,
                                    size_t *out_words, size_t *out_chars) {
  const char *p;
  size_t words = 0;
  size_t chars = 0;
  int in_word = 0;

  if (editor == NULL || out_words == NULL || out_chars == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = editor->text_buffer;
  while (*p != '\0') {
    chars++;
    if (isspace((unsigned char)*p)) {
      in_word = 0;
    } else {
      if (!in_word) {
        words++;
        in_word = 1;
      }
    }
    p++;
  }

  *out_words = words;
  *out_chars = chars;
  return UI_ERROR_NONE;
}

ui_error_t md3_rich_text_editor_undo(struct md3_rich_text_editor *editor) {
  if (editor == NULL || editor->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_rich_text_editor_base_undo(editor->base);
}

ui_error_t md3_rich_text_editor_redo(struct md3_rich_text_editor *editor) {
  if (editor == NULL || editor->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_rich_text_editor_base_redo(editor->base);
}

ui_error_t
md3_rich_text_editor_set_expressive(struct md3_rich_text_editor *editor,
                                    int enabled) {
  if (editor == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  editor->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_section_index                                                         */
/* ========================================================================= */

ui_error_t md3_section_index_create(struct ui_engine *engine,
                                    struct md3_section_index **out_index) {
  struct md3_section_index *si;
  ui_error_t rc;

  if (engine == NULL || out_index == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  si = (struct md3_section_index *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_section_index));
  if (si == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(si, 0, sizeof(struct md3_section_index));

  rc = ui_section_index_base_create(&si->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(si);
    return rc;
  }

  si->active_section = -1;
  si->show_magnifier = 0;
  si->is_rtl = 0;
  si->is_expressive = 0;
  si->section_count = 0;

  *out_index = si;
  return UI_ERROR_NONE;
}

ui_error_t md3_section_index_destroy(struct md3_section_index *index) {
  ui_error_t rc;

  if (index == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index->base != NULL) {
    rc = ui_section_index_base_destroy(index->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    index->base = NULL;
  }

  C_MULTIPLATFORM_FREE(index);
  return UI_ERROR_NONE;
}

ui_error_t md3_section_index_set_sections(struct md3_section_index *index,
                                          const char **sections, size_t count) {
  ui_error_t rc;

  if (index == NULL || sections == NULL || index->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_section_index_base_set_sections(index->base, sections, count);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  index->section_count = count;
  return UI_ERROR_NONE;
}

ui_error_t md3_section_index_set_active_section(struct md3_section_index *index,
                                                int active_idx) {
  ui_error_t rc;

  if (index == NULL || index->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_section_index_base_set_active_section(index->base, active_idx);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  index->active_section = active_idx;
  return UI_ERROR_NONE;
}

ui_error_t
md3_section_index_get_active_section(const struct md3_section_index *index,
                                     int *out_active_idx) {
  if (index == NULL || out_active_idx == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_active_idx = index->active_section;
  return UI_ERROR_NONE;
}

ui_error_t md3_section_index_set_magnifier(struct md3_section_index *index,
                                           int show_magnifier) {
  if (index == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  index->show_magnifier = show_magnifier ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_section_index_set_rtl(struct md3_section_index *index,
                                     int is_rtl) {
  if (index == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  index->is_rtl = is_rtl ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_section_index_set_expressive(struct md3_section_index *index,
                                            int enabled) {
  if (index == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  index->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* md3_dockable_layout                                                       */
/* ========================================================================= */

ui_error_t md3_dockable_layout_create(struct ui_engine *engine,
                                      struct md3_dockable_layout **out_layout) {
  struct md3_dockable_layout *dl;
  ui_error_t rc;

  if (engine == NULL || out_layout == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dl = (struct md3_dockable_layout *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_dockable_layout));
  if (dl == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(dl, 0, sizeof(struct md3_dockable_layout));

  rc = ui_dockable_layout_base_create(&dl->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(dl);
    return rc;
  }

  dl->is_expressive = 0;

  *out_layout = dl;
  return UI_ERROR_NONE;
}

ui_error_t md3_dockable_layout_destroy(struct md3_dockable_layout *layout) {
  ui_error_t rc;

  if (layout == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (layout->base != NULL) {
    rc = ui_dockable_layout_base_destroy(layout->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    layout->base = NULL;
  }

  C_MULTIPLATFORM_FREE(layout);
  return UI_ERROR_NONE;
}

ui_error_t md3_dockable_layout_dock_panel(struct md3_dockable_layout *layout,
                                          int panel_id, int target_panel_id,
                                          enum ui_dock_edge edge) {
  if (layout == NULL || layout->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dockable_layout_base_dock_panel(layout->base, panel_id,
                                            target_panel_id, edge);
}

ui_error_t md3_dockable_layout_remove_panel(struct md3_dockable_layout *layout,
                                            int panel_id) {
  if (layout == NULL || layout->base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dockable_layout_base_remove_panel(layout->base, panel_id);
}

ui_error_t md3_dockable_layout_serialize(struct md3_dockable_layout *layout,
                                         char *out_buffer, size_t buffer_size) {
  if (layout == NULL || layout->base == NULL || out_buffer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dockable_layout_base_serialize(layout->base, out_buffer,
                                           buffer_size);
}

ui_error_t
md3_dockable_layout_set_expressive(struct md3_dockable_layout *layout,
                                   int enabled) {
  if (layout == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  layout->is_expressive = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}
