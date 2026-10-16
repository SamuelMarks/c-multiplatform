/**
 * @file test_ui_runtime_eject.c
 * @brief Unit tests for ui_runtime_eject code generator.
 */

/* clang-format off */
#include "ui_runtime_eject.h"
#include "ui_runtime_schema.h"
#include "ui_arena.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_eject_mock_format_fail;
extern int g_eject_mock_copy_fail;
#ifdef UI_TEST_MOCK_ALLOC
struct node_meta;
extern ui_error_t
ui_test_eject_init_node_meta(struct node_meta *meta, int id_num,
                             const struct ui_runtime_node *node);
#endif

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_runtime_node *node = NULL;
  char c_buf[16384];
  char h_buf[4096];
  ui_error_t rc;

  const char *test_json =
      "{\"id\":\"survey_card\",\"type\":\"ui_card_base\","
      "\"props\":{\"title\":\"Survey Question 1\"},\"children\":["
      "{\"id\":\"user_email\",\"type\":\"ui_input_base\","
      "\"props\":{\"placeholder\":\"name@domain.com\"},"
      "\"bindings\":{\"bind_cva\":\"user.email\"},"
      "\"validators\":[{\"type\":\"required\"},{\"type\":\"pattern\","
      "\"pattern\":\"^.+@.+\\\\..+$\"}]},"
      "{\"id\":\"submit_btn\",\"type\":\"ui_button_base\","
      "\"props\":{\"text\":\"Next\"},"
      "\"bindings\":{\"bind_disabled\":\"is_loading\"}}]}";

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks */
  rc = ui_runtime_eject_tree_to_c_buffer(NULL, "test_func", c_buf,
                                         sizeof(c_buf), h_buf, sizeof(h_buf));
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_schema_parse_node(arena, test_json, &node);
  assert(rc == UI_ERROR_NONE);
  assert(node != NULL);

  /* Eject to memory buffers */
  rc = ui_runtime_eject_tree_to_c_buffer(node, "test_survey_form", c_buf,
                                         sizeof(c_buf), h_buf, sizeof(h_buf));
  assert(rc == UI_ERROR_NONE);

  /* Header checks */
  assert(strstr(h_buf, "TEST_SURVEY_FORM_H") != NULL);
  assert(strstr(h_buf, "extern \"C\"") != NULL);
  assert(strstr(h_buf, "test_survey_form_create") != NULL);
  assert(strstr(h_buf, "/* clang-"
                       "format off */") != NULL);

  /* Source checks */
  assert(strstr(c_buf, "test_survey_form_create") != NULL);
  assert(strstr(c_buf, "ui_card_base_create") != NULL);
  assert(strstr(c_buf, "ui_input_base_create") != NULL);
  assert(strstr(c_buf, "ui_button_base_create") != NULL);
  assert(strstr(c_buf, "ui_input_base_get_cva") != NULL);
  assert(strstr(c_buf, "ui_form_control_bind_cva") != NULL);
  assert(strstr(c_buf, "ui_validators_required") != NULL);
  assert(strstr(c_buf, "ui_form_control_add_validator") != NULL);
  assert(strstr(c_buf, "goto cleanup;") != NULL);
  assert(strstr(c_buf, "cleanup:") != NULL);

  /* Eject to file test */
  rc = ui_runtime_eject_node_to_c(node, "test_survey_form", "test_out.c",
                                  "test_out.h");
  assert(rc == UI_ERROR_NONE);

  /* Remove temporary test files */
  remove("test_out.c");
  remove("test_out.h");

  /* Test ejecting column, grid, checkbox, label, slider */
  {
    const char *test_json2 = "{\n"
                             "  \"id\": \"col_root\",\n"
                             "  \"type\": \"column\",\n"
                             "  \"children\": [\n"
                             "    {\n"
                             "      \"id\": \"my_grid\",\n"
                             "      \"type\": \"grid\",\n"
                             "      \"children\": [\n"
                             "        {\n"
                             "          \"id\": \"chk_opt\",\n"
                             "          \"type\": \"ui_checkbox_base\"\n"
                             "        },\n"
                             "        {\n"
                             "          \"id\": \"lbl_txt\",\n"
                             "          \"type\": \"ui_label_base\"\n"
                             "        },\n"
                             "        {\n"
                             "          \"id\": \"sld_vol\",\n"
                             "          \"type\": \"ui_slider_base\"\n"
                             "        }\n"
                             "      ]\n"
                             "    }\n"
                             "  ]\n"
                             "}";
    struct ui_runtime_node *node2 = NULL;
    rc = ui_runtime_schema_parse_node(arena, test_json2, &node2);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_eject_tree_to_c_buffer(node2, "test_layout_form", c_buf,
                                           sizeof(c_buf), h_buf, sizeof(h_buf));
    assert(rc == UI_ERROR_NONE);
    assert(strstr(c_buf, "ui_checkbox_base_create") != NULL);
    assert(strstr(c_buf, "ui_label_base_create") != NULL);
    assert(strstr(c_buf, "ui_slider_base_create") != NULL);
  }

  /* Test manifest eject */
  {
    const char *manifest_json = "{\n"
                                "  \"routes\": [\n"
                                "    {\n"
                                "      \"path\": \"/\",\n"
                                "      \"root\": {\n"
                                "        \"type\": \"row\"\n"
                                "      }\n"
                                "    }\n"
                                "  ]\n"
                                "}";
    struct ui_runtime_app_manifest *manifest = NULL;
    rc = ui_runtime_schema_parse(arena, manifest_json, &manifest);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_eject_to_c(manifest, "manifest_ejected", "manifest_out.c",
                               "manifest_out.h");
    assert(rc == UI_ERROR_NONE);
    remove("manifest_out.c");
    remove("manifest_out.h");
  }

  /* Comprehensive component and truncation tests */
  {
    char comp_json[1024];
    const char *comp_p1 =
        "{\"id\":\"root_box\",\"type\":\"box\",\"props\":{\"data-role\":"
        "\"main\"},\"children\":["
        "{\"id\":\"btn1\",\"type\":\"ui_button_base\",\"props\":{\"text\":"
        "\"Click\",\"custom_btn_prop\":\"val\"},"
        "\"bindings\":{\"bind_cva\":\"btn.cva\",\"bind_text\":\"btn.label\","
        "\"bind_disabled\":\"btn.disabled\"}},"
        "{\"id\":\"inp1\",\"type\":\"ui_input_base\",\"props\":{"
        "\"placeholder\":\"Enter name\",\"custom_inp_prop\":\"val\"},"
        "\"bindings\":{\"bind_cva\":\"user.name\",\"bind_text\":\"some.text\","
        "\"bind_disabled\":\"some.disabled\"},";
    const char *comp_p2 =
        "\"validators\":[{\"type\":\"required\"},{\"type\":\"pattern\","
        "\"pattern\":\"^[A-Z]+$\"},"
        "{\"type\":\"pattern\"},{\"type\":\"min_length\",\"value\":5}]},"
        "{\"id\":\"card1\",\"type\":\"ui_card_base\",\"props\":{\"title\":"
        "\"Card Title\",\"custom_card_prop\":\"val\"}},"
        "{\"id\":\"chk1\",\"type\":\"ui_checkbox_base\",\"props\":{\"checked\":"
        "\"true\"},\"bindings\":{\"bind_visibility\":\"app.visible\"}},";
    const char *comp_p3 =
        "{\"id\":\"lbl1\",\"type\":\"ui_label_base\",\"props\":{\"text\":"
        "\"Label text\"}},"
        "{\"id\":\"sld1\",\"type\":\"ui_slider_base\",\"props\":{\"min\":\"0\"}"
        ",\"validators\":[{\"type\":\"pattern\"}]},"
        "{\"type\":\"row\"},{\"id\":\"col1\",\"type\":\"column\"},{\"id\":"
        "\"grid1\",\"type\":\"grid\"}]}";
    struct ui_runtime_node *comp_node = NULL;
    size_t sz;

#if defined(_MSC_VER)
    strcpy_s(comp_json, sizeof(comp_json), comp_p1);
    strcat_s(comp_json, sizeof(comp_json), comp_p2);
    strcat_s(comp_json, sizeof(comp_json), comp_p3);
#else
    strcpy(comp_json, comp_p1);
    strcat(comp_json, comp_p2);
    strcat(comp_json, comp_p3);
#endif

    rc = ui_runtime_schema_parse_node(arena, comp_json, &comp_node);
    assert(rc == UI_ERROR_NONE);
    assert(comp_node != NULL);

    rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "mount_all", c_buf,
                                           sizeof(c_buf), h_buf, sizeof(h_buf));
    assert(rc == UI_ERROR_NONE);

    /* Truncation loops testing every buffer boundary failure */
    for (sz = 1; sz <= 2000; sz += 1) {
      rc = ui_runtime_eject_tree_to_c_buffer(node, "mount_one", c_buf, sz,
                                             h_buf, sizeof(h_buf));
      if (rc != UI_ERROR_NONE) {
        /* expected truncation error */
      }
    }
    for (sz = 1; sz <= 600; sz += 1) {
      rc = ui_runtime_eject_tree_to_c_buffer(node, "mount_one", c_buf,
                                             sizeof(c_buf), h_buf, sz);
      if (rc != UI_ERROR_NONE) {
        /* expected truncation error */
      }
    }
    for (sz = 1; sz <= 10000; sz += 1) {
      rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "mount_all", c_buf, sz,
                                             h_buf, sizeof(h_buf));
      if (rc != UI_ERROR_NONE) {
        /* expected truncation error */
      }
    }
    for (sz = 1; sz <= 2000; sz += 1) {
      rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "mount_all", c_buf,
                                             sizeof(c_buf), h_buf, sz);
      if (rc != UI_ERROR_NONE) {
        /* expected truncation error */
      }
    }

    /* Long function name exceeding upper_name buffer */
    {
      char long_fn[256];
      memset(long_fn, 'a', 200);
      long_fn[200] = '\0';
      rc = ui_runtime_eject_tree_to_c_buffer(
          comp_node, long_fn, c_buf, sizeof(c_buf), h_buf, sizeof(h_buf));
      assert(rc != UI_ERROR_NONE);
      rc = ui_runtime_eject_node_to_c(comp_node, long_fn, "out.c", "out.h");
      assert(rc != UI_ERROR_NONE);
    }

    /* Manifest eject argument checks */
    {
      struct ui_runtime_app_manifest m_null;
      struct ui_runtime_route_def r_null;
      memset(&m_null, 0, sizeof(m_null));
      memset(&r_null, 0, sizeof(r_null));

      rc = ui_runtime_eject_to_c(NULL, "f", "c", "h");
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = ui_runtime_eject_to_c(&m_null, "f", "c", "h");
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      m_null.routes = &r_null;
      rc = ui_runtime_eject_to_c(&m_null, "f", "c", "h");
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
    }

    /* Node eject argument and file error checks */
    {

      rc = ui_runtime_eject_node_to_c(NULL, "f", "c", "h");
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = ui_runtime_eject_node_to_c(comp_node, NULL, "c", "h");
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = ui_runtime_eject_node_to_c(comp_node, "f", NULL, "h");
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = ui_runtime_eject_node_to_c(comp_node, "f", "c", NULL);
      assert(rc == UI_ERROR_INVALID_ARGUMENT);

      /* OOM checks */
      g_malloc_fail_countdown = 0;
      rc = ui_runtime_eject_node_to_c(comp_node, "f", "c", "h");
      assert(rc == UI_ERROR_OUT_OF_MEMORY);
      g_malloc_fail_countdown = 1;
      rc = ui_runtime_eject_node_to_c(comp_node, "f", "c", "h");
      assert(rc == UI_ERROR_OUT_OF_MEMORY);
      g_malloc_fail_countdown = -1;

      /* File open failure checks */
      rc = ui_runtime_eject_node_to_c(comp_node, "f",
                                      "/nonexistent_dir_1234/out.c", "out.h");
      assert(rc == UI_ERROR_UNKNOWN);
      rc = ui_runtime_eject_node_to_c(comp_node, "f", "out.c",
                                      "/nonexistent_dir_1234/out.h");
      assert(rc == UI_ERROR_UNKNOWN);
      remove("out.c");
    }

    /* Tree to c buffer argument checks */
    rc = ui_runtime_eject_tree_to_c_buffer(comp_node, NULL, c_buf,
                                           sizeof(c_buf), h_buf, sizeof(h_buf));
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
    rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "f", NULL, sizeof(c_buf),
                                           h_buf, sizeof(h_buf));
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
    rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "f", c_buf, 0, h_buf,
                                           sizeof(h_buf));
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
    rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "f", c_buf, sizeof(c_buf),
                                           NULL, sizeof(h_buf));
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
    rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "f", c_buf, sizeof(c_buf),
                                           h_buf, 0);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
  }

#ifdef UI_TEST_MOCK_ALLOC
  {
    struct ui_runtime_node single_node;
    char meta_buf[512];
    int fail_idx;

    memset(&single_node, 0, sizeof(single_node));
    single_node.type = "ui_button_base";

    rc = ui_test_eject_init_node_meta(NULL, 0, &single_node);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
    rc = ui_test_eject_init_node_meta((struct node_meta *)meta_buf, 0, NULL);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
    rc = ui_test_eject_init_node_meta((struct node_meta *)meta_buf, 0,
                                      &single_node);
    assert(rc == UI_ERROR_NONE);

    for (fail_idx = 1; fail_idx <= 6; fail_idx++) {
      g_eject_mock_format_fail = fail_idx;
      rc = ui_runtime_eject_tree_to_c_buffer(&single_node, "test_fail", c_buf,
                                             sizeof(c_buf), h_buf,
                                             sizeof(h_buf));
      assert(rc != UI_ERROR_NONE);
    }
    g_eject_mock_format_fail = 0;

    /* 6 calls in declarations, 7th in body */
    g_eject_mock_format_fail = 7;
    rc =
        ui_runtime_eject_tree_to_c_buffer(&single_node, "test_body_fail", c_buf,
                                          sizeof(c_buf), h_buf, sizeof(h_buf));
    assert(rc != UI_ERROR_NONE);
    g_eject_mock_format_fail = 0;

    /* 1st copy call: upper_name in tree_to_c_buffer */
    g_eject_mock_copy_fail = 1;
    rc =
        ui_runtime_eject_tree_to_c_buffer(&single_node, "test_copy_fail", c_buf,
                                          sizeof(c_buf), h_buf, sizeof(h_buf));
    assert(rc != UI_ERROR_NONE);
    g_eject_mock_copy_fail = 0;

    /* 2nd copy call: out_dom_var in emit_body_recursive */
    g_eject_mock_copy_fail = 2;
    rc = ui_runtime_eject_tree_to_c_buffer(&single_node, "test_copy_body_fail",
                                           c_buf, sizeof(c_buf), h_buf,
                                           sizeof(h_buf));
    assert(rc != UI_ERROR_NONE);
    g_eject_mock_copy_fail = 0;
  }
#endif

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_runtime_eject passed\n");
  return 0;
}
