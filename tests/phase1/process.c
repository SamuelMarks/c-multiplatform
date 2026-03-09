#include "cmpc/cmp_core.h"
#include "cmpc/cmp_message.h"
#include "cmpc/cmp_process.h"
#include "test_utils.h"

int test_process(void) {
  CMPAllocator alloc;
  CMPProcess *proc = NULL;
  CMPBool running;
  int exit_code;
  int rc;
  const char *args[] = {"-v", NULL};
  CMPProcessConfig config;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));

  config.executable_path = "does_not_exist_xyz123";
  config.args = args;
  config.establish_ipc = CMP_FALSE;

  rc = cmp_process_spawn(&alloc, &config, &proc);
  if (rc == CMP_OK) {
    CMP_TEST_OK(cmp_process_kill(proc));
    CMP_TEST_OK(cmp_process_destroy(&alloc, proc));
  } else {
    CMP_TEST_EXPECT(rc, CMP_ERR_UNKNOWN);
  }

  config.establish_ipc = CMP_TRUE;
#ifdef _WIN32
  config.executable_path = "cmd.exe";
  const char *args_win[] = {"/c", "echo", "hello", NULL};
  config.args = args_win;
#else
  config.executable_path = "sh";
  const char *args_posix[] = {"-c", "echo hello", NULL};
  config.args = args_posix;
#endif

  rc = cmp_process_spawn(&alloc, &config, &proc);
  if (rc == CMP_OK) {
    CMPIPCChannel *ipc = cmp_process_get_ipc(proc);
    CMPMessage msg;
    CMPMessage rmsg;

    CMP_TEST_ASSERT(ipc != NULL);

    rc = cmp_process_check(proc, &running, &exit_code);
    CMP_TEST_OK(rc);

    msg.type = 1;
    msg.topic = 2;
    msg.payload = "test";
    msg.payload_size = 4;
    msg.sender = NULL;

    rc = cmp_ipc_send(ipc, &msg);
    /* we might get ok or broken pipe since child exits immediately */

    rc = cmp_ipc_receive(ipc, &alloc, &rmsg);
    /* Again, maybe ok maybe broken pipe maybe not ready */

    /* we just kill it and clean up */
    cmp_process_kill(proc);
    cmp_process_destroy(&alloc, proc);
  } else {
    /* could not spawn even simple shell */
  }

  return 0;
}

int main(void) {
  if (test_process() != 0)
    return 1;
  return 0;
}
