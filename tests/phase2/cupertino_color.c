/* clang-format off */
#include "cupertino/cupertino_color.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_color_get_system(void) {
  CMPColor color;

  /* Invalid arg */
  CMP_TEST_EXPECT(
      cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cupertino_color_get_system((CupertinoSystemColor)999, CMP_FALSE, &color),
      CMP_ERR_INVALID_ARGUMENT);

  /* Valid colors */
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_TRUE, &color));

  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_BROWN, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_CYAN, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GREEN, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_INDIGO, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_MINT, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_ORANGE, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_PINK, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_PURPLE, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_RED, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_TEAL, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_YELLOW, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY2, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY3, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY4, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY5, CMP_FALSE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY6, CMP_FALSE, &color));

  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_BROWN, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_CYAN, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GREEN, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_INDIGO, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_MINT, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_ORANGE, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_PINK, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_PURPLE, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_RED, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_TEAL, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_YELLOW, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY2, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY3, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY4, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY5, CMP_TRUE, &color));
  CMP_TEST_OK(
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY6, CMP_TRUE, &color));

  return 0;
}

int main(void) {
  if (test_cupertino_color_get_system() != 0)
    return 1;
  return 0;
}
