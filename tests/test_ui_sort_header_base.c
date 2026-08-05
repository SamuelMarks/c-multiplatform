/* clang-format off */
#include "ui_sort_header_base.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    ui_error_t _err = (expr);                                                  \
    if (_err != UI_ERROR_NONE) {                                               \
      printf("Failed at line %d: %d\n", __LINE__, _err);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    ui_error_t _err = (expr);                                                  \
    if (_err != (expected)) {                                                  \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             _err);                                                            \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_INT_EQ(expr, expected)                                          \
  do {                                                                         \
    int val = (expr);                                                          \
    if (val != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             val);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_PTR_EQ(expr, expected)                                          \
  do {                                                                         \
    void *val = (expr);                                                        \
    if (val != (expected)) {                                                   \
      printf("Failed at line %d: expected %p, got %p\n", __LINE__,             \
             (void *)(expected), (void *)val);                                 \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int test_ui_sort_header_base_create_destroy(void) {
  struct ui_sort_header_base *header = NULL;

  ASSERT_EQ(ui_sort_header_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_sort_header_base_create(&header));
  if (!header)
    return 1;

  ui_sort_header_base_destroy(header);
  ui_sort_header_base_destroy(NULL);
  return 0;
}

static int test_ui_sort_header_base_toggle_and_get(void) {
  struct ui_sort_header_base *header = NULL;
  enum ui_sort_direction dir;
  void *id1 = (void *)1;

  ASSERT_SUCCESS(ui_sort_header_base_create(&header));

  ASSERT_EQ(ui_sort_header_base_toggle(NULL, id1), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_EQ(ui_sort_header_base_get_direction(NULL, id1, &dir),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sort_header_base_get_direction(header, id1, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_sort_header_base_get_direction(header, id1, &dir));
  ASSERT_INT_EQ(dir, UI_SORT_NONE);

  ASSERT_SUCCESS(ui_sort_header_base_toggle(header, id1));
  ASSERT_SUCCESS(ui_sort_header_base_get_direction(header, id1, &dir));
  ASSERT_INT_EQ(dir, UI_SORT_ASCENDING);

  ASSERT_SUCCESS(ui_sort_header_base_toggle(header, id1));
  ASSERT_SUCCESS(ui_sort_header_base_get_direction(header, id1, &dir));
  ASSERT_INT_EQ(dir, UI_SORT_DESCENDING);

  ASSERT_SUCCESS(ui_sort_header_base_toggle(header, id1));
  ASSERT_SUCCESS(ui_sort_header_base_get_direction(header, id1, &dir));
  ASSERT_INT_EQ(dir, UI_SORT_NONE);

  ui_sort_header_base_destroy(header);
  return 0;
}

static int test_ui_sort_header_base_multi_sort(void) {
  struct ui_sort_header_base *header = NULL;
  enum ui_sort_direction dir;
  void *id1 = (void *)1;
  void *id2 = (void *)2;
  void *id3 = (void *)3;
  void *id4 = (void *)4;
  void *id5 = (void *)5;
  void *id6 = (void *)6;
  struct ui_sort_state states[10];
  size_t count;

  ASSERT_SUCCESS(ui_sort_header_base_create(&header));

  ASSERT_EQ(ui_sort_header_base_set_multi_sort(NULL, 1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_sort_header_base_set_multi_sort(header, 1));

  ASSERT_EQ(ui_sort_header_base_set_direction(NULL, id1, UI_SORT_ASCENDING),
            UI_ERROR_INVALID_ARGUMENT);

  /* Test capacity expansion */
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, id1, UI_SORT_ASCENDING));
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, id2, UI_SORT_DESCENDING));
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, id3, UI_SORT_ASCENDING));
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, id4, UI_SORT_DESCENDING));
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, id5, UI_SORT_ASCENDING));
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, id6, UI_SORT_DESCENDING));

  ASSERT_EQ(ui_sort_header_base_get_active_sorts(NULL, states, 10, &count),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sort_header_base_get_active_sorts(header, NULL, 10, &count),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_sort_header_base_get_active_sorts(header, states, 10, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(
      ui_sort_header_base_get_active_sorts(header, states, 10, &count));
  ASSERT_INT_EQ(count, 6);
  ASSERT_PTR_EQ(states[0].id, id1);
  ASSERT_INT_EQ(states[0].direction, UI_SORT_ASCENDING);

  /* Now change direction of existing state */
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, id2, UI_SORT_ASCENDING));
  ASSERT_SUCCESS(ui_sort_header_base_get_direction(header, id2, &dir));
  ASSERT_INT_EQ(dir, UI_SORT_ASCENDING);

  /* Turn multi sort off */
  ASSERT_SUCCESS(ui_sort_header_base_set_multi_sort(header, 0));
  ASSERT_SUCCESS(
      ui_sort_header_base_get_active_sorts(header, states, 10, &count));
  ASSERT_INT_EQ(count, 1); /* Keeps only first */

  ui_sort_header_base_destroy(header);
  return 0;
}

static int test_ui_sort_header_base_clear_and_bind(void) {
  struct ui_sort_header_base *header = NULL;
  void *id1 = (void *)1;
  size_t count;
  struct ui_sort_state states[10];

  ASSERT_SUCCESS(ui_sort_header_base_create(&header));

  ASSERT_EQ(ui_sort_header_base_clear(NULL), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, id1, UI_SORT_ASCENDING));
  ASSERT_SUCCESS(ui_sort_header_base_clear(header));

  ASSERT_SUCCESS(
      ui_sort_header_base_get_active_sorts(header, states, 10, &count));
  ASSERT_INT_EQ(count, 0);

  ASSERT_EQ(ui_sort_header_base_bind_direction(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_sort_header_base_bind_direction(header, NULL));

  ui_sort_header_base_destroy(header);
  return 0;
}

static int test_ui_sort_header_base_allocation_failure(void) {
  struct ui_sort_header_base *header = NULL;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(ui_sort_header_base_create(&header), UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  ASSERT_SUCCESS(ui_sort_header_base_create(&header));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  /* First element will trigger allocation of states array */
  ASSERT_EQ(
      ui_sort_header_base_set_direction(header, (void *)1, UI_SORT_ASCENDING),
      UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  ui_sort_header_base_destroy(header);
  return 0;
}

static int test_ui_sort_header_base_edge_cases(void) {
  struct ui_sort_header_base *header = NULL;
  struct ui_sort_state states[2];
  size_t count;
  ASSERT_SUCCESS(ui_sort_header_base_create(&header));

  /* Set NONE on non-existent element */
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, (void *)99, UI_SORT_NONE));

  /* Setting something and then removing it to trigger remove_state_at */
  ASSERT_SUCCESS(ui_sort_header_base_set_multi_sort(header, 1));
  ASSERT_SUCCESS(ui_sort_header_base_set_direction(header, (void *)100,
                                                   UI_SORT_ASCENDING));
  ASSERT_SUCCESS(ui_sort_header_base_set_direction(header, (void *)101,
                                                   UI_SORT_DESCENDING));
  ASSERT_SUCCESS(
      ui_sort_header_base_set_direction(header, (void *)100, UI_SORT_NONE));

  /* Test capacity < count in get_active_sorts */
  ASSERT_SUCCESS(ui_sort_header_base_set_direction(header, (void *)102,
                                                   UI_SORT_ASCENDING));
  ASSERT_SUCCESS(
      ui_sort_header_base_get_active_sorts(header, states, 1, &count));
  ASSERT_INT_EQ(count, 1);

  /* Test remove_state_at out of bounds explicitly if possible.
     Wait, remove_state_at is static. But we can trigger toggling NONE on
     existing. */
  ASSERT_SUCCESS(ui_sort_header_base_toggle(header, (void *)101));
  ASSERT_SUCCESS(ui_sort_header_base_toggle(header, (void *)101));

  /* Test default case in toggle's switch */
  ASSERT_SUCCESS(ui_sort_header_base_set_direction(
      header, (void *)103, (enum ui_sort_direction)999));
  ASSERT_SUCCESS(ui_sort_header_base_toggle(header, (void *)103));

  /* Test set_multi_sort when count <= 1 */
  ASSERT_SUCCESS(ui_sort_header_base_clear(header));
  ASSERT_SUCCESS(ui_sort_header_base_set_direction(header, (void *)104,
                                                   UI_SORT_ASCENDING));
  ASSERT_SUCCESS(ui_sort_header_base_set_multi_sort(header, 0));

  ui_sort_header_base_destroy(header);
  return 0;
}

int main(void) {
  if (test_ui_sort_header_base_create_destroy())
    return 1;
  if (test_ui_sort_header_base_toggle_and_get())
    return 1;
  if (test_ui_sort_header_base_multi_sort())
    return 1;
  if (test_ui_sort_header_base_clear_and_bind())
    return 1;
  if (test_ui_sort_header_base_allocation_failure())
    return 1;
  if (test_ui_sort_header_base_edge_cases())
    return 1;
  return 0;
}
