/* clang-format off */
#include "cmpc/cmp_process.h"

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#endif
/* clang-format on */

struct CMPIPCChannel {
#if defined(_WIN32)
  HANDLE read_pipe;
  HANDLE write_pipe;
#else
  int read_fd;
  int write_fd;
#endif
};

struct CMPProcess {
  CMPAllocator allocator;
  CMPIPCChannel *ipc;
#if defined(_WIN32)
  HANDLE process_handle;
  DWORD process_id;
#else
  pid_t pid;
#endif
};

/* Simplified IPC: raw bytes over pipes. In a real system, you'd want
 * length-prefixed framing and structured serialization. This is an MVP. */
typedef struct IPCFrameHeader {
  cmp_u32 magic; /* 0xC119C119 */
  cmp_u32 type;
  cmp_u32 topic;
  cmp_u32 payload_size;
} IPCFrameHeader;

#define IPC_MAGIC 0xC119C119

CMP_API int CMP_CALL cmp_process_spawn(CMPAllocator *alloc,
                                       const CMPProcessConfig *config,
                                       CMPProcess **out_process) {
  CMPProcess *proc;
  int rc;
#if defined(_WIN32)
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  HANDLE parent_read = NULL, parent_write = NULL;
  HANDLE child_read = NULL, child_write = NULL;
  SECURITY_ATTRIBUTES sa;
  char cmdline[1024]; /* Fixed size for MVP */
  int i;
#else
  int parent_to_child[2] = {-1, -1};
  int child_to_parent[2] = {-1, -1};
  pid_t pid;
#endif

  if (alloc == NULL || alloc->alloc == NULL || config == NULL ||
      config->executable_path == NULL || out_process == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = alloc->alloc(alloc->ctx, sizeof(CMPProcess), (void **)&proc);
  if (rc != CMP_OK || proc == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  memset(proc, 0, sizeof(*proc));
  proc->allocator = *alloc;

#if defined(_WIN32)
  if (config->establish_ipc) {
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&parent_read, &child_write, &sa, 0))
      goto error_win32;
    if (!SetHandleInformation(parent_read, HANDLE_FLAG_INHERIT, 0))
      goto error_win32;

    if (!CreatePipe(&child_read, &parent_write, &sa, 0))
      goto error_win32;
    if (!SetHandleInformation(parent_write, HANDLE_FLAG_INHERIT, 0))
      goto error_win32;
  }

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  if (config->establish_ipc) {
    si.hStdError = child_write;
    si.hStdOutput = child_write;
    si.hStdInput = child_read;
    si.dwFlags |= STARTF_USESTDHANDLES;
  }
  ZeroMemory(&pi, sizeof(pi));

  cmdline[0] = '\0';
#if defined(_MSC_VER)
  strncat_s(cmdline, sizeof(cmdline), config->executable_path, _TRUNCATE);
  if (config->args != NULL) {
    for (i = 0; config->args[i] != NULL; i++) {
      strncat_s(cmdline, sizeof(cmdline), " ", _TRUNCATE);
      strncat_s(cmdline, sizeof(cmdline), config->args[i], _TRUNCATE);
    }
  }
#else
  strncat(cmdline, config->executable_path, sizeof(cmdline) - 1);
  if (config->args != NULL) {
    for (i = 0; config->args[i] != NULL; i++) {
      strncat(cmdline, " ", sizeof(cmdline) - strlen(cmdline) - 1);
      strncat(cmdline, config->args[i], sizeof(cmdline) - strlen(cmdline) - 1);
    }
  }
#endif

  if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si,
                      &pi)) {
    goto error_win32;
  }

  proc->process_handle = pi.hProcess;
  proc->process_id = pi.dwProcessId;
  CloseHandle(pi.hThread);

  if (config->establish_ipc) {
    CloseHandle(child_read);
    CloseHandle(child_write);
    rc = alloc->alloc(alloc->ctx, sizeof(CMPIPCChannel), (void **)&proc->ipc);
    if (rc == CMP_OK && proc->ipc != NULL) {
      proc->ipc->read_pipe = parent_read;
      proc->ipc->write_pipe = parent_write;
    }
  }

  *out_process = proc;
  return CMP_OK;

error_win32:
  if (parent_read)
    CloseHandle(parent_read);
  if (parent_write)
    CloseHandle(parent_write);
  if (child_read)
    CloseHandle(child_read);
  if (child_write)
    CloseHandle(child_write);
  alloc->free(alloc->ctx, proc);
  return CMP_ERR_UNKNOWN;

#else
  if (config->establish_ipc) {
    if (pipe(parent_to_child) < 0)
      goto error_posix;
    if (pipe(child_to_parent) < 0)
      goto error_posix;
  }

  pid = fork();
  if (pid < 0) {
    goto error_posix;
  } else if (pid == 0) {
    /* Child */
    if (config->establish_ipc) {
      close(parent_to_child[1]);
      close(child_to_parent[0]);
      dup2(parent_to_child[0], STDIN_FILENO);
      dup2(child_to_parent[1], STDOUT_FILENO);
      close(parent_to_child[0]);
      close(child_to_parent[1]);
    }

    /* Exec */
    /* Needs execv/execvp with casted args for C89 */
    {
      char *argv[64]; /* MVP fixed size */
      int i = 0;
      argv[0] = (char *)config->executable_path;
      if (config->args != NULL) {
        for (i = 0; config->args[i] != NULL && i < 62; i++) {
          argv[i + 1] = (char *)config->args[i];
        }
      }
      argv[i + 1] = NULL;
      execvp(config->executable_path, argv);
    }
    exit(127); /* If exec fails */
  } else {
    /* Parent */
    proc->pid = pid;
    if (config->establish_ipc) {
      close(parent_to_child[0]);
      close(child_to_parent[1]);

      /* Set non-blocking */
      fcntl(child_to_parent[0], F_SETFL,
            fcntl(child_to_parent[0], F_GETFL) | O_NONBLOCK);

      rc = alloc->alloc(alloc->ctx, sizeof(CMPIPCChannel), (void **)&proc->ipc);
      if (rc == CMP_OK && proc->ipc != NULL) {
        proc->ipc->read_fd = child_to_parent[0];
        proc->ipc->write_fd = parent_to_child[1];
      }
    }

    *out_process = proc;
    return CMP_OK;
  }

error_posix:
  if (parent_to_child[0] >= 0)
    close(parent_to_child[0]);
  if (parent_to_child[1] >= 0)
    close(parent_to_child[1]);
  if (child_to_parent[0] >= 0)
    close(child_to_parent[0]);
  if (child_to_parent[1] >= 0)
    close(child_to_parent[1]);
  alloc->free(alloc->ctx, proc);
  return CMP_ERR_UNKNOWN;
#endif
}

CMP_API int CMP_CALL cmp_process_check(CMPProcess *process,
                                       CMPBool *out_running,
                                       int *out_exit_code) {
#if defined(_WIN32)
  DWORD exit_code = 0;
#else
  int status;
  pid_t result;
#endif

  if (process == NULL || out_running == NULL || out_exit_code == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  if (GetExitCodeProcess(process->process_handle, &exit_code)) {
    if (exit_code == STILL_ACTIVE) {
      *out_running = 1;
      *out_exit_code = 0;
    } else {
      *out_running = 0;
      *out_exit_code = (int)exit_code;
    }
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#else
  result = waitpid(process->pid, &status, WNOHANG);
  if (result == 0) {
    *out_running = 1;
    *out_exit_code = 0;
    return CMP_OK;
  } else if (result == process->pid) {
    *out_running = 0;
    if (WIFEXITED(status)) {
      *out_exit_code = WEXITSTATUS(status);
    } else {
      *out_exit_code = -1; /* Killed by signal */
    }
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#endif
}

CMP_API int CMP_CALL cmp_process_kill(CMPProcess *process) {
  if (process == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(_WIN32)
  if (TerminateProcess(process->process_handle, 1)) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#else
  if (kill(process->pid, SIGKILL) == 0) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#endif
}

CMP_API int CMP_CALL cmp_process_destroy(CMPAllocator *alloc,
                                         CMPProcess *process) {
  if (alloc == NULL || alloc->free == NULL || process == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  CloseHandle(process->process_handle);
  if (process->ipc != NULL) {
    CloseHandle(process->ipc->read_pipe);
    CloseHandle(process->ipc->write_pipe);
    alloc->free(alloc->ctx, process->ipc);
  }
#else
  if (process->ipc != NULL) {
    close(process->ipc->read_fd);
    close(process->ipc->write_fd);
    alloc->free(alloc->ctx, process->ipc);
  }
#endif

  alloc->free(alloc->ctx, process);
  return CMP_OK;
}

CMP_API CMPIPCChannel *CMP_CALL cmp_process_get_ipc(CMPProcess *process) {
  if (process == NULL) {
    return NULL;
  }
  return process->ipc;
}

CMP_API int CMP_CALL cmp_ipc_send(CMPIPCChannel *channel,
                                  const CMPMessage *msg) {
  IPCFrameHeader header;
#if defined(_WIN32)
  DWORD written;
#else
  ssize_t written;
#endif

  if (channel == NULL || msg == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  header.magic = IPC_MAGIC;
  header.type = msg->type;
  header.topic = msg->topic;
  header.payload_size = (cmp_u32)msg->payload_size;

#if defined(_WIN32)
  if (!WriteFile(channel->write_pipe, &header, sizeof(header), &written,
                 NULL)) {
    return CMP_ERR_IO;
  }
  if (msg->payload_size > 0 && msg->payload != NULL) {
    if (!WriteFile(channel->write_pipe, msg->payload, (DWORD)msg->payload_size,
                   &written, NULL)) {
      return CMP_ERR_IO;
    }
  }
#else
  written = write(channel->write_fd, &header, sizeof(header));
  if (written != sizeof(header)) {
    return CMP_ERR_IO;
  }
  if (msg->payload_size > 0 && msg->payload != NULL) {
    written = write(channel->write_fd, msg->payload, msg->payload_size);
    if (written != (ssize_t)msg->payload_size) {
      return CMP_ERR_IO;
    }
  }
#endif

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_ipc_receive(CMPIPCChannel *channel,
                                     CMPAllocator *alloc, CMPMessage *out_msg) {
  IPCFrameHeader header;
#if defined(_WIN32)
  DWORD bytes_read;
  DWORD available;
#else
  ssize_t bytes_read;
#endif

  if (channel == NULL || alloc == NULL || alloc->alloc == NULL ||
      out_msg == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  if (!PeekNamedPipe(channel->read_pipe, NULL, 0, NULL, &available, NULL)) {
    return CMP_ERR_IO;
  }
  if (available < sizeof(header)) {
    return CMP_ERR_NOT_READY;
  }

  if (!ReadFile(channel->read_pipe, &header, sizeof(header), &bytes_read,
                NULL)) {
    return CMP_ERR_IO;
  }
#else
  bytes_read = read(channel->read_fd, &header, sizeof(header));
  if (bytes_read < 0) {
    return CMP_ERR_NOT_READY; /* Would block */
  } else if (bytes_read == 0) {
    return CMP_ERR_IO; /* EOF */
  } else if (bytes_read != sizeof(header)) {
    /* Partial read MVP issue, robust framing needed for prod */
    return CMP_ERR_IO;
  }
#endif

  if (header.magic != IPC_MAGIC) {
    return CMP_ERR_UNKNOWN; /* Corrupt stream */
  }

  out_msg->type = header.type;
  out_msg->topic = header.topic;
  out_msg->payload_size = header.payload_size;
  out_msg->sender = NULL;
  out_msg->payload = NULL;

  if (header.payload_size > 0) {
    if (alloc->alloc(alloc->ctx, header.payload_size, &out_msg->payload) !=
        CMP_OK) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
#if defined(_WIN32)
    if (!ReadFile(channel->read_pipe, out_msg->payload, header.payload_size,
                  &bytes_read, NULL)) {
      alloc->free(alloc->ctx, out_msg->payload);
      out_msg->payload = NULL;
      return CMP_ERR_IO;
    }
#else
    bytes_read = read(channel->read_fd, out_msg->payload, header.payload_size);
    if (bytes_read != (ssize_t)header.payload_size) {
      alloc->free(alloc->ctx, out_msg->payload);
      out_msg->payload = NULL;
      return CMP_ERR_IO;
    }
#endif
  }

  return CMP_OK;
}