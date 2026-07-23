/* clang-format off */
#include "ui_geometry.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

static int test_invalid_args(void) {
  struct ui_dom_point p;
  struct ui_dom_rect r;
  struct ui_dom_quad q;
  struct ui_dom_matrix m;

  /* These should just return early, no crash */
  ui_dom_point_init(NULL, 1.0, 2.0, 3.0, 4.0);
  ui_dom_rect_init(NULL, 1.0, 2.0, 3.0, 4.0);

  ui_dom_point_init(&p, 1.0, 2.0, 3.0, 4.0);
  ui_dom_quad_init(NULL, &p, &p, &p, &p);
  ui_dom_quad_init(&q, NULL, &p, &p, &p);
  ui_dom_quad_init(&q, &p, NULL, &p, &p);
  ui_dom_quad_init(&q, &p, &p, NULL, &p);
  ui_dom_quad_init(&q, &p, &p, &p, NULL);

  ui_dom_matrix_init_identity(NULL);

  return 0;
}

static int test_dom_point_init(void) {
  struct ui_dom_point p;
  ui_dom_point_init(&p, 1.0, 2.0, 3.0, 4.0);
  if (p.x != 1.0 || p.y != 2.0 || p.z != 3.0 || p.w != 4.0) {
    printf("test_dom_point_init failed\n");
    return 1;
  }
  return 0;
}

static int test_dom_rect_init_positive(void) {
  struct ui_dom_rect r;
  ui_dom_rect_init(&r, 10.0, 20.0, 50.0, 60.0);
  if (r.x != 10.0 || r.y != 20.0 || r.width != 50.0 || r.height != 60.0 ||
      r.left != 10.0 || r.right != 60.0 || r.top != 20.0 || r.bottom != 80.0) {
    printf("test_dom_rect_init_positive failed\n");
    return 1;
  }
  return 0;
}

static int test_dom_rect_init_negative(void) {
  struct ui_dom_rect r;
  ui_dom_rect_init(&r, 10.0, 20.0, -50.0, -60.0);
  if (r.x != 10.0 || r.y != 20.0 || r.width != -50.0 || r.height != -60.0 ||
      r.left != -40.0 || r.right != 10.0 || r.top != -40.0 ||
      r.bottom != 20.0) {
    printf("test_dom_rect_init_negative failed\n");
    return 1;
  }
  return 0;
}

static int test_dom_quad_init(void) {
  struct ui_dom_point p1, p2, p3, p4;
  struct ui_dom_quad q;

  ui_dom_point_init(&p1, 0.0, 0.0, 0.0, 1.0);
  ui_dom_point_init(&p2, 100.0, 0.0, 0.0, 1.0);
  ui_dom_point_init(&p3, 100.0, 100.0, 0.0, 1.0);
  ui_dom_point_init(&p4, 0.0, 100.0, 0.0, 1.0);

  ui_dom_quad_init(&q, &p1, &p2, &p3, &p4);

  if (q.bounds.x != 0.0 || q.bounds.y != 0.0 || q.bounds.width != 100.0 ||
      q.bounds.height != 100.0) {
    printf("test_dom_quad_init failed bounds check\n");
    return 1;
  }
  return 0;
}

static int test_dom_matrix_init(void) {
  struct ui_dom_matrix m;
  ui_dom_matrix_init_identity(&m);

  if (m.m11 != 1.0 || m.m22 != 1.0 || m.m33 != 1.0 || m.m44 != 1.0 ||
      m.m12 != 0.0 || m.m21 != 0.0) {
    printf("test_dom_matrix_init failed\n");
    return 1;
  }
  return 0;
}

static int run_extra_geometry(void);

int main(void) {
  int failures = 0;
  failures += test_invalid_args();
  failures += test_dom_point_init();
  failures += test_dom_rect_init_positive();
  failures += test_dom_rect_init_negative();
  failures += test_dom_quad_init();
  failures += test_dom_matrix_init();
  failures += run_extra_geometry();

  if (failures == 0) {
    printf("test_ui_geometry passed\n");
  } else {
    printf("test_ui_geometry had %d failures\n", failures);
  }
  return failures;
}

static int run_extra_geometry(void) {
  /* test internal coverage of min/max macros if not fully hit.
     Our quad test uses (0,0), (100,0), (100,100), (0,100).
     Let's ensure we hit all variations.
  */
  struct ui_dom_point p1, p2, p3, p4;
  struct ui_dom_quad q;
  ui_dom_point_init(&p1, 20.0, 30.0, 0.0, 1.0);
  ui_dom_point_init(&p2, 10.0, 40.0, 0.0, 1.0);
  ui_dom_point_init(&p3, 50.0, 10.0, 0.0, 1.0);
  ui_dom_point_init(&p4, 5.0, 50.0, 0.0, 1.0);
  ui_dom_quad_init(&q, &p1, &p2, &p3, &p4);

  return 0;
}
