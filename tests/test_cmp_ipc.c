/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

TEST test_msg_serialization(void) {
  cmp_msg_t *msg;
  uint8_t *buffer = NULL;
  size_t buffer_size = 0;
  cmp_msg_t *deserialized = NULL;
  int err;

  /* We must define cmp_msg_create, cmp_msg_serialize, cmp_msg_deserialize */
  err = cmp_msg_create(&msg);
  ASSERT_EQ(CMP_SUCCESS, err);

  err = cmp_msg_set_payload(msg, "hello", 6);
  ASSERT_EQ(CMP_SUCCESS, err);

  err = cmp_msg_serialize(msg, &buffer, &buffer_size);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT(buffer != NULL);
  ASSERT(buffer_size > 0);

  err = cmp_msg_deserialize(buffer, buffer_size, &deserialized);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT(deserialized != NULL);

  /* Clean up */
  cmp_msg_destroy(msg);
  cmp_msg_destroy(deserialized);
  CMP_FREE(buffer);

  PASS();
}

TEST test_multiprocess_communication(void) {
  cmp_process_t *proc = NULL;
  int err;

  err = cmp_process_spawn(&proc);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT(proc != NULL);

  /* Send and receive message */
  cmp_msg_t *msg;
  cmp_msg_create(&msg);
  cmp_msg_set_payload(msg, "ping", 5);

  err = cmp_process_send(proc, msg);
  ASSERT_EQ(CMP_SUCCESS, err);

  cmp_msg_t *response = NULL;
  /* Not blocking for real in this mock test, just verifying API */
  err = cmp_process_recv(proc, &response);
  ASSERT_EQ(CMP_SUCCESS, err);

  if (response) {
    cmp_msg_destroy(response);
  }

  cmp_msg_destroy(msg);
  cmp_process_destroy(proc);

  PASS();
}

SUITE(cmp_ipc_suite) {
  RUN_TEST(test_msg_serialization);
  RUN_TEST(test_multiprocess_communication);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ipc_suite);
  GREATEST_MAIN_END();
}
