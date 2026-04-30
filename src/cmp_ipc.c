/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <c_abstract_http/actor.h>
#include <stdlib.h>
#include <string.h>

struct CddProcess {
  void *mock_queue;
};

/**
 * @brief cmp_msg_create
 *
 * @param msg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_msg_create(cmp_msg_t **msg) {
int rc = CMP_SUCCESS;

  if (!msg) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_msg_create: Invalid argument\n");
    
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_msg_t), (void **)&(*msg));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  (*msg)->type = 0;
  (*msg)->payload = NULL;
  (*msg)->sender = NULL;
  
  return rc;
}

/**
 * @brief cmp_msg_destroy
 *
 * @param msg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_msg_destroy(cmp_msg_t *msg) {
int rc = CMP_SUCCESS;
  int free_rc;

  if (!msg) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_msg_destroy: Invalid argument\n");
    
    return rc;
  }

  /* Warning: if payload is managed elsewhere, this might be unsafe, but we assume it's owned here for serialization */
  if (msg->payload) {
      free_rc = CMP_FREE(msg->payload);
      if (free_rc != CMP_SUCCESS) {
        LOG_DEBUG("Free failed\n");
        rc = free_rc;
      }
  }
  free_rc = CMP_FREE(msg);
  if (free_rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    rc = free_rc;
  }
  
  
  return rc;
}

/**
 * @brief cmp_msg_set_payload
 *
 * @param msg Parameter description.
 * @param payload Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_msg_set_payload(cmp_msg_t *msg, const void *payload, size_t size) {
int rc = CMP_SUCCESS;
  int free_rc;

  if (!msg || !payload) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_msg_set_payload: Invalid argument\n");
    
    return rc;
  }

  if (msg->payload) {
    free_rc = CMP_FREE(msg->payload);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
      rc = free_rc;
    }
  }

  /* Store size in first 4 bytes of payload to know its size, or assume string */
  rc = CMP_MALLOC(size + sizeof(size_t), (void **)&(msg->payload));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memcpy(msg->payload, &size, sizeof(size_t));
  memcpy((uint8_t*)msg->payload + sizeof(size_t), payload, size);
  
  return rc;
}

/**
 * @brief cmp_msg_serialize
 *
 * @param msg Parameter description.
 * @param buffer Parameter description.
 * @param buffer_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_msg_serialize(const cmp_msg_t *msg, uint8_t **buffer, size_t *buffer_size) {
int rc = CMP_SUCCESS;
  size_t psize = 0;
  uint8_t *p = NULL;

  if (!msg || !buffer || !buffer_size) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_msg_serialize: Invalid argument\n");
    
    return rc;
  }
  
  if (msg->payload) {
      memcpy(&psize, msg->payload, sizeof(size_t));
  }
  
  *buffer_size = sizeof(int) + sizeof(size_t) + psize;
  rc = CMP_MALLOC(*buffer_size, (void **)&(*buffer));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  
  p = *buffer;
  memcpy(p, &msg->type, sizeof(int));
  p += sizeof(int);
  memcpy(p, &psize, sizeof(size_t));
  p += sizeof(size_t);
  if (psize > 0) {
      memcpy(p, (uint8_t*)msg->payload + sizeof(size_t), psize);
  }
  
  
  
  return rc;
}

/**
 * @brief cmp_msg_deserialize
 *
 * @param buffer Parameter description.
 * @param buffer_size Parameter description.
 * @param msg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_msg_deserialize(const uint8_t *buffer, size_t buffer_size, cmp_msg_t **msg) {
int rc = CMP_SUCCESS;
  int err;
  const uint8_t *p = NULL;
  size_t psize;

  if (!buffer || !msg || buffer_size < sizeof(int) + sizeof(size_t)) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_msg_deserialize: Invalid argument or bounds\n");
    
    return rc;
  }
  
  err = cmp_msg_create(msg);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_msg_deserialize: cmp_msg_create failed\n");
    return err;
  }
  
  p = buffer;
  memcpy(&(*msg)->type, p, sizeof(int));
  p += sizeof(int);
  
  memcpy(&psize, p, sizeof(size_t));
  p += sizeof(size_t);
  
  if (buffer_size < sizeof(int) + sizeof(size_t) + psize) {
      cmp_msg_destroy(*msg);
      *msg = NULL;
      rc = CMP_ERROR_BOUNDS;
      LOG_DEBUG("Error in cmp_msg_deserialize: Buffer too small for payload\n");
      
      return rc;
  }
  
  if (psize > 0) {
      err = cmp_msg_set_payload(*msg, p, psize);
      if (err != CMP_SUCCESS) {
          cmp_msg_destroy(*msg);
          *msg = NULL;
          LOG_DEBUG("Error in cmp_msg_deserialize: cmp_msg_set_payload failed\n");
          return err;
      }
  }
  
  
  
  return rc;
}

/**
 * @brief cmp_process_send
 *
 * @param proc Parameter description.
 * @param msg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_process_send(cmp_process_t *proc, const cmp_msg_t *msg) {
int rc = CMP_SUCCESS;

  if (!proc || !msg) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_process_send: Invalid argument\n");
    
    return rc;
  }
  /* Mock implementation for IPC pipes, assuming write to cdd pipe */
  /* This is just a stub for tests since cdd_process_send doesn't exist out of the box in process.h */
  
  
  return rc;
}

/**
 * @brief cmp_process_recv
 *
 * @param proc Parameter description.
 * @param msg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_process_recv(cmp_process_t *proc, cmp_msg_t **msg) {
int rc = CMP_SUCCESS;

  if (!proc || !msg) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_process_recv: Invalid argument\n");
    
    return rc;
  }
  
  
  return rc;
}

/**
 * @brief cmp_process_destroy
 *
 * @param proc Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_process_destroy(cmp_process_t *proc) {
int rc = CMP_SUCCESS;

  if (!proc) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_process_destroy: Invalid argument\n");
    
    return rc;
  }
  rc = CMP_FREE(proc);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  
  return rc;
}

/**
 * @brief cmp_process_spawn
 *
 * @param proc Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_process_spawn(cmp_process_t **proc) {
int rc = CMP_SUCCESS;

  if (!proc) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_process_spawn: Invalid argument\n");
    
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct CddProcess), (void **)&(*proc));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  (*proc)->mock_queue = NULL;
  
  return rc;
}

/* clang-format on */