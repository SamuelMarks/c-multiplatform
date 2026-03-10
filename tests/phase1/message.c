#include "cmpc/cmp_core.h"
#include "cmpc/cmp_message.h"
#include "test_utils.h"

static int test_handler_called = 0;
static const CMPMessageTopic TEST_TOPIC = 1;

static int CMP_CALL my_handler(void *ctx, const CMPMessage *msg) {
  (void)ctx;
  if (msg->topic == TEST_TOPIC) {
    test_handler_called++;
  }
  return CMP_OK;
}

static int CMP_CALL actor_handler(CMPActor *actor, const CMPMessage *msg) {
  (void)actor;
  if (msg->topic == TEST_TOPIC) {
    if (msg->type == 999) {
      return CMP_ERR_UNKNOWN; /* Simulate crash */
    }
    test_handler_called++;
  }
  return CMP_OK;
}

static int CMP_CALL actor_handler_no_restart(CMPActor *actor,
                                             const CMPMessage *msg) {
  (void)actor;
  if (msg->topic == TEST_TOPIC) {
    if (msg->type == 999) {
      return CMP_ERR_UNKNOWN; /* Simulate crash */
    }
    test_handler_called++;
  }
  return CMP_OK;
}

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

int test_message_bus(void) {
  CMPAllocator alloc;
  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  CMPAllocator bad_alloc = alloc;
  bad_alloc.alloc = NULL;
  bad_alloc.free = NULL;

  CMPAllocator fail_allocator;
  fail_allocator.ctx = NULL;
  fail_allocator.alloc = fail_alloc;
  fail_allocator.realloc = NULL;
  fail_allocator.free = fake_free;

  CMPMessageBus *bus = NULL;
  void *ctx_ptr = NULL;
  CMPActorState st;
  CMPMessageBusConfig config = {.max_subscribers = 2};
  CMPMessageBusConfig no_limit_config = {.max_subscribers = 0};

  /* Test invalid args for create */
  CMP_TEST_EXPECT(cmp_message_bus_create(NULL, &config, &bus),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_message_bus_create(&bad_alloc, &config, &bus),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_message_bus_create(&alloc, NULL, &bus),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_message_bus_create(&alloc, &config, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test out of memory */
  CMP_TEST_EXPECT(cmp_message_bus_create(&fail_allocator, &config, &bus),
                  CMP_ERR_OUT_OF_MEMORY);

  /* Create success */
  CMP_TEST_OK(cmp_message_bus_create(&alloc, &config, &bus));

  /* Test invalid args for destroy */
  CMP_TEST_EXPECT(cmp_message_bus_destroy(NULL, bus), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_message_bus_destroy(&bad_alloc, bus),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_message_bus_destroy(&alloc, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test invalid args for subscribe */
  CMPMessageSubscription sub1 = NULL;
  CMP_TEST_EXPECT(
      cmp_message_bus_subscribe(NULL, TEST_TOPIC, my_handler, NULL, &sub1),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_message_bus_subscribe(bus, TEST_TOPIC, NULL, NULL, &sub1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_message_bus_subscribe(bus, TEST_TOPIC, my_handler, NULL, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  /* Test valid subscribe */
  CMP_TEST_OK(
      cmp_message_bus_subscribe(bus, TEST_TOPIC, my_handler, NULL, &sub1));

  /* Test max subscribers limit */
  CMPMessageSubscription sub2 = NULL;
  CMP_TEST_OK(
      cmp_message_bus_subscribe(bus, TEST_TOPIC, my_handler, NULL, &sub2));
  CMPMessageSubscription sub3 = NULL;
  CMP_TEST_EXPECT(
      cmp_message_bus_subscribe(bus, TEST_TOPIC, my_handler, NULL, &sub3),
      CMP_ERR_RANGE);

  /* Test invalid args for publish */
  CMPMessage msg = {.topic = TEST_TOPIC,
                    .payload = NULL,
                    .payload_size = 0,
                    .type = 0,
                    .sender = NULL};
  CMP_TEST_EXPECT(cmp_message_bus_publish(NULL, &msg),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_message_bus_publish(bus, NULL), CMP_ERR_INVALID_ARGUMENT);

  /* Publish */
  test_handler_called = 0;
  CMP_TEST_OK(cmp_message_bus_publish(bus, &msg));
  CMP_TEST_EXPECT(test_handler_called, 2);

  /* Invalid args for unsubscribe */
  CMP_TEST_EXPECT(cmp_message_bus_unsubscribe(NULL, sub1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_message_bus_unsubscribe(bus, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Unsubscribe first node */
  CMP_TEST_OK(cmp_message_bus_unsubscribe(
      bus, sub2)); /* sub2 is the first in list (LIFO) */
  /* Unsubscribe not found */
  CMP_TEST_EXPECT(cmp_message_bus_unsubscribe(bus, sub2), CMP_ERR_NOT_FOUND);

  /* Unsubscribe remaining */
  CMP_TEST_OK(cmp_message_bus_unsubscribe(bus, sub1));

  /* Destroy bus to re-create */
  CMP_TEST_OK(cmp_message_bus_destroy(&alloc, bus));

  /* Recreate normal bus */
  CMP_TEST_OK(cmp_message_bus_create(&alloc, &no_limit_config, &bus));

  /* Test actor invalid args */
  CMPActor *actor = NULL;
  CMPActorConfig a_cfg = {.topic = TEST_TOPIC,
                          .handler = actor_handler,
                          .ctx = (void *)123,
                          .restart_on_crash = 1};
  CMPActorConfig bad_a_cfg = {
      .topic = TEST_TOPIC, .handler = NULL, .ctx = NULL, .restart_on_crash = 1};

  CMP_TEST_EXPECT(cmp_actor_spawn(NULL, &a_cfg, &actor),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_actor_spawn(bus, NULL, &actor), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_actor_spawn(bus, &bad_a_cfg, &actor),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_actor_spawn(bus, &a_cfg, NULL), CMP_ERR_INVALID_ARGUMENT);

  /* Test actor getters with NULL */
  CMP_TEST_EXPECT(cmp_actor_get_context(NULL, &ctx_ptr) != 0, 1);
  CMP_TEST_EXPECT(cmp_actor_get_state(NULL, &st) != 0, 1);

  /* Spawn actor */
  CMP_TEST_OK(cmp_actor_spawn(bus, &a_cfg, &actor));
  CMP_TEST_EXPECT(cmp_actor_get_context(actor, &ctx_ptr) == 0, 1);
  CMP_TEST_EXPECT(ctx_ptr == (void *)123, 1);
  CMP_TEST_EXPECT(cmp_actor_get_state(actor, &st) == 0, 1);
  CMP_TEST_EXPECT(st, CMP_ACTOR_STATE_RUNNING);

  /* Publish to actor */
  test_handler_called = 0;
  CMP_TEST_OK(cmp_message_bus_publish(bus, &msg));
  CMP_TEST_EXPECT(test_handler_called, 1);

  /* Publish crash message to actor with restart_on_crash = 1 */
  CMPMessage crash_msg = {.topic = TEST_TOPIC,
                          .payload = NULL,
                          .payload_size = 0,
                          .type = 999,
                          .sender = NULL};
  CMP_TEST_OK(cmp_message_bus_publish(bus, &crash_msg));
  CMP_TEST_EXPECT(cmp_actor_get_state(actor, &st) == 0, 1);
  CMP_TEST_EXPECT(st, CMP_ACTOR_STATE_RUNNING);

  /* Test invalid actor destroy */
  CMP_TEST_EXPECT(cmp_actor_destroy(NULL, actor), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_actor_destroy(bus, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMPMessageBus *bus2 = NULL;
  CMP_TEST_OK(cmp_message_bus_create(&alloc, &no_limit_config, &bus2));
  CMP_TEST_EXPECT(cmp_actor_destroy(bus2, actor), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_message_bus_destroy(&alloc, bus2));

  /* Destroy actor */
  CMP_TEST_OK(cmp_actor_destroy(bus, actor));

  /* Spawn actor without restart */
  a_cfg.handler = actor_handler_no_restart;
  a_cfg.restart_on_crash = 0;
  CMP_TEST_OK(cmp_actor_spawn(bus, &a_cfg, &actor));
  CMP_TEST_OK(cmp_message_bus_publish(bus, &crash_msg));
  CMP_TEST_EXPECT(cmp_actor_get_state(actor, &st) == 0, 1);
  CMP_TEST_EXPECT(st, CMP_ACTOR_STATE_CRASHED);

  /* Message should be ignored if not running */
  test_handler_called = 0;
  CMP_TEST_OK(cmp_message_bus_publish(bus, &msg));
  CMP_TEST_EXPECT(test_handler_called, 0);

  CMP_TEST_OK(cmp_actor_destroy(bus, actor));

  /* Destroy bus */
  CMP_TEST_OK(cmp_message_bus_destroy(&alloc, bus));

  return 0;
}

int main(void) {
  if (test_message_bus() != 0)
    return 1;
  return 0;
}
