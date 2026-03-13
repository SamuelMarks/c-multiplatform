/* clang-format off */
#include <cmpc/cmp_api_env.h>
#include <cmpc/cmp_backend_gtk4.h>
#include <cupertino/cupertino_button.h>
#include <cupertino/cupertino_progress_bar.h>
#include <cupertino/cupertino_segmented_control.h>
#include <cupertino/cupertino_slider.h>
#include <cupertino/cupertino_switch.h>
#include <cupertino/cupertino_tab_bar.h>
#include <cupertino/cupertino_text_field.h>
#include <cupertino/cupertino_tv_card.h>
#include <f2/f2_badge.h>
#include <f2/f2_button.h>
#include <f2/f2_card.h>
#include <f2/f2_navigation.h>
#include <f2/f2_progress.h>
#include <f2/f2_selection.h>
#include <f2/f2_slider.h>
#include <f2/f2_text_field.h>
#include <m3/m3_button.h>
#include <m3/m3_card.h>
#include <m3/m3_chip.h>
#include <m3/m3_progress.h>
#include <m3/m3_selection.h>
#include <m3/m3_tabs.h>
#include <m3/m3_text_field.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
/* clang-format on */
#define MKDIR(path) mkdir(path, 0777)
#endif

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: %s <widget_name> <out_dir>\n", argv[0]);
    return 1;
  }
  const char *widget_name = argv[1];
  const char *out_dir = argv[2];

  const char *themes[] = {"google_material_3", "microsoft_fluent_2",
                          "apple_cupertino"};

  char path_buf[1024];
  for (int t = 0; t < 3; t++) {
    snprintf(path_buf, sizeof(path_buf), "%s/%s", out_dir, themes[t]);
    MKDIR(path_buf);
  }

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

  for (int t = 0; t < 3; t++) {
    const char *theme = themes[t];
    CMPHandle win;
    ws.vtable->create_window(ws.ctx, &wcfg, &win);

    for (int i = 0; i < 10; i++) {
      CMPInputEvent ev;
      CMPBool has;
      ws.vtable->poll_event(ws.ctx, &ev, &has);
    }

    CMPTextBackend text_backend;
    text_backend.ctx = gfx.ctx;
    text_backend.vtable = gfx.text_vtable;

    gfx.vtable->begin_frame(gfx.ctx, win, 200, 200, 1.0f);
    CMPColor white = {255, 255, 255, 255};
    gfx.vtable->clear(gfx.ctx, white);

    CMPWidget *w_ptr = NULL;

    if (strcmp(theme, "google_material_3") == 0) {
      if (strcmp(widget_name, "button") == 0) {
        static M3ButtonStyle style;
        m3_button_style_init_filled(&style);
        static M3Button w;
        m3_button_init(&w, &text_backend, &style, "Button", 6);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "card") == 0) {
        static M3CardStyle style;
        m3_card_style_init_elevated(&style);
        static M3Card w;
        m3_card_init(&w, &style);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "checkbox") == 0) {
        static M3CheckboxStyle style;
        m3_checkbox_style_init(&style);
        static M3Checkbox w;
        m3_checkbox_init(&w, &style, CMP_TRUE);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "switch") == 0) {
        static M3SwitchStyle style;
        m3_switch_style_init(&style);
        static M3Switch w;
        m3_switch_init(&w, &style, CMP_TRUE);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "slider") == 0) {
        static M3SliderStyle style;
        m3_slider_style_init(&style);
        static M3Slider w;
        m3_slider_init(&w, &style, 0, 100, 50);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "progress") == 0) {
        static M3LinearProgressStyle style;
        m3_linear_progress_style_init(&style);
        static M3LinearProgress w;
        m3_linear_progress_init(&w, &style, 0.5f);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "chip") == 0) {
        static M3ChipStyle style;
        m3_chip_style_init_filter(&style);
        static M3Chip w;
        m3_chip_init(&w, &text_backend, &style, "Chip", 4);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "text_field") == 0) {
        CMPAllocator allocator;
        cmp_get_default_allocator(&allocator);
        static M3TextFieldStyle style;
        m3_text_field_style_init(&style, CMP_TRUE);
        static M3TextField w;
        m3_text_field_init(&w, &style, allocator, text_backend);
        m3_text_field_set_label(&w, "Label");
        m3_text_field_set_text(&w, "Value");
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "tabs") == 0) {
        static M3TabRowStyle style;
        m3_tab_row_style_init(&style);
        static M3TabItem items[2];
        memset(items, 0, sizeof(items));
        items[0].utf8_label = "Tab 1";
        items[0].utf8_len = 5;
        items[1].utf8_label = "Tab 2";
        items[1].utf8_len = 5;
        static M3TabRow w;
        m3_tab_row_init(&w, &text_backend, &style, items, 2, 0);
        w_ptr = &w.widget;
      }
    } else if (strcmp(theme, "microsoft_fluent_2") == 0) {
      if (strcmp(widget_name, "button") == 0) {
        static F2ButtonStyle style;
        f2_button_style_init_primary(&style);
        static F2Button w;
        f2_button_init(&w, &text_backend, &style, "Button", 6);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "card") == 0) {
        static F2CardStyle style;
        f2_card_style_init(&style);
        static F2Card w;
        f2_card_init(&w, &style);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "checkbox") == 0) {
        static F2CheckboxStyle style;
        f2_checkbox_style_init(&style);
        static F2Checkbox w;
        f2_checkbox_init(&w, &style, CMP_TRUE);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "switch") == 0) {
        static F2SwitchStyle style;
        f2_switch_style_init(&style);
        static F2Switch w;
        f2_switch_init(&w, &style, CMP_TRUE);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "slider") == 0) {
        static F2SliderStyle style;
        f2_slider_style_init(&style);
        static F2Slider w;
        f2_slider_init(&w, &style, 0, 100, 50);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "progress") == 0) {
        static F2LinearProgressStyle style;
        f2_linear_progress_style_init(&style);
        static F2LinearProgress w;
        f2_linear_progress_init(&w, &style, 0.5f);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "chip") == 0) {
        static F2BadgeStyle style;
        f2_badge_style_init(&style);
        static F2Badge w;
        f2_badge_init(&w, &style, "Chip", 4);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "text_field") == 0) {
        CMPAllocator allocator;
        cmp_get_default_allocator(&allocator);
        static F2TextFieldStyle style;
        f2_text_field_style_init(&style);
        static F2TextField w;
        f2_text_field_init(&w, &text_backend, &style, &allocator);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "tabs") == 0) {
        static F2NavItem items[2];
        items[0].utf8_label = "Tab 1";
        items[0].utf8_len = 5;
        items[1].utf8_label = "Tab 2";
        items[1].utf8_len = 5;
        static F2NavViewStyle style;
        f2_navigation_view_style_init(&style);
        static F2NavigationView w;
        f2_navigation_view_init(&w, &style, items, 2);
        w_ptr = &w.widget;
      }
    } else if (strcmp(theme, "apple_cupertino") == 0) {
      if (strcmp(widget_name, "button") == 0) {
        static CupertinoButton w;
        cupertino_button_init(&w, &text_backend);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "card") == 0) {
        static CupertinoTVCard w;
        cupertino_tv_card_init(&w, NULL);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "checkbox") == 0) {
        static CupertinoSwitch w;
        cupertino_switch_init(&w);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "switch") == 0) {
        static CupertinoSwitch w;
        cupertino_switch_init(&w);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "slider") == 0) {
        static CupertinoSlider w;
        cupertino_slider_init(&w);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "progress") == 0) {
        static CupertinoProgressBar w;
        cupertino_progress_bar_init(&w);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "chip") == 0) {
        static CupertinoSegmentedControl w;
        cupertino_segmented_control_init(&w, &text_backend);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "text_field") == 0) {
        static CupertinoTextField w;
        cupertino_text_field_init(&w, &text_backend);
        w_ptr = &w.widget;
      } else if (strcmp(widget_name, "tabs") == 0) {
        static CupertinoTabBar w;
        cupertino_tab_bar_init(&w, &text_backend);
        w_ptr = &w.widget;
      }
    }

    if (w_ptr) {
      CMPRect bounds = {20, 20, 160, 160};
      if (strcmp(widget_name, "button") == 0) {
        bounds.x = 50;
        bounds.y = 50;
        bounds.width = 100;
        bounds.height = 40;
      } else if (strcmp(widget_name, "checkbox") == 0) {
        bounds.x = 80;
        bounds.y = 80;
        bounds.width = 40;
        bounds.height = 40;
      } else if (strcmp(widget_name, "switch") == 0) {
        bounds.x = 60;
        bounds.y = 80;
        bounds.width = 80;
        bounds.height = 40;
      } else if (strcmp(widget_name, "slider") == 0) {
        bounds.x = 20;
        bounds.y = 80;
        bounds.width = 160;
        bounds.height = 40;
      } else if (strcmp(widget_name, "progress") == 0) {
        bounds.x = 20;
        bounds.y = 90;
        bounds.width = 160;
        bounds.height = 20;
      } else if (strcmp(widget_name, "chip") == 0) {
        bounds.x = 60;
        bounds.y = 80;
        bounds.width = 80;
        bounds.height = 40;
      } else if (strcmp(widget_name, "text_field") == 0) {
        bounds.x = 20;
        bounds.y = 80;
        bounds.width = 160;
        bounds.height = 56;
      } else if (strcmp(widget_name, "tabs") == 0) {
        bounds.x = 0;
        bounds.y = 80;
        bounds.width = 200;
        bounds.height = 48;
      }

      w_ptr->vtable->layout(w_ptr->ctx, bounds);
      CMPPaintContext pctx;
      pctx.gfx = &gfx;
      pctx.clip.x = 0;
      pctx.clip.y = 0;
      pctx.clip.width = 200;
      pctx.clip.height = 200;
      pctx.dpi_scale = 1.0f;
      w_ptr->vtable->paint(w_ptr->ctx, &pctx);
    } else {
      printf("Unknown widget or no implementation: %s for %s\n", widget_name,
             theme);
    }

    gfx.vtable->end_frame(gfx.ctx, win);

    char out_path[1024];
    snprintf(out_path, sizeof(out_path), "%s/%s/%s.png", out_dir, theme,
             widget_name);
    cmp_gtk4_backend_save_png(be, win, out_path);

    ws.vtable->destroy_window(ws.ctx, win);
  }

  cmp_gtk4_backend_destroy(be);
  return 0;
}
