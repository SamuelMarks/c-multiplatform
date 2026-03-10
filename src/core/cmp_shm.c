/* clang-format off */
#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#endif
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "cmpc/cmp_shm.h"

#include <stdlib.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 500
#endif
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#endif
/* clang-format on */

struct CMPSharedMemory {
  void *ptr;
  cmp_usize size;
#if defined(_WIN32)
  HANDLE handle;
#else
  int fd;
#endif
};

CMP_API int CMP_CALL cmp_shm_create(const char *name, cmp_usize size,
                                    CMPSharedMemory **out_shm) {
  CMPSharedMemory *shm;
  CMPAllocator alloc;
  int rc;

  if (name == NULL || size == 0 || out_shm == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_get_default_allocator(&alloc);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = alloc.alloc(alloc.ctx, sizeof(CMPSharedMemory), (void **)&shm);
  if (rc != CMP_OK || shm == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  shm->size = size;

#if defined(_WIN32)
  shm->handle =
      CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                         (DWORD)(size >> 32), (DWORD)(size & 0xFFFFFFFF), name);
  if (shm->handle == NULL) {
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_UNKNOWN;
  }

  shm->ptr = MapViewOfFile(shm->handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
  if (shm->ptr == NULL) {
    CloseHandle(shm->handle);
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_UNKNOWN;
  }
#else
  shm->fd = shm_open(name, O_CREAT | O_RDWR, 0666);
  if (shm->fd < 0) {
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_UNKNOWN;
  }

  if (ftruncate(shm->fd, (off_t)size) == -1) {
    close(shm->fd);
    shm_unlink(name);
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_UNKNOWN;
  }

  shm->ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fd, 0);
  if (shm->ptr == MAP_FAILED) {
    close(shm->fd);
    shm_unlink(name);
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_UNKNOWN;
  }
#endif

  *out_shm = shm;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_shm_open(const char *name, cmp_usize size,
                                  CMPSharedMemory **out_shm) {
  CMPSharedMemory *shm;
  CMPAllocator alloc;
  int rc;

  if (name == NULL || size == 0 || out_shm == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_get_default_allocator(&alloc);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = alloc.alloc(alloc.ctx, sizeof(CMPSharedMemory), (void **)&shm);
  if (rc != CMP_OK || shm == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  shm->size = size;

#if defined(_WIN32)
  shm->handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name);
  if (shm->handle == NULL) {
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_NOT_FOUND;
  }

  shm->ptr = MapViewOfFile(shm->handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
  if (shm->ptr == NULL) {
    CloseHandle(shm->handle);
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_UNKNOWN;
  }
#else
  shm->fd = shm_open(name, O_RDWR, 0666);
  if (shm->fd < 0) {
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_NOT_FOUND;
  }

  shm->ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fd, 0);
  if (shm->ptr == MAP_FAILED) {
    close(shm->fd);
    alloc.free(alloc.ctx, shm);
    return CMP_ERR_UNKNOWN;
  }
#endif

  *out_shm = shm;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_shm_get_ptr(CMPSharedMemory *shm, void **out_ptr) {
  if (!out_ptr)
    return 1;
  if (shm == NULL) {
    return 1;
  }
  *out_ptr = shm->ptr;
  return 0;
}

CMP_API int CMP_CALL cmp_shm_close(CMPSharedMemory *shm) {
  CMPAllocator alloc;
  int rc;

  if (shm == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_get_default_allocator(&alloc);
  if (rc != CMP_OK) {
    return rc; /* Should not happen if shm exists, but safe fallback */
  }

#if defined(_WIN32)
  UnmapViewOfFile(shm->ptr);
  CloseHandle(shm->handle);
#else
  munmap(shm->ptr, shm->size);
  close(shm->fd);
#endif

  alloc.free(alloc.ctx, shm);
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_shm_unlink(const char *name) {
  if (name == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  /* Windows unlinks automatically when the last handle is closed. */
  return CMP_OK;
#else
  if (shm_unlink(name) == 0) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#endif
}