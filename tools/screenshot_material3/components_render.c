/**
 * @file components_render.c
 * @brief High-level Material 3 component rendering module for screenshot
 * generation.
 */

/* clang-format off */
#include "components_render.h"
#include "rasterizer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static ui_error_t make_path(char *dest, size_t dest_sz, const char *dir,
                            const char *filename) {
  int written;
  if (!dest || dest_sz == 0 || !dir || !filename) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#if defined(_MSC_VER)
  written = sprintf_s(dest, dest_sz, "%s/%s", dir, filename);
  if (written < 0) {
    return UI_ERROR_IO_FAILED;
  }
#else
  written = snprintf(dest, dest_sz, "%s/%s", dir, filename);
  if (written < 0 || (size_t)written >= dest_sz) {
    return UI_ERROR_IO_FAILED;
  }
#endif
  return UI_ERROR_NONE;
}

#define CHECK_RC(expr)                                                         \
  do {                                                                         \
    rc = (expr);                                                               \
    if (rc != UI_ERROR_NONE) {                                                 \
      if (c) {                                                                 \
        canvas_destroy(c);                                                     \
        c = NULL;                                                              \
      }                                                                        \
      return rc;                                                               \
    }                                                                          \
  } while (0)

/* ========================================================================= */
/* Buttons                                                                   */
/* ========================================================================= */

static ui_error_t draw_btn_elevated(struct canvas *c, float x, float y, float w,
                                    float h, const char *text,
                                    const struct md3_color_scheme *s) {
  ui_error_t rc;
  rc = draw_shadow(c, x, y, w, h, h * 0.5f, 1);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = draw_rounded_rect(c, x, y, w, h, h * 0.5f, s->surface_container_low);
  if (rc != UI_ERROR_NONE)
    return rc;
  return draw_text_centered(c, x + w * 0.5f, y + (h - 16.0f) * 0.5f, text, 1.0f,
                            s->primary);
}

static ui_error_t draw_btn_filled(struct canvas *c, float x, float y, float w,
                                  float h, const char *text,
                                  const struct md3_color_scheme *s) {
  ui_error_t rc;
  rc = draw_rounded_rect(c, x, y, w, h, h * 0.5f, s->primary);
  if (rc != UI_ERROR_NONE)
    return rc;
  return draw_text_centered(c, x + w * 0.5f, y + (h - 16.0f) * 0.5f, text, 1.0f,
                            s->on_primary);
}

static ui_error_t draw_btn_filled_tonal(struct canvas *c, float x, float y,
                                        float w, float h, const char *text,
                                        const struct md3_color_scheme *s) {
  ui_error_t rc;
  rc = draw_rounded_rect(c, x, y, w, h, h * 0.5f, s->secondary_container);
  if (rc != UI_ERROR_NONE)
    return rc;
  return draw_text_centered(c, x + w * 0.5f, y + (h - 16.0f) * 0.5f, text, 1.0f,
                            s->on_secondary_container);
}

static ui_error_t draw_btn_outlined(struct canvas *c, float x, float y, float w,
                                    float h, const char *text,
                                    const struct md3_color_scheme *s) {
  ui_error_t rc;
  rc = draw_rounded_rect_stroke(c, x, y, w, h, h * 0.5f, 1.0f, s->outline);
  if (rc != UI_ERROR_NONE)
    return rc;
  return draw_text_centered(c, x + w * 0.5f, y + (h - 16.0f) * 0.5f, text, 1.0f,
                            s->primary);
}

static ui_error_t draw_btn_text(struct canvas *c, float x, float y, float w,
                                float h, const char *text,
                                const struct md3_color_scheme *s) {
  return draw_text_centered(c, x + w * 0.5f, y + (h - 16.0f) * 0.5f, text, 1.0f,
                            s->primary);
}

ui_error_t render_all_buttons(const struct md3_color_scheme *s,
                              const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;

  /* 1. button_elevated.png */
  CHECK_RC(canvas_create(220, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_btn_elevated(c, 30.0f, 30.0f, 160.0f, 40.0f, "Elevated", s));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "button_elevated.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. button_filled.png */
  CHECK_RC(canvas_create(220, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_btn_filled(c, 30.0f, 30.0f, 160.0f, 40.0f, "Filled", s));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "button_filled.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. button_filled_tonal.png */
  CHECK_RC(canvas_create(220, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(
      draw_btn_filled_tonal(c, 30.0f, 30.0f, 160.0f, 40.0f, "Filled Tonal", s));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir,
                     "button_filled_tonal.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 4. button_outlined.png */
  CHECK_RC(canvas_create(220, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_btn_outlined(c, 30.0f, 30.0f, 160.0f, 40.0f, "Outlined", s));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "button_outlined.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 5. button_text.png */
  CHECK_RC(canvas_create(220, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_btn_text(c, 30.0f, 30.0f, 160.0f, 40.0f, "Text Button", s));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "button_text.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 6. button_icon.png */
  CHECK_RC(canvas_create(320, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Standard */
  CHECK_RC(
      draw_icon(c, 50.0f, 50.0f, ICON_HEART, 24.0f, s->on_surface_variant));
  /* Filled */
  CHECK_RC(draw_circle(c, 110.0f, 50.0f, 20.0f, s->primary));
  CHECK_RC(draw_icon(c, 110.0f, 50.0f, ICON_HEART, 20.0f, s->on_primary));
  /* Filled Tonal */
  CHECK_RC(draw_circle(c, 170.0f, 50.0f, 20.0f, s->secondary_container));
  CHECK_RC(draw_icon(c, 170.0f, 50.0f, ICON_HEART, 20.0f,
                     s->on_secondary_container));
  /* Outlined */
  CHECK_RC(draw_circle_stroke(c, 230.0f, 50.0f, 20.0f, 1.0f, s->outline));
  CHECK_RC(
      draw_icon(c, 230.0f, 50.0f, ICON_HEART, 20.0f, s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "button_icon.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 7. button_split.png */
  CHECK_RC(canvas_create(260, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(
      draw_rounded_rect(c, 30.0f, 30.0f, 140.0f, 40.0f, 20.0f, s->primary));
  CHECK_RC(draw_text_centered(c, 90.0f, 42.0f, "Publish", 1.0f, s->on_primary));
  CHECK_RC(draw_line(c, 145.0f, 34.0f, 145.0f, 66.0f, 1.0f, s->on_primary));
  CHECK_RC(draw_icon(c, 157.0f, 50.0f, ICON_ARROW_DOWN, 16.0f, s->on_primary));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "button_split.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 8. button_segmented.png */
  CHECK_RC(canvas_create(340, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect_stroke(c, 20.0f, 30.0f, 300.0f, 40.0f, 20.0f, 1.0f,
                                    s->outline));
  /* Segment 1: Selected */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 30.0f, 100.0f, 40.0f, 20.0f,
                             s->secondary_container));
  CHECK_RC(
      draw_icon(c, 42.0f, 50.0f, ICON_CHECK, 14.0f, s->on_secondary_container));
  CHECK_RC(draw_text(c, 56.0f, 42.0f, "Day", 1.0f, s->on_secondary_container));
  /* Divider 1 */
  CHECK_RC(draw_line(c, 120.0f, 30.0f, 120.0f, 70.0f, 1.0f, s->outline));
  /* Segment 2 */
  CHECK_RC(draw_text(c, 148.0f, 42.0f, "Week", 1.0f, s->on_surface));
  /* Divider 2 */
  CHECK_RC(draw_line(c, 220.0f, 30.0f, 220.0f, 70.0f, 1.0f, s->outline));
  /* Segment 3 */
  CHECK_RC(draw_text(c, 246.0f, 42.0f, "Month", 1.0f, s->on_surface));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "button_segmented.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 9. fab.png */
  CHECK_RC(canvas_create(360, 120, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Small FAB */
  CHECK_RC(draw_shadow(c, 30.0f, 40.0f, 40.0f, 40.0f, 12.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 40.0f, 40.0f, 40.0f, 12.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 50.0f, 60.0f, ICON_ADD, 20.0f, s->on_primary_container));
  /* Regular FAB */
  CHECK_RC(draw_shadow(c, 90.0f, 32.0f, 56.0f, 56.0f, 16.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 90.0f, 32.0f, 56.0f, 56.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 118.0f, 60.0f, ICON_EDIT, 24.0f, s->on_primary_container));
  /* Large FAB */
  CHECK_RC(draw_shadow(c, 166.0f, 12.0f, 96.0f, 96.0f, 28.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 166.0f, 12.0f, 96.0f, 96.0f, 28.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 214.0f, 60.0f, ICON_ADD, 36.0f, s->on_primary_container));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "fab.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 10. fab_extended.png */
  CHECK_RC(canvas_create(260, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 30.0f, 22.0f, 200.0f, 56.0f, 16.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 22.0f, 200.0f, 56.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 58.0f, 50.0f, ICON_EDIT, 24.0f, s->on_primary_container));
  CHECK_RC(
      draw_text(c, 80.0f, 42.0f, "Compose", 1.25f, s->on_primary_container));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "fab_extended.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 11. fab_menu.png */
  CHECK_RC(canvas_create(180, 240, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Action 1 */
  CHECK_RC(draw_shadow(c, 90.0f, 30.0f, 40.0f, 40.0f, 12.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 90.0f, 30.0f, 40.0f, 40.0f, 12.0f,
                             s->secondary_container));
  CHECK_RC(draw_icon(c, 110.0f, 50.0f, ICON_SETTINGS, 20.0f,
                     s->on_secondary_container));
  /* Action 2 */
  CHECK_RC(draw_shadow(c, 90.0f, 85.0f, 40.0f, 40.0f, 12.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 90.0f, 85.0f, 40.0f, 40.0f, 12.0f,
                             s->secondary_container));
  CHECK_RC(draw_icon(c, 110.0f, 105.0f, ICON_SHARE, 20.0f,
                     s->on_secondary_container));
  /* Main FAB (Expanded with close icon) */
  CHECK_RC(draw_shadow(c, 82.0f, 145.0f, 56.0f, 56.0f, 16.0f, 4));
  CHECK_RC(draw_rounded_rect(c, 82.0f, 145.0f, 56.0f, 56.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 110.0f, 173.0f, ICON_CLOSE, 24.0f, s->on_primary_container));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "fab_menu.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 12. button_group.png */
  CHECK_RC(canvas_create(340, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_btn_filled(c, 20.0f, 30.0f, 90.0f, 40.0f, "Action 1", s));
  CHECK_RC(
      draw_btn_filled_tonal(c, 120.0f, 30.0f, 90.0f, 40.0f, "Action 2", s));
  CHECK_RC(draw_btn_outlined(c, 220.0f, 30.0f, 90.0f, 40.0f, "Action 3", s));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "button_group.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 13. buttons_all.png (Composite) */
  CHECK_RC(canvas_create(700, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_text(c, 30.0f, 25.0f, "Material 3 Button Family", 1.2f,
                     s->on_surface));
  CHECK_RC(draw_btn_elevated(c, 30.0f, 60.0f, 120.0f, 40.0f, "Elevated", s));
  CHECK_RC(draw_btn_filled(c, 160.0f, 60.0f, 120.0f, 40.0f, "Filled", s));
  CHECK_RC(draw_btn_filled_tonal(c, 290.0f, 60.0f, 120.0f, 40.0f, "Tonal", s));
  CHECK_RC(draw_btn_outlined(c, 420.0f, 60.0f, 120.0f, 40.0f, "Outlined", s));
  CHECK_RC(draw_btn_text(c, 550.0f, 60.0f, 110.0f, 40.0f, "Text", s));
  /* Row 2: FAB, Segmented, Icon */
  CHECK_RC(draw_shadow(c, 30.0f, 130.0f, 48.0f, 48.0f, 16.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 130.0f, 48.0f, 48.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 54.0f, 154.0f, ICON_ADD, 24.0f, s->on_primary_container));

  CHECK_RC(draw_shadow(c, 95.0f, 134.0f, 140.0f, 40.0f, 16.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 95.0f, 134.0f, 140.0f, 40.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 115.0f, 154.0f, ICON_EDIT, 18.0f, s->on_primary_container));
  CHECK_RC(
      draw_text(c, 135.0f, 146.0f, "Compose", 1.0f, s->on_primary_container));

  CHECK_RC(draw_rounded_rect_stroke(c, 260.0f, 134.0f, 220.0f, 40.0f, 20.0f,
                                    1.0f, s->outline));
  CHECK_RC(draw_rounded_rect(c, 260.0f, 134.0f, 75.0f, 40.0f, 20.0f,
                             s->secondary_container));
  CHECK_RC(draw_text_centered(c, 297.0f, 146.0f, "List", 1.0f,
                              s->on_secondary_container));
  CHECK_RC(draw_line(c, 335.0f, 134.0f, 335.0f, 174.0f, 1.0f, s->outline));
  CHECK_RC(draw_text_centered(c, 370.0f, 146.0f, "Grid", 1.0f, s->on_surface));
  CHECK_RC(draw_line(c, 405.0f, 134.0f, 405.0f, 174.0f, 1.0f, s->outline));
  CHECK_RC(draw_text_centered(c, 442.0f, 146.0f, "Cards", 1.0f, s->on_surface));

  CHECK_RC(draw_circle(c, 520.0f, 154.0f, 20.0f, s->primary));
  CHECK_RC(draw_icon(c, 520.0f, 154.0f, ICON_HEART, 18.0f, s->on_primary));
  CHECK_RC(draw_circle(c, 570.0f, 154.0f, 20.0f, s->secondary_container));
  CHECK_RC(draw_icon(c, 570.0f, 154.0f, ICON_STAR, 18.0f,
                     s->on_secondary_container));
  CHECK_RC(draw_circle_stroke(c, 620.0f, 154.0f, 20.0f, 1.0f, s->outline));
  CHECK_RC(draw_icon(c, 620.0f, 154.0f, ICON_SETTINGS, 18.0f,
                     s->on_surface_variant));

  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "buttons_all.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Pickers                                                                   */
/* ========================================================================= */

ui_error_t render_all_pickers(const struct md3_color_scheme *s,
                              const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;
  int row, col;
  int day = 1;
  const char *weekdays[] = {"S", "M", "T", "W", "T", "F", "S"};

  /* 1. datepicker.png */
  CHECK_RC(canvas_create(360, 480, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 15.0f, 15.0f, 330.0f, 450.0f, 28.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 15.0f, 15.0f, 330.0f, 450.0f, 28.0f,
                             s->surface_container_high));

  /* Header */
  CHECK_RC(
      draw_text(c, 35.0f, 35.0f, "Select date", 0.9f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 35.0f, 58.0f, "Wed, Sep 30", 1.7f, s->on_surface));
  CHECK_RC(
      draw_line(c, 15.0f, 105.0f, 345.0f, 105.0f, 1.0f, s->outline_variant));

  /* Month nav */
  CHECK_RC(draw_text(c, 35.0f, 122.0f, "September 2026", 1.0f, s->on_surface));
  CHECK_RC(draw_icon(c, 275.0f, 130.0f, ICON_ARROW_LEFT, 18.0f,
                     s->on_surface_variant));
  CHECK_RC(draw_icon(c, 310.0f, 130.0f, ICON_ARROW_RIGHT, 18.0f,
                     s->on_surface_variant));

  /* Weekday labels */
  for (col = 0; col < 7; ++col) {
    CHECK_RC(draw_text_centered(c, 45.0f + (float)col * 40.0f, 158.0f,
                                weekdays[col], 0.9f, s->on_surface_variant));
  }

  /* Calendar days grid (Sep 2026 starts on Tuesday = col 2) */
  for (row = 0; row < 5; ++row) {
    for (col = 0; col < 7; ++col) {
      char day_str[8];
      float cx = 45.0f + (float)col * 40.0f;
      float cy = 200.0f + (float)row * 40.0f;

      if (row == 0 && col < 2)
        continue;
      if (day > 30)
        break;

#if defined(_MSC_VER)
      sprintf_s(day_str, sizeof(day_str), "%d", day);
#else
      snprintf(day_str, sizeof(day_str), "%d", day);
#endif

      if (day == 30) {
        /* Selected Day */
        CHECK_RC(draw_circle(c, cx, cy, 18.0f, s->primary));
        CHECK_RC(
            draw_text_centered(c, cx, cy - 8.0f, day_str, 1.0f, s->on_primary));
      } else if (day == 29) {
        /* Today indicator */
        CHECK_RC(draw_circle_stroke(c, cx, cy, 18.0f, 1.0f, s->primary));
        CHECK_RC(
            draw_text_centered(c, cx, cy - 8.0f, day_str, 1.0f, s->primary));
      } else {
        CHECK_RC(
            draw_text_centered(c, cx, cy - 8.0f, day_str, 1.0f, s->on_surface));
      }
      day++;
    }
  }

  /* Actions */
  CHECK_RC(draw_text_centered(c, 240.0f, 425.0f, "Cancel", 1.0f, s->primary));
  CHECK_RC(draw_text_centered(c, 305.0f, 425.0f, "OK", 1.0f, s->primary));

  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "datepicker.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. date_range_picker.png */
  CHECK_RC(canvas_create(360, 480, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 15.0f, 15.0f, 330.0f, 450.0f, 28.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 15.0f, 15.0f, 330.0f, 450.0f, 28.0f,
                             s->surface_container_high));
  CHECK_RC(
      draw_text(c, 35.0f, 35.0f, "Select range", 0.9f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 35.0f, 58.0f, "Sep 15 - Sep 22", 1.5f, s->on_surface));
  CHECK_RC(
      draw_line(c, 15.0f, 105.0f, 345.0f, 105.0f, 1.0f, s->outline_variant));

  CHECK_RC(draw_text(c, 35.0f, 122.0f, "September 2026", 1.0f, s->on_surface));
  for (col = 0; col < 7; ++col) {
    CHECK_RC(draw_text_centered(c, 45.0f + (float)col * 40.0f, 158.0f,
                                weekdays[col], 0.9f, s->on_surface_variant));
  }

  /* Range span on Row 2 (Sep 15 is col 2, Sep 20 is col 0 row 3) */
  CHECK_RC(draw_rounded_rect(c, 45.0f + 2.0f * 40.0f, 262.0f, 4.0f * 40.0f,
                             36.0f, 18.0f, s->secondary_container));

  day = 1;
  for (row = 0; row < 5; ++row) {
    for (col = 0; col < 7; ++col) {
      char day_str[8];
      float cx = 45.0f + (float)col * 40.0f;
      float cy = 200.0f + (float)row * 40.0f;

      if (row == 0 && col < 2)
        continue;
      if (day > 30)
        break;

#if defined(_MSC_VER)
      sprintf_s(day_str, sizeof(day_str), "%d", day);
#else
      snprintf(day_str, sizeof(day_str), "%d", day);
#endif

      if (day == 15 || day == 19) {
        CHECK_RC(draw_circle(c, cx, cy, 18.0f, s->primary));
        CHECK_RC(
            draw_text_centered(c, cx, cy - 8.0f, day_str, 1.0f, s->on_primary));
      } else if (day > 15 && day < 19) {
        CHECK_RC(draw_text_centered(c, cx, cy - 8.0f, day_str, 1.0f,
                                    s->on_secondary_container));
      } else {
        CHECK_RC(
            draw_text_centered(c, cx, cy - 8.0f, day_str, 1.0f, s->on_surface));
      }
      day++;
    }
  }

  CHECK_RC(draw_text_centered(c, 240.0f, 425.0f, "Cancel", 1.0f, s->primary));
  CHECK_RC(draw_text_centered(c, 305.0f, 425.0f, "Save", 1.0f, s->primary));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "date_range_picker.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. timepicker.png */
  CHECK_RC(canvas_create(340, 440, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 15.0f, 15.0f, 310.0f, 410.0f, 28.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 15.0f, 15.0f, 310.0f, 410.0f, 28.0f,
                             s->surface_container_high));

  CHECK_RC(
      draw_text(c, 35.0f, 35.0f, "Select time", 0.9f, s->on_surface_variant));

  /* Digital display chips: 10 : 30 AM */
  CHECK_RC(draw_rounded_rect(c, 35.0f, 65.0f, 80.0f, 60.0f, 12.0f,
                             s->primary_container));
  CHECK_RC(
      draw_text_centered(c, 75.0f, 80.0f, "10", 2.0f, s->on_primary_container));
  CHECK_RC(draw_text_centered(c, 125.0f, 80.0f, ":", 2.0f, s->on_surface));
  CHECK_RC(draw_rounded_rect(c, 135.0f, 65.0f, 80.0f, 60.0f, 12.0f,
                             s->surface_container_highest));
  CHECK_RC(draw_text_centered(c, 175.0f, 80.0f, "30", 2.0f, s->on_surface));

  /* AM/PM toggle */
  CHECK_RC(draw_rounded_rect_stroke(c, 230.0f, 65.0f, 65.0f, 60.0f, 12.0f, 1.0f,
                                    s->outline));
  CHECK_RC(draw_rounded_rect(c, 230.0f, 65.0f, 65.0f, 30.0f, 12.0f,
                             s->secondary_container));
  CHECK_RC(draw_text_centered(c, 262.0f, 72.0f, "AM", 0.9f,
                              s->on_secondary_container));
  CHECK_RC(
      draw_text_centered(c, 262.0f, 102.0f, "PM", 0.9f, s->on_surface_variant));

  /* Clock Dial */
  {
    float dial_cx = 170.0f;
    float dial_cy = 245.0f;
    float dial_r = 95.0f;
    int h_idx;

    CHECK_RC(
        draw_circle(c, dial_cx, dial_cy, dial_r, s->surface_container_highest));
    CHECK_RC(draw_circle(c, dial_cx, dial_cy, 4.0f, s->primary));

    /* Clock hand to 10 */
    {
      float angle = (-1.5707963f + 10.0f * (3.14159265f / 6.0f));
      float hx = dial_cx + cosf(angle) * (dial_r - 25.0f);
      float hy = dial_cy + sinf(angle) * (dial_r - 25.0f);
      CHECK_RC(draw_line(c, dial_cx, dial_cy, hx, hy, 2.0f, s->primary));
      CHECK_RC(draw_circle(c, hx, hy, 16.0f, s->primary));
    }

    for (h_idx = 1; h_idx <= 12; ++h_idx) {
      char num_str[4];
      float angle = (-1.5707963f + (float)h_idx * (3.14159265f / 6.0f));
      float nx = dial_cx + cosf(angle) * (dial_r - 25.0f);
      float ny = dial_cy + sinf(angle) * (dial_r - 25.0f);
#if defined(_MSC_VER)
      sprintf_s(num_str, sizeof(num_str), "%d", h_idx);
#else
      snprintf(num_str, sizeof(num_str), "%d", h_idx);
#endif
      CHECK_RC(
          draw_text_centered(c, nx, ny - 8.0f, num_str, 0.9f,
                             (h_idx == 10) ? s->on_primary : s->on_surface));
    }
  }

  CHECK_RC(
      draw_icon(c, 45.0f, 395.0f, ICON_CLOCK, 22.0f, s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 230.0f, 388.0f, "Cancel", 1.0f, s->primary));
  CHECK_RC(draw_text_centered(c, 285.0f, 388.0f, "OK", 1.0f, s->primary));

  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "timepicker.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Selection Controls                                                        */
/* ========================================================================= */

ui_error_t render_all_selection_controls(const struct md3_color_scheme *s,
                                         const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;
  int i;

  /* 1. checkbox.png */
  CHECK_RC(canvas_create(280, 150, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Unchecked */
  CHECK_RC(draw_rounded_rect_stroke(c, 30.0f, 25.0f, 20.0f, 20.0f, 2.0f, 2.0f,
                                    s->outline));
  CHECK_RC(draw_text(c, 65.0f, 28.0f, "Unchecked", 1.0f, s->on_surface));
  /* Checked */
  CHECK_RC(draw_rounded_rect(c, 30.0f, 65.0f, 20.0f, 20.0f, 2.0f, s->primary));
  CHECK_RC(draw_icon(c, 40.0f, 75.0f, ICON_CHECK, 14.0f, s->on_primary));
  CHECK_RC(draw_text(c, 65.0f, 68.0f, "Checked", 1.0f, s->on_surface));
  /* Indeterminate */
  CHECK_RC(draw_rounded_rect(c, 30.0f, 105.0f, 20.0f, 20.0f, 2.0f, s->primary));
  CHECK_RC(draw_line(c, 35.0f, 115.0f, 45.0f, 115.0f, 2.0f, s->on_primary));
  CHECK_RC(draw_text(c, 65.0f, 108.0f, "Indeterminate", 1.0f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "checkbox.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. radio_button.png */
  CHECK_RC(canvas_create(280, 110, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Selected */
  CHECK_RC(draw_circle_stroke(c, 40.0f, 35.0f, 10.0f, 2.0f, s->primary));
  CHECK_RC(draw_circle(c, 40.0f, 35.0f, 5.0f, s->primary));
  CHECK_RC(draw_text(c, 65.0f, 28.0f, "Selected option", 1.0f, s->on_surface));
  /* Unselected */
  CHECK_RC(draw_circle_stroke(c, 40.0f, 75.0f, 10.0f, 2.0f, s->outline));
  CHECK_RC(
      draw_text(c, 65.0f, 68.0f, "Unselected option", 1.0f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "radio_button.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. switch.png */
  CHECK_RC(canvas_create(280, 120, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Switch ON */
  CHECK_RC(draw_rounded_rect(c, 30.0f, 25.0f, 52.0f, 32.0f, 16.0f, s->primary));
  CHECK_RC(draw_circle(c, 66.0f, 41.0f, 12.0f, s->on_primary));
  CHECK_RC(draw_icon(c, 66.0f, 41.0f, ICON_CHECK, 12.0f, s->primary));
  CHECK_RC(draw_text(c, 100.0f, 32.0f, "Enabled ON", 1.0f, s->on_surface));
  /* Switch OFF */
  CHECK_RC(draw_rounded_rect_stroke(c, 30.0f, 68.0f, 52.0f, 32.0f, 16.0f, 2.0f,
                                    s->outline));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 68.0f, 52.0f, 32.0f, 16.0f,
                             s->surface_container_highest));
  CHECK_RC(draw_circle(c, 46.0f, 84.0f, 8.0f, s->outline));
  CHECK_RC(draw_text(c, 100.0f, 75.0f, "Disabled OFF", 1.0f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "switch.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 4. slider.png */
  CHECK_RC(canvas_create(340, 140, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Continuous Slider with value badge */
  CHECK_RC(draw_rounded_rect(c, 30.0f, 40.0f, 280.0f, 16.0f, 8.0f,
                             s->secondary_container));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 40.0f, 170.0f, 16.0f, 8.0f, s->primary));
  /* Value tooltip badge */
  CHECK_RC(
      draw_rounded_rect(c, 185.0f, 10.0f, 30.0f, 24.0f, 12.0f, s->primary));
  CHECK_RC(draw_text_centered(c, 200.0f, 14.0f, "60", 0.85f, s->on_primary));
  /* Thumb handle */
  CHECK_RC(draw_rounded_rect(c, 198.0f, 26.0f, 4.0f, 44.0f, 2.0f, s->primary));

  /* Discrete Slider with tick marks */
  CHECK_RC(draw_rounded_rect(c, 30.0f, 105.0f, 280.0f, 16.0f, 8.0f,
                             s->secondary_container));
  CHECK_RC(
      draw_rounded_rect(c, 30.0f, 105.0f, 112.0f, 16.0f, 8.0f, s->primary));
  /* Ticks */
  for (i = 0; i <= 5; ++i) {
    float tx = 30.0f + (float)i * 56.0f;
    CHECK_RC(draw_circle(c, tx, 113.0f, 2.0f,
                         (i <= 2) ? s->on_primary : s->primary));
  }
  CHECK_RC(draw_rounded_rect(c, 140.0f, 91.0f, 4.0f, 44.0f, 2.0f, s->primary));

  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "slider.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 5. range_slider.png */
  CHECK_RC(canvas_create(340, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Track */
  CHECK_RC(draw_rounded_rect(c, 30.0f, 42.0f, 280.0f, 16.0f, 8.0f,
                             s->secondary_container));
  /* Active Range Span */
  CHECK_RC(draw_rounded_rect(c, 90.0f, 42.0f, 140.0f, 16.0f, 8.0f, s->primary));
  /* Left Thumb */
  CHECK_RC(draw_rounded_rect(c, 88.0f, 28.0f, 4.0f, 44.0f, 2.0f, s->primary));
  /* Right Thumb */
  CHECK_RC(draw_rounded_rect(c, 228.0f, 28.0f, 4.0f, 44.0f, 2.0f, s->primary));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "range_slider.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 6. pin_input.png */
  CHECK_RC(canvas_create(360, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  for (i = 0; i < 6; ++i) {
    float bx = 30.0f + (float)i * 52.0f;
    if (i == 2) {
      CHECK_RC(draw_rounded_rect_stroke(c, bx, 25.0f, 44.0f, 50.0f, 12.0f, 2.0f,
                                        s->primary));
      CHECK_RC(
          draw_line(c, bx + 22.0f, 35.0f, bx + 22.0f, 65.0f, 2.0f, s->primary));
    } else if (i < 2) {
      CHECK_RC(draw_rounded_rect_stroke(c, bx, 25.0f, 44.0f, 50.0f, 12.0f, 1.0f,
                                        s->outline));
      CHECK_RC(draw_circle(c, bx + 22.0f, 50.0f, 6.0f, s->on_surface));
    } else {
      CHECK_RC(draw_rounded_rect_stroke(c, bx, 25.0f, 44.0f, 50.0f, 12.0f, 1.0f,
                                        s->outline_variant));
    }
  }
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "pin_input.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 7. rating.png */
  CHECK_RC(canvas_create(260, 80, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  for (i = 0; i < 5; ++i) {
    float sx = 40.0f + (float)i * 42.0f;
    CHECK_RC(
        draw_icon(c, sx, 40.0f, (i < 4) ? ICON_STAR : ICON_STAR_OUTLINE, 28.0f,
                  (i < 4) ? UI_COLOR_ARGB(255, 235, 175, 25) : s->outline));
  }
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "rating.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 8. spin_button.png */
  CHECK_RC(canvas_create(240, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect_stroke(c, 30.0f, 25.0f, 160.0f, 48.0f, 8.0f, 1.0f,
                                    s->outline));
  CHECK_RC(draw_text(c, 50.0f, 40.0f, "Quantity: 4", 1.0f, s->on_surface));
  /* Stepper buttons */
  CHECK_RC(draw_line(c, 150.0f, 25.0f, 150.0f, 73.0f, 1.0f, s->outline));
  CHECK_RC(draw_line(c, 150.0f, 49.0f, 190.0f, 49.0f, 1.0f, s->outline));
  CHECK_RC(
      draw_icon(c, 170.0f, 37.0f, ICON_ARROW_UP, 14.0f, s->on_surface_variant));
  CHECK_RC(draw_icon(c, 170.0f, 61.0f, ICON_ARROW_DOWN, 14.0f,
                     s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "spin_button.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 9. autocomplete.png */
  CHECK_RC(canvas_create(320, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Text field */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 280.0f, 52.0f, 4.0f,
                             s->surface_container_highest));
  CHECK_RC(draw_line(c, 20.0f, 70.0f, 300.0f, 70.0f, 2.0f, s->primary));
  CHECK_RC(draw_text(c, 35.0f, 26.0f, "Country", 0.75f, s->primary));
  CHECK_RC(draw_text(c, 35.0f, 44.0f, "Unit", 1.0f, s->on_surface));
  CHECK_RC(draw_line(c, 70.0f, 42.0f, 70.0f, 60.0f, 1.5f, s->primary));
  /* Dropdown suggestions */
  CHECK_RC(draw_shadow(c, 20.0f, 76.0f, 280.0f, 120.0f, 8.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 20.0f, 76.0f, 280.0f, 120.0f, 8.0f,
                             s->surface_container_low));
  /* Item 1 */
  CHECK_RC(draw_text(c, 35.0f, 92.0f, "United States", 1.0f, s->on_surface));
  /* Item 2 (Highlighted) */
  CHECK_RC(draw_rounded_rect(c, 22.0f, 116.0f, 276.0f, 36.0f, 4.0f,
                             s->secondary_container));
  CHECK_RC(draw_text(c, 35.0f, 126.0f, "United Kingdom", 1.0f,
                     s->on_secondary_container));
  /* Item 3 */
  CHECK_RC(
      draw_text(c, 35.0f, 162.0f, "United Arab Emirates", 1.0f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "autocomplete.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 10. select.png */
  CHECK_RC(canvas_create(280, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect_stroke(c, 25.0f, 22.0f, 230.0f, 56.0f, 8.0f, 1.0f,
                                    s->outline));
  CHECK_RC(draw_text(c, 40.0f, 14.0f, " Fruit ", 0.75f, s->primary));
  CHECK_RC(draw_text(c, 40.0f, 42.0f, "Apple", 1.0f, s->on_surface));
  CHECK_RC(draw_icon(c, 230.0f, 50.0f, ICON_ARROW_DOWN, 18.0f,
                     s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "select.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 11. text_field.png */
  CHECK_RC(canvas_create(340, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Filled Text Field */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 300.0f, 56.0f, 4.0f,
                             s->surface_container_highest));
  CHECK_RC(draw_line(c, 20.0f, 74.0f, 320.0f, 74.0f, 2.0f, s->primary));
  CHECK_RC(draw_text(c, 35.0f, 27.0f, "Label (Filled)", 0.75f, s->primary));
  CHECK_RC(draw_text(c, 35.0f, 46.0f, "Input text value", 1.0f, s->on_surface));

  /* Outlined Text Field */
  CHECK_RC(draw_rounded_rect_stroke(c, 20.0f, 100.0f, 300.0f, 56.0f, 8.0f, 1.0f,
                                    s->outline));
  CHECK_RC(draw_text(c, 35.0f, 92.0f, " Label (Outlined) ", 0.75f, s->primary));
  CHECK_RC(
      draw_text(c, 35.0f, 122.0f, "Supporting value", 1.0f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "text_field.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Containment & Cards                                                       */
/* ========================================================================= */

ui_error_t render_all_containment_and_cards(const struct md3_color_scheme *s,
                                            const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;
  int i;

  /* 1. card_elevated.png */
  CHECK_RC(canvas_create(320, 240, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 20.0f, 20.0f, 280.0f, 200.0f, 16.0f, 1));
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 280.0f, 200.0f, 16.0f,
                             s->surface_container_low));
  /* Thumbnail media */
  CHECK_RC(draw_rounded_rect(c, 35.0f, 35.0f, 250.0f, 80.0f, 10.0f,
                             s->secondary_container));
  CHECK_RC(
      draw_icon(c, 160.0f, 75.0f, ICON_STAR, 32.0f, s->on_secondary_container));
  CHECK_RC(
      draw_text(c, 35.0f, 130.0f, "Elevated Card Title", 1.2f, s->on_surface));
  CHECK_RC(draw_text(c, 35.0f, 155.0f, "Supporting secondary text line.", 0.85f,
                     s->on_surface_variant));
  CHECK_RC(draw_btn_filled_tonal(c, 190.0f, 180.0f, 95.0f, 32.0f, "Action", s));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "card_elevated.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. card_filled.png */
  CHECK_RC(canvas_create(320, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 280.0f, 140.0f, 16.0f,
                             s->surface_container_highest));
  CHECK_RC(
      draw_text(c, 40.0f, 40.0f, "Filled Card Headline", 1.2f, s->on_surface));
  CHECK_RC(draw_text(c, 40.0f, 65.0f, "Material 3 tonal surface fill.", 0.9f,
                     s->on_surface_variant));
  CHECK_RC(draw_btn_filled(c, 180.0f, 110.0f, 100.0f, 36.0f, "Explore", s));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "card_filled.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. card_outlined.png */
  CHECK_RC(canvas_create(320, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect_stroke(c, 20.0f, 20.0f, 280.0f, 140.0f, 16.0f,
                                    1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 40.0f, 40.0f, "Outlined Card", 1.2f, s->on_surface));
  CHECK_RC(draw_text(c, 40.0f, 65.0f, "Subtle structural containment.", 0.9f,
                     s->on_surface_variant));
  CHECK_RC(draw_btn_outlined(c, 180.0f, 110.0f, 100.0f, 36.0f, "Details", s));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "card_outlined.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 4. chips.png */
  CHECK_RC(canvas_create(440, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Assist chip */
  CHECK_RC(draw_rounded_rect_stroke(c, 20.0f, 32.0f, 90.0f, 32.0f, 8.0f, 1.0f,
                                    s->outline));
  CHECK_RC(draw_icon(c, 34.0f, 48.0f, ICON_CALENDAR, 14.0f, s->primary));
  CHECK_RC(draw_text(c, 48.0f, 42.0f, "Assist", 0.9f, s->on_surface));

  /* Filter chip (selected) */
  CHECK_RC(draw_rounded_rect(c, 120.0f, 32.0f, 90.0f, 32.0f, 8.0f,
                             s->secondary_container));
  CHECK_RC(draw_icon(c, 134.0f, 48.0f, ICON_CHECK, 14.0f,
                     s->on_secondary_container));
  CHECK_RC(
      draw_text(c, 148.0f, 42.0f, "Filter", 0.9f, s->on_secondary_container));

  /* Input chip */
  CHECK_RC(draw_rounded_rect_stroke(c, 220.0f, 32.0f, 90.0f, 32.0f, 8.0f, 1.0f,
                                    s->outline));
  CHECK_RC(draw_text(c, 234.0f, 42.0f, "Input", 0.9f, s->on_surface));
  CHECK_RC(
      draw_icon(c, 292.0f, 48.0f, ICON_CLOSE, 14.0f, s->on_surface_variant));

  /* Suggestion chip */
  CHECK_RC(draw_rounded_rect_stroke(c, 320.0f, 32.0f, 105.0f, 32.0f, 8.0f, 1.0f,
                                    s->outline));
  CHECK_RC(draw_text(c, 335.0f, 42.0f, "Suggestion", 0.9f, s->on_surface));

  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "chips.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 5. badge.png */
  CHECK_RC(canvas_create(220, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Icon with dot badge */
  CHECK_RC(draw_icon(c, 60.0f, 50.0f, ICON_BELL, 28.0f, s->on_surface_variant));
  CHECK_RC(draw_circle(c, 72.0f, 38.0f, 4.0f, s->error));
  /* Icon with count badge */
  CHECK_RC(
      draw_icon(c, 140.0f, 50.0f, ICON_HOME, 28.0f, s->on_surface_variant));
  CHECK_RC(draw_rounded_rect(c, 148.0f, 30.0f, 22.0f, 16.0f, 8.0f, s->error));
  CHECK_RC(draw_text_centered(c, 159.0f, 33.0f, "9+", 0.7f, s->on_error));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "badge.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 6. avatar.png */
  CHECK_RC(canvas_create(340, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Initials */
  CHECK_RC(draw_circle(c, 50.0f, 50.0f, 24.0f, s->primary_container));
  CHECK_RC(
      draw_text_centered(c, 50.0f, 42.0f, "SM", 1.0f, s->on_primary_container));
  /* Icon */
  CHECK_RC(draw_circle(c, 115.0f, 50.0f, 24.0f, s->secondary_container));
  CHECK_RC(draw_icon(c, 115.0f, 50.0f, ICON_PERSON, 24.0f,
                     s->on_secondary_container));
  /* Avatar Group */
  for (i = 0; i < 3; ++i) {
    float gx = 195.0f + (float)i * 32.0f;
    CHECK_RC(draw_circle(c, gx, 50.0f, 20.0f, s->surface_container_highest));
    CHECK_RC(draw_circle_stroke(c, gx, 50.0f, 20.0f, 2.0f, s->surface));
  }
  CHECK_RC(draw_circle(c, 291.0f, 50.0f, 20.0f, s->surface_container_high));
  CHECK_RC(draw_circle_stroke(c, 291.0f, 50.0f, 20.0f, 2.0f, s->surface));
  CHECK_RC(draw_text_centered(c, 291.0f, 44.0f, "+4", 0.8f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "avatar.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 7. divider.png */
  CHECK_RC(canvas_create(320, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(
      draw_text(c, 20.0f, 20.0f, "Full Width Divider", 0.9f, s->on_surface));
  CHECK_RC(draw_line(c, 20.0f, 40.0f, 300.0f, 40.0f, 1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 60.0f, 55.0f, "Inset Divider Below Item", 0.9f,
                     s->on_surface));
  CHECK_RC(draw_line(c, 60.0f, 75.0f, 300.0f, 75.0f, 1.0f, s->outline_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "divider.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 8. list.png */
  CHECK_RC(canvas_create(340, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Item 1 */
  CHECK_RC(draw_circle(c, 45.0f, 45.0f, 18.0f, s->primary_container));
  CHECK_RC(
      draw_icon(c, 45.0f, 45.0f, ICON_FOLDER, 18.0f, s->on_primary_container));
  CHECK_RC(draw_text(c, 75.0f, 35.0f, "Photos", 1.0f, s->on_surface));
  CHECK_RC(
      draw_text(c, 75.0f, 50.0f, "Jan 9, 2026", 0.8f, s->on_surface_variant));
  CHECK_RC(draw_icon(c, 305.0f, 45.0f, ICON_MORE_VERT, 16.0f,
                     s->on_surface_variant));
  CHECK_RC(draw_line(c, 75.0f, 70.0f, 320.0f, 70.0f, 1.0f, s->outline_variant));

  /* Item 2 */
  CHECK_RC(draw_circle(c, 45.0f, 95.0f, 18.0f, s->secondary_container));
  CHECK_RC(
      draw_icon(c, 45.0f, 95.0f, ICON_EDIT, 18.0f, s->on_secondary_container));
  CHECK_RC(draw_text(c, 75.0f, 85.0f, "Documents", 1.0f, s->on_surface));
  CHECK_RC(
      draw_text(c, 75.0f, 100.0f, "Feb 17, 2026", 0.8f, s->on_surface_variant));
  CHECK_RC(draw_icon(c, 305.0f, 95.0f, ICON_MORE_VERT, 16.0f,
                     s->on_surface_variant));
  CHECK_RC(
      draw_line(c, 75.0f, 120.0f, 320.0f, 120.0f, 1.0f, s->outline_variant));

  /* Item 3 */
  CHECK_RC(draw_circle(c, 45.0f, 145.0f, 18.0f, s->tertiary_container));
  CHECK_RC(
      draw_icon(c, 45.0f, 145.0f, ICON_STAR, 18.0f, s->on_tertiary_container));
  CHECK_RC(draw_text(c, 75.0f, 135.0f, "Favorites", 1.0f, s->on_surface));
  CHECK_RC(draw_text(c, 75.0f, 150.0f, "Work files and specs", 0.8f,
                     s->on_surface_variant));
  CHECK_RC(draw_icon(c, 305.0f, 145.0f, ICON_MORE_VERT, 16.0f,
                     s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "list.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 9. accordion.png */
  CHECK_RC(canvas_create(340, 160, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Expanded panel */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 15.0f, 300.0f, 85.0f, 12.0f,
                             s->surface_container_low));
  CHECK_RC(
      draw_text(c, 35.0f, 30.0f, "Personal Information", 1.0f, s->on_surface));
  CHECK_RC(
      draw_icon(c, 295.0f, 35.0f, ICON_ARROW_UP, 16.0f, s->on_surface_variant));
  CHECK_RC(draw_line(c, 35.0f, 50.0f, 305.0f, 50.0f, 1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 35.0f, 62.0f, "Name: Samuel Marks", 0.85f,
                     s->on_surface_variant));
  CHECK_RC(draw_text(c, 35.0f, 78.0f, "Location: Sydney, Australia", 0.85f,
                     s->on_surface_variant));

  /* Collapsed panel */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 108.0f, 300.0f, 40.0f, 12.0f,
                             s->surface_container_low));
  CHECK_RC(
      draw_text(c, 35.0f, 120.0f, "Account Preferences", 1.0f, s->on_surface));
  CHECK_RC(draw_icon(c, 295.0f, 126.0f, ICON_ARROW_DOWN, 16.0f,
                     s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "accordion.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 10. carousel.png */
  CHECK_RC(canvas_create(440, 160, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Card 1 (Hero) */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 15.0f, 180.0f, 130.0f, 20.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 110.0f, 60.0f, ICON_STAR, 32.0f, s->on_primary_container));
  CHECK_RC(draw_text(c, 35.0f, 115.0f, "Featured Story", 1.0f,
                     s->on_primary_container));
  /* Card 2 */
  CHECK_RC(draw_rounded_rect(c, 210.0f, 15.0f, 140.0f, 130.0f, 20.0f,
                             s->secondary_container));
  CHECK_RC(draw_icon(c, 280.0f, 60.0f, ICON_HEART, 28.0f,
                     s->on_secondary_container));
  CHECK_RC(
      draw_text(c, 225.0f, 115.0f, "Popular", 1.0f, s->on_secondary_container));
  /* Card 3 (Peeked) */
  CHECK_RC(draw_rounded_rect(c, 360.0f, 15.0f, 80.0f, 130.0f, 20.0f,
                             s->tertiary_container));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "carousel.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 11. empty_state.png */
  CHECK_RC(canvas_create(320, 240, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_circle(c, 160.0f, 70.0f, 36.0f, s->surface_container_highest));
  CHECK_RC(
      draw_icon(c, 160.0f, 70.0f, ICON_SEARCH, 36.0f, s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 160.0f, 125.0f, "No Results Found", 1.2f,
                              s->on_surface));
  CHECK_RC(draw_text_centered(c, 160.0f, 150.0f,
                              "Try adjusting your search criteria.", 0.85f,
                              s->on_surface_variant));
  CHECK_RC(draw_btn_filled(c, 110.0f, 180.0f, 100.0f, 36.0f, "Clear", s));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "empty_state.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 12. skeleton.png */
  CHECK_RC(canvas_create(320, 160, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 25.0f, 25.0f, 50.0f, 50.0f, 25.0f,
                             s->surface_container_highest));
  CHECK_RC(draw_rounded_rect(c, 90.0f, 32.0f, 180.0f, 14.0f, 4.0f,
                             s->surface_container_highest));
  CHECK_RC(draw_rounded_rect(c, 90.0f, 54.0f, 130.0f, 12.0f, 4.0f,
                             s->surface_container_highest));
  CHECK_RC(draw_rounded_rect(c, 25.0f, 95.0f, 270.0f, 16.0f, 4.0f,
                             s->surface_container_highest));
  CHECK_RC(draw_rounded_rect(c, 25.0f, 120.0f, 220.0f, 16.0f, 4.0f,
                             s->surface_container_highest));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "skeleton.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 13. aspect_ratio.png */
  CHECK_RC(canvas_create(320, 200, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 280.0f, 157.5f, 12.0f,
                             s->surface_container_high));
  CHECK_RC(draw_icon(c, 160.0f, 90.0f, ICON_PLAY, 40.0f, s->primary));
  CHECK_RC(draw_text_centered(c, 160.0f, 130.0f, "16:9 Aspect Ratio", 0.9f,
                              s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "aspect_ratio.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Navigation                                                                */
/* ========================================================================= */

ui_error_t render_all_navigation(const struct md3_color_scheme *s,
                                 const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;
  int i;

  /* 1. navigation_bar.png (Bottom navigation) */
  CHECK_RC(canvas_create(360, 110, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 0.0f, 10.0f, 360.0f, 90.0f, 0.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 0.0f, 15.0f, 360.0f, 90.0f, 0.0f,
                             s->surface_container));
  /* Destination 1: Active */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 28.0f, 60.0f, 32.0f, 16.0f,
                             s->secondary_container));
  CHECK_RC(
      draw_icon(c, 50.0f, 44.0f, ICON_HOME, 20.0f, s->on_secondary_container));
  CHECK_RC(draw_text_centered(c, 50.0f, 68.0f, "Home", 0.8f, s->on_surface));
  /* Destination 2 */
  CHECK_RC(
      draw_icon(c, 135.0f, 44.0f, ICON_SEARCH, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 135.0f, 68.0f, "Search", 0.8f,
                              s->on_surface_variant));
  /* Destination 3 with Badge */
  CHECK_RC(
      draw_icon(c, 225.0f, 44.0f, ICON_BELL, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_circle(c, 235.0f, 36.0f, 4.0f, s->error));
  CHECK_RC(draw_text_centered(c, 225.0f, 68.0f, "Alerts", 0.8f,
                              s->on_surface_variant));
  /* Destination 4 */
  CHECK_RC(
      draw_icon(c, 310.0f, 44.0f, ICON_SETTINGS, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 310.0f, 68.0f, "Settings", 0.8f,
                              s->on_surface_variant));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "navigation_bar.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. navigation_rail.png */
  CHECK_RC(canvas_create(110, 360, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 10.0f, 10.0f, 90.0f, 340.0f, 16.0f,
                             s->surface_container));
  /* Menu icon */
  CHECK_RC(draw_icon(c, 55.0f, 35.0f, ICON_MENU, 22.0f, s->on_surface_variant));
  /* FAB in rail */
  CHECK_RC(draw_shadow(c, 27.0f, 65.0f, 56.0f, 56.0f, 16.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 27.0f, 65.0f, 56.0f, 56.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 55.0f, 93.0f, ICON_EDIT, 22.0f, s->on_primary_container));
  /* Item 1 Active */
  CHECK_RC(draw_rounded_rect(c, 25.0f, 150.0f, 60.0f, 32.0f, 16.0f,
                             s->secondary_container));
  CHECK_RC(
      draw_icon(c, 55.0f, 166.0f, ICON_HOME, 20.0f, s->on_secondary_container));
  CHECK_RC(draw_text_centered(c, 55.0f, 188.0f, "Home", 0.75f, s->on_surface));
  /* Item 2 */
  CHECK_RC(
      draw_icon(c, 55.0f, 225.0f, ICON_STAR, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 55.0f, 245.0f, "Starred", 0.75f,
                              s->on_surface_variant));
  /* Item 3 */
  CHECK_RC(
      draw_icon(c, 55.0f, 280.0f, ICON_SETTINGS, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 55.0f, 300.0f, "Settings", 0.75f,
                              s->on_surface_variant));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "navigation_rail.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. navigation_drawer.png */
  CHECK_RC(canvas_create(280, 360, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 10.0f, 10.0f, 260.0f, 340.0f, 16.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 10.0f, 10.0f, 260.0f, 340.0f, 16.0f,
                             s->surface_container_low));
  CHECK_RC(draw_text(c, 30.0f, 35.0f, "Mail & Notes", 1.2f, s->on_surface));
  /* Active Destination Pill */
  CHECK_RC(draw_rounded_rect(c, 22.0f, 65.0f, 236.0f, 48.0f, 24.0f,
                             s->secondary_container));
  CHECK_RC(
      draw_icon(c, 45.0f, 89.0f, ICON_HOME, 20.0f, s->on_secondary_container));
  CHECK_RC(
      draw_text(c, 70.0f, 82.0f, "Inbox", 1.0f, s->on_secondary_container));
  CHECK_RC(draw_text(c, 220.0f, 82.0f, "24", 0.9f, s->on_secondary_container));
  /* Item 2 */
  CHECK_RC(
      draw_icon(c, 45.0f, 137.0f, ICON_STAR, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 70.0f, 130.0f, "Starred", 1.0f, s->on_surface_variant));
  /* Item 3 */
  CHECK_RC(
      draw_icon(c, 45.0f, 185.0f, ICON_BELL, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 70.0f, 178.0f, "Notifications", 1.0f,
                     s->on_surface_variant));
  /* Divider */
  CHECK_RC(
      draw_line(c, 25.0f, 215.0f, 255.0f, 215.0f, 1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 30.0f, 230.0f, "Labels", 0.85f, s->on_surface_variant));
  /* Item 4 */
  CHECK_RC(
      draw_icon(c, 45.0f, 265.0f, ICON_FOLDER, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 70.0f, 258.0f, "Work Projects", 1.0f,
                     s->on_surface_variant));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "navigation_drawer.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 4. top_app_bar.png */
  CHECK_RC(canvas_create(440, 90, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 10.0f, 10.0f, 420.0f, 68.0f, 12.0f,
                             s->surface_container));
  CHECK_RC(draw_icon(c, 40.0f, 44.0f, ICON_MENU, 24.0f, s->on_surface));
  CHECK_RC(draw_text(c, 75.0f, 35.0f, "Dashboard Title", 1.25f, s->on_surface));
  CHECK_RC(
      draw_icon(c, 325.0f, 44.0f, ICON_SEARCH, 20.0f, s->on_surface_variant));
  CHECK_RC(
      draw_icon(c, 365.0f, 44.0f, ICON_BELL, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_icon(c, 400.0f, 44.0f, ICON_MORE_VERT, 20.0f,
                     s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "top_app_bar.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 5. bottom_app_bar.png */
  CHECK_RC(canvas_create(440, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 10.0f, 15.0f, 420.0f, 70.0f, 16.0f,
                             s->surface_container));
  CHECK_RC(draw_icon(c, 45.0f, 50.0f, ICON_MENU, 22.0f, s->on_surface));
  CHECK_RC(
      draw_icon(c, 95.0f, 50.0f, ICON_SEARCH, 22.0f, s->on_surface_variant));
  CHECK_RC(
      draw_icon(c, 145.0f, 50.0f, ICON_DELETE, 22.0f, s->on_surface_variant));
  /* Docked FAB */
  CHECK_RC(draw_shadow(c, 345.0f, 22.0f, 56.0f, 56.0f, 16.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 345.0f, 22.0f, 56.0f, 56.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 373.0f, 50.0f, ICON_ADD, 24.0f, s->on_primary_container));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "bottom_app_bar.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 6. tabs.png */
  CHECK_RC(canvas_create(400, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 15.0f, 25.0f, 370.0f, 50.0f, 8.0f,
                             s->surface_container));
  /* Tab 1 Active */
  CHECK_RC(draw_text_centered(c, 75.0f, 42.0f, "Overview", 1.0f, s->primary));
  CHECK_RC(draw_rounded_rect(c, 45.0f, 71.0f, 60.0f, 3.0f, 1.5f, s->primary));
  /* Tab 2 */
  CHECK_RC(draw_text_centered(c, 200.0f, 42.0f, "Specs", 1.0f,
                              s->on_surface_variant));
  /* Tab 3 */
  CHECK_RC(draw_text_centered(c, 325.0f, 42.0f, "Metrics", 1.0f,
                              s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "tabs.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 7. breadcrumbs.png */
  CHECK_RC(canvas_create(380, 80, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_text(c, 25.0f, 32.0f, "Home", 1.0f, s->primary));
  CHECK_RC(draw_icon(c, 75.0f, 40.0f, ICON_ARROW_RIGHT, 14.0f, s->outline));
  CHECK_RC(draw_text(c, 90.0f, 32.0f, "Library", 1.0f, s->primary));
  CHECK_RC(draw_icon(c, 160.0f, 40.0f, ICON_ARROW_RIGHT, 14.0f, s->outline));
  CHECK_RC(draw_text(c, 175.0f, 32.0f, "Components", 1.0f, s->primary));
  CHECK_RC(draw_icon(c, 275.0f, 40.0f, ICON_ARROW_RIGHT, 14.0f, s->outline));
  CHECK_RC(draw_text(c, 290.0f, 32.0f, "Button", 1.0f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "breadcrumbs.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 8. stepper.png */
  CHECK_RC(canvas_create(380, 100, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Connecting lines */
  CHECK_RC(draw_line(c, 60.0f, 40.0f, 190.0f, 40.0f, 2.0f, s->primary));
  CHECK_RC(
      draw_line(c, 190.0f, 40.0f, 320.0f, 40.0f, 2.0f, s->outline_variant));
  /* Step 1: Completed */
  CHECK_RC(draw_circle(c, 60.0f, 40.0f, 16.0f, s->primary));
  CHECK_RC(draw_icon(c, 60.0f, 40.0f, ICON_CHECK, 12.0f, s->on_primary));
  CHECK_RC(draw_text_centered(c, 60.0f, 65.0f, "Cart", 0.85f, s->on_surface));
  /* Step 2: Active */
  CHECK_RC(draw_circle(c, 190.0f, 40.0f, 16.0f, s->primary));
  CHECK_RC(draw_text_centered(c, 190.0f, 32.0f, "2", 1.0f, s->on_primary));
  CHECK_RC(draw_text_centered(c, 190.0f, 65.0f, "Shipping", 0.85f, s->primary));
  /* Step 3: Upcoming */
  CHECK_RC(draw_circle_stroke(c, 320.0f, 40.0f, 16.0f, 2.0f, s->outline));
  CHECK_RC(draw_text_centered(c, 320.0f, 32.0f, "3", 1.0f, s->outline));
  CHECK_RC(draw_text_centered(c, 320.0f, 65.0f, "Payment", 0.85f,
                              s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "stepper.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 9. page_indicator.png */
  CHECK_RC(canvas_create(240, 60, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_circle(c, 70.0f, 30.0f, 4.0f, s->outline_variant));
  /* Active elongated pill */
  CHECK_RC(draw_rounded_rect(c, 86.0f, 26.0f, 24.0f, 8.0f, 4.0f, s->primary));
  CHECK_RC(draw_circle(c, 124.0f, 30.0f, 4.0f, s->outline_variant));
  CHECK_RC(draw_circle(c, 142.0f, 30.0f, 4.0f, s->outline_variant));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "page_indicator.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Overlays, Dialogs & Menus                                                 */
/* ========================================================================= */

ui_error_t render_all_overlays_and_dialogs(const struct md3_color_scheme *s,
                                           const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;

  /* 1. dialog.png */
  CHECK_RC(canvas_create(340, 260, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 20.0f, 20.0f, 300.0f, 220.0f, 28.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 300.0f, 220.0f, 28.0f,
                             s->surface_container_high));
  /* Icon */
  CHECK_RC(draw_icon(c, 170.0f, 55.0f, ICON_INFO, 28.0f, s->secondary));
  /* Headline */
  CHECK_RC(draw_text_centered(c, 170.0f, 85.0f, "Reset Settings?", 1.3f,
                              s->on_surface));
  /* Supporting text */
  CHECK_RC(draw_text_centered(c, 170.0f, 115.0f, "This will restore default",
                              0.85f, s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 170.0f, 133.0f, "system preferences.", 0.85f,
                              s->on_surface_variant));
  /* Actions */
  CHECK_RC(draw_btn_text(c, 140.0f, 185.0f, 70.0f, 36.0f, "Cancel", s));
  CHECK_RC(draw_btn_filled(c, 220.0f, 185.0f, 80.0f, 36.0f, "Confirm", s));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "dialog.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. bottom_sheet.png */
  CHECK_RC(canvas_create(340, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 15.0f, 20.0f, 310.0f, 200.0f, 28.0f, 4));
  CHECK_RC(draw_rounded_rect(c, 15.0f, 20.0f, 310.0f, 200.0f, 28.0f,
                             s->surface_container_low));
  CHECK_RC(draw_icon(c, 170.0f, 35.0f, ICON_DRAG_HANDLE, 28.0f, s->outline));
  CHECK_RC(draw_text(c, 35.0f, 55.0f, "Share Options", 1.2f, s->on_surface));
  /* Option 1 */
  CHECK_RC(
      draw_icon(c, 45.0f, 100.0f, ICON_SHARE, 20.0f, s->on_surface_variant));
  CHECK_RC(
      draw_text(c, 75.0f, 92.0f, "Send copy via link", 1.0f, s->on_surface));
  /* Option 2 */
  CHECK_RC(
      draw_icon(c, 45.0f, 145.0f, ICON_EDIT, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 75.0f, 137.0f, "Collaborate with team", 1.0f,
                     s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "bottom_sheet.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. snackbar.png */
  CHECK_RC(canvas_create(360, 90, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 20.0f, 20.0f, 320.0f, 48.0f, 8.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 320.0f, 48.0f, 8.0f,
                             s->inverse_surface));
  CHECK_RC(draw_text(c, 36.0f, 36.0f, "Message sent to archive.", 0.9f,
                     s->inverse_on_surface));
  CHECK_RC(draw_text(c, 275.0f, 36.0f, "Undo", 0.95f, s->inverse_primary));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "snackbar.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 4. banner.png */
  CHECK_RC(canvas_create(360, 110, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 15.0f, 10.0f, 330.0f, 88.0f, 12.0f,
                             s->surface_container));
  CHECK_RC(draw_icon(c, 40.0f, 40.0f, ICON_WARNING, 24.0f, s->primary));
  CHECK_RC(draw_text(c, 65.0f, 28.0f, "Low storage remaining.", 1.0f,
                     s->on_surface));
  CHECK_RC(draw_text(c, 65.0f, 46.0f, "Back up files now.", 0.85f,
                     s->on_surface_variant));
  CHECK_RC(draw_text(c, 190.0f, 70.0f, "Dismiss", 0.9f, s->primary));
  CHECK_RC(draw_text(c, 260.0f, 70.0f, "Manage", 0.9f, s->primary));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "banner.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 5. inline_alert.png */
  CHECK_RC(canvas_create(360, 80, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 15.0f, 15.0f, 330.0f, 50.0f, 12.0f,
                             s->error_container));
  CHECK_RC(
      draw_icon(c, 40.0f, 40.0f, ICON_WARNING, 20.0f, s->on_error_container));
  CHECK_RC(draw_text(c, 65.0f, 32.0f, "Critical network outage detected.",
                     0.95f, s->on_error_container));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "inline_alert.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 6. tooltip.png */
  CHECK_RC(canvas_create(220, 80, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 30.0f, 20.0f, 160.0f, 36.0f, 6.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 20.0f, 160.0f, 36.0f, 6.0f,
                             s->inverse_surface));
  CHECK_RC(draw_text_centered(c, 110.0f, 30.0f, "Copy to clipboard", 0.85f,
                              s->inverse_on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "tooltip.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 7. menu.png */
  CHECK_RC(canvas_create(240, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 20.0f, 15.0f, 200.0f, 150.0f, 8.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 20.0f, 15.0f, 200.0f, 150.0f, 8.0f,
                             s->surface_container));
  /* Item 1 */
  CHECK_RC(draw_icon(c, 42.0f, 40.0f, ICON_EDIT, 16.0f, s->on_surface));
  CHECK_RC(draw_text(c, 65.0f, 32.0f, "Edit file", 0.95f, s->on_surface));
  CHECK_RC(draw_text(c, 170.0f, 32.0f, "Ctrl+E", 0.75f, s->outline));
  /* Item 2 */
  CHECK_RC(draw_icon(c, 42.0f, 75.0f, ICON_SHARE, 16.0f, s->on_surface));
  CHECK_RC(draw_text(c, 65.0f, 67.0f, "Share link", 0.95f, s->on_surface));
  /* Divider */
  CHECK_RC(
      draw_line(c, 20.0f, 100.0f, 220.0f, 100.0f, 1.0f, s->outline_variant));
  /* Item 3 (Destructive) */
  CHECK_RC(draw_icon(c, 42.0f, 125.0f, ICON_DELETE, 16.0f, s->error));
  CHECK_RC(draw_text(c, 65.0f, 117.0f, "Remove", 0.95f, s->error));
  CHECK_RC(draw_text(c, 175.0f, 117.0f, "Del", 0.75f, s->outline));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "menu.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 8. color_picker.png */
  CHECK_RC(canvas_create(280, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 20.0f, 15.0f, 240.0f, 190.0f, 16.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 20.0f, 15.0f, 240.0f, 190.0f, 16.0f,
                             s->surface_container));
  /* Color palette preview block */
  CHECK_RC(draw_rounded_rect(c, 35.0f, 30.0f, 210.0f, 80.0f, 8.0f, s->primary));
  /* Hue slider */
  CHECK_RC(draw_rounded_rect(c, 35.0f, 125.0f, 210.0f, 16.0f, 8.0f,
                             s->secondary_container));
  CHECK_RC(draw_circle(c, 120.0f, 133.0f, 10.0f, s->primary));
  CHECK_RC(draw_circle_stroke(c, 120.0f, 133.0f, 10.0f, 2.0f, s->surface));
  /* Hex value */
  CHECK_RC(draw_text(c, 35.0f, 162.0f, "HEX: #6750A4", 1.0f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "color_picker.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 9. command_palette.png */
  CHECK_RC(canvas_create(360, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_shadow(c, 15.0f, 15.0f, 330.0f, 190.0f, 16.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 15.0f, 15.0f, 330.0f, 190.0f, 16.0f,
                             s->surface_container_high));
  /* Search input */
  CHECK_RC(draw_icon(c, 38.0f, 42.0f, ICON_SEARCH, 20.0f, s->primary));
  CHECK_RC(draw_text(c, 60.0f, 34.0f, "Type command...", 1.0f,
                     s->on_surface_variant));
  CHECK_RC(draw_line(c, 15.0f, 65.0f, 345.0f, 65.0f, 1.0f, s->outline_variant));
  /* Result 1 (Active) */
  CHECK_RC(draw_rounded_rect(c, 25.0f, 75.0f, 310.0f, 36.0f, 8.0f,
                             s->secondary_container));
  CHECK_RC(draw_icon(c, 45.0f, 93.0f, ICON_SETTINGS, 16.0f,
                     s->on_secondary_container));
  CHECK_RC(draw_text(c, 68.0f, 85.0f, "Open User Settings", 0.95f,
                     s->on_secondary_container));
  /* Result 2 */
  CHECK_RC(
      draw_icon(c, 45.0f, 130.0f, ICON_FOLDER, 16.0f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 68.0f, 122.0f, "Browse Recent Projects", 0.95f,
                     s->on_surface));
  /* Result 3 */
  CHECK_RC(
      draw_icon(c, 45.0f, 165.0f, ICON_STAR, 16.0f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 68.0f, 157.0f, "View Component Catalog", 0.95f,
                     s->on_surface));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "command_palette.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Data & Hierarchy                                                          */
/* ========================================================================= */

ui_error_t render_all_data_and_hierarchy(const struct md3_color_scheme *s,
                                         const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;

  /* 1. table.png */
  CHECK_RC(canvas_create(440, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect_stroke(c, 15.0f, 15.0f, 410.0f, 190.0f, 12.0f,
                                    1.0f, s->outline_variant));
  /* Header Row */
  CHECK_RC(draw_rounded_rect(c, 15.0f, 15.0f, 410.0f, 40.0f, 12.0f,
                             s->surface_container));
  CHECK_RC(draw_rounded_rect_stroke(c, 30.0f, 26.0f, 16.0f, 16.0f, 2.0f, 1.5f,
                                    s->outline));
  CHECK_RC(draw_text(c, 65.0f, 28.0f, "Item Name", 0.9f, s->on_surface));
  CHECK_RC(draw_icon(c, 145.0f, 35.0f, ICON_ARROW_DOWN, 12.0f, s->on_surface));
  CHECK_RC(draw_text(c, 195.0f, 28.0f, "Category", 0.9f, s->on_surface));
  CHECK_RC(draw_text(c, 320.0f, 28.0f, "Price", 0.9f, s->on_surface));
  CHECK_RC(draw_line(c, 15.0f, 55.0f, 425.0f, 55.0f, 1.0f, s->outline_variant));

  /* Row 1 */
  CHECK_RC(draw_rounded_rect_stroke(c, 30.0f, 72.0f, 16.0f, 16.0f, 2.0f, 1.5f,
                                    s->outline));
  CHECK_RC(draw_text(c, 65.0f, 72.0f, "Widget Alpha", 0.95f, s->on_surface));
  CHECK_RC(
      draw_text(c, 195.0f, 72.0f, "Electronics", 0.85f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 320.0f, 72.0f, "$49.99", 0.95f, s->on_surface));
  CHECK_RC(
      draw_line(c, 15.0f, 100.0f, 425.0f, 100.0f, 1.0f, s->outline_variant));

  /* Row 2 (Selected) */
  CHECK_RC(draw_rounded_rect(c, 16.0f, 101.0f, 408.0f, 45.0f, 0.0f,
                             s->secondary_container));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 116.0f, 16.0f, 16.0f, 2.0f, s->primary));
  CHECK_RC(draw_icon(c, 38.0f, 124.0f, ICON_CHECK, 12.0f, s->on_primary));
  CHECK_RC(draw_text(c, 65.0f, 116.0f, "Widget Beta", 0.95f,
                     s->on_secondary_container));
  CHECK_RC(draw_text(c, 195.0f, 116.0f, "Industrial", 0.85f,
                     s->on_secondary_container));
  CHECK_RC(draw_text(c, 320.0f, 116.0f, "$129.00", 0.95f,
                     s->on_secondary_container));
  CHECK_RC(
      draw_line(c, 15.0f, 146.0f, 425.0f, 146.0f, 1.0f, s->outline_variant));

  /* Row 3 */
  CHECK_RC(draw_rounded_rect_stroke(c, 30.0f, 162.0f, 16.0f, 16.0f, 2.0f, 1.5f,
                                    s->outline));
  CHECK_RC(draw_text(c, 65.0f, 162.0f, "Widget Gamma", 0.95f, s->on_surface));
  CHECK_RC(
      draw_text(c, 195.0f, 162.0f, "Hardware", 0.85f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 320.0f, 162.0f, "$18.50", 0.95f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "table.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. paginator.png */
  CHECK_RC(canvas_create(380, 80, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_text(c, 25.0f, 32.0f, "Rows per page: 10", 0.9f,
                     s->on_surface_variant));
  CHECK_RC(draw_icon(c, 185.0f, 40.0f, ICON_ARROW_DOWN, 12.0f,
                     s->on_surface_variant));
  CHECK_RC(draw_text(c, 220.0f, 32.0f, "1-10 of 42", 0.9f, s->on_surface));
  CHECK_RC(
      draw_icon(c, 310.0f, 40.0f, ICON_ARROW_LEFT, 16.0f, s->outline_variant));
  CHECK_RC(draw_icon(c, 345.0f, 40.0f, ICON_ARROW_RIGHT, 16.0f, s->on_surface));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "paginator.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. tree.png */
  CHECK_RC(canvas_create(300, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Level 0 */
  CHECK_RC(draw_icon(c, 35.0f, 30.0f, ICON_ARROW_DOWN, 14.0f,
                     s->on_surface_variant));
  CHECK_RC(draw_icon(c, 55.0f, 30.0f, ICON_FOLDER, 16.0f, s->primary));
  CHECK_RC(draw_text(c, 75.0f, 22.0f, "src", 1.0f, s->on_surface));
  /* Level 1 */
  CHECK_RC(draw_icon(c, 65.0f, 65.0f, ICON_ARROW_DOWN, 14.0f,
                     s->on_surface_variant));
  CHECK_RC(draw_icon(c, 85.0f, 65.0f, ICON_FOLDER, 16.0f, s->primary));
  CHECK_RC(draw_text(c, 105.0f, 57.0f, "material3", 1.0f, s->on_surface));
  /* Level 2 Leaves */
  CHECK_RC(
      draw_text(c, 125.0f, 95.0f, "md3_button.c", 0.9f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 125.0f, 125.0f, "md3_datepicker.c", 0.9f,
                     s->on_surface_variant));
  CHECK_RC(
      draw_text(c, 125.0f, 155.0f, "md3_card.c", 0.9f, s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "tree.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 4. transfer_list.png */
  CHECK_RC(canvas_create(380, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Left Box */
  CHECK_RC(draw_rounded_rect_stroke(c, 20.0f, 20.0f, 140.0f, 140.0f, 8.0f, 1.0f,
                                    s->outline_variant));
  CHECK_RC(draw_text(c, 35.0f, 35.0f, "Available", 0.9f, s->primary));
  CHECK_RC(draw_line(c, 20.0f, 55.0f, 160.0f, 55.0f, 1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 35.0f, 70.0f, "Item 1", 0.85f, s->on_surface));
  CHECK_RC(draw_text(c, 35.0f, 95.0f, "Item 2", 0.85f, s->on_surface));
  /* Center Transfer buttons */
  CHECK_RC(draw_circle(c, 190.0f, 70.0f, 16.0f, s->surface_container_high));
  CHECK_RC(draw_icon(c, 190.0f, 70.0f, ICON_ARROW_RIGHT, 14.0f, s->on_surface));
  CHECK_RC(draw_circle(c, 190.0f, 110.0f, 16.0f, s->surface_container_high));
  CHECK_RC(
      draw_icon(c, 190.0f, 110.0f, ICON_ARROW_LEFT, 14.0f, s->outline_variant));
  /* Right Box */
  CHECK_RC(draw_rounded_rect_stroke(c, 220.0f, 20.0f, 140.0f, 140.0f, 8.0f,
                                    1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 235.0f, 35.0f, "Selected", 0.9f, s->primary));
  CHECK_RC(
      draw_line(c, 220.0f, 55.0f, 360.0f, 55.0f, 1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 235.0f, 70.0f, "Item 3", 0.85f, s->on_surface));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "transfer_list.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Media & Workspace                                                         */
/* ========================================================================= */

ui_error_t render_all_media_and_workspace(const struct md3_color_scheme *s,
                                          const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;

  /* 1. chat_bubble.png */
  CHECK_RC(canvas_create(340, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Incoming Bubble */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 200.0f, 50.0f, 16.0f,
                             s->surface_container_high));
  CHECK_RC(draw_text(c, 35.0f, 32.0f, "Hi! How is the M3 design?", 0.9f,
                     s->on_surface));
  CHECK_RC(draw_text(c, 175.0f, 50.0f, "10:14", 0.7f, s->on_surface_variant));
  /* Outgoing Bubble */
  CHECK_RC(draw_rounded_rect(c, 120.0f, 90.0f, 200.0f, 50.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(draw_text(c, 135.0f, 102.0f, "It looks truly pristine!", 0.9f,
                     s->on_primary_container));
  CHECK_RC(
      draw_text(c, 275.0f, 120.0f, "10:15", 0.7f, s->on_primary_container));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "chat_bubble.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. timeline.png */
  CHECK_RC(canvas_create(320, 200, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Vertical line */
  CHECK_RC(draw_line(c, 40.0f, 25.0f, 40.0f, 175.0f, 2.0f, s->outline_variant));
  /* Node 1 */
  CHECK_RC(draw_circle(c, 40.0f, 40.0f, 10.0f, s->primary));
  CHECK_RC(draw_text(c, 65.0f, 32.0f, "Project Kickoff", 1.0f, s->on_surface));
  CHECK_RC(
      draw_text(c, 65.0f, 48.0f, "September 1", 0.8f, s->on_surface_variant));
  /* Node 2 */
  CHECK_RC(draw_circle(c, 40.0f, 100.0f, 10.0f, s->primary));
  CHECK_RC(
      draw_text(c, 65.0f, 92.0f, "M3 Component Beta", 1.0f, s->on_surface));
  CHECK_RC(
      draw_text(c, 65.0f, 108.0f, "September 15", 0.8f, s->on_surface_variant));
  /* Node 3 */
  CHECK_RC(draw_circle(c, 40.0f, 160.0f, 10.0f, s->secondary_container));
  CHECK_RC(
      draw_text(c, 65.0f, 152.0f, "Production Release", 1.0f, s->on_surface));
  CHECK_RC(
      draw_text(c, 65.0f, 168.0f, "October 1", 0.8f, s->on_surface_variant));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "timeline.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. file_uploader.png */
  CHECK_RC(canvas_create(340, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect_stroke(c, 20.0f, 20.0f, 300.0f, 140.0f, 12.0f,
                                    1.5f, s->primary));
  CHECK_RC(draw_icon(c, 170.0f, 60.0f, ICON_CLOUD_UPLOAD, 36.0f, s->primary));
  CHECK_RC(draw_text_centered(c, 170.0f, 95.0f, "Drag & drop files here", 1.0f,
                              s->on_surface));
  CHECK_RC(
      draw_btn_filled_tonal(c, 120.0f, 115.0f, 100.0f, 32.0f, "Browse", s));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "file_uploader.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 4. rich_text_editor.png */
  CHECK_RC(canvas_create(380, 180, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect_stroke(c, 15.0f, 15.0f, 350.0f, 150.0f, 12.0f,
                                    1.0f, s->outline_variant));
  /* Toolbar */
  CHECK_RC(draw_rounded_rect(c, 15.0f, 15.0f, 350.0f, 40.0f, 12.0f,
                             s->surface_container));
  CHECK_RC(draw_text(c, 35.0f, 27.0f, "B", 1.2f, s->on_surface));
  CHECK_RC(draw_text(c, 65.0f, 27.0f, "I", 1.2f, s->on_surface));
  CHECK_RC(draw_text(c, 95.0f, 27.0f, "U", 1.2f, s->on_surface));
  CHECK_RC(
      draw_line(c, 120.0f, 20.0f, 120.0f, 50.0f, 1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 135.0f, 27.0f, "H1", 1.0f, s->on_surface));
  CHECK_RC(draw_text(c, 170.0f, 27.0f, "H2", 1.0f, s->on_surface));
  CHECK_RC(draw_line(c, 15.0f, 55.0f, 365.0f, 55.0f, 1.0f, s->outline_variant));
  /* Document Content */
  CHECK_RC(draw_text(c, 30.0f, 75.0f, "Material 3 Component Specifications",
                     1.0f, s->on_surface));
  CHECK_RC(draw_text(c, 30.0f, 100.0f,
                     "Rich text formatting support with ease.", 0.85f,
                     s->on_surface_variant));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "rich_text_editor.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 5. video_player.png */
  CHECK_RC(canvas_create(360, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Video canvas */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 20.0f, 320.0f, 180.0f, 16.0f,
                             UI_COLOR_ARGB(255, 20, 20, 25)));
  CHECK_RC(
      draw_circle(c, 180.0f, 100.0f, 28.0f, UI_COLOR_ARGB(200, 255, 255, 255)));
  CHECK_RC(draw_icon(c, 182.0f, 100.0f, ICON_PLAY, 22.0f,
                     UI_COLOR_ARGB(255, 30, 30, 35)));
  /* Control bar */
  CHECK_RC(draw_rounded_rect(c, 20.0f, 160.0f, 320.0f, 40.0f, 16.0f,
                             UI_COLOR_ARGB(220, 30, 30, 35)));
  CHECK_RC(draw_icon(c, 40.0f, 180.0f, ICON_PAUSE, 16.0f,
                     UI_COLOR_ARGB(255, 255, 255, 255)));
  CHECK_RC(draw_rounded_rect(c, 65.0f, 178.0f, 180.0f, 4.0f, 2.0f,
                             UI_COLOR_ARGB(120, 255, 255, 255)));
  CHECK_RC(draw_rounded_rect(c, 65.0f, 178.0f, 90.0f, 4.0f, 2.0f, s->primary));
  CHECK_RC(draw_circle(c, 155.0f, 180.0f, 6.0f, s->primary));
  CHECK_RC(draw_text(c, 255.0f, 174.0f, "02:14 / 04:30", 0.65f,
                     UI_COLOR_ARGB(255, 255, 255, 255)));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "video_player.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 6. dockable_layout.png */
  CHECK_RC(canvas_create(380, 220, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Window Frame */
  CHECK_RC(draw_rounded_rect_stroke(c, 15.0f, 15.0f, 350.0f, 190.0f, 12.0f,
                                    1.0f, s->outline_variant));
  /* Top App Header */
  CHECK_RC(draw_rounded_rect(c, 15.0f, 15.0f, 350.0f, 32.0f, 12.0f,
                             s->surface_container));
  CHECK_RC(draw_text(c, 30.0f, 24.0f, "Workspace - Project Explorer", 0.85f,
                     s->on_surface));
  /* Left Sidebar Pane */
  CHECK_RC(draw_rounded_rect(c, 16.0f, 48.0f, 100.0f, 156.0f, 0.0f,
                             s->surface_container_low));
  CHECK_RC(draw_text(c, 25.0f, 60.0f, "Files", 0.8f, s->primary));
  CHECK_RC(
      draw_line(c, 116.0f, 48.0f, 116.0f, 204.0f, 1.0f, s->outline_variant));
  /* Main Editor Pane */
  CHECK_RC(draw_text(c, 130.0f, 60.0f, "main.c", 0.85f, s->on_surface));
  CHECK_RC(
      draw_line(c, 116.0f, 140.0f, 364.0f, 140.0f, 1.0f, s->outline_variant));
  /* Bottom Terminal Pane */
  CHECK_RC(
      draw_text(c, 130.0f, 150.0f, "Terminal", 0.8f, s->on_surface_variant));
  CHECK_RC(draw_text(c, 130.0f, 170.0f, "$ ./screenshot_material3", 0.75f,
                     s->primary));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "dockable_layout.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Progress & Expressive Shapes                                              */
/* ========================================================================= */

ui_error_t render_all_progress_and_shapes(const struct md3_color_scheme *s,
                                          const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;
  int i;

  /* 1. progress_linear.png */
  CHECK_RC(canvas_create(320, 80, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 36.0f, 260.0f, 8.0f, 4.0f,
                             s->secondary_container));
  CHECK_RC(draw_rounded_rect(c, 30.0f, 36.0f, 175.0f, 8.0f, 4.0f, s->primary));
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "progress_linear.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 2. progress_circular.png */
  CHECK_RC(canvas_create(140, 140, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  CHECK_RC(
      draw_circle_stroke(c, 70.0f, 70.0f, 40.0f, 6.0f, s->secondary_container));
  /* Arc segment */
  for (i = 0; i < 20; ++i) {
    float angle = -1.5707963f + (float)i * 0.22f;
    float px = 70.0f + cosf(angle) * 40.0f;
    float py = 70.0f + sinf(angle) * 40.0f;
    CHECK_RC(draw_circle(c, px, py, 3.0f, s->primary));
  }
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "progress_circular.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 3. loading_indicator.png */
  CHECK_RC(canvas_create(160, 80, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  for (i = 0; i < 3; ++i) {
    float cx = 50.0f + (float)i * 30.0f;
    float cy = 40.0f - (i == 1 ? 6.0f : 0.0f);
    CHECK_RC(draw_circle(c, cx, cy, 7.0f,
                         (i == 1) ? s->primary : s->primary_container));
  }
  CHECK_RC(
      make_path(path, sizeof(path), opts->output_dir, "loading_indicator.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  /* 4. shape_morph.png (Expressive shapes) */
  CHECK_RC(canvas_create(440, 140, &c));
  CHECK_RC(canvas_clear(c, s->surface));
  /* Shape 1: Circle */
  CHECK_RC(draw_circle(c, 60.0f, 70.0f, 40.0f, s->primary_container));
  /* Shape 2: Rounded Rect */
  CHECK_RC(draw_rounded_rect(c, 130.0f, 30.0f, 80.0f, 80.0f, 24.0f,
                             s->secondary_container));
  /* Shape 3: 8-point Star / Flower */
  for (i = 0; i < 8; ++i) {
    float angle = (float)i * (3.14159265f / 4.0f);
    float px = 280.0f + cosf(angle) * 20.0f;
    float py = 70.0f + sinf(angle) * 20.0f;
    CHECK_RC(draw_circle(c, px, py, 24.0f, s->tertiary_container));
  }
  CHECK_RC(draw_circle(c, 280.0f, 70.0f, 22.0f, s->tertiary));

  /* Shape 4: Pill */
  CHECK_RC(draw_rounded_rect(c, 350.0f, 45.0f, 75.0f, 50.0f, 25.0f,
                             s->surface_container_highest));
  CHECK_RC(make_path(path, sizeof(path), opts->output_dir, "shape_morph.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Catalog Overview Hero Showcase                                            */
/* ========================================================================= */

ui_error_t render_catalog_overview(const struct md3_color_scheme *s,
                                   const struct render_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  if (!s || !opts || !opts->output_dir)
    return UI_ERROR_INVALID_ARGUMENT;

  CHECK_RC(canvas_create(960, 680, &c));
  CHECK_RC(canvas_clear(c, s->surface));

  /* Hero Header */
  CHECK_RC(draw_text(c, 40.0f, 30.0f, "Material 3 Component System", 1.8f,
                     s->on_surface));
  CHECK_RC(
      draw_text(c, 40.0f, 65.0f,
                "C-Multiplatform Native & Cross-Platform UI Component Gallery",
                1.0f, s->on_surface_variant));
  CHECK_RC(draw_line(c, 40.0f, 90.0f, 920.0f, 90.0f, 1.0f, s->outline_variant));

  /* Section 1: Buttons */
  CHECK_RC(draw_text(c, 40.0f, 110.0f, "Buttons & Actions", 1.1f, s->primary));
  CHECK_RC(draw_btn_elevated(c, 40.0f, 135.0f, 110.0f, 40.0f, "Elevated", s));
  CHECK_RC(draw_btn_filled(c, 160.0f, 135.0f, 110.0f, 40.0f, "Filled", s));
  CHECK_RC(draw_btn_filled_tonal(c, 280.0f, 135.0f, 110.0f, 40.0f, "Tonal", s));
  CHECK_RC(draw_btn_outlined(c, 400.0f, 135.0f, 110.0f, 40.0f, "Outlined", s));

  /* FAB */
  CHECK_RC(draw_shadow(c, 525.0f, 131.0f, 48.0f, 48.0f, 16.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 525.0f, 131.0f, 48.0f, 48.0f, 16.0f,
                             s->primary_container));
  CHECK_RC(
      draw_icon(c, 549.0f, 155.0f, ICON_ADD, 22.0f, s->on_primary_container));

  /* Section 2: Cards & Surfaces */
  CHECK_RC(draw_text(c, 620.0f, 110.0f, "Card Component", 1.1f, s->primary));
  CHECK_RC(draw_shadow(c, 620.0f, 135.0f, 300.0f, 180.0f, 16.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 620.0f, 135.0f, 300.0f, 180.0f, 16.0f,
                             s->surface_container_low));
  CHECK_RC(draw_text(c, 640.0f, 155.0f, "Elevated Card", 1.2f, s->on_surface));
  CHECK_RC(draw_text(c, 640.0f, 180.0f, "Material 3 design token styling",
                     0.85f, s->on_surface_variant));
  CHECK_RC(draw_btn_filled(c, 800.0f, 260.0f, 100.0f, 36.0f, "Action", s));

  /* Section 3: Pickers (Datepicker mini) */
  CHECK_RC(
      draw_text(c, 40.0f, 205.0f, "Datepicker & Calendar", 1.1f, s->primary));
  CHECK_RC(draw_shadow(c, 40.0f, 230.0f, 250.0f, 220.0f, 20.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 40.0f, 230.0f, 250.0f, 220.0f, 20.0f,
                             s->surface_container_high));
  CHECK_RC(draw_text(c, 55.0f, 245.0f, "Wed, Sep 30", 1.2f, s->on_surface));
  CHECK_RC(
      draw_line(c, 40.0f, 275.0f, 290.0f, 275.0f, 1.0f, s->outline_variant));
  CHECK_RC(draw_text(c, 55.0f, 288.0f, "September 2026", 0.9f, s->on_surface));
  /* Mini day grid */
  {
    int r_idx, c_idx, d = 1;
    for (r_idx = 0; r_idx < 3; ++r_idx) {
      for (c_idx = 0; c_idx < 7; ++c_idx) {
        char buf[4];
        float cx = 60.0f + (float)c_idx * 30.0f;
        float cy = 325.0f + (float)r_idx * 28.0f;
#if defined(_MSC_VER)
        sprintf_s(buf, sizeof(buf), "%d", d);
#else
        snprintf(buf, sizeof(buf), "%d", d);
#endif
        if (d == 15) {
          CHECK_RC(draw_circle(c, cx, cy, 12.0f, s->primary));
          CHECK_RC(
              draw_text_centered(c, cx, cy - 6.0f, buf, 0.8f, s->on_primary));
        } else {
          CHECK_RC(
              draw_text_centered(c, cx, cy - 6.0f, buf, 0.8f, s->on_surface));
        }
        d++;
      }
    }
  }

  /* Section 4: Selection Controls */
  CHECK_RC(
      draw_text(c, 320.0f, 205.0f, "Selection Controls", 1.1f, s->primary));
  /* Switch */
  CHECK_RC(
      draw_rounded_rect(c, 320.0f, 235.0f, 52.0f, 32.0f, 16.0f, s->primary));
  CHECK_RC(draw_circle(c, 356.0f, 251.0f, 12.0f, s->on_primary));
  CHECK_RC(draw_icon(c, 356.0f, 251.0f, ICON_CHECK, 12.0f, s->primary));
  CHECK_RC(draw_text(c, 385.0f, 243.0f, "Switch ON", 0.9f, s->on_surface));

  /* Checkbox */
  CHECK_RC(
      draw_rounded_rect(c, 320.0f, 280.0f, 20.0f, 20.0f, 2.0f, s->primary));
  CHECK_RC(draw_icon(c, 330.0f, 290.0f, ICON_CHECK, 14.0f, s->on_primary));
  CHECK_RC(draw_text(c, 350.0f, 282.0f, "Checkbox", 0.9f, s->on_surface));

  /* Radio */
  CHECK_RC(draw_circle_stroke(c, 330.0f, 325.0f, 10.0f, 2.0f, s->primary));
  CHECK_RC(draw_circle(c, 330.0f, 325.0f, 5.0f, s->primary));
  CHECK_RC(draw_text(c, 350.0f, 318.0f, "Radio selected", 0.9f, s->on_surface));

  /* Slider */
  CHECK_RC(draw_rounded_rect(c, 320.0f, 365.0f, 250.0f, 16.0f, 8.0f,
                             s->secondary_container));
  CHECK_RC(
      draw_rounded_rect(c, 320.0f, 365.0f, 150.0f, 16.0f, 8.0f, s->primary));
  CHECK_RC(draw_rounded_rect(c, 468.0f, 351.0f, 4.0f, 44.0f, 2.0f, s->primary));

  /* Section 5: Dialog (Mini) */
  CHECK_RC(draw_text(c, 620.0f, 340.0f, "Modal Dialog", 1.1f, s->primary));
  CHECK_RC(draw_shadow(c, 620.0f, 365.0f, 300.0f, 150.0f, 24.0f, 3));
  CHECK_RC(draw_rounded_rect(c, 620.0f, 365.0f, 300.0f, 150.0f, 24.0f,
                             s->surface_container_high));
  CHECK_RC(draw_icon(c, 770.0f, 395.0f, ICON_INFO, 24.0f, s->secondary));
  CHECK_RC(draw_text_centered(c, 770.0f, 420.0f, "Confirm changes?", 1.1f,
                              s->on_surface));
  CHECK_RC(draw_btn_text(c, 710.0f, 465.0f, 60.0f, 32.0f, "Cancel", s));
  CHECK_RC(draw_btn_filled(c, 785.0f, 465.0f, 75.0f, 32.0f, "Apply", s));

  /* Section 6: Navigation Bar Bottom Banner */
  CHECK_RC(draw_shadow(c, 40.0f, 550.0f, 880.0f, 80.0f, 16.0f, 2));
  CHECK_RC(draw_rounded_rect(c, 40.0f, 550.0f, 880.0f, 80.0f, 16.0f,
                             s->surface_container));
  /* Nav 1 Active */
  CHECK_RC(draw_rounded_rect(c, 90.0f, 565.0f, 70.0f, 32.0f, 16.0f,
                             s->secondary_container));
  CHECK_RC(draw_icon(c, 125.0f, 581.0f, ICON_HOME, 20.0f,
                     s->on_secondary_container));
  CHECK_RC(draw_text_centered(c, 125.0f, 604.0f, "Home", 0.8f, s->on_surface));
  /* Nav 2 */
  CHECK_RC(
      draw_icon(c, 320.0f, 581.0f, ICON_SEARCH, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 320.0f, 604.0f, "Search", 0.8f,
                              s->on_surface_variant));
  /* Nav 3 */
  CHECK_RC(
      draw_icon(c, 520.0f, 581.0f, ICON_BELL, 20.0f, s->on_surface_variant));
  CHECK_RC(draw_circle(c, 530.0f, 573.0f, 4.0f, s->error));
  CHECK_RC(draw_text_centered(c, 520.0f, 604.0f, "Alerts", 0.8f,
                              s->on_surface_variant));
  /* Nav 4 */
  CHECK_RC(draw_icon(c, 720.0f, 581.0f, ICON_SETTINGS, 20.0f,
                     s->on_surface_variant));
  CHECK_RC(draw_text_centered(c, 720.0f, 604.0f, "Settings", 0.8f,
                              s->on_surface_variant));

  CHECK_RC(make_path(path, sizeof(path), opts->output_dir,
                     "material3_catalog_overview.png"));
  CHECK_RC(save_canvas_png(c, path));
  CHECK_RC(canvas_destroy(c));
  c = NULL;

  return UI_ERROR_NONE;
}
