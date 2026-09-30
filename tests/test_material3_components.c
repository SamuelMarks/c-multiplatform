/* clang-format off */
#include "greatest.h"
#include "material3/md3_shape_morph.h"
#include "material3/md3_spring.h"
#include "material3/md3_motion.h"
#include "material3/md3_button.h"
#include "material3/md3_split_button.h"
#include "material3/md3_segmented_button.h"
#include "material3/md3_icon_button.h"
#include "material3/md3_fab.h"
#include "material3/md3_checkbox.h"
#include "material3/md3_radio_button.h"
#include "material3/md3_switch.h"
#include "material3/md3_text_field.h"
#include "material3/md3_search.h"
#include "material3/md3_slider.h"
#include "material3/md3_progress.h"
#include "material3/md3_card.h"
#include "material3/md3_divider.h"
#include "material3/md3_list.h"
#include "material3/md3_carousel.h"
#include "material3/md3_chip.h"
#include "material3/md3_navigation.h"
#include "material3/md3_overlay.h"
#include "ui_error.h"
#include "ui_test_mock_mem.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_md3_shape_morph_creation(void) {
  struct md3_shape shape;
  char svg_buf[512];
  ui_error_t rc;
  int type_idx;

  /* Invalid arguments */
  rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_RECTANGLE, 2, &shape);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_RECTANGLE, 100, &shape);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_RECTANGLE, 16, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_create_expressive((enum md3_expressive_shape_type) - 1, 16,
                                   &shape);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Exercise all expressive shape types */
  for (type_idx = 0; type_idx < MD3_EXPRESSIVE_SHAPE_TYPE_COUNT; type_idx++) {
    rc = md3_shape_create_expressive((enum md3_expressive_shape_type)type_idx,
                                     16, &shape);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(16, (int)shape.vertex_count);
    ASSERT_EQ(type_idx, (int)shape.type);

    /* Render to SVG path */
    rc =
        md3_shape_to_svg_path(&shape, 100.0f, 100.0f, svg_buf, sizeof(svg_buf));
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(strlen(svg_buf) > 0);
  }

  /* SVG path invalid arguments / buffer too small */
  rc = md3_shape_to_svg_path(NULL, 100.0f, 100.0f, svg_buf, sizeof(svg_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_to_svg_path(&shape, 100.0f, 100.0f, NULL, sizeof(svg_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_to_svg_path(&shape, 100.0f, 100.0f, svg_buf, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_to_svg_path(&shape, 0.0f, 100.0f, svg_buf, sizeof(svg_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_to_svg_path(&shape, 100.0f, 0.0f, svg_buf, sizeof(svg_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Shape with < 3 vertices */
  {
    struct md3_shape invalid_shape;
    invalid_shape = shape;
    invalid_shape.vertex_count = 2;
    rc = md3_shape_to_svg_path(&invalid_shape, 100.0f, 100.0f, svg_buf,
                               sizeof(svg_buf));
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }

  rc = md3_shape_to_svg_path(&shape, 100.0f, 100.0f, svg_buf, 5);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Buffer exactly fitting vertices but lacking room for 'Z\0' */
  {
    struct md3_shape small_shape;
    rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_CIRCLE, 4,
                                     &small_shape);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_shape_to_svg_path(&small_shape, 100.0f, 100.0f, svg_buf, 45);
    ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  }

  PASS();
}

TEST test_md3_shape_morph_interpolation(void) {
  struct md3_shape shape_a;
  struct md3_shape shape_b;
  struct md3_shape morphed;
  ui_error_t rc;

  rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_CIRCLE, 16, &shape_a);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_STAR_8, 16, &shape_b);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid args */
  rc = md3_shape_morph_interpolate(NULL, &shape_b, 0.5f, &morphed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_morph_interpolate(&shape_a, NULL, 0.5f, &morphed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_morph_interpolate(&shape_a, &shape_b, 0.5f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Midpoint morph */
  rc = md3_shape_morph_interpolate(&shape_a, &shape_b, 0.5f, &morphed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(16, (int)morphed.vertex_count);

  /* Unequal vertex counts: from->vertex_count < to->vertex_count */
  {
    struct md3_shape small_shape;
    rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_CIRCLE, 8,
                                     &small_shape);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_shape_morph_interpolate(&small_shape, &shape_b, 0.5f, &morphed);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(8, (int)morphed.vertex_count);

    /* Zero count shape */
    small_shape.vertex_count = 0;
    rc = md3_shape_morph_interpolate(&small_shape, &shape_b, 0.5f, &morphed);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }

  /* Bounds clamp < 0 and > 1 */
  rc = md3_shape_morph_interpolate(&shape_a, &shape_b, -0.5f, &morphed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_EXPRESSIVE_SHAPE_CIRCLE, morphed.type);

  rc = md3_shape_morph_interpolate(&shape_a, &shape_b, 1.5f, &morphed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_EXPRESSIVE_SHAPE_STAR_8, morphed.type);

  PASS();
}

TEST test_md3_spring_physics(void) {
  struct md3_spring_config cfg;
  float pos;
  float vel;
  ui_error_t rc;
  int token_idx;

  /* Invalid args to get_config */
  rc = md3_spring_get_config((enum md3_spring_token) - 1, &cfg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_spring_get_config(MD3_SPRING_TOKEN_COUNT, &cfg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_spring_get_config(MD3_SPRING_TOKEN_FAST_SPATIAL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Evaluate all tokens */
  for (token_idx = 0; token_idx < MD3_SPRING_TOKEN_COUNT; token_idx++) {
    rc = md3_spring_get_config((enum md3_spring_token)token_idx, &cfg);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = md3_spring_evaluate(&cfg, 0.0f, &pos, &vel);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ_FMT(0.0f, pos, "%f");

    rc = md3_spring_evaluate(&cfg, 1.0f, &pos, &vel);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test critically damped (zeta == 1.0) */
  cfg.damping_ratio = 1.0f;
  cfg.stiffness = 100.0f;
  cfg.mass = 1.0f;
  cfg.initial_position = 0.0f;
  cfg.target_position = 10.0f;
  cfg.initial_velocity = 0.0f;
  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pos > 0.0f && pos <= 10.0f);

  /* Test overdamped (zeta == 1.5) */
  cfg.damping_ratio = 1.5f;
  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pos > 0.0f && pos <= 10.0f);

  /* Test invalid parameters */
  rc = md3_spring_evaluate(NULL, 0.1f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  cfg.mass = -1.0f;
  rc = md3_spring_evaluate(&cfg, 0.1f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

static ui_error_t on_test_button_clicked(struct ui_button_base *button,
                                         void *user_data) {
  int *clicked;
  if (!button || !user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  clicked = (int *)user_data;
  *clicked += 1;
  return UI_ERROR_NONE;
}

TEST test_md3_button_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_button *button = NULL;
  struct ui_button_base *base = NULL;
  int clicked_count = 0;
  ui_error_t rc;

  /* Invalid args */
  rc = md3_button_create(NULL, MD3_BUTTON_FILLED, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_create(dummy_engine, (enum md3_button_variant) - 1, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_create(dummy_engine, MD3_BUTTON_VARIANT_COUNT, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_create(dummy_engine, MD3_BUTTON_FILLED, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_set_text(NULL, "Submit");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_set_loading(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_set_icon(NULL, "check");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_set_on_click(NULL, on_test_button_clicked, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Successful creation */
  rc = md3_button_create(dummy_engine, MD3_BUTTON_FILLED, &button);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(button != NULL);

  rc = md3_button_get_base(button, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_set_text(button, "Submit");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_button_set_disabled(button, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_button_set_disabled(button, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_button_set_loading(button, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, button->is_loading);

  rc = md3_button_set_loading(button, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, button->is_loading);

  rc = md3_button_set_icon(button, "check");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("check", button->icon_name);

  rc = md3_button_set_icon(button, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', button->icon_name[0]);

  rc = md3_button_set_on_click(button, on_test_button_clicked, &clicked_count);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_button_get_base(button, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  {
    struct md3_button *btn2 = NULL;
    rc = md3_button_create(dummy_engine, MD3_BUTTON_FILLED, &btn2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(btn2 != NULL);
    ui_button_base_destroy(btn2->base);
    btn2->base = NULL;
    rc = md3_button_destroy(btn2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_button_destroy(button);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_checkbox_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_checkbox *cb = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_checkbox_base *base = NULL;
  int is_checked = 0;
  ui_error_t rc;

  /* Invalid args */
  rc = md3_checkbox_create(NULL, &cb, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_set_checked(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_get_checked(NULL, &is_checked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_set_indeterminate(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Creation with out_cva == NULL */
  rc = md3_checkbox_create(dummy_engine, &cb, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cb != NULL);
  rc = md3_checkbox_destroy(cb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Successful creation */
  rc = md3_checkbox_create(dummy_engine, &cb, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cb != NULL);
  ASSERT(cva != NULL);

  rc = md3_checkbox_get_base(cb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_get_checked(cb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_checkbox_set_checked(cb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_checkbox_get_checked(cb, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_checked);

  rc = md3_checkbox_set_indeterminate(cb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, cb->is_indeterminate);

  rc = md3_checkbox_set_indeterminate(cb, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, cb->is_indeterminate);

  rc = md3_checkbox_set_disabled(cb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_checkbox_set_disabled(cb, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA invocation */
  {
    union ui_signal_payload payload;
    memset(&payload, 0, sizeof(payload));
    rc = cva->write_value(NULL, payload);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    payload.bool_val = UI_FALSE;
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    payload.bool_val = UI_TRUE;
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = cva->set_disabled_state(NULL, UI_TRUE);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    rc = cva->set_disabled_state(cva->component, UI_TRUE);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = cva->set_disabled_state(cva->component, UI_FALSE);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_checkbox_get_checked(cb, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_checked);

  rc = md3_checkbox_get_base(cb, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  {
    struct ui_checkbox_base *saved_base = cb->base;
    cb->base = NULL;
    rc = md3_checkbox_get_checked(cb, &is_checked);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    cb->base = saved_base;
  }

  {
    struct md3_checkbox *cb2 = NULL;
    rc = md3_checkbox_create(dummy_engine, &cb2, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(cb2 != NULL);
    ui_checkbox_base_destroy(cb2->base);
    cb2->base = NULL;
    rc = md3_checkbox_destroy(cb2);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }

  rc = md3_checkbox_destroy(cb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_radio_group_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_radio_group *grp = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_radio_group_base *base = NULL;
  int selected = -1;
  ui_error_t rc;

  /* Invalid args */
  rc = md3_radio_group_create(NULL, &grp, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_radio_group_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_radio_group_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_radio_group_set_selected_index(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_radio_group_get_selected_index(NULL, &selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_radio_group_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_radio_group_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Successful creation with out_cva NULL */
  rc = md3_radio_group_create(dummy_engine, &grp, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(grp != NULL);
  rc = md3_radio_group_destroy(grp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Successful creation */
  rc = md3_radio_group_create(dummy_engine, &grp, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(grp != NULL);
  ASSERT(cva != NULL);

  rc = md3_radio_group_get_base(grp, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_radio_group_get_selected_index(grp, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_radio_group_set_selected_index(grp, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_radio_group_get_selected_index(grp, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, selected);

  rc = md3_radio_group_set_disabled(grp, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, grp->is_disabled);

  rc = md3_radio_group_set_disabled(grp, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, grp->is_disabled);

  /* CVA invocation */
  {
    union ui_signal_payload payload;
    memset(&payload, 0, sizeof(payload));
    rc = cva->write_value(NULL, payload);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    payload.int_val = 1;
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_radio_group_get_selected_index(grp, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);

  rc = cva->set_disabled_state(NULL, UI_TRUE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cva->set_disabled_state(cva->component, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, grp->is_disabled);

  rc = cva->set_disabled_state(cva->component, UI_FALSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, grp->is_disabled);

  rc = md3_radio_group_get_base(grp, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  {
    struct md3_radio_group *grp2 = NULL;
    rc = md3_radio_group_create(dummy_engine, &grp2, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(grp2 != NULL);
    ui_radio_group_base_destroy(grp2->base);
    grp2->base = NULL;
    rc = md3_radio_group_destroy(grp2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_radio_group_destroy(grp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_switch_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_switch *sw = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_slide_toggle_base *base = NULL;
  int is_checked = 0;
  ui_error_t rc;

  /* Invalid args */
  rc = md3_switch_create(NULL, &sw, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_set_checked(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_get_checked(NULL, &is_checked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_set_show_icon(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Successful creation with out_cva NULL */
  rc = md3_switch_create(dummy_engine, &sw, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sw != NULL);
  rc = md3_switch_destroy(sw);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Successful creation */
  rc = md3_switch_create(dummy_engine, &sw, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sw != NULL);
  ASSERT(cva != NULL);

  rc = md3_switch_get_base(sw, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_get_checked(sw, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_switch_set_checked(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_switch_get_checked(sw, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_checked);

  rc = md3_switch_set_checked(sw, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_switch_get_checked(sw, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_checked);

  rc = md3_switch_set_show_icon(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, sw->show_icon);

  rc = md3_switch_set_show_icon(sw, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, sw->show_icon);

  rc = md3_switch_set_disabled(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_switch_set_disabled(sw, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA invocation */
  {
    union ui_signal_payload payload;
    memset(&payload, 0, sizeof(payload));
    rc = cva->write_value(NULL, payload);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    payload.bool_val = UI_FALSE;
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    payload.bool_val = UI_TRUE;
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_switch_get_checked(sw, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_checked);

  rc = cva->set_disabled_state(NULL, UI_TRUE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cva->set_disabled_state(cva->component, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cva->set_disabled_state(cva->component, UI_FALSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_switch_get_base(sw, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  {
    struct md3_switch *sw2 = NULL;
    rc = md3_switch_create(dummy_engine, &sw2, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(sw2 != NULL);
    ui_slide_toggle_base_destroy(sw2->base);
    sw2->base = NULL;
    rc = md3_switch_destroy(sw2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_switch_destroy(sw);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_text_field_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_text_field *tf = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_input_base *input = NULL;
  const char *val = NULL;
  struct md3_text_field *dummy_tf = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  /* Invalid args */
  rc = md3_text_field_create(NULL, MD3_TEXT_FIELD_FILLED, &tf, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_text_field_create(dummy_engine, (enum md3_text_field_variant) - 1,
                             &tf, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_text_field_create(dummy_engine, MD3_TEXT_FIELD_VARIANT_COUNT, &tf,
                             &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_text_field_create(dummy_engine, MD3_TEXT_FIELD_FILLED, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_text_field_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Test destroy with NULL internal fields */
  dummy_tf = (struct md3_text_field *)malloc(sizeof(struct md3_text_field));
  ASSERT(dummy_tf != NULL);
  memset(dummy_tf, 0, sizeof(struct md3_text_field));
  rc = md3_text_field_destroy(dummy_tf);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Successful creation with out_cva == NULL */
  rc = md3_text_field_create(dummy_engine, MD3_TEXT_FIELD_OUTLINED, &tf, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tf != NULL);
  rc = md3_text_field_destroy(tf);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  tf = NULL;

  /* Successful creation with out_cva */
  rc = md3_text_field_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &tf, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tf != NULL);
  ASSERT(cva != NULL);

  /* NULL field invalid arguments */
  rc = md3_text_field_set_label(NULL, "Username");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_set_supporting_text(NULL, "Hint");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_set_prefix(NULL, "$");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_set_suffix(NULL, "%");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_set_text(NULL, "text");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_get_text(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_get_text(tf, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_set_error_text(NULL, "err");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_get_input_base(NULL, &input);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_text_field_get_input_base(tf, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid operations */
  rc = md3_text_field_set_label(tf, "Username");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_set_supporting_text(tf, "Required field");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_set_prefix(tf, "@");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("@", tf->prefix_text);

  rc = md3_text_field_set_prefix(tf, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', tf->prefix_text[0]);

  rc = md3_text_field_set_suffix(tf, ".com");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ(".com", tf->suffix_text);

  rc = md3_text_field_set_suffix(tf, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', tf->suffix_text[0]);

  rc = md3_text_field_set_text(tf, "johndoe");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_get_text(tf, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("johndoe", val);

  rc = md3_text_field_set_text(tf, "");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_set_text(tf, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_set_error_text(tf, "Invalid user");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_set_error_text(tf, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_set_disabled(tf, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_text_field_set_disabled(tf, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA invocation */
  payload.ptr_val = (void *)"newuser";
  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  payload.ptr_val = NULL;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_get_text(tf, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", val);

  rc = cva->set_disabled_state(NULL, UI_FALSE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cva->set_disabled_state(cva->component, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cva->set_disabled_state(cva->component, UI_FALSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_text_field_get_input_base(tf, &input);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(input != NULL);

  rc = md3_text_field_destroy(tf);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

static ui_error_t on_split_test_main_click(struct ui_button_base *btn,
                                           void *user_data) {
  int *c;
  if (!btn || !user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  c = (int *)user_data;
  *c += 1;
  return UI_ERROR_NONE;
}

static ui_error_t on_split_test_trigger_click(struct ui_button_base *btn,
                                              void *user_data) {
  int *c;
  if (!btn || !user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  c = (int *)user_data;
  *c += 10;
  return UI_ERROR_NONE;
}

static ui_error_t trigger_button_click(struct ui_button_base *btn) {
  struct ui_event ev;
  ui_error_t rc;

  if (!btn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  rc = ui_button_base_process_event(btn, &ev, 10.0);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  ev.type = UI_EVENT_MOUSE_UP;
  return ui_button_base_process_event(btn, &ev, 20.0);
}

TEST test_md3_motion_cubic_bezier(void) {
  struct md3_cubic_bezier bez;
  float val;
  ui_error_t rc;
  int c;
  int p;
  static const float test_progresses[] = {0.05f, 0.15f, 0.3f, 0.5f,
                                          0.7f,  0.85f, 0.95f};

  /* Invalid arguments */
  rc = md3_motion_get_curve((enum md3_motion_curve) - 1, &bez);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_motion_get_curve(MD3_MOTION_CURVE_COUNT, &bez);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_motion_get_curve(MD3_MOTION_CURVE_STANDARD, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_motion_cubic_bezier_evaluate(MD3_MOTION_CURVE_STANDARD, 0.5f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      md3_motion_cubic_bezier_evaluate((enum md3_motion_curve) - 1, 0.5f, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_motion_cubic_bezier_evaluate(MD3_MOTION_CURVE_COUNT, 0.5f, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Boundary values */
  rc = md3_motion_cubic_bezier_evaluate(MD3_MOTION_CURVE_STANDARD, -0.5f, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(0.0f, val, "%f");

  rc = md3_motion_cubic_bezier_evaluate(MD3_MOTION_CURVE_STANDARD, 0.0f, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(0.0f, val, "%f");

  rc = md3_motion_cubic_bezier_evaluate(MD3_MOTION_CURVE_STANDARD, 1.0f, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(1.0f, val, "%f");

  rc = md3_motion_cubic_bezier_evaluate(MD3_MOTION_CURVE_STANDARD, 1.5f, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(1.0f, val, "%f");

  /* All curves evaluation across diverse progress values */
  for (c = 0; c < (int)MD3_MOTION_CURVE_COUNT; c++) {
    rc = md3_motion_get_curve((enum md3_motion_curve)c, &bez);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    for (p = 0; p < (int)(sizeof(test_progresses) / sizeof(test_progresses[0]));
         p++) {
      rc = md3_motion_cubic_bezier_evaluate((enum md3_motion_curve)c,
                                            test_progresses[p], &val);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      ASSERT(val >= 0.0f && val <= 1.0f);
    }
  }

  PASS();
}

TEST test_md3_split_button_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_split_button *sb = NULL;
  struct ui_split_button_base *base = NULL;
  struct ui_button_base *main_b = NULL;
  struct ui_button_base *trig_b = NULL;
  int main_clicked = 0;
  int trig_clicked = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_split_button_create(NULL, MD3_BUTTON_FILLED, &sb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_split_button_create(dummy_engine, MD3_BUTTON_FILLED, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      md3_split_button_create(dummy_engine, (enum md3_button_variant) - 1, &sb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_split_button_create(dummy_engine, MD3_BUTTON_VARIANT_COUNT, &sb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_get_main_button(NULL, &main_b);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_get_trigger_button(NULL, &trig_b);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_set_text(NULL, "Save");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_set_on_click(NULL, on_split_test_main_click,
                                     &main_clicked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_set_on_trigger_click(NULL, on_split_test_trigger_click,
                                             &trig_clicked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_create(dummy_engine, MD3_BUTTON_FILLED, &sb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sb != NULL);

  rc = md3_split_button_get_base(sb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_get_base(sb, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_split_button_get_main_button(sb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_get_main_button(sb, &main_b);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(main_b != NULL);

  rc = md3_split_button_get_trigger_button(sb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_split_button_get_trigger_button(sb, &trig_b);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(trig_b != NULL);

  rc = md3_split_button_set_text(sb, "Save");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_split_button_set_on_click(sb, on_split_test_main_click,
                                     &main_clicked);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_split_button_set_on_trigger_click(sb, on_split_test_trigger_click,
                                             &trig_clicked);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = trigger_button_click(main_b);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, main_clicked);

  rc = trigger_button_click(trig_b);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, trig_clicked);

  rc = md3_split_button_set_disabled(sb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_split_button_set_disabled(sb, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  {
    struct ui_split_button_base *saved_base = sb->base;
    sb->base = NULL;
    rc = md3_split_button_set_text(sb, "Save");
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_split_button_set_on_click(sb, on_split_test_main_click, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_split_button_set_on_trigger_click(sb, on_split_test_trigger_click,
                                               NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    sb->base = saved_base;
  }

  {
    struct md3_split_button *sb2 = NULL;
    rc = md3_split_button_create(dummy_engine, MD3_BUTTON_FILLED, &sb2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(sb2 != NULL);
    ui_split_button_base_destroy(sb2->base);
    sb2->base = NULL;
    rc = md3_split_button_destroy(sb2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_split_button_destroy(sb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_segmented_button_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_segmented_button *sb = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_segmented_control_base *base = NULL;
  struct ui_segmented_button_base *seg = NULL;
  int is_sel;
  int i;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_segmented_button_create(NULL, UI_SEGMENTED_CONTROL_MODE_SINGLE, &sb,
                                   &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_segmented_button_create(
      dummy_engine, UI_SEGMENTED_CONTROL_MODE_SINGLE, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_segmented_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Single-select mode */
  rc = md3_segmented_button_create(dummy_engine,
                                   UI_SEGMENTED_CONTROL_MODE_SINGLE, &sb, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sb != NULL);
  ASSERT(cva != NULL);

  rc = md3_segmented_button_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_segmented_button_get_base(sb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_segmented_button_get_base(sb, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_segmented_button_add_segment(NULL, "Day", "sun", 101, &seg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_segmented_button_add_segment(sb, "Day", "sun", 101, &seg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(seg != NULL);

  /* Icon only and label only */
  rc = md3_segmented_button_add_segment(sb, "Week", NULL, 102, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_segmented_button_add_segment(sb, NULL, "grid", 103, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_segmented_button_add_segment(sb, NULL, NULL, 104, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_segmented_button_select_segment(NULL, 102);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_segmented_button_select_segment(sb, 102);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_segmented_button_is_selected(NULL, 102, &is_sel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_segmented_button_is_selected(sb, 102, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_segmented_button_is_selected(sb, 102, &is_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_sel);

  rc = md3_segmented_button_is_selected(sb, 101, &is_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_sel);

  /* Non-existent segment */
  rc = md3_segmented_button_select_segment(sb, 999);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  rc = md3_segmented_button_is_selected(sb, 999, &is_sel);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* CVA invocation */
  {
    union ui_signal_payload payload;
    payload.int_val = 101;
    rc = cva->write_value(NULL, payload);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_segmented_button_is_selected(sb, 101, &is_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_sel);

  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva->set_disabled_state(cva->component, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Fill up segments to test capacity guard */
  for (i = 0; i < 12; i++) {
    rc = md3_segmented_button_add_segment(sb, "Extra", NULL, 200 + i, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_segmented_button_add_segment(sb, "Overflow", NULL, 999, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_segmented_button_destroy(sb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test destroy with a segment having NULL base */
  {
    struct md3_segmented_button *sb_null_seg = NULL;
    rc = md3_segmented_button_create(
        dummy_engine, UI_SEGMENTED_CONTROL_MODE_SINGLE, &sb_null_seg, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_segmented_button_add_segment(sb_null_seg, "Seg", NULL, 1, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    sb_null_seg->segments[0].base = NULL;
    rc = md3_segmented_button_destroy(sb_null_seg);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Multi-select mode */
  rc = md3_segmented_button_create(dummy_engine,
                                   UI_SEGMENTED_CONTROL_MODE_MULTI, &sb, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_segmented_button_add_segment(sb, "Bold", "format_bold", 1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_segmented_button_add_segment(sb, "Italic", "format_italic", 2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_segmented_button_select_segment(sb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_segmented_button_select_segment(sb, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_segmented_button_is_selected(sb, 1, &is_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_sel);
  rc = md3_segmented_button_is_selected(sb, 2, &is_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_sel);

  /* Toggle segment 1 off */
  rc = md3_segmented_button_select_segment(sb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_segmented_button_is_selected(sb, 1, &is_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_sel);

  rc = md3_segmented_button_destroy(sb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_icon_button_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_icon_button *ib = NULL;
  struct ui_button_base *base = NULL;
  int is_sel;
  int clicked = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_icon_button_create(NULL, MD3_ICON_BUTTON_STANDARD, "star", &ib);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_button_create(dummy_engine, MD3_ICON_BUTTON_STANDARD, "star",
                              NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_button_create(dummy_engine, (enum md3_icon_button_variant) - 1,
                              "star", &ib);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_icon_button_create(dummy_engine, MD3_ICON_BUTTON_VARIANT_COUNT,
                              "star", &ib);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc =
      md3_icon_button_create(dummy_engine, MD3_ICON_BUTTON_STANDARD, NULL, &ib);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ib != NULL);
  rc = md3_icon_button_destroy(ib);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_icon_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_set_on_click(NULL, on_test_button_clicked, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_set_toggleable(NULL, 1, "star");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_set_selected(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_is_selected(NULL, &is_sel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_set_icon(NULL, "star");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_create(dummy_engine, MD3_ICON_BUTTON_OUTLINED, "star",
                              &ib);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ib != NULL);

  rc = md3_icon_button_get_base(ib, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_is_selected(ib, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_icon_button_set_icon(ib, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', ib->icon_name[0]);

  rc = md3_icon_button_get_base(ib, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_icon_button_set_disabled(ib, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_icon_button_set_disabled(ib, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_icon_button_set_on_click(ib, on_test_button_clicked, &clicked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = trigger_button_click(base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, clicked);

  /* Non-toggleable selected state change */
  rc = md3_icon_button_set_selected(ib, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Toggleable configuration */
  rc = md3_icon_button_set_toggleable(ib, 1, "star_filled");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_icon_button_set_selected(ib, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_icon_button_is_selected(ib, &is_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_sel);

  rc = md3_icon_button_set_selected(ib, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_icon_button_is_selected(ib, &is_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_sel);

  rc = md3_icon_button_set_toggleable(ib, 0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Toggleable without selected icon name */
  rc = md3_icon_button_set_toggleable(ib, 1, "");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_icon_button_set_selected(ib, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_icon_button_set_toggleable(ib, 0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_icon_button_set_icon(ib, "favorite");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Non-toggleable selected changes with valid base and icon_name present */
  rc = md3_icon_button_set_selected(ib, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_icon_button_set_selected(ib, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set selected on button with no icon */
  {
    struct md3_icon_button *no_icon = NULL;
    struct ui_button_base *saved_base = NULL;
    rc = md3_icon_button_create(dummy_engine, MD3_ICON_BUTTON_STANDARD, NULL,
                                &no_icon);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_icon_button_set_selected(no_icon, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_icon_button_set_selected(no_icon, 0);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    saved_base = no_icon->base;
    no_icon->base = NULL;
    rc = md3_icon_button_set_selected(no_icon, 0);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    no_icon->base = saved_base;
    rc = md3_icon_button_destroy(no_icon);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Failure path with base = NULL when toggleable is true and selected is 1 */
  {
    struct ui_button_base *saved_base = NULL;
    rc = md3_icon_button_set_toggleable(ib, 1, "favorite_sel");
    ASSERT_EQ(UI_ERROR_NONE, rc);
    saved_base = ib->base;
    ib->base = NULL;
    rc = md3_icon_button_set_selected(ib, 1);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_icon_button_set_selected(ib, 0);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    ib->base = saved_base;
  }

  {
    struct md3_icon_button *ib2 = NULL;
    rc = md3_icon_button_create(dummy_engine, MD3_ICON_BUTTON_STANDARD, NULL,
                                &ib2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(ib2 != NULL);
    ui_button_base_destroy(ib2->base);
    ib2->base = NULL;
    rc = md3_icon_button_destroy(ib2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_icon_button_destroy(ib);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

static ui_error_t on_fab_speed_dial_click(struct ui_button_base *btn,
                                          void *user_data) {
  int *c;
  if (!btn || !user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  c = (int *)user_data;
  *c += 5;
  return UI_ERROR_NONE;
}

TEST test_md3_fab_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_fab *fab = NULL;
  struct md3_fab *fab_alt = NULL;
  struct ui_fab_base *base = NULL;
  struct md3_fab dummy_fab;
  enum ui_fab_state state;
  int action_clicked = 0;
  int fab_clicked = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_fab_create(NULL, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY, "add", NULL,
                      &fab);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_create(dummy_engine, (enum md3_fab_size) - 1, MD3_FAB_PRIMARY,
                      "add", NULL, &fab);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_COUNT, MD3_FAB_PRIMARY, "add",
                      NULL, &fab);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR,
                      (enum md3_fab_variant) - 1, "add", NULL, &fab);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_VARIANT_COUNT,
                      "add", NULL, &fab);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                      "add", NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Alternative label/icon combinations */
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_SMALL, MD3_FAB_SURFACE, NULL,
                      "LabelOnly", &fab_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fab_alt != NULL);
  rc = md3_fab_destroy(fab_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_LARGE, MD3_FAB_SECONDARY,
                      "IconOnly", NULL, &fab_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fab_alt != NULL);
  rc = md3_fab_destroy(fab_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_TERTIARY,
                      NULL, NULL, &fab_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fab_alt != NULL);
  rc = md3_fab_destroy(fab_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  fab_alt = NULL;

  /* Extended FAB */
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_EXTENDED, MD3_FAB_PRIMARY,
                      "edit", "Compose", &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fab != NULL);

  rc = md3_fab_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_get_base(fab, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_get_base(fab, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_fab_set_elevation(NULL, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_set_elevation(fab, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_fab_set_elevation(fab, 99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_add_speed_dial_action(NULL, "attach_file", "Attach",
                                     on_fab_speed_dial_click, &action_clicked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_add_speed_dial_action(fab, "attach_file", "Attach",
                                     on_fab_speed_dial_click, &action_clicked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_fab_add_speed_dial_action(fab, "icon_only", NULL, NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_fab_add_speed_dial_action(fab, NULL, NULL, NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_toggle(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_toggle(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_get_speed_dial_state(NULL, &state);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_get_speed_dial_state(fab, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_get_speed_dial_state(fab, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FAB_STATE_EXPANDING, state);

  rc = md3_fab_tick(NULL, 350.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_tick(fab, 350.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_get_speed_dial_state(fab, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_FAB_STATE_EXPANDED, state);

  rc = md3_fab_set_on_click(NULL, on_test_button_clicked, &fab_clicked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  memset(&dummy_fab, 0, sizeof(dummy_fab));
  rc = md3_fab_set_on_click(&dummy_fab, on_test_button_clicked, &fab_clicked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_set_on_click(fab, on_test_button_clicked, &fab_clicked);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_search_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_search_bar *sb = NULL;
  struct md3_search_bar *sb2 = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_search_bar_base *base = NULL;
  struct md3_search_view *view = NULL;
  const char *q = NULL;
  size_t count = 0;
  const char *item = NULL;
  size_t i;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_search_bar_create(NULL, "Search", &sb, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_create(dummy_engine, "Search", NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_search_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create with placeholder=NULL, out_cva=NULL */
  rc = md3_search_bar_create(dummy_engine, NULL, &sb2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sb2 != NULL);
  rc = md3_search_bar_destroy(sb2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_search_bar_create(dummy_engine, "Search...", &sb, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sb != NULL);

  rc = md3_search_bar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_get_base(sb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_get_base(sb, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_search_bar_set_query(NULL, "q");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_set_query(sb, "Material Design");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_search_bar_get_query(NULL, &q);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_get_query(sb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_get_query(sb, &q);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Material Design", q);

  rc = md3_search_bar_set_loading(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_set_loading(sb, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_search_bar_set_loading(sb, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_search_bar_open_view(NULL, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_open_view(sb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_bar_open_view(sb, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);
  ASSERT_EQ(1, view->is_open);

  rc = md3_search_view_add_history_item(NULL, "tokens");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_view_add_history_item(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_search_view_add_history_item(view, "tokens");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_search_view_add_history_item(view, "elevation");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_search_view_get_history_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_view_get_history_count(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_view_get_history_count(view, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count);

  rc = md3_search_view_get_history_item(NULL, 0, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_view_get_history_item(view, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_view_get_history_item(view, 0, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("tokens", item);

  rc = md3_search_view_get_history_item(view, 99, &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Fill up history to test overflow branch */
  for (i = 2; i < MD3_SEARCH_MAX_HISTORY; i++) {
    rc = md3_search_view_add_history_item(view, "dummy");
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_search_view_add_history_item(view, "overflow");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_search_view_close(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_search_view_close(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, view->is_open);

  rc = md3_search_bar_destroy(sb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

static ui_error_t on_slider_change(struct ui_slider_base *s, float val,
                                   void *ud) {
  float *v;
  if (!s || !ud) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  v = (float *)ud;
  *v = val;
  return UI_ERROR_NONE;
}

TEST test_md3_slider_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_slider *s = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_slider_base *base = NULL;
  struct md3_range_slider *rs = NULL;
  struct ui_range_slider_base *rbase = NULL;
  float val = 0.0f;
  float low = 0.0f;
  float high = 0.0f;
  float cb_val = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_slider_create(NULL, MD3_SLIDER_CONTINUOUS, 0.0f, 100.0f, &s, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_create(dummy_engine, MD3_SLIDER_CONTINUOUS, 100.0f, 0.0f, &s,
                         &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_create(dummy_engine, MD3_SLIDER_CONTINUOUS, 0.0f, 100.0f,
                         NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_slider_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Continuous Slider */
  rc = md3_slider_create(dummy_engine, MD3_SLIDER_CONTINUOUS, 0.0f, 100.0f, &s,
                         &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(s != NULL);

  rc = md3_slider_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_get_base(s, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_get_base(s, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_slider_set_value(NULL, 42.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_set_value(s, 42.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_slider_get_value(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_get_value(s, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_get_value(s, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(42.0f, val, "%f");

  rc = md3_slider_set_step(NULL, 2.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_set_step(s, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_set_step(s, 2.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_slider_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_set_disabled(s, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_slider_set_disabled(s, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_slider_set_on_change(NULL, on_slider_change, &cb_val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_slider_set_on_change(s, on_slider_change, &cb_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_slider_destroy(s);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Discrete Slider */
  rc = md3_slider_create(dummy_engine, MD3_SLIDER_DISCRETE, 0.0f, 10.0f, &s,
                         NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(s != NULL);
  rc = md3_slider_destroy(s);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Range Slider */
  rc = md3_range_slider_create(NULL, 0.0f, 100.0f, &rs);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_range_slider_create(dummy_engine, 100.0f, 0.0f, &rs);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_range_slider_create(dummy_engine, 0.0f, 100.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_range_slider_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_range_slider_create(dummy_engine, 0.0f, 100.0f, &rs);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(rs != NULL);

  rc = md3_range_slider_get_base(NULL, &rbase);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_range_slider_get_base(rs, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_range_slider_get_base(rs, &rbase);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(rbase != NULL);

  rc = md3_range_slider_set_values(NULL, 20.0f, 80.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_range_slider_set_values(rs, 20.0f, 80.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_range_slider_get_values(NULL, &low, &high);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_range_slider_get_values(rs, NULL, &high);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_range_slider_get_values(rs, &low, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_range_slider_get_values(rs, &low, &high);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(20.0f, low, "%f");
  ASSERT_EQ_FMT(80.0f, high, "%f");

  rc = md3_range_slider_destroy(rs);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_progress_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_progress *p = NULL;
  struct ui_progress_base *base = NULL;
  float pct = 0.0f;
  int is_ind = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_progress_create(NULL, MD3_PROGRESS_LINEAR, &p);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_progress_create(dummy_engine, (enum md3_progress_type) - 1, &p);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_progress_create(dummy_engine, MD3_PROGRESS_TYPE_COUNT, &p);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_progress_create(dummy_engine, MD3_PROGRESS_LINEAR, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_set_determinate(NULL, 50.0f, 0.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_set_indeterminate(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_get_percentage(NULL, &pct);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_is_indeterminate(NULL, &is_ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Linear Progress */
  rc = md3_progress_create(dummy_engine, MD3_PROGRESS_LINEAR, &p);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(p != NULL);

  rc = md3_progress_set_determinate(p, 50.0f, 100.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_get_percentage(p, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_is_indeterminate(p, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_get_base(p, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_progress_get_base(p, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_progress_set_determinate(p, 50.0f, 0.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_progress_get_percentage(p, &pct);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(0.5f, pct, "%f");

  rc = md3_progress_is_indeterminate(p, &is_ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_ind);

  rc = md3_progress_set_indeterminate(p);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_progress_is_indeterminate(p, &is_ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_ind);

  {
    struct md3_progress *p2 = NULL;
    rc = md3_progress_create(dummy_engine, MD3_PROGRESS_LINEAR, &p2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(p2 != NULL);
    ui_progress_base_destroy(p2->base);
    p2->base = NULL;
    rc = md3_progress_destroy(p2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_progress_destroy(p);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Circular Progress */
  rc = md3_progress_create(dummy_engine, MD3_PROGRESS_CIRCULAR, &p);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_progress_destroy(p);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Expressive Segmented Progress */
  rc = md3_progress_create(dummy_engine, MD3_PROGRESS_EXPRESSIVE_SEGMENTED, &p);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_progress_destroy(p);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_card_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_card *card = NULL;
  struct ui_card_base *base = NULL;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_card_create(NULL, MD3_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_card_create(dummy_engine, (enum md3_card_variant) - 1, &card);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_card_create(dummy_engine, MD3_CARD_VARIANT_COUNT, &card);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_card_create(dummy_engine, MD3_CARD_ELEVATED, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_set_title(NULL, "Headline Card");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_set_subtitle(NULL, "Supporting text");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_set_header(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_set_content(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_set_actions(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_create(dummy_engine, MD3_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(card != NULL);

  rc = md3_card_get_base(card, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_get_base(card, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_card_set_title(card, "Headline Card");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Headline Card", card->title);

  rc = md3_card_set_title(card, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_set_subtitle(card, "Supporting text");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Supporting text", card->subtitle);

  rc = md3_card_set_subtitle(card, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_card_set_header(card, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_card_set_content(card, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_card_set_actions(card, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  {
    struct md3_card *card2 = NULL;
    rc = md3_card_create(dummy_engine, MD3_CARD_ELEVATED, &card2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(card2 != NULL);
    ui_card_base_destroy(card2->base);
    card2->base = NULL;
    rc = md3_card_destroy(card2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_card_destroy(card);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_divider_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_divider *div = NULL;
  struct ui_divider_base *base = NULL;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_divider_create(NULL, UI_DIVIDER_ORIENTATION_HORIZONTAL, 0, &div);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_divider_create(dummy_engine, (enum ui_divider_orientation) - 1, 0,
                          &div);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_divider_create(dummy_engine, (enum ui_divider_orientation)99, 0,
                          &div);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_divider_create(dummy_engine, UI_DIVIDER_ORIENTATION_HORIZONTAL, 0,
                          NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_divider_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_divider_set_orientation(NULL, UI_DIVIDER_ORIENTATION_VERTICAL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_divider_set_inset(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_divider_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Creation with vertical orientation and is_inset = 0 */
  rc = md3_divider_create(dummy_engine, UI_DIVIDER_ORIENTATION_VERTICAL, 0,
                          &div);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(div != NULL);
  ASSERT_EQ(0, div->is_inset);
  ASSERT_EQ(UI_DIVIDER_ORIENTATION_VERTICAL, div->orientation);
  rc = md3_divider_destroy(div);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Creation with horizontal orientation and is_inset = 1 */
  rc = md3_divider_create(dummy_engine, UI_DIVIDER_ORIENTATION_HORIZONTAL, 1,
                          &div);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(div != NULL);
  ASSERT_EQ(1, div->is_inset);

  rc = md3_divider_get_base(div, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_divider_get_base(div, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_divider_set_orientation(div, (enum ui_divider_orientation) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_divider_set_orientation(div, (enum ui_divider_orientation)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_divider_set_orientation(div, UI_DIVIDER_ORIENTATION_VERTICAL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_DIVIDER_ORIENTATION_VERTICAL, div->orientation);

  rc = md3_divider_set_orientation(div, UI_DIVIDER_ORIENTATION_HORIZONTAL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_DIVIDER_ORIENTATION_HORIZONTAL, div->orientation);

  rc = md3_divider_set_inset(div, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, div->is_inset);

  rc = md3_divider_set_inset(div, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, div->is_inset);

  {
    struct md3_divider *div2 = NULL;
    rc = md3_divider_create(dummy_engine, UI_DIVIDER_ORIENTATION_HORIZONTAL, 0,
                            &div2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(div2 != NULL);
    ui_divider_base_destroy(div2->base);
    div2->base = NULL;
    rc = md3_divider_destroy(div2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_divider_destroy(div);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_list_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_list *list = NULL;
  struct md3_list_item *item = NULL;
  struct md3_list_item *item_alt = NULL;
  struct md3_list_item dummy_item;
  struct ui_component *comp = NULL;
  int selected = 0;
  ui_error_t rc;

  /* Invalid args */
  rc = md3_list_create(NULL, &list);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_list_set_segmented(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_list_create(dummy_engine, &list);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(list != NULL);

  rc = md3_list_set_segmented(list, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, list->is_segmented);
  rc = md3_list_set_segmented(list, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, list->is_segmented);

  /* Item lifecycle */
  rc = md3_list_item_create(NULL, MD3_LIST_ITEM_TWO_LINE, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_create(dummy_engine, (enum md3_list_item_lines)0, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_create(dummy_engine, (enum md3_list_item_lines)4, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_create(dummy_engine, MD3_LIST_ITEM_TWO_LINE, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_list_item_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_list_item_create(dummy_engine, MD3_LIST_ITEM_ONE_LINE, &item_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_item_destroy(item_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_list_item_create(dummy_engine, MD3_LIST_ITEM_THREE_LINE, &item_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_item_destroy(item_alt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  item_alt = NULL;

  rc = md3_list_item_create(dummy_engine, MD3_LIST_ITEM_TWO_LINE, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(item != NULL);

  rc = md3_list_item_set_headline(NULL, "h");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_set_headline(item, "Headline text");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_item_set_headline(item, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', item->headline[0]);

  rc = md3_list_item_set_supporting_text(NULL, "s");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_set_supporting_text(item, "Supporting line text");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_item_set_supporting_text(item, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', item->supporting_text[0]);

  rc = md3_list_item_set_trailing_supporting_text(NULL, "t");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_set_trailing_supporting_text(item, "10m ago");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_item_set_trailing_supporting_text(item, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', item->trailing_supporting_text[0]);

  rc = md3_list_item_set_selected(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_set_selected(item, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_item_get_selected(NULL, &selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_get_selected(item, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_get_selected(item, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);
  rc = md3_list_item_set_selected(item, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_item_get_selected(item, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, selected);

  rc = md3_list_item_get_component(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_item_get_component(item, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  memset(&dummy_item, 0, sizeof(dummy_item));
  rc = md3_list_item_get_component(&dummy_item, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_list_item_get_component(item, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  rc = md3_list_append_item(NULL, item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_append_item(list, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_append_item(list, &dummy_item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  {
    struct md3_list dummy_list;
    memset(&dummy_list, 0, sizeof(dummy_list));
    rc = md3_list_append_item(&dummy_list, item);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }

  rc = md3_list_append_item(list, item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_list_item_destroy(item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_destroy(list);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

static ui_error_t mock_carousel_create_node(size_t index,
                                            struct ui_dom_node **out_node,
                                            void *user_data) {
  if (index > 1000 || user_data == (void *)1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, out_node);
}

static ui_error_t mock_carousel_update_node(size_t index,
                                            struct ui_dom_node *node,
                                            void *user_data) {
  if (index > 1000 || !node || user_data == (void *)1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

TEST test_md3_carousel_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_carousel *carousel = NULL;
  struct ui_carousel_config cfg;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  memset(&cfg, 0, sizeof(cfg));
  cfg.orientation = UI_CAROUSEL_ORIENTATION_HORIZONTAL;
  cfg.initial_item_count = 5;
  cfg.item_size = 200.0f;
  cfg.create_node = mock_carousel_create_node;
  cfg.update_node = mock_carousel_update_node;

  /* Invalid arguments */
  rc = md3_carousel_create(NULL, MD3_CAROUSEL_LAYOUT_MULTI_BROWSE, &cfg,
                           &carousel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_carousel_create(dummy_engine, (enum md3_carousel_layout_type) - 1,
                           &cfg, &carousel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_carousel_create(dummy_engine, MD3_CAROUSEL_LAYOUT_COUNT, &cfg,
                           &carousel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_carousel_create(dummy_engine, MD3_CAROUSEL_LAYOUT_MULTI_BROWSE, NULL,
                           &carousel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_carousel_create(dummy_engine, MD3_CAROUSEL_LAYOUT_MULTI_BROWSE, &cfg,
                           NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_carousel_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_carousel_set_mask_morphing(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_carousel_set_corner_radius(NULL, 32.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_carousel_scroll_to(NULL, 0, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_carousel_get_component(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_carousel_create(dummy_engine, MD3_CAROUSEL_LAYOUT_MULTI_BROWSE, &cfg,
                           &carousel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(carousel != NULL);

  rc = md3_carousel_get_component(carousel, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_carousel_set_corner_radius(carousel, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_carousel_set_mask_morphing(carousel, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, carousel->dynamic_mask_morphing);

  rc = md3_carousel_set_mask_morphing(carousel, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, carousel->dynamic_mask_morphing);

  rc = md3_carousel_set_corner_radius(carousel, 32.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_carousel_scroll_to(carousel, 2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_carousel_get_component(carousel, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  {
    struct ui_carousel_base *saved_base = carousel->base;
    carousel->base = NULL;
    rc = md3_carousel_scroll_to(carousel, 0, 0);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_carousel_get_component(carousel, &comp);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    carousel->base = saved_base;
  }

  rc = md3_carousel_destroy(carousel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_chip_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_chip *chip = NULL;
  const char *label = NULL;
  size_t count = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_chip_create(NULL, MD3_CHIP_FILTER, &chip, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chip_create(dummy_engine, (enum md3_chip_type) - 1, &chip, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chip_create(dummy_engine, MD3_CHIP_TYPE_COUNT, &chip, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_chip_create(dummy_engine, MD3_CHIP_FILTER, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_set_elevated(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_add(NULL, "First Chip");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_remove(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_get_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_get_label(NULL, 0, &label);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_create(dummy_engine, MD3_CHIP_FILTER, &chip, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(chip != NULL);

  rc = md3_chip_get_count(chip, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_get_label(chip, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_add(chip, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_chip_set_elevated(chip, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, chip->elevated);

  rc = md3_chip_set_elevated(chip, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, chip->elevated);

  rc = md3_chip_add(chip, "First Chip");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_chip_add(chip, "Second Chip");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_chip_get_count(chip, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count);

  rc = md3_chip_get_label(chip, 0, &label);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("First Chip", label);

  rc = md3_chip_get_label(chip, 999, &label);
  ASSERT(rc != UI_ERROR_NONE);

  rc = md3_chip_remove(chip, 999);
  ASSERT(rc != UI_ERROR_NONE);

  rc = md3_chip_remove(chip, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_chip_get_count(chip, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, count);

  {
    struct md3_chip *chip2 = NULL;
    rc = md3_chip_create(dummy_engine, MD3_CHIP_FILTER, &chip2, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(chip2 != NULL);
    ui_chips_base_destroy(chip2->base);
    chip2->base = NULL;
    rc = md3_chip_destroy(chip2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_chip_destroy(chip);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_navigation_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_navigation_bar *bar = NULL;
  struct md3_navigation_rail *rail = NULL;
  struct md3_navigation_drawer *drawer = NULL;
  struct md3_navigation_suite *suite = NULL;
  struct md3_top_app_bar *top_bar = NULL;
  struct md3_bottom_app_bar *bottom_bar = NULL;
  struct md3_tabs *tabs = NULL;
  size_t idx = 0;
  ui_error_t rc;

  /* --- Navigation Bar --- */
  /* Null checks */
  rc = md3_navigation_bar_create(NULL, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_bar_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_bar_add_item(NULL, "Home", "home_icon");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_bar_set_selected(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_bar_get_selected(NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = md3_navigation_bar_create(dummy_engine, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  rc = md3_navigation_bar_get_selected(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_navigation_bar_add_item(bar, NULL, "home_icon");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_bar_add_item(bar, "Home", "home_icon");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_navigation_bar_add_item(bar, "Search", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_navigation_bar_set_selected(bar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_navigation_bar_get_selected(bar, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  rc = md3_navigation_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* OOM branches for bar */
#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_navigation_bar_create(dummy_engine, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_navigation_bar_create(dummy_engine, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
  rc = md3_navigation_bar_create(dummy_engine, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  {
    int k;
    for (k = 0; k < 15; k++) {
      g_malloc_fail_countdown = k;
      rc = md3_navigation_bar_add_item(bar, "OOM", "icon");
      if (rc == UI_ERROR_NONE) {
        break;
      }
      ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    }
  }

  g_malloc_fail_countdown = -1;
  rc = md3_navigation_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif

  /* --- Navigation Rail --- */
  rc = md3_navigation_rail_create(NULL, &rail);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_rail_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_rail_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_rail_set_expanded(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_navigation_rail_create(dummy_engine, &rail);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(rail != NULL);
  rc = md3_navigation_rail_set_expanded(rail, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, rail->expanded);
  rc = md3_navigation_rail_set_expanded(rail, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, rail->expanded);
  rc = md3_navigation_rail_destroy(rail);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_navigation_rail_create(dummy_engine, &rail);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_navigation_rail_create(dummy_engine, &rail);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Navigation Drawer --- */
  rc = md3_navigation_drawer_create(NULL, MD3_DRAWER_STANDARD, &drawer);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_drawer_create(dummy_engine, MD3_DRAWER_STANDARD, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_drawer_create(
      dummy_engine, (enum md3_navigation_drawer_variant)99, &drawer);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_drawer_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_drawer_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_navigation_drawer_create(dummy_engine, MD3_DRAWER_MODAL, &drawer);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(drawer != NULL);
  rc = md3_navigation_drawer_set_open(drawer, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, drawer->is_open);
  rc = md3_navigation_drawer_set_open(drawer, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, drawer->is_open);
  rc = md3_navigation_drawer_destroy(drawer);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_navigation_drawer_create(dummy_engine, MD3_DRAWER_STANDARD, &drawer);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_navigation_drawer_create(dummy_engine, MD3_DRAWER_STANDARD, &drawer);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Navigation Suite --- */
  rc = md3_navigation_suite_create(NULL, &suite);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_suite_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_suite_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_suite_update_size_class(NULL, MD3_WINDOW_SIZE_COMPACT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_navigation_suite_create(dummy_engine, &suite);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(suite != NULL);

  rc = md3_navigation_suite_update_size_class(suite,
                                              (enum md3_window_size_class)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_suite_update_size_class(suite, MD3_WINDOW_SIZE_COMPACT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_navigation_suite_update_size_class(suite, MD3_WINDOW_SIZE_MEDIUM);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_navigation_suite_update_size_class(suite, MD3_WINDOW_SIZE_EXPANDED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_WINDOW_SIZE_EXPANDED, suite->current_class);

  rc = md3_navigation_suite_destroy(suite);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  /* Suite OOM paths */
  {
    int k;
    for (k = 0; k < 25; k++) {
      g_malloc_fail_countdown = k;
      rc = md3_navigation_suite_create(dummy_engine, &suite);
      if (rc == UI_ERROR_NONE) {
        md3_navigation_suite_destroy(suite);
        suite = NULL;
        break;
      }
      ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    }
  }
  g_malloc_fail_countdown = -1;
#endif

  /* --- Top App Bar --- */
  rc = md3_top_app_bar_create(NULL, MD3_TOP_APP_BAR_SMALL, "T", &top_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_create(dummy_engine, MD3_TOP_APP_BAR_SMALL, "T", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_create(dummy_engine, (enum md3_top_app_bar_variant)99,
                              "T", &top_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_set_scroll_offset(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Center-aligned variant without title */
  rc = md3_top_app_bar_create(dummy_engine, MD3_TOP_APP_BAR_CENTER_ALIGNED,
                              NULL, &top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(top_bar != NULL);
  rc = md3_top_app_bar_destroy(top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Medium variant with title */
  rc = md3_top_app_bar_create(dummy_engine, MD3_TOP_APP_BAR_MEDIUM,
                              "Medium Title", &top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(top_bar != NULL);
  rc = md3_top_app_bar_set_scroll_offset(top_bar, 20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_top_app_bar_destroy(top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Large variant with title */
  rc = md3_top_app_bar_create(dummy_engine, MD3_TOP_APP_BAR_LARGE, "Title",
                              &top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(top_bar != NULL);
  rc = md3_top_app_bar_set_scroll_offset(top_bar, 45.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_top_app_bar_destroy(top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_top_app_bar_create(dummy_engine, MD3_TOP_APP_BAR_LARGE, "Title",
                              &top_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_top_app_bar_create(dummy_engine, MD3_TOP_APP_BAR_LARGE, "Title",
                              &top_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 2;
  rc = md3_top_app_bar_create(dummy_engine, MD3_TOP_APP_BAR_LARGE, "Title",
                              &top_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Bottom App Bar --- */
  rc = md3_bottom_app_bar_create(NULL, 1, &bottom_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bottom_app_bar_create(dummy_engine, 1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bottom_app_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_bottom_app_bar_create(dummy_engine, 0, &bottom_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bottom_bar != NULL);
  ASSERT_EQ(0, bottom_bar->has_fab);
  rc = md3_bottom_app_bar_destroy(bottom_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_bottom_app_bar_create(dummy_engine, 1, &bottom_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bottom_bar != NULL);
  ASSERT_EQ(1, bottom_bar->has_fab);
  rc = md3_bottom_app_bar_destroy(bottom_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_bottom_app_bar_create(dummy_engine, 1, &bottom_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_bottom_app_bar_create(dummy_engine, 1, &bottom_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Tabs --- */
  rc = md3_tabs_create(NULL, MD3_TABS_PRIMARY, &tabs);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tabs_create(dummy_engine, MD3_TABS_PRIMARY, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tabs_create(dummy_engine, (enum md3_tabs_variant)99, &tabs);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tabs_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tabs_set_selected(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tabs_get_selected(NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  {
    struct ui_dom_node *h1 = NULL, *p1 = NULL;
    struct ui_dom_node *h2 = NULL, *p2 = NULL;
    rc = md3_tabs_create(dummy_engine, MD3_TABS_EXPRESSIVE_PILL, &tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(tabs != NULL);

    rc = md3_tabs_get_selected(tabs, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_add_tab(tabs->base, "t1", h1, p1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_add_tab(tabs->base, "t2", h2, p2);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = md3_tabs_set_selected(tabs, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_tabs_get_selected(tabs, &idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(1, idx);

    rc = md3_tabs_destroy(tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_tabs_create(dummy_engine, MD3_TABS_PRIMARY, &tabs);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_tabs_create(dummy_engine, MD3_TABS_PRIMARY, &tabs);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  PASS();
}

TEST test_md3_overlay_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_dialog *dialog = NULL;
  struct md3_bottom_sheet *bottom_sheet = NULL;
  struct md3_side_sheet *side_sheet = NULL;
  struct md3_menu *menu = NULL;
  struct md3_tooltip *tooltip = NULL;
  struct md3_snackbar *snackbar = NULL;
  struct md3_badge *badge = NULL;
  struct md3_datepicker *datepicker = NULL;
  struct md3_timepicker *timepicker = NULL;
  struct md3_pull_to_refresh *ptr = NULL;
  ui_error_t rc;

  /* --- Dialog --- */
  rc = md3_dialog_create(NULL, MD3_DIALOG_ALERT, &dialog);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dialog_create(dummy_engine, MD3_DIALOG_ALERT, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dialog_create(dummy_engine, (enum md3_dialog_variant)99, &dialog);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dialog_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dialog_set_headline(NULL, "H");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dialog_set_supporting_text(NULL, "S");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_dialog_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_dialog_create(dummy_engine, MD3_DIALOG_ALERT, &dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(dialog != NULL);
  rc = md3_dialog_set_headline(dialog, "Alert title");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_dialog_set_headline(dialog, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_dialog_set_supporting_text(dialog, "Alert details text");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_dialog_set_supporting_text(dialog, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_dialog_set_open(dialog, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_dialog_set_open(dialog, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_dialog_destroy(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_dialog_create(dummy_engine, MD3_DIALOG_ALERT, &dialog);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_dialog_create(dummy_engine, MD3_DIALOG_ALERT, &dialog);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Bottom sheet --- */
  rc = md3_bottom_sheet_create(NULL, MD3_BOTTOM_SHEET_FLOATING, &bottom_sheet);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bottom_sheet_create(dummy_engine, MD3_BOTTOM_SHEET_FLOATING, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bottom_sheet_create(dummy_engine, (enum md3_bottom_sheet_variant)99,
                               &bottom_sheet);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bottom_sheet_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bottom_sheet_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_bottom_sheet_create(dummy_engine, MD3_BOTTOM_SHEET_FLOATING,
                               &bottom_sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bottom_sheet != NULL);
  rc = md3_bottom_sheet_set_open(bottom_sheet, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_bottom_sheet_set_open(bottom_sheet, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_bottom_sheet_destroy(bottom_sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_bottom_sheet_create(dummy_engine, MD3_BOTTOM_SHEET_STANDARD,
                               &bottom_sheet);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_bottom_sheet_create(dummy_engine, MD3_BOTTOM_SHEET_STANDARD,
                               &bottom_sheet);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Side sheet --- */
  rc = md3_side_sheet_create(NULL, 1, &side_sheet);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_side_sheet_create(dummy_engine, 1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_side_sheet_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_side_sheet_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_side_sheet_create(dummy_engine, 1, &side_sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(side_sheet != NULL);
  rc = md3_side_sheet_set_open(side_sheet, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_side_sheet_set_open(side_sheet, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_side_sheet_destroy(side_sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_side_sheet_create(dummy_engine, 0, &side_sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(side_sheet != NULL);
  rc = md3_side_sheet_destroy(side_sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_side_sheet_create(dummy_engine, 0, &side_sheet);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_side_sheet_create(dummy_engine, 0, &side_sheet);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Menu --- */
  rc = md3_menu_create(NULL, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_menu_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_menu_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_menu_create(dummy_engine, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(menu != NULL);
  rc = md3_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_menu_create(dummy_engine, &menu);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_menu_create(dummy_engine, &menu);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Tooltip --- */
  rc = md3_tooltip_create(NULL, MD3_TOOLTIP_PLAIN, "T", &tooltip);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tooltip_create(dummy_engine, MD3_TOOLTIP_PLAIN, "T", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tooltip_create(dummy_engine, (enum md3_tooltip_variant)99, "T",
                          &tooltip);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_tooltip_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_tooltip_create(dummy_engine, MD3_TOOLTIP_RICH, "Tooltip message",
                          &tooltip);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tooltip != NULL);
  rc = md3_tooltip_destroy(tooltip);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_tooltip_create(dummy_engine, MD3_TOOLTIP_PLAIN, NULL, &tooltip);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tooltip != NULL);
  rc = md3_tooltip_destroy(tooltip);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_tooltip_create(dummy_engine, MD3_TOOLTIP_PLAIN, "T", &tooltip);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_tooltip_create(dummy_engine, MD3_TOOLTIP_PLAIN, "T", &tooltip);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Snackbar --- */
  rc = md3_snackbar_create(NULL, MD3_SNACKBAR_SINGLE_LINE, "M", &snackbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_snackbar_create(dummy_engine, MD3_SNACKBAR_SINGLE_LINE, "M", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_snackbar_create(dummy_engine, (enum md3_snackbar_variant)99, "M",
                           &snackbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_snackbar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_snackbar_create(dummy_engine, MD3_SNACKBAR_FLOATING_PILL,
                           "Notice message", &snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(snackbar != NULL);
  rc = md3_snackbar_destroy(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc =
      md3_snackbar_create(dummy_engine, MD3_SNACKBAR_TWO_LINE, NULL, &snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(snackbar != NULL);
  rc = md3_snackbar_destroy(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  {
    int k;
    for (k = 0; k < 15; k++) {
      g_malloc_fail_countdown = k;
      rc = md3_snackbar_create(dummy_engine, MD3_SNACKBAR_SINGLE_LINE, "M",
                               &snackbar);
      if (rc == UI_ERROR_NONE) {
        md3_snackbar_destroy(snackbar);
        snackbar = NULL;
        break;
      }
      ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    }
    g_malloc_fail_countdown = -1;
  }
#endif

  /* --- Badge --- */
  rc = md3_badge_create(NULL, 5, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_badge_create(dummy_engine, 5, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_badge_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_badge_set_count(NULL, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_badge_create(dummy_engine, -1, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);
  ASSERT_EQ(0, badge->has_count);
  rc = md3_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_badge_create(dummy_engine, 5, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);
  ASSERT_EQ(5, badge->count);
  rc = md3_badge_set_count(badge, 12);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(12, badge->count);
  rc = md3_badge_set_count(badge, -2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, badge->has_count);
  rc = md3_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_badge_create(dummy_engine, 0, &badge);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_badge_create(dummy_engine, 0, &badge);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Datepicker --- */
  rc = md3_datepicker_create(NULL, &datepicker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_datepicker_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_datepicker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_datepicker_create(dummy_engine, &datepicker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(datepicker != NULL);
  rc = md3_datepicker_destroy(datepicker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  {
    int k;
    for (k = 0; k < 100; k++) {
      g_malloc_fail_countdown = k;
      rc = md3_datepicker_create(dummy_engine, &datepicker);
      if (rc == UI_ERROR_NONE) {
        md3_datepicker_destroy(datepicker);
        datepicker = NULL;
        break;
      }
      ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    }
    g_malloc_fail_countdown = -1;
  }
#endif

  /* --- Timepicker --- */
  rc = md3_timepicker_create(NULL, 1, &timepicker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timepicker_create(dummy_engine, 1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_timepicker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_timepicker_create(dummy_engine, 1, &timepicker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(timepicker != NULL);
  ASSERT_EQ(1, timepicker->is_24h);
  rc = md3_timepicker_destroy(timepicker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_timepicker_create(dummy_engine, 0, &timepicker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(timepicker != NULL);
  ASSERT_EQ(0, timepicker->is_24h);
  rc = md3_timepicker_destroy(timepicker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_timepicker_create(dummy_engine, 1, &timepicker);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_timepicker_create(dummy_engine, 1, &timepicker);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  /* --- Pull-to-refresh --- */
  rc = md3_pull_to_refresh_create(NULL, &ptr);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pull_to_refresh_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pull_to_refresh_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_pull_to_refresh_create(dummy_engine, &ptr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ptr != NULL);
  rc = md3_pull_to_refresh_destroy(ptr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_pull_to_refresh_create(dummy_engine, &ptr);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_pull_to_refresh_create(dummy_engine, &ptr);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  PASS();
}

SUITE(md3_components_suite) {
  RUN_TEST(test_md3_shape_morph_creation);
  RUN_TEST(test_md3_shape_morph_interpolation);
  RUN_TEST(test_md3_spring_physics);
  RUN_TEST(test_md3_motion_cubic_bezier);
  RUN_TEST(test_md3_button_lifecycle);
  RUN_TEST(test_md3_split_button_lifecycle);
  RUN_TEST(test_md3_segmented_button_lifecycle);
  RUN_TEST(test_md3_icon_button_lifecycle);
  RUN_TEST(test_md3_fab_lifecycle);
  RUN_TEST(test_md3_checkbox_lifecycle);
  RUN_TEST(test_md3_radio_group_lifecycle);
  RUN_TEST(test_md3_switch_lifecycle);
  RUN_TEST(test_md3_text_field_lifecycle);
  RUN_TEST(test_md3_search_lifecycle);
  RUN_TEST(test_md3_slider_lifecycle);
  RUN_TEST(test_md3_progress_lifecycle);
  RUN_TEST(test_md3_card_lifecycle);
  RUN_TEST(test_md3_divider_lifecycle);
  RUN_TEST(test_md3_list_lifecycle);
  RUN_TEST(test_md3_carousel_lifecycle);
  RUN_TEST(test_md3_chip_lifecycle);
  RUN_TEST(test_md3_navigation_lifecycle);
  RUN_TEST(test_md3_overlay_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_components_suite);
  GREATEST_MAIN_END();
}
