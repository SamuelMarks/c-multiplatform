/* clang-format off */
#include "cupertino/cupertino_pull_to_refresh.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_pull_to_refresh(void) {
  CupertinoRefreshControl ptr;
  CupertinoRefreshStyle style;
  CMPWidget child = {0};
  CMPRect bounds = {0.0f, 0.0f, 320.0f, 480.0f};

  CMP_TEST_EXPECT(cupertino_refresh_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_refresh_style_init(&style));

  CMP_TEST_EXPECT(cupertino_refresh_init(NULL, &child),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_refresh_init(&ptr, &child));

  CMP_TEST_EXPECT(cupertino_refresh_set_refreshing(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_refresh_set_refreshing(&ptr, CMP_TRUE));
  CMP_TEST_EXPECT(ptr.is_refreshing, CMP_TRUE);

  CMP_TEST_OK(cupertino_refresh_set_refreshing(&ptr, CMP_FALSE));
  CMP_TEST_EXPECT(ptr.is_refreshing, CMP_FALSE);
  CMP_TEST_EXPECT((int)ptr.current_pull, 0);

  CMP_TEST_EXPECT(cupertino_refresh_set_callback(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_refresh_set_callback(&ptr, NULL, NULL));

  CMP_TEST_EXPECT(cupertino_refresh_layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_refresh_layout(&ptr, bounds));

  return 0;
}

int main(void) {
  if (test_cupertino_pull_to_refresh() != 0)
    return 1;
  return 0;
}
