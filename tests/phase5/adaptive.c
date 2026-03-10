/* clang-format off */
#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_core.h"
#include "m3/m3_adaptive.h"
#include <stdio.h>

static int test_anim_bezier(void) {
  CMPScalar val = -1.0f;
  int rc;
  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_EMPHASIZED, 0.0f, &val) !=
          CMP_OK ||
      val != 0.0f) {
    printf("fail 1\n");
    return 1;
  }

  rc = cmp_anim_test_apply_ease(CMP_ANIM_EASE_EMPHASIZED, 1.0f, &val);
  if (rc != CMP_OK || val != 1.0f) {
    printf("fail 2: rc=%d val=%f\n", rc, val);
    return 1;
  }
  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_EMPHASIZED, 0.5f, &val) !=
      CMP_OK) {
    printf("fail 3\n");
    return 1;
  }

  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_EMPHASIZED_DECELERATE, 0.5f,
                               &val) != CMP_OK) {
    printf("fail 4\n");
    return 1;
  }
  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_EMPHASIZED_ACCELERATE, 0.5f,
                               &val) != CMP_OK) {
    printf("fail 5\n");
    return 1;
  }
  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_STANDARD, 0.5f, &val) != CMP_OK) {
    printf("fail 6\n");
    return 1;
  }
  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_STANDARD_DECELERATE, 0.5f, &val) !=
      CMP_OK) {
    printf("fail 7\n");
    return 1;
  }
  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_STANDARD_ACCELERATE, 0.5f, &val) !=
      CMP_OK) {
    printf("fail 8\n");
    return 1;
  }

  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_IN_OUT, -0.5f, &val) != CMP_OK ||
      val != 0.0f) {
    printf("fail 9\n");
    return 1;
  }
  if (cmp_anim_test_apply_ease(CMP_ANIM_EASE_IN_OUT, 1.5f, &val) != CMP_OK ||
      val != 1.0f) {
    printf("fail 10\n");
    return 1;
  }
  if (cmp_anim_test_apply_ease(999, 0.5f, &val) != CMP_ERR_INVALID_ARGUMENT) {
    printf("fail 11\n");
    return 1;
  }

  return 0;
}

static int test_window_size_class(void) {
  cmp_u32 class;

  if (m3_window_size_class(-10.0f, &class) != CMP_ERR_RANGE)
    return 1;
  if (m3_window_size_class(100.0f, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (m3_window_size_class(0.0f, &class) != CMP_OK ||
      class != M3_WINDOW_CLASS_COMPACT)
    return 1;
  if (m3_window_size_class(599.0f, &class) != CMP_OK ||
      class != M3_WINDOW_CLASS_COMPACT)
    return 1;
  if (m3_window_size_class(600.0f, &class) != CMP_OK ||
      class != M3_WINDOW_CLASS_MEDIUM)
    return 1;
  if (m3_window_size_class(839.0f, &class) != CMP_OK ||
      class != M3_WINDOW_CLASS_MEDIUM)
    return 1;
  if (m3_window_size_class(840.0f, &class) != CMP_OK ||
      class != M3_WINDOW_CLASS_EXPANDED)
    return 1;
  if (m3_window_size_class(1199.0f, &class) != CMP_OK ||
      class != M3_WINDOW_CLASS_EXPANDED)
    return 1;
  if (m3_window_size_class(1200.0f, &class) != CMP_OK ||
      class != M3_WINDOW_CLASS_EXTRA_LARGE)
    return 1;
  if (m3_window_size_class(1920.0f, &class) != CMP_OK ||
      class != M3_WINDOW_CLASS_EXTRA_LARGE)
    return 1;

  return 0;
}

static int test_adaptive_list_detail_measure(void) {
  M3AdaptiveLayout layout = {0};
  CMPScalar w, h;

  layout.safe_width = 1000.0f;
  layout.safe_height = 800.0f;

  if (m3_adaptive_list_detail_measure(NULL, &w, &h) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_adaptive_list_detail_measure(&layout, NULL, &h) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_adaptive_list_detail_measure(&layout, &w, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  layout.safe_width = -1.0f;
  if (m3_adaptive_list_detail_measure(&layout, &w, &h) != CMP_ERR_RANGE)
    return 1;
  layout.safe_width = 1000.0f;
  layout.safe_height = -1.0f;
  if (m3_adaptive_list_detail_measure(&layout, &w, &h) != CMP_ERR_RANGE)
    return 1;

  layout.safe_height = 800.0f;
  if (m3_adaptive_list_detail_measure(&layout, &w, &h) != CMP_OK)
    return 1;
  if (w != 1000.0f || h != 800.0f)
    return 1;

  return 0;
}

static int test_adaptive_list_detail_layout(void) {
  M3AdaptiveLayout layout = {0};
  CMPRect bounds;
  CMPRect prim, sec;

  layout.window_class = M3_WINDOW_CLASS_COMPACT;
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 400.0f;
  bounds.height = 800.0f;

  if (m3_adaptive_list_detail_layout(NULL, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_adaptive_list_detail_layout(&layout, bounds, NULL, &sec, CMP_FALSE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, NULL, CMP_FALSE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  bounds.width = -1.0f;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_ERR_RANGE)
    return 1;
  bounds.width = 400.0f;
  bounds.height = -1.0f;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_ERR_RANGE)
    return 1;
  bounds.height = 800.0f;

  /* Compact */
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (prim.width != 400.0f || prim.height != 800.0f)
    return 1;
  if (sec.width != 0.0f || sec.height != 0.0f)
    return 1;

  /* Medium */
  layout.window_class = M3_WINDOW_CLASS_MEDIUM;
  bounds.width = 600.0f;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  /* (600 - 24) / 3 = 192 */
  if (prim.width != 192.0f)
    return 1;
  if (sec.width != 600.0f - 24.0f - 192.0f)
    return 1;

  /* Medium too small width */
  bounds.width = 10.0f;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (prim.width != 0.0f || sec.width != 0.0f)
    return 1;

  /* Expanded / Extra Large */
  layout.window_class = M3_WINDOW_CLASS_EXPANDED;
  bounds.width = 1000.0f;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (prim.width != 360.0f)
    return 1;
  if (sec.width != 1000.0f - 360.0f - 24.0f)
    return 1;

  /* Expanded too small width */
  bounds.width = 300.0f;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (prim.width != 300.0f || sec.width != 0.0f)
    return 1;

  return 0;
}

static int test_adaptive_feed_layout(void) {
  M3AdaptiveLayout layout = {0};
  CMPRect bounds;
  CMPRect prim, sec;

  layout.window_class = M3_WINDOW_CLASS_COMPACT;
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 400.0f;
  bounds.height = 800.0f;

  if (m3_adaptive_feed_layout(NULL, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_adaptive_feed_layout(&layout, bounds, NULL, &sec, CMP_FALSE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, NULL, CMP_FALSE) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  bounds.width = -1.0f;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_ERR_RANGE)
    return 1;
  bounds.width = 400.0f;
  bounds.height = -1.0f;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_ERR_RANGE)
    return 1;
  bounds.height = 800.0f;

  /* Compact */
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (prim.width != 400.0f)
    return 1;
  if (sec.width != 0.0f)
    return 1;

  /* Medium */
  layout.window_class = M3_WINDOW_CLASS_MEDIUM;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (prim.width != 400.0f - 48.0f)
    return 1;
  if (sec.width != 0.0f)
    return 1;

  /* Medium too small */
  bounds.width = 20.0f;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (prim.width != 0.0f)
    return 1;

  /* Expanded */
  layout.window_class = M3_WINDOW_CLASS_EXPANDED;
  bounds.width = 1200.0f;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (sec.width != 300.0f)
    return 1;
  if (prim.width != 1200.0f - 300.0f - 24.0f - 48.0f)
    return 1;

  /* Expanded too small */
  bounds.width = 200.0f;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (sec.width != 200.0f)
    return 1;
  if (prim.width != 0.0f)
    return 1;

  /* Expanded max width feed */
  bounds.width = 2000.0f;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (prim.width != 840.0f)
    return 1; /* Capped */

  /* Expanded max width feed RTL */
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_TRUE) != CMP_OK)
    return 1;
  if (prim.width != 840.0f)
    return 1; /* Capped */

  return 0;
}

static int test_adaptive_supporting_pane_layout(void) {
  M3AdaptiveLayout layout = {0};
  CMPRect bounds;
  CMPRect prim, sec;

  layout.window_class = M3_WINDOW_CLASS_COMPACT;
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 400.0f;
  bounds.height = 800.0f;

  if (m3_adaptive_supporting_pane_layout(NULL, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, NULL, &sec,
                                         CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, NULL,
                                         CMP_FALSE) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  bounds.width = -1.0f;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_ERR_RANGE)
    return 1;
  bounds.width = 400.0f;
  bounds.height = -1.0f;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_ERR_RANGE)
    return 1;
  bounds.height = 800.0f;

  /* Compact */
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_OK)
    return 1;
  if (prim.width != 400.0f)
    return 1;
  if (sec.width != 0.0f)
    return 1;

  /* Medium */
  layout.window_class = M3_WINDOW_CLASS_MEDIUM;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_OK)
    return 1;
  if (prim.width != 400.0f)
    return 1;
  if (sec.width != 0.0f)
    return 1;

  /* Expanded */
  layout.window_class = M3_WINDOW_CLASS_EXPANDED;
  bounds.width = 1200.0f;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_OK)
    return 1;
  if (sec.width != 360.0f)
    return 1;
  if (prim.width != 1200.0f - 360.0f - 24.0f - 48.0f)
    return 1;

  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_TRUE) != CMP_OK)
    return 1;
  if (sec.x != 24.0f)
    return 1;

  /* Expanded too small */
  bounds.width = 200.0f;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_OK)
    return 1;
  if (sec.width != 200.0f)
    return 1;
  if (prim.width != 0.0f)
    return 1;

  return 0;
}

static int test_adaptive_rtl_layouts(void) {
  M3AdaptiveLayout layout = {0};
  CMPRect bounds;
  CMPRect prim, sec;

  layout.window_class = M3_WINDOW_CLASS_EXPANDED;
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 1200.0f;
  bounds.height = 800.0f;

  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_TRUE) !=
      CMP_OK)
    return 1;
  if (sec.x != 0.0f)
    return 1;
  if (prim.x <= sec.x)
    return 1;

  /* RTL Medium */
  layout.window_class = M3_WINDOW_CLASS_MEDIUM;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_TRUE) !=
      CMP_OK)
    return 1;
  if (sec.x != 0.0f)
    return 1;
  if (prim.x <= sec.x)
    return 1;
  layout.window_class = M3_WINDOW_CLASS_EXPANDED;

  /* RTL Medium Feed */
  layout.window_class = M3_WINDOW_CLASS_MEDIUM;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_TRUE) != CMP_OK)
    return 1;
  if (prim.x <= sec.x)
    return 1;

  /* RTL Expanded Feed */
  layout.window_class = M3_WINDOW_CLASS_EXPANDED;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_TRUE) != CMP_OK)
    return 1;
  if (prim.x <= sec.x)
    return 1;

  /* RTL Medium Supporting Pane */
  layout.window_class = M3_WINDOW_CLASS_MEDIUM;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_TRUE) != CMP_OK)
    return 1;

  /* RTL Expanded Supporting Pane */
  layout.window_class = M3_WINDOW_CLASS_EXPANDED;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_TRUE) != CMP_OK)
    return 1;
  if (sec.x != 24.0f)
    return 1;

  return 0;
}

static int test_adaptive_hinge(void) {
  M3AdaptiveLayout layout = {0};
  CMPRect bounds = {0.0f, 0.0f, 1000.0f, 800.0f};
  CMPRect prim, sec;

  layout.hinge.is_separating = CMP_TRUE;
  layout.hinge.posture = M3_POSTURE_HALF_OPENED_BOOK;
  layout.hinge.bounds.x = 400.0f;
  layout.hinge.bounds.y = 0.0f;
  layout.hinge.bounds.width = 10.0f;
  layout.hinge.bounds.height = 800.0f;

  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_TRUE) !=
      CMP_OK)
    return 1;

  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_TRUE) != CMP_OK)
    return 1;

  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_OK)
    return 1;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_TRUE) != CMP_OK)
    return 1;

  /* Hinge with extreme bounds to trigger negative width handling */
  bounds.width = 100.0f;
  layout.hinge.bounds.x = 200.0f;
  layout.hinge.bounds.width = 50.0f;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_FALSE) !=
      CMP_OK)
    return 1;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_FALSE) != CMP_OK)
    return 1;
  if (m3_adaptive_list_detail_layout(&layout, bounds, &prim, &sec, CMP_TRUE) !=
      CMP_OK)
    return 1;
  if (m3_adaptive_feed_layout(&layout, bounds, &prim, &sec, CMP_TRUE) != CMP_OK)
    return 1;
  if (m3_adaptive_supporting_pane_layout(&layout, bounds, &prim, &sec,
                                         CMP_TRUE) != CMP_OK)
    return 1;

  return 0;
}

#include <stdio.h>
/* clang-format on */

int main(void) {
  if (test_anim_bezier() != 0) {
    printf("anim_bezier failed\n");
    return 1;
  }
  if (test_window_size_class() != 0) {
    printf("window_size failed\n");
    return 1;
  }
  if (test_adaptive_list_detail_measure() != 0) {
    printf("measure failed\n");
    return 1;
  }
  if (test_adaptive_list_detail_layout() != 0) {
    printf("layout failed\n");
    return 1;
  }
  if (test_adaptive_feed_layout() != 0) {
    printf("feed layout failed\n");
    return 1;
  }
  if (test_adaptive_supporting_pane_layout() != 0) {
    printf("supporting pane layout failed\n");
    return 1;
  }
  if (test_adaptive_rtl_layouts() != 0) {
    printf("rtl layout failed\n");
    return 1;
  }
  if (test_adaptive_hinge() != 0) {
    printf("hinge layout failed\n");
    return 1;
  }
  return 0;
}