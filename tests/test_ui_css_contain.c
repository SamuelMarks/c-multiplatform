/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_css_contain.h"
/* clang-format on */

static int test_parse_contain(void) {
  unsigned int flags = 0;

  if (ui_css_parse_contain("none", &flags) != UI_ERROR_NONE ||
      flags != UI_CSS_CONTAIN_NONE)
    return 1;

  if (ui_css_parse_contain("strict", &flags) != UI_ERROR_NONE ||
      flags !=
          (UI_CSS_CONTAIN_STRICT | UI_CSS_CONTAIN_SIZE | UI_CSS_CONTAIN_LAYOUT |
           UI_CSS_CONTAIN_PAINT | UI_CSS_CONTAIN_STYLE))
    return 1;

  if (ui_css_parse_contain("content", &flags) != UI_ERROR_NONE ||
      flags != (UI_CSS_CONTAIN_CONTENT | UI_CSS_CONTAIN_LAYOUT |
                UI_CSS_CONTAIN_PAINT | UI_CSS_CONTAIN_STYLE))
    return 1;

  if (ui_css_parse_contain("size paint", &flags) != UI_ERROR_NONE ||
      flags != (UI_CSS_CONTAIN_SIZE | UI_CSS_CONTAIN_PAINT))
    return 1;

  if (ui_css_parse_contain("inline-size layout style", &flags) !=
          UI_ERROR_NONE ||
      flags != (UI_CSS_CONTAIN_INLINE_SIZE | UI_CSS_CONTAIN_LAYOUT |
                UI_CSS_CONTAIN_STYLE))
    return 1;

  /* Invalid combinations */
  if (ui_css_parse_contain("none size", &flags) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain("none strict", &flags) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain("none content", &flags) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain("strict content", &flags) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain("strict paint", &flags) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain("content layout", &flags) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain("invalid", &flags) == UI_ERROR_NONE)
    return 1;

  if (ui_css_parse_contain(NULL, &flags) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_contain("none", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_parse_content_visibility(void) {
  enum ui_css_content_visibility vis;

  if (ui_css_parse_content_visibility("visible", &vis) != UI_ERROR_NONE ||
      vis != UI_CSS_CONTENT_VISIBILITY_VISIBLE)
    return 1;
  if (ui_css_parse_content_visibility("auto", &vis) != UI_ERROR_NONE ||
      vis != UI_CSS_CONTENT_VISIBILITY_AUTO)
    return 1;
  if (ui_css_parse_content_visibility("hidden", &vis) != UI_ERROR_NONE ||
      vis != UI_CSS_CONTENT_VISIBILITY_HIDDEN)
    return 1;
  if (ui_css_parse_content_visibility("invalid", &vis) == UI_ERROR_NONE)
    return 1;

  if (ui_css_parse_content_visibility(NULL, &vis) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_content_visibility("auto", NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_parse_contain_intrinsic_size(void) {
  struct ui_css_contain_intrinsic_size size;

  if (ui_css_parse_contain_intrinsic_size(NULL, &size) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_contain_intrinsic_size("none", NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_contain_intrinsic_size("none", &size) != UI_ERROR_NONE)
    return 1;
  if (!size.width.is_none || size.width.has_auto || !size.height.is_none ||
      size.height.has_auto)
    return 1;

  if (ui_css_parse_contain_intrinsic_size("auto", &size) != UI_ERROR_NONE)
    return 1;
  if (!size.width.is_none || !size.width.has_auto || !size.height.is_none ||
      !size.height.has_auto)
    return 1;

  if (ui_css_parse_contain_intrinsic_size("100px", &size) != UI_ERROR_NONE)
    return 1;
  if (size.width.is_none || size.width.has_auto ||
      size.width.length.value != 100.0f)
    return 1;
  if (size.height.is_none || size.height.has_auto ||
      size.height.length.value != 100.0f)
    return 1;

  if (ui_css_parse_contain_intrinsic_size("100px none", &size) != UI_ERROR_NONE)
    return 1;
  if (size.width.is_none || size.width.has_auto ||
      size.width.length.value != 100.0f)
    return 1;
  if (!size.height.is_none || size.height.has_auto)
    return 1;

  if (ui_css_parse_contain_intrinsic_size("auto none", &size) != UI_ERROR_NONE)
    return 1;
  if (!size.width.is_none || !size.width.has_auto)
    return 1;
  if (!size.height.is_none || !size.height.has_auto)
    return 1;

  if (ui_css_parse_contain_intrinsic_size("auto 100px auto 200px", &size) !=
      UI_ERROR_NONE)
    return 1;
  if (size.width.is_none || !size.width.has_auto ||
      size.width.length.value != 100.0f)
    return 1;
  if (size.height.is_none || !size.height.has_auto ||
      size.height.length.value != 200.0f)
    return 1;

  /* Invalid parses */
  if (ui_css_parse_contain_intrinsic_size("invalid", &size) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain_intrinsic_size("100px 200px 300px", &size) ==
      UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain_intrinsic_size("100px invalid", &size) ==
      UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain_intrinsic_size("  ", &size) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain_intrinsic_size("autoa", &size) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_contain_intrinsic_size(
          "100px "
          "averylongstringthatexceedssixtyfourcharacterssoittriggersbufferclamp"
          "inglogicforthetokenbuffer",
          &size) == UI_ERROR_NONE)
    return 1;

  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_parse_contain();
  failures += test_parse_content_visibility();
  failures += test_parse_contain_intrinsic_size();

  if (failures == 0) {
    printf("test_ui_css_contain passed\n");
  } else {
    printf("test_ui_css_contain failed with %d errors\n", failures);
  }

  return failures;
}
