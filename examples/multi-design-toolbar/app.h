#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initializes the application.
 * @return 0 on success, non-zero on failure.
 */
int app_init(void);

/**
 * @brief Runs the main application loop.
 * @return 0 on success, non-zero on failure.
 */
int app_run(void);

/**
 * @brief Shuts down the application and cleans up resources.
 * @return 0 on success, non-zero on failure.
 */
int app_shutdown(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APP_H */
