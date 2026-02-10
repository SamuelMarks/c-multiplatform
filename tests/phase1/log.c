#include "test_utils.h"
#include "m3/m3_log.h"

#include <string.h>

typedef struct TestSinkState {
    int calls;
    int fail;
    M3LogLevel last_level;
    m3_usize last_length;
    char last_tag[32];
    char last_message[64];
} TestSinkState;

static int M3_CALL test_sink_write(void *ctx, M3LogLevel level, const char *tag, const char *message, m3_usize length)
{
    TestSinkState *state;
    m3_usize copy_len;

    if (ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestSinkState *)ctx;
    state->calls += 1;
    state->last_level = level;
    state->last_length = length;

    if (tag == NULL) {
        state->last_tag[0] = '\0';
    } else {
        strncpy(state->last_tag, tag, sizeof(state->last_tag) - 1);
        state->last_tag[sizeof(state->last_tag) - 1] = '\0';
    }

    if (length == 0 || message == NULL) {
        state->last_message[0] = '\0';
    } else {
        copy_len = length;
        if (copy_len >= (m3_usize)sizeof(state->last_message)) {
            copy_len = sizeof(state->last_message) - 1;
        }
        memcpy(state->last_message, message, (size_t)copy_len);
        state->last_message[copy_len] = '\0';
    }

    if (state->fail) {
        return M3_ERR_IO;
    }

    return M3_OK;
}

int main(void)
{
    M3Allocator bad_alloc;
    M3Allocator default_alloc;
    M3LogSink sink;
    M3LogSink current;
    TestSinkState state;
    m3_usize max_size;

    memset(&bad_alloc, 0, sizeof(bad_alloc));
    memset(&state, 0, sizeof(state));

    M3_TEST_EXPECT(m3_log_test_mutex_lock(), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_log_test_mutex_unlock(), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_log_test_mutex_shutdown(), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_log_test_cstrlen(NULL, &max_size), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_log_test_cstrlen("x", NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_log_shutdown(), M3_ERR_STATE);
    sink.ctx = &state;
    sink.write = test_sink_write;
    M3_TEST_EXPECT(m3_log_set_sink(&sink), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_log_get_sink(&current), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_log_write(M3_LOG_LEVEL_INFO, "core", "msg"), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_log_write(M3_LOG_LEVEL_INFO, "core", NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_log_init(&bad_alloc), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_TRUE, M3_FALSE, M3_FALSE, M3_FALSE));
    M3_TEST_EXPECT(m3_log_init(NULL), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));

#ifdef M3_TESTING
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
    M3_TEST_EXPECT(m3_log_init(NULL), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));
#endif

    M3_TEST_OK(m3_get_default_allocator(&default_alloc));
    M3_TEST_OK(m3_log_init(&default_alloc));
    M3_TEST_EXPECT(m3_log_init(&default_alloc), M3_ERR_STATE);

    M3_TEST_EXPECT(m3_log_set_sink(NULL), M3_ERR_INVALID_ARGUMENT);
    sink.ctx = NULL;
    sink.write = NULL;
    M3_TEST_EXPECT(m3_log_set_sink(&sink), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_TRUE, M3_FALSE, M3_FALSE));
    sink.ctx = &state;
    sink.write = test_sink_write;
    M3_TEST_EXPECT(m3_log_set_sink(&sink), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));

    M3_TEST_EXPECT(m3_log_get_sink(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_log_get_sink(&current));
    M3_TEST_ASSERT(current.write != NULL);

    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "core", NULL, 1), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_log_write(M3_LOG_LEVEL_TRACE, "core", "trace"));
    M3_TEST_OK(m3_log_write(M3_LOG_LEVEL_INFO, "core", "hello"));
    M3_TEST_OK(m3_log_write(M3_LOG_LEVEL_DEBUG, "", "empty-tag"));
    M3_TEST_OK(m3_log_write(M3_LOG_LEVEL_WARN, "core", "warn"));
    M3_TEST_OK(m3_log_write(M3_LOG_LEVEL_ERROR, "core", "error"));
    M3_TEST_OK(m3_log_write(M3_LOG_LEVEL_FATAL, "core", "fatal"));
    M3_TEST_OK(m3_log_write_n(M3_LOG_LEVEL_DEBUG, "", NULL, 0));
    M3_TEST_OK(m3_log_write_n(99, "core", "x", 1));

    M3_TEST_OK(m3_log_test_set_io_fail(M3_TRUE));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "core", "x", 1), M3_ERR_IO);
    M3_TEST_OK(m3_log_test_set_io_fail(M3_FALSE));

    M3_TEST_EXPECT(m3_log_test_set_cstr_limit(0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_log_test_set_cstr_limit(4));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "toolong", "x", 1), M3_ERR_OVERFLOW);
    M3_TEST_OK(m3_log_test_set_cstr_limit(1));
    M3_TEST_EXPECT(m3_log_write(M3_LOG_LEVEL_INFO, "core", "ab"), M3_ERR_OVERFLOW);
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_TRACE, NULL, "x", 1), M3_ERR_OVERFLOW);
    max_size = (m3_usize)~(m3_usize)0;
    M3_TEST_OK(m3_log_test_set_cstr_limit(max_size));

    M3_TEST_OK(m3_log_test_set_io_fail_on_call(1));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "tag", "msg", 3), M3_ERR_IO);
    M3_TEST_OK(m3_log_test_set_io_fail_on_call(2));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "tag", "msg", 3), M3_ERR_IO);
    M3_TEST_OK(m3_log_test_set_io_fail_on_call(3));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "tag", "msg", 3), M3_ERR_IO);
    M3_TEST_OK(m3_log_test_set_io_fail_on_call(4));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "tag", "msg", 3), M3_ERR_IO);
    M3_TEST_OK(m3_log_test_set_io_fail_on_call(5));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "tag", "msg", 3), M3_ERR_IO);
    M3_TEST_OK(m3_log_test_set_io_fail_on_call(6));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "tag", "msg", 3), M3_ERR_IO);
    M3_TEST_OK(m3_log_test_set_io_fail_on_call(7));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "tag", "msg", 3), M3_ERR_IO);
    M3_TEST_OK(m3_log_test_set_io_fail_on_call(0));

    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_TRUE, M3_FALSE, M3_FALSE));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "core", "x", 1), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));

    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_TRUE, M3_FALSE));
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_INFO, "core", "x", 1), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));

    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_TRUE, M3_FALSE));
    sink.ctx = &state;
    sink.write = test_sink_write;
    M3_TEST_EXPECT(m3_log_set_sink(&sink), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));

    sink.ctx = &state;
    sink.write = test_sink_write;
    M3_TEST_OK(m3_log_set_sink(&sink));

    M3_TEST_OK(m3_log_write_n(M3_LOG_LEVEL_WARN, "tag", "msg", 3));
    M3_TEST_ASSERT(state.calls == 1);
    M3_TEST_ASSERT(state.last_level == M3_LOG_LEVEL_WARN);
    M3_TEST_ASSERT(state.last_length == 3);
    M3_TEST_ASSERT(strcmp(state.last_tag, "tag") == 0);
    M3_TEST_ASSERT(strcmp(state.last_message, "msg") == 0);

    state.fail = 1;
    M3_TEST_EXPECT(m3_log_write_n(M3_LOG_LEVEL_ERROR, "tag", "msg", 3), M3_ERR_IO);
    state.fail = 0;

    M3_TEST_OK(m3_log_get_sink(&current));
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_TRUE, M3_FALSE, M3_FALSE));
    M3_TEST_EXPECT(m3_log_get_sink(&current), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));

    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_TRUE, M3_FALSE));
    M3_TEST_EXPECT(m3_log_get_sink(&current), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));

    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_TRUE));
    M3_TEST_EXPECT(m3_log_shutdown(), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));

    M3_TEST_OK(m3_log_shutdown());
    M3_TEST_EXPECT(m3_log_shutdown(), M3_ERR_STATE);

    return 0;
}
