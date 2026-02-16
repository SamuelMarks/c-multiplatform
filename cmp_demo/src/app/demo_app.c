#include "demo_app.h"

#include <stdio.h>
#include <string.h>

/* Core Includes */
#include <cmpc/cmp_event.h>
#include <cmpc/cmp_layout.h>
#include <cmpc/cmp_list.h>
#include <cmpc/cmp_math.h>
#include <cmpc/cmp_render.h>
#include <cmpc/cmp_text_field.h>

/* Material 3 Includes */
#include <m3/m3_app_bar.h>
#include <m3/m3_button.h>
#include <m3/m3_card.h>
#include <m3/m3_chip.h>
#include <m3/m3_dialogs.h> /* For snackbar */
#include <m3/m3_progress.h>
#include <m3/m3_scaffold.h>
#include <m3/m3_selection.h>

/* Platform font selection */
#if defined(_WIN32)
#define DEMO_FONT "Segoe UI"
#elif defined(__APPLE__)
#define DEMO_FONT "Helvetica Neue"
#elif defined(__EMSCRIPTEN__)
#define DEMO_FONT "sans-serif"
#else
#define DEMO_FONT "Sans"
#endif

typedef enum { SCR_LOGIN, SCR_SHOWCASE, SCR_DETAIL } ScreenId;

struct DemoApp {
  CMPAllocator allocator;
  ScreenId current_screen;
  CMPRenderList render_list;
  CMPTextBackend text_backend;
  CMPEventDispatcher dispatcher;
  CMPLayoutDirection layout_dir;

  /* Styles (Shared) */
  M3ButtonStyle s_btn_filled;
  M3ButtonStyle s_btn_text;
  M3CardStyle s_card;
  CMPTextFieldStyle s_tf;
  M3AppBarStyle s_app_bar;
  M3SwitchStyle s_switch;
  M3CheckboxStyle s_checkbox;
  M3SliderStyle s_slider;
  M3LinearProgressStyle s_progress;
  M3ChipStyle s_chip;
  M3SnackbarStyle s_snackbar;

  /* --- Screen 0: Login --- */
  M3Card login_card;
  CMPTextField tf_user;
  CMPTextField tf_pass;
  M3Button btn_login;
  M3Snackbar login_snackbar; // Error message
  // Layout Nodes
  CMPLayoutNode l_root;
  CMPLayoutNode l_card;
  CMPLayoutNode l_tf_user;
  CMPLayoutNode l_tf_pass;
  CMPLayoutNode l_btn;
  CMPLayoutNode *c_root[1];
  CMPLayoutNode *c_card[3];

  /* --- Screen 1: Showcase --- */
  M3Scaffold show_scaffold;
  M3AppBar show_app_bar;
  CMPListView show_list;
  M3Button fab;
  // Recycled widgets for list
  M3Card r_card;
  M3Switch r_switch;
  M3Checkbox r_check;
  M3Slider r_slider;
  M3LinearProgress r_progress;
  M3Chip r_chip;
  // Data State
  CMPBool val_sw;
  CMPBool val_chk;
  CMPScalar val_slider;

  /* --- Screen 2: Detail --- */
  M3Scaffold det_scaffold;
  M3AppBar det_bar;
  M3Card det_card;
  M3Button btn_back;
  M3Button btn_action;
  // Layout Nodes
  CMPLayoutNode l_det_root;
  CMPLayoutNode l_det_card;
  CMPLayoutNode l_det_back;
  CMPLayoutNode l_det_act;
  CMPLayoutNode *c_det_root[1];
  CMPLayoutNode *c_det_card[2];

  CMPBool init_done;
};

/* --- Helpers --- */

static int measure_adapter(void *ctx, CMPLayoutMeasureSpec w,
                           CMPLayoutMeasureSpec h, CMPSize *out) {
  CMPWidget *widget = (CMPWidget *)ctx;
  CMPMeasureSpec ws = {w.mode, w.size};
  CMPMeasureSpec hs = {h.mode, h.size};
  return widget->vtable->measure(widget, ws, hs, out);
}

static void update_widget_focus(DemoApp *app, CMPWidget *widget,
                                CMPBool focused) {
  if (widget == NULL)
    return;

  /* Check if widget is one of the known text fields before casting */
  if (widget == (CMPWidget *)&app->tf_user ||
      widget == (CMPWidget *)&app->tf_pass) {
    cmp_text_field_set_focus((CMPTextField *)widget, focused);
  }
  /* Buttons don't have explicit set_focus in this API version,
     they rely on 'pressed' state from events. */
}

static CMPBool is_pointer_event(const CMPInputEvent *e) {
  return (e->type == CMP_INPUT_POINTER_DOWN ||
          e->type == CMP_INPUT_POINTER_UP ||
          e->type == CMP_INPUT_POINTER_MOVE ||
          e->type == CMP_INPUT_POINTER_SCROLL)
             ? CMP_TRUE
             : CMP_FALSE;
}

static CMPBool widget_contains_point(const CMPRect *bounds,
                                     const CMPInputEvent *evt) {
  CMPBool contains = CMP_FALSE;
  cmp_rect_contains_point(bounds, (CMPScalar)evt->data.pointer.x,
                          (CMPScalar)evt->data.pointer.y, &contains);
  return contains;
}

/* --- Callbacks --- */

static int on_login_click(void *ctx, M3Button *btn) {
  DemoApp *app = (DemoApp *)ctx;
  (void)btn;

  const char *text;
  cmp_usize len;
  cmp_text_field_get_text(&app->tf_user, &text, &len);

  if (len > 0) {
    printf("Login successful.\n");
    app->current_screen = SCR_SHOWCASE;
  } else {
    m3_snackbar_set_message(&app->login_snackbar, "Please enter a username",
                            23);
  }
  return CMP_OK;
}

static int on_fab_click(void *ctx, M3Button *btn) {
  (void)ctx;
  (void)btn;
  printf("FAB Action\n");
  return CMP_OK;
}

static int on_list_card_click(void *ctx, M3Card *card) {
  DemoApp *app = (DemoApp *)ctx;
  (void)card;
  app->current_screen = SCR_DETAIL;
  return CMP_OK;
}

static int on_back_click(void *ctx, M3Button *btn) {
  DemoApp *app = (DemoApp *)ctx;
  (void)btn;
  app->current_screen = SCR_SHOWCASE;
  return CMP_OK;
}

static int on_switch_chg(void *ctx, M3Switch *sw, CMPBool val) {
  DemoApp *app = (DemoApp *)ctx;
  (void)sw;
  app->val_sw = val;
  return CMP_OK;
}

static int on_check_chg(void *ctx, M3Checkbox *cb, CMPBool val) {
  DemoApp *app = (DemoApp *)ctx;
  (void)cb;
  app->val_chk = val;
  return CMP_OK;
}

static int on_slider_chg(void *ctx, M3Slider *sl, CMPScalar val) {
  DemoApp *app = (DemoApp *)ctx;
  (void)sl;
  app->val_slider = val;
  return CMP_OK;
}

/* List Binder: Simulates different content based on index */
static int showcase_bind(void *ctx, CMPListSlot *slot, cmp_usize index) {
  DemoApp *app = (DemoApp *)ctx;

  int type = index % 6;

  switch (type) {
  case 0:
    m3_card_set_style(&app->r_card, &app->s_card);
    slot->node.widget = (CMPWidget *)&app->r_card;
    break;
  case 1:
    m3_switch_set_on(&app->r_switch, app->val_sw);
    m3_switch_set_label(&app->r_switch, "Network", 7);
    slot->node.widget = (CMPWidget *)&app->r_switch;
    break;
  case 2:
    m3_slider_set_value(&app->r_slider, app->val_slider);
    m3_slider_set_label(&app->r_slider, "Volume", 6);
    slot->node.widget = (CMPWidget *)&app->r_slider;
    break;
  case 3:
    m3_linear_progress_set_value(&app->r_progress, app->val_slider / 100.0f);
    m3_linear_progress_set_label(&app->r_progress, "Loading...", 10);
    slot->node.widget = (CMPWidget *)&app->r_progress;
    break;
  case 4:
    m3_checkbox_set_checked(&app->r_check, app->val_chk);
    m3_checkbox_set_label(&app->r_check, "Terms", 5);
    slot->node.widget = (CMPWidget *)&app->r_check;
    break;
  case 5:
    m3_chip_set_label(&app->r_chip, "Filter", 6);
    m3_chip_set_selected(&app->r_chip, index % 2 == 0);
    slot->node.widget = (CMPWidget *)&app->r_chip;
    break;
  }
  return CMP_OK;
}

/* --- Life Cycle --- */

int demo_app_create(CMPAllocator *allocator, DemoApp **out_app) {
  DemoApp *app;
  allocator->alloc(allocator->ctx, sizeof(DemoApp), (void **)&app);
  memset(app, 0, sizeof(DemoApp));
  app->allocator = *allocator;

  cmp_render_list_init(&app->render_list, allocator, 4096);
  cmp_event_dispatcher_init(&app->dispatcher);
  cmp_layout_direction_init(&app->layout_dir, CMP_DIRECTION_LTR);

  app->current_screen = SCR_LOGIN;
  app->val_sw = CMP_TRUE;
  app->val_slider = 30.0f;

  *out_app = app;
  return CMP_OK;
}

int demo_app_destroy(DemoApp *app) {
  if (!app)
    return CMP_OK;
  cmp_render_list_shutdown(&app->render_list);
  app->allocator.free(app->allocator.ctx, app);
  return CMP_OK;
}

int demo_app_init_resources(DemoApp *app, CMPGfx *gfx, CMPEnv *env) {
  if (app->init_done)
    return CMP_OK;
  (void)env;

  /* 1. Init Backend */
  cmp_text_backend_from_gfx(gfx, &app->text_backend);

  /* 2. Init Styles */
  m3_button_style_init_filled(&app->s_btn_filled);
  app->s_btn_filled.text_style.utf8_family = DEMO_FONT;

  m3_button_style_init_text(&app->s_btn_text);
  app->s_btn_text.text_style.utf8_family = DEMO_FONT;

  m3_card_style_init_elevated(&app->s_card);

  cmp_text_field_style_init(&app->s_tf);
  app->s_tf.text_style.utf8_family = DEMO_FONT;
  app->s_tf.label_style.utf8_family = DEMO_FONT;

  m3_app_bar_style_init_small(&app->s_app_bar);
  app->s_app_bar.title_style.utf8_family = DEMO_FONT;

  m3_switch_style_init(&app->s_switch);
  m3_checkbox_style_init(&app->s_checkbox);
  m3_slider_style_init(&app->s_slider);
  m3_linear_progress_style_init(&app->s_progress);
  m3_chip_style_init_filter(&app->s_chip);
  app->s_chip.text_style.utf8_family = DEMO_FONT;

  m3_snackbar_style_init(&app->s_snackbar);
  app->s_snackbar.message_style.utf8_family = DEMO_FONT;
  app->s_snackbar.action_style.utf8_family = DEMO_FONT;

  /* 3. Setup Login Screen */
  m3_card_init(&app->login_card, &app->s_card);

  cmp_text_field_init(&app->tf_user, &app->text_backend, &app->s_tf,
                      &app->allocator, "", 0);
  cmp_text_field_set_label(&app->tf_user, "Username", 8);
  cmp_text_field_set_placeholder(&app->tf_user, "Enter user", 10);

  cmp_text_field_init(&app->tf_pass, &app->text_backend, &app->s_tf,
                      &app->allocator, "", 0);
  cmp_text_field_set_label(&app->tf_pass, "Password", 8);

  m3_button_init(&app->btn_login, &app->text_backend, &app->s_btn_filled,
                 "Sign In", 7);
  m3_button_set_on_click(&app->btn_login, on_login_click, app);

  m3_snackbar_init(&app->login_snackbar, &app->text_backend, &app->s_snackbar,
                   "", 0);

  // Login Layout
  CMPLayoutStyle s_col;
  cmp_layout_style_init(&s_col);
  s_col.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  s_col.align_cross = CMP_LAYOUT_ALIGN_STRETCH;

  // Root centers the card
  CMPLayoutStyle s_root = s_col;
  s_root.align_main = CMP_LAYOUT_ALIGN_CENTER;
  s_root.align_cross = CMP_LAYOUT_ALIGN_CENTER;
  s_root.width = CMP_LAYOUT_AUTO;
  s_root.height = CMP_LAYOUT_AUTO;
  cmp_layout_node_init(&app->l_root, &s_root);

  // Card contains inputs
  CMPLayoutStyle s_card = s_col;
  s_card.padding = (CMPLayoutEdges){24, 24, 24, 24};
  s_card.width = 340; // Increased width for better hit targets
  s_card.height = CMP_LAYOUT_AUTO;
  cmp_layout_node_init(&app->l_card, &s_card);

  CMPLayoutStyle s_item;
  cmp_layout_style_init(&s_item);
  s_item.width = CMP_LAYOUT_AUTO;
  s_item.height = CMP_LAYOUT_AUTO;
  s_item.padding.bottom = 24;

  cmp_layout_node_init(&app->l_tf_user, &s_item);
  cmp_layout_node_set_measure(&app->l_tf_user, measure_adapter, &app->tf_user);

  cmp_layout_node_init(&app->l_tf_pass, &s_item);
  cmp_layout_node_set_measure(&app->l_tf_pass, measure_adapter, &app->tf_pass);

  s_item.padding.bottom = 0;
  cmp_layout_node_init(&app->l_btn, &s_item);
  cmp_layout_node_set_measure(&app->l_btn, measure_adapter, &app->btn_login);

  app->c_card[0] = &app->l_tf_user;
  app->c_card[1] = &app->l_tf_pass;
  app->c_card[2] = &app->l_btn;
  cmp_layout_node_set_children(&app->l_card, app->c_card, 3);

  app->c_root[0] = &app->l_card;
  cmp_layout_node_set_children(&app->l_root, app->c_root, 1);

  /* 4. Setup Showcase Screen */
  m3_app_bar_init(&app->show_app_bar, &app->text_backend, &app->s_app_bar,
                  "Showcase", 8);

  M3ButtonStyle fab_style;
  m3_button_style_init_fab(&fab_style);
  fab_style.text_style.utf8_family = DEMO_FONT;
  m3_button_init(&app->fab, &app->text_backend, &fab_style, "+", 1);
  m3_button_set_on_click(&app->fab, on_fab_click, app);

  CMPListStyle list_style;
  cmp_list_style_init(&list_style);
  list_style.padding = (CMPLayoutEdges){16, 88, 16, 88};
  list_style.item_extent = 80;
  list_style.spacing = 16; /* Increased spacing */
  cmp_list_view_init(&app->show_list, &list_style, &app->allocator, 100, 20);
  cmp_list_view_set_bind(&app->show_list, showcase_bind, app);

  M3ScaffoldStyle scaf_style;
  m3_scaffold_style_init(&scaf_style);
  m3_scaffold_init(
      &app->show_scaffold, &scaf_style, (CMPWidget *)&app->show_list,
      (CMPWidget *)&app->show_app_bar, NULL, (CMPWidget *)&app->fab, NULL);

  // Initialize recycled widgets
  m3_card_init(&app->r_card, &app->s_card);
  m3_card_set_on_click(&app->r_card, on_list_card_click, app);
  m3_switch_init(&app->r_switch, &app->s_switch, CMP_FALSE);
  m3_switch_set_on_change(&app->r_switch, on_switch_chg, app);
  m3_checkbox_init(&app->r_check, &app->s_checkbox, CMP_FALSE);
  m3_checkbox_set_on_change(&app->r_check, on_check_chg, app);
  m3_slider_init(&app->r_slider, &app->s_slider, 0, 100, 50);
  m3_slider_set_on_change(&app->r_slider, on_slider_chg, app);
  m3_linear_progress_init(&app->r_progress, &app->s_progress, 0.5);
  m3_chip_init(&app->r_chip, &app->text_backend, &app->s_chip, "Chip", 4);

  /* 5. Setup Detail Screen */
  m3_app_bar_init(&app->det_bar, &app->text_backend, &app->s_app_bar, "Details",
                  7);
  m3_card_init(&app->det_card, &app->s_card);

  m3_button_init(&app->btn_back, &app->text_backend, &app->s_btn_filled,
                 "Go Back", 7);
  m3_button_set_on_click(&app->btn_back, on_back_click, app);

  m3_button_init(&app->btn_action, &app->text_backend, &app->s_btn_text,
                 "Action", 6);

  M3ScaffoldStyle dscaf_style = scaf_style;
  dscaf_style.padding.top = 80;
  m3_scaffold_init(&app->det_scaffold, &dscaf_style, NULL,
                   (CMPWidget *)&app->det_bar, NULL, NULL, NULL);

  // Detail Layout inside body rect
  CMPLayoutStyle s_det_center = s_root;
  cmp_layout_node_init(&app->l_det_root, &s_det_center);

  CMPLayoutStyle s_det_card = s_card;
  s_det_card.width = 300;
  s_det_card.height = 200;
  cmp_layout_node_init(&app->l_det_card, &s_det_card);

  CMPLayoutStyle s_det_item = s_item;
  cmp_layout_node_init(&app->l_det_back, &s_det_item);
  cmp_layout_node_set_measure(&app->l_det_back, measure_adapter,
                              &app->btn_back);

  cmp_layout_node_init(&app->l_det_act, &s_det_item);
  cmp_layout_node_set_measure(&app->l_det_act, measure_adapter,
                              &app->btn_action);

  app->c_det_card[0] = &app->l_det_back;
  app->c_det_card[1] = &app->l_det_act;
  cmp_layout_node_set_children(&app->l_det_card, app->c_det_card, 2);

  app->c_det_root[0] = &app->l_det_card;
  cmp_layout_node_set_children(&app->l_det_root, app->c_det_root, 1);

  app->init_done = CMP_TRUE;
  return CMP_OK;
}

int demo_app_update(DemoApp *app, double dt) {
  if (!app->init_done)
    return CMP_OK;
  float fdt = (float)dt;
  CMPBool changed;

  if (app->current_screen == SCR_LOGIN) {
    cmp_text_field_step(&app->tf_user, fdt, &changed);
    cmp_text_field_step(&app->tf_pass, fdt, &changed);
  } else if (app->current_screen == SCR_SHOWCASE) {
    m3_scaffold_step(&app->show_scaffold, fdt, &changed);
  } else {
    m3_scaffold_step(&app->det_scaffold, fdt, &changed);
  }
  return CMP_OK;
}

int demo_app_handle_event(DemoApp *app, const CMPInputEvent *evt,
                          CMPBool *out_handled) {
  if (!app->init_done)
    return CMP_OK;
  CMPBool h = CMP_FALSE;
  CMPWidget *w = NULL;

  /* Dispatch order: children first/z-order or dispatch tree */
  if (app->current_screen == SCR_LOGIN) {
    /* Manual Tab Navigation */
    if (evt->type == CMP_INPUT_KEY_DOWN && evt->data.key.key_code == 9) {
      CMPWidget *curr = NULL;
      CMPWidget *next = NULL;
      cmp_event_dispatcher_get_focus(&app->dispatcher, &curr);

      if (curr == (CMPWidget *)&app->tf_user) {
        next = (CMPWidget *)&app->tf_pass;
      } else if (curr == (CMPWidget *)&app->tf_pass) {
        next = (CMPWidget *)&app->btn_login;
      } else {
        next = (CMPWidget *)&app->tf_user;
      }

      /* Explicitly propagate focus state to widgets so they redraw with cursors
       */
      if (curr) {
        update_widget_focus(app, curr, CMP_FALSE);
      }

      cmp_event_dispatcher_set_focus(&app->dispatcher, next);

      if (next) {
        update_widget_focus(app, next, CMP_TRUE);
      }

      h = CMP_TRUE;
    }

    /* FIX: Dispatch Keyboard/Text events to Focused widget first */
    if (!h && (evt->type == CMP_INPUT_KEY_DOWN || evt->type == CMP_INPUT_TEXT ||
               evt->type == CMP_INPUT_TEXT_UTF8)) {
      CMPWidget *curr = NULL;
      cmp_event_dispatcher_get_focus(&app->dispatcher, &curr);
      if (curr) {
        curr->vtable->event(curr, evt, &h);
      }
    }

    /* General Dispatch (Clicks / Unhandled Keys) */
    if (!h) {
      /* Manual check order for hit testing: Top-most first */
      /* Z-Order: Button (top), Pass, User, Card (bottom) */

      /* FIX: Explicit hit-testing before dispatching pointer events to prevent
       * "stealing" */
      w = (CMPWidget *)&app->btn_login;
      if (!is_pointer_event(evt) ||
          widget_contains_point(&app->btn_login.bounds, evt)) {
        w->vtable->event(w, evt, &h);
      }

      if (!h) {
        w = (CMPWidget *)&app->tf_pass;
        if (!is_pointer_event(evt) ||
            widget_contains_point(&app->tf_pass.bounds, evt)) {
          w->vtable->event(w, evt, &h);
        }
      }
      if (!h) {
        w = (CMPWidget *)&app->tf_user;
        if (!is_pointer_event(evt) ||
            widget_contains_point(&app->tf_user.bounds, evt)) {
          w->vtable->event(w, evt, &h);
        }
      }

      // Handle focus on Click
      if (h && evt->type == CMP_INPUT_POINTER_DOWN) {
        CMPWidget *prev = NULL;
        cmp_event_dispatcher_get_focus(&app->dispatcher, &prev);

        if (prev != w) {
          if (prev) {
            update_widget_focus(app, prev, CMP_FALSE);
          }
          cmp_event_dispatcher_set_focus(&app->dispatcher, w);
          if (w) {
            update_widget_focus(app, w, CMP_TRUE);
          }
        }
      }
    }
  } else if (app->current_screen == SCR_SHOWCASE) {
    w = (CMPWidget *)&app->fab;
    if (!is_pointer_event(evt) ||
        widget_contains_point(&app->fab.bounds, evt)) {
      w->vtable->event(w, evt, &h);
    }
    if (!h) {
      w = (CMPWidget *)&app->show_scaffold;
      w->vtable->event(w, evt, &h);
    }
  } else if (app->current_screen == SCR_DETAIL) {
    w = (CMPWidget *)&app->btn_back;
    if (!is_pointer_event(evt) ||
        widget_contains_point(&app->btn_back.bounds, evt)) {
      w->vtable->event(w, evt, &h);
    }
    if (!h) {
      w = (CMPWidget *)&app->btn_action;
      if (!is_pointer_event(evt) ||
          widget_contains_point(&app->btn_action.bounds, evt)) {
        w->vtable->event(w, evt, &h);
      }
    }
    if (!h) {
      w = (CMPWidget *)&app->det_scaffold;
      w->vtable->event(w, evt, &h);
    }
  }

  *out_handled = h;
  return CMP_OK;
}

int demo_app_render(DemoApp *app, CMPGfx *gfx, CMPHandle window, int width,
                    int height, float dpi) {
  if (!app->init_done)
    return CMP_OK;

  cmp_render_list_reset(&app->render_list);
  gfx->vtable->begin_frame(gfx->ctx, window, width, height, (CMPScalar)dpi);

  // Draw Material Surface Background
  gfx->vtable->clear(gfx->ctx, (CMPColor){0.98f, 0.98f, 0.98f, 1.0f});

  CMPRect screen_rect = {0, 0, (float)width, (float)height};
  CMPPaintContext pctx = {gfx, screen_rect, (CMPScalar)dpi};

  if (app->current_screen == SCR_LOGIN) {
    CMPLayoutMeasureSpec ws = {CMP_LAYOUT_MEASURE_EXACTLY, (float)width};
    CMPLayoutMeasureSpec hs = {CMP_LAYOUT_MEASURE_EXACTLY, (float)height};

    cmp_layout_compute(&app->l_root, &app->layout_dir, ws, hs);

    // Manually traverse and paint based on layout result
    // Note: Assuming cmp_layout returns absolute coordinates logic implies
    // we do NOT add the parent offset manually if the engine recursively
    // propagated it. Based on user feedback ("inputs separate from card...
    // bottom left away"), previous manual addition was incorrect.

    CMPRect r_card;
    cmp_layout_node_get_layout(&app->l_card, &r_card);
    app->login_card.widget.vtable->layout(&app->login_card, r_card);
    app->login_card.widget.vtable->paint(&app->login_card, &pctx);

    CMPRect r_child;

    cmp_layout_node_get_layout(&app->l_tf_user, &r_child);
    app->tf_user.widget.vtable->layout(&app->tf_user, r_child);
    app->tf_user.widget.vtable->paint(&app->tf_user, &pctx);

    cmp_layout_node_get_layout(&app->l_tf_pass, &r_child);
    app->tf_pass.widget.vtable->layout(&app->tf_pass, r_child);
    app->tf_pass.widget.vtable->paint(&app->tf_pass, &pctx);

    cmp_layout_node_get_layout(&app->l_btn, &r_child);
    app->btn_login.widget.vtable->layout(&app->btn_login, r_child);
    app->btn_login.widget.vtable->paint(&app->btn_login, &pctx);
  } else if (app->current_screen == SCR_SHOWCASE) {
    app->show_scaffold.widget.vtable->measure(
        &app->show_scaffold,
        (CMPMeasureSpec){CMP_MEASURE_EXACTLY, (float)width},
        (CMPMeasureSpec){CMP_MEASURE_EXACTLY, (float)height}, NULL);
    app->show_scaffold.widget.vtable->layout(&app->show_scaffold, screen_rect);
    app->show_scaffold.widget.vtable->paint(&app->show_scaffold, &pctx);
  } else if (app->current_screen == SCR_DETAIL) {
    // 1. Paint Scaffold (Bar)
    app->det_scaffold.widget.vtable->measure(
        &app->det_scaffold, (CMPMeasureSpec){CMP_MEASURE_EXACTLY, (float)width},
        (CMPMeasureSpec){CMP_MEASURE_EXACTLY, (float)height}, NULL);
    app->det_scaffold.widget.vtable->layout(&app->det_scaffold, screen_rect);
    app->det_scaffold.widget.vtable->paint(&app->det_scaffold, &pctx);

    // 2. Paint Content (Manually, using scaffold safe area as offset)
    CMPRect body = app->det_scaffold.body_bounds;
    CMPLayoutMeasureSpec ws = {CMP_LAYOUT_MEASURE_EXACTLY, body.width};
    CMPLayoutMeasureSpec hs = {CMP_LAYOUT_MEASURE_EXACTLY, body.height};

    cmp_layout_compute(&app->l_det_root, &app->layout_dir, ws, hs);

    CMPRect r;
    cmp_layout_node_get_layout(&app->l_det_card, &r);
    r.x += body.x;
    r.y += body.y;
    app->det_card.widget.vtable->layout(&app->det_card, r);
    app->det_card.widget.vtable->paint(&app->det_card, &pctx);

    // Map button positions relative to card layout logic if needed
    // (Assuming detail uses standard absolute flow here too now)
    CMPRect r_btn;

    cmp_layout_node_get_layout(&app->l_det_back, &r_btn);
    // Detail screen manual layout was relative to card body logic previously?
    // If we assume same engine, we trust absolute coords, but we must add
    // SCAFFOLD body offset
    r_btn.x += body.x;
    r_btn.y += body.y;
    app->btn_back.widget.vtable->layout(&app->btn_back, r_btn);
    app->btn_back.widget.vtable->paint(&app->btn_back, &pctx);

    cmp_layout_node_get_layout(&app->l_det_act, &r_btn);
    r_btn.x += body.x;
    r_btn.y += body.y;
    app->btn_action.widget.vtable->layout(&app->btn_action, r_btn);
    app->btn_action.widget.vtable->paint(&app->btn_action, &pctx);
  }

  // Flush to GPU
  cmp_render_list_execute(&app->render_list, gfx);
  gfx->vtable->end_frame(gfx->ctx, window);
  return CMP_OK;
}