/* clang-format off */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../include/ui_design_tokens.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../src/ui_internal_mem.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_ui_design_token_dict_init(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_error_t rc;

  ui_arena_create(16, &arena);

  rc = ui_design_token_dict_init(arena, &dict);
  if (rc != UI_ERROR_NONE || dict.count != 0 || dict.capacity != 64 ||
      dict.tokens == NULL)
    return 1;

  rc = ui_design_token_dict_init(NULL, &dict);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_design_token_dict_init(arena, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ui_design_token_dict_init_malloc_fail(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_error_t rc;

  ui_arena_create(16, &arena);

  g_malloc_fail_countdown = 0;
  rc = ui_design_token_dict_init(arena, &dict);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    (void)ui_arena_destroy(arena);
    return 1;
  }

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ui_design_token_set_get_color(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_color_t color;
  ui_error_t rc;

  ui_arena_create(16, &arena);
  ui_design_token_dict_init(arena, &dict);

  rc = ui_design_token_set_color(&dict, "primary-color", 0xFF00FF00);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_design_token_set_color(&dict, "primary-color", 0xFF0000FF);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_design_token_get_color(&dict, "primary-color", &color);
  if (rc != UI_ERROR_NONE || color != 0xFF0000FF)
    return 1;

  rc = ui_design_token_set_color(NULL, "color", 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_set_color(&dict, NULL, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_design_token_get_color(NULL, "color", &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_get_color(&dict, NULL, &color);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_get_color(&dict, "color", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_design_token_get_color(&dict, "missing", &color);
  if (rc != UI_ERROR_NOT_FOUND)
    return 1;

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ui_design_token_set_get_number(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  float number;
  ui_error_t rc;

  ui_arena_create(16, &arena);
  ui_design_token_dict_init(arena, &dict);

  rc = ui_design_token_set_number(&dict, "spacing-md", 16.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_design_token_get_number(&dict, "spacing-md", &number);
  if (rc != UI_ERROR_NONE || number != 16.0f)
    return 1;

  rc = ui_design_token_get_color(&dict, "spacing-md", (ui_color_t *)&number);
  if (rc != UI_ERROR_UNSUPPORTED)
    return 1;

  rc = ui_design_token_set_number(NULL, "num", 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_set_number(&dict, NULL, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_design_token_get_number(NULL, "num", &number);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_get_number(&dict, NULL, &number);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_get_number(&dict, "num", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_design_token_get_number(&dict, "missing", &number);
  if (rc != UI_ERROR_NOT_FOUND)
    return 1;

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ui_design_token_set_string(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_error_t rc;

  ui_arena_create(16, &arena);
  ui_design_token_dict_init(arena, &dict);

  rc = ui_design_token_set_string(&dict, "font-family", "Arial, sans-serif");
  if (rc != UI_ERROR_NONE)
    return 1;

  float number;
  rc = ui_design_token_get_number(&dict, "font-family", &number);
  if (rc != UI_ERROR_UNSUPPORTED)
    return 1;

  rc = ui_design_token_set_string(NULL, "str", "val");
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_set_string(&dict, NULL, "val");
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_set_string(&dict, "str", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    void *dummy;
    ui_arena_alloc(arena, 4096, 1, &dummy);
  }
  g_malloc_fail_countdown = 0;
  rc = ui_design_token_set_string(&dict, "str", "val");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    (void)ui_arena_destroy(arena);
    return 1;
  }

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ui_design_token_alias(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_color_t color;
  float number;
  ui_error_t rc;

  ui_arena_create(16, &arena);
  ui_design_token_dict_init(arena, &dict);

  ui_design_token_set_color(&dict, "base-red", 0xFFFF0000);
  ui_design_token_set_number(&dict, "base-spacing", 8.0f);

  rc = ui_design_token_set_alias(&dict, "theme-danger", "base-red");
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_design_token_set_alias(&dict, "spacing-sm", "base-spacing");
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_design_token_set_alias(&dict, "spacing-tiny", "spacing-sm");
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_design_token_get_color(&dict, "theme-danger", &color);
  if (rc != UI_ERROR_NONE || color != 0xFFFF0000)
    return 1;

  rc = ui_design_token_get_number(&dict, "spacing-tiny", &number);
  if (rc != UI_ERROR_NONE || number != 8.0f)
    return 1;

  rc = ui_design_token_set_alias(NULL, "a", "b");
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_set_alias(&dict, NULL, "b");
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_design_token_set_alias(&dict, "a", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    void *dummy;
    ui_arena_alloc(arena, 4096, 1, &dummy);
  }
  g_malloc_fail_countdown = 0;
  rc = ui_design_token_set_alias(&dict, "a", "b");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    (void)ui_arena_destroy(arena);
    return 1;
  }

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ui_design_token_alias_cycle(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_color_t color;
  ui_error_t rc;

  ui_arena_create(16, &arena);
  ui_design_token_dict_init(arena, &dict);

  ui_design_token_set_alias(&dict, "a", "b");
  ui_design_token_set_alias(&dict, "b", "c");
  ui_design_token_set_alias(&dict, "c", "a");

  rc = ui_design_token_get_color(&dict, "a", &color);
  if (rc != UI_ERROR_UNKNOWN)
    return 1;

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ui_design_token_capacity_expansion(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  char name[32];
  int i;
  ui_error_t rc;
  float val;

  ui_arena_create(16, &arena);
  ui_design_token_dict_init(arena, &dict);

  for (i = 0; i < 70; ++i) {
    sprintf(name, "token-%d", i);
    rc = ui_design_token_set_number(&dict, name, (float)i);
    if (rc != UI_ERROR_NONE)
      return 1;
  }

  if (dict.capacity <= 64 || dict.count != 70)
    return 1;

  ui_design_token_get_number(&dict, "token-0", &val);
  if (val != 0.0f)
    return 1;
  ui_design_token_get_number(&dict, "token-69", &val);
  if (val != 69.0f)
    return 1;

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ui_design_token_capacity_expansion_fail(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  char name[32];
  int i;
  ui_error_t rc;

  ui_arena_create(16, &arena);
  ui_design_token_dict_init(arena, &dict);

  for (i = 0; i < 64; ++i) {
    sprintf(name, "token-%d", i);
    rc = ui_design_token_set_number(&dict, name, (float)i);
    if (rc != UI_ERROR_NONE)
      return 1;
  }

  {
    void *dummy;
    ui_arena_alloc(arena, 4096, 1, &dummy);
  }

  g_malloc_fail_countdown = 0;
  rc = ui_design_token_set_number(&dict, "token-64", 64.0f);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    (void)ui_arena_destroy(arena);
    return 1;
  }

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_get_or_create_malloc_fail_name(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_error_t rc;

  ui_arena_create(16, &arena);
  ui_design_token_dict_init(arena, &dict);

  {
    void *dummy;
    ui_arena_alloc(arena, 4096, 1, &dummy);
  }

  g_malloc_fail_countdown = 0;
  rc = ui_design_token_set_number(&dict, "new_token", 1.0f);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    (void)ui_arena_destroy(arena);
    printf("1\n");
    return 1;
  }

  if (dict.count != 0) {
    (void)ui_arena_destroy(arena);
    printf("2\n");
    return 1;
  }

  /* For color */
  {
    void *dummy;
    ui_arena_alloc(arena, 4096, 1, &dummy);
  }
  g_malloc_fail_countdown = 0;
  rc = ui_design_token_set_color(&dict, "new_color", 0xFF000000);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    (void)ui_arena_destroy(arena);
    printf("3\n");
    return 1;
  }

  /* For string */
  {
    void *dummy;
    ui_arena_alloc(arena, 4096, 1, &dummy);
  }
  g_malloc_fail_countdown = 1;
  rc = ui_design_token_set_string(&dict, "new_string_very_long_name", "val");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    (void)ui_arena_destroy(arena);
    printf("4\n");
    return 1;
  }

  /* For alias */
  {
    void *dummy;
    ui_arena_alloc(arena, 4096, 1, &dummy);
  }
  g_malloc_fail_countdown = 1;
  rc = ui_design_token_set_alias(&dict, "new_alias_very_long_name", "target");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    (void)ui_arena_destroy(arena);
    printf("5\n");
    return 1;
  }

  (void)ui_arena_destroy(arena);
  return 0;
}

int main(void) {
  int failed = 0;

  if (test_ui_design_token_dict_init()) {
    printf("failed init\n");
    failed = 1;
  }
  if (test_ui_design_token_dict_init_malloc_fail()) {
    printf("failed init malloc fail\n");
    failed = 1;
  }
  if (test_ui_design_token_set_get_color()) {
    printf("failed color\n");
    failed = 1;
  }
  if (test_ui_design_token_set_get_number()) {
    printf("failed number\n");
    failed = 1;
  }
  if (test_ui_design_token_set_string()) {
    printf("failed string\n");
    failed = 1;
  }
  if (test_ui_design_token_alias()) {
    printf("failed alias\n");
    failed = 1;
  }
  if (test_ui_design_token_alias_cycle()) {
    printf("failed cycle\n");
    failed = 1;
  }
  if (test_ui_design_token_capacity_expansion()) {
    printf("failed expand\n");
    failed = 1;
  }
  if (test_ui_design_token_capacity_expansion_fail()) {
    printf("failed expand fail\n");
    failed = 1;
  }
  if (test_get_or_create_malloc_fail_name()) {
    printf("failed name fail\n");
    failed = 1;
  }

  if (failed) {
    printf("Tests failed\n");
    return 1;
  }

  printf("All tests passed\n");
  return 0;
}
