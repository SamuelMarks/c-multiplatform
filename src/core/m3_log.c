#include "m3/m3_log.h"

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#define M3_LOG_USE_WIN32 1
#include <windows.h>
#else
#define M3_LOG_USE_PTHREAD 1
#include <pthread.h>
#endif

typedef struct M3LogState {
  M3Bool initialized;
  M3LogSink sink;
  M3Allocator allocator;
} M3LogState;

static M3LogState g_log_state = {
    M3_FALSE, {NULL, NULL}, {NULL, NULL, NULL, NULL}};

#if defined(M3_LOG_USE_WIN32)
static CRITICAL_SECTION g_log_mutex;
static M3Bool g_log_mutex_initialized = M3_FALSE;
#else
static pthread_mutex_t g_log_mutex;
static M3Bool g_log_mutex_initialized = M3_FALSE;
#endif

#ifdef M3_TESTING
static M3Bool g_log_force_mutex_init_fail = M3_FALSE;
static M3Bool g_log_force_mutex_lock_fail = M3_FALSE;
static M3Bool g_log_force_mutex_unlock_fail = M3_FALSE;
static M3Bool g_log_force_mutex_shutdown_fail = M3_FALSE;
static M3Bool g_log_force_io_fail = M3_FALSE;
static m3_usize g_log_cstr_limit_override = 0;
static m3_usize g_log_force_io_fail_on_call = 0;
static m3_usize g_log_io_call_count = 0;
#endif

static m3_usize m3_usize_max_value(void) { return (m3_usize) ~(m3_usize)0; }

static m3_usize m3_log_cstr_limit(void) {
#ifdef M3_TESTING
  if (g_log_cstr_limit_override != 0) {
    return g_log_cstr_limit_override;
  }
#endif
  return m3_usize_max_value();
}

static int m3_log_cstrlen(const char *cstr, m3_usize *out_len) {
  m3_usize max_len;
  m3_usize length;

  if (cstr == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_len = m3_log_cstr_limit();
  length = 0;

  while (cstr[length] != '\0') {
    if (length == max_len) {
      return M3_ERR_OVERFLOW;
    }
    length += 1;
  }

  *out_len = length;
  return M3_OK;
}

static int m3_log_mutex_init(void) {
  int result;
#if defined(M3_LOG_USE_WIN32)
#ifdef M3_TESTING
  if (g_log_force_mutex_init_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif
  M3_UNUSED(result);
  InitializeCriticalSection(&g_log_mutex);
  g_log_mutex_initialized = M3_TRUE;
  return M3_OK;
#else
#ifdef M3_TESTING
  if (g_log_force_mutex_init_fail) {
    result = 1;
  } else {
    result = pthread_mutex_init(&g_log_mutex, NULL);
  }
#else
  result = pthread_mutex_init(&g_log_mutex, NULL);
#endif
  if (result != 0) {
    return M3_ERR_UNKNOWN;
  }
  g_log_mutex_initialized = M3_TRUE;
  return M3_OK;
#endif
}

static int m3_log_mutex_lock(void) {
  int result;

  if (!g_log_mutex_initialized) {
    return M3_ERR_STATE;
  }
#if defined(M3_LOG_USE_WIN32)
#ifdef M3_TESTING
  if (g_log_force_mutex_lock_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif
  M3_UNUSED(result);
  EnterCriticalSection(&g_log_mutex);
  return M3_OK;
#else
#ifdef M3_TESTING
  if (g_log_force_mutex_lock_fail) {
    result = 1;
  } else {
    result = pthread_mutex_lock(&g_log_mutex);
  }
#else
  result = pthread_mutex_lock(&g_log_mutex);
#endif
  if (result != 0) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
#endif
}

static int m3_log_mutex_unlock(void) {
  int result;

  if (!g_log_mutex_initialized) {
    return M3_ERR_STATE;
  }
#if defined(M3_LOG_USE_WIN32)
  M3_UNUSED(result);
  LeaveCriticalSection(&g_log_mutex);
#ifdef M3_TESTING
  if (g_log_force_mutex_unlock_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif
  return M3_OK;
#else
  result = pthread_mutex_unlock(&g_log_mutex);
#ifdef M3_TESTING
  if (g_log_force_mutex_unlock_fail) {
    result = 1;
  }
#endif
  if (result != 0) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
#endif
}

static int m3_log_mutex_shutdown(void) {
  int result;

  if (!g_log_mutex_initialized) {
    return M3_ERR_STATE;
  }
#if defined(M3_LOG_USE_WIN32)
#ifdef M3_TESTING
  if (g_log_force_mutex_shutdown_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif
  M3_UNUSED(result);
  DeleteCriticalSection(&g_log_mutex);
  g_log_mutex_initialized = M3_FALSE;
  return M3_OK;
#else
#ifdef M3_TESTING
  if (g_log_force_mutex_shutdown_fail) {
    result = 1;
  } else {
    result = pthread_mutex_destroy(&g_log_mutex);
  }
#else
  result = pthread_mutex_destroy(&g_log_mutex);
#endif
  if (result != 0) {
    return M3_ERR_UNKNOWN;
  }
  g_log_mutex_initialized = M3_FALSE;
  return M3_OK;
#endif
}

static const char *m3_log_level_name(M3LogLevel level) {
  switch (level) {
  case M3_LOG_LEVEL_TRACE:
    return "TRACE";
  case M3_LOG_LEVEL_DEBUG:
    return "DEBUG";
  case M3_LOG_LEVEL_INFO:
    return "INFO";
  case M3_LOG_LEVEL_WARN:
    return "WARN";
  case M3_LOG_LEVEL_ERROR:
    return "ERROR";
  case M3_LOG_LEVEL_FATAL:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}

static int m3_log_write_bytes(FILE *stream, const char *data, m3_usize length) {
  size_t written;

  if (length == 0) {
    return M3_OK;
  }
#ifdef M3_TESTING
  if (g_log_force_io_fail_on_call != 0) {
    g_log_io_call_count += 1;
    if (g_log_io_call_count == g_log_force_io_fail_on_call) {
      written = (size_t)(length - 1);
    } else {
      written = fwrite(data, 1, (size_t)length, stream);
    }
  } else if (g_log_force_io_fail) {
    written = (size_t)(length - 1);
  } else {
    written = fwrite(data, 1, (size_t)length, stream);
  }
#else
  written = fwrite(data, 1, (size_t)length, stream);
#endif
  if (written != (size_t)length) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int M3_CALL m3_log_default_sink(void *ctx, M3LogLevel level,
                                       const char *tag, const char *message,
                                       m3_usize length) {
  FILE *stream;
  const char *level_name;
  m3_usize level_len;
  m3_usize tag_len;
  int rc;

  stream = (FILE *)ctx;
  if (stream == NULL) {
    stream = stderr;
  }

  level_name = m3_log_level_name(level);
  rc = m3_log_cstrlen(level_name, &level_len);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_log_write_bytes(stream, "[", 1);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_log_write_bytes(stream, level_name, level_len);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_log_write_bytes(stream, "] ", 2);
  if (rc != M3_OK) {
    return rc;
  }

  if (tag != NULL && tag[0] != '\0') {
    rc = m3_log_cstrlen(tag, &tag_len);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_log_write_bytes(stream, tag, tag_len);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_log_write_bytes(stream, ": ", 2);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_log_write_bytes(stream, message, length);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_log_write_bytes(stream, "\n", 1);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_log_init(const M3Allocator *allocator) {
  M3Allocator default_alloc;
  int rc;

  if (g_log_state.initialized) {
    return M3_ERR_STATE;
  }

  if (allocator == NULL) {
    rc = m3_get_default_allocator(&default_alloc);
    if (rc != M3_OK) {
      return rc;
    }
    allocator = &default_alloc;
  }

  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_log_mutex_init();
  if (rc != M3_OK) {
    return rc;
  }

  g_log_state.allocator = *allocator;
  g_log_state.sink.ctx = NULL;
  g_log_state.sink.write = m3_log_default_sink;
  g_log_state.initialized = M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_log_shutdown(void) {
  int rc;

  if (!g_log_state.initialized) {
    return M3_ERR_STATE;
  }

  rc = m3_log_mutex_shutdown();
  if (rc != M3_OK) {
    return rc;
  }

  g_log_state.initialized = M3_FALSE;
  g_log_state.sink.ctx = NULL;
  g_log_state.sink.write = NULL;
  g_log_state.allocator.ctx = NULL;
  g_log_state.allocator.alloc = NULL;
  g_log_state.allocator.realloc = NULL;
  g_log_state.allocator.free = NULL;
  return M3_OK;
}

int M3_CALL m3_log_set_sink(const M3LogSink *sink) {
  int rc;

  if (!g_log_state.initialized) {
    return M3_ERR_STATE;
  }
  if (sink == NULL || sink->write == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_log_mutex_lock();
  if (rc != M3_OK) {
    return rc;
  }

  g_log_state.sink = *sink;

  rc = m3_log_mutex_unlock();
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_log_get_sink(M3LogSink *out_sink) {
  int rc;

  if (!g_log_state.initialized) {
    return M3_ERR_STATE;
  }
  if (out_sink == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_log_mutex_lock();
  if (rc != M3_OK) {
    return rc;
  }

  *out_sink = g_log_state.sink;

  rc = m3_log_mutex_unlock();
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_log_write(M3LogLevel level, const char *tag,
                         const char *message) {
  m3_usize length;
  int rc;

  if (message == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_log_cstrlen(message, &length);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_log_write_n(level, tag, message, length);
}

int M3_CALL m3_log_write_n(M3LogLevel level, const char *tag,
                           const char *message, m3_usize length) {
  int rc;
  int write_rc;

  if (!g_log_state.initialized) {
    return M3_ERR_STATE;
  }
  if (message == NULL && length != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_log_mutex_lock();
  if (rc != M3_OK) {
    return rc;
  }

  write_rc =
      g_log_state.sink.write(g_log_state.sink.ctx, level, tag, message, length);

  rc = m3_log_mutex_unlock();
  if (rc != M3_OK) {
    return rc;
  }

  return write_rc;
}

#ifdef M3_TESTING
int M3_CALL m3_log_test_set_mutex_failures(M3Bool init_fail, M3Bool lock_fail,
                                           M3Bool unlock_fail,
                                           M3Bool shutdown_fail) {
  g_log_force_mutex_init_fail = init_fail;
  g_log_force_mutex_lock_fail = lock_fail;
  g_log_force_mutex_unlock_fail = unlock_fail;
  g_log_force_mutex_shutdown_fail = shutdown_fail;
  return M3_OK;
}

int M3_CALL m3_log_test_set_io_fail(M3Bool write_fail) {
  g_log_force_io_fail = write_fail;
  return M3_OK;
}

int M3_CALL m3_log_test_set_cstr_limit(m3_usize max_len) {
  if (max_len == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  g_log_cstr_limit_override = max_len;
  return M3_OK;
}

int M3_CALL m3_log_test_set_io_fail_on_call(m3_usize call_index) {
  g_log_force_io_fail_on_call = call_index;
  g_log_io_call_count = 0;
  return M3_OK;
}

int M3_CALL m3_log_test_cstrlen(const char *cstr, m3_usize *out_len) {
  return m3_log_cstrlen(cstr, out_len);
}

int M3_CALL m3_log_test_mutex_lock(void) { return m3_log_mutex_lock(); }

int M3_CALL m3_log_test_mutex_unlock(void) { return m3_log_mutex_unlock(); }

int M3_CALL m3_log_test_mutex_shutdown(void) { return m3_log_mutex_shutdown(); }
#endif
