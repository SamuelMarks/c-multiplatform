/**
 * @file md3_media_workspace.h
 * @brief Material 3 & Expressive Iconography, Media, Workflow, and Desktop
 * Workspace Shells.
 *
 * Provides spec-compliant Material 3 implementations for:
 * - md3_icon (wrapping ui_icon_base)
 * - md3_chat_bubble (wrapping ui_chat_bubble_base)
 * - md3_timeline (wrapping ui_timeline_base)
 * - md3_file_uploader (wrapping ui_file_uploader_base)
 * - md3_video_player (wrapping ui_video_player_base)
 * - md3_rich_text_editor (wrapping ui_rich_text_editor_base)
 * - md3_section_index (wrapping ui_section_index_base)
 * - md3_dockable_layout (wrapping ui_dockable_layout_base)
 */

#ifndef MATERIAL3_MD3_MEDIA_WORKSPACE_H
#define MATERIAL3_MD3_MEDIA_WORKSPACE_H

/* clang-format off */
#include "ui_arena.h"
#include "ui_chat_bubble_base.h"
#include "ui_component.h"
#include "ui_dockable_layout_base.h"
#include "ui_error.h"
#include "ui_file_uploader_base.h"
#include "ui_icon_base.h"
#include "ui_rich_text_editor_base.h"
#include "ui_section_index_base.h"
#include "ui_timeline_base.h"
#include "ui_types.h"
#include "ui_video_player_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/* ========================================================================= */
/* md3_icon                                                                  */
/* ========================================================================= */

/**
 * @enum md3_icon_style
 * @brief Visual style variants for Material Symbols.
 */
enum md3_icon_style {
  MD3_ICON_OUTLINED = 0,
  MD3_ICON_FILLED = 1,
  MD3_ICON_ROUNDED = 2,
  MD3_ICON_SHARP = 3
};

/**
 * @struct md3_icon
 * @brief Material 3 Icon component wrapping ui_icon_base.
 */
struct md3_icon {
  struct ui_icon_base *base;
  enum md3_icon_style style;
  float size_dp;
  int font_weight;
  int font_fill;
  int font_grade;
  float optical_size;
  int rtl_mirroring;
  int is_semantic;
  char aria_label[128];
  char current_glyph[64];
  char morph_target[256];
  float morph_progress;
  int is_morphing;
};

/**
 * @brief Creates a Material 3 Icon.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_icon Pointer to receive allocated icon.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_create(struct ui_engine *engine, struct md3_icon **out_icon);

/**
 * @brief Destroys a Material 3 Icon.
 *
 * @param icon Icon instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_destroy(struct md3_icon *icon);

/**
 * @brief Sets the symbol font style (Outlined, Filled, Rounded, Sharp).
 *
 * @param icon Icon instance.
 * @param style Symbol style variant.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_set_style(struct md3_icon *icon, enum md3_icon_style style);

/**
 * @brief Sets the glyph identifier name (e.g. "home", "search").
 *
 * @param icon Icon instance.
 * @param glyph_name Glyph name string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_set_glyph(struct md3_icon *icon, const char *glyph_name);

/**
 * @brief Sets raw SVG path vector data fallback.
 *
 * @param icon Icon instance.
 * @param svg_path SVG path data string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_set_svg_path(struct md3_icon *icon, const char *svg_path);

/**
 * @brief Configures variable font axis values.
 *
 * @param icon Icon instance.
 * @param weight Font weight (100 to 700).
 * @param fill Fill value (0 or 1).
 * @param grade Font grade (-25 to 200).
 * @param optical_size Optical size (20.0 to 48.0dp).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_icon_set_font_axes(
    struct md3_icon *icon, int weight, int fill, int grade, float optical_size);

/**
 * @brief Configures icon display size in DP (16, 20, 24, 32, 40, 48).
 *
 * @param icon Icon instance.
 * @param size_dp Size in DP.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_set_size(struct md3_icon *icon, float size_dp);

/**
 * @brief Configures whether icon automatically mirrors in RTL layout.
 *
 * @param icon Icon instance.
 * @param auto_mirror 1 to mirror in RTL, 0 for invariant LTR.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_set_rtl_mirroring(struct md3_icon *icon, int auto_mirror);

/**
 * @brief Configures semantic vs decorative accessibility for the icon.
 *
 * @param icon Icon instance.
 * @param is_semantic 1 if icon conveys semantic meaning, 0 for decorative.
 * @param aria_label Accessible label required when semantic is 1.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_icon_set_semantic(
    struct md3_icon *icon, int is_semantic, const char *aria_label);

/**
 * @brief Performs dynamic vector icon morphing toward target SVG path.
 *
 * @param icon Icon instance.
 * @param target_svg_path Destination SVG vector path.
 * @param progress Morph interpolation progress (0.0f to 1.0f).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_icon_morph_to(
    struct md3_icon *icon, const char *target_svg_path, float progress);

/* ========================================================================= */
/* md3_chat_bubble                                                           */
/* ========================================================================= */

/**
 * @enum md3_chat_bubble_direction
 * @brief Directional orientation for chat messages.
 */
enum md3_chat_bubble_direction { MD3_CHAT_INCOMING = 0, MD3_CHAT_OUTGOING = 1 };

/**
 * @enum md3_message_status
 * @brief Delivery confirmation state for chat messages.
 */
enum md3_message_status {
  MD3_MSG_PENDING = 0,
  MD3_MSG_SENT = 1,
  MD3_MSG_DELIVERED = 2,
  MD3_MSG_READ = 3
};

/**
 * @struct md3_chat_reaction
 * @brief Emoji reaction chip attached to chat bubble.
 */
struct md3_chat_reaction {
  char emoji[16];
  int count;
};

/**
 * @struct md3_chat_bubble
 * @brief Material 3 Chat Bubble component wrapping ui_chat_bubble_base.
 */
struct md3_chat_bubble {
  struct ui_chat_bubble_base *base;
  struct ui_arena *arena;
  enum md3_chat_bubble_direction direction;
  char message_text[512];
  char timestamp[32];
  enum md3_message_status status;
  struct md3_chat_reaction reactions[8];
  size_t reaction_count;
  int is_expressive;
};

/**
 * @brief Creates a Material 3 Chat Bubble.
 *
 * @param engine Pointer to ui_engine instance.
 * @param direction Incoming or outgoing direction.
 * @param out_bubble Pointer to receive allocated chat bubble.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_chat_bubble_create(
    struct ui_engine *engine, enum md3_chat_bubble_direction direction,
    struct md3_chat_bubble **out_bubble);

/**
 * @brief Destroys a Material 3 Chat Bubble.
 *
 * @param bubble Bubble instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_chat_bubble_destroy(struct md3_chat_bubble *bubble);

/**
 * @brief Sets message text content, timestamp, and status.
 *
 * @param bubble Bubble instance.
 * @param text Message body string.
 * @param timestamp Localized time string.
 * @param status Message delivery status.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_chat_bubble_set_message(
    struct md3_chat_bubble *bubble, const char *text, const char *timestamp,
    enum md3_message_status status);

/**
 * @brief Adds an emoji reaction pill to the chat bubble.
 *
 * @param bubble Bubble instance.
 * @param emoji UTF-8 emoji string.
 * @param count Count of users reacting with this emoji.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_chat_bubble_add_reaction(
    struct md3_chat_bubble *bubble, const char *emoji, int count);

/**
 * @brief Enables or disables Expressive bouncy spring slide-in animations.
 *
 * @param bubble Bubble instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_chat_bubble_set_expressive(struct md3_chat_bubble *bubble, int enabled);

/* ========================================================================= */
/* md3_timeline                                                              */
/* ========================================================================= */

/**
 * @enum md3_timeline_orientation
 * @brief Axis layout orientation for timeline tracks.
 */
enum md3_timeline_orientation {
  MD3_TIMELINE_VERTICAL = 0,
  MD3_TIMELINE_HORIZONTAL = 1
};

/**
 * @enum md3_timeline_marker
 * @brief Marker node badge styling on timeline tracks.
 */
enum md3_timeline_marker {
  MD3_MARKER_FILLED = 0,
  MD3_MARKER_OUTLINED = 1,
  MD3_MARKER_ICON = 2,
  MD3_MARKER_AVATAR = 3
};

/**
 * @enum md3_timeline_line_style
 * @brief Connecting line stroke style between timeline nodes.
 */
enum md3_timeline_line_style {
  MD3_LINE_SOLID = 0,
  MD3_LINE_DASHED = 1,
  MD3_LINE_GRADIENT = 2
};

/**
 * @struct md3_timeline_item
 * @brief Individual node record in an md3_timeline.
 */
struct md3_timeline_item {
  char title[64];
  char description[128];
  enum md3_timeline_marker marker;
  enum md3_timeline_line_style line_style;
  int is_expanded;
};

/**
 * @struct md3_timeline
 * @brief Material 3 Timeline component wrapping ui_timeline_base.
 */
struct md3_timeline {
  struct ui_timeline_base *base;
  enum md3_timeline_orientation orientation;
  int is_opposing;
  int is_expressive;
  struct md3_timeline_item items[32];
  size_t item_count;
};

/**
 * @brief Creates a Material 3 Timeline.
 *
 * @param engine Pointer to ui_engine instance.
 * @param orientation Vertical or horizontal track orientation.
 * @param out_timeline Pointer to receive allocated timeline.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_timeline_create(
    struct ui_engine *engine, enum md3_timeline_orientation orientation,
    struct md3_timeline **out_timeline);

/**
 * @brief Destroys a Material 3 Timeline.
 *
 * @param timeline Timeline instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_timeline_destroy(struct md3_timeline *timeline);

/**
 * @brief Adds a node event to the timeline track.
 *
 * @param timeline Timeline instance.
 * @param title Header title of the event.
 * @param description Detailed event body text.
 * @param marker Visual node marker style.
 * @param line_style Connecting track line stroke style.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_timeline_add_node(
    struct md3_timeline *timeline, const char *title, const char *description,
    enum md3_timeline_marker marker, enum md3_timeline_line_style line_style);

/**
 * @brief Configures opposing layout (cards alternating left and right).
 *
 * @param timeline Timeline instance.
 * @param opposing 1 for alternating layout, 0 for aligned layout.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_timeline_set_opposing(struct md3_timeline *timeline, int opposing);

/**
 * @brief Enables or disables Expressive spring node expand animations.
 *
 * @param timeline Timeline instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_timeline_set_expressive(struct md3_timeline *timeline, int enabled);

/* ========================================================================= */
/* md3_file_uploader                                                         */
/* ========================================================================= */

/**
 * @struct md3_upload_file_item
 * @brief Individual file in an upload queue.
 */
struct md3_upload_file_item {
  char file_name[128];
  size_t file_size;
  float progress;
  int is_error;
};

/**
 * @struct md3_file_uploader
 * @brief Material 3 File Uploader component wrapping ui_file_uploader_base.
 */
struct md3_file_uploader {
  struct ui_file_uploader_base base;
  int max_files;
  char allowed_formats[128];
  int is_drag_over;
  int is_expressive;
  struct md3_upload_file_item files[32];
  size_t file_count;
};

/**
 * @brief Creates a Material 3 File Uploader.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_uploader Pointer to receive allocated uploader.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_file_uploader_create(
    struct ui_engine *engine, struct md3_file_uploader **out_uploader);

/**
 * @brief Destroys a Material 3 File Uploader.
 *
 * @param uploader Uploader instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_file_uploader_destroy(struct md3_file_uploader *uploader);

/**
 * @brief Sets maximum number of allowed files.
 *
 * @param uploader Uploader instance.
 * @param max_files Maximum file count limit.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_file_uploader_set_max_files(
    struct md3_file_uploader *uploader, int max_files);

/**
 * @brief Sets allowed file extensions (e.g. ".png,.jpg,.pdf").
 *
 * @param uploader Uploader instance.
 * @param formats Comma-separated extension list.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_file_uploader_set_allowed_formats(
    struct md3_file_uploader *uploader, const char *formats);

/**
 * @brief Enqueues a file for upload.
 *
 * @param uploader Uploader instance.
 * @param file_name Name of file.
 * @param file_size Size in bytes.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_file_uploader_add_file(struct md3_file_uploader *uploader,
                           const char *file_name, size_t file_size);

/**
 * @brief Updates upload progress for a specific queued file.
 *
 * @param uploader Uploader instance.
 * @param file_index Zero-based file queue index.
 * @param progress Fraction complete (0.0 to 1.0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_file_uploader_set_file_progress(
    struct md3_file_uploader *uploader, int file_index, float progress);

/**
 * @brief Updates active drag-over highlight state.
 *
 * @param uploader Uploader instance.
 * @param drag_over 1 if pointer is dragging files over dropzone, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_file_uploader_set_drag_over(
    struct md3_file_uploader *uploader, int drag_over);

/**
 * @brief Enables or disables Expressive spring scaling on dragover.
 *
 * @param uploader Uploader instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_file_uploader_set_expressive(
    struct md3_file_uploader *uploader, int enabled);

/* ========================================================================= */
/* md3_video_player                                                          */
/* ========================================================================= */

/**
 * @struct md3_video_player
 * @brief Material 3 Video Player component wrapping ui_video_player_base.
 */
struct md3_video_player {
  struct ui_video_player_base base;
  struct ui_component comp;
  float playback_speed;
  int is_expressive;
};

/**
 * @brief Creates a Material 3 Video Player.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_player Pointer to receive allocated video player.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_video_player_create(
    struct ui_engine *engine, struct md3_video_player **out_player);

/**
 * @brief Destroys a Material 3 Video Player.
 *
 * @param player Video player instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_video_player_destroy(struct md3_video_player *player);

/**
 * @brief Starts video playback.
 *
 * @param player Video player instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_video_player_play(struct md3_video_player *player);

/**
 * @brief Pauses video playback.
 *
 * @param player Video player instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_video_player_pause(struct md3_video_player *player);

/**
 * @brief Seeks to an exact playback position in seconds.
 *
 * @param player Video player instance.
 * @param time_seconds Target timestamp in seconds.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_video_player_seek(struct md3_video_player *player, float time_seconds);

/**
 * @brief Sets playback volume level (0.0 to 1.0).
 *
 * @param player Video player instance.
 * @param volume Volume level.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_video_player_set_volume(struct md3_video_player *player, float volume);

/**
 * @brief Sets playback speed rate multiplier (0.5x, 1.0x, 1.5x, 2.0x).
 *
 * @param player Video player instance.
 * @param speed Playback speed multiplier.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_video_player_set_playback_speed(
    struct md3_video_player *player, float speed);

/**
 * @brief Sets fullscreen display mode.
 *
 * @param player Video player instance.
 * @param fullscreen 1 for fullscreen, 0 for inline.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_video_player_set_fullscreen(
    struct md3_video_player *player, int fullscreen);

/**
 * @brief Asserts LTR playback transport direction invariant (no RTL mirroring).
 *
 * @param player Video player instance.
 * @param out_is_ltr Pointer to receive 1 indicating strictly LTR transport.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_video_player_get_playback_direction(const struct md3_video_player *player,
                                        int *out_is_ltr);

/**
 * @brief Enables or disables Expressive elastic scrubber animations.
 *
 * @param player Video player instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_video_player_set_expressive(struct md3_video_player *player, int enabled);

/* ========================================================================= */
/* md3_rich_text_editor                                                      */
/* ========================================================================= */

/**
 * @struct md3_rich_text_editor
 * @brief Material 3 Rich Text Editor component wrapping
 * ui_rich_text_editor_base.
 */
struct md3_rich_text_editor {
  struct ui_rich_text_editor_base *base;
  char text_buffer[2048];
  int is_expressive;
};

/**
 * @brief Creates a Material 3 Rich Text Editor.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_editor Pointer to receive allocated editor.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_rich_text_editor_create(
    struct ui_engine *engine, struct md3_rich_text_editor **out_editor);

/**
 * @brief Destroys a Material 3 Rich Text Editor.
 *
 * @param editor Editor instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rich_text_editor_destroy(struct md3_rich_text_editor *editor);

/**
 * @brief Sets raw text content for the editor.
 *
 * @param editor Editor instance.
 * @param text Content string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_rich_text_editor_set_text(
    struct md3_rich_text_editor *editor, const char *text);

/**
 * @brief Applies inline or block formatting tag (bold, italic, h1, code).
 *
 * @param editor Editor instance.
 * @param format_tag Formatting tag name.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_rich_text_editor_apply_format(
    struct md3_rich_text_editor *editor, const char *format_tag);

/**
 * @brief Calculates current word count and character count.
 *
 * @param editor Editor instance.
 * @param out_words Pointer to receive word count.
 * @param out_chars Pointer to receive character count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rich_text_editor_get_word_count(const struct md3_rich_text_editor *editor,
                                    size_t *out_words, size_t *out_chars);

/**
 * @brief Performs undo operation.
 *
 * @param editor Editor instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rich_text_editor_undo(struct md3_rich_text_editor *editor);

/**
 * @brief Performs redo operation.
 *
 * @param editor Editor instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rich_text_editor_redo(struct md3_rich_text_editor *editor);

/**
 * @brief Enables or disables Expressive morphing toolbar highlight.
 *
 * @param editor Editor instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_rich_text_editor_set_expressive(
    struct md3_rich_text_editor *editor, int enabled);

/* ========================================================================= */
/* md3_section_index                                                         */
/* ========================================================================= */

/**
 * @struct md3_section_index
 * @brief Material 3 Alphabetical Section Jump Index wrapping
 * ui_section_index_base.
 */
struct md3_section_index {
  struct ui_section_index_base *base;
  int active_section;
  int show_magnifier;
  int is_rtl;
  int is_expressive;
  size_t section_count;
};

/**
 * @brief Creates a Material 3 Section Index.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_index Pointer to receive allocated section index.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_section_index_create(
    struct ui_engine *engine, struct md3_section_index **out_index);

/**
 * @brief Destroys a Material 3 Section Index.
 *
 * @param index Section index instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_section_index_destroy(struct md3_section_index *index);

/**
 * @brief Sets the list of section labels (e.g. "A", "B", ... "Z").
 *
 * @param index Section index instance.
 * @param sections Array of section string pointers.
 * @param count Number of sections.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_section_index_set_sections(
    struct md3_section_index *index, const char **sections, size_t count);

/**
 * @brief Sets active highlighted section index (-1 to clear).
 *
 * @param index Section index instance.
 * @param active_idx Active index in sections array.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_section_index_set_active_section(
    struct md3_section_index *index, int active_idx);

/**
 * @brief Gets active highlighted section index.
 *
 * @param index Section index instance.
 * @param out_active_idx Pointer to receive active section index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_section_index_get_active_section(
    const struct md3_section_index *index, int *out_active_idx);

/**
 * @brief Configures display of the floating magnifier letter bubble during
 * scrubbing.
 *
 * @param index Section index instance.
 * @param show_magnifier 1 to display magnifier bubble, 0 to hide.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_section_index_set_magnifier(
    struct md3_section_index *index, int show_magnifier);

/**
 * @brief Configures RTL orientation (index moves to leading screen edge).
 *
 * @param index Section index instance.
 * @param is_rtl 1 if layout is RTL, 0 for standard LTR.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_section_index_set_rtl(struct md3_section_index *index, int is_rtl);

/**
 * @brief Enables or disables Expressive elastic letter magnification.
 *
 * @param index Section index instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_section_index_set_expressive(struct md3_section_index *index, int enabled);

/* ========================================================================= */
/* md3_dockable_layout                                                       */
/* ========================================================================= */

/**
 * @struct md3_dockable_layout
 * @brief Material 3 Desktop Dockable Workspace Shell wrapping
 * ui_dockable_layout_base.
 */
struct md3_dockable_layout {
  struct ui_dockable_layout_base *base;
  int is_expressive;
};

/**
 * @brief Creates a Material 3 Dockable Layout.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_layout Pointer to receive allocated layout.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_dockable_layout_create(
    struct ui_engine *engine, struct md3_dockable_layout **out_layout);

/**
 * @brief Destroys a Material 3 Dockable Layout.
 *
 * @param layout Layout instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_dockable_layout_destroy(struct md3_dockable_layout *layout);

/**
 * @brief Docks a panel into one of the layout edges or center.
 *
 * @param layout Layout instance.
 * @param panel_id Unique panel identifier.
 * @param target_panel_id Existing panel ID to dock against (or 0 for root).
 * @param edge Target edge (Left, Right, Top, Bottom, Center).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_dockable_layout_dock_panel(struct md3_dockable_layout *layout, int panel_id,
                               int target_panel_id, enum ui_dock_edge edge);

/**
 * @brief Removes a panel from the dockable layout.
 *
 * @param layout Layout instance.
 * @param panel_id Panel identifier to remove.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_dockable_layout_remove_panel(
    struct md3_dockable_layout *layout, int panel_id);

/**
 * @brief Serializes workspace docking configuration state to string.
 *
 * @param layout Layout instance.
 * @param out_buffer Output buffer to receive serialized text.
 * @param buffer_size Size of output buffer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_dockable_layout_serialize(
    struct md3_dockable_layout *layout, char *out_buffer, size_t buffer_size);

/**
 * @brief Enables or disables Expressive spring snapping and ghost drop
 * previews.
 *
 * @param layout Layout instance.
 * @param enabled 1 to enable Expressive features, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_dockable_layout_set_expressive(
    struct md3_dockable_layout *layout, int enabled);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_MEDIA_WORKSPACE_H */
