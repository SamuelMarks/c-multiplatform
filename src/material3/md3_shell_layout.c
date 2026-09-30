/**
 * @file md3_shell_layout.c
 * @brief Material 3 & Expressive Shell, Layout, and Workflow Components
 * implementation.
 */

/* clang-format off */
#include "material3/md3_shell_layout.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Global flag to simulate failures in mocked dependencies. */
int g_md3_shell_layout_mock_fail = 0;

/**
 * @brief Mock for ui_progress_base_create.
 * @param out Output progress pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_progress_base_create(struct ui_progress_base **out) {
  if (g_md3_shell_layout_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_progress_base_create(out);
}

/**
 * @brief Mock for ui_progress_base_set_indeterminate.
 * @param b Progress pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_progress_base_set_indeterminate(struct ui_progress_base *b) {
  if (g_md3_shell_layout_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_progress_base_set_indeterminate(b);
}

/**
 * @brief Mock for ui_progress_base_destroy.
 * @param b Progress pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_progress_base_destroy(struct ui_progress_base *b) {
  if (g_md3_shell_layout_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_progress_base_destroy(b);
}

/**
 * @brief Mock for ui_stepper_base_create.
 * @param out Output stepper pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_stepper_base_create(struct ui_stepper_base **out) {
  if (g_md3_shell_layout_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_stepper_base_create(out);
}

/**
 * @brief Mock for ui_stepper_base_set_mode.
 * @param b Stepper pointer.
 * @param m Mode.
 * @return ui_error_t result code.
 */
static ui_error_t mock_stepper_base_set_mode(struct ui_stepper_base *b,
                                             enum ui_stepper_mode m) {
  if (g_md3_shell_layout_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_stepper_base_set_mode(b, m);
}

/**
 * @brief Mock for ui_stepper_base_destroy.
 * @param b Stepper pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_stepper_base_destroy(struct ui_stepper_base *b) {
  if (g_md3_shell_layout_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_stepper_base_destroy(b);
}

/**
 * @brief Mock for ui_dom_node_create.
 * @param t Node type.
 * @param out Output node pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_dom_node_create(enum ui_dom_node_type t,
                                       struct ui_dom_node **out) {
  if (g_md3_shell_layout_mock_fail == 7) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_md3_shell_layout_mock_fail == 32) {
    static int calls = 0;
    if (++calls == 2) {
      calls = 0;
      g_md3_shell_layout_mock_fail = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_create(t, out);
}

/**
 * @brief Mock for ui_stepper_base_add_step.
 * @param b Stepper pointer.
 * @param id Step ID.
 * @param h Header node.
 * @param c Content node.
 * @return ui_error_t result code.
 */
static ui_error_t mock_stepper_base_add_step(struct ui_stepper_base *b,
                                             const char *id,
                                             struct ui_dom_node *h,
                                             struct ui_dom_node *c) {
  if (g_md3_shell_layout_mock_fail == 8) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_stepper_base_add_step(b, id, h, c);
}

/**
 * @brief Mock for ui_stepper_base_set_step_state.
 * @param b Stepper pointer.
 * @param idx Step index.
 * @param s State.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_stepper_base_set_step_state(struct ui_stepper_base *b, int idx,
                                 enum ui_stepper_step_state s) {
  if (g_md3_shell_layout_mock_fail == 9) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_stepper_base_set_step_state(b, idx, s);
}

/**
 * @brief Mock for ui_stepper_base_set_active_index.
 * @param b Stepper pointer.
 * @param idx Active index.
 * @return ui_error_t result code.
 */
static ui_error_t mock_stepper_base_set_active_index(struct ui_stepper_base *b,
                                                     int idx) {
  if (g_md3_shell_layout_mock_fail == 10) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_stepper_base_set_active_index(b, idx);
}

/**
 * @brief Mock for ui_stepper_base_set_validate_hook.
 * @param b Stepper pointer.
 * @param h Hook function.
 * @param ud User data.
 * @return ui_error_t result code.
 */
static ui_error_t mock_stepper_base_set_validate_hook(struct ui_stepper_base *b,
                                                      ui_stepper_validate_t h,
                                                      void *ud) {
  if (g_md3_shell_layout_mock_fail == 11) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_stepper_base_set_validate_hook(b, h, ud);
}

/**
 * @brief Mock for ui_scaffold_base_create.
 * @param out Output scaffold pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_scaffold_base_create(struct ui_scaffold_base **out) {
  if (g_md3_shell_layout_mock_fail == 12) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_scaffold_base_create(out);
}

/**
 * @brief Mock for ui_component_destroy.
 * @param c Component pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_component_destroy(struct ui_component *c) {
  if (g_md3_shell_layout_mock_fail == 13) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(c);
}

/**
 * @brief Mock for ui_scaffold_base_set_top_bar.
 * @param s Scaffold pointer.
 * @param tb Top bar component.
 * @return ui_error_t result code.
 */
static ui_error_t mock_scaffold_base_set_top_bar(struct ui_scaffold_base *s,
                                                 struct ui_component *tb) {
  if (g_md3_shell_layout_mock_fail == 14) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_scaffold_base_set_top_bar(s, tb);
}

/**
 * @brief Mock for ui_scaffold_base_set_main_content.
 * @param s Scaffold pointer.
 * @param c Main content component.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_scaffold_base_set_main_content(struct ui_scaffold_base *s,
                                    struct ui_component *c) {
  if (g_md3_shell_layout_mock_fail == 15) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_scaffold_base_set_main_content(s, c);
}

/**
 * @brief Mock for ui_arena_create.
 * @param sz Initial size.
 * @param out Output arena pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_arena_create(size_t sz, struct ui_arena **out) {
  if (g_md3_shell_layout_mock_fail == 16) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_arena_create(sz, out);
}

/**
 * @brief Mock for ui_canonical_layout_base_create.
 * @param a Arena pointer.
 * @param cfg Configuration.
 * @param out Output layout pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_canonical_layout_base_create(struct ui_arena *a,
                                  const struct ui_canonical_layout_config *cfg,
                                  struct ui_canonical_layout_base **out) {
  if (g_md3_shell_layout_mock_fail == 17) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_canonical_layout_base_create(a, cfg, out);
}

/**
 * @brief Mock for ui_canonical_layout_base_destroy.
 * @param b Layout pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_canonical_layout_base_destroy(struct ui_canonical_layout_base *b) {
  if (g_md3_shell_layout_mock_fail == 18) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_canonical_layout_base_destroy(b);
}

/**
 * @brief Mock for ui_arena_destroy.
 * @param a Arena pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_arena_destroy(struct ui_arena *a) {
  if (g_md3_shell_layout_mock_fail == 19) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_arena_destroy(a);
}

/**
 * @brief Mock for ui_canonical_layout_base_set_size_class.
 * @param b Layout pointer.
 * @param sc Size class.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_canonical_layout_base_set_size_class(struct ui_canonical_layout_base *b,
                                          enum ui_window_size_class sc) {
  if (g_md3_shell_layout_mock_fail == 20) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_canonical_layout_base_set_size_class(b, sc);
}

/**
 * @brief Mock for ui_canonical_layout_base_set_body.
 * @param b Layout pointer.
 * @param c Component pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_canonical_layout_base_set_body(struct ui_canonical_layout_base *b,
                                    struct ui_component *c) {
  if (g_md3_shell_layout_mock_fail == 21) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_canonical_layout_base_set_body(b, c);
}

/**
 * @brief Mock for ui_canonical_layout_base_set_leading_pane.
 * @param b Layout pointer.
 * @param c Component pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_canonical_layout_base_set_leading_pane(struct ui_canonical_layout_base *b,
                                            struct ui_component *c) {
  if (g_md3_shell_layout_mock_fail == 22) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_canonical_layout_base_set_leading_pane(b, c);
}

/**
 * @brief Mock for ui_canonical_layout_base_set_trailing_pane.
 * @param b Layout pointer.
 * @param c Component pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_canonical_layout_base_set_trailing_pane(struct ui_canonical_layout_base *b,
                                             struct ui_component *c) {
  if (g_md3_shell_layout_mock_fail == 23) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_canonical_layout_base_set_trailing_pane(b, c);
}

/**
 * @brief Mock for ui_breadcrumbs_base_create.
 * @param r Router pointer.
 * @param out Output breadcrumbs pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_breadcrumbs_base_create(struct ui_router *r,
                             struct ui_breadcrumbs_base **out) {
  if (g_md3_shell_layout_mock_fail == 24) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_breadcrumbs_base_create(r, out);
}

/**
 * @brief Mock for ui_breadcrumbs_base_destroy.
 * @param b Breadcrumbs pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_breadcrumbs_base_destroy(struct ui_breadcrumbs_base *b) {
  if (g_md3_shell_layout_mock_fail == 25) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_breadcrumbs_base_destroy(b);
}

/**
 * @brief Mock for ui_breadcrumbs_base_set_path.
 * @param b Breadcrumbs pointer.
 * @param p Path string.
 * @return ui_error_t result code.
 */
static ui_error_t mock_breadcrumbs_base_set_path(struct ui_breadcrumbs_base *b,
                                                 const char *p) {
  if (g_md3_shell_layout_mock_fail == 26) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_breadcrumbs_base_set_path(b, p);
}

/**
 * @brief Mock for ui_breadcrumbs_base_simulate_click.
 * @param b Breadcrumbs pointer.
 * @param idx Index.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_breadcrumbs_base_simulate_click(struct ui_breadcrumbs_base *b,
                                     size_t idx) {
  if (g_md3_shell_layout_mock_fail == 27) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_breadcrumbs_base_simulate_click(b, idx);
}

/**
 * @brief Mock for ui_page_control_base_create.
 * @param out Output page control pointer.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_page_control_base_create(struct ui_page_control_base **out) {
  ui_error_t rc;
  if (g_md3_shell_layout_mock_fail == 28) {
    return UI_ERROR_UNKNOWN;
  }
  rc = ui_page_control_base_create(out);
  if (g_md3_shell_layout_mock_fail == 33 ||
      g_md3_shell_layout_mock_fail == 34) {
    ui_dom_node_destroy((*out)->base.shadow_root);
    (*out)->base.shadow_root = NULL;
  }
  return rc;
}

/**
 * @brief Mock for ui_page_control_base_set_number_of_pages.
 * @param b Page control pointer.
 * @param n Number of pages.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_page_control_base_set_number_of_pages(struct ui_page_control_base *b,
                                           int n) {
  if (g_md3_shell_layout_mock_fail == 29 ||
      g_md3_shell_layout_mock_fail == 33) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_md3_shell_layout_mock_fail == 34) {
    return UI_ERROR_NONE;
  }
  return ui_page_control_base_set_number_of_pages(b, n);
}

/**
 * @brief Mock for ui_page_control_base_set_current_page.
 * @param b Page control pointer.
 * @param p Current page index.
 * @return ui_error_t result code.
 */
static ui_error_t
mock_page_control_base_set_current_page(struct ui_page_control_base *b, int p) {
  if (g_md3_shell_layout_mock_fail == 30 ||
      g_md3_shell_layout_mock_fail == 34) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_page_control_base_set_current_page(b, p);
}

/**
 * @brief Mock for ui_dom_node_destroy.
 * @param n Node pointer.
 * @return ui_error_t result code.
 */
static ui_error_t mock_dom_node_destroy(struct ui_dom_node *n) {
  if (g_md3_shell_layout_mock_fail == 31) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_destroy(n);
}

#undef ui_progress_base_create
#define ui_progress_base_create mock_progress_base_create
#undef ui_progress_base_set_indeterminate
#define ui_progress_base_set_indeterminate mock_progress_base_set_indeterminate
#undef ui_progress_base_destroy
#define ui_progress_base_destroy mock_progress_base_destroy
#undef ui_stepper_base_create
#define ui_stepper_base_create mock_stepper_base_create
#undef ui_stepper_base_set_mode
#define ui_stepper_base_set_mode mock_stepper_base_set_mode
#undef ui_stepper_base_destroy
#define ui_stepper_base_destroy mock_stepper_base_destroy
#undef ui_dom_node_create
#define ui_dom_node_create mock_dom_node_create
#undef ui_dom_node_destroy
#define ui_dom_node_destroy mock_dom_node_destroy
#undef ui_stepper_base_add_step
#define ui_stepper_base_add_step mock_stepper_base_add_step
#undef ui_stepper_base_set_step_state
#define ui_stepper_base_set_step_state mock_stepper_base_set_step_state
#undef ui_stepper_base_set_active_index
#define ui_stepper_base_set_active_index mock_stepper_base_set_active_index
#undef ui_stepper_base_set_validate_hook
#define ui_stepper_base_set_validate_hook mock_stepper_base_set_validate_hook
#undef ui_scaffold_base_create
#define ui_scaffold_base_create mock_scaffold_base_create
#undef ui_component_destroy
#define ui_component_destroy mock_component_destroy
#undef ui_scaffold_base_set_top_bar
#define ui_scaffold_base_set_top_bar mock_scaffold_base_set_top_bar
#undef ui_scaffold_base_set_main_content
#define ui_scaffold_base_set_main_content mock_scaffold_base_set_main_content
#undef ui_arena_create
#define ui_arena_create mock_arena_create
#undef ui_canonical_layout_base_create
#define ui_canonical_layout_base_create mock_canonical_layout_base_create
#undef ui_canonical_layout_base_destroy
#define ui_canonical_layout_base_destroy mock_canonical_layout_base_destroy
#undef ui_arena_destroy
#define ui_arena_destroy mock_arena_destroy
#undef ui_canonical_layout_base_set_size_class
#define ui_canonical_layout_base_set_size_class                                \
  mock_canonical_layout_base_set_size_class
#undef ui_canonical_layout_base_set_body
#define ui_canonical_layout_base_set_body mock_canonical_layout_base_set_body
#undef ui_canonical_layout_base_set_leading_pane
#define ui_canonical_layout_base_set_leading_pane                              \
  mock_canonical_layout_base_set_leading_pane
#undef ui_canonical_layout_base_set_trailing_pane
#define ui_canonical_layout_base_set_trailing_pane                             \
  mock_canonical_layout_base_set_trailing_pane
#undef ui_breadcrumbs_base_create
#define ui_breadcrumbs_base_create mock_breadcrumbs_base_create
#undef ui_breadcrumbs_base_destroy
#define ui_breadcrumbs_base_destroy mock_breadcrumbs_base_destroy
#undef ui_breadcrumbs_base_set_path
#define ui_breadcrumbs_base_set_path mock_breadcrumbs_base_set_path
#undef ui_breadcrumbs_base_simulate_click
#define ui_breadcrumbs_base_simulate_click mock_breadcrumbs_base_simulate_click
#undef ui_page_control_base_create
#define ui_page_control_base_create mock_page_control_base_create
#undef ui_page_control_base_set_number_of_pages
#define ui_page_control_base_set_number_of_pages                               \
  mock_page_control_base_set_number_of_pages
#undef ui_page_control_base_set_current_page
#define ui_page_control_base_set_current_page                                  \
  mock_page_control_base_set_current_page
#endif

/* ========================================================================= */
/* Loading Indicator Implementation                                          */
/* ========================================================================= */

ui_error_t
md3_loading_indicator_create(struct ui_engine *engine,
                             enum md3_loading_indicator_style style,
                             struct md3_loading_indicator **out_indicator) {
  struct md3_loading_indicator *ind;
  ui_error_t rc;

  if (!engine || !out_indicator ||
      (style != MD3_LOADING_INDICATOR_CONTAINED &&
       style != MD3_LOADING_INDICATOR_UNCONTAINED)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ind = (struct md3_loading_indicator *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_loading_indicator));
  if (!ind) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ind, 0, sizeof(struct md3_loading_indicator));
  ind->style = style;
  ind->size = MD3_LOADING_INDICATOR_SIZE_STANDARD;
  ind->progress = 0.0f;
  ind->shape_stage = 0;
  ind->is_reduced_motion = 0;
  ind->pulse_opacity = 1.0f;

  rc = ui_progress_base_create(&ind->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ind);
    return rc;
  }

  rc = ui_progress_base_set_indeterminate(ind->base);
  if (rc != UI_ERROR_NONE) {
    ui_progress_base_destroy(ind->base);
    C_MULTIPLATFORM_FREE(ind);
    return rc;
  }

  *out_indicator = ind;
  return UI_ERROR_NONE;
}

ui_error_t
md3_loading_indicator_destroy(struct md3_loading_indicator *indicator) {
  ui_error_t rc;

  if (!indicator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (indicator->base) {
    rc = ui_progress_base_destroy(indicator->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(indicator);
  return UI_ERROR_NONE;
}

ui_error_t
md3_loading_indicator_set_size(struct md3_loading_indicator *indicator,
                               enum md3_loading_indicator_size size) {
  if (!indicator || (size != MD3_LOADING_INDICATOR_SIZE_COMPACT &&
                     size != MD3_LOADING_INDICATOR_SIZE_STANDARD &&
                     size != MD3_LOADING_INDICATOR_SIZE_LARGE)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  indicator->size = size;
  return UI_ERROR_NONE;
}

ui_error_t md3_loading_indicator_set_reduced_motion(
    struct md3_loading_indicator *indicator, int enabled) {
  if (!indicator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  indicator->is_reduced_motion = (enabled != 0);
  return UI_ERROR_NONE;
}

ui_error_t md3_loading_indicator_update(struct md3_loading_indicator *indicator,
                                        float elapsed_s) {
  if (!indicator || elapsed_s < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (indicator->is_reduced_motion) {
    indicator->pulse_opacity += elapsed_s * 2.0f;
    if (indicator->pulse_opacity > 1.0f) {
      indicator->pulse_opacity = 0.3f;
    }
  } else {
    indicator->progress += elapsed_s * 1.5f;
    while (indicator->progress >= 1.0f) {
      indicator->progress -= 1.0f;
      indicator->shape_stage = (indicator->shape_stage + 1) % 5;
    }
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Stepper Implementation                                                    */
/* ========================================================================= */

ui_error_t md3_stepper_create(struct ui_engine *engine,
                              enum ui_stepper_mode mode,
                              enum md3_stepper_orientation orientation,
                              struct md3_stepper **out_stepper) {
  struct md3_stepper *stp;
  ui_error_t rc;

  if (!engine || !out_stepper ||
      (mode != UI_STEPPER_MODE_LINEAR && mode != UI_STEPPER_MODE_NON_LINEAR) ||
      (orientation != MD3_STEPPER_HORIZONTAL &&
       orientation != MD3_STEPPER_VERTICAL)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stp =
      (struct md3_stepper *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_stepper));
  if (!stp) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(stp, 0, sizeof(struct md3_stepper));
  stp->mode = mode;
  stp->orientation = orientation;
  stp->active_index = 0;
  stp->step_count = 0;

  rc = ui_stepper_base_create(&stp->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(stp);
    return rc;
  }

  rc = ui_stepper_base_set_mode(stp->base, mode);
  if (rc != UI_ERROR_NONE) {
    ui_stepper_base_destroy(stp->base);
    C_MULTIPLATFORM_FREE(stp);
    return rc;
  }

  *out_stepper = stp;
  return UI_ERROR_NONE;
}

ui_error_t md3_stepper_destroy(struct md3_stepper *stepper) {
  ui_error_t rc;

  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->base) {
    rc = ui_stepper_base_destroy(stepper->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(stepper);
  return UI_ERROR_NONE;
}

ui_error_t md3_stepper_add_step(struct md3_stepper *stepper,
                                const char *step_id, const char *title,
                                const char *subtitle) {
  struct md3_step *step;
  struct ui_dom_node *hdr_node;
  struct ui_dom_node *cnt_node;
  ui_error_t rc;

  if (!stepper || !step_id || !title || stepper->step_count >= 32) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  step = &stepper->steps[stepper->step_count];
  memset(step, 0, sizeof(struct md3_step));

#if defined(_MSC_VER)
  strncpy_s(step->step_id, sizeof(step->step_id), step_id, _TRUNCATE);
  strncpy_s(step->title, sizeof(step->title), title, _TRUNCATE);
  if (subtitle) {
    strncpy_s(step->subtitle, sizeof(step->subtitle), subtitle, _TRUNCATE);
  }
#else
  strncpy(step->step_id, step_id, sizeof(step->step_id) - 1);
  step->step_id[sizeof(step->step_id) - 1] = '\0';
  strncpy(step->title, title, sizeof(step->title) - 1);
  step->title[sizeof(step->title) - 1] = '\0';
  if (subtitle) {
    strncpy(step->subtitle, subtitle, sizeof(step->subtitle) - 1);
    step->subtitle[sizeof(step->subtitle) - 1] = '\0';
  }
#endif

  step->state = (stepper->step_count == 0) ? UI_STEPPER_STEP_STATE_ACTIVE
                                           : UI_STEPPER_STEP_STATE_DEFAULT;
  step->is_editable = 1;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &hdr_node);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cnt_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(hdr_node);
    return rc;
  }

  rc = ui_stepper_base_add_step(stepper->base, step_id, hdr_node, cnt_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(cnt_node);
    ui_dom_node_destroy(hdr_node);
    return rc;
  }

  stepper->step_count++;
  return UI_ERROR_NONE;
}

ui_error_t md3_stepper_set_step_state(struct md3_stepper *stepper,
                                      int step_index,
                                      enum ui_stepper_step_state state) {
  ui_error_t rc;

  if (!stepper || step_index < 0 || (size_t)step_index >= stepper->step_count ||
      (state != UI_STEPPER_STEP_STATE_DEFAULT &&
       state != UI_STEPPER_STEP_STATE_ACTIVE &&
       state != UI_STEPPER_STEP_STATE_COMPLETED &&
       state != UI_STEPPER_STEP_STATE_ERROR)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper->steps[step_index].state = state;
  rc = ui_stepper_base_set_step_state(stepper->base, step_index, state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_stepper_get_step_state(const struct md3_stepper *stepper,
                                      int step_index,
                                      enum ui_stepper_step_state *out_state) {
  if (!stepper || !out_state || step_index < 0 ||
      (size_t)step_index >= stepper->step_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = stepper->steps[step_index].state;
  return UI_ERROR_NONE;
}

ui_error_t md3_stepper_set_active_index(struct md3_stepper *stepper,
                                        int index) {
  int i;
  ui_error_t rc;

  if (!stepper || index < 0 || (size_t)index >= stepper->step_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->mode == UI_STEPPER_MODE_LINEAR &&
      index > stepper->active_index) {
    for (i = stepper->active_index; i < index; i++) {
      if (stepper->validate_hook) {
        int valid = stepper->validate_hook(stepper->base, i,
                                           stepper->validate_user_data);
        if (!valid) {
          stepper->steps[i].state = UI_STEPPER_STEP_STATE_ERROR;
          return UI_ERROR_INVALID_ARGUMENT;
        }
      }
      stepper->steps[i].state = UI_STEPPER_STEP_STATE_COMPLETED;
    }
  }

  rc = ui_stepper_base_set_active_index(stepper->base, index);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (stepper->active_index != index &&
      stepper->steps[stepper->active_index].state ==
          UI_STEPPER_STEP_STATE_ACTIVE) {
    stepper->steps[stepper->active_index].state =
        UI_STEPPER_STEP_STATE_COMPLETED;
  }

  stepper->active_index = index;
  stepper->steps[index].state = UI_STEPPER_STEP_STATE_ACTIVE;

  return UI_ERROR_NONE;
}

ui_error_t md3_stepper_get_active_index(const struct md3_stepper *stepper,
                                        int *out_index) {
  if (!stepper || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = stepper->active_index;
  return UI_ERROR_NONE;
}

ui_error_t md3_stepper_next_step(struct md3_stepper *stepper) {
  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((size_t)(stepper->active_index + 1) >= stepper->step_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return md3_stepper_set_active_index(stepper, stepper->active_index + 1);
}

ui_error_t md3_stepper_prev_step(struct md3_stepper *stepper) {
  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->active_index <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return md3_stepper_set_active_index(stepper, stepper->active_index - 1);
}

ui_error_t md3_stepper_set_validate_hook(struct md3_stepper *stepper,
                                         ui_stepper_validate_t hook,
                                         void *user_data) {
  ui_error_t rc;

  if (!stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper->validate_hook = hook;
  stepper->validate_user_data = user_data;

  rc = ui_stepper_base_set_validate_hook(stepper->base, hook, user_data);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_stepper_set_editable(struct md3_stepper *stepper, int step_index,
                                    int editable) {
  if (!stepper || step_index < 0 || (size_t)step_index >= stepper->step_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper->steps[step_index].is_editable = (editable != 0);
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Scaffold Implementation                                                   */
/* ========================================================================= */

ui_error_t md3_scaffold_create(struct ui_engine *engine,
                               struct md3_scaffold **out_scaffold) {
  struct md3_scaffold *scf;
  ui_error_t rc;

  if (!engine || !out_scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scf = (struct md3_scaffold *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_scaffold));
  if (!scf) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(scf, 0, sizeof(struct md3_scaffold));

  rc = ui_scaffold_base_create(&scf->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(scf);
    return rc;
  }

  *out_scaffold = scf;
  return UI_ERROR_NONE;
}

ui_error_t md3_scaffold_destroy(struct md3_scaffold *scaffold) {
  ui_error_t rc;

  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (scaffold->base) {
    rc = ui_component_destroy((struct ui_component *)scaffold->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(scaffold);
  return UI_ERROR_NONE;
}

ui_error_t md3_scaffold_set_top_bar(struct md3_scaffold *scaffold,
                                    struct ui_component *top_bar) {
  ui_error_t rc;

  if (!scaffold || !top_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!top_bar->shadow_root) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &top_bar->shadow_root);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  scaffold->top_bar = top_bar;
  rc = ui_scaffold_base_set_top_bar(scaffold->base, top_bar);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_scaffold_set_bottom_bar(struct md3_scaffold *scaffold,
                                       struct ui_component *bottom_bar) {
  if (!scaffold || !bottom_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->bottom_bar = bottom_bar;
  return UI_ERROR_NONE;
}

ui_error_t md3_scaffold_set_side_nav(struct md3_scaffold *scaffold,
                                     struct ui_component *side_nav) {
  if (!scaffold || !side_nav) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->side_nav = side_nav;
  return UI_ERROR_NONE;
}

ui_error_t md3_scaffold_set_main_content(struct md3_scaffold *scaffold,
                                         struct ui_component *content) {
  ui_error_t rc;

  if (!scaffold || !content) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!content->shadow_root) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content->shadow_root);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  scaffold->main_content = content;
  rc = ui_scaffold_base_set_main_content(scaffold->base, content);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_scaffold_set_fab(struct md3_scaffold *scaffold,
                                struct ui_component *fab, int cradle_docked) {
  if (!scaffold || !fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->fab = fab;
  scaffold->fab_cradle_docked = (cradle_docked != 0);
  return UI_ERROR_NONE;
}

ui_error_t md3_scaffold_set_safe_area(struct md3_scaffold *scaffold, float top,
                                      float bottom, float left, float right) {
  if (!scaffold || top < 0.0f || bottom < 0.0f || left < 0.0f || right < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->safe_area_top = top;
  scaffold->safe_area_bottom = bottom;
  scaffold->safe_area_left = left;
  scaffold->safe_area_right = right;
  return UI_ERROR_NONE;
}

ui_error_t md3_scaffold_on_scroll(struct md3_scaffold *scaffold,
                                  float scroll_offset_y) {
  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->scroll_offset_y = scroll_offset_y;
  if (scroll_offset_y > 4.0f) {
    scaffold->top_bar_elevation = 2.0f;
    scaffold->bottom_bar_elevation = 2.0f;
  } else {
    scaffold->top_bar_elevation = 0.0f;
    scaffold->bottom_bar_elevation = 0.0f;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Canonical Layout Implementation                                           */
/* ========================================================================= */

ui_error_t
md3_canonical_layout_create(struct ui_engine *engine,
                            enum md3_canonical_layout_type type,
                            struct md3_canonical_layout **out_layout) {
  struct md3_canonical_layout *lay;
  struct ui_canonical_layout_config config;
  ui_error_t rc;

  if (!engine || !out_layout ||
      (type != MD3_CANONICAL_LAYOUT_LIST_DETAIL &&
       type != MD3_CANONICAL_LAYOUT_SUPPORTING_PANE &&
       type != MD3_CANONICAL_LAYOUT_FEED)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  lay = (struct md3_canonical_layout *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_canonical_layout));
  if (!lay) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(lay, 0, sizeof(struct md3_canonical_layout));
  lay->type = type;
  lay->size_class = UI_WINDOW_SIZE_CLASS_MEDIUM;
  lay->split_ratio =
      (type == MD3_CANONICAL_LAYOUT_SUPPORTING_PANE) ? 0.35f : 0.40f;
  lay->feed_columns = (type == MD3_CANONICAL_LAYOUT_FEED) ? 2 : 1;

  rc = ui_arena_create(4096, &lay->arena);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(lay);
    return rc;
  }

  memset(&config, 0, sizeof(config));
  config.initial_size_class = UI_WINDOW_SIZE_CLASS_MEDIUM;
  config.has_leading_pane = UI_TRUE;
  config.has_trailing_pane =
      (type == MD3_CANONICAL_LAYOUT_SUPPORTING_PANE) ? UI_TRUE : UI_FALSE;
  config.has_bottom_bar = UI_FALSE;

  rc = ui_canonical_layout_base_create(lay->arena, &config, &lay->base);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(lay->arena);
    C_MULTIPLATFORM_FREE(lay);
    return rc;
  }

  *out_layout = lay;
  return UI_ERROR_NONE;
}

ui_error_t md3_canonical_layout_destroy(struct md3_canonical_layout *layout) {
  ui_error_t rc;

  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (layout->base) {
    rc = ui_canonical_layout_base_destroy(layout->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (layout->arena) {
    rc = ui_arena_destroy(layout->arena);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(layout);
  return UI_ERROR_NONE;
}

ui_error_t
md3_canonical_layout_set_size_class(struct md3_canonical_layout *layout,
                                    enum ui_window_size_class size_class) {
  ui_error_t rc;

  if (!layout || (size_class != UI_WINDOW_SIZE_CLASS_COMPACT &&
                  size_class != UI_WINDOW_SIZE_CLASS_MEDIUM &&
                  size_class != UI_WINDOW_SIZE_CLASS_EXPANDED)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout->size_class = size_class;
  if (layout->type == MD3_CANONICAL_LAYOUT_FEED) {
    if (size_class == UI_WINDOW_SIZE_CLASS_COMPACT) {
      layout->feed_columns = 1;
    } else if (size_class == UI_WINDOW_SIZE_CLASS_MEDIUM) {
      layout->feed_columns = 2;
    } else {
      layout->feed_columns = 3;
    }
  }

  rc = ui_canonical_layout_base_set_size_class(layout->base, size_class);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_canonical_layout_get_size_class(const struct md3_canonical_layout *layout,
                                    enum ui_window_size_class *out_size_class) {
  if (!layout || !out_size_class) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_size_class = layout->size_class;
  return UI_ERROR_NONE;
}

ui_error_t md3_canonical_layout_set_body(struct md3_canonical_layout *layout,
                                         struct ui_component *body) {
  ui_error_t rc;

  if (!layout || !body) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout->body = body;
  rc = ui_canonical_layout_base_set_body(layout->base, body);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_canonical_layout_set_leading_pane(struct md3_canonical_layout *layout,
                                      struct ui_component *leading) {
  ui_error_t rc;

  if (!layout || !leading) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout->leading_pane = leading;
  rc = ui_canonical_layout_base_set_leading_pane(layout->base, leading);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_canonical_layout_set_trailing_pane(struct md3_canonical_layout *layout,
                                       struct ui_component *trailing) {
  ui_error_t rc;

  if (!layout || !trailing) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout->trailing_pane = trailing;
  rc = ui_canonical_layout_base_set_trailing_pane(layout->base, trailing);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_canonical_layout_set_split_ratio(struct md3_canonical_layout *layout,
                                     float ratio) {
  if (!layout || ratio < 0.1f || ratio > 0.9f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout->split_ratio = ratio;
  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Breadcrumbs Implementation                                                */
/* ========================================================================= */

ui_error_t md3_breadcrumbs_create(struct ui_engine *engine,
                                  struct md3_breadcrumbs **out_breadcrumbs) {
  struct md3_breadcrumbs *bc;
  ui_error_t rc;

  if (!engine || !out_breadcrumbs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bc = (struct md3_breadcrumbs *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_breadcrumbs));
  if (!bc) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(bc, 0, sizeof(struct md3_breadcrumbs));
  bc->max_items = 8;
  bc->item_count = 0;
  bc->active_index = 0;

#if defined(_MSC_VER)
  strncpy_s(bc->separator, sizeof(bc->separator), "/", _TRUNCATE);
#else
  strncpy(bc->separator, "/", sizeof(bc->separator) - 1);
  bc->separator[sizeof(bc->separator) - 1] = '\0';
#endif

  rc = ui_breadcrumbs_base_create(NULL, &bc->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(bc);
    return rc;
  }

  *out_breadcrumbs = bc;
  return UI_ERROR_NONE;
}

ui_error_t md3_breadcrumbs_destroy(struct md3_breadcrumbs *breadcrumbs) {
  ui_error_t rc;

  if (!breadcrumbs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (breadcrumbs->base) {
    rc = ui_breadcrumbs_base_destroy(breadcrumbs->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(breadcrumbs);
  return UI_ERROR_NONE;
}

ui_error_t md3_breadcrumbs_add_item(struct md3_breadcrumbs *breadcrumbs,
                                    const char *label, const char *href) {
  struct md3_breadcrumb_item *item;
  char full_path[512];
  size_t i;
  ui_error_t rc;

  if (!breadcrumbs || !label || breadcrumbs->item_count >= 32) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = &breadcrumbs->items[breadcrumbs->item_count];
  memset(item, 0, sizeof(struct md3_breadcrumb_item));

#if defined(_MSC_VER)
  strncpy_s(item->label, sizeof(item->label), label, _TRUNCATE);
  if (href) {
    strncpy_s(item->href, sizeof(item->href), href, _TRUNCATE);
  }
#else
  strncpy(item->label, label, sizeof(item->label) - 1);
  item->label[sizeof(item->label) - 1] = '\0';
  if (href) {
    strncpy(item->href, href, sizeof(item->href) - 1);
    item->href[sizeof(item->href) - 1] = '\0';
  }
#endif

  breadcrumbs->active_index = breadcrumbs->item_count;
  breadcrumbs->item_count++;

  full_path[0] = '\0';
  for (i = 0; i < breadcrumbs->item_count; i++) {
#if defined(_MSC_VER)
    strcat_s(full_path, sizeof(full_path), "/");
    strcat_s(full_path, sizeof(full_path), breadcrumbs->items[i].label);
#else
    strncat(full_path, "/", sizeof(full_path) - strlen(full_path) - 1);
    strncat(full_path, breadcrumbs->items[i].label,
            sizeof(full_path) - strlen(full_path) - 1);
#endif
  }

  rc = ui_breadcrumbs_base_set_path(breadcrumbs->base, full_path);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_breadcrumbs_set_separator(struct md3_breadcrumbs *breadcrumbs,
                                         const char *separator) {
  if (!breadcrumbs || !separator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(breadcrumbs->separator, sizeof(breadcrumbs->separator), separator,
            _TRUNCATE);
#else
  strncpy(breadcrumbs->separator, separator,
          sizeof(breadcrumbs->separator) - 1);
  breadcrumbs->separator[sizeof(breadcrumbs->separator) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t md3_breadcrumbs_set_max_items(struct md3_breadcrumbs *breadcrumbs,
                                         size_t max_items) {
  if (!breadcrumbs || max_items < 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  breadcrumbs->max_items = max_items;
  return UI_ERROR_NONE;
}

ui_error_t md3_breadcrumbs_simulate_click(struct md3_breadcrumbs *breadcrumbs,
                                          size_t index) {
  ui_error_t rc;

  if (!breadcrumbs || index >= breadcrumbs->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  breadcrumbs->active_index = index;
  rc = ui_breadcrumbs_base_simulate_click(breadcrumbs->base, index);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/* ========================================================================= */
/* Page Indicator Implementation                                             */
/* ========================================================================= */

ui_error_t
md3_page_indicator_create(struct ui_engine *engine, int page_count,
                          struct md3_page_indicator **out_indicator) {
  struct md3_page_indicator *ind;
  ui_error_t rc;

  if (!engine || !out_indicator || page_count <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ind = (struct md3_page_indicator *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_page_indicator));
  if (!ind) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ind, 0, sizeof(struct md3_page_indicator));
  ind->page_count = page_count;
  ind->current_page = 0;
  ind->pill_width = 16.0f;

  rc = ui_page_control_base_create(&ind->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ind);
    return rc;
  }

  rc = ui_page_control_base_set_number_of_pages(ind->base, page_count);
  if (rc != UI_ERROR_NONE) {
    if (ind->base->base.shadow_root) {
      ui_dom_node_destroy(ind->base->base.shadow_root);
    }
    C_MULTIPLATFORM_FREE(ind->base);
    C_MULTIPLATFORM_FREE(ind);
    return rc;
  }

  rc = ui_page_control_base_set_current_page(ind->base, 0);
  if (rc != UI_ERROR_NONE) {
    if (ind->base->base.shadow_root) {
      ui_dom_node_destroy(ind->base->base.shadow_root);
    }
    C_MULTIPLATFORM_FREE(ind->base);
    C_MULTIPLATFORM_FREE(ind);
    return rc;
  }

  *out_indicator = ind;
  return UI_ERROR_NONE;
}

ui_error_t md3_page_indicator_destroy(struct md3_page_indicator *indicator) {
  ui_error_t rc;

  if (!indicator) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (indicator->base) {
    if (indicator->base->base.shadow_root) {
      rc = ui_dom_node_destroy(indicator->base->base.shadow_root);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    C_MULTIPLATFORM_FREE(indicator->base);
  }

  C_MULTIPLATFORM_FREE(indicator);
  return UI_ERROR_NONE;
}

ui_error_t
md3_page_indicator_set_page_count(struct md3_page_indicator *indicator,
                                  int page_count) {
  ui_error_t rc;

  if (!indicator || page_count <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  indicator->page_count = page_count;
  if (indicator->current_page >= page_count) {
    indicator->current_page = page_count - 1;
  }

  rc = ui_page_control_base_set_number_of_pages(indicator->base, page_count);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_page_indicator_set_current_page(struct md3_page_indicator *indicator,
                                    int current_page) {
  ui_error_t rc;

  if (!indicator || current_page < 0 || current_page >= indicator->page_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  indicator->current_page = current_page;
  rc = ui_page_control_base_set_current_page(indicator->base, current_page);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_page_indicator_get_current_page(const struct md3_page_indicator *indicator,
                                    int *out_page) {
  if (!indicator || !out_page) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_page = indicator->current_page;
  return UI_ERROR_NONE;
}
