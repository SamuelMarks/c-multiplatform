#include "m3/m3_typography.h"
#include "test_utils.h"
#include <string.h>

int main(void) {
  M3TypographyScale scale;
  CMPTextStyle style;

  CMP_TEST_EXPECT(m3_typography_scale_init(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_typography_scale_init(&scale));

  CMP_TEST_EXPECT(
      m3_typography_get_style(NULL, M3_TYPOGRAPHY_BODY_LARGE, &style),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_typography_get_style(&scale, M3_TYPOGRAPHY_BODY_LARGE, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_typography_get_style(&scale, (M3TypographyRole)-1, &style),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      m3_typography_get_style(&scale, M3_TYPOGRAPHY_ROLE_COUNT, &style),
      CMP_ERR_RANGE);

  CMP_TEST_OK(
      m3_typography_get_style(&scale, M3_TYPOGRAPHY_BODY_LARGE, &style));
  CMP_TEST_EXPECT(style.size_px, 16);
  CMP_TEST_EXPECT(style.weight, 400);

  CMP_TEST_OK(
      m3_typography_get_style(&scale, M3_TYPOGRAPHY_DISPLAY_LARGE, &style));
  CMP_TEST_EXPECT(style.size_px, 57);

  {
    CMPColor c;
    c.r = 0.1f;
    c.g = 0.2f;
    c.b = 0.3f;
    c.a = 1.0f;
    CMP_TEST_EXPECT(m3_typography_scale_set_color(NULL, c),
                    CMP_ERR_INVALID_ARGUMENT);
  }
  {
    CMPColor c;
    c.r = 0.1f;
    c.g = 0.2f;
    c.b = 0.3f;
    c.a = 1.0f;
    CMP_TEST_OK(m3_typography_scale_set_color(&scale, c));
  }
  CMP_TEST_OK(
      m3_typography_get_style(&scale, M3_TYPOGRAPHY_TITLE_SMALL, &style));
  CMP_TEST_EXPECT(
      style.color.r > 0.09f && style.color.r < 0.11f ? CMP_OK : CMP_ERR_UNKNOWN,
      CMP_OK);

  CMP_TEST_EXPECT(m3_typography_scale_set_family(NULL, "Arial"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_typography_scale_set_family(&scale, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_typography_scale_set_family(&scale, "Arial"));
  CMP_TEST_OK(
      m3_typography_get_style(&scale, M3_TYPOGRAPHY_LABEL_SMALL, &style));
  CMP_TEST_EXPECT(strcmp(style.utf8_family, "Arial") == 0 ? CMP_OK
                                                          : CMP_ERR_UNKNOWN,
                  CMP_OK);

  return 0;
}
