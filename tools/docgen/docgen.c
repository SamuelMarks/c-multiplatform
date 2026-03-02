
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_text.h"
#include "m3/m3_app_bar.h"
#include "m3/m3_button.h"
#include "m3/m3_card.h"
#include "m3/m3_chip.h"
#include "m3/m3_color.h"
#include "m3/m3_dialogs.h"
#include "m3/m3_menu.h"
#include "m3/m3_navigation.h"
#include "m3/m3_progress.h"
#include "m3/m3_scaffold.h"
#include "m3/m3_selection.h"
#include "m3/m3_sheet.h"
#include "m3/m3_tabs.h"

static const char *g_platform = "linux";
static const char *g_theme = "material";
static FILE *g_json_file = NULL;
static int g_first_widget = 1;

typedef struct SvgContext {
  FILE *file;
  cmp_i32 width;
  cmp_i32 height;
} SvgContext;

static int CMP_CALL svg_begin_frame(void *gfx, CMPHandle window, cmp_i32 width,
                                    cmp_i32 height, CMPScalar dpi_scale) {
  SvgContext *ctx = (SvgContext *)gfx;
  CMP_UNUSED(window);
  CMP_UNUSED(dpi_scale);
  ctx->width = width;
  ctx->height = height;
  fprintf(ctx->file,
          "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" "
          "height=\"%d\" viewBox=\"0 0 %d %d\">\n",
          width, height, width, height);
  return CMP_OK;
}

static int CMP_CALL svg_end_frame(void *gfx, CMPHandle window) {
  SvgContext *ctx = (SvgContext *)gfx;
  CMP_UNUSED(window);
  fprintf(ctx->file, "</svg>\n");
  return CMP_OK;
}

static int CMP_CALL svg_clear(void *gfx, CMPColor color) {
  CMP_UNUSED(gfx);
  CMP_UNUSED(color);
  return CMP_OK;
}

static void print_color(FILE *f, CMPColor c) {
  int r = (int)(c.r * 255.0f);
  int g = (int)(c.g * 255.0f);
  int b = (int)(c.b * 255.0f);
  fprintf(f, "rgb(%d,%d,%d)", r, g, b);
}

static float get_alpha(CMPColor c) { return c.a; }

static int CMP_CALL svg_draw_rect(void *gfx, const CMPRect *rect,
                                  CMPColor color, CMPScalar corner_radius) {
  SvgContext *ctx = (SvgContext *)gfx;
  fprintf(ctx->file,
          "  <rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" rx=\"%f\" "
          "ry=\"%f\" fill=\"",
          rect->x, rect->y, rect->width, rect->height, corner_radius,
          corner_radius);
  print_color(ctx->file, color);
  fprintf(ctx->file, "\" fill-opacity=\"%f\" />\n", get_alpha(color));
  return CMP_OK;
}

static int CMP_CALL svg_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                                  CMPScalar x1, CMPScalar y1, CMPColor color,
                                  CMPScalar thickness) {
  SvgContext *ctx = (SvgContext *)gfx;
  fprintf(ctx->file,
          "  <line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" stroke=\"", x0, y0,
          x1, y1);
  print_color(ctx->file, color);
  fprintf(ctx->file, "\" stroke-width=\"%f\" stroke-opacity=\"%f\" />\n",
          thickness, get_alpha(color));
  return CMP_OK;
}

static int CMP_CALL svg_draw_path(void *gfx, const CMPPath *path,
                                  CMPColor color) {
  SvgContext *ctx = (SvgContext *)gfx;
  cmp_usize i;
  fprintf(ctx->file, "  <path d=\"");
  for (i = 0; i < path->count; ++i) {
    CMPPathCmd *cmd = &path->commands[i];
    if (cmd->type == CMP_PATH_CMD_MOVE_TO) {
      fprintf(ctx->file, "M %f %f ", cmd->data.move_to.x, cmd->data.move_to.y);
    } else if (cmd->type == CMP_PATH_CMD_LINE_TO) {
      fprintf(ctx->file, "L %f %f ", cmd->data.line_to.x, cmd->data.line_to.y);
    } else if (cmd->type == CMP_PATH_CMD_QUAD_TO) {
      fprintf(ctx->file, "Q %f %f %f %f ", cmd->data.quad_to.cx,
              cmd->data.quad_to.cy, cmd->data.quad_to.x, cmd->data.quad_to.y);
    } else if (cmd->type == CMP_PATH_CMD_CUBIC_TO) {
      fprintf(ctx->file, "C %f %f %f %f %f %f ", cmd->data.cubic_to.cx1,
              cmd->data.cubic_to.cy1, cmd->data.cubic_to.cx2,
              cmd->data.cubic_to.cy2, cmd->data.cubic_to.x,
              cmd->data.cubic_to.y);
    } else if (cmd->type == CMP_PATH_CMD_CLOSE) {
      fprintf(ctx->file, "Z ");
    }
  }
  fprintf(ctx->file, "\" fill=\"");
  print_color(ctx->file, color);
  fprintf(ctx->file, "\" fill-opacity=\"%f\" />\n", get_alpha(color));
  return CMP_OK;
}

static int CMP_CALL svg_push_clip(void *gfx, const CMPRect *rect) {
  CMP_UNUSED(gfx);
  CMP_UNUSED(rect);
  return CMP_OK;
}

static int CMP_CALL svg_pop_clip(void *gfx) {
  CMP_UNUSED(gfx);
  return CMP_OK;
}

static int CMP_CALL svg_set_transform(void *gfx, const CMPMat3 *transform) {
  CMP_UNUSED(gfx);
  CMP_UNUSED(transform);
  return CMP_OK;
}

static int CMP_CALL svg_create_texture(void *gfx, cmp_i32 width, cmp_i32 height,
                                       cmp_u32 format, const void *pixels,
                                       cmp_usize size, CMPHandle *out_texture) {
  CMP_UNUSED(gfx);
  CMP_UNUSED(width);
  CMP_UNUSED(height);
  CMP_UNUSED(format);
  CMP_UNUSED(pixels);
  CMP_UNUSED(size);
  if (out_texture) {
    out_texture->id = 0;
    out_texture->generation = 0;
  }
  return CMP_OK;
}

static int CMP_CALL svg_update_texture(void *gfx, CMPHandle texture, cmp_i32 x,
                                       cmp_i32 y, cmp_i32 width, cmp_i32 height,
                                       const void *pixels, cmp_usize size) {
  CMP_UNUSED(gfx);
  CMP_UNUSED(texture);
  CMP_UNUSED(x);
  CMP_UNUSED(y);
  CMP_UNUSED(width);
  CMP_UNUSED(height);
  CMP_UNUSED(pixels);
  CMP_UNUSED(size);
  return CMP_OK;
}

static int CMP_CALL svg_destroy_texture(void *gfx, CMPHandle texture) {
  CMP_UNUSED(gfx);
  CMP_UNUSED(texture);
  return CMP_OK;
}

static int CMP_CALL svg_draw_texture(void *gfx, CMPHandle texture,
                                     const CMPRect *src, const CMPRect *dst,
                                     CMPScalar opacity) {
  CMP_UNUSED(gfx);
  CMP_UNUSED(texture);
  CMP_UNUSED(src);
  CMP_UNUSED(dst);
  CMP_UNUSED(opacity);
  return CMP_OK;
}

static const CMPGfxVTable svg_gfx_vtable = {
    svg_begin_frame,    svg_end_frame,      svg_clear,
    svg_draw_rect,      svg_draw_line,      svg_draw_path,
    svg_push_clip,      svg_pop_clip,       svg_set_transform,
    svg_create_texture, svg_update_texture, svg_destroy_texture,
    svg_draw_texture};

static int CMP_CALL doc_create_font(void *text, const char *utf8_family,
                                    cmp_i32 size_px, cmp_i32 weight,
                                    CMPBool italic, CMPHandle *out_font) {
  CMP_UNUSED(text);
  CMP_UNUSED(utf8_family);
  CMP_UNUSED(size_px);
  CMP_UNUSED(weight);
  CMP_UNUSED(italic);
  if (out_font) {
    out_font->id = 1;
    out_font->generation = 1;
  }
  return CMP_OK;
}

static int CMP_CALL doc_destroy_font(void *text, CMPHandle font) {
  CMP_UNUSED(text);
  CMP_UNUSED(font);
  return CMP_OK;
}

static int CMP_CALL doc_measure_text(void *text, CMPHandle font,
                                     const char *utf8, cmp_usize utf8_len,
                                     CMPScalar *out_width,
                                     CMPScalar *out_height,
                                     CMPScalar *out_baseline) {
  CMP_UNUSED(text);
  CMP_UNUSED(font);
  CMP_UNUSED(utf8);
  if (out_width)
    *out_width = (CMPScalar)utf8_len * 8.0f;
  if (out_height)
    *out_height = 14.0f;
  if (out_baseline)
    *out_baseline = 10.0f;
  return CMP_OK;
}

static int CMP_CALL doc_draw_text(void *text, CMPHandle font, const char *utf8,
                                  cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                                  CMPColor color) {
  SvgContext *ctx = (SvgContext *)text;
  CMP_UNUSED(font);
  fprintf(ctx->file,
          "  <text x=\"%f\" y=\"%f\" font-family=\"sans-serif\" "
          "font-size=\"14px\" fill=\"",
          x, y + 10.0f);
  print_color(ctx->file, color);
  fprintf(ctx->file, "\" fill-opacity=\"%f\">", get_alpha(color));
  fwrite(utf8, 1, utf8_len, ctx->file);
  fprintf(ctx->file, "</text>\n");
  return CMP_OK;
}

static const CMPTextVTable doc_text_vtable = {doc_create_font, doc_destroy_font,
                                              doc_measure_text, doc_draw_text};

static void write_json_entry(const char *category, const char *name,
                             const char *file) {
  if (!g_first_widget) {
    fprintf(g_json_file, ",\n");
  }
  fprintf(g_json_file,
          "    { \"category\": \"%s\", \"name\": \"%s\", \"file\": \"%s\" }",
          category, name, file);
  g_first_widget = 0;
}

static void generate_widget(const char *category, const char *name,
                            CMPWidget *widget) {
  char svg_filename[512];
  char relative_filename[256];
  SvgContext svg_ctx;
  CMPGfx gfx;
  CMPMeasureSpec spec_w;
  CMPMeasureSpec spec_h;
  CMPSize size;
  cmp_i32 width, height;
  CMPRect bounds;
  CMPPaintContext paint_ctx;
  CMPHandle null_h;

  null_h.id = 0;
  null_h.generation = 0;

  sprintf(relative_filename, "assets/%s_%s_%s_%s.svg", category, name,
          g_platform, g_theme);
  sprintf(svg_filename, "docs/%s", relative_filename);
  svg_ctx.file = fopen(svg_filename, "wb");
  if (!svg_ctx.file) {
    printf("Failed to open %s\n", svg_filename);
    return;
  }

  gfx.ctx = &svg_ctx;
  gfx.vtable = &svg_gfx_vtable;
  gfx.text_vtable = &doc_text_vtable;

  spec_w.mode = CMP_MEASURE_UNSPECIFIED;
  spec_w.size = 0;
  spec_h.mode = CMP_MEASURE_UNSPECIFIED;
  spec_h.size = 0;
  widget->vtable->measure(widget, spec_w, spec_h, &size);

  width = (cmp_i32)size.width + 16;
  height = (cmp_i32)size.height + 16;

  bounds.x = 8.0f;
  bounds.y = 8.0f;
  bounds.width = size.width;
  bounds.height = size.height;
  widget->vtable->layout(widget, bounds);

  svg_begin_frame(gfx.ctx, null_h, width, height, 1.0f);
  paint_ctx.gfx = &gfx;
  paint_ctx.clip = bounds;
  paint_ctx.dpi_scale = 1.0f;
  widget->vtable->paint(widget, &paint_ctx);
  svg_end_frame(gfx.ctx, null_h);

  fclose(svg_ctx.file);
  write_json_entry(category, name, relative_filename);
}

int main(int argc, char **argv) {
  int i;
  char json_filename[128];
  CMPTextBackend text_backend;
  CMPGfx dummy_gfx;

  M3ButtonStyle button_style;
  M3Button button;

  M3ChipStyle chip_style;
  M3Chip chip;

  M3CheckboxStyle chk_style;
  M3Checkbox chk;

  M3RadioStyle rad_style;
  M3Radio rad;

  M3SwitchStyle sw_style;
  M3Switch sw;

  M3CardStyle card_style;
  M3Card card;

  M3LinearProgressStyle lin_prog_style;
  M3LinearProgress lin_prog;

  M3CircularProgressStyle circ_prog_style;
  M3CircularProgress circ_prog;

  M3TabRowStyle tab_style;
  M3TabRow tab;
  M3TabItem tab_items[3];

  M3AppBarStyle app_bar_style;
  M3AppBar app_bar;

  M3AlertDialogStyle alert_style;
  M3AlertDialog alert;
  M3FullScreenDialogStyle fullscreen_style;
  M3FullScreenDialog fullscreen;
  M3SnackbarStyle snackbar_style;
  M3Snackbar snackbar_widget;
  M3SheetStyle sheet_style;
  M3Sheet sheet;
  M3MenuStyle menu_style;
  M3Menu menu;
  M3MenuItem menu_items[3];
  M3NavigationStyle nav_style;
  M3Navigation nav;
  M3NavigationItem nav_items[3];
  M3ScaffoldStyle scaffold_style;
  M3Scaffold scaffold;

  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--platform") == 0 && i + 1 < argc) {
      g_platform = argv[++i];
    } else if (strcmp(argv[i], "--theme") == 0 && i + 1 < argc) {
      g_theme = argv[++i];
    }
  }

  dummy_gfx.ctx = NULL;
  dummy_gfx.vtable = &svg_gfx_vtable;
  dummy_gfx.text_vtable = &doc_text_vtable;
  cmp_text_backend_from_gfx(&dummy_gfx, &text_backend);

#if defined(_WIN32)
  system("mkdir docs\\assets >nul 2>&1");
#else
  system("mkdir -p docs/assets");
#endif

  sprintf(json_filename, "docs/doc_%s_%s.json", g_platform, g_theme);
  g_json_file = fopen(json_filename, "w");
  if (!g_json_file) {
    printf("Failed to open json %s\n", json_filename);
    return 1;
  }

  fprintf(g_json_file, "{\n");
  fprintf(g_json_file, "  \"platform\": \"%s\",\n", g_platform);
  fprintf(g_json_file, "  \"theme\": \"%s\",\n", g_theme);
  fprintf(g_json_file, "  \"widgets\": [\n");

  /* Buttons */
  m3_button_style_init_filled(&button_style);
  m3_button_init(&button, &text_backend, &button_style, "Button", 6);
  generate_widget("button", "filled", &button.widget);

  m3_button_style_init_tonal(&button_style);
  m3_button_init(&button, &text_backend, &button_style, "Button", 6);
  generate_widget("button", "tonal", &button.widget);

  m3_button_style_init_outlined(&button_style);
  m3_button_init(&button, &text_backend, &button_style, "Button", 6);
  generate_widget("button", "outlined", &button.widget);

  m3_button_style_init_text(&button_style);
  m3_button_init(&button, &text_backend, &button_style, "Button", 6);
  generate_widget("button", "text", &button.widget);

  m3_button_style_init_elevated(&button_style);
  m3_button_init(&button, &text_backend, &button_style, "Button", 6);
  generate_widget("button", "elevated", &button.widget);

  m3_button_style_init_fab(&button_style);
  m3_button_init(&button, &text_backend, &button_style, "FAB", 3);
  generate_widget("button", "fab", &button.widget);

  /* Chips */
  m3_chip_style_init_assist(&chip_style);
  m3_chip_init(&chip, &text_backend, &chip_style, "Chip", 4);
  generate_widget("chip", "assist", &chip.widget);

  m3_chip_style_init_filter(&chip_style);
  m3_chip_init(&chip, &text_backend, &chip_style, "Chip", 4);
  generate_widget("chip", "filter", &chip.widget);

  m3_chip_style_init_input(&chip_style);
  m3_chip_init(&chip, &text_backend, &chip_style, "Chip", 4);
  generate_widget("chip", "input", &chip.widget);

  m3_chip_style_init_suggestion(&chip_style);
  m3_chip_init(&chip, &text_backend, &chip_style, "Chip", 4);
  generate_widget("chip", "suggestion", &chip.widget);

  /* Checkboxes */
  m3_checkbox_style_init(&chk_style);
  m3_checkbox_init(&chk, &chk_style, 0);
  generate_widget("checkbox", "unchecked", &chk.widget);

  m3_checkbox_init(&chk, &chk_style, 1);
  generate_widget("checkbox", "checked", &chk.widget);

  /* Radios */
  m3_radio_style_init(&rad_style);
  m3_radio_init(&rad, &rad_style, 0);
  generate_widget("radio", "unselected", &rad.widget);

  m3_radio_init(&rad, &rad_style, 1);
  generate_widget("radio", "selected", &rad.widget);

  /* Switches */
  m3_switch_style_init(&sw_style);
  m3_switch_init(&sw, &sw_style, 0);
  generate_widget("switch", "off", &sw.widget);

  m3_switch_init(&sw, &sw_style, 1);
  generate_widget("switch", "on", &sw.widget);

  /* Cards */
  m3_card_style_init_elevated(&card_style);
  card_style.min_width = 120.0f;
  card_style.min_height = 80.0f;
  m3_card_init(&card, &card_style);
  generate_widget("card", "elevated", &card.widget);

  m3_card_style_init_filled(&card_style);
  card_style.min_width = 120.0f;
  card_style.min_height = 80.0f;
  m3_card_init(&card, &card_style);
  generate_widget("card", "filled", &card.widget);

  m3_card_style_init_outlined(&card_style);
  card_style.min_width = 120.0f;
  card_style.min_height = 80.0f;
  m3_card_init(&card, &card_style);
  generate_widget("card", "outlined", &card.widget);

  /* Progress Indicators */
  m3_linear_progress_style_init(&lin_prog_style);
  m3_linear_progress_init(&lin_prog, &lin_prog_style, 0.45f);
  generate_widget("linear_progress", "determinate", &lin_prog.widget);

  m3_circular_progress_style_init(&circ_prog_style);
  m3_circular_progress_init(&circ_prog, &circ_prog_style, 0.70f);
  generate_widget("circular_progress", "determinate", &circ_prog.widget);

  /* Tabs */
  tab_items[0].utf8_label = "Home";
  tab_items[0].utf8_len = 4;
  tab_items[1].utf8_label = "Settings";
  tab_items[1].utf8_len = 8;
  tab_items[2].utf8_label = "Profile";
  tab_items[2].utf8_len = 7;

  m3_tab_row_style_init(&tab_style);
  m3_tab_row_init(&tab, &text_backend, &tab_style, tab_items, 3, 0);
  generate_widget("tabs", "primary", &tab.widget);

  /* App Bars */
  m3_app_bar_style_init_small(&app_bar_style);
  m3_app_bar_init(&app_bar, &text_backend, &app_bar_style, "Title", 5);
  generate_widget("app_bar", "small", &app_bar.widget);

  m3_app_bar_style_init_center(&app_bar_style);
  m3_app_bar_init(&app_bar, &text_backend, &app_bar_style, "Center", 6);
  generate_widget("app_bar", "center", &app_bar.widget);

  m3_app_bar_style_init_medium(&app_bar_style);
  m3_app_bar_init(&app_bar, &text_backend, &app_bar_style, "Medium", 6);
  generate_widget("app_bar", "medium", &app_bar.widget);

  m3_app_bar_style_init_large(&app_bar_style);
  m3_app_bar_init(&app_bar, &text_backend, &app_bar_style, "Large", 5);
  generate_widget("app_bar", "large", &app_bar.widget);

  /* Dialogs */
  m3_alert_dialog_style_init(&alert_style);
  m3_alert_dialog_init(&alert, &text_backend, &alert_style, "Alert Title", 11,
                       "This is the body of the alert dialog.", 37);
  generate_widget("dialog", "alert", &alert.widget);

  m3_fullscreen_dialog_style_init(&fullscreen_style);
  m3_fullscreen_dialog_init(&fullscreen, &text_backend, &fullscreen_style,
                            "Full Screen", 11,
                            "This dialog takes up the full screen.", 37);
  generate_widget("dialog", "fullscreen", &fullscreen.widget);

  /* Snackbar */
  m3_snackbar_style_init(&snackbar_style);
  m3_snackbar_init(&snackbar_widget, &text_backend, &snackbar_style,
                   "Message sent.", 13);
  generate_widget("snackbar", "default", &snackbar_widget.widget);

  /* Sheet */
  m3_sheet_style_init_standard(&sheet_style);
  m3_sheet_init(&sheet, &sheet_style);
  sheet.sheet_bounds.width = 300;
  sheet.sheet_bounds.height = 400;
  generate_widget("sheet", "standard", &sheet.widget);

  m3_sheet_style_init_modal(&sheet_style);
  m3_sheet_init(&sheet, &sheet_style);
  sheet.sheet_bounds.width = 300;
  sheet.sheet_bounds.height = 400;
  generate_widget("sheet", "modal", &sheet.widget);

  /* Menu */
  menu_items[0].utf8_label = "Copy";
  menu_items[0].utf8_len = 4;
  menu_items[0].enabled = CMP_TRUE;
  menu_items[1].utf8_label = "Paste";
  menu_items[1].utf8_len = 5;
  menu_items[1].enabled = CMP_TRUE;
  menu_items[2].utf8_label = "Delete";
  menu_items[2].utf8_len = 6;
  menu_items[2].enabled = CMP_FALSE;

  m3_menu_style_init(&menu_style);
  m3_menu_init(&menu, &text_backend, &menu_style, menu_items, 3);
  generate_widget("menu", "default", &menu.widget);

  /* Navigation */
  nav_items[0].utf8_label = "Home";
  nav_items[0].utf8_len = 4;
  nav_items[1].utf8_label = "Inbox";
  nav_items[1].utf8_len = 5;
  nav_items[2].utf8_label = "Settings";
  nav_items[2].utf8_len = 8;

  m3_navigation_style_init(&nav_style);
  m3_navigation_init(&nav, &text_backend, &nav_style, nav_items, 3, 0);
  generate_widget("navigation", "default", &nav.widget);

  /* Scaffold */
  m3_scaffold_style_init(&scaffold_style);
  m3_app_bar_style_init_small(&app_bar_style);
  m3_app_bar_init(&app_bar, &text_backend, &app_bar_style, "App Title", 9);
  m3_button_style_init_fab(&button_style);
  m3_button_init(&button, &text_backend, &button_style, "Add", 3);
  m3_scaffold_init(&scaffold, &scaffold_style, NULL, &app_bar.widget, NULL,
                   &button.widget, NULL);
  generate_widget("scaffold", "default", &scaffold.widget);

  fprintf(g_json_file, "\n  ]\n}\n");
  fclose(g_json_file);

  printf("Generated JSON manifest in %s\n", json_filename);
  return 0;
}
