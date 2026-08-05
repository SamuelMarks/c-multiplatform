/* clang-format off */
#include "ui_css_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_types.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

#define EXPECT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      fprintf(stderr,                                                          \
              "%s:%d: EXPECT_EQ failed: %" UI_PRId64 " != %" UI_PRId64 "\n",   \
              __FILE__, __LINE__, (ui_int64)(expected), (ui_int64)(actual));   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define EXPECT_FLOAT_EQ(expected, actual)                                      \
  do {                                                                         \
    float diff = (expected) - (actual);                                        \
    if (diff < -0.001f || diff > 0.001f) {                                     \
      fprintf(stderr, "%s:%d: EXPECT_FLOAT_EQ failed: %f != %f\n", __FILE__,   \
              __LINE__, (float)(expected), (float)(actual));                   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static ui_error_t test_parse_outline(void) {
  struct ui_css_outline outline;
  struct ui_css_value width;
  enum ui_css_outline_style style;
  ui_error_t err;

  /* Null checks */
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_outline_width(NULL, &width));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_outline_width("thin", NULL));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_outline_style(NULL, &style));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_outline_style("solid", NULL));

  err = ui_css_parse_outline("   solid 5px #ff0000", &outline);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_SOLID, outline.style);
  EXPECT_EQ(UI_CSS_UNIT_PX, outline.width.unit);
  EXPECT_FLOAT_EQ(5.0f, outline.width.value);
  EXPECT_FLOAT_EQ(1.0f, outline.color.components[0]); /* red */
  EXPECT_EQ(0, outline.is_color_invert);

  err = ui_css_parse_outline("invert dotted thick", &outline);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_DOTTED, outline.style);
  EXPECT_EQ(UI_CSS_UNIT_PX, outline.width.unit);
  EXPECT_FLOAT_EQ(5.0f, outline.width.value);
  EXPECT_EQ(1, outline.is_color_invert);

  err = ui_css_parse_outline("thin none", &outline);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_NONE, outline.style);
  EXPECT_FLOAT_EQ(1.0f, outline.width.value);

  err = ui_css_parse_outline("medium hidden", &outline);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_HIDDEN, outline.style);
  EXPECT_FLOAT_EQ(3.0f, outline.width.value);

  err = ui_css_parse_outline(
      "dashed auto",
      &outline); /* Too many styles not supported, one will fail color parse */
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  ui_css_parse_outline_style("dashed", &outline.style);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_DASHED, outline.style);
  ui_css_parse_outline_style("double", &outline.style);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_DOUBLE, outline.style);
  ui_css_parse_outline_style("groove", &outline.style);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_GROOVE, outline.style);
  ui_css_parse_outline_style("ridge", &outline.style);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_RIDGE, outline.style);
  ui_css_parse_outline_style("inset", &outline.style);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_INSET, outline.style);
  ui_css_parse_outline_style("outset", &outline.style);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_OUTSET, outline.style);
  ui_css_parse_outline_style("auto", &outline.style);
  EXPECT_EQ(UI_CSS_OUTLINE_STYLE_AUTO, outline.style);

  err = ui_css_parse_outline("none", &outline);
  EXPECT_EQ(UI_ERROR_NONE, err);

  err = ui_css_parse_outline("double solid", &outline);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);
  err = ui_css_parse_outline("1px 2px", &outline);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);
  /* Test too many outline values */
  err = ui_css_parse_outline("5px solid #fff auto", &outline);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  err = ui_css_parse_outline("5px solid unknown_color", &outline);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  err = ui_css_parse_outline("invert #fff", &outline);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_outline(NULL, &outline));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_outline("solid", NULL));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_outline_width(NULL, &outline.width));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_outline_style(NULL, &outline.style));

  /* Truncation tests */
  {
    char long_buf[400];
    memset(long_buf, 'a', 399);
    long_buf[399] = '\0';
    err = ui_css_parse_outline(long_buf, &outline);
    EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);
  }

  return UI_ERROR_NONE;
}

static ui_error_t test_parse_cursor(void) {
  struct ui_css_cursor cursor;
  ui_error_t err;

  err = ui_css_parse_cursor("pointer", &cursor);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_CURSOR_POINTER, cursor.keyword);
  EXPECT_EQ(NULL, cursor.images);
  ui_css_cursor_destroy(&cursor);

  err = ui_css_parse_cursor("url(custom.png) 10 20, auto", &cursor);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_CURSOR_AUTO, cursor.keyword);
  if (!cursor.images) {
    fprintf(stderr, "Expected cursor image\n");
    exit(1);
    return UI_ERROR_NONE;
  }
  EXPECT_EQ(1, cursor.images->has_hotspot);
  EXPECT_FLOAT_EQ(10.0f, cursor.images->hotspot_x);
  EXPECT_FLOAT_EQ(20.0f, cursor.images->hotspot_y);
  ui_css_cursor_destroy(&cursor);

  err = ui_css_parse_cursor("url(custom.png), url(other.png), none", &cursor);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_CURSOR_NONE, cursor.keyword);
  EXPECT_EQ(0, cursor.images->has_hotspot);
  ui_css_cursor_destroy(&cursor);

  /* long url test to hit strncpy truncation */
  {
    char long_url[1024] = "url(";
    memset(long_url + 4, 'A', 600);
    strcpy(long_url + 604, "), auto");
    err = ui_css_parse_cursor(long_url, &cursor);
    EXPECT_EQ(UI_ERROR_NONE, err);
    ui_css_cursor_destroy(&cursor);
  }

  /* Test image with unparseable coords */
  err = ui_css_parse_cursor("url(test.png) invalid 20, auto", &cursor);
  EXPECT_EQ(UI_ERROR_NONE, err);
  if (cursor.images) {
    EXPECT_EQ(0, cursor.images->has_hotspot);
  }
  ui_css_cursor_destroy(&cursor);

  err = ui_css_parse_cursor("image-set('test.png' 1x), auto", &cursor);
  EXPECT_EQ(UI_ERROR_NONE, err);
  ui_css_cursor_destroy(&cursor);

  err = ui_css_parse_cursor("-webkit-image-set('test.png' 1x), auto", &cursor);
  EXPECT_EQ(UI_ERROR_NONE, err);
  ui_css_cursor_destroy(&cursor);

  /* test invalid image parse falls back gracefully */
  err = ui_css_parse_cursor("url(), auto", &cursor);
  EXPECT_EQ(UI_ERROR_NONE, err);
  ui_css_cursor_destroy(&cursor);

  /* Truncation tests */
  {
    char long_buf[1200];
    memset(long_buf, 'a', 1199);
    long_buf[1199] = '\0';
    err = ui_css_parse_cursor(long_buf, &cursor);
    EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);
    ui_css_cursor_destroy(&cursor);
  }

  ui_css_parse_cursor("default", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_DEFAULT, cursor.keyword);
  ui_css_parse_cursor("context-menu", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_CONTEXT_MENU, cursor.keyword);
  ui_css_parse_cursor("help", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_HELP, cursor.keyword);
  ui_css_parse_cursor("progress", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_PROGRESS, cursor.keyword);
  ui_css_parse_cursor("wait", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_WAIT, cursor.keyword);
  ui_css_parse_cursor("cell", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_CELL, cursor.keyword);
  ui_css_parse_cursor("crosshair", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_CROSSHAIR, cursor.keyword);
  ui_css_parse_cursor("text", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_TEXT, cursor.keyword);
  ui_css_parse_cursor("vertical-text", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_VERTICAL_TEXT, cursor.keyword);
  ui_css_parse_cursor("alias", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_ALIAS, cursor.keyword);
  ui_css_parse_cursor("copy", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_COPY, cursor.keyword);
  ui_css_parse_cursor("move", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_MOVE, cursor.keyword);
  ui_css_parse_cursor("no-drop", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_NO_DROP, cursor.keyword);
  ui_css_parse_cursor("not-allowed", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_NOT_ALLOWED, cursor.keyword);
  ui_css_parse_cursor("grab", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_GRAB, cursor.keyword);
  ui_css_parse_cursor("grabbing", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_GRABBING, cursor.keyword);
  ui_css_parse_cursor("e-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_E_RESIZE, cursor.keyword);
  ui_css_parse_cursor("n-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_N_RESIZE, cursor.keyword);
  ui_css_parse_cursor("ne-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_NE_RESIZE, cursor.keyword);
  ui_css_parse_cursor("nw-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_NW_RESIZE, cursor.keyword);
  ui_css_parse_cursor("s-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_S_RESIZE, cursor.keyword);
  ui_css_parse_cursor("se-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_SE_RESIZE, cursor.keyword);
  ui_css_parse_cursor("sw-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_SW_RESIZE, cursor.keyword);
  ui_css_parse_cursor("w-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_W_RESIZE, cursor.keyword);
  ui_css_parse_cursor("ew-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_EW_RESIZE, cursor.keyword);
  ui_css_parse_cursor("ns-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_NS_RESIZE, cursor.keyword);
  ui_css_parse_cursor("nesw-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_NESW_RESIZE, cursor.keyword);
  ui_css_parse_cursor("nwse-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_NWSE_RESIZE, cursor.keyword);
  ui_css_parse_cursor("col-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_COL_RESIZE, cursor.keyword);
  ui_css_parse_cursor("row-resize", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_ROW_RESIZE, cursor.keyword);
  ui_css_parse_cursor("all-scroll", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_ALL_SCROLL, cursor.keyword);
  ui_css_parse_cursor("zoom-in", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_ZOOM_IN, cursor.keyword);
  ui_css_parse_cursor("zoom-out", &cursor);
  EXPECT_EQ(UI_CSS_CURSOR_ZOOM_OUT, cursor.keyword);

  err = ui_css_parse_cursor("invalid_cursor", &cursor);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_cursor(NULL, &cursor));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_cursor("auto", NULL));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_cursor_destroy(NULL));

  /* test parsing malloc failure */
  g_malloc_fail_countdown = 0;
  err = ui_css_parse_cursor("url(test.png), auto", &cursor);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, err);
  g_malloc_fail_countdown = -1;

  /* test parsing malformed url */
  err = ui_css_parse_cursor("url(malformed, auto", &cursor);
  /* The parsing ignores the malformed URL image and parses the keyword 'auto',
   * returning NONE */
  EXPECT_EQ(UI_ERROR_NONE, err);
  return UI_ERROR_NONE;
}

static ui_error_t test_parse_user_select(void) {
  enum ui_css_user_select select;
  ui_error_t err;

  err = ui_css_parse_user_select("none", &select);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_USER_SELECT_NONE, select);

  ui_css_parse_user_select("auto", &select);
  EXPECT_EQ(UI_CSS_USER_SELECT_AUTO, select);
  ui_css_parse_user_select("text", &select);
  EXPECT_EQ(UI_CSS_USER_SELECT_TEXT, select);
  ui_css_parse_user_select("all", &select);
  EXPECT_EQ(UI_CSS_USER_SELECT_ALL, select);
  ui_css_parse_user_select("contain", &select);
  EXPECT_EQ(UI_CSS_USER_SELECT_CONTAIN, select);

  err = ui_css_parse_user_select("invalid", &select);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_user_select(NULL, &select));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_user_select("auto", NULL));
  return UI_ERROR_NONE;
}

static ui_error_t test_parse_appearance(void) {
  enum ui_css_appearance appearance;
  ui_error_t err;

  err = ui_css_parse_appearance("button", &appearance);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_APPEARANCE_BUTTON, appearance);

  ui_css_parse_appearance("none", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_NONE, appearance);
  ui_css_parse_appearance("auto", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_AUTO, appearance);
  ui_css_parse_appearance("menulist-button", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_MENULIST_BUTTON, appearance);
  ui_css_parse_appearance("textfield", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_TEXTFIELD, appearance);
  ui_css_parse_appearance("searchfield", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_SEARCHFIELD, appearance);
  ui_css_parse_appearance("textarea", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_TEXTAREA, appearance);
  ui_css_parse_appearance("push-button", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_PUSH_BUTTON, appearance);
  ui_css_parse_appearance("slider-horizontal", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_SLIDER_HORIZONTAL, appearance);
  ui_css_parse_appearance("checkbox", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_CHECKBOX, appearance);
  ui_css_parse_appearance("radio", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_RADIO, appearance);
  ui_css_parse_appearance("square-button", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_SQUARE_BUTTON, appearance);
  ui_css_parse_appearance("menulist", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_MENULIST, appearance);
  ui_css_parse_appearance("listbox", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_LISTBOX, appearance);
  ui_css_parse_appearance("meter", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_METER, appearance);
  ui_css_parse_appearance("progress-bar", &appearance);
  EXPECT_EQ(UI_CSS_APPEARANCE_PROGRESS_BAR, appearance);

  err = ui_css_parse_appearance("invalid", &appearance);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_appearance(NULL, &appearance));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_appearance("none", NULL));
  return UI_ERROR_NONE;
}

static ui_error_t test_parse_pointer_events(void) {
  enum ui_css_pointer_events events;
  ui_error_t err;

  err = ui_css_parse_pointer_events("none", &events);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_NONE, events);

  ui_css_parse_pointer_events("auto", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_AUTO, events);
  ui_css_parse_pointer_events("visiblePainted", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_VISIBLE_PAINTED, events);
  ui_css_parse_pointer_events("visibleFill", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_VISIBLE_FILL, events);
  ui_css_parse_pointer_events("visibleStroke", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_VISIBLE_STROKE, events);
  ui_css_parse_pointer_events("visible", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_VISIBLE, events);
  ui_css_parse_pointer_events("painted", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_PAINTED, events);
  ui_css_parse_pointer_events("fill", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_FILL, events);
  ui_css_parse_pointer_events("stroke", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_STROKE, events);
  ui_css_parse_pointer_events("all", &events);
  EXPECT_EQ(UI_CSS_POINTER_EVENTS_ALL, events);

  err = ui_css_parse_pointer_events("invalid", &events);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_pointer_events(NULL, &events));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_pointer_events("none", NULL));
  return UI_ERROR_NONE;
}

static ui_error_t test_parse_overscroll_behavior(void) {
  struct ui_css_overscroll_behavior_shorthand shorthand;
  enum ui_css_overscroll_behavior behavior;
  ui_error_t err;

  err = ui_css_parse_overscroll_behavior("none", &shorthand);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_OVERSCROLL_BEHAVIOR_NONE, shorthand.x);
  EXPECT_EQ(UI_CSS_OVERSCROLL_BEHAVIOR_NONE, shorthand.y);

  err = ui_css_parse_overscroll_behavior("contain auto", &shorthand);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_OVERSCROLL_BEHAVIOR_CONTAIN, shorthand.x);
  EXPECT_EQ(UI_CSS_OVERSCROLL_BEHAVIOR_AUTO, shorthand.y);

  err = ui_css_parse_overscroll_behavior("invalid", &shorthand);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  err = ui_css_parse_overscroll_behavior("auto contain none", &shorthand);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  err = ui_css_parse_overscroll_behavior("auto invalid", &shorthand);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  err = ui_css_parse_overscroll_behavior("  ", &shorthand);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_overscroll_behavior(NULL, &shorthand));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_overscroll_behavior("none", NULL));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_overscroll_behavior_axis(NULL, &behavior));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_overscroll_behavior_axis("none", NULL));

  err = ui_css_parse_overscroll_behavior_axis("auto", &behavior);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_OVERSCROLL_BEHAVIOR_AUTO, behavior);

  err = ui_css_parse_overscroll_behavior_axis("none", &behavior);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(UI_CSS_OVERSCROLL_BEHAVIOR_NONE, behavior);

  /* Truncation tests */
  {
    char long_buf[400];
    memset(long_buf, 'a', 399);
    long_buf[399] = '\0';
    err = ui_css_parse_overscroll_behavior(long_buf, &shorthand);
    EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);
  }

  return UI_ERROR_NONE;
}

int main(void) {
  test_parse_outline();
  test_parse_cursor();
  test_parse_user_select();
  test_parse_appearance();
  test_parse_pointer_events();
  test_parse_overscroll_behavior();
  printf("All CSS UI tests passed.\n");
  return 0;
}
