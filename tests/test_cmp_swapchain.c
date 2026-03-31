/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_swapchain_create_destroy(void) {
  cmp_swapchain_t *swapchain = NULL;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;

  cmp_window_system_init();

  /* Since the window abstraction exists, we can mock it here for the parameter
   */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_create(&cfg, &win));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_swapchain_create(win, CMP_SWAPCHAIN_FIFO, &swapchain));
  ASSERT_NEQ(NULL, swapchain);

  ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_destroy(swapchain));

  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_swapchain_acquire_present(void) {
  cmp_swapchain_t *swapchain = NULL;
  cmp_texture_t *frame_texture = NULL;
  void *first_handle;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;

  cmp_window_system_init();
  cmp_window_create(&cfg, &win);
  ASSERT_EQ(CMP_SUCCESS,
            cmp_swapchain_create(win, CMP_SWAPCHAIN_MAILBOX, &swapchain));

  /* First frame */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_swapchain_acquire_next_image(swapchain, &frame_texture));
  ASSERT_NEQ(NULL, frame_texture);
  first_handle = frame_texture->internal_handle;

  /* Present the frame */
  ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_present(swapchain));

  /* Second frame - should be the other buffer because we swapped */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_swapchain_acquire_next_image(swapchain, &frame_texture));
  ASSERT_NEQ(NULL, frame_texture);
  ASSERT_NEQ(first_handle, frame_texture->internal_handle);

  ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_present(swapchain));

  ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_destroy(swapchain));
  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_swapchain_edge_cases(void) {
  cmp_swapchain_t *swapchain = NULL;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;
  cmp_texture_t *tex = NULL;

  cmp_window_system_init();
  cmp_window_create(&cfg, &win);

  /* NULL args */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_swapchain_create(NULL, CMP_SWAPCHAIN_FIFO, &swapchain));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_swapchain_create(win, CMP_SWAPCHAIN_FIFO, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_swapchain_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_swapchain_create(win, CMP_SWAPCHAIN_IMMEDIATE, &swapchain));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_swapchain_acquire_next_image(NULL, &tex));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_swapchain_acquire_next_image(swapchain, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_swapchain_present(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_destroy(swapchain));
  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  PASS();
}

SUITE(cmp_swapchain_suite) {
  RUN_TEST(test_swapchain_create_destroy);
  RUN_TEST(test_swapchain_acquire_present);
  RUN_TEST(test_swapchain_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_swapchain_suite);
  GREATEST_MAIN_END();
}
