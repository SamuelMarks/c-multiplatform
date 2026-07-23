/* clang-format off */
#include "ui_timeline_base.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

static int test_normal(void) {
  struct ui_timeline_base *tl = NULL;
  enum ui_timeline_alignment align;
  size_t count;
  const char *title, *desc;
  int failed = 0;

  failed |= (ui_timeline_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT);
  ui_timeline_base_destroy(NULL);

  ACCUM_ERR(failed, ui_timeline_base_create(&tl));

  failed |= (ui_timeline_base_set_alignment(NULL, UI_TIMELINE_ALIGN_LEFT) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timeline_base_get_alignment(NULL, &align) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timeline_base_get_alignment(tl, NULL) != UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, ui_timeline_base_get_alignment(tl, &align));
  failed |= (align != UI_TIMELINE_ALIGN_ALTERNATE);

  ACCUM_ERR(failed, ui_timeline_base_set_alignment(tl, UI_TIMELINE_ALIGN_LEFT));
  ACCUM_ERR(failed, ui_timeline_base_get_alignment(tl, &align));
  failed |= (align != UI_TIMELINE_ALIGN_LEFT);

  failed |= (ui_timeline_base_get_node_count(NULL, &count) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timeline_base_get_node_count(tl, NULL) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_timeline_base_get_node_count(tl, &count));
  failed |= (count != 0);

  failed |=
      (ui_timeline_base_add_node(NULL, "a", "b") != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timeline_base_add_node(tl, NULL, "b") != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_timeline_base_add_node(tl, "a", NULL) != UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_timeline_base_get_node(NULL, 0, &title, &desc) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timeline_base_get_node(tl, 0, NULL, &desc) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timeline_base_get_node(tl, 0, &title, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_timeline_base_get_node(tl, 0, &title, &desc) !=
             UI_ERROR_OUT_OF_BOUNDS);

  ACCUM_ERR(failed, ui_timeline_base_add_node(tl, "t1", "d1"));
  ACCUM_ERR(failed, ui_timeline_base_get_node_count(tl, &count));
  failed |= (count != 1);

  ACCUM_ERR(failed, ui_timeline_base_get_node(tl, 0, &title, &desc));

  ACCUM_ERR(failed, ui_timeline_base_add_node(tl, "t2", "d2"));
  ACCUM_ERR(failed, ui_timeline_base_add_node(tl, "t3", "d3"));
  ACCUM_ERR(failed, ui_timeline_base_add_node(tl, "t4", "d4"));
  ACCUM_ERR(failed, ui_timeline_base_add_node(tl, "t5", "d5"));

  failed |= (ui_timeline_base_render(NULL) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_timeline_base_render(tl));

  failed |=
      (ui_timeline_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed,
            ui_timeline_base_bind_data(tl, NULL)); /* NULL signal should pass */

  ui_timeline_base_destroy(tl);

  return failed;
}

static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_timeline_base *tl;
  int i;

  g_malloc_fail_countdown = 0;
  failed |= (ui_timeline_base_create(&tl) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_timeline_base_create(&tl);

  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    failed |= (ui_timeline_base_add_node(tl, "t", "d") == UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;
  }
  g_malloc_fail_countdown = -1;

  /* Force reallocation OOM */
  ui_timeline_base_add_node(tl, "t1", "d1");
  ui_timeline_base_add_node(tl, "t2", "d2");
  ui_timeline_base_add_node(tl, "t3", "d3");
  ui_timeline_base_add_node(tl, "t4", "d4");

  /* Next add triggers realloc, which uses a new alloc */
  g_malloc_fail_countdown = 2; /* 0=title, 1=desc, 2=new_nodes */
  failed |= (ui_timeline_base_add_node(tl, "t5", "d5") == UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  ui_timeline_base_destroy(tl);
#endif
  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_normal();
  failed |= test_oom();
  if (!failed) {
    printf("All ui_timeline_base tests passed.\n");
  }
  return failed;
}
