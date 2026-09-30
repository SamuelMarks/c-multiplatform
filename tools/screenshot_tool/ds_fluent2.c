/**
 * @file ds_fluent2.c
 * @brief Microsoft Fluent 2 design system screenshot generator.
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

struct fluent2_palette {
  ui_color_t brand;
  ui_color_t brand_hover;
  ui_color_t background;
  ui_color_t surface;
  ui_color_t stroke;
  ui_color_t text_primary;
  ui_color_t text_secondary;
  ui_color_t neutral_subtle;
  ui_color_t status_success;
  ui_color_t status_error;
};

static ui_error_t init_fluent2_palette(int is_dark, struct fluent2_palette *p) {
  if (!p) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (is_dark) {
    p->brand = UI_COLOR_ARGB(255, 40, 134, 222);
    p->brand_hover = UI_COLOR_ARGB(255, 62, 149, 230);
    p->background = UI_COLOR_ARGB(255, 32, 31, 30);
    p->surface = UI_COLOR_ARGB(255, 41, 40, 39);
    p->stroke = UI_COLOR_ARGB(255, 72, 70, 68);
    p->text_primary = UI_COLOR_ARGB(255, 255, 255, 255);
    p->text_secondary = UI_COLOR_ARGB(255, 210, 208, 206);
    p->neutral_subtle = UI_COLOR_ARGB(255, 49, 48, 46);
    p->status_success = UI_COLOR_ARGB(255, 84, 176, 52);
    p->status_error = UI_COLOR_ARGB(255, 232, 17, 35);
  } else {
    p->brand = UI_COLOR_ARGB(255, 15, 108, 189);
    p->brand_hover = UI_COLOR_ARGB(255, 17, 94, 163);
    p->background = UI_COLOR_ARGB(255, 250, 249, 248);
    p->surface = UI_COLOR_ARGB(255, 255, 255, 255);
    p->stroke = UI_COLOR_ARGB(255, 209, 209, 209);
    p->text_primary = UI_COLOR_ARGB(255, 36, 36, 36);
    p->text_secondary = UI_COLOR_ARGB(255, 97, 97, 97);
    p->neutral_subtle = UI_COLOR_ARGB(255, 245, 245, 245);
    p->status_success = UI_COLOR_ARGB(255, 16, 124, 65);
    p->status_error = UI_COLOR_ARGB(255, 197, 15, 31);
  }
  return UI_ERROR_NONE;
}

static ui_error_t fluent2_render_all(const struct design_system_options *opts) {
  char path[512];
  struct canvas *c = NULL;
  struct fluent2_palette p;
  ui_error_t rc;
  int i;

  if (!opts || !opts->output_dir) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = init_fluent2_palette(opts->is_dark, &p);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* 1. button_primary.png */
  rc = canvas_create(220, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 25.0f, 22.0f, 170.0f, 36.0f, 4.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 110.0f, 32.0f, "Primary", 1.0f,
                          UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_primary.png");
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

  /* 2. button_standard.png */
  rc = canvas_create(220, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 25.0f, 22.0f, 170.0f, 36.0f, 4.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 25.0f, 22.0f, 170.0f, 36.0f, 4.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 110.0f, 32.0f, "Standard", 1.0f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_standard.png");
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

  /* 3. button_subtle.png */
  rc = canvas_create(200, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 100.0f, 32.0f, "Subtle Action", 1.0f,
                          p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_subtle.png");
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

  /* 4. button_split.png */
  rc = canvas_create(240, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 25.0f, 22.0f, 190.0f, 36.0f, 4.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 45.0f, 32.0f, "New Item", 1.0f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 175.0f, 26.0f, 175.0f, 54.0f, 1.0f,
                 UI_COLOR_ARGB(120, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 193.0f, 40.0f, ICON_ARROW_DOWN, 14.0f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_split.png");
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

  /* 5. button_toggle.png */
  rc = canvas_create(220, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_rounded_rect(c, 25.0f, 22.0f, 170.0f, 36.0f, 4.0f, p.neutral_subtle);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 25.0f, 22.0f, 170.0f, 36.0f, 4.0f, 1.0f,
                                p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 110.0f, 32.0f, "Toggle: ON", 1.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "button_toggle.png");
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

  /* 6. checkbox.png */
  rc = canvas_create(260, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 28.0f, 20.0f, 20.0f, 3.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 40.0f, 38.0f, ICON_CHECK, 12.0f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 62.0f, 31.0f, "Checked box", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "checkbox.png");
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

  /* 7. switch.png */
  rc = canvas_create(280, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 26.0f, 44.0f, 22.0f, 11.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle(c, 62.0f, 37.0f, 8.0f, UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 88.0f, 30.0f, "Toggle switch", 0.95f, p.text_primary);
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

  /* 8. slider.png */
  rc = canvas_create(320, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 38.0f, 260.0f, 4.0f, 2.0f, p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 30.0f, 38.0f, 150.0f, 4.0f, 2.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle(c, 180.0f, 40.0f, 9.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle_stroke(c, 180.0f, 40.0f, 9.0f, 2.0f, p.brand);
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

  /* 9. card_acrylic.png */
  rc = canvas_create(320, 180, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 20.0f, 20.0f, 280.0f, 140.0f, 8.0f, 2);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 20.0f, 20.0f, 280.0f, 140.0f, 8.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 20.0f, 20.0f, 280.0f, 140.0f, 8.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 35.0f, 38.0f, "Fluent 2 Surface Card", 1.15f,
                 p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 35.0f, 62.0f, "Acrylic translucent material layer.", 0.85f,
                 p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 180.0f, 110.0f, 100.0f, 32.0f, 4.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 230.0f, 118.0f, "Explore", 0.95f,
                          UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "card_acrylic.png");
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

  /* 10. flyout_dialog.png */
  rc = canvas_create(320, 180, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 20.0f, 15.0f, 280.0f, 150.0f, 8.0f, 3);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 20.0f, 15.0f, 280.0f, 150.0f, 8.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 20.0f, 15.0f, 280.0f, 150.0f, 8.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 35.0f, 32.0f, "Save Changes?", 1.2f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 35.0f, 58.0f, "Do you want to save modifications", 0.85f,
                 p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 35.0f, 75.0f, "to your workspace project?", 0.85f,
                 p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 90.0f, 115.0f, 80.0f, 32.0f, 4.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 90.0f, 115.0f, 80.0f, 32.0f, 4.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 130.0f, 123.0f, "Don't Save", 0.85f,
                          p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 180.0f, 115.0f, 100.0f, 32.0f, 4.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 230.0f, 123.0f, "Save", 0.95f,
                          UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "flyout_dialog.png");
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

  /* 11. info_bar.png */
  rc = canvas_create(360, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 15.0f, 15.0f, 330.0f, 50.0f, 4.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 15.0f, 15.0f, 330.0f, 50.0f, 4.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 15.0f, 15.0f, 4.0f, 50.0f, 2.0f, p.status_success);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 35.0f, 40.0f, ICON_CHECK, 16.0f, p.status_success);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 55.0f, 27.0f, "Update Available", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 55.0f, 45.0f, "A newer package build was found.", 0.8f,
                 p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 325.0f, 40.0f, ICON_CLOSE, 14.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "info_bar.png");
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

  /* 12. datagrid.png */
  rc = canvas_create(400, 180, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 15.0f, 15.0f, 370.0f, 150.0f, 4.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 15.0f, 15.0f, 370.0f, 150.0f, 4.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 30.0f, 26.0f, "Name", 0.9f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 80.0f, 33.0f, ICON_ARROW_DOWN, 10.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 150.0f, 26.0f, "Modified", 0.9f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 290.0f, 26.0f, "Size", 0.9f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 15.0f, 48.0f, 385.0f, 48.0f, 1.0f, p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 35.0f, 65.0f, ICON_FOLDER, 14.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 55.0f, 58.0f, "Documents", 0.9f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 150.0f, 58.0f, "Sep 28, 2026", 0.85f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 290.0f, 58.0f, "24 MB", 0.85f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 15.0f, 85.0f, 385.0f, 85.0f, 1.0f, p.neutral_subtle);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_rounded_rect(c, 16.0f, 86.0f, 368.0f, 36.0f, 0.0f, p.neutral_subtle);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 16.0f, 94.0f, 3.0f, 20.0f, 1.5f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 35.0f, 104.0f, ICON_EDIT, 14.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 55.0f, 97.0f, "report.docx", 0.9f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 150.0f, 97.0f, "Sep 29, 2026", 0.85f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 290.0f, 97.0f, "1.2 MB", 0.85f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 15.0f, 124.0f, 385.0f, 124.0f, 1.0f, p.neutral_subtle);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 35.0f, 143.0f, ICON_STAR, 14.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 55.0f, 136.0f, "notes.txt", 0.9f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 150.0f, 136.0f, "Sep 30, 2026", 0.85f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 290.0f, 136.0f, "15 KB", 0.85f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "datagrid.png");
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

  /* 13. tab_list.png */
  rc = canvas_create(340, 80, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 60.0f, 30.0f, "Home", 1.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 35.0f, 54.0f, 50.0f, 3.0f, 1.5f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 160.0f, 30.0f, "Files", 1.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 260.0f, 30.0f, "Activity", 1.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 20.0f, 57.0f, 320.0f, 57.0f, 1.0f, p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "tab_list.png");
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

  /* 14. progress_ring.png */
  rc = canvas_create(140, 140, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle_stroke(c, 70.0f, 70.0f, 36.0f, 4.0f, p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  for (i = 0; i < 16; ++i) {
    float a = -1.5707963f + (float)i * 0.28f;
    float px = 70.0f + cosf(a) * 36.0f;
    float py = 70.0f + sinf(a) * 36.0f;
    rc = draw_circle(c, px, py, 2.5f, p.brand);
    if (rc != UI_ERROR_NONE) {
      canvas_destroy(c);
      return rc;
    }
  }
  rc = make_path(path, sizeof(path), opts->output_dir, "progress_ring.png");
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

  /* 15. fluent2_catalog_overview.png */
  rc = canvas_create(960, 640, &c);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = canvas_clear(c, p.background);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 40.0f, 30.0f, "Microsoft Fluent 2 Design System", 1.8f,
                 p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 40.0f, 65.0f,
                 "Cross-Platform Fluent 2 UI Controls & Design Tokens", 1.0f,
                 p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 40.0f, 90.0f, 920.0f, 90.0f, 1.0f, p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_text(c, 40.0f, 110.0f, "Button Hierarchy", 1.1f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 40.0f, 135.0f, 110.0f, 36.0f, 4.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 95.0f, 145.0f, "Primary", 0.95f,
                          UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 160.0f, 135.0f, 110.0f, 36.0f, 4.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 160.0f, 135.0f, 110.0f, 36.0f, 4.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 215.0f, 145.0f, "Standard", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 280.0f, 135.0f, 140.0f, 36.0f, 4.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 295.0f, 145.0f, "Split Button", 0.95f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 390.0f, 139.0f, 390.0f, 167.0f, 1.0f,
                 UI_COLOR_ARGB(120, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 405.0f, 153.0f, ICON_ARROW_DOWN, 12.0f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_rounded_rect(c, 440.0f, 143.0f, 20.0f, 20.0f, 3.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_icon(c, 450.0f, 153.0f, ICON_CHECK, 12.0f,
                 UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 470.0f, 146.0f, "Checkbox", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 560.0f, 142.0f, 44.0f, 22.0f, 11.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_circle(c, 592.0f, 153.0f, 8.0f, UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 615.0f, 146.0f, "Switch", 0.95f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_text(c, 40.0f, 205.0f, "Acrylic Surface Card", 1.1f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 40.0f, 230.0f, 260.0f, 150.0f, 8.0f, 2);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 40.0f, 230.0f, 260.0f, 150.0f, 8.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 40.0f, 230.0f, 260.0f, 150.0f, 8.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_text(c, 55.0f, 250.0f, "Fluent Card Headline", 1.1f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 55.0f, 275.0f, "Translucent depth texture.", 0.85f,
                 p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 160.0f, 330.0f, 120.0f, 32.0f, 4.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 220.0f, 338.0f, "Action CTA", 0.9f,
                          UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_text(c, 340.0f, 205.0f, "Flyout / Dialog", 1.1f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_shadow(c, 340.0f, 230.0f, 280.0f, 150.0f, 8.0f, 3);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 340.0f, 230.0f, 280.0f, 150.0f, 8.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 340.0f, 230.0f, 280.0f, 150.0f, 8.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 360.0f, 250.0f, "Publish Workflow?", 1.1f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 360.0f, 275.0f, "Deploy updates to production.", 0.85f,
                 p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 430.0f, 330.0f, 80.0f, 32.0f, 4.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 430.0f, 330.0f, 80.0f, 32.0f, 4.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 470.0f, 338.0f, "Cancel", 0.85f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 520.0f, 330.0f, 85.0f, 32.0f, 4.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 562.0f, 338.0f, "Publish", 0.85f,
                          UI_COLOR_ARGB(255, 255, 255, 255));
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_text(c, 660.0f, 205.0f, "DataGrid Hierarchy", 1.1f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 660.0f, 230.0f, 260.0f, 150.0f, 4.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 660.0f, 230.0f, 260.0f, 150.0f, 4.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 675.0f, 245.0f, "Resource", 0.85f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 840.0f, 245.0f, "State", 0.85f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 660.0f, 265.0f, 920.0f, 265.0f, 1.0f, p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 675.0f, 280.0f, "AppService", 0.9f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 840.0f, 280.0f, "Running", 0.85f, p.status_success);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_line(c, 660.0f, 305.0f, 920.0f, 305.0f, 1.0f, p.neutral_subtle);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 661.0f, 306.0f, 258.0f, 30.0f, 0.0f,
                         p.neutral_subtle);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 661.0f, 311.0f, 3.0f, 20.0f, 1.5f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 675.0f, 318.0f, "Database", 0.9f, p.text_primary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text(c, 840.0f, 318.0f, "Active", 0.85f, p.status_success);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = draw_rounded_rect(c, 40.0f, 520.0f, 880.0f, 70.0f, 8.0f, p.surface);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect_stroke(c, 40.0f, 520.0f, 880.0f, 70.0f, 8.0f, 1.0f,
                                p.stroke);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 150.0f, 545.0f, "Overview", 1.0f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_rounded_rect(c, 115.0f, 570.0f, 70.0f, 3.0f, 1.5f, p.brand);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 370.0f, 545.0f, "Diagnostics", 1.0f,
                          p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc =
      draw_text_centered(c, 590.0f, 545.0f, "Settings", 1.0f, p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }
  rc = draw_text_centered(c, 810.0f, 545.0f, "Documentation", 1.0f,
                          p.text_secondary);
  if (rc != UI_ERROR_NONE) {
    canvas_destroy(c);
    return rc;
  }

  rc = make_path(path, sizeof(path), opts->output_dir,
                 "fluent2_catalog_overview.png");
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

const struct design_system_driver g_driver_fluent2 = {
    "fluent2", "Fluent 2 (Microsoft)",
    "Microsoft Fluent 2 design language with 4px control radius, acrylic "
    "elevation, and brand palettes",
    fluent2_render_all};
