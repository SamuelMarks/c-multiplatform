#ifndef CMP_DB_H
#define CMP_DB_H

/**
 * @file cmp_db.h
 * @brief Database operations for LibCMPC using c-orm.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_core.h"
/* clang-format on */

/**
 * @brief Database connection handle structure.
 */
typedef struct CMPDb {
  void *handle;           /**< Internal c-orm database handle (c_orm_db_t*). */
  CMPAllocator allocator; /**< Allocator for the structure. */
} CMPDb;

/**
 * @brief Database asynchronous execute callback signature.
 * @param ctx The context for the callback.
 * @param status CMP_OK on success or an error code.
 * @param user User context passed to cmp_db_execute_async.
 */
typedef void(CMP_CALL *CMPDbExecuteCallback)(void *ctx, int status, void *user);

/**
 * @brief Initialize a database connection synchronously.
 * @param db Database instance to initialize.
 * @param allocator Allocator for internal state.
 * @param url Connection string or path (e.g. ":memory:").
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_db_init(CMPDb *db, const CMPAllocator *allocator,
                                 const char *url);

/**
 * @brief Shut down and disconnect a database connection.
 * @param db Database instance.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_db_shutdown(CMPDb *db);

/**
 * @brief Execute a raw SQL query synchronously.
 * @param db Database instance.
 * @param sql Raw SQL query string.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_db_execute(CMPDb *db, const char *sql);

/**
 * @brief Execute a raw SQL query asynchronously.
 * @param db Database instance.
 * @param tasks Tasks interface for executing the query off the main thread.
 * @param loop Event loop for invoking the completion callback on the main
 * thread.
 * @param sql Raw SQL query string.
 * @param callback Callback to invoke on completion.
 * @param user User context to pass to the callback.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_db_execute_async(CMPDb *db, CMPTasks *tasks,
                                          CMPEventLoop *loop, const char *sql,
                                          CMPDbExecuteCallback callback,
                                          void *user);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_DB_H */
