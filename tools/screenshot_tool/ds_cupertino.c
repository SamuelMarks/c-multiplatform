/**
 * @file ds_cupertino.c
 * @brief Cupertino (Apple iOS / macOS HIG) design system screenshot generator.
 */

/* clang-format off */
#include "design_system_renderer.h"
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

struct cupertino_palette {
  ui_color_t system_blue;
  ui_color_t system_green;
  ui_color_t system_red;
  ui_color_t system_orange;
  ui_color_t background;
  ui_color_t surface;
  ui_color_t text_primary;
  ui_color_t text_secondary;
  ui_color_t separator;
  ui_color_t fill_tertiary;
};

static ui_error_t init_cupertino_palette(int is_dark,
                                         struct cupertino_palette *p) {
  if (!p) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (is_dark) {
    p->system_blue = UI_COLOR_ARGB(255, 10, 132, 255);
    p->system_green = UI_COLOR_ARGB(255, 48, 209, 88);
    p->system_red = UI_COLOR_ARGB(255, 255, 69, 58);
    p->system_orange = UI_COLOR_ARGB(255, 255, 159, 10);
    p->background = UI_COLOR_ARGB(255, 0, 0, 0);
    p->surface = UI_COLOR_ARGB(255, 28, 28, 30);
    p->text_primary = UI_COLOR_ARGB(255, 255, 255, 255);
    p->text_secondary = UI_COLOR_ARGB(255, 142, 142, 147);
    p->separator = UI_COLOR_ARGB(255, 56, 56, 58);
    p->fill_tertiary = UI_COLOR_ARGB(255, 44, 44, 46);
  } else {
    p->system_blue = UI_COLOR_ARGB(255, 0, 122, 255);
    p->system_green = UI_COLOR_ARGB(255, 52, 199, 89);
    p->system_red = UI_COLOR_ARGB(255, 255, 59, 48);
    p->system_orange = UI_COLOR_ARGB(255, 255, 149, 0);
    p->background = UI_COLOR_ARGB(255, 242, 242, 247);
    p->surface = UI_COLOR_ARGB(255, 255, 255, 255);
    p->text_primary = UI_COLOR_ARGB(255, 0, 0, 0);
    p->text_secondary = UI_COLOR_ARGB(255, 142, 142, 147);
    p->separator = UI_COLOR_ARGB(255, 218, 218, 222);
    p->fill_tertiary = UI_COLOR_ARGB(255, 230, 230, 235);
  }
  return UI_ERROR_NONE;
}

static ui_error_t
cupertino_render_all(const struct design_system_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  struct cupertino_palette p;
  ui_error_t rc;
  int col, row, day;
  const char *wks[7];

  wks[0] = "SUN";
  wks[1] = "MON";
  wks[2] = "TUE";
  wks[3] = "WED";
  wks[4] = "THU";
  wks[5] = "FRI";
  wks[6] = "SAT";

  if (!opts || !opts->output_dir) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = init_cupertino_palette(opts->is_dark, &p);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* 1. button_filled.png */
  rc = canvas_create(220, 90, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 25.0f, 25.0f, 170.0f, 44.0f, 12.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 110.0f, 38.0f, "Continue", 1.05f,
                          UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_filled.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 2. button_tinted.png */
  rc = canvas_create(220, 90, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 25.0f, 25.0f, 170.0f, 44.0f, 12.0f,
                         UI_COLOR_ARGB(40, 0, 122, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 110.0f, 38.0f, "Details", 1.05f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_tinted.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 3. button_plain.png */
  rc = canvas_create(200, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_text_centered(c, 100.0f, 32.0f, "Action Link", 1.05f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_plain.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 4. button_bordered.png */
  rc = canvas_create(220, 90, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 25.0f, 25.0f, 170.0f, 44.0f, 12.0f, 1.5f,
                                p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 110.0f, 38.0f, "Bordered", 1.05f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_bordered.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 5. button_segmented.png */
  rc = canvas_create(320, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 20.0f, 20.0f, 280.0f, 36.0f, 9.0f, p.fill_tertiary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 22.0f, 22.0f, 90.0f, 32.0f, 7.0f, 1);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 22.0f, 22.0f, 90.0f, 32.0f, 7.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 67.0f, 30.0f, "Daily", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 160.0f, 30.0f, "Weekly", 0.95f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 253.0f, 30.0f, "Monthly", 0.95f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_segmented.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 6. switch.png */
  rc = canvas_create(280, 110, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 20.0f, 51.0f, 31.0f, 15.5f, p.system_green);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 52.0f, 22.0f, 27.0f, 27.0f, 13.5f, 1);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle(c, 65.5f, 35.5f, 13.5f, UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 95.0f, 27.0f, "Airplane Mode", 1.0f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 60.0f, 51.0f, 31.0f, 15.5f, p.fill_tertiary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 32.0f, 62.0f, 27.0f, 27.0f, 13.5f, 1);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle(c, 45.5f, 75.5f, 13.5f, UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 95.0f, 67.0f, "Bluetooth", 1.0f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "switch.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 7. slider.png */
  rc = canvas_create(320, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 38.0f, 260.0f, 4.0f, 2.0f, p.fill_tertiary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 38.0f, 160.0f, 4.0f, 2.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 176.0f, 26.0f, 28.0f, 28.0f, 14.0f, 2);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle(c, 190.0f, 40.0f, 14.0f, UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "slider.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 8. stepper.png */
  rc = canvas_create(220, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 22.0f, 94.0f, 32.0f, 8.0f, p.fill_tertiary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 53.0f, 28.0f, "-", 1.3f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 77.0f, 26.0f, 77.0f, 50.0f, 1.0f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 101.0f, 28.0f, "+", 1.3f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 135.0f, 30.0f, "Qty: 3", 1.0f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "stepper.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 9. datepicker.png */
  rc = canvas_create(340, 360, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 15.0f, 15.0f, 310.0f, 330.0f, 14.0f, 2);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 15.0f, 15.0f, 310.0f, 330.0f, 14.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 30.0f, 35.0f, "September 2026", 1.2f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 260.0f, 42.0f, ICON_ARROW_LEFT, 16.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 295.0f, 42.0f, ICON_ARROW_RIGHT, 16.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  for (col = 0; col < 7; ++col) {
    rc = draw_text_centered(c, 40.0f + (float)col * 38.0f, 72.0f, wks[col],
                            0.65f, p.text_secondary);
    if (rc != UI_ERROR_NONE) {
      canvas_destroy(c);
      return rc;
    }
  }
  day = 1;
  for (row = 0; row < 5; ++row) {
    for (col = 0; col < 7; ++col) {
      char num[4];
      float cx = 40.0f + (float)col * 38.0f;
      float cy = 115.0f + (float)row * 40.0f;
      if (row == 0 && col < 2)
        continue;
      if (day > 30)
        break;
#if defined(_MSC_VER)
      sprintf_s(num, sizeof(num), "%d", day);
#else
      snprintf(num, sizeof(num), "%d", day);
#endif
      if (day == 30) {
        rc = draw_circle(c, cx, cy, 16.0f, p.system_blue);
        if (rc != UI_ERROR_NONE) {
          canvas_destroy(c);
          return rc;
        }
        rc = draw_text_centered(c, cx, cy - 8.0f, num, 0.95f,
                                UI_COLOR_ARGB(255, 255, 255, 255));
        if (rc != UI_ERROR_NONE) {
          canvas_destroy(c);
          return rc;
        }
      } else {
        rc = draw_text_centered(c, cx, cy - 8.0f, num, 0.95f, p.text_primary);
        if (rc != UI_ERROR_NONE) {
          canvas_destroy(c);
          return rc;
        }
      }
      day++;
    }
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "datepicker.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 10. alert_dialog.png */
  rc = canvas_create(300, 200, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 15.0f, 15.0f, 270.0f, 170.0f, 14.0f, 3);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 15.0f, 15.0f, 270.0f, 170.0f, 14.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 150.0f, 35.0f, "Delete Photo?", 1.15f,
                          p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 150.0f, 62.0f, "This photo will be deleted", 0.8f,
                          p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_text_centered(c, 150.0f, 78.0f, "from iCloud Photos on all devices.",
                         0.8f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 15.0f, 120.0f, 285.0f, 120.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 150.0f, 120.0f, 150.0f, 185.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 82.0f, 145.0f, "Cancel", 1.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 217.0f, 145.0f, "Delete", 1.0f, p.system_red);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "alert_dialog.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 11. action_sheet.png */
  rc = canvas_create(320, 220, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 20.0f, 15.0f, 280.0f, 130.0f, 14.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 160.0f, 28.0f, "Choose Destination", 0.85f,
                          p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 20.0f, 50.0f, 300.0f, 50.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 160.0f, 65.0f, "Save to Files", 1.05f,
                          p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 20.0f, 95.0f, 300.0f, 95.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_text_centered(c, 160.0f, 110.0f, "Remove Item", 1.05f, p.system_red);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 20.0f, 155.0f, 280.0f, 50.0f, 14.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 160.0f, 172.0f, "Cancel", 1.1f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "action_sheet.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 12. navigation_bar.png */
  rc = canvas_create(360, 110, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 0.0f, 0.0f, 360.0f, 110.0f, 0.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 30.0f, 30.0f, ICON_ARROW_LEFT, 16.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 45.0f, 22.0f, "Settings", 1.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 300.0f, 22.0f, "Edit", 1.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 25.0f, 65.0f, "General", 1.7f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 0.0f, 109.0f, 360.0f, 109.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "navigation_bar.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 13. tab_bar.png */
  rc = canvas_create(360, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 0.0f, 0.0f, 360.0f, 80.0f, 0.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 0.0f, 0.0f, 360.0f, 0.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 45.0f, 26.0f, ICON_HOME, 20.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 45.0f, 48.0f, "Today", 0.75f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 135.0f, 26.0f, ICON_STAR, 20.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 135.0f, 48.0f, "Games", 0.75f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 225.0f, 26.0f, ICON_FOLDER, 20.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 225.0f, 48.0f, "Apps", 0.75f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 315.0f, 26.0f, ICON_SEARCH, 20.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 315.0f, 48.0f, "Search", 0.75f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "tab_bar.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 14. grouped_list.png */
  rc = canvas_create(340, 200, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 30.0f, 15.0f, "ACCOUNT", 0.75f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 20.0f, 30.0f, 300.0f, 140.0f, 12.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 32.0f, 42.0f, 28.0f, 28.0f, 6.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 46.0f, 56.0f, ICON_PERSON, 16.0f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 70.0f, 48.0f, "Profile Information", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 295.0f, 56.0f, ICON_ARROW_RIGHT, 12.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 70.0f, 76.0f, 320.0f, 76.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 32.0f, 86.0f, 28.0f, 28.0f, 6.0f, p.system_green);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 46.0f, 100.0f, ICON_SETTINGS, 16.0f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 70.0f, 92.0f, "Security & Privacy", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 295.0f, 100.0f, ICON_ARROW_RIGHT, 12.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 70.0f, 120.0f, 320.0f, 120.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 32.0f, 130.0f, 28.0f, 28.0f, 6.0f, p.system_orange);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 46.0f, 144.0f, ICON_BELL, 16.0f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 70.0f, 136.0f, "Notifications", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 295.0f, 144.0f, ICON_ARROW_RIGHT, 12.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "grouped_list.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 15. search_bar.png */
  rc = canvas_create(340, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_rounded_rect(c, 20.0f, 22.0f, 240.0f, 36.0f, 10.0f, p.fill_tertiary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 38.0f, 40.0f, ICON_SEARCH, 16.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 56.0f, 32.0f, "Search apps, files", 0.9f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 270.0f, 32.0f, "Cancel", 0.95f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "search_bar.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* 16. cupertino_catalog_overview.png */
  rc = canvas_create(960, 640, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 40.0f, 30.0f, "Cupertino Design System", 1.8f,
                 p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 40.0f, 65.0f,
                 "Apple Human Interface Guidelines (HIG) Component Suite", 1.0f,
                 p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 40.0f, 90.0f, 920.0f, 90.0f, 1.0f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_text(c, 40.0f, 110.0f, "Controls & Buttons", 1.1f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 40.0f, 135.0f, 110.0f, 40.0f, 12.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 95.0f, 147.0f, "Filled", 1.0f,
                          UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 165.0f, 135.0f, 110.0f, 40.0f, 12.0f,
                         UI_COLOR_ARGB(40, 0, 122, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 220.0f, 147.0f, "Tinted", 1.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 290.0f, 135.0f, 110.0f, 40.0f, 12.0f, 1.5f,
                                p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 345.0f, 147.0f, "Bordered", 1.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_rounded_rect(c, 420.0f, 137.0f, 210.0f, 36.0f, 9.0f,
                         p.fill_tertiary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 422.0f, 139.0f, 68.0f, 32.0f, 7.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 456.0f, 147.0f, "Day", 0.9f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 525.0f, 147.0f, "Week", 0.9f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 595.0f, 147.0f, "Month", 0.9f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc =
      draw_rounded_rect(c, 660.0f, 140.0f, 51.0f, 31.0f, 15.5f, p.system_green);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle(c, 695.5f, 155.5f, 13.5f, UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc =
      draw_text(c, 40.0f, 205.0f, "Modal Alert & Pickers", 1.1f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 40.0f, 230.0f, 260.0f, 150.0f, 14.0f, 2);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 40.0f, 230.0f, 260.0f, 150.0f, 14.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 170.0f, 250.0f, "Allow Notifications?", 1.1f,
                          p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 170.0f, 275.0f, "Stay updated with latest alerts.",
                          0.8f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 40.0f, 315.0f, 300.0f, 315.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 170.0f, 315.0f, 170.0f, 380.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 105.0f, 338.0f, "Don't Allow", 0.95f,
                          p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 235.0f, 338.0f, "Allow", 0.95f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_text(c, 340.0f, 205.0f, "Inset Grouped List", 1.1f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 340.0f, 230.0f, 290.0f, 150.0f, 12.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 360.0f, 250.0f, "Wi-Fi", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 560.0f, 250.0f, "Office", 0.9f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 360.0f, 278.0f, 630.0f, 278.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 360.0f, 295.0f, "Bluetooth", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 580.0f, 295.0f, "On", 0.9f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 360.0f, 323.0f, 630.0f, 323.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 360.0f, 340.0f, "Cellular", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_text(c, 660.0f, 205.0f, "Action Sheet", 1.1f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 660.0f, 230.0f, 260.0f, 150.0f, 14.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 790.0f, 245.0f, "Select Action", 0.85f,
                          p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 660.0f, 270.0f, 920.0f, 270.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 790.0f, 288.0f, "Open in New Tab", 1.0f,
                          p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 660.0f, 318.0f, 920.0f, 318.0f, 0.5f, p.separator);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 790.0f, 338.0f, "Remove", 1.0f, p.system_red);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_rounded_rect(c, 40.0f, 520.0f, 880.0f, 75.0f, 16.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 150.0f, 545.0f, ICON_HOME, 20.0f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 150.0f, 568.0f, "Home", 0.8f, p.system_blue);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 370.0f, 545.0f, ICON_SEARCH, 20.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 370.0f, 568.0f, "Search", 0.8f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 590.0f, 545.0f, ICON_BELL, 20.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 590.0f, 568.0f, "Notifications", 0.8f,
                          p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 810.0f, 545.0f, ICON_SETTINGS, 20.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_text_centered(c, 810.0f, 568.0f, "Settings", 0.8f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = make_path(path, sizeof(path), opts->output_dir,
                 "cupertino_catalog_overview.png");
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = save_canvas_png(c, path);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = canvas_destroy(c);
  return rc;
}

const struct design_system_driver g_driver_cupertino = {
    "cupertino", "Cupertino (Apple iOS / macOS HIG)",
    "Native Apple visual fidelity, squircle curves, segmented controls, and "
    "inset tables",
    cupertino_render_all};
