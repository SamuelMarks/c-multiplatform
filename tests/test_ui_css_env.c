/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_css_env.h"
#include "ui_css_values.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_env_manager(void) {
  struct ui_css_env_manager *manager = NULL;
  struct ui_css_value_ext *value = NULL;
  struct ui_css_value_ext *value2 = NULL;
  struct ui_css_value_ext *val_ptr = NULL;
  const struct ui_css_value_ext *retrieved = NULL;
  ui_error_t rc;

  rc = ui_css_env_manager_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_css_env_manager_create(&manager);
  if (rc != UI_ERROR_NONE || !manager)
    return 1;

  ui_css_env_manager_destroy(NULL); /* no-op */

  if (ui_css_env_manager_set(NULL, "a", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_env_manager_set(manager, NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_env_manager_set(manager, "a", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_env_manager_get(manager, NULL, &retrieved) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_env_manager_get(manager, "a", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_env_manager_remove(NULL, "a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_env_manager_remove(manager, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_css_parse_value_ext("10px", &value);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_css_env_manager_set(manager, "safe-area-inset-top", value);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_css_parse_value_ext("15px", &value2);
  if (rc != UI_ERROR_NONE)
    return 1;
  /* Update existing */
  rc = ui_css_env_manager_set(manager, "safe-area-inset-top", value2);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_css_env_manager_get(NULL, "a", NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_css_env_manager_get(manager, "safe-area-inset-top", &retrieved);
  if (rc != UI_ERROR_NONE || !retrieved)
    return 1;
  if (retrieved->type != UI_CSS_VALUE_TYPE_SCALAR ||
      retrieved->value.scalar.value != 15.0f) {
    return 1;
  }

  rc = ui_css_env_manager_get(manager, "non-existent", &retrieved);
  if (rc != UI_ERROR_NOT_FOUND)
    return 1;

  rc = ui_css_env_manager_remove(NULL, "a");
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_css_env_manager_remove(manager, "non-existent");
  if (rc != UI_ERROR_NOT_FOUND)
    return 1;

  rc = ui_css_env_manager_remove(manager, "safe-area-inset-top");
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Destroy with entries inside */
  rc = ui_css_parse_value_ext("10px", &value);
  ui_css_env_manager_set(manager, "test1", value);

  /* Destroy handles it */
  ui_css_env_manager_destroy(manager);

  /* OOM */
  g_malloc_fail_countdown = 0;
  if (ui_css_env_manager_create(&manager) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_css_env_manager_create(&manager);
  rc = ui_css_parse_value_ext("10px", &value);
  g_malloc_fail_countdown = 0;
  if (ui_css_env_manager_set(manager, "test2", value) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
  ui_css_value_ext_destroy(value);
  ui_css_env_manager_destroy(manager);

  return 0;
}

static int test_env_parser(void) {
  struct ui_css_value_ext *ext = NULL;
  ui_error_t rc;

  rc = ui_css_parse_value_ext("env(safe-area-inset-top, 20px)", &ext);
  if (rc != UI_ERROR_NONE || !ext)
    return 1;

  if (ext->type != UI_CSS_VALUE_TYPE_ENV)
    return 1;
  if (strcmp(ext->value.env->name, "safe-area-inset-top") != 0)
    return 1;
  if (!ext->value.env->fallback)
    return 1;
  if (ext->value.env->fallback->type != UI_CSS_VALUE_TYPE_SCALAR)
    return 1;
  if (ext->value.env->fallback->value.scalar.value != 20.0f)
    return 1;

  ui_css_value_ext_destroy(ext);
  return 0;
}

static int test_env_remove(void) {
  struct ui_css_env_manager *manager = NULL;
  struct ui_css_value_ext *v1 = NULL;
  struct ui_css_value_ext *v2 = NULL;
  struct ui_css_value_ext *v3 = NULL;

  ui_css_env_manager_create(&manager);

  ui_css_parse_value_ext("1px", &v1);
  ui_css_parse_value_ext("2px", &v2);
  ui_css_parse_value_ext("3px", &v3);

  ui_css_env_manager_set(manager, "first", v1);
  ui_css_env_manager_set(manager, "second", v2);
  ui_css_env_manager_set(manager, "third", v3);

  /* list is third -> second -> first */

  /* remove middle (second) */
  if (ui_css_env_manager_remove(manager, "second") != UI_ERROR_NONE)
    return 1;

  ui_css_env_manager_destroy(manager);
  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_env_manager();
  failures += test_env_parser();
  failures += test_env_remove();

  if (failures == 0) {
    printf("test_ui_css_env passed\n");
  } else {
    printf("test_ui_css_env failed with %d errors\n", failures);
  }

  return failures;
}
