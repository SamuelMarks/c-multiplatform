#ifndef UI_ASSET_STREAMER_H
#define UI_ASSET_STREAMER_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_promise.h"
#include "ui_thread_pool.h"
#include "ui_execution_context.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents the types of assets that can be streamed.
 */
enum ui_asset_type {
  UI_ASSET_TYPE_UNKNOWN = 0, /**< Unknown asset type */
  UI_ASSET_TYPE_IMAGE,       /**< Image asset */
  UI_ASSET_TYPE_FONT,        /**< Font asset */
  UI_ASSET_TYPE_VIDEO,       /**< Video asset */
  UI_ASSET_TYPE_AUDIO,       /**< Audio asset */
  UI_ASSET_TYPE_TEXT,        /**< Text asset */
  UI_ASSET_TYPE_BINARY       /**< Binary asset */
};

/**
 * @brief Represents a loaded asset.
 */
struct ui_asset {
  char *url; /**< The URL or path from which the asset was loaded */
  enum ui_asset_type type; /**< The type of the loaded asset */
  void *data;              /**< Pointer to the asset data */
  ui_uint32 size;          /**< Size of the asset data in bytes */
};

/**
 * @brief Opaque handle representing the asynchronous asset streamer.
 */
struct ui_asset_streamer;

/**
 * @brief Creates a new asset streamer.
 *
 * @param pool The thread pool to use for background loading.
 * @param ctx The execution context to use for resolving promises.
 * @param out_streamer Pointer to receive the new streamer handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_asset_streamer_create(struct ui_thread_pool *pool,
                                    struct ui_execution_context *ctx,
                                    struct ui_asset_streamer **out_streamer);

/**
 * @brief Destroys an asset streamer and frees its resources.
 *
 * @param streamer The streamer to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if streamer is
 * NULL.
 */
ui_error_t ui_asset_streamer_destroy(struct ui_asset_streamer *streamer);

/**
 * @brief Requests an asset to be loaded asynchronously.
 *
 * @param streamer The streamer.
 * @param url The URL or file path of the asset.
 * @param type The expected type of the asset.
 * @param out_promise Pointer to receive a promise that resolves with (struct
 * ui_asset*).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_asset_streamer_request(struct ui_asset_streamer *streamer,
                                     const char *url, enum ui_asset_type type,
                                     struct ui_promise **out_promise);

/**
 * @brief Frees a loaded asset.
 *
 * @param asset The asset to free.
 */
ui_error_t ui_asset_destroy(struct ui_asset *asset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ASSET_STREAMER_H */
