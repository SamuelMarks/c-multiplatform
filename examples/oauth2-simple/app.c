/* clang-format off */
#include "app.h"
#include "cmp.h"
#include <stdio.h>
#include <string.h>
#include "themes/cmp_f2_text_inputs.h"
#include "themes/cmp_material3_text_inputs.h"
#include "themes/cmp_material3_components.h"
#include "themes/cmp_f2_button.h"
#include "themes/cmp_cupertino.h"


#ifdef _WIN32
extern int __stdcall GetSystemMetrics(int nIndex);
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#endif
/* clang-format on */

static cmp_window_t *g_window = NULL;
static cmp_ui_node_t *g_ui_tree = NULL;
static cmp_theme_t *g_theme = NULL;
static int g_current_theme = 2; /* 1: Material3, 2: Fluent2, 3: Cupertino */
static int g_current_route = 0; /* 0: Login, 1: Secrets */
static int g_is_dark = 0;       /* 0: Light Mode, 1: Dark Mode */

static float g_window_width = 800.0f;
static float g_window_height = 600.0f;
static float g_dpi_scale = 1.0f;

static int g_focused_input = 0;
static char g_user_text[256] = "";
static char g_pass_text[256] = "";
static cmp_a11y_tree_t *g_a11y_tree = NULL;
static cmp_screen_reader_t *g_screen_reader = NULL;
static cmp_focus_ring_t *g_focus_ring = NULL;
static cmp_focus_manager_t *g_focus_manager = NULL;
static cmp_aria_t *g_aria = NULL;
static cmp_caret_t *g_caret = NULL;
static cmp_keyboard_avoidance_t *g_kb_avoidance = NULL;
static cmp_form_controls_t *g_form_controls = NULL;
static cmp_hover_intent_t *g_hover_intent = NULL;
static cmp_compositor_anim_t *g_btn_opacity_anim = NULL;
static cmp_keyframe_t *g_loading_spinner = NULL;
static int g_is_hovering_btn = 0;
static int g_is_loading = 0;
static float g_btn_opacity = 1.0f;
#define ID_TITLE 100

static char g_pass_display[256] = "";

#define ID_THEME_BTN 101
#define ID_LOGIN_BTN 102
#define ID_LOGOUT_BTN 103
#define ID_DARK_BTN 104
#define ID_USER_IN 105
#define ID_PASS_IN 106

/**
 * @brief Manually perform hit testing on the UI tree
 * @param node The root node to test
 * @param x The X coordinate
 * @param y The Y coordinate
 * @return The ID of the node that was hit, or -1 if none
 */

/**
 * @brief Constructs the application's UI tree
 * @return 0 on success, or an error code
 */

static void add_i18n_text(cmp_ui_node_t *parent, const char *key,
                          cmp_ui_node_t **out_node) {
  cmp_string_t localized = {0};
  if (cmp_i18n_translate(key, &localized) == CMP_SUCCESS && localized.data) {
    cmp_ui_text_create(out_node, localized.data, -1);
    cmp_string_destroy(&localized);
  } else {
    cmp_ui_text_create(out_node, key, -1);
  }
  if (parent && out_node && *out_node) {
    cmp_ui_node_add_child(parent, *out_node);
  }
}

static void set_i18n_button(cmp_ui_node_t **btn, const char *key) {
  cmp_string_t localized = {0};
  const char *text = key;
  if (cmp_i18n_translate(key, &localized) == CMP_SUCCESS && localized.data) {
    text = localized.data;
  }
  if (g_current_theme == 2) {
    cmp_f2_button_create(btn, text, NULL);
    if (*btn) {
      cmp_f2_button_set_variant(*btn, CMP_F2_BUTTON_VARIANT_PRIMARY);
      {
        cmp_ui_node_t *t = NULL;
        if (cmp_ui_text_create(&t, text, -1) == CMP_SUCCESS) {
          t->text_color = 0xFFFFFFFF;
          cmp_ui_node_add_child(*btn, t);
        }
      }
    }
  } else {
    cmp_ui_button_create(btn, text, -1);
    if (*btn && g_current_theme == 1) {
      cmp_m3_button_metrics_t m3b;
      if (cmp_m3_button_resolve(CMP_M3_BUTTON_FILLED, 0, &m3b) == CMP_SUCCESS) {
        (*btn)->layout->padding[3] = m3b.padding_left;
        (*btn)->layout->padding[1] = m3b.padding_right;
        (*btn)->layout->height = m3b.height;
      }
    } else if (*btn && g_current_theme == 3) {
      (*btn)->layout->height = 44.0f;
    }
  }
  if (localized.data)
    cmp_string_destroy(&localized);
}

static int build_ui(void) {
  cmp_ui_node_t *top_bar = NULL;
  cmp_ui_node_t *theme_btn = NULL;
  cmp_ui_node_t *dark_btn = NULL;
  cmp_ui_node_t *content_box = NULL;
  const char *theme_name = "";
  int res;

  uint32_t bg_main, text_main;
  uint32_t lbl_color, input_bg, input_text, btn_bg, btn_text;
  float input_h, btn_h, input_pad, btn_pad;

  int is_user_focused = (g_focused_input == ID_USER_IN);
  int has_user_text = (strlen(g_user_text) > 0);
  int user_active = is_user_focused || has_user_text;

  int is_pass_focused = (g_focused_input == ID_PASS_IN);
  int has_pass_text = (strlen(g_pass_text) > 0);
  int pass_active = is_pass_focused || has_pass_text;

  if (g_ui_tree != NULL) {
    cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  res = cmp_ui_box_create(&g_ui_tree);
  if (res != CMP_SUCCESS)
    return res;

  g_ui_tree->design_language_override = g_current_theme;

  if (g_current_theme == 1) { /* Material 3 */
    bg_main = g_is_dark ? 0xFF141218 : 0xFFFEF7FF;
    text_main = g_is_dark ? 0xFFE6E0E9 : 0xFF1D1B20;
    lbl_color = g_is_dark ? 0xFFCAC4D0 : 0xFF49454F;
    input_bg = g_is_dark ? 0xFF49454F : 0xFFE6E0E9;
    input_text = g_is_dark ? 0xFFE6E0E9 : 0xFF1D1B20;
    btn_bg = g_is_dark ? 0xFFD0BCFF : 0xFF6750A4;
    btn_text = g_is_dark ? 0xFF381E72 : 0xFFFFFFFF;
    input_h = 56.0f;
    btn_h = 40.0f;
    input_pad = 16.0f;
    btn_pad = 24.0f;
  } else if (g_current_theme == 2) { /* Fluent 2 */
    bg_main = g_is_dark ? 0xFF202020 : 0xFFF3F3F3;
    text_main = g_is_dark ? 0xFFFFFFFF : 0xFF000000;
    lbl_color = g_is_dark ? 0xFFFFFFFF : 0xFF000000;
    input_bg = g_is_dark ? 0xFF2D2D2D : 0xFFFFFFFF;
    input_text = g_is_dark ? 0xFFFFFFFF : 0xFF000000;
    btn_bg = g_is_dark ? 0xFF0078D4 : 0xFF005FB8;
    btn_text = 0xFFFFFFFF;
    input_h = 56.0f; /* Matched to M3 height for layout consistency here */
    btn_h = 32.0f;
    input_pad = 8.0f;
    btn_pad = 12.0f;
  } else { /* Cupertino */
    bg_main = g_is_dark ? 0xFF000000 : 0xFFFFFFFF;
    text_main = g_is_dark ? 0xFFFFFFFF : 0xFF000000;
    lbl_color = g_is_dark ? 0xFFEBEBF5 : 0xFF3C3C43;
    input_bg = g_is_dark ? 0xFF1C1C1E : 0xFFF2F2F7;
    input_text = g_is_dark ? 0xFFFFFFFF : 0xFF000000;
    btn_bg = g_is_dark ? 0xFF0A84FF : 0xFF007AFF;
    btn_text = 0xFFFFFFFF;
    input_h = 56.0f;
    btn_h = 44.0f;
    input_pad = 12.0f;
    btn_pad = 20.0f;
  }

  g_ui_tree->bg_color = bg_main;
  g_ui_tree->text_color = text_main;

  g_ui_tree->layout->direction = CMP_FLEX_COLUMN;
  g_ui_tree->layout->width = g_window_width;
  g_ui_tree->layout->height = g_window_height;
  g_ui_tree->layout->padding[0] = 16.0f;
  g_ui_tree->layout->padding[1] = 16.0f;
  g_ui_tree->layout->padding[2] = 16.0f;
  g_ui_tree->layout->padding[3] = 16.0f;

  /* Top Bar */
  if (cmp_ui_box_create(&top_bar) == CMP_SUCCESS) {
    cmp_ui_node_t *spacer = NULL;

    top_bar->layout->direction = CMP_FLEX_ROW;
    top_bar->layout->width = g_window_width - 32.0f;
    top_bar->layout->height = 48.0f;
    top_bar->layout->align_items = 1; /* CMP_FLEX_ALIGN_CENTER */

    if (g_current_route == 1) {
      cmp_ui_node_t *logout_btn = NULL;
      set_i18n_button(&logout_btn, "Logout");
      if (logout_btn) {
        logout_btn->layout->id = ID_LOGOUT_BTN;
        logout_btn->layout->width = 120.0f;
        logout_btn->layout->height = btn_h;
        logout_btn->bg_color = g_is_dark ? 0xFF880000 : 0xFFCC0000;
        logout_btn->text_color = 0xFFFFFFFF;
        cmp_ui_node_add_child(top_bar, logout_btn);
      }
    }

    if (cmp_ui_box_create(&spacer) == CMP_SUCCESS) {
      spacer->layout->flex_grow = 1.0f;
      cmp_ui_node_add_child(top_bar, spacer);
    }

    switch (g_current_theme) {
    case 1:
      theme_name = "Theme: Material 3";
      break;
    case 2:
      theme_name = "Theme: Fluent 2";
      break;
    case 3:
      theme_name = "Theme: Cupertino";
      break;
    default:
      theme_name = "Theme: Unknown";
      break;
    }

    set_i18n_button(&theme_btn, theme_name);
    if (theme_btn) {
      theme_btn->layout->id = ID_THEME_BTN;
      theme_btn->layout->width = 200.0f;
      theme_btn->layout->height = btn_h;
      theme_btn->layout->margin[1] = 8.0f;
      theme_btn->bg_color = input_bg;
      theme_btn->text_color = input_text;
      cmp_ui_node_add_child(top_bar, theme_btn);
    }

    if (cmp_ui_button_create(&dark_btn, g_is_dark ? "Light Mode" : "Dark Mode",
                             -1) == CMP_SUCCESS) {
      dark_btn->layout->id = ID_DARK_BTN;
      dark_btn->layout->width = 120.0f;
      dark_btn->layout->height = btn_h;
      dark_btn->bg_color = input_bg;
      dark_btn->text_color = input_text;
      cmp_ui_node_add_child(top_bar, dark_btn);
    }

    cmp_ui_node_add_child(g_ui_tree, top_bar);
  }

  /* Content */
  if (cmp_ui_box_create(&content_box) == CMP_SUCCESS) {
    content_box->layout->width = g_window_width - 32.0f;
    content_box->layout->direction = CMP_FLEX_COLUMN;
    content_box->layout->align_items = 1;     /* CMP_FLEX_ALIGN_CENTER */
    content_box->layout->justify_content = 1; /* CMP_FLEX_ALIGN_CENTER */
    content_box->layout->flex_grow = 1.0f;

    if (g_current_route == 0) {
      /* Login Route */
      cmp_ui_node_t *title = NULL;
      cmp_ui_node_t *user_container = NULL;
      cmp_ui_node_t *user_lbl = NULL;
      cmp_ui_node_t *user_val = NULL;
      cmp_ui_node_t *user_border = NULL;
      cmp_ui_node_t *pass_container = NULL;
      cmp_ui_node_t *pass_lbl = NULL;
      cmp_ui_node_t *pass_val = NULL;
      cmp_ui_node_t *pass_border = NULL;
      cmp_ui_node_t *login_btn = NULL;

      if (cmp_ui_text_create(&title, "Login to OAuth2 Simple", -1) ==
          CMP_SUCCESS) {
        title->layout->width = g_window_width - 64.0f;
        title->layout->height = 36.0f;
        title->layout->margin[2] = 24.0f;
        title->layout->align_self = 1; /* CMP_FLEX_ALIGN_CENTER */
        title->text_color = text_main;
        cmp_ui_node_add_child(content_box, title);
      }

      /* Username Field */

      if (g_current_theme == 2) {
        cmp_f2_text_input_create(&user_container);
        cmp_f2_text_input_set_variant(user_container,
                                      CMP_F2_TEXT_INPUT_VARIANT_OUTLINE);
      } else {
        cmp_ui_text_input_create(&user_container);
      }
      if (user_container) {

        user_container->layout->id = ID_USER_IN;
        user_container->layout->width = g_window_width - 64.0f;
        user_container->layout->height = input_h;
        user_container->layout->margin[2] = 16.0f;
        user_container->bg_color = input_bg;
        user_container->layout->direction = CMP_FLEX_COLUMN;

        add_i18n_text(NULL, "Username", &user_lbl);
        user_lbl->bg_color = 0;

        if (user_active) {
          user_lbl->font_size = 12.0f;
          user_lbl->text_color = is_user_focused ? btn_bg : lbl_color;
          user_lbl->layout->height = 24.0f;
          user_lbl->layout->padding[3] = input_pad;
          user_lbl->layout->padding[0] = 6.0f;
          cmp_ui_node_add_child(user_container, user_lbl);

          cmp_ui_text_create(&user_val, g_user_text, -1);
          user_val->font_size = 16.0f;
          user_val->text_color = input_text;
          user_val->bg_color = 0;
          user_val->layout->height = 30.0f;
          user_val->layout->padding[3] = input_pad;

          cmp_ui_node_add_child(user_container, user_val);
          if (g_caret && is_user_focused) {
            int is_visible = 0;
            cmp_caret_update_blink(g_caret, 16.0, &is_visible);
            if (is_visible) {
              cmp_ui_node_t *caret_node = NULL;
              cmp_ui_box_create(&caret_node);
              caret_node->layout->width = 2.0f;
              caret_node->layout->height = 20.0f;
              caret_node->bg_color = btn_bg;
              cmp_ui_node_add_child(user_container, caret_node);
            }
          }

        } else {
          user_lbl->font_size = 16.0f;
          user_lbl->text_color = lbl_color;
          user_lbl->layout->height = input_h - 2.0f;
          user_lbl->layout->padding[3] = input_pad;
          cmp_ui_node_add_child(user_container, user_lbl);
        }

        if (g_current_theme == 1 || g_current_theme == 2) {
          cmp_ui_box_create(&user_border);
          user_border->layout->width = g_window_width - 64.0f;
          user_border->layout->height = is_user_focused ? 2.0f : 1.0f;
          user_border->bg_color = is_user_focused ? btn_bg : lbl_color;
          cmp_ui_node_add_child(user_container, user_border);
        }
        cmp_ui_node_add_child(content_box, user_container);
      }

      /* Password Field */

      if (g_current_theme == 2) {
        cmp_f2_text_input_create(&pass_container);
        cmp_f2_text_input_set_password_mode(pass_container, 1);
      } else {
        cmp_ui_text_input_create(&pass_container);
      }
      if (pass_container) {

        pass_container->layout->id = ID_PASS_IN;
        pass_container->layout->width = g_window_width - 64.0f;
        pass_container->layout->height = input_h;
        pass_container->layout->margin[2] = 32.0f;
        pass_container->bg_color = input_bg;
        pass_container->layout->direction = CMP_FLEX_COLUMN;

        add_i18n_text(NULL, "Password", &pass_lbl);
        pass_lbl->bg_color = 0;

        if (pass_active) {
          size_t p_len = strlen(g_pass_text);
          size_t i;
          for (i = 0; i < p_len; i++)
            g_pass_display[i] = '*';
          g_pass_display[p_len] = '\0';

          pass_lbl->font_size = 12.0f;
          pass_lbl->text_color = is_pass_focused ? btn_bg : lbl_color;
          pass_lbl->layout->height = 24.0f;
          pass_lbl->layout->padding[3] = input_pad;
          pass_lbl->layout->padding[0] = 6.0f;
          cmp_ui_node_add_child(pass_container, pass_lbl);

          cmp_ui_text_create(&pass_val, g_pass_display, -1);
          pass_val->font_size = 16.0f;
          pass_val->text_color = input_text;
          pass_val->bg_color = 0;
          pass_val->layout->height = 30.0f;
          pass_val->layout->padding[3] = input_pad;

          cmp_ui_node_add_child(pass_container, pass_val);
          if (g_caret && is_pass_focused) {
            int is_visible = 0;
            cmp_caret_update_blink(g_caret, 16.0, &is_visible);
            if (is_visible) {
              cmp_ui_node_t *caret_node = NULL;
              cmp_ui_box_create(&caret_node);
              caret_node->layout->width = 2.0f;
              caret_node->layout->height = 20.0f;
              caret_node->bg_color = btn_bg;
              cmp_ui_node_add_child(pass_container, caret_node);
            }
          }

        } else {
          pass_lbl->font_size = 16.0f;
          pass_lbl->text_color = lbl_color;
          pass_lbl->layout->height = input_h - 2.0f;
          pass_lbl->layout->padding[3] = input_pad;
          cmp_ui_node_add_child(pass_container, pass_lbl);
        }

        if (g_current_theme == 1 || g_current_theme == 2) {
          cmp_ui_box_create(&pass_border);
          pass_border->layout->width = g_window_width - 64.0f;
          pass_border->layout->height = is_pass_focused ? 2.0f : 1.0f;
          pass_border->bg_color = is_pass_focused ? btn_bg : lbl_color;
          cmp_ui_node_add_child(pass_container, pass_border);
        }
        cmp_ui_node_add_child(content_box, pass_container);
      }

      set_i18n_button(&login_btn, "Login");
      if (login_btn) {
        login_btn->layout->id = ID_LOGIN_BTN;
        login_btn->layout->width = g_window_width - 64.0f;
        login_btn->layout->height = btn_h;
        login_btn->layout->padding[1] = btn_pad;
        login_btn->layout->padding[3] = btn_pad;
        login_btn->bg_color =
            (btn_bg & 0x00FFFFFF) | ((uint32_t)(255.0f * g_btn_opacity) << 24);
        login_btn->text_color = btn_text;
        cmp_ui_node_add_child(content_box, login_btn);
      }

    } else {
      /* Secrets Route */
      cmp_ui_node_t *title = NULL;
      cmp_ui_node_t *secret_txt = NULL;

      if (cmp_ui_text_create(&title, "Secrets Page", -1) == CMP_SUCCESS) {
        title->layout->width = g_window_width - 64.0f;
        title->layout->height = 36.0f;
        title->layout->margin[2] = 24.0f;
        title->layout->align_self = 1; /* CMP_FLEX_ALIGN_CENTER */
        title->text_color = text_main;
        cmp_ui_node_add_child(content_box, title);
      }

      if (cmp_ui_text_create(&secret_txt,
                             "Here are your highly confidential secrets: 42",
                             -1) == CMP_SUCCESS) {
        secret_txt->layout->width = g_window_width - 64.0f;
        secret_txt->layout->height = 36.0f;
        secret_txt->layout->align_self = 1; /* CMP_FLEX_ALIGN_CENTER */
        secret_txt->text_color = g_is_dark ? 0xFF00FF00 : 0xFF008000;
        cmp_ui_node_add_child(content_box, secret_txt);
      }
    }

    cmp_ui_node_add_child(g_ui_tree, content_box);
  }

  return CMP_SUCCESS;
}
int app_init(void) {
  cmp_window_config_t config;
  cmp_dpi_t *dpi = NULL;

  cmp_event_system_init();
  cmp_vfs_init();
  cmp_window_system_init();

  if (cmp_dpi_awareness_init() == CMP_SUCCESS) {
    if (cmp_dpi_create(&dpi) == CMP_SUCCESS) {
      if (cmp_dpi_get_monitor_scale(dpi, 0, &g_dpi_scale) != CMP_SUCCESS) {
        g_dpi_scale = 1.0f;
      }
      cmp_dpi_destroy(dpi);
    }
  }

  g_window_width = 1024.0f;
  g_window_height = 768.0f;

#ifdef _WIN32
  g_window_width = (float)GetSystemMetrics(SM_CXSCREEN) / (3.0f * g_dpi_scale);
  g_window_height = (float)GetSystemMetrics(SM_CYSCREEN) / (3.0f * g_dpi_scale);
#endif

  if (cmp_theme_create(&g_theme) == CMP_SUCCESS) {
    g_theme->language = g_current_theme;
    g_theme->is_dark_mode = g_is_dark;
  }

  config.title = "OAuth2 Simple";
  config.width = (int)(g_window_width * g_dpi_scale);
  config.height = (int)(g_window_height * g_dpi_scale);
  config.x = -1;
  config.y = -1;
  config.hidden = 0;
  config.frameless = 0;
  config.use_legacy_backend = 0;

  if (cmp_window_create(&config, &g_window) != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (g_theme) {
    cmp_window_set_theme(g_window, g_theme);
  }

  if (build_ui() != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (g_ui_tree) {
    cmp_layout_calculate(g_ui_tree->layout, g_window_width, g_window_height);
  }
  cmp_window_set_ui_tree(g_window, g_ui_tree);
  cmp_window_show(g_window);

  return CMP_SUCCESS;
}

int app_run(void) {
  cmp_event_t evt;
  int running = 1;

  while (running) {
    cmp_window_poll_events(g_window);

    while (cmp_event_pop(&evt) == CMP_SUCCESS) {
      if (evt.type == 4) { /* CMP_EVENT_TYPE_RESIZE */
        g_window_width = (float)evt.x;
        g_window_height = (float)evt.y;
        build_ui();
        if (g_ui_tree) {
          cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                               g_window_height);
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        }
        continue;
      }

      if (evt.type == 1) { /* CMP_EVENT_TYPE_POINTER_MOVE */
        int hn = -1;
        cmp_hit_test_t *ht = NULL;
        cmp_ui_node_t *hnode = NULL;
        if (cmp_hit_test_create(g_ui_tree, &ht) == CMP_SUCCESS) {
          if (cmp_hit_test_query(ht, (float)evt.x, (float)evt.y, &hnode) ==
                  CMP_SUCCESS &&
              hnode) {
            cmp_ui_node_t *curr = hnode;
            while (curr) {
              if (curr->layout && curr->layout->id == ID_LOGIN_BTN) {
                hn = ID_LOGIN_BTN;
                break;
              }
              curr = curr->parent;
            }
          }
          cmp_hit_test_destroy(ht);
        }

        if (hn == ID_LOGIN_BTN) {
          if (!g_is_hovering_btn) {
            if (cmp_hover_intent_process(g_hover_intent, &evt, 16.0f)) {
              cmp_compositor_val_t s = {0}, e = {0};
              g_is_hovering_btn = 1;
              s.opacity = g_btn_opacity;
              e.opacity = 0.8f;
              cmp_compositor_anim_set_range(g_btn_opacity_anim, &s, &e);
            }
          }
        } else {
          if (g_is_hovering_btn) {
            cmp_compositor_val_t s = {0}, e = {0};
            g_is_hovering_btn = 0;
            s.opacity = g_btn_opacity;
            e.opacity = 1.0f;
            cmp_compositor_anim_set_range(g_btn_opacity_anim, &s, &e);
          }
          evt.action = CMP_ACTION_CANCEL;
          cmp_hover_intent_process(g_hover_intent, &evt, 0.0f);
        }
      }

      if (evt.action == CMP_ACTION_DOWN && (evt.type == 1 || evt.type == 2)) {
        int hit_node = -1;
        cmp_hit_test_t *ht = NULL;
        cmp_ui_node_t *hn = NULL;
        if (cmp_hit_test_create(g_ui_tree, &ht) == CMP_SUCCESS) {
          if (cmp_hit_test_query(ht, (float)evt.x, (float)evt.y, &hn) ==
                  CMP_SUCCESS &&
              hn) {
            while (hn) {
              if (hn->layout && hn->layout->id != 0) {
                hit_node = hn->layout->id;
                break;
              }
              hn = hn->parent;
            }
          }
          cmp_hit_test_destroy(ht);
        }

        if (hit_node == ID_USER_IN || hit_node == ID_PASS_IN) {
          g_focused_input = hit_node;
          build_ui();
          if (g_ui_tree) {
            cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                 g_window_height);
          }
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        } else if (hit_node != ID_THEME_BTN && hit_node != ID_DARK_BTN &&
                   hit_node != ID_LOGIN_BTN && hit_node != ID_LOGOUT_BTN) {
          g_focused_input = 0;
          build_ui();
          if (g_ui_tree) {
            cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                 g_window_height);
          }
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        }

        if (hit_node == ID_THEME_BTN) {
          g_current_theme++;
          if (g_current_theme > 3)
            g_current_theme = 1;
          if (g_theme) {
            g_theme->language = g_current_theme;
            cmp_window_set_theme(g_window, g_theme);
          }
          build_ui();
          if (g_ui_tree) {
            cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                 g_window_height);
          }
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        } else if (hit_node == ID_DARK_BTN) {
          g_is_dark = !g_is_dark;
          if (g_theme) {
            g_theme->is_dark_mode = g_is_dark;
            cmp_window_set_theme(g_window, g_theme);
          }
          build_ui();
          if (g_ui_tree) {
            cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                 g_window_height);
          }
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        } else if (hit_node == ID_LOGIN_BTN) {
          g_current_route = 1;
          build_ui();
          if (g_ui_tree) {
            cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                 g_window_height);
          }
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        } else if (hit_node == ID_LOGOUT_BTN) {
          g_current_route = 0;
          build_ui();
          if (g_ui_tree) {
            cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                 g_window_height);
          }
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        }
      }

      if (evt.type == 3) { /* Keyboard */
        if (evt.action == CMP_ACTION_DOWN ||
            evt.action == CMP_ACTION_MOVE) { /* CHAR mapping hack */
          char c = (char)evt.source_id;
          char *target = NULL;

          if (g_focused_input == ID_USER_IN) {
            target = g_user_text;
          } else if (g_focused_input == ID_PASS_IN) {
            target = g_pass_text;
          }

          if (target) {
            size_t len = strlen(target);
            if (c == '\b' && len > 0) {
              target[len - 1] = '\0';
            } else if (c >= 32 && c <= 126 && len < 255) {
              target[len] = c;
              target[len + 1] = '\0';
            }
            build_ui();
            if (g_ui_tree) {
              cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                   g_window_height);
            }
            cmp_window_set_ui_tree(g_window, g_ui_tree);
          }
        }
      }
    }

    if (cmp_window_should_close(g_window)) {
      running = 0;
    }
  }

  return CMP_SUCCESS;
}

int app_shutdown(void) {
  if (g_ui_tree) {
    cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  if (g_window) {
    cmp_window_destroy(g_window);
    g_window = NULL;
  }

  if (g_theme) {
    cmp_theme_destroy(g_theme);
    g_theme = NULL;
  }

  cmp_window_system_shutdown();
  cmp_vfs_shutdown();
  cmp_event_system_shutdown();

  return CMP_SUCCESS;
}
