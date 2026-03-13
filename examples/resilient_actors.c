/* resilient_actors.c */
/* clang-format off */
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_message.h"
#include "cmpc/cmp_tasks.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#define TOPIC_JOB 1
#define TOPIC_STATUS 2

#define MSG_TYPE_CRASH_COMMAND 100
#define MSG_TYPE_DO_WORK 101

typedef struct {
  int actor_id;
  int work_completed;
  CMPMessageBus *bus;
} WorkerContext;

static int CMP_CALL worker_handler(CMPActor *actor, const CMPMessage *msg) {
  WorkerContext *ctx = NULL;
  cmp_actor_get_context(actor, (void **)&ctx);

  if (msg->type == MSG_TYPE_CRASH_COMMAND) {
    printf("Worker %d received crash command! Simulating catastrophic "
           "failure...\n",
           ctx->actor_id);
    /* Return an error code. The supervision tree will catch this and restart
     * the actor. */
    return CMP_ERR_UNKNOWN;
  } else if (msg->type == MSG_TYPE_DO_WORK) {
    ctx->work_completed++;
    printf("Worker %d completed work unit. Total completed: %d\n",
           ctx->actor_id, ctx->work_completed);
  }

  return CMP_OK;
}

int main(void) {
  CMPAllocator alloc;
  CMPMessageBus *bus;
  CMPMessageBusConfig bus_config;
  CMPActor *worker;
  CMPActorConfig actor_config;
  WorkerContext worker_ctx;
  CMPMessage msg;
  int i;

  printf("Initializing Resilient Actors example (Message Passing & "
         "Supervision)...\n");

  cmp_get_default_allocator(&alloc);

  bus_config.max_subscribers = 10;
  cmp_message_bus_create(&alloc, &bus_config, &bus);

  worker_ctx.actor_id = 42;
  worker_ctx.work_completed = 0;
  worker_ctx.bus = bus;

  actor_config.handler = worker_handler;
  actor_config.ctx = &worker_ctx;
  actor_config.topic = TOPIC_JOB;
  actor_config.restart_on_crash = 1; /* Key to resilience! */

  printf("Spawning worker actor ID %d on topic %d with restart_on_crash = "
         "true...\n",
         worker_ctx.actor_id, TOPIC_JOB);
  cmp_actor_spawn(bus, &actor_config, &worker);

  /* Send some normal work messages */
  msg.topic = TOPIC_JOB;
  msg.type = MSG_TYPE_DO_WORK;
  msg.payload = NULL;
  msg.payload_size = 0;
  msg.sender = NULL;

  for (i = 0; i < 3; i++) {
    printf("Main sending DO_WORK message to topic %d...\n", TOPIC_JOB);
    cmp_message_bus_publish(bus, &msg);
  }

  /* Send a crash command */
  printf("\n--- Injecting Fault ---\n");
  msg.type = MSG_TYPE_CRASH_COMMAND;
  printf("Main sending CRASH_COMMAND message to topic %d...\n", TOPIC_JOB);
  cmp_message_bus_publish(bus, &msg);

  /* Validate that the actor is marked as crashed before the next iteration or
   * restart logic */
  /* In this simple synchronous example, the framework would have restarted it
     immediately upon return of CMP_ERR_UNKNOWN, but we need to verify its state
     or context behavior. For our simple implementation, it re-registers the
     handler. */
  printf("--- Fault Injection Complete ---\n\n");

  /* Because of restart_on_crash, the actor should still be alive and receive
   * this next message */
  msg.type = MSG_TYPE_DO_WORK;
  printf("Main sending DO_WORK message to topic %d (Checking if actor "
         "survived/restarted)...\n",
         TOPIC_JOB);
  cmp_message_bus_publish(bus, &msg);

  printf("Cleaning up...\n");
  cmp_actor_destroy(bus, worker);
  cmp_message_bus_destroy(&alloc, bus);

  return 0;
}