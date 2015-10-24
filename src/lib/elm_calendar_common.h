/**
 * @addtogroup Calendar
 *
 * @{
 */
/**
 * @enum Elm_Calendar_Mark_Repeat_Type
 * Event periodicity, used to define if a mark should be repeated
 * @b beyond event's day. It's set when a mark is added.
 *
 * So, for a mark added to 13th May with periodicity set to WEEKLY,
 * there will be marks every week after this date. Marks will be displayed
 * at 13th, 20th, 27th, 3rd June ...
 *
 * Values don't work as bitmask, only one can be chosen.
 *
 * @see elm_calendar_mark_add()
 */
typedef enum
{
   ELM_CALENDAR_UNIQUE, /**< Default value. Marks will be displayed only on event day. */
   ELM_CALENDAR_DAILY, /**< Marks will be displayed every day after event day (inclusive). */
   ELM_CALENDAR_WEEKLY, /**< Marks will be displayed every week after event day (inclusive) - i.e. each seven days. */
   ELM_CALENDAR_MONTHLY, /**< Marks will be displayed every month day that coincides to event day. E.g.: if an event is set to 30th Jan, no marks will be displayed on Feb, but will be displayed on 30th Mar*/
   ELM_CALENDAR_ANNUALLY, /**< Marks will be displayed every year that coincides to event day (and month). E.g. an event added to 30th Jan 2012 will be repeated on 30th Jan 2013. */
   ELM_CALENDAR_LAST_DAY_OF_MONTH /**< Marks will be displayed every last day of month after event day (inclusive).  @since 1.7 */
} Elm_Calendar_Mark_Repeat_Type;

/**
 * @enum Elm_Calendar_Weekday
 * A weekday
 *
 * @see elm_calendar_first_day_of_week_set()
 */
typedef enum
{
   ELM_DAY_SUNDAY,
   ELM_DAY_MONDAY,
   ELM_DAY_TUESDAY,
   ELM_DAY_WEDNESDAY,
   ELM_DAY_THURSDAY,
   ELM_DAY_FRIDAY,
   ELM_DAY_SATURDAY,
   ELM_DAY_LAST
} Elm_Calendar_Weekday;

/**
 * @enum Elm_Calendar_Select_Mode
 *
 * The mode, who determine how user could select a day
 *
 * @see elm_calendar_select_mode_set()
 */
typedef enum
{
   ELM_CALENDAR_SELECT_MODE_DEFAULT = 0, /**< Default value. a day is always selected. */
   ELM_CALENDAR_SELECT_MODE_ALWAYS, /**< a day is always selected. */
   ELM_CALENDAR_SELECT_MODE_NONE, /**< None of the days can be selected. */
   ELM_CALENDAR_SELECT_MODE_ONDEMAND /**< User may have selected a day or not. */
} Elm_Calendar_Select_Mode;

/**
 * @Elm_Calendar_Selectable
 *
 * A bitmask used to define which fields of a @b tm struct will be taken into
 * account, when elm_calendar_selected_time_set() is invoked.
 *
 * @see elm_calendar_selectable_set()
 * @see elm_calendar_selected_time_set()
 * @since 1.8
 */
typedef enum
{
   ELM_CALENDAR_SELECTABLE_NONE = 0,
   ELM_CALENDAR_SELECTABLE_YEAR = (1 << 0),
   ELM_CALENDAR_SELECTABLE_MONTH = (1 << 1),
   ELM_CALENDAR_SELECTABLE_DAY = (1 << 2)
} Elm_Calendar_Selectable;

typedef struct _Elm_Calendar_Mark Elm_Calendar_Mark;    /**< Item handle for a calendar mark. Created with elm_calendar_mark_add() and deleted with elm_calendar_mark_del(). */

/**
 * This callback type is used to format the string that will be used
 * to display month and year.
 *
 * @param stime Struct representing time.
 * @return String representing time that will be set to calendar's text.
 *
 * @see elm_calendar_format_function_set()
 */
typedef char * (*Elm_Calendar_Format_Cb)(struct tm *stime);

/**
 * @MOBILE_ONLY
 *
 * @brief Delete mark from the calendar.
 *
 * @param mark The mark to be deleted.
 *
 * @remark If deleting all calendar marks is required, elm_calendar_marks_clear()
 * should be used instead of getting marks list and deleting each one.
 *
 * @see elm_calendar_mark_add()
 *
 * @if MOBILE @since_tizen 2.4
 * @endif
 */
EAPI void                 elm_calendar_mark_del(Elm_Calendar_Mark *mark);

/**
 * @}
 */
