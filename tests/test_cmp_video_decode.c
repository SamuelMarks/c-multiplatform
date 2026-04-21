/* clang-format off */
#include "cmp.h"
#include "cutest.h"
#include <string.h>
/* clang-format on */

TEST test_video_decode_create(void) {
  cmp_hw_video_decoder_t *decoder = NULL;
  char nv12_buf[1024];

  ASSERT_EQ(CMP_SUCCESS, cmp_hw_video_decoder_create(&decoder));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_hw_video_decoder_decode_frame(decoder, "dummy", 5, nv12_buf));
  ASSERT_EQ(CMP_SUCCESS, cmp_hw_video_decoder_destroy(decoder));

  return 0;
}

SUITE(suite_video_decode) { RUN_TEST(test_video_decode_create); }

int main(void) {
  RUN_SUITE(suite_video_decode);
  return TEST_SUCCESS;
}
