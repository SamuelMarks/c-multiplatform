#include <cmpc/cmp_backend_gtk4.h>
#include <m3/m3_button.h>
#include <m3/m3_card.h>
#include <m3/m3_chip.h>
#include <m3/m3_progress.h>
#include <m3/m3_selection.h>
#include <m3/m3_tabs.h>
#include <m3/m3_text_field.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: %s <widget_name> <output.png>\n", argv[0]);
    return 1;
  }
  const char *widget_name = argv[1];
  const char *out_path = argv[2];

  CMPGTK4BackendConfig cfg;
  cmp_gtk4_backend_config_init(&cfg);
  CMPGTK4Backend *be = NULL;
  cmp_gtk4_backend_create(&cfg, &be);

  CMPWS ws;
  cmp_gtk4_backend_get_ws(be, &ws);
  CMPGfx gfx;
  cmp_gtk4_backend_get_gfx(be, &gfx);
  CMPEnv env;
  cmp_gtk4_backend_get_env(be, &env);

  CMPWSWindowConfig wcfg;
  memset(&wcfg, 0, sizeof(wcfg));
  wcfg.width = 200;
  wcfg.height = 200;
  wcfg.utf8_title = "Screenshot";

  CMPHandle win;
  ws.vtable->create_window(ws.ctx, &wcfg, &win);

  int i;
  for (i = 0; i < 10; i++)
    do {
      CMPInputEvent ev;
      CMPBool has;
      ws.vtable->poll_event(ws.ctx, &ev, &has);
    } while (0);

  CMPTextBackend text_backend;
  text_backend.ctx = gfx.ctx;
  text_backend.vtable = gfx.text_vtable;

  gfx.vtable->begin_frame(gfx.ctx, win, 200, 200, 1.0f);
  CMPColor white = {255, 255, 255, 255};
  gfx.vtable->clear(gfx.ctx, white);

  if (strcmp(widget_name, "button") == 0) {
    M3ButtonStyle style;
    m3_button_style_init_filled(&style);
    M3Button w;
    m3_button_init(&w, &text_backend, &style, "Button", 6);
    CMPRect bounds = {50, 50, 100, 40};

    w.widget.vtable->layout(w.widget.ctx, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(w.widget.ctx, &pctx);
  } else if (strcmp(widget_name, "card") == 0) {
    M3CardStyle style;
    m3_card_style_init_elevated(&style);
    M3Card w;
    m3_card_init(&w, &style);
    CMPRect bounds = {20, 20, 160, 160};

    w.widget.vtable->layout(w.widget.ctx, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(w.widget.ctx, &pctx);
  } else if (strcmp(widget_name, "checkbox") == 0) {
    M3CheckboxStyle style;
    m3_checkbox_style_init(&style);
    M3Checkbox w;
    m3_checkbox_init(&w, &style, CMP_TRUE);
    CMPRect bounds = {80, 80, 40, 40};

    w.widget.vtable->layout(w.widget.ctx, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(w.widget.ctx, &pctx);
  } else if (strcmp(widget_name, "switch") == 0) {
    M3SwitchStyle style;
    m3_switch_style_init(&style);
    M3Switch w;
    m3_switch_init(&w, &style, CMP_TRUE);
    CMPRect bounds = {60, 80, 80, 40};

    w.widget.vtable->layout(w.widget.ctx, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(w.widget.ctx, &pctx);
  } else if (strcmp(widget_name, "slider") == 0) {
    M3SliderStyle style;
    m3_slider_style_init(&style);
    M3Slider w;
    m3_slider_init(&w, &style, 0, 100, 50);
    CMPRect bounds = {20, 80, 160, 40};

    w.widget.vtable->layout(w.widget.ctx, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(w.widget.ctx, &pctx);
  } else if (strcmp(widget_name, "progress") == 0) {
    M3LinearProgressStyle style;
    m3_linear_progress_style_init(&style);
    M3LinearProgress w;
    m3_linear_progress_init(&w, &style, 0.5f);
    CMPRect bounds = {20, 90, 160, 20};

    w.widget.vtable->layout(w.widget.ctx, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(w.widget.ctx, &pctx);
  } else if (strcmp(widget_name, "chip") == 0) {
    M3ChipStyle style;
    m3_chip_style_init_filter(&style);
    M3Chip w;
    m3_chip_init(&w, &text_backend, &style, "Chip", 4);
    CMPRect bounds = {60, 80, 80, 40};

    w.widget.vtable->layout(w.widget.ctx, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(w.widget.ctx, &pctx);

  } else if (strcmp(widget_name, "text_field") == 0) {
    CMPAllocator allocator;
    cmp_get_default_allocator(&allocator);
    M3TextFieldStyle style;
    m3_text_field_style_init(&style, CMP_TRUE);
    M3TextField w;
    m3_text_field_init(&w, &style, allocator, text_backend);
    m3_text_field_set_label(&w, "Label");
    m3_text_field_set_text(&w, "Value");
    CMPRect bounds = {20, 80, 160, 56};
    w.widget.vtable->layout(&w, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(&w, &pctx);
  } else if (strcmp(widget_name, "tabs") == 0) {

    M3TabRowStyle style;
    m3_tab_row_style_init(&style);
    M3TabItem items[2];
    memset(items, 0, sizeof(items));
    items[0].utf8_label = "Tab 1";
    items[0].utf8_len = 5;
    items[1].utf8_label = "Tab 2";
    items[1].utf8_len = 5;
    M3TabRow w;
    m3_tab_row_init(&w, &text_backend, &style, items, 2, 0);
    CMPRect bounds = {0, 80, 200, 48};
    w.widget.vtable->layout(&w, bounds);
    CMPPaintContext pctx;
    pctx.gfx = &gfx;
    pctx.clip.x = 0;
    pctx.clip.y = 0;
    pctx.clip.width = 200;
    pctx.clip.height = 200;
    pctx.dpi_scale = 1.0f;
    w.widget.vtable->paint(&w, &pctx);
  } else {
    printf("Unknown widget: %s\n", widget_name);
  }

  gfx.vtable->end_frame(gfx.ctx, win);
  cmp_gtk4_backend_save_png(be, win, out_path);

  ws.vtable->destroy_window(ws.ctx, win);
  cmp_gtk4_backend_destroy(be);
  return 0;
}
