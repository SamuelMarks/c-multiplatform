#include "cmpc/cmp_log.h"

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#define CMP_LOG_USE_WIN32 1
#include <windows.h>
#else
#define CMP_LOG_USE_PTHREAD 1
#include <pthread.h>
#endif

typedef struct CMPLogState {
  CMPBool initialized;
  CMPLogSink sink;
  CMPAllocator allocator;
} CMPLogState;

static CMPLogState g_log_state = {
    CMP_FALSE, {NULL, NULL}, {NULL, NULL, NULL, NULL}};

#if defined(CMP_LOG_USE_WIN32)
static CRITICAL_SECTION g_log_mutex;
static CMPBool g_log_mutex_initialized = CMP_FALSE;
#else
static pthread_mutex_t g_log_mutex;
static CMPBool g_log_mutex_initialized = CMP_FALSE;
#endif

#ifdef CMP_TESTING
static CMPBool g_log_force_mutex_init_fail = CMP_FALSE;
static CMPBool g_log_force_mutex_lock_fail = CMP_FALSE;
static CMPBool g_log_force_mutex_unlock_fail = CMP_FALSE;
static CMPBool g_log_force_mutex_shutdown_fail = CMP_FALSE;
static CMPBool g_log_force_io_fail = CMP_FALSE;
static cmp_usize g_log_cstr_limit_override = 0;
static cmp_usize g_log_force_io_fail_on_call = 0;
static cmp_usize g_log_io_call_count = 0;
#endif

static cmp_usize cmp_usize_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static cmp_usize cmp_log_cstr_limit(void) {
#ifdef CMP_TESTING
  if (g_log_cstr_limit_override != 0) {
    return g_log_cstr_limit_override;
  }
#endif
  return cmp_usize_max_value();
}

static int cmp_log_cstrlen(const char *cstr, cmp_usize *out_len) {
  cmp_usize max_len;
  cmp_usize length;

  if (cstr == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_len = cmp_log_cstr_limit();
  length = 0;

  while (cstr[length] != '\0') {
    if (length == max_len) {
      return CMP_ERR_OVERFLOW;
    }
    length += 1;
  }

  *out_len = length;
  return CMP_OK;
}

static int cmp_log_mutex_init(void) {
  int result;
#if defined(CMP_LOG_USE_WIN32)
#ifdef CMP_TESTING
  if (g_log_force_mutex_init_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  CMP_UNUSED(result);
  InitializeCriticalSection(&g_log_mutex);
  g_log_mutex_initialized = CMP_TRUE;
  return CMP_OK;
#else
#ifdef CMP_TESTING
  if (g_log_force_mutex_init_fail) {
    result = 1;
  } else {
    result = pthread_mutex_init(&g_log_mutex, NULL);
  }
#else
  result = pthread_mutex_init(&g_log_mutex, NULL);
#endif
  if (result != 0) {
    return CMP_ERR_UNKNOWN;
  }
  g_log_mutex_initialized = CMP_TRUE;
  return CMP_OK;
#endif
}

static int cmp_log_mutex_lock(void) {
  int result;

  if (!g_log_mutex_initialized) {
    return CMP_ERR_STATE;
  }
#if defined(CMP_LOG_USE_WIN32)
#ifdef CMP_TESTING
  if (g_log_force_mutex_lock_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  CMP_UNUSED(result);
  EnterCriticalSection(&g_log_mutex);
  return CMP_OK;
#else
#ifdef CMP_TESTING
  if (g_log_force_mutex_lock_fail) {
    result = 1;
  } else {
    result = pthread_mutex_lock(&g_log_mutex);
  }
#else
  result = pthread_mutex_lock(&g_log_mutex);
#endif
  if (result != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

static int cmp_log_mutex_unlock(void) {
  int result;

  if (!g_log_mutex_initialized) {
    return CMP_ERR_STATE;
  }
#if defined(CMP_LOG_USE_WIN32)
  CMP_UNUSED(result);
  LeaveCriticalSection(&g_log_mutex);
#ifdef CMP_TESTING
  if (g_log_force_mutex_unlock_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  return CMP_OK;
#else
  result = pthread_mutex_unlock(&g_log_mutex);
#ifdef CMP_TESTING
  if (g_log_force_mutex_unlock_fail) {
    result = 1;
  }
#endif
  if (result != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

static int cmp_log_mutex_shutdown(void) {
  int result;

  if (!g_log_mutex_initialized) {
    return CMP_ERR_STATE;
  }
#if defined(CMP_LOG_USE_WIN32)
#ifdef CMP_TESTING
  if (g_log_force_mutex_shutdown_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  CMP_UNUSED(result);
  DeleteCriticalSection(&g_log_mutex);
  g_log_mutex_initialized = CMP_FALSE;
  return CMP_OK;
#else
#ifdef CMP_TESTING
  if (g_log_force_mutex_shutdown_fail) {
    result = 1;
  } else {
    result = pthread_mutex_destroy(&g_log_mutex);
  }
#else
  result = pthread_mutex_destroy(&g_log_mutex);
#endif
  if (result != 0) {
    return CMP_ERR_UNKNOWN;
  }
  g_log_mutex_initialized = CMP_FALSE;
  return CMP_OK;
#endif
}

static const char *cmp_log_level_name(CMPLogLevel level) {
  switch (level) {
  case CMP_LOG_LEVEL_TRACE:
    return "TRACE";
  case CMP_LOG_LEVEL_DEBUG:
    return "DEBUG";
  case CMP_LOG_LEVEL_INFO:
    return "INFO";
  case CMP_LOG_LEVEL_WARN:
    return "WARN";
  case CMP_LOG_LEVEL_ERROR:
    return "ERROR";
  case CMP_LOG_LEVEL_FATAL:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}

static int cmp_log_write_bytes(FILE *stream, const char *data, cmp_usize length) {
  size_t written;

  if (length == 0) {
    return CMP_OK;
  }
#ifdef CMP_TESTING
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
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int CMP_CALL cmp_log_default_sink(void *ctx, CMPLogLevel level,
                                       const char *tag, const char *message,
                                       cmp_usize length) {
  FILE *stream;
  const char *level_name;
  cmp_usize level_len;
  cmp_usize tag_len;
  int rc;

  stream = (FILE *)ctx;
  if (stream == NULL) {
    stream = stderr;
  }

  level_name = cmp_log_level_name(level);
  rc = cmp_log_cstrlen(level_name, &level_len);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_log_write_bytes(stream, "[", 1);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_log_write_bytes(stream, level_name, level_len);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_log_write_bytes(stream, "] ", 2);
  if (rc != CMP_OK) {
    return rc;
  }

  if (tag != NULL && tag[0] != '\0') {
    rc = cmp_log_cstrlen(tag, &tag_len);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_log_write_bytes(stream, tag, tag_len);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_log_write_bytes(stream, ": ", 2);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = cmp_log_write_bytes(stream, message, length);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_log_write_bytes(stream, "\n", 1);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_log_init(const CMPAllocator *allocator) {
  CMPAllocator default_alloc;
  int rc;

  if (g_log_state.initialized) {
    return CMP_ERR_STATE;
  }

  if (allocator == NULL) {
    rc = cmp_get_default_allocator(&default_alloc);
    if (rc != CMP_OK) {
      return rc;
    }
    allocator = &default_alloc;
  }

  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_log_mutex_init();
  if (rc != CMP_OK) {
    return rc;
  }

  g_log_state.allocator = *allocator;
  g_log_state.sink.ctx = NULL;
  g_log_state.sink.write = cmp_log_default_sink;
  g_log_state.initialized = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_log_shutdown(void) {
  int rc;

  if (!g_log_state.initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_log_mutex_shutdown();
  if (rc != CMP_OK) {
    return rc;
  }

  g_log_state.initialized = CMP_FALSE;
  g_log_state.sink.ctx = NULL;
  g_log_state.sink.write = NULL;
  g_log_state.allocator.ctx = NULL;
  g_log_state.allocator.alloc = NULL;
  g_log_state.allocator.realloc = NULL;
  g_log_state.allocator.free = NULL;
  return CMP_OK;
}

int CMP_CALL cmp_log_set_sink(const CMPLogSink *sink) {
  int rc;

  if (!g_log_state.initialized) {
    return CMP_ERR_STATE;
  }
  if (sink == NULL || sink->write == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_log_mutex_lock();
  if (rc != CMP_OK) {
    return rc;
  }

  g_log_state.sink = *sink;

  rc = cmp_log_mutex_unlock();
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_log_get_sink(CMPLogSink *out_sink) {
  int rc;

  if (!g_log_state.initialized) {
    return CMP_ERR_STATE;
  }
  if (out_sink == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_log_mutex_lock();
  if (rc != CMP_OK) {
    return rc;
  }

  *out_sink = g_log_state.sink;

  rc = cmp_log_mutex_unlock();
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_log_write(CMPLogLevel level, const char *tag,
                         const char *message) {
  cmp_usize length;
  int rc;

  if (message == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_log_cstrlen(message, &length);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_log_write_n(level, tag, message, length);
}

int CMP_CALL cmp_log_write_n(CMPLogLevel level, const char *tag,
                           const char *message, cmp_usize length) {
  int rc;
  int write_rc;

  if (!g_log_state.initialized) {
    return CMP_ERR_STATE;
  }
  if (message == NULL && length != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_log_mutex_lock();
  if (rc != CMP_OK) {
    return rc;
  }

  write_rc =
      g_log_state.sink.write(g_log_state.sink.ctx, level, tag, message, length);

  rc = cmp_log_mutex_unlock();
  if (rc != CMP_OK) {
    return rc;
  }

  return write_rc;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_log_test_set_mutex_failures(CMPBool init_fail, CMPBool lock_fail,
                                           CMPBool unlock_fail,
                                           CMPBool shutdown_fail) {
  g_log_force_mutex_init_fail = init_fail;
  g_log_force_mutex_lock_fail = lock_fail;
  g_log_force_mutex_unlock_fail = unlock_fail;
  g_log_force_mutex_shutdown_fail = shutdown_fail;
  return CMP_OK;
}

int CMP_CALL cmp_log_test_set_io_fail(CMPBool write_fail) {
  g_log_force_io_fail = write_fail;
  return CMP_OK;
}

int CMP_CALL cmp_log_test_set_cstr_limit(cmp_usize max_len) {
  if (max_len == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  g_log_cstr_limit_override = max_len;
  return CMP_OK;
}

int CMP_CALL cmp_log_test_set_io_fail_on_call(cmp_usize call_index) {
  g_log_force_io_fail_on_call = call_index;
  g_log_io_call_count = 0;
  return CMP_OK;
}

int CMP_CALL cmp_log_test_cstrlen(const char *cstr, cmp_usize *out_len) {
  return cmp_log_cstrlen(cstr, out_len);
}

int CMP_CALL cmp_log_test_mutex_lock(void) { return cmp_log_mutex_lock(); }

int CMP_CALL cmp_log_test_mutex_unlock(void) { return cmp_log_mutex_unlock(); }

int CMP_CALL cmp_log_test_mutex_shutdown(void) { return cmp_log_mutex_shutdown(); }
#endif
