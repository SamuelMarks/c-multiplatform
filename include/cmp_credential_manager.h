#ifndef CMP_CREDENTIAL_MANAGER_H
#define CMP_CREDENTIAL_MANAGER_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_credential_manager.h
 * @brief OS Keychain / Credential Manager wrappers (DPAPI, Secret Service,
 * macOS Keychain).
 */

/**
 * @brief Opaque type for credential manager.
 */
typedef struct cmp_credential_manager cmp_credential_manager_t;

/**
 * @brief Creates a new credential manager instance.
 * @param out_manager Pointer to receive the created instance.
 * @return 0 on success, or an error code.
 */
int CMP_API
cmp_credential_manager_create(cmp_credential_manager_t **out_manager);

/**
 * @brief Destroys a credential manager instance.
 * @param manager The instance to destroy.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_credential_manager_destroy(cmp_credential_manager_t *manager);

/**
 * @brief Stores a secret securely in the OS credential manager.
 * @param manager The manager instance.
 * @param service The service or application name.
 * @param account The account or username.
 * @param secret The secret data.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_credential_manager_set_secret(cmp_credential_manager_t *manager,
                                              const char *service,
                                              const char *account,
                                              const char *secret);

/**
 * @brief Retrieves a secret securely from the OS credential manager.
 * @param manager The manager instance.
 * @param service The service or application name.
 * @param account The account or username.
 * @param out_secret Pointer to receive the secret data. Must be freed by
 * caller.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_credential_manager_get_secret(cmp_credential_manager_t *manager,
                                              const char *service,
                                              const char *account,
                                              char **out_secret);

/**
 * @brief Deletes a secret from the OS credential manager.
 * @param manager The manager instance.
 * @param service The service or application name.
 * @param account The account or username.
 * @return 0 on success, or an error code.
 */
int CMP_API
cmp_credential_manager_delete_secret(cmp_credential_manager_t *manager,
                                     const char *service, const char *account);

/**
 * @brief Frees a string retrieved from the credential manager.
 * @param secret The secret string to free.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_credential_manager_free_secret(char *secret);
#ifdef __cplusplus
}
#endif

#endif /* CMP_CREDENTIAL_MANAGER_H */
