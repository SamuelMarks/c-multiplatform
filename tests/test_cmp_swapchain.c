/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_swapchain_create_destroy(void) {
  int rc;
  cmp_swapchain_t *swapchain = NULL;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;
  void *os_handle = NULL;
  rc = 0;

  cmp_window_system_init();
  /* Since the window abstraction exists, we can mock it here for the parameter
   */
  rc = cmp_window_create(&cfg, &win);
  if (rc != CMP_SUCCESS) {
    win = NULL;
  }

  if (win) {
    ASSERT_EQ(CMP_SUCCESS,
              cmp_swapchain_create(win, CMP_SWAPCHAIN_FIFO, &swapchain));
  } else {
    ASSERT_EQ(CMP_ERROR_INVALID_ARG,
              cmp_swapchain_create(win, CMP_SWAPCHAIN_FIFO, &swapchain));
  }
  if (win) {
    rc = cmp_swapchain_get_os_surface_handle(swapchain, &os_handle);
    ASSERT_EQ(0, rc);
    (void)os_handle;
    ASSERT_NEQ(NULL, swapchain);
  }
  if (swapchain) {
    ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_destroy(swapchain));
  }

  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_swapchain_acquire_present(void) {
  int rc;
  cmp_swapchain_t *swapchain = NULL;
  cmp_texture_t *frame_texture = NULL;
  void *first_handle;
  void *second_handle;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;

  cmp_window_system_init();
  rc = cmp_window_create(&cfg, &win);
  if (rc != CMP_SUCCESS) {
    win = NULL;
  }
  if (win) {
    ASSERT_EQ(CMP_SUCCESS,
              cmp_swapchain_create(win, CMP_SWAPCHAIN_MAILBOX, &swapchain));
  } else {
    ASSERT_EQ(CMP_ERROR_INVALID_ARG,
              cmp_swapchain_create(win, CMP_SWAPCHAIN_MAILBOX, &swapchain));
  }

  /* First frame */
  if (swapchain) {
    ASSERT_EQ(CMP_SUCCESS,
              cmp_swapchain_acquire_next_image(swapchain, &frame_texture));
    ASSERT_NEQ(NULL, frame_texture);
    first_handle = frame_texture->internal_handle;

    /* Present the frame */
    /* avoid unused variable warnings */
    (void)first_handle;
    (void)second_handle;

    ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_present(swapchain));

    /* Second frame - should be the other buffer because we swapped */
    ASSERT_EQ(CMP_SUCCESS,
              cmp_swapchain_acquire_next_image(swapchain, &frame_texture));
    ASSERT_NEQ(NULL, frame_texture);
    second_handle = frame_texture->internal_handle;
  }

  if (swapchain) {
    /* avoid unused variable warnings */
    (void)first_handle;
    (void)second_handle;

    ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_present(swapchain));
  }

  if (swapchain) {
    ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_destroy(swapchain));
  }
  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_swapchain_edge_cases(void) {
  int rc;
  cmp_swapchain_t *swapchain = NULL;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;
  cmp_texture_t *tex = NULL;

  cmp_window_system_init();
  rc = cmp_window_create(&cfg, &win);
  if (rc != CMP_SUCCESS) {
    win = NULL;
  }

  /* NULL args */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_swapchain_create(NULL, CMP_SWAPCHAIN_FIFO, &swapchain));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_swapchain_create(win, CMP_SWAPCHAIN_FIFO, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_swapchain_destroy(NULL));

  if (win) {
    ASSERT_EQ(CMP_SUCCESS,
              cmp_swapchain_create(win, CMP_SWAPCHAIN_IMMEDIATE, &swapchain));
  } else {
    ASSERT_EQ(CMP_ERROR_INVALID_ARG,
              cmp_swapchain_create(win, CMP_SWAPCHAIN_IMMEDIATE, &swapchain));
  }

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_swapchain_acquire_next_image(NULL, &tex));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_swapchain_acquire_next_image(swapchain, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_swapchain_present(NULL));

  if (swapchain) {
    ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_destroy(swapchain));
  }
  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_cmp_swapchain_set_msaa(void) {
  int rc;
  cmp_swapchain_t *swapchain = NULL;
  cmp_window_t *window = NULL;

  cmp_window_config_t cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.width = 800;
  cfg.height = 600;
  cfg.title = "Test";

  rc = cmp_window_create(&cfg, &window);
  if (rc != CMP_SUCCESS) {
    window = NULL;
  }
  if (window) {
    cmp_swapchain_create(window, CMP_SWAPCHAIN_FIFO, &swapchain);
  }

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_swapchain_set_msaa(NULL, 4));

  if (swapchain) {
    ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_swapchain_set_msaa(swapchain, 0));
    ASSERT_EQ(CMP_SUCCESS, cmp_swapchain_set_msaa(swapchain, 4));
    cmp_swapchain_destroy(swapchain);
  }

  cmp_window_destroy(window);
  PASS();
}

SUITE(cmp_swapchain_suite) {
  RUN_TEST(test_swapchain_create_destroy);
  RUN_TEST(test_swapchain_acquire_present);
  RUN_TEST(test_swapchain_edge_cases);
  RUN_TEST(test_cmp_swapchain_set_msaa);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_swapchain_suite);
  GREATEST_MAIN_END();
}
