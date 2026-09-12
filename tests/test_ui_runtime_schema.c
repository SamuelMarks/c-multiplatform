/**
 * @file test_ui_runtime_schema.c
 * @brief Unit tests for ui_runtime_schema validation and parsing.
 */

/* clang-format off */
#include "ui_runtime_schema.h"
#include "ui_arena.h"
#include "ui_coercion_utils.h"
#include "ui_test_mock_mem.h"
#include "parson.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

static int s_parson_calls = 0;
static int s_parson_fail_at = -1;
static void *mock_parson_malloc(size_t size) {
  s_parson_calls++;
  if (s_parson_fail_at >= 0 && s_parson_calls >= s_parson_fail_at) {
    return NULL;
  }
  return malloc(size);
}

int main(void) {
  struct ui_arena *arena = NULL;
  ui_error_t rc;
  struct ui_runtime_app_manifest *manifest = NULL;
  struct ui_runtime_node *single_node = NULL;

  json_set_allocation_functions(mock_parson_malloc, free);

  const char *valid_manifest_json =
      "{\n"
      "  \"app_id\": \"survey_app\",\n"
      "  \"initial_route\": \"/home\",\n"
      "  \"global_state\": {\n"
      "    \"user_name\": \"Alice\",\n"
      "    \"cart_count\": 5,\n"
      "    \"is_admin\": true\n"
      "  },\n"
      "  \"routes\": [\n"
      "    {\n"
      "      \"path\": \"/home\",\n"
      "      \"title\": \"Home Screen\",\n"
      "      \"root\": {\n"
      "        \"id\": \"root_card\",\n"
      "        \"type\": \"ui_card_base\",\n"
      "        \"props\": {\n"
      "          \"title\": \"Welcome\",\n"
      "          \"elevation\": 2\n"
      "        },\n"
      "        \"bindings\": {\n"
      "          \"bind_text\": \"app.user_name\"\n"
      "        },\n"
      "        \"validators\": [\n"
      "          {\"type\": \"required\"},\n"
      "          {\"type\": \"pattern\", \"pattern\": \"^[a-z]+$\"},\n"
      "          {\"type\": \"min_length\", \"min_length\": 3},\n"
      "          {\"type\": \"max_length\", \"max_length\": 50}\n"
      "        ],\n"
      "        \"events\": {\n"
      "          \"on_click\": {\n"
      "            \"steps\": [\n"
      "              {\"stepId\": \"step1\", \"action\": \"mutate_state\", "
      "\"target\": \"cart_count\", \"value\": \"6\"},\n"
      "              {\"stepId\": \"step2\", \"action\": \"navigate\", "
      "\"target\": \"/checkout\"},\n"
      "              {\"stepId\": \"step3\", \"action\": \"http_request\", "
      "\"target\": \"/api/submit\", \"successCriteria\": \"$code == 200\"}\n"
      "            ]\n"
      "          }\n"
      "        },\n"
      "        \"children\": [\n"
      "          {\n"
      "            \"id\": \"btn_submit\",\n"
      "            \"type\": \"ui_button_base\",\n"
      "            \"props\": {\n"
      "              \"text\": \"Submit\"\n"
      "            },\n"
      "            \"bindings\": {\n"
      "              \"bind_disabled\": \"is_submitting\",\n"
      "              \"bind_visibility\": \"show_btn\"\n"
      "            }\n"
      "          },\n"
      "          {\n"
      "            \"id\": \"btn_cancel\",\n"
      "            \"type\": \"ui_button_base\",\n"
      "            \"props\": {\n"
      "              \"text\": \"Cancel\"\n"
      "            }\n"
      "          }\n"
      "        ]\n"
      "      }\n"
      "    },\n"
      "    {\n"
      "      \"path\": \"/about\",\n"
      "      \"root\": {\n"
      "        \"type\": \"ui_label_base\"\n"
      "      }\n"
      "    }\n"
      "  ]\n"
      "}";

  const char *valid_node_json = "{\n"
                                "  \"id\": \"input_email\",\n"
                                "  \"type\": \"ui_input_base\",\n"
                                "  \"props\": {\n"
                                "    \"placeholder\": \"Enter email\",\n"
                                "    \"type\": \"email\"\n"
                                "  },\n"
                                "  \"bindings\": {\n"
                                "    \"bind_cva\": \"user.email\"\n"
                                "  },\n"
                                "  \"validators\": [\n"
                                "    {\"type\": \"required\"}\n"
                                "  ]\n"
                                "}";

  const char *unknown_field_json = "{\n"
                                   "  \"app_id\": \"test\",\n"
                                   "  \"unknown_key\": 123,\n"
                                   "  \"routes\": []\n"
                                   "}";

  const char *unknown_node_field_json = "{\n"
                                        "  \"id\": \"btn1\",\n"
                                        "  \"type\": \"ui_button_base\",\n"
                                        "  \"bordr_radius\": 10\n"
                                        "}";

  const char *rich_node_json =
      "{\n"
      "  \"id\": \"rich_node\",\n"
      "  \"type\": \"custom_widget\",\n"
      "  \"props\": {\n"
      "    \"bool_p\": false,\n"
      "    \"num_p\": 12.34,\n"
      "    \"str_p\": \"text\"\n"
      "  },\n"
      "  \"bindings\": {\n"
      "    \"bind_disabled\": \"app.is_disabled\",\n"
      "    \"bind_visibility\": \"app.is_visible\",\n"
      "    \"bind_signal\": \"app.custom_sig\"\n"
      "  },\n"
      "  \"validators\": [\n"
      "    {\"type\": \"pattern\"},\n"
      "    {\"type\": \"pattern\", \"pattern\": \"^\\\\d+$\"},\n"
      "    {\"type\": \"min_length\", \"value\": 5},\n"
      "    {\"type\": \"max_length\", \"value\": 10}\n"
      "  ],\n"
      "  \"events\": {\n"
      "    \"on_change\": {\n"
      "      \"steps\": [\n"
      "        {\"stepId\": \"s1\", \"action\": \"navigate\", \"target\": "
      "\"/next\"},\n"
      "        {\"action\": \"http_request\"}\n"
      "      ]\n"
      "    },\n"
      "    \"on_submit\": {\n"
      "      \"steps\": []\n"
      "    },\n"
      "    \"on_blur\": {\n"
      "      \"steps\": []\n"
      "    }\n"
      "  }\n"
      "}";

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks */
  rc = ui_runtime_schema_validate(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_schema_parse(NULL, valid_manifest_json, &manifest);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_schema_parse(arena, NULL, &manifest);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_schema_parse(arena, valid_manifest_json, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Malformed JSON */
  rc = ui_runtime_schema_validate("{not a json");
  assert(rc == UI_ERROR_PARSE_FAILED);

  /* Manifest with type property (tests both branches of single_node detection)
   */
  rc = ui_runtime_schema_validate(
      "{\"app_id\": \"test\", \"type\": \"app\", \"routes\": []}");
  assert(rc == UI_ERROR_PARSE_FAILED);

  /* additionalProperties: false violations */
  rc = ui_runtime_schema_validate(unknown_field_json);
  assert(rc == UI_ERROR_PARSE_FAILED);

  rc = ui_runtime_schema_validate(unknown_node_field_json);
  assert(rc == UI_ERROR_PARSE_FAILED);

  /* Valid manifest validation & parsing */
  rc = ui_runtime_schema_validate(valid_manifest_json);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_schema_parse(arena, valid_manifest_json, &manifest);
  assert(rc == UI_ERROR_NONE);
  assert(manifest != NULL);
  assert(strcmp(manifest->app_id, "survey_app") == 0);
  assert(strcmp(manifest->initial_route, "/home") == 0);

  /* Check global state entries */
  assert(manifest->global_state != NULL);
  assert(strcmp(manifest->global_state->key, "user_name") == 0);
  assert(strcmp(manifest->global_state->initial_val, "Alice") == 0);

  /* Check routes */
  assert(manifest->routes != NULL);
  assert(strcmp(manifest->routes->path, "/home") == 0);
  assert(strcmp(manifest->routes->title, "Home Screen") == 0);

  /* Check root node */
  assert(manifest->routes->root_node != NULL);
  assert(strcmp(manifest->routes->root_node->id, "root_card") == 0);
  assert(strcmp(manifest->routes->root_node->type, "ui_card_base") == 0);

  /* Check props */
  assert(manifest->routes->root_node->props != NULL);
  assert(strcmp(manifest->routes->root_node->props->key, "title") == 0);
  assert(strcmp(manifest->routes->root_node->props->val, "Welcome") == 0);

  /* Check bindings */
  assert(manifest->routes->root_node->bindings != NULL);
  assert(manifest->routes->root_node->bindings->type ==
         UI_RUNTIME_BINDING_TEXT);
  assert(strcmp(manifest->routes->root_node->bindings->source_path,
                "app.user_name") == 0);

  /* Check validators */
  assert(manifest->routes->root_node->validators != NULL);
  assert(manifest->routes->root_node->validators->type ==
         UI_RUNTIME_VALIDATOR_REQUIRED);

  /* Check workflows */
  assert(manifest->routes->root_node->workflows != NULL);
  assert(strcmp(manifest->routes->root_node->workflows->event_name,
                "on_click") == 0);
  assert(manifest->routes->root_node->workflows->first_step != NULL);
  assert(manifest->routes->root_node->workflows->first_step->action ==
         UI_RUNTIME_ACTION_MUTATE_STATE);
  assert(strcmp(manifest->routes->root_node->workflows->first_step->target,
                "cart_count") == 0);

  /* Check children */
  assert(manifest->routes->root_node->first_child != NULL);
  assert(strcmp(manifest->routes->root_node->first_child->id, "btn_submit") ==
         0);
  assert(strcmp(manifest->routes->root_node->first_child->type,
                "ui_button_base") == 0);

  /* Single node parsing */
  rc = ui_runtime_schema_validate(valid_node_json);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_schema_parse_node(arena, valid_node_json, &single_node);
  assert(rc == UI_ERROR_NONE);
  assert(single_node != NULL);
  assert(strcmp(single_node->id, "input_email") == 0);
  assert(strcmp(single_node->type, "ui_input_base") == 0);
  assert(single_node->bindings != NULL);
  assert(single_node->bindings->type == UI_RUNTIME_BINDING_CVA);
  assert(strcmp(single_node->bindings->source_path, "user.email") == 0);

  /* Comprehensive invalid schemas testing */
  {
    static const char *const invalid_schemas[] = {
        "\"just a string\"",
        "12345",
        "true",
        "{\"id\": \"node1\"}",
        "{\"type\": \"\"}",
        "{\"type\": \"btn\", \"id\": 123}",
        "{\"type\": \"btn\", \"props\": \"not_an_obj\"}",
        "{\"type\": \"btn\", \"props\": {\"bad_prop\": [1,2]}}",
        "{\"type\": \"btn\", \"bindings\": \"not_an_obj\"}",
        "{\"type\": \"btn\", \"bindings\": {\"unknown_binding\": \"foo\"}}",
        "{\"type\": \"btn\", \"bindings\": {\"bind_text\": 123}}",
        "{\"type\": \"btn\", \"validators\": \"not_an_arr\"}",
        "{\"type\": \"btn\", \"validators\": [\"not_an_obj\"]}",
        "{\"type\": \"btn\", \"validators\": [{\"type\": \"required\", "
        "\"bad_k\": 1}]}",
        "{\"type\": \"btn\", \"validators\": [{}]}",
        "{\"type\": \"btn\", \"validators\": [{\"type\": \"unknown_val\"}]}",
        "{\"type\": \"btn\", \"events\": \"not_an_obj\"}",
        "{\"type\": \"btn\", \"events\": {\"on_unknown\": {\"steps\": []}}}",
        "{\"type\": \"btn\", \"events\": {\"on_click\": \"not_an_obj\"}}",
        "{\"type\": \"btn\", \"events\": {\"on_click\": {\"steps\": [], "
        "\"bad_key\": 1}}}",
        "{\"type\": \"btn\", \"events\": {\"on_click\": {\"steps\": "
        "\"not_an_arr\"}}}",
        "{\"type\": \"btn\", \"events\": {\"on_click\": {\"steps\": "
        "[\"not_an_obj\"]}}}",
        "{\"type\": \"btn\", \"events\": {\"on_click\": {\"steps\": "
        "[{\"action\": \"navigate\", \"bad_step_key\": 1}]}}}",
        "{\"type\": \"btn\", \"events\": {\"on_click\": {\"steps\": [{}]}}}",
        "{\"type\": \"btn\", \"events\": {\"on_click\": {\"steps\": "
        "[{\"action\": \"unknown_act\"}]}}}",
        "{\"type\": \"btn\", \"children\": \"not_an_arr\"}",
        "{\"type\": \"btn\", \"children\": [\"not_an_obj\"]}",
        "{\"type\": \"btn\", \"children\": [{\"id\": \"no_type\"}]}",
        "{\"type\": \"btn\", \"children\": [{\"bad_k\": 1}]}",
        "{\"app_id\": \"test\"}",
        "{\"app_id\": 123, \"routes\": []}",
        "{\"initial_route\": 123, \"routes\": []}",
        "{\"global_state\": \"not_an_obj\", \"routes\": []}",
        "{\"global_state\": {\"key\": [1, 2]}, \"routes\": []}",
        "{\"routes\": \"not_an_arr\"}",
        "{\"routes\": [\"not_an_obj\"]}",
        "{\"routes\": [{\"path\": \"/\", \"root\": [123]}]}",
        "{\"routes\": [{\"path\": \"/\", \"root\": {\"id\": \"no_type\"}}]}",
        "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"btn\"}, "
        "\"extra_k\": 1}]}",
        "{\"routes\": [{\"root\": {\"type\": \"btn\"}}]}",
        "{\"routes\": [{\"path\": \"/\"}]}",
        "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"\"}}]}",
        NULL};
    int idx = 0;
    while (invalid_schemas[idx]) {
      rc = ui_runtime_schema_validate(invalid_schemas[idx]);
      assert(rc == UI_ERROR_PARSE_FAILED);
      idx++;
    }
  }

  /* Test single node parsing edge cases */
  {
    struct ui_runtime_node *rich_node = NULL;
    rc = ui_runtime_schema_validate(rich_node_json);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_schema_parse_node(arena, rich_node_json, &rich_node);
    assert(rc == UI_ERROR_NONE);
    assert(rich_node != NULL);
  }

  /* Test manifest without title in route, and without app_id/initial_route */
  {
    const char *minimal_manifest_json = "{\n"
                                        "  \"routes\": [\n"
                                        "    {\n"
                                        "      \"path\": \"/simple\",\n"
                                        "      \"root\": {\n"
                                        "        \"type\": \"container\"\n"
                                        "      }\n"
                                        "    }\n"
                                        "  ]\n"
                                        "}";
    struct ui_runtime_app_manifest *min_m = NULL;
    rc = ui_runtime_schema_validate(minimal_manifest_json);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_schema_parse(arena, minimal_manifest_json, &min_m);
    assert(rc == UI_ERROR_NONE);
    assert(min_m != NULL);
    assert(min_m->routes != NULL);
    assert(min_m->routes->title == NULL);
  }

  /* Test parse_node argument validation */
  rc = ui_runtime_schema_parse_node(NULL, valid_node_json, &single_node);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_schema_parse_node(arena, NULL, &single_node);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_schema_parse_node(arena, valid_node_json, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_schema_parse_node(arena, "{bad json", &single_node);
  assert(rc == UI_ERROR_PARSE_FAILED);
  rc = ui_runtime_schema_parse(arena, "{bad json", &manifest);
  assert(rc == UI_ERROR_PARSE_FAILED);

  /* Test arena OOM during parse */
  {
    int i;
    extern int g_malloc_fail_countdown;
    extern int g_mock_strcpy_fail;
    struct ui_runtime_node *fail_node = NULL;
    struct ui_runtime_app_manifest *fail_m = NULL;
    struct ui_arena *small_arena = NULL;

    rc = ui_arena_create(16, &small_arena);
    assert(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 0;
    rc = ui_runtime_schema_parse_node(small_arena, valid_node_json, &fail_node);
    assert(rc != UI_ERROR_NONE);

    g_malloc_fail_countdown = 0;
    rc = ui_runtime_schema_parse(small_arena, valid_manifest_json, &fail_m);
    assert(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;

    ui_arena_destroy(small_arena);

    for (i = 0; i < 35; i++) {
      struct ui_arena *oom_arena = NULL;
      struct ui_runtime_node *oom_node = NULL;

      rc = ui_arena_create(1, &oom_arena);
      assert(rc == UI_ERROR_NONE);

      g_malloc_fail_countdown = i;
      rc = ui_runtime_schema_parse_node(oom_arena, rich_node_json, &oom_node);
      (void)rc;
      g_malloc_fail_countdown = -1;

      ui_arena_destroy(oom_arena);
    }

    for (i = 0; i < 75; i++) {
      struct ui_arena *oom_arena = NULL;
      struct ui_runtime_app_manifest *oom_m = NULL;

      rc = ui_arena_create(1, &oom_arena);
      assert(rc == UI_ERROR_NONE);

      g_malloc_fail_countdown = i;
      rc = ui_runtime_schema_parse(oom_arena, valid_manifest_json, &oom_m);
      (void)rc;
      g_malloc_fail_countdown = -1;

      ui_arena_destroy(oom_arena);
    }

    g_mock_strcpy_fail = 1;
    rc = ui_runtime_schema_parse_node(arena, valid_node_json, &fail_node);
    assert(rc == UI_ERROR_UNKNOWN);
    g_mock_strcpy_fail = 0;

    /* Fail st->step_id arena_strdup */
    {
      const char *step_oom_json =
          "{\"type\": \"btn\", \"events\": {\"on_click\": {\"steps\": "
          "[{\"stepId\": \"very_long_step_id_that_exceeds_padding\", "
          "\"action\": \"navigate\"}]}}}";
      struct ui_arena *fresh_arena = NULL;
      struct ui_runtime_node *st_node = NULL;
      rc = ui_arena_create(1, &fresh_arena);
      assert(rc == UI_ERROR_NONE);
      g_malloc_fail_countdown = 4;
      rc = ui_runtime_schema_parse_node(fresh_arena, step_oom_json, &st_node);
      assert(rc == UI_ERROR_OUT_OF_MEMORY);
      g_malloc_fail_countdown = -1;
      ui_arena_destroy(fresh_arena);
    }

    /* Fail rdef->path and rdef->title arena_strdup */
    {
      const char *route_title_oom_json =
          "{\"routes\": [{\"path\": "
          "\"/very_long_path_that_takes_its_own_block\", \"title\": "
          "\"very_long_title_that_takes_its_own_block\", \"root\": {\"type\": "
          "\"btn\"}}]}";
      struct ui_arena *fresh_arena = NULL;
      struct ui_runtime_app_manifest *fail_rt_m = NULL;

      /* Fail rpath (alloc 2) */
      rc = ui_arena_create(1, &fresh_arena);
      assert(rc == UI_ERROR_NONE);
      g_malloc_fail_countdown = 2;
      rc = ui_runtime_schema_parse(fresh_arena, route_title_oom_json,
                                   &fail_rt_m);
      assert(rc == UI_ERROR_OUT_OF_MEMORY);
      g_malloc_fail_countdown = -1;
      ui_arena_destroy(fresh_arena);

      /* Fail rtitle (alloc 3) */
      rc = ui_arena_create(1, &fresh_arena);
      assert(rc == UI_ERROR_NONE);
      g_malloc_fail_countdown = 3;
      rc = ui_runtime_schema_parse(fresh_arena, route_title_oom_json,
                                   &fail_rt_m);
      assert(rc == UI_ERROR_OUT_OF_MEMORY);
      g_malloc_fail_countdown = -1;
      ui_arena_destroy(fresh_arena);
    }

    /* Trigger !root_val in ui_runtime_schema_parse */
    {
      int base_calls;
      struct ui_runtime_app_manifest *fail_m2 = NULL;
      s_parson_calls = 0;
      s_parson_fail_at = -1;
      rc = ui_runtime_schema_validate(valid_manifest_json);
      assert(rc == UI_ERROR_NONE);
      base_calls = s_parson_calls;

      s_parson_calls = 0;
      s_parson_fail_at = base_calls + 1;
      rc = ui_runtime_schema_parse(arena, valid_manifest_json, &fail_m2);
      assert(rc == UI_ERROR_PARSE_FAILED);
      s_parson_fail_at = -1;
    }

    /* Trigger !root_val in ui_runtime_schema_parse_node */
    {
      int base_calls;
      struct ui_runtime_node *fail_n2 = NULL;
      s_parson_calls = 0;
      s_parson_fail_at = -1;
      rc = ui_runtime_schema_validate(valid_node_json);
      assert(rc == UI_ERROR_NONE);
      base_calls = s_parson_calls;

      s_parson_calls = 0;
      s_parson_fail_at = base_calls + 1;
      rc = ui_runtime_schema_parse_node(arena, valid_node_json, &fail_n2);
      assert(rc == UI_ERROR_PARSE_FAILED);
      s_parson_fail_at = -1;
    }
  }

  /* Test prop and global_state long string overflow (> 127 chars) */
  {
    char long_str[256];
    char full_json[1024];
    struct ui_runtime_node *lp_node = NULL;
    struct ui_runtime_app_manifest *lg_m = NULL;

    memset(long_str, 'a', 200);
    long_str[200] = '\0';

    rc = ui_safe_string_format(
        full_json, sizeof(full_json),
        "{\"type\": \"btn\", \"props\": {\"k\": \"%s\"}}", long_str);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_schema_parse_node(arena, full_json, &lp_node);
    assert(rc != UI_ERROR_NONE);

    rc = ui_safe_string_format(
        full_json, sizeof(full_json),
        "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"btn\"}}], "
        "\"global_state\": {\"g\": \"%s\"}}",
        long_str);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_schema_parse(arena, full_json, &lg_m);
    assert(rc != UI_ERROR_NONE);
  }

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_runtime_schema passed\n");
  return 0;
}
