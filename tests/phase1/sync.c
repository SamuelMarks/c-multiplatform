#include "cmpc/cmp_core.h"
#include "cmpc/cmp_sync.h"
#include "test_utils.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#if defined(_MSC_VER) && !defined(_X86_) && !defined(_AMD64_) &&               \
    !defined(_ARM_) && !defined(_ARM64_)
#if defined(_M_AMD64)
#define _AMD64_
#elif defined(_M_IX86)
#define _X86_
#elif defined(_M_ARM64)
#define _ARM64_
#elif defined(_M_ARM)
#define _ARM_
#endif
#endif
/* clang-format off */
#include <windef.h>
#include <winbase.h>
/* clang-format on */
#else
#include <pthread.h>
#include <unistd.h>
#endif

static int CMP_CALL fail_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  (void)ctx;
  (void)size;
  *out_ptr = NULL;
  return CMP_ERR_OUT_OF_MEMORY;
}

static int CMP_CALL fake_free(void *ctx, void *ptr) {
  (void)ctx;
  (void)ptr;
  return CMP_OK;
}

typedef struct {
  CMPCondVar *cond;
  CMPMutex *mutex;
} ThreadData;

#if defined(_WIN32)
static DWORD WINAPI signal_thread(LPVOID lpParam) {
  ThreadData *data = (ThreadData *)lpParam;
  Sleep(50);
  cmp_mutex_lock(data->mutex);
  cmp_condvar_signal(data->cond);
  cmp_mutex_unlock(data->mutex);
  return 0;
}
#else
static void *signal_thread(void *arg) {
  ThreadData *data = (ThreadData *)arg;
  usleep(50000);
  cmp_mutex_lock(data->mutex);
  cmp_condvar_signal(data->cond);
  cmp_mutex_unlock(data->mutex);
  return NULL;
}
#endif

int test_sync(void) {
  CMPAllocator alloc;
  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  CMPAllocator bad_alloc = alloc;
  bad_alloc.alloc = NULL;
  bad_alloc.free = NULL;
  CMPAllocator fail_allocator = {
      .ctx = NULL, .alloc = fail_alloc, .free = fake_free};

  CMPMutex *mutex = NULL;

  /* MUTEX */
  CMP_TEST_EXPECT(cmp_mutex_create(NULL, &mutex), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mutex_create(&bad_alloc, &mutex),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mutex_create(&alloc, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mutex_create(&fail_allocator, &mutex),
                  CMP_ERR_OUT_OF_MEMORY);

  CMP_TEST_OK(cmp_mutex_create(&alloc, &mutex));

  CMP_TEST_EXPECT(cmp_mutex_lock(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mutex_trylock(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mutex_unlock(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_mutex_lock(mutex));
  CMP_TEST_OK(cmp_mutex_unlock(mutex));

  CMP_TEST_OK(cmp_mutex_trylock(mutex));
  CMP_TEST_OK(cmp_mutex_unlock(mutex));

  /* SEMAPHORE */
  CMPSemaphore *sem = NULL;
  CMP_TEST_EXPECT(cmp_semaphore_create(NULL, 1, &sem),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_semaphore_create(&bad_alloc, 1, &sem),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_semaphore_create(&alloc, 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_semaphore_create(&fail_allocator, 1, &sem),
                  CMP_ERR_OUT_OF_MEMORY);

  CMP_TEST_OK(cmp_semaphore_create(&alloc, 1, &sem));

  CMP_TEST_EXPECT(cmp_semaphore_wait(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_semaphore_post(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_semaphore_wait(sem));
  CMP_TEST_OK(cmp_semaphore_post(sem));

  CMP_TEST_EXPECT(cmp_semaphore_destroy(NULL, sem), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_semaphore_destroy(&bad_alloc, sem),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_semaphore_destroy(&alloc, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_semaphore_destroy(&alloc, sem));

  /* CONDVAR */
  CMPCondVar *cond = NULL;
  CMP_TEST_EXPECT(cmp_condvar_create(NULL, &cond), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_condvar_create(&bad_alloc, &cond),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_condvar_create(&alloc, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_condvar_create(&fail_allocator, &cond),
                  CMP_ERR_OUT_OF_MEMORY);

  CMP_TEST_OK(cmp_condvar_create(&alloc, &cond));

  CMP_TEST_EXPECT(cmp_condvar_wait(NULL, mutex), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_condvar_wait(cond, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_condvar_signal(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_condvar_broadcast(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_condvar_signal(cond));
  CMP_TEST_OK(cmp_condvar_broadcast(cond));

  /* Test condvar wait with a thread */
  ThreadData tdata = {.cond = cond, .mutex = mutex};
#if defined(_WIN32)
  HANDLE hThread = CreateThread(NULL, 0, signal_thread, &tdata, 0, NULL);
#else
  pthread_t thread;
  pthread_create(&thread, NULL, signal_thread, &tdata);
#endif

  CMP_TEST_OK(cmp_mutex_lock(mutex));
  CMP_TEST_OK(cmp_condvar_wait(cond, mutex));
  CMP_TEST_OK(cmp_mutex_unlock(mutex));

#if defined(_WIN32)
  WaitForSingleObject(hThread, INFINITE);
  CloseHandle(hThread);
#else
  pthread_join(thread, NULL);
#endif

  CMP_TEST_EXPECT(cmp_condvar_destroy(NULL, cond), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_condvar_destroy(&bad_alloc, cond),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_condvar_destroy(&alloc, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_condvar_destroy(&alloc, cond));
  CMP_TEST_OK(cmp_mutex_destroy(&alloc, mutex));

  return 0;
}

int main(void) {
  if (test_sync() != 0)
    return 1;
  return 0;
}
