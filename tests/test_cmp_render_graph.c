/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

static void dummy_pass_execute(cmp_command_buffer_t *cb, void *user_data) {
  int *order = (int *)user_data;
  (void)cb;
  (*order)++;
}

TEST test_render_graph(void) {
  cmp_gpu_t *gpu = NULL;
  cmp_command_buffer_t *cb = NULL;
  cmp_render_graph_t *graph = NULL;
  cmp_render_pass_config_t p1, p2, p3;
  int order1 = 0, order2 = 0, order3 = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_create(CMP_BACKEND_CPU_SOFTWARE, &gpu));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_create(gpu, 0, &cb));
  ASSERT_EQ(CMP_SUCCESS, cmp_render_graph_create(&graph));

  memset(&p1, 0, sizeof(p1));
  p1.id = 1;
  p1.execute_cb = dummy_pass_execute;
  p1.user_data = &order1;

  memset(&p2, 0, sizeof(p2));
  p2.id = 2;
  p2.dependency_count = 1;
  p2.dependencies[0] = 1;
  p2.execute_cb = dummy_pass_execute;
  p2.user_data = &order2;

  memset(&p3, 0, sizeof(p3));
  p3.id = 3;
  p3.dependency_count = 1;
  p3.dependencies[0] = 2;
  p3.execute_cb = dummy_pass_execute;
  p3.user_data = &order3;

  /* Add them in reverse order to ensure it sorts correctly */
  ASSERT_EQ(CMP_SUCCESS, cmp_render_graph_add_pass(graph, &p3));
  ASSERT_EQ(CMP_SUCCESS, cmp_render_graph_add_pass(graph, &p2));
  ASSERT_EQ(CMP_SUCCESS, cmp_render_graph_add_pass(graph, &p1));

  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_begin(cb));
  ASSERT_EQ(CMP_SUCCESS, cmp_render_graph_execute(graph, cb));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_end(cb));

  ASSERT_EQ(1, order1);
  ASSERT_EQ(1, order2);
  ASSERT_EQ(1, order3);

  ASSERT_EQ(CMP_SUCCESS, cmp_render_graph_destroy(graph));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_destroy(cb));
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_destroy(gpu));
  PASS();
}

SUITE(cmp_render_graph_suite) { RUN_TEST(test_render_graph); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_render_graph_suite);
  GREATEST_MAIN_END();
}
