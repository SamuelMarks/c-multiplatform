/* clang-format off */
#include "../include/ui_sort_header_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

static int test_sort_header_single(void) {
  struct ui_sort_header_base *sort = NULL;
  enum ui_sort_direction dir;
  struct ui_sort_state states[2];
  size_t count;

  if (ui_sort_header_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("f1\n");
    return 1;
  }
  if (ui_sort_header_base_create(&sort) != UI_ERROR_NONE) {
    printf("f2\n");
    return 1;
  }
  if (ui_sort_header_base_set_multi_sort(NULL, 1) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f3\n");
    return 1;
  }
  if (ui_sort_header_base_toggle(NULL, (void *)1) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f4\n");
    return 1;
  }
  if (ui_sort_header_base_set_direction(NULL, (void *)1, UI_SORT_ASCENDING) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f5\n");
    return 1;
  }
  if (ui_sort_header_base_get_direction(NULL, (void *)1, &dir) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f6\n");
    return 1;
  }
  if (ui_sort_header_base_get_direction(sort, (void *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f7\n");
    return 1;
  }
  if (ui_sort_header_base_get_active_sorts(NULL, states, 2, &count) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f8\n");
    return 1;
  }
  if (ui_sort_header_base_get_active_sorts(sort, NULL, 2, &count) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f9\n");
    return 1;
  }
  if (ui_sort_header_base_get_active_sorts(sort, states, 2, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f10\n");
    return 1;
  }
  if (ui_sort_header_base_clear(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("f11\n");
    return 1;
  }
  if (ui_sort_header_base_bind_direction(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("f12\n");
    return 1;
  }
  if (ui_sort_header_base_bind_direction(sort, NULL) != UI_ERROR_NONE) {
    printf("f13\n");
    return 1;
  }

  if (ui_sort_header_base_get_direction(sort, (void *)1, &dir) !=
      UI_ERROR_NONE) {
    printf("f14\n");
    return 1;
  }
  if (dir != UI_SORT_NONE) {
    printf("f15\n");
    return 1;
  }

  if (ui_sort_header_base_toggle(sort, (void *)1) != UI_ERROR_NONE) {
    printf("f16\n");
    return 1;
  }
  ui_sort_header_base_get_direction(sort, (void *)1, &dir);
  if (dir != UI_SORT_ASCENDING) {
    printf("f17\n");
    return 1;
  }

  ui_sort_header_base_get_active_sorts(sort, states, 2, &count);
  if (count != 1 || states[0].id != (void *)1 ||
      states[0].direction != UI_SORT_ASCENDING) {
    printf("f18\n");
    return 1;
  }

  ui_sort_header_base_toggle(sort, (void *)1);
  ui_sort_header_base_get_direction(sort, (void *)1, &dir);
  if (dir != UI_SORT_DESCENDING) {
    printf("f19\n");
    return 1;
  }

  ui_sort_header_base_toggle(sort, (void *)1);
  ui_sort_header_base_get_direction(sort, (void *)1, &dir);
  if (dir != UI_SORT_NONE) {
    printf("f20\n");
    return 1;
  }

  ui_sort_header_base_get_active_sorts(sort, states, 2, &count);
  if (count != 0) {
    printf("f21\n");
    return 1;
  }

  ui_sort_header_base_set_direction(sort, (void *)2, UI_SORT_ASCENDING);
  ui_sort_header_base_get_direction(sort, (void *)2, &dir);
  if (dir != UI_SORT_ASCENDING) {
    printf("f22\n");
    return 1;
  }

  ui_sort_header_base_set_direction(sort, (void *)3, UI_SORT_DESCENDING);
  ui_sort_header_base_get_direction(sort, (void *)2, &dir);
  if (dir != UI_SORT_NONE) {
    printf("f23\n");
    return 1;
  }
  ui_sort_header_base_get_direction(sort, (void *)3, &dir);
  if (dir != UI_SORT_DESCENDING) {
    printf("f24\n");
    return 1;
  }

  ui_sort_header_base_clear(sort);
  ui_sort_header_base_get_direction(sort, (void *)3, &dir);
  if (dir != UI_SORT_NONE) {
    printf("f25\n");
    return 1;
  }

  ui_sort_header_base_destroy(sort);
  ui_sort_header_base_destroy(NULL);
  return 0;
}

static int test_sort_header_multi(void) {
  struct ui_sort_header_base *sort = NULL;
  enum ui_sort_direction dir;
  struct ui_sort_state states[4];
  size_t count;

  if (ui_sort_header_base_create(&sort) != UI_ERROR_NONE) {
    printf("m1\n");
    return 1;
  }
  ui_sort_header_base_set_multi_sort(sort, 1);

  ui_sort_header_base_toggle(sort, (void *)1);
  ui_sort_header_base_toggle(sort, (void *)2);
  ui_sort_header_base_set_direction(sort, (void *)3, UI_SORT_DESCENDING);

  ui_sort_header_base_get_direction(sort, (void *)1, &dir);
  if (dir != UI_SORT_ASCENDING) {
    printf("m2\n");
    return 1;
  }
  ui_sort_header_base_get_direction(sort, (void *)2, &dir);
  if (dir != UI_SORT_ASCENDING) {
    printf("m3\n");
    return 1;
  }
  ui_sort_header_base_get_direction(sort, (void *)3, &dir);
  if (dir != UI_SORT_DESCENDING) {
    printf("m4\n");
    return 1;
  }

  ui_sort_header_base_get_active_sorts(sort, states, 4, &count);
  if (count != 3) {
    printf("m5\n");
    return 1;
  }
  if (states[0].id != (void *)1 || states[1].id != (void *)2 ||
      states[2].id != (void *)3) {
    printf("m6\n");
    return 1;
  }

  ui_sort_header_base_toggle(sort, (void *)2);
  ui_sort_header_base_get_active_sorts(sort, states, 4, &count);
  if (states[1].id != (void *)2 || states[1].direction != UI_SORT_DESCENDING) {
    printf("m7\n");
    return 1;
  }

  ui_sort_header_base_toggle(sort, (void *)2);
  ui_sort_header_base_get_active_sorts(sort, states, 4, &count);
  if (count != 2) {
    printf("m8\n");
    return 1;
  }
  if (states[0].id != (void *)1 || states[1].id != (void *)3) {
    printf("m9\n");
    return 1;
  }

  ui_sort_header_base_set_multi_sort(sort, 0);
  ui_sort_header_base_get_active_sorts(sort, states, 4, &count);
  if (count != 1) {
    printf("m10\n");
    return 1;
  }
  if (states[0].id != (void *)1) {
    printf("m11\n");
    return 1;
  }

  {
    extern int g_malloc_fail_countdown;
    ui_sort_header_base_set_multi_sort(sort, 1);
    /* Add items until capacity is reached (current count 1, cap 4) */
    ui_sort_header_base_toggle(sort, (void *)6);
    ui_sort_header_base_toggle(sort, (void *)7);
    ui_sort_header_base_toggle(sort, (void *)8);
    /* Now count should be 4, capacity 4. Next toggle will reallocate. */
    g_malloc_fail_countdown = 0;
    if (ui_sort_header_base_toggle(sort, (void *)9) != UI_ERROR_OUT_OF_MEMORY) {
      printf("m12\n");
      return 1;
    }
    g_malloc_fail_countdown = -1;
    ui_sort_header_base_toggle(sort, (void *)9);
    ui_sort_header_base_toggle(sort, (void *)10);
  }

  ui_sort_header_base_destroy(sort);
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_sort_header_single();
  failed |= test_sort_header_multi();
  if (failed)
    return 1;
  return 0;
}
