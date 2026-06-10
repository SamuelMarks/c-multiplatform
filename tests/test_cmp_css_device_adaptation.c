/* clang-format off */
#include "greatest.h"
#include "cmp_css_device_adaptation.h"
/* clang-format on */

TEST test_viewport_width(void) {
  cmp_viewport_width_t vw;

  ASSERT_EQ(0, cmp_viewport_width_init(&vw));
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, vw.width.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, vw.min_width.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, vw.max_width.type);

  ASSERT_EQ(-1, cmp_viewport_width_init(NULL));

  PASS();
}

TEST test_viewport_height(void) {
  cmp_viewport_height_t vh;

  ASSERT_EQ(0, cmp_viewport_height_init(&vh));
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, vh.height.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, vh.min_height.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, vh.max_height.type);

  ASSERT_EQ(-1, cmp_viewport_height_init(NULL));

  PASS();
}

TEST test_viewport_zoom(void) {
  cmp_viewport_zoom_t zoom;

  ASSERT_EQ(0, cmp_viewport_zoom_init(&zoom));
  ASSERT_EQ(1.0f, zoom.zoom);
  ASSERT_EQ(0.0f, zoom.min_zoom);
  ASSERT_EQ(0.0f, zoom.max_zoom);
  ASSERT_EQ(CMP_VIEWPORT_USER_ZOOM_ZOOM, zoom.user_zoom);

  ASSERT_EQ(-1, cmp_viewport_zoom_init(NULL));

  PASS();
}

TEST test_viewport_orientation_enum(void) {
  cmp_viewport_orientation_t o = CMP_VIEWPORT_ORIENTATION_LANDSCAPE;
  ASSERT_EQ(CMP_VIEWPORT_ORIENTATION_LANDSCAPE, o);
  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_viewport_width);
  RUN_TEST(test_viewport_height);
  RUN_TEST(test_viewport_zoom);
  RUN_TEST(test_viewport_orientation_enum);
  GREATEST_MAIN_END();
}
