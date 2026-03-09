#include "cmpc/cmp_message.h"

typedef struct CMPSubscriptionNode {
  CMPMessageTopic topic;
  CMPMessageHandler handler;
  void *ctx;
  struct CMPSubscriptionNode *next;
} CMPSubscriptionNode;

struct CMPMessageBus {
  CMPSubscriptionNode *subscriptions;
  CMPAllocator allocator;
  cmp_usize max_subscribers;
  cmp_usize current_subscribers;
};

CMP_API int CMP_CALL cmp_message_bus_create(CMPAllocator *alloc,
                                            const CMPMessageBusConfig *config,
                                            CMPMessageBus **out_bus) {
  CMPMessageBus *bus;
  int rc;

  if (alloc == NULL || alloc->alloc == NULL || out_bus == NULL ||
      config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = alloc->alloc(alloc->ctx, sizeof(CMPMessageBus), (void **)&bus);
  if (rc != CMP_OK || bus == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  bus->allocator = *alloc;
  bus->max_subscribers = config->max_subscribers;
  bus->current_subscribers = 0;
  bus->subscriptions = NULL;

  *out_bus = bus;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_message_bus_destroy(CMPAllocator *alloc,
                                             CMPMessageBus *bus) {
  CMPSubscriptionNode *current;
  CMPSubscriptionNode *next;

  if (alloc == NULL || alloc->free == NULL || bus == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  current = bus->subscriptions;
  while (current != NULL) {
    next = current->next;
    alloc->free(alloc->ctx, current);
    current = next;
  }

  alloc->free(alloc->ctx, bus);
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_message_bus_subscribe(
    CMPMessageBus *bus, CMPMessageTopic topic, CMPMessageHandler handler,
    void *ctx, CMPMessageSubscription *out_sub) {
  CMPSubscriptionNode *node;
  int rc;

  if (bus == NULL || handler == NULL || out_sub == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (bus->max_subscribers > 0 &&
      bus->current_subscribers >= bus->max_subscribers) {
    return CMP_ERR_RANGE;
  }

  rc = bus->allocator.alloc(bus->allocator.ctx, sizeof(CMPSubscriptionNode),
                            (void **)&node);
  if (rc != CMP_OK || node == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  node->topic = topic;
  node->handler = handler;
  node->ctx = ctx;
  node->next = bus->subscriptions;
  bus->subscriptions = node;
  bus->current_subscribers++;

  *out_sub = (CMPMessageSubscription)node;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_message_bus_unsubscribe(CMPMessageBus *bus,
                                                 CMPMessageSubscription sub) {
  CMPSubscriptionNode *current;
  CMPSubscriptionNode *prev;
  CMPSubscriptionNode *target;

  if (bus == NULL || sub == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  target = (CMPSubscriptionNode *)sub;
  prev = NULL;
  current = bus->subscriptions;

  while (current != NULL) {
    if (current == target) {
      if (prev == NULL) {
        bus->subscriptions = current->next;
      } else {
        prev->next = current->next;
      }
      bus->allocator.free(bus->allocator.ctx, current);
      if (bus->current_subscribers > 0) {
        bus->current_subscribers--;
      }
      return CMP_OK;
    }
    prev = current;
    current = current->next;
  }

  return CMP_ERR_NOT_FOUND;
}

CMP_API int CMP_CALL cmp_message_bus_publish(CMPMessageBus *bus,
                                             const CMPMessage *msg) {
  CMPSubscriptionNode *current;

  if (bus == NULL || msg == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  current = bus->subscriptions;
  while (current != NULL) {
    if (current->topic == msg->topic) {
      /* Synchronous publish ignores individual handler return codes to ensure
       * all get called. */
      current->handler(current->ctx, msg);
    }
    current = current->next;
  }

  return CMP_OK;
}

struct CMPActor {
  CMPMessageBus *bus;
  CMPActorHandler handler;
  void *ctx;
  CMPMessageTopic topic;
  CMPMessageSubscription subscription;
  CMPActorState state;
  CMPBool restart_on_crash;
};

static int CMP_CALL actor_message_proxy(void *ctx, const CMPMessage *msg) {
  CMPActor *actor = (CMPActor *)ctx;
  int rc;

  if (actor == NULL || actor->handler == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (actor->state != CMP_ACTOR_STATE_RUNNING) {
    return CMP_OK; /* Ignore messages if not running */
  }

  rc = actor->handler(actor, msg);
  if (rc != CMP_OK) {
    /* Actor crashed */
    actor->state = CMP_ACTOR_STATE_CRASHED;
    if (actor->restart_on_crash) {
      /* Basic supervision: automatically restart */
      actor->state = CMP_ACTOR_STATE_RUNNING;
    }
  }

  return rc;
}

CMP_API int CMP_CALL cmp_actor_spawn(CMPMessageBus *bus,
                                     const CMPActorConfig *config,
                                     CMPActor **out_actor) {
  CMPActor *actor;
  int rc;

  if (bus == NULL || config == NULL || config->handler == NULL ||
      out_actor == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = bus->allocator.alloc(bus->allocator.ctx, sizeof(CMPActor),
                            (void **)&actor);
  if (rc != CMP_OK || actor == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  actor->bus = bus;
  actor->handler = config->handler;
  actor->ctx = config->ctx;
  actor->topic = config->topic;
  actor->state = CMP_ACTOR_STATE_RUNNING;
  actor->restart_on_crash = config->restart_on_crash;

  rc = cmp_message_bus_subscribe(bus, config->topic, actor_message_proxy, actor,
                                 &actor->subscription);
  if (rc != CMP_OK) {
    bus->allocator.free(bus->allocator.ctx, actor);
    return rc;
  }

  *out_actor = actor;
  return CMP_OK;
}

CMP_API void *CMP_CALL cmp_actor_get_context(CMPActor *actor) {
  if (actor == NULL) {
    return NULL;
  }
  return actor->ctx;
}

CMP_API CMPActorState CMP_CALL cmp_actor_get_state(CMPActor *actor) {
  if (actor == NULL) {
    return CMP_ACTOR_STATE_STOPPED;
  }
  return actor->state;
}

CMP_API int CMP_CALL cmp_actor_destroy(CMPMessageBus *bus, CMPActor *actor) {
  int rc;

  if (bus == NULL || actor == NULL || actor->bus != bus) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  actor->state = CMP_ACTOR_STATE_STOPPED;
  rc = cmp_message_bus_unsubscribe(bus, actor->subscription);
  bus->allocator.free(bus->allocator.ctx, actor);

  return rc;
}