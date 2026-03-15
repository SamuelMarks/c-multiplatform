#ifndef CMP_MESSAGE_H
#define CMP_MESSAGE_H

/**
 * @file cmp_message.h
 * @brief Unified messaging bus and actor primitives for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
/* clang-format on */

/**
 * @brief Message type identifier.
 */
typedef cmp_u32 CMPMessageType;

/**
 * @brief Message topic/channel identifier.
 */
typedef cmp_u32 CMPMessageTopic;

/**
 * @brief Base message structure. All custom messages must start with this.
 *
 * Memory Ownership Semantics:
 * - Within a single process/thread (synchronous publish): The publisher owns
 *   the message and its payload. Handlers MUST NOT modify or free the message
 *   or its payload. If a handler needs to keep the data, it MUST copy it.
 * - Across processes (IPC): The framework copies the message and payload.
 *   The receiving framework allocates the payload (using the provided
 * allocator), and the receiving user code MUST free the payload after handling
 * it.
 */
typedef struct CMPMessage {
  CMPMessageType type;    /**< Identifier for the payload type. */
  CMPMessageTopic topic;  /**< Destination topic. */
  void *payload;          /**< Pointer to payload data, or NULL. */
  cmp_usize payload_size; /**< Size of the payload data. */
  void *sender;           /**< Identifier of the sender (e.g., actor ID). */
} CMPMessage;

/**
 * @brief Function signature for a message handler callback.
 * @param ctx User-defined context.
 * @param msg The message received.
 * @return CMP_OK on success or an error code.
 */
typedef int(CMP_CALL *CMPMessageHandler)(void *ctx, const CMPMessage *msg);

/**
 * @brief Opaque handle for a message subscription.
 */
typedef void *CMPMessageSubscription;

/**
 * @brief Opaque handle for a message bus.
 */
typedef struct CMPMessageBus CMPMessageBus;

/**
 * @brief Configuration for a message bus.
 */
typedef struct CMPMessageBusConfig {
  cmp_usize max_subscribers; /**< Maximum number of allowed subscribers. */
} CMPMessageBusConfig;

/**
 * @brief Initialize a new message bus.
 * @param alloc Allocator to use for internal state.
 * @param config Configuration parameters.
 * @param out_bus Output message bus handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_message_bus_create(CMPAllocator *alloc,
                                            const CMPMessageBusConfig *config,
                                            CMPMessageBus **out_bus);

/**
 * @brief Destroy a message bus.
 * @param alloc Allocator used during creation.
 * @param bus The message bus to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_message_bus_destroy(CMPAllocator *alloc,
                                             CMPMessageBus *bus);

/**
 * @brief Subscribe to a specific message topic on the bus.
 * @param bus The message bus.
 * @param topic The topic to subscribe to.
 * @param handler The callback function to invoke when a message arrives.
 * @param ctx User context to pass to the callback.
 * @param out_sub Output subscription handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_message_bus_subscribe(CMPMessageBus *bus,
                                               CMPMessageTopic topic,
                                               CMPMessageHandler handler,
                                               void *ctx,
                                               CMPMessageSubscription *out_sub);

/**
 * @brief Unsubscribe from a message topic.
 * @param bus The message bus.
 * @param sub The subscription handle to remove.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_message_bus_unsubscribe(CMPMessageBus *bus,
                                                 CMPMessageSubscription sub);

/**
 * @brief Publish a message to a topic synchronously.
 * All subscribers on the current process/thread will be called immediately.
 * @param bus The message bus.
 * @param msg The message to publish.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_message_bus_publish(CMPMessageBus *bus,
                                             const CMPMessage *msg);

/**
 * @brief Opaque handle for an actor.
 */
typedef struct CMPActor CMPActor;

/**
 * @brief Represents different actor states in the supervision tree.
 */
typedef enum CMPActorState {
  CMP_ACTOR_STATE_RUNNING, /**< Actor is alive and handling messages. */
  CMP_ACTOR_STATE_STOPPED, /**< Actor has stopped normally. */
  CMP_ACTOR_STATE_CRASHED  /**< Actor returned an error and crashed. */
} CMPActorState;

/**
 * @brief Signature for an actor's message handling routine.
 * @param actor The actor receiving the message.
 * @param msg The message received.
 * @return CMP_OK on success or an error code.
 */
typedef int(CMP_CALL *CMPActorHandler)(CMPActor *actor, const CMPMessage *msg);

/**
 * @brief Configuration for spawning an actor.
 */
typedef struct CMPActorConfig {
  CMPActorHandler handler;  /**< Message handler for the actor. */
  void *ctx;                /**< Initial state/context for the actor. */
  CMPMessageTopic topic;    /**< Topic the actor listens to. */
  CMPBool restart_on_crash; /**< Supervisor flag: automatically restart if it
                               crashes. */
} CMPActorConfig;

/**
 * @brief Spawn a new actor on the message bus.
 * @param bus The message bus.
 * @param config Configuration for the actor.
 * @param out_actor Output actor handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_actor_spawn(CMPMessageBus *bus,
                                     const CMPActorConfig *config,
                                     CMPActor **out_actor);

/**
 * @brief Get the user context associated with an actor.
 * @param actor The actor.
 * @return Pointer to the context, or NULL.
 */
CMP_API int CMP_CALL cmp_actor_get_context(CMPActor *actor, void **out_context);

/**
 * @brief Get the current state of the actor.
 * @param actor The actor.
 * @return The state of the actor.
 */
CMP_API int CMP_CALL cmp_actor_get_state(CMPActor *actor,
                                         CMPActorState *out_state);

/**
 * @brief Destroy an actor and unregister it from the bus.
 * @param bus The message bus.
 * @param actor The actor to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_actor_destroy(CMPMessageBus *bus, CMPActor *actor);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_MESSAGE_H */