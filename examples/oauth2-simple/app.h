#ifndef OAUTH2_SIMPLE_APP_H
#define OAUTH2_SIMPLE_APP_H

/**
 * @file app.h
 * @brief OAuth2 Simple Example Application via C-Multiplatform.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize the application systems.
 * @return 0 on success, error code otherwise.
 */
int app_init(void);

/**
 * @brief Execute the main application loop.
 * @return 0 on successful exit.
 */
int app_run(void);

/**
 * @brief Shutdown systems gracefully.
 * @return 0 on success.
 */
int app_shutdown(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OAUTH2_SIMPLE_APP_H */
