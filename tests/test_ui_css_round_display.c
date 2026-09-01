/* clang-format off */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ui_css_round_display.h"
/* clang-format on */

#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Assertion failed: %s at %s:%d\n", #cond, __FILE__,      \
              __LINE__);                                                       \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static int test_parse_shape_inside(void) {
  struct ui_css_shape_inside shape;

  /* Null arguments */
  TEST_ASSERT(ui_css_parse_shape_inside(NULL, &shape) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_parse_shape_inside("auto", NULL) ==
              UI_ERROR_INVALID_ARGUMENT);

  /* auto */
  TEST_ASSERT(ui_css_parse_shape_inside("auto", &shape) == UI_ERROR_NONE);
  TEST_ASSERT(shape.is_display == 0);
  TEST_ASSERT(shape.is_outside_shape == 0);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_NONE);

  /* whitespace */
  TEST_ASSERT(ui_css_parse_shape_inside("   auto", &shape) == UI_ERROR_NONE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_NONE);

  /* display */
  TEST_ASSERT(ui_css_parse_shape_inside("display", &shape) == UI_ERROR_NONE);
  TEST_ASSERT(shape.is_display == 1);

  /* outside-shape */
  TEST_ASSERT(ui_css_parse_shape_inside("outside-shape", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.is_outside_shape == 1);

  /* Boxes alone */
  TEST_ASSERT(ui_css_parse_shape_inside("margin-box", &shape) == UI_ERROR_NONE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_MARGIN_BOX);

  TEST_ASSERT(ui_css_parse_shape_inside("border-box", &shape) == UI_ERROR_NONE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_BORDER_BOX);

  TEST_ASSERT(ui_css_parse_shape_inside("padding-box", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_PADDING_BOX);

  TEST_ASSERT(ui_css_parse_shape_inside("content-box", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_CONTENT_BOX);

  TEST_ASSERT(ui_css_parse_shape_inside("fill-box", &shape) == UI_ERROR_NONE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_FILL_BOX);

  TEST_ASSERT(ui_css_parse_shape_inside("stroke-box", &shape) == UI_ERROR_NONE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_STROKE_BOX);

  TEST_ASSERT(ui_css_parse_shape_inside("view-box", &shape) == UI_ERROR_NONE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_VIEW_BOX);

  /* Basic shapes alone */
  TEST_ASSERT(ui_css_parse_shape_inside("inset(10px)", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.shape.type == UI_CSS_BASIC_SHAPE_INSET);
  TEST_ASSERT(strcmp(shape.shape.arguments, "10px") == 0);

  TEST_ASSERT(ui_css_parse_shape_inside("circle(50%)", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.shape.type == UI_CSS_BASIC_SHAPE_CIRCLE);
  TEST_ASSERT(strcmp(shape.shape.arguments, "50%") == 0);

  TEST_ASSERT(ui_css_parse_shape_inside("ellipse(50% 50%)", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.shape.type == UI_CSS_BASIC_SHAPE_ELLIPSE);
  TEST_ASSERT(strcmp(shape.shape.arguments, "50% 50%") == 0);

  TEST_ASSERT(ui_css_parse_shape_inside("polygon(0 0, 100 100)", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.shape.type == UI_CSS_BASIC_SHAPE_POLYGON);
  TEST_ASSERT(strcmp(shape.shape.arguments, "0 0, 100 100") == 0);

  TEST_ASSERT(ui_css_parse_shape_inside("path(M 0 0)", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.shape.type == UI_CSS_BASIC_SHAPE_PATH);
  TEST_ASSERT(strcmp(shape.shape.arguments, "M 0 0") == 0);

  /* Shape and box */
  TEST_ASSERT(ui_css_parse_shape_inside("circle(50%) margin-box", &shape) ==
              UI_ERROR_NONE);
  TEST_ASSERT(shape.shape.type == UI_CSS_BASIC_SHAPE_CIRCLE);
  TEST_ASSERT(shape.box == UI_CSS_GEOMETRY_BOX_MARGIN_BOX);

  /* Too long arguments string inside paren */
  {
    char long_shape[300] = "circle(";
    memset(long_shape + 7, 'a', 256);
    long_shape[263] = ')';
    long_shape[264] = '\0';
    TEST_ASSERT(ui_css_parse_shape_inside(long_shape, &shape) == UI_ERROR_NONE);
    /* Should truncate arguments or just leave empty if it doesn't fit,
       the parsing logic checks `len >= sizeof(out_shape->shape.arguments)`,
       if not, it doesn't copy it. So `shape.arguments[0]` is '\0'.
       Wait, I changed it to `memcpy` and truncate. */
    TEST_ASSERT(shape.shape.arguments[0] != 0);
  }

  /* Malformed parens */
  TEST_ASSERT(ui_css_parse_shape_inside("circle(50%", &shape) ==
              UI_ERROR_NONE); /* No closing paren */
  TEST_ASSERT(shape.shape.arguments[0] == 0);

  TEST_ASSERT(ui_css_parse_shape_inside("circle)", &shape) ==
              UI_ERROR_PARSE_FAILED); /* No opening paren */

  TEST_ASSERT(ui_css_parse_shape_inside("circle) (", &shape) ==
              UI_ERROR_PARSE_FAILED); /* Swapped parens */

  /* Missing opening parenthesis entirely */
  TEST_ASSERT(ui_css_parse_shape_inside("circle", &shape) ==
              UI_ERROR_PARSE_FAILED);

  /* Invalid */
  TEST_ASSERT(ui_css_parse_shape_inside("invalid", &shape) ==
              UI_ERROR_PARSE_FAILED);
  TEST_ASSERT(ui_css_parse_shape_inside("", &shape) == UI_ERROR_PARSE_FAILED);

  return 0;
}

static int test_parse_border_boundary(void) {
  enum ui_css_border_boundary boundary;

  TEST_ASSERT(ui_css_parse_border_boundary(NULL, &boundary) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_parse_border_boundary("none", NULL) ==
              UI_ERROR_INVALID_ARGUMENT);

  TEST_ASSERT(ui_css_parse_border_boundary("none", &boundary) == UI_ERROR_NONE);
  TEST_ASSERT(boundary == UI_CSS_BORDER_BOUNDARY_NONE);

  TEST_ASSERT(ui_css_parse_border_boundary("parent", &boundary) ==
              UI_ERROR_NONE);
  TEST_ASSERT(boundary == UI_CSS_BORDER_BOUNDARY_PARENT);

  TEST_ASSERT(ui_css_parse_border_boundary("display", &boundary) ==
              UI_ERROR_NONE);
  TEST_ASSERT(boundary == UI_CSS_BORDER_BOUNDARY_DISPLAY);

  TEST_ASSERT(ui_css_parse_border_boundary("invalid", &boundary) ==
              UI_ERROR_PARSE_FAILED);
  TEST_ASSERT(ui_css_parse_border_boundary("", &boundary) ==
              UI_ERROR_PARSE_FAILED);

  return 0;
}

int main(void) {
  test_parse_shape_inside();
  test_parse_border_boundary();
  printf("test_ui_css_round_display passed\n");
  return 0;
}
