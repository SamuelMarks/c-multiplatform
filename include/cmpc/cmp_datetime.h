#ifndef CMP_DATETIME_H
#define CMP_DATETIME_H

/**
 * @file cmp_datetime.h
 * @brief Date/time primitives shared across LibCMPC components.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

/** @brief Minimum supported year for dates. */
#define CMP_DATE_MIN_YEAR 0
/** @brief Maximum supported year for dates. */
#define CMP_DATE_MAX_YEAR 10000

/**
 * @brief Calendar date descriptor.
 */
typedef struct CMPDate {
  cmp_i32 year;  /**< Year component (CMP_DATE_MIN_YEAR..CMP_DATE_MAX_YEAR). */
  cmp_u32 month; /**< Month component (1..12). */
  cmp_u32 day;   /**< Day component (1..31). */
} CMPDate;

/**
 * @brief Time descriptor.
 */
typedef struct CMPTime {
  cmp_u32 hour;   /**< Hour component (0..23). */
  cmp_u32 minute; /**< Minute component (0..59). */
} CMPTime;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_DATETIME_H */
