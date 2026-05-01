/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_command_palette_lifecycle(void) {
  cmp_command_palette_t *palette = NULL;
  int res;

  res = cmp_command_palette_create(&palette);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, palette);

  res = cmp_command_palette_destroy(palette);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_command_palette_null_args(void) {
  cmp_command_palette_t *palette = NULL;
  cmp_command_item_t *results[10];
  size_t out_count;
  int res;

  res = cmp_command_palette_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_create(&palette);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_command_palette_add_item(NULL, "id", "Text", "Subtext");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_add_item(palette, NULL, "Text", "Subtext");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_add_item(palette, "id", NULL, "Subtext");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_search(NULL, "query", results, 10, &out_count);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_search(palette, NULL, results, 10, &out_count);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_search(palette, "query", NULL, 10, &out_count);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_search(palette, "query", results, 10, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_command_palette_destroy(palette);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_command_palette_search(void) {
  cmp_command_palette_t *palette = NULL;
  cmp_command_item_t *results[10];
  size_t out_count;
  int res;

  res = cmp_command_palette_create(&palette);
  ASSERT_EQ(CMP_SUCCESS, res);

  res =
      cmp_command_palette_add_item(palette, "file.save", "Save File", "Ctrl+S");
  ASSERT_EQ(CMP_SUCCESS, res);

  res =
      cmp_command_palette_add_item(palette, "file.open", "Open File", "Ctrl+O");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_command_palette_add_item(palette, "edit.copy", "Copy", "Ctrl+C");
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Test exact match */
  res =
      cmp_command_palette_search(palette, "Save File", results, 10, &out_count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, (int)out_count);
  ASSERT_STR_EQ("Save File", results[0]->display_text);

  /* Test fuzzy match */
  res = cmp_command_palette_search(palette, "sf", results, 10, &out_count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, (int)out_count);
  ASSERT_STR_EQ("Save File", results[0]->display_text);

  /* Test match by subtext */
  res = cmp_command_palette_search(palette, "Ctrl+O", results, 10, &out_count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, (int)out_count);
  ASSERT_STR_EQ("Open File", results[0]->display_text);

  /* Test multiple matches / no match */
  res = cmp_command_palette_search(palette, "File", results, 10, &out_count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2, (int)out_count);

  res = cmp_command_palette_search(palette, "xyz", results, 10, &out_count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, (int)out_count);

  res = cmp_command_palette_destroy(palette);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(command_palette_suite) {
  RUN_TEST(test_command_palette_lifecycle);
  RUN_TEST(test_command_palette_null_args);
  RUN_TEST(test_command_palette_search);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(command_palette_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
