#ifndef M3_DATE_PICKER_H
#define M3_DATE_PICKER_H

/**
 * @file m3_date_picker.h
 * @brief Date picker widget for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"
#include "m3_layout.h"

/** @brief Minimum supported year for dates. */
#define M3_DATE_MIN_YEAR 0
/** @brief Maximum supported year for dates. */
#define M3_DATE_MAX_YEAR 10000

/** @brief Sunday weekday index. */
#define M3_DATE_PICKER_WEEKDAY_SUNDAY 0u
/** @brief Monday weekday index. */
#define M3_DATE_PICKER_WEEKDAY_MONDAY 1u
/** @brief Tuesday weekday index. */
#define M3_DATE_PICKER_WEEKDAY_TUESDAY 2u
/** @brief Wednesday weekday index. */
#define M3_DATE_PICKER_WEEKDAY_WEDNESDAY 3u
/** @brief Thursday weekday index. */
#define M3_DATE_PICKER_WEEKDAY_THURSDAY 4u
/** @brief Friday weekday index. */
#define M3_DATE_PICKER_WEEKDAY_FRIDAY 5u
/** @brief Saturday weekday index. */
#define M3_DATE_PICKER_WEEKDAY_SATURDAY 6u

/** @brief Week starts on Sunday. */
#define M3_DATE_PICKER_WEEK_START_SUNDAY M3_DATE_PICKER_WEEKDAY_SUNDAY
/** @brief Week starts on Monday. */
#define M3_DATE_PICKER_WEEK_START_MONDAY M3_DATE_PICKER_WEEKDAY_MONDAY

/** @brief Single-date selection mode. */
#define M3_DATE_PICKER_MODE_SINGLE 0u
/** @brief Range selection mode. */
#define M3_DATE_PICKER_MODE_RANGE 1u

/** @brief Date picker grid row count. */
#define M3_DATE_PICKER_GRID_ROWS 6u
/** @brief Date picker grid column count. */
#define M3_DATE_PICKER_GRID_COLS 7u
/** @brief Date picker total cell count. */
#define M3_DATE_PICKER_GRID_COUNT                                              \
  (M3_DATE_PICKER_GRID_ROWS * M3_DATE_PICKER_GRID_COLS)

/** @brief Invalid cell index sentinel. */
#define M3_DATE_PICKER_INVALID_INDEX ((m3_usize) ~(m3_usize)0)

/** @brief Default cell width in pixels. */
#define M3_DATE_PICKER_DEFAULT_CELL_WIDTH 40.0f
/** @brief Default cell height in pixels. */
#define M3_DATE_PICKER_DEFAULT_CELL_HEIGHT 40.0f
/** @brief Default horizontal cell spacing in pixels. */
#define M3_DATE_PICKER_DEFAULT_CELL_SPACING_X 4.0f
/** @brief Default vertical cell spacing in pixels. */
#define M3_DATE_PICKER_DEFAULT_CELL_SPACING_Y 4.0f
/** @brief Default header height in pixels. */
#define M3_DATE_PICKER_DEFAULT_HEADER_HEIGHT 40.0f
/** @brief Default weekday row height in pixels. */
#define M3_DATE_PICKER_DEFAULT_WEEKDAY_HEIGHT 20.0f
/** @brief Default padding in pixels. */
#define M3_DATE_PICKER_DEFAULT_PADDING 8.0f
/** @brief Default cell corner radius in pixels. */
#define M3_DATE_PICKER_DEFAULT_CELL_CORNER_RADIUS 4.0f

/** @brief Cell belongs to the current month. */
#define M3_DATE_CELL_FLAG_CURRENT_MONTH 0x01u
/** @brief Cell belongs to an adjacent month. */
#define M3_DATE_CELL_FLAG_OTHER_MONTH 0x02u
/** @brief Cell date lies outside the allowed range. */
#define M3_DATE_CELL_FLAG_OUT_OF_RANGE 0x04u
/** @brief Cell date is selected. */
#define M3_DATE_CELL_FLAG_SELECTED 0x08u
/** @brief Cell date is the start of a selected range. */
#define M3_DATE_CELL_FLAG_RANGE_START 0x10u
/** @brief Cell date is the end of a selected range. */
#define M3_DATE_CELL_FLAG_RANGE_END 0x20u
/** @brief Cell date lies inside a selected range. */
#define M3_DATE_CELL_FLAG_IN_RANGE 0x40u

/**
 * @brief Calendar date descriptor.
 */
typedef struct M3Date {
  m3_i32 year;  /**< Year component (M3_DATE_MIN_YEAR..M3_DATE_MAX_YEAR). */
  m3_u32 month; /**< Month component (1..12). */
  m3_u32 day;   /**< Day component (1..31). */
} M3Date;

/**
 * @brief Date range descriptor.
 */
typedef struct M3DateRange {
  M3Date start;     /**< Range start date. */
  M3Date end;       /**< Range end date. */
  M3Bool has_start; /**< M3_TRUE when start is set. */
  M3Bool has_end;   /**< M3_TRUE when end is set. */
} M3DateRange;

/**
 * @brief Date picker cell descriptor.
 */
typedef struct M3DatePickerCell {
  M3Date date;   /**< Cell date. */
  m3_u32 flags;  /**< Cell flags (M3_DATE_CELL_FLAG_*). */
  M3Rect bounds; /**< Cell bounds in pixels. */
} M3DatePickerCell;

/**
 * @brief Date picker style descriptor.
 */
typedef struct M3DatePickerStyle {
  M3LayoutEdges padding;       /**< Padding around the picker grid. */
  M3Scalar cell_width;         /**< Cell width in pixels (> 0). */
  M3Scalar cell_height;        /**< Cell height in pixels (> 0). */
  M3Scalar cell_spacing_x;     /**< Horizontal spacing between cells (>= 0). */
  M3Scalar cell_spacing_y;     /**< Vertical spacing between cells (>= 0). */
  M3Scalar header_height;      /**< Header height in pixels (>= 0). */
  M3Scalar weekday_height;     /**< Weekday row height in pixels (>= 0). */
  M3Scalar cell_corner_radius; /**< Cell corner radius in pixels (>= 0). */
  M3Color background_color;    /**< Background color. */
  M3Color cell_color;          /**< Default cell color. */
  M3Color cell_range_color;    /**< In-range cell color. */
  M3Color cell_selected_color; /**< Selected cell color. */
  M3Color cell_disabled_color; /**< Disabled/out-of-range cell color. */
} M3DatePickerStyle;

struct M3DatePicker;

/**
 * @brief Date picker change callback signature.
 * @param ctx User callback context pointer.
 * @param picker Date picker instance that changed.
 * @param range Current selection range.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3DatePickerOnChange)(void *ctx,
                                           struct M3DatePicker *picker,
                                           const M3DateRange *range);

/**
 * @brief Date picker widget instance.
 */
typedef struct M3DatePicker {
  M3Widget widget;         /**< Widget interface (points to this instance). */
  M3DatePickerStyle style; /**< Current style. */
  M3Rect bounds;           /**< Layout bounds. */
  m3_i32 display_year;     /**< Displayed year. */
  m3_u32 display_month;    /**< Displayed month (1..12). */
  m3_u32 week_start;       /**< Week start weekday (0..6). */
  m3_u32 mode;             /**< Selection mode (M3_DATE_PICKER_MODE_*). */
  M3DateRange selection;   /**< Current selection range. */
  M3DateRange constraints; /**< Allowed date range constraints. */
  M3DatePickerCell cells[M3_DATE_PICKER_GRID_COUNT]; /**< Cached cells. */
  m3_usize
      pressed_index; /**< Pressed cell index or M3_DATE_PICKER_INVALID_INDEX. */
  M3DatePickerOnChange on_change; /**< Change callback (may be NULL). */
  void *on_change_ctx;            /**< Change callback context pointer. */
} M3DatePicker;

/**
 * @brief Initialize a date picker style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_style_init(M3DatePickerStyle *style);

/**
 * @brief Initialize a date picker widget.
 * @param picker Date picker instance.
 * @param style Style descriptor.
 * @param year Display year.
 * @param month Display month.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_init(M3DatePicker *picker,
                                       const M3DatePickerStyle *style,
                                       m3_i32 year, m3_u32 month);

/**
 * @brief Update the date picker style.
 * @param picker Date picker instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_set_style(M3DatePicker *picker,
                                            const M3DatePickerStyle *style);

/**
 * @brief Update the selection mode.
 * @param picker Date picker instance.
 * @param mode Selection mode (M3_DATE_PICKER_MODE_*).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_set_mode(M3DatePicker *picker, m3_u32 mode);

/**
 * @brief Update the week start.
 * @param picker Date picker instance.
 * @param week_start Week start weekday (0..6).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_set_week_start(M3DatePicker *picker,
                                                 m3_u32 week_start);

/**
 * @brief Update the displayed month.
 * @param picker Date picker instance.
 * @param year Display year.
 * @param month Display month.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_set_display_month(M3DatePicker *picker,
                                                    m3_i32 year, m3_u32 month);

/**
 * @brief Retrieve the displayed month.
 * @param picker Date picker instance.
 * @param out_year Receives the display year.
 * @param out_month Receives the display month.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_get_display_month(const M3DatePicker *picker,
                                                    m3_i32 *out_year,
                                                    m3_u32 *out_month);

/**
 * @brief Advance the displayed month by one.
 * @param picker Date picker instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_next_month(M3DatePicker *picker);

/**
 * @brief Move the displayed month back by one.
 * @param picker Date picker instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_prev_month(M3DatePicker *picker);

/**
 * @brief Update date range constraints.
 * @param picker Date picker instance.
 * @param constraints Constraints range descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_set_constraints(
    M3DatePicker *picker, const M3DateRange *constraints);

/**
 * @brief Retrieve date range constraints.
 * @param picker Date picker instance.
 * @param out_constraints Receives constraints.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_get_constraints(const M3DatePicker *picker,
                                                  M3DateRange *out_constraints);

/**
 * @brief Update the selected date range.
 * @param picker Date picker instance.
 * @param selection Selection range descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_set_selection(M3DatePicker *picker,
                                                const M3DateRange *selection);

/**
 * @brief Retrieve the selected date range.
 * @param picker Date picker instance.
 * @param out_selection Receives the selection range.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_get_selection(const M3DatePicker *picker,
                                                M3DateRange *out_selection);

/**
 * @brief Assign a change callback.
 * @param picker Date picker instance.
 * @param on_change Change callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_set_on_change(M3DatePicker *picker,
                                                M3DatePickerOnChange on_change,
                                                void *ctx);

/**
 * @brief Recompute cached grid cells.
 * @param picker Date picker instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_update(M3DatePicker *picker);

/**
 * @brief Retrieve cached grid cells.
 * @param picker Date picker instance.
 * @param out_cells Receives the cell array pointer.
 * @param out_count Receives the number of cells.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_get_cells(const M3DatePicker *picker,
                                            const M3DatePickerCell **out_cells,
                                            m3_usize *out_count);

/**
 * @brief Retrieve a cell by index.
 * @param picker Date picker instance.
 * @param index Cell index (0..M3_DATE_PICKER_GRID_COUNT-1).
 * @param out_cell Receives the cell descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_get_cell(const M3DatePicker *picker,
                                           m3_usize index,
                                           M3DatePickerCell *out_cell);

#ifdef M3_TESTING
/**
 * @brief Test hook for validating colors.
 * @param color Color to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_validate_color(const M3Color *color);
/**
 * @brief Test hook for validating layout edges.
 * @param edges Edges to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_date_picker_test_validate_edges(const M3LayoutEdges *edges);
/**
 * @brief Test hook for validating styles.
 * @param style Style to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_date_picker_test_validate_style(const M3DatePickerStyle *style);
/**
 * @brief Test hook for validating measure specs.
 * @param spec Measure spec to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_date_picker_test_validate_measure_spec(M3MeasureSpec spec);
/**
 * @brief Test hook for validating rectangles.
 * @param rect Rectangle to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_validate_rect(const M3Rect *rect);
/**
 * @brief Test hook for validating dates.
 * @param date Date to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_validate_date(const M3Date *date);
/**
 * @brief Test hook for leap year checks.
 * @param year Year to test.
 * @param out_leap Receives M3_TRUE if leap year.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_is_leap_year(m3_i32 year,
                                                    M3Bool *out_leap);
/**
 * @brief Test hook for days-in-month calculations.
 * @param year Year to test.
 * @param month Month to test.
 * @param out_days Receives the number of days.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_days_in_month(m3_i32 year, m3_u32 month,
                                                     m3_u32 *out_days);
/**
 * @brief Test hook for day-of-week calculations.
 * @param date Date to test.
 * @param out_weekday Receives weekday index (0..6).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_day_of_week(const M3Date *date,
                                                   m3_u32 *out_weekday);
/**
 * @brief Test hook for comparing two dates.
 * @param a First date.
 * @param b Second date.
 * @param out_cmp Receives comparison (-1,0,1).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_compare_dates(const M3Date *a,
                                                     const M3Date *b,
                                                     m3_i32 *out_cmp);
/**
 * @brief Test hook for checking whether a date is in range.
 * @param date Date to test.
 * @param range Range descriptor.
 * @param out_in_range Receives M3_TRUE if the date is in range.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_date_in_range(const M3Date *date,
                                                     const M3DateRange *range,
                                                     M3Bool *out_in_range);
/**
 * @brief Test hook for computing weekday offsets.
 * @param weekday Weekday index (0..6).
 * @param week_start Week start weekday (0..6).
 * @param out_offset Receives the offset in days.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_compute_offset(m3_u32 weekday,
                                                      m3_u32 week_start,
                                                      m3_u32 *out_offset);
/**
 * @brief Test hook for shifting months.
 * @param year Input year.
 * @param month Input month.
 * @param delta Month delta (negative or positive).
 * @param out_year Receives the shifted year.
 * @param out_month Receives the shifted month.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_shift_month(m3_i32 year, m3_u32 month,
                                                   m3_i32 delta,
                                                   m3_i32 *out_year,
                                                   m3_u32 *out_month);
/**
 * @brief Test hook for computing cell bounds.
 * @param picker Date picker instance.
 * @param row Cell row index.
 * @param col Cell column index.
 * @param out_bounds Receives the cell bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_compute_cell_bounds(
    const M3DatePicker *picker, m3_u32 row, m3_u32 col, M3Rect *out_bounds);
/**
 * @brief Test hook for hit-testing a point against the grid.
 * @param picker Date picker instance.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 * @param out_index Receives the hit cell index or M3_DATE_PICKER_INVALID_INDEX.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_hit_test(const M3DatePicker *picker,
                                                M3Scalar x, M3Scalar y,
                                                m3_usize *out_index);
/**
 * @brief Test hook for updating cached grid cells.
 * @param picker Date picker instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_date_picker_test_update_grid(M3DatePicker *picker);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_DATE_PICKER_H */
