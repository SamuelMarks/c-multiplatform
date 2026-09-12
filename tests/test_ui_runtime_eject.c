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

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_runtime_node *node = NULL;
  char c_buf[16384];
  char h_buf[4096];
  ui_error_t rc;

  const char *test_json =
      "{\n"
      "  \"id\": \"survey_card\",\n"
      "  \"type\": \"ui_card_base\",\n"
      "  \"props\": {\n"
      "    \"title\": \"Survey Question 1\"\n"
      "  },\n"
      "  \"children\": [\n"
      "    {\n"
      "      \"id\": \"user_email\",\n"
      "      \"type\": \"ui_input_base\",\n"
      "      \"props\": {\n"
      "        \"placeholder\": \"name@domain.com\"\n"
      "      },\n"
      "      \"bindings\": {\n"
      "        \"bind_cva\": \"user.email\"\n"
      "      },\n"
      "      \"validators\": [\n"
      "        {\"type\": \"required\"},\n"
      "        {\"type\": \"pattern\", \"pattern\": \"^.+@.+\\\\..+$\"}\n"
      "      ]\n"
      "    },\n"
      "    {\n"
      "      \"id\": \"submit_btn\",\n"
      "      \"type\": \"ui_button_base\",\n"
      "      \"props\": {\n"
      "        \"text\": \"Next\"\n"
      "      },\n"
      "      \"bindings\": {\n"
      "        \"bind_disabled\": \"is_loading\"\n"
      "      }\n"
      "    }\n"
      "  ]\n"
      "}";

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
  assert(strstr(h_buf, "/* clang-format off */") != NULL);

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
    const char *comp_json =
        "{\n"
        "  \"id\": \"root_box\",\n"
        "  \"type\": \"box\",\n"
        "  \"props\": {\n"
        "    \"data-role\": \"main\"\n"
        "  },\n"
        "  \"children\": [\n"
        "    {\n"
        "      \"id\": \"btn1\",\n"
        "      \"type\": \"ui_button_base\",\n"
        "      \"props\": {\n"
        "        \"text\": \"Click\",\n"
        "        \"custom_btn_prop\": \"val\"\n"
        "      },\n"
        "      \"bindings\": {\n"
        "        \"bind_cva\": \"btn.cva\",\n"
        "        \"bind_text\": \"btn.label\",\n"
        "        \"bind_disabled\": \"btn.disabled\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"inp1\",\n"
        "      \"type\": \"ui_input_base\",\n"
        "      \"props\": {\n"
        "        \"placeholder\": \"Enter name\",\n"
        "        \"custom_inp_prop\": \"val\"\n"
        "      },\n"
        "      \"bindings\": {\n"
        "        \"bind_cva\": \"user.name\",\n"
        "        \"bind_text\": \"some.text\",\n"
        "        \"bind_disabled\": \"some.disabled\"\n"
        "      },\n"
        "      \"validators\": [\n"
        "        {\"type\": \"required\"},\n"
        "        {\"type\": \"pattern\", \"pattern\": \"^[A-Z]+$\"},\n"
        "        {\"type\": \"pattern\"},\n"
        "        {\"type\": \"min_length\", \"value\": 5}\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"card1\",\n"
        "      \"type\": \"ui_card_base\",\n"
        "      \"props\": {\n"
        "        \"title\": \"Card Title\",\n"
        "        \"custom_card_prop\": \"val\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"chk1\",\n"
        "      \"type\": \"ui_checkbox_base\",\n"
        "      \"props\": {\n"
        "        \"checked\": \"true\"\n"
        "      },\n"
        "      \"bindings\": {\n"
        "        \"bind_visibility\": \"app.visible\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"lbl1\",\n"
        "      \"type\": \"ui_label_base\",\n"
        "      \"props\": {\n"
        "        \"text\": \"Label text\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"sld1\",\n"
        "      \"type\": \"ui_slider_base\",\n"
        "      \"props\": {\n"
        "        \"min\": \"0\"\n"
        "      },\n"
        "      \"validators\": [\n"
        "        {\"type\": \"pattern\"}\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"type\": \"row\"\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"col1\",\n"
        "      \"type\": \"column\"\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"grid1\",\n"
        "      \"type\": \"grid\"\n"
        "    }\n"
        "  ]\n"
        "}";
    struct ui_runtime_node *comp_node = NULL;
    size_t sz;

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
      (void)rc;
    }
    for (sz = 1; sz <= 600; sz += 1) {
      rc = ui_runtime_eject_tree_to_c_buffer(node, "mount_one", c_buf,
                                             sizeof(c_buf), h_buf, sz);
      (void)rc;
    }
    for (sz = 1; sz <= 10000; sz += 1) {
      rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "mount_all", c_buf, sz,
                                             h_buf, sizeof(h_buf));
      (void)rc;
    }
    for (sz = 1; sz <= 2000; sz += 1) {
      rc = ui_runtime_eject_tree_to_c_buffer(comp_node, "mount_all", c_buf,
                                             sizeof(c_buf), h_buf, sz);
      (void)rc;
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
      extern int g_malloc_fail_countdown;

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

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_runtime_eject passed\n");
  return 0;
}
