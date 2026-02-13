#include "cmpc/cmp_log.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestSinkState {
  int calls;
  int fail;
  CMPLogLevel last_level;
  cmp_usize last_length;
  char last_tag[32];
  char last_message[64];
} TestSinkState;

static int CMP_CALL test_sink_write(void *ctx, CMPLogLevel level,
                                    const char *tag, const char *message,
                                    cmp_usize length) {
  TestSinkState *state;
  cmp_usize copy_len;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestSinkState *)ctx;
  state->calls += 1;
  state->last_level = level;
  state->last_length = length;

  if (tag == NULL) {
    state->last_tag[0] = '\0';
  } else {
    size_t tag_len = strlen(tag);
    if (tag_len >= sizeof(state->last_tag)) {
      tag_len = sizeof(state->last_tag) - 1;
    }
    memcpy(state->last_tag, tag, tag_len);
    state->last_tag[tag_len] = '\0';
  }

  if (length == 0 || message == NULL) {
    state->last_message[0] = '\0';
  } else {
    copy_len = length;
    if (copy_len >= (cmp_usize)sizeof(state->last_message)) {
      copy_len = sizeof(state->last_message) - 1;
    }
    memcpy(state->last_message, message, (size_t)copy_len);
    state->last_message[copy_len] = '\0';
  }

  if (state->fail) {
    return CMP_ERR_IO;
  }

  return CMP_OK;
}

int main(void) {
  CMPAllocator bad_alloc;
  CMPAllocator default_alloc;
  CMPLogSink sink;
  CMPLogSink current;
  TestSinkState state;
  cmp_usize max_size;

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  memset(&state, 0, sizeof(state));

  CMP_TEST_EXPECT(cmp_log_test_mutex_lock(), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_log_test_mutex_unlock(), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_log_test_mutex_shutdown(), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_log_test_cstrlen(NULL, &max_size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_log_test_cstrlen("x", NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_log_shutdown(), CMP_ERR_STATE);
  sink.ctx = &state;
  sink.write = test_sink_write;
  CMP_TEST_EXPECT(cmp_log_set_sink(&sink), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_log_get_sink(&current), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_log_write(CMP_LOG_LEVEL_INFO, "core", "msg"),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_log_write(CMP_LOG_LEVEL_INFO, "core", NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_log_init(&bad_alloc), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_TRUE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));
  CMP_TEST_EXPECT(cmp_log_init(NULL), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));

#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_log_init(NULL), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));
#endif

  CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
  CMP_TEST_OK(cmp_log_init(&default_alloc));
  CMP_TEST_EXPECT(cmp_log_init(&default_alloc), CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_log_set_sink(NULL), CMP_ERR_INVALID_ARGUMENT);
  sink.ctx = NULL;
  sink.write = NULL;
  CMP_TEST_EXPECT(cmp_log_set_sink(&sink), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_TRUE, CMP_FALSE,
                                              CMP_FALSE));
  sink.ctx = &state;
  sink.write = test_sink_write;
  CMP_TEST_EXPECT(cmp_log_set_sink(&sink), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));

  CMP_TEST_EXPECT(cmp_log_get_sink(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_log_get_sink(&current));
  CMP_TEST_ASSERT(current.write != NULL);

  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "core", NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_log_write(CMP_LOG_LEVEL_TRACE, "core", "trace"));
  CMP_TEST_OK(cmp_log_write(CMP_LOG_LEVEL_INFO, "core", "hello"));
  CMP_TEST_OK(cmp_log_write(CMP_LOG_LEVEL_DEBUG, "", "empty-tag"));
  CMP_TEST_OK(cmp_log_write(CMP_LOG_LEVEL_WARN, "core", "warn"));
  CMP_TEST_OK(cmp_log_write(CMP_LOG_LEVEL_ERROR, "core", "error"));
  CMP_TEST_OK(cmp_log_write(CMP_LOG_LEVEL_FATAL, "core", "fatal"));
  CMP_TEST_OK(cmp_log_write_n(CMP_LOG_LEVEL_DEBUG, "", NULL, 0));
  CMP_TEST_OK(cmp_log_write_n(99, "core", "x", 1));

  CMP_TEST_OK(cmp_log_test_set_io_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "core", "x", 1),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_log_test_set_io_fail(CMP_FALSE));

  CMP_TEST_EXPECT(cmp_log_test_set_cstr_limit(0), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_log_test_set_cstr_limit(4));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "toolong", "x", 1),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_log_test_set_cstr_limit(1));
  CMP_TEST_EXPECT(cmp_log_write(CMP_LOG_LEVEL_INFO, "core", "ab"),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_TRACE, NULL, "x", 1),
                  CMP_ERR_OVERFLOW);
  max_size = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_OK(cmp_log_test_set_cstr_limit(max_size));

  CMP_TEST_OK(cmp_log_test_set_io_fail_on_call(1));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "tag", "msg", 3),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_log_test_set_io_fail_on_call(2));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "tag", "msg", 3),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_log_test_set_io_fail_on_call(3));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "tag", "msg", 3),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_log_test_set_io_fail_on_call(4));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "tag", "msg", 3),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_log_test_set_io_fail_on_call(5));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "tag", "msg", 3),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_log_test_set_io_fail_on_call(6));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "tag", "msg", 3),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_log_test_set_io_fail_on_call(7));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "tag", "msg", 3),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_log_test_set_io_fail_on_call(0));

  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_TRUE, CMP_FALSE,
                                              CMP_FALSE));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "core", "x", 1),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));

  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_TRUE,
                                              CMP_FALSE));
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_INFO, "core", "x", 1),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));

  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_TRUE,
                                              CMP_FALSE));
  sink.ctx = &state;
  sink.write = test_sink_write;
  CMP_TEST_EXPECT(cmp_log_set_sink(&sink), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));

  sink.ctx = &state;
  sink.write = test_sink_write;
  CMP_TEST_OK(cmp_log_set_sink(&sink));

  CMP_TEST_OK(cmp_log_write_n(CMP_LOG_LEVEL_WARN, "tag", "msg", 3));
  CMP_TEST_ASSERT(state.calls == 1);
  CMP_TEST_ASSERT(state.last_level == CMP_LOG_LEVEL_WARN);
  CMP_TEST_ASSERT(state.last_length == 3);
  CMP_TEST_ASSERT(strcmp(state.last_tag, "tag") == 0);
  CMP_TEST_ASSERT(strcmp(state.last_message, "msg") == 0);

  state.fail = 1;
  CMP_TEST_EXPECT(cmp_log_write_n(CMP_LOG_LEVEL_ERROR, "tag", "msg", 3),
                  CMP_ERR_IO);
  state.fail = 0;

  CMP_TEST_OK(cmp_log_get_sink(&current));
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_TRUE, CMP_FALSE,
                                              CMP_FALSE));
  CMP_TEST_EXPECT(cmp_log_get_sink(&current), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));

  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_TRUE,
                                              CMP_FALSE));
  CMP_TEST_EXPECT(cmp_log_get_sink(&current), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));

  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_TRUE));
  CMP_TEST_EXPECT(cmp_log_shutdown(), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                              CMP_FALSE));

  CMP_TEST_OK(cmp_log_shutdown());
  CMP_TEST_EXPECT(cmp_log_shutdown(), CMP_ERR_STATE);

  return 0;
}
