/**
 * @file ui_live_announcer.h
 * @brief Accessibility utility for injecting dynamic speech into screen
 * readers.
 */

#ifndef UI_LIVE_ANNOUNCER_H
#define UI_LIVE_ANNOUNCER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

struct ui_live_announcer;

/**
 * @brief Politeness level for a live announcement.
 */
enum ui_live_politeness {
  UI_LIVE_POLITE = 0,   /**< Announce when the screen reader is idle. */
  UI_LIVE_ASSERTIVE = 1 /**< Announce immediately, interrupting other speech. */
};

/**
 * @brief Creates a new live announcer manager.
 *
 * @param out_announcer Pointer to receive the allocated live announcer.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_live_announcer_create(struct ui_live_announcer **out_announcer);

/**
 * @brief Destroys a live announcer manager.
 *
 * @param announcer The live announcer manager to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_live_announcer_destroy(struct ui_live_announcer *announcer);

/**
 * @brief Queues text to be announced by the screen reader.
 *
 * @param announcer The live announcer manager.
 * @param message The text to announce.
 * @param politeness The politeness level (polite or assertive).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_live_announce(struct ui_live_announcer *announcer,
                            const char *message,
                            enum ui_live_politeness politeness);

/**
 * @brief Clears any pending announcements in the queue.
 *
 * @param announcer The live announcer manager.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_live_announcer_clear(struct ui_live_announcer *announcer);

#ifdef __cplusplus
}
#endif

#endif /* UI_LIVE_ANNOUNCER_H */
