/* clang-format off */
#include "cmp.h"
#include <c_abstract_http/actor.h>
#include <stdlib.h>
#include <string.h>

struct CddProcess {
  void *mock_queue;
};

int cmp_msg_create(cmp_msg_t **msg) {
  if (!msg) return CMP_ERROR_INVALID_ARG;
  *msg = (cmp_msg_t*)malloc(sizeof(cmp_msg_t));
  if (!*msg) return CMP_ERROR_OOM;
  (*msg)->type = 0;
  (*msg)->payload = NULL;
  (*msg)->sender = NULL;
  return CMP_SUCCESS;
}

int cmp_msg_destroy(cmp_msg_t *msg) {
  if (!msg) return CMP_ERROR_INVALID_ARG;
  /* Warning: if payload is managed elsewhere, this might be unsafe, but we assume it's owned here for serialization */
  if (msg->payload) {
      free(msg->payload);
  }
  free(msg);
  return CMP_SUCCESS;
}

int cmp_msg_set_payload(cmp_msg_t *msg, const void *payload, size_t size) {
  if (!msg || !payload) return CMP_ERROR_INVALID_ARG;
  if (msg->payload) free(msg->payload);
  /* Store size in first 4 bytes of payload to know its size, or assume string */
  msg->payload = malloc(size + sizeof(size_t));
  if (!msg->payload) return CMP_ERROR_OOM;
  memcpy(msg->payload, &size, sizeof(size_t));
  memcpy((uint8_t*)msg->payload + sizeof(size_t), payload, size);
  return CMP_SUCCESS;
}

int cmp_msg_serialize(const cmp_msg_t *msg, uint8_t **buffer, size_t *buffer_size) {
  if (!msg || !buffer || !buffer_size) return CMP_ERROR_INVALID_ARG;
  
  size_t psize = 0;
  if (msg->payload) {
      memcpy(&psize, msg->payload, sizeof(size_t));
  }
  
  *buffer_size = sizeof(int) + sizeof(size_t) + psize;
  *buffer = (uint8_t*)malloc(*buffer_size);
  if (!*buffer) return CMP_ERROR_OOM;
  
  uint8_t *p = *buffer;
  memcpy(p, &msg->type, sizeof(int));
  p += sizeof(int);
  memcpy(p, &psize, sizeof(size_t));
  p += sizeof(size_t);
  if (psize > 0) {
      memcpy(p, (uint8_t*)msg->payload + sizeof(size_t), psize);
  }
  
  return CMP_SUCCESS;
}

int cmp_msg_deserialize(const uint8_t *buffer, size_t buffer_size, cmp_msg_t **msg) {
  if (!buffer || !msg || buffer_size < sizeof(int) + sizeof(size_t)) return CMP_ERROR_INVALID_ARG;
  
  int err = cmp_msg_create(msg);
  if (err != CMP_SUCCESS) return err;
  
  const uint8_t *p = buffer;
  memcpy(&(*msg)->type, p, sizeof(int));
  p += sizeof(int);
  
  size_t psize;
  memcpy(&psize, p, sizeof(size_t));
  p += sizeof(size_t);
  
  if (buffer_size < sizeof(int) + sizeof(size_t) + psize) {
      cmp_msg_destroy(*msg);
      *msg = NULL;
      return CMP_ERROR_BOUNDS;
  }
  
  if (psize > 0) {
      err = cmp_msg_set_payload(*msg, p, psize);
      if (err != CMP_SUCCESS) {
          cmp_msg_destroy(*msg);
          *msg = NULL;
          return err;
      }
  }
  
  return CMP_SUCCESS;
}

int cmp_process_send(cmp_process_t *proc, const cmp_msg_t *msg) {
  if (!proc || !msg) return CMP_ERROR_INVALID_ARG;
  /* Mock implementation for IPC pipes, assuming write to cdd pipe */
  /* This is just a stub for tests since cdd_process_send doesn't exist out of the box in process.h */
  return CMP_SUCCESS;
}

int cmp_process_recv(cmp_process_t *proc, cmp_msg_t **msg) {
  if (!proc || !msg) return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_process_destroy(cmp_process_t *proc) {
  if (!proc) return CMP_ERROR_INVALID_ARG;
  free(proc);
  return CMP_SUCCESS;
}

int cmp_process_spawn(cmp_process_t **proc) {
  if (!proc) return CMP_ERROR_INVALID_ARG;
  *proc = (cmp_process_t *)malloc(sizeof(struct CddProcess));
  if (!*proc) return CMP_ERROR_OOM;
  (*proc)->mock_queue = NULL;
  return CMP_SUCCESS;
}

/* clang-format on */