#include <Elementary.h>
#include "elm_priv.h"

#define DATETIME_FIELD_COUNT            6
#define BUFF_SIZE                       1024
#define MONTH_STRING_MAX_SIZE           32
#define TOTAL_NUMBER_OF_MONTHS          12
#define STRUCT_TM_YEAR_BASE_VALUE       1900
#define STRUCT_TM_TIME_12HRS_MAX_VALUE  12
#define STRUCT_TM_TIME_24HRS_MAX_VALUE  23

static const char *field_styles[] = {
                         "year", "month", "date", "hour", "minute", "ampm" };

static int days_in_month[TOTAL_NUMBER_OF_MONTHS] = {
                         31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static char month_arr[TOTAL_NUMBER_OF_MONTHS][MONTH_STRING_MAX_SIZE];

typedef struct _Popup_Module_Data Popup_Module_Data;

struct _Popup_Module_Data
{
   Elm_Datetime_Module_Data mod_data;
   Evas_Object *popup;
   Evas_Object *datepicker_layout, *timepicker_layout;
   Evas_Object *popup_field[DATETIME_FIELD_COUNT];
   double time_hours;
   Eina_Bool hour_incr, hour_decr;
   Eina_Bool time_24hr;
};

static void
_set_datepicker_value(Popup_Module_Data *popup_mod)
{
   struct tm set_time;

   if (!popup_mod) return;

   elm_datetime_value_get(popup_mod->mod_data.base, &set_time);
   set_time.tm_year = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_YEAR]) - STRUCT_TM_YEAR_BASE_VALUE;
   set_time.tm_mon = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_MONTH]) - 1;
   set_time.tm_mday = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_DATE]);
   elm_datetime_value_set(popup_mod->mod_data.base, &set_time);
}

static void
_set_timepicker_value(Popup_Module_Data *popup_mod)
{
   struct tm set_time;
   int is_pm;

   if (!popup_mod) return;

   elm_datetime_value_get(popup_mod->mod_data.base, &set_time);

   set_time.tm_hour = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_HOUR]);
   set_time.tm_min = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_MINUTE]);
   if (!popup_mod->time_24hr)
     {
        is_pm = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_AMPM]);
        if (is_pm) set_time.tm_hour += STRUCT_TM_TIME_12HRS_MAX_VALUE;
        set_time.tm_hour = (set_time.tm_hour == STRUCT_TM_TIME_12HRS_MAX_VALUE) ? 0 : set_time.tm_hour;
        set_time.tm_hour = (set_time.tm_hour == 24) ? STRUCT_TM_TIME_12HRS_MAX_VALUE : set_time.tm_hour;
     }
   elm_datetime_value_set(popup_mod->mod_data.base, &set_time);
}

static void
_popup_set_btn_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   Evas_Object *content;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   evas_object_hide(popup_mod->popup);
   content = elm_object_content_get(popup_mod->popup);
   if (content == popup_mod->datepicker_layout)
     _set_datepicker_value(popup_mod);
   else if (content == popup_mod->timepicker_layout)
     _set_timepicker_value(popup_mod);
}

static void
_popup_cancel_btn_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   evas_object_hide(popup_mod->popup);
}

static void
_datepicker_hide_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   evas_object_hide(popup_mod->popup);
}

static void
_timepicker_hide_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   evas_object_hide(popup_mod->popup);
}

static void
_hour_increment_start_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   popup_mod->hour_incr = EINA_TRUE;
}

static void
_hour_increment_stop_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   popup_mod->hour_incr = EINA_FALSE;
}

static void
_hour_decrement_start_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   popup_mod->hour_decr = EINA_TRUE;
}

static void
_hour_decrement_stop_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   popup_mod->hour_decr = EINA_FALSE;
}

static void
_entry_activated_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   Evas_Object *entry, *en;
   int idx;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   elm_object_focus_set(obj, EINA_FALSE);
   for (idx = 0; idx < ELM_DATETIME_DATE; idx++)
     {
        entry = elm_object_part_content_get(popup_mod->popup_field[idx], "elm.swallow.entry");
        if (obj == entry)
          {
             idx++;
             en = elm_object_part_content_get(popup_mod->popup_field[idx], "elm.swallow.entry");
             elm_layout_signal_emit(popup_mod->popup_field[idx], "elm,action,entry,toggle", "elm");
             elm_object_focus_set(en, EINA_TRUE);
             return;
          }
     }
   entry = elm_object_part_content_get(popup_mod->popup_field[ELM_DATETIME_HOUR], "elm.swallow.entry");
   if (obj == entry)
     {
        en = elm_object_part_content_get(popup_mod->popup_field[ELM_DATETIME_MINUTE], "elm.swallow.entry");
        elm_layout_signal_emit(popup_mod->popup_field[ELM_DATETIME_MINUTE],
                               "elm,action,entry,toggle", "elm");
        elm_object_focus_set(en, EINA_TRUE);
     }
}

static void
_entry_clicked_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod || !obj) return;
}

static void
_entry_focused_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   Evas_Object *entry;
   int idx, value;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   entry = elm_object_part_content_get(popup_mod->popup_field[ELM_DATETIME_MONTH],
                                       "elm.swallow.entry");
   if (obj == entry)
     {
        value = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_MONTH]) - 1;
        elm_object_text_set(obj, month_arr[value]);
     }
   for (idx = 0; idx < DATETIME_FIELD_COUNT -1; idx++)
     {
        entry = elm_object_part_content_get(popup_mod->popup_field[idx], "elm.swallow.entry");
        if ((obj != entry) && elm_object_focus_get(entry))
          {
             elm_object_focus_set(entry, EINA_FALSE);
             elm_entry_input_panel_hide(obj);
             return;
          }
     }

   elm_entry_select_all(obj);
}

static void
_entry_unfocused_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   // TODO: entry_unfocused code
}

static void
_adjust_max_days(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   int year, month, max_days;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   year = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_YEAR]);
   month = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_MONTH]) - 1;

   max_days = days_in_month[month];
  // if ((year % 4 == 0) && (year % 100 != 0 || year % 400 == 0)&& (month == 1))
   if (__isleap(year) && (month == 1))
     max_days += 1;

   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_DATE], 1, max_days);
}

static void
_adjust_ampm(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   int time_hours, ampm;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   if(popup_mod->time_24hr) return;

   time_hours = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_HOUR]);
   if (time_hours == 12 && (popup_mod->hour_incr || (popup_mod->time_hours == 11)))
     {
        ampm = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_AMPM]);
        ampm = !ampm;
        elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_AMPM], ampm);
     }
   else if (time_hours == 11 && (popup_mod->hour_decr || (popup_mod->time_hours == 12)))
     {
        ampm = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_AMPM]);
        ampm = !ampm;
        elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_AMPM], ampm);
     }
   popup_mod->time_hours = time_hours;
}

static void
_year_validity_checking_filter(void *data, Evas_Object *obj, char **text)
{
   Popup_Module_Data *popup_mod;
   Evas_Object *entry;
   char new_str[BUFF_SIZE] = {0,};
   int min, max, val = 0, len;
   char *insert;
   const char *curr_str;

   EINA_SAFETY_ON_NULL_RETURN(text);
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   insert = *text;
   len = strlen(elm_object_text_get(obj));
   if (len < 3) return;

   curr_str = elm_object_text_get(obj);
   if (curr_str) strncpy(new_str, curr_str, BUFF_SIZE);
   strncat(new_str, insert, 1);
   if (new_str[0]) val = atoi(new_str);

   popup_mod->mod_data.field_limit_get(popup_mod->mod_data.base, ELM_DATETIME_YEAR, &min, &max);
   min += STRUCT_TM_YEAR_BASE_VALUE;
   max += STRUCT_TM_YEAR_BASE_VALUE;

   if (val <= max)
     {
       elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_YEAR], val);
       elm_layout_signal_emit(popup_mod->popup_field[ELM_DATETIME_YEAR],
                              "elm,action,entry,toggle", "elm");
       elm_object_focus_set(obj, EINA_FALSE);

       entry = elm_object_part_content_get(popup_mod->popup_field[ELM_DATETIME_MONTH],
                                           "elm.swallow.entry");
       if (!elm_object_focus_get(entry))
         {
            elm_layout_signal_emit(popup_mod->popup_field[ELM_DATETIME_MONTH],
                                   "elm,action,entry,toggle", "elm");
            elm_object_focus_set(entry, EINA_TRUE);
         }
     }

   *insert = 0;
}

static void
_month_validity_checking_filter(void *data, Evas_Object *obj, char **text)
{
   Popup_Module_Data *popup_mod;
   Evas_Object *entry;
   char *insert;
   int min, max, val = 0;
   char new_str[BUFF_SIZE];

   EINA_SAFETY_ON_NULL_RETURN(text);
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   insert = *text;
   strncpy(new_str, insert, BUFF_SIZE);
   new_str[BUFF_SIZE - 1] = '\0';
   if (new_str[0]) val = atoi(new_str) - 1; //tm struct accepts month values from 0 while ISE output starts from 1.

   popup_mod->mod_data.field_limit_get(popup_mod->mod_data.base, ELM_DATETIME_MONTH, &min, &max);

   if ((val >= min) && (val <= max))
     {
       elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_MONTH], val+1);
       elm_layout_signal_emit(popup_mod->popup_field[ELM_DATETIME_MONTH],
                             "elm,action,entry,toggle", "elm");
       elm_object_focus_set(obj, EINA_FALSE);

       entry = elm_object_part_content_get(popup_mod->popup_field[ELM_DATETIME_DATE],
                                         "elm.swallow.entry");
       if (!elm_object_focus_get(entry))
         {
            elm_layout_signal_emit(popup_mod->popup_field[ELM_DATETIME_DATE],
                                 "elm,action,entry,toggle", "elm");
            elm_object_focus_set(entry, EINA_TRUE);
         }
     }
   *insert = 0;
}

static void
_hour_validity_checking_filter(void *data, Evas_Object *obj, char **text)
{
   Popup_Module_Data *popup_mod;
   Evas_Object *entry;
   char new_str[BUFF_SIZE] = {0,};
   int val = 0, len;
   char *insert;
   const char *curr_str;

   EINA_SAFETY_ON_NULL_RETURN(text);

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   insert = *text;

   insert = *text;
   len = strlen(elm_object_text_get(obj));
   if (len < 1) return;

   curr_str = elm_object_text_get(obj);
   if (curr_str) strncpy(new_str, curr_str, BUFF_SIZE);
   strncat(new_str, insert, 1);
   if (new_str[0]) val = atoi(new_str);

   if (!popup_mod->time_24hr && val > STRUCT_TM_TIME_12HRS_MAX_VALUE)
     {
        *insert = 0;
        return;
     }
   else if (popup_mod->time_24hr && val > STRUCT_TM_TIME_24HRS_MAX_VALUE)
     {
        *insert = 0;
        return;
     }

    elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_HOUR], val);
    elm_layout_signal_emit(popup_mod->popup_field[ELM_DATETIME_HOUR],
                                    "elm,action,entry,toggle", "elm");
    elm_object_focus_set(obj, EINA_FALSE);

    entry = elm_object_part_content_get(popup_mod->popup_field[ELM_DATETIME_MINUTE],
                                                 "elm.swallow.entry");
    if (!elm_object_focus_get(entry))
      {
         elm_layout_signal_emit(popup_mod->popup_field[ELM_DATETIME_MINUTE],
                                         "elm,action,entry,toggle", "elm");
         elm_object_focus_set(entry, EINA_TRUE);
      }
}

static void
_date_validity_checking_filter(void *data, Evas_Object *obj, char **text)
{
   Popup_Module_Data *popup_mod;
   char new_str[BUFF_SIZE] = {0,};
   int min, max, val = 0, len;
   const char *curr_str;
   char *insert;

   EINA_SAFETY_ON_NULL_RETURN(text);
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   insert = *text;
   len = strlen(elm_object_text_get(obj));
   if (len < 1) return;

   curr_str = elm_object_text_get(obj);
   if (curr_str) strncpy(new_str, curr_str, BUFF_SIZE);
   strncat(new_str, insert, 1);
   if (new_str[0]) val = atoi(new_str);

   popup_mod->mod_data.field_limit_get(popup_mod->mod_data.base, ELM_DATETIME_DATE, &min, &max);

   if ((val < min) || (val > max))
     *insert = 0;
}

static void
_minute_validity_checking_filter(void *data, Evas_Object *obj, char **text)
{
   Popup_Module_Data *popup_mod;
   char new_str[BUFF_SIZE];
   int min, max, val = 0, len;
   char *insert;
   const char *curr_str;

   EINA_SAFETY_ON_NULL_RETURN(text);
   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   insert = *text;
   len = strlen(elm_object_text_get(obj));
   if (len < 1) return;

   curr_str = elm_object_text_get(obj);
   if (curr_str) strncpy(new_str, curr_str, BUFF_SIZE);
   new_str[BUFF_SIZE - 1] = '\0';
   strncat(new_str, insert, 1);
   if (new_str[0]) val = atoi(new_str);

   popup_mod->mod_data.field_limit_get(popup_mod->mod_data.base, ELM_DATETIME_MINUTE, &min, &max);

   if ((val < min) || (val > max))
     *insert = 0;
}

static void
_set_datepicker_entry_filter(Popup_Module_Data *popup_mod)
{
   Evas_Object *spinner, *entry;
   static Elm_Entry_Filter_Accept_Set digits_filter_data;
   static Elm_Entry_Filter_Limit_Size limit_filter_data;
   char buf[BUFF_SIZE];
   int idx, value;

   if (!popup_mod) return;

   digits_filter_data.accepted = "0123456789";
   digits_filter_data.rejected = NULL;

   limit_filter_data.max_byte_count = 0;

   for (idx = 0; idx <= ELM_DATETIME_DATE; idx++)
     {
       spinner = popup_mod->popup_field[idx];
       entry = elm_object_part_content_get(spinner, "elm.swallow.entry");
       if (!entry) continue;

       snprintf(buf, sizeof(buf), "datetime_popup/%s", field_styles[idx]);
       elm_object_style_set(entry, buf);
       elm_entry_magnifier_disabled_set(entry, EINA_TRUE);
       elm_entry_context_menu_disabled_set(entry, EINA_TRUE);

       if (idx == ELM_DATETIME_MONTH)
         {
            value = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_MONTH]) - 1;
            elm_object_text_set(entry, month_arr[value]);
         }

       elm_entry_markup_filter_append(entry, elm_entry_filter_accept_set, &digits_filter_data);

       if (idx == ELM_DATETIME_MONTH)
         elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_MONTH);
       else
         elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_DATETIME);

       if (idx == ELM_DATETIME_YEAR)
         limit_filter_data.max_char_count = 4;
       else
         limit_filter_data.max_char_count = 2;

       elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

       evas_object_smart_callback_add(entry, "focused", _entry_focused_cb, popup_mod);
       evas_object_smart_callback_add(entry, "unfocused", _entry_unfocused_cb, popup_mod);
       evas_object_smart_callback_add(entry, "activated", _entry_activated_cb, popup_mod);
       evas_object_smart_callback_add(entry, "clicked", _entry_clicked_cb, popup_mod);

       if (idx == ELM_DATETIME_YEAR)
         elm_entry_markup_filter_append(entry, _year_validity_checking_filter, popup_mod);
       else if (idx == ELM_DATETIME_MONTH)
         elm_entry_markup_filter_append(entry, _month_validity_checking_filter, popup_mod);
       else if (idx == ELM_DATETIME_DATE)
         elm_entry_markup_filter_append(entry, _date_validity_checking_filter, popup_mod);
     }
}

static void
_set_timepicker_entry_filter(Popup_Module_Data *popup_mod)
{
   Evas_Object *spinner, *entry;
   static Elm_Entry_Filter_Accept_Set digits_filter_data;
   static Elm_Entry_Filter_Limit_Size limit_filter_data;
   char buf[BUFF_SIZE];
   int idx;

   if (!popup_mod) return;

   digits_filter_data.accepted = "0123456789";
   digits_filter_data.rejected = NULL;

   limit_filter_data.max_byte_count = 0;

   for (idx = ELM_DATETIME_HOUR; idx < ELM_DATETIME_AMPM; idx++)
     {
       spinner = popup_mod->popup_field[idx];
       entry = elm_object_part_content_get(spinner, "elm.swallow.entry");
       if (!entry) continue;

       snprintf(buf, sizeof(buf), "datetime_popup/%s", field_styles[idx]);
       elm_object_style_set(entry, buf);
       elm_entry_magnifier_disabled_set(entry, EINA_TRUE);
       elm_entry_context_menu_disabled_set(entry, EINA_TRUE);
       elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_DATETIME);
       elm_entry_markup_filter_append(entry, elm_entry_filter_accept_set, &digits_filter_data);

       limit_filter_data.max_char_count = 2;
       elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

       evas_object_smart_callback_add(entry, "focused", _entry_focused_cb, popup_mod);
       evas_object_smart_callback_add(entry, "unfocused", _entry_unfocused_cb, popup_mod);
       evas_object_smart_callback_add(entry, "activated", _entry_activated_cb, popup_mod);
       evas_object_smart_callback_add(entry, "clicked", _entry_clicked_cb, popup_mod);

       if (idx == ELM_DATETIME_HOUR)
         elm_entry_markup_filter_append(entry, _hour_validity_checking_filter, popup_mod);
       else if (idx == ELM_DATETIME_MINUTE)
         elm_entry_markup_filter_append(entry, _minute_validity_checking_filter, popup_mod);
     }
}

static void
_show_datepicker_layout(Popup_Module_Data *popup_mod)
{
   Evas_Object *content;
   struct tm curr_time;
   int idx, year, month, min, max;
   Elm_Datetime_Field_Type  field_type = ELM_DATETIME_YEAR;

   if (!popup_mod || !popup_mod->mod_data.base) return;

   content = elm_object_content_get(popup_mod->popup);
   if (content != popup_mod->datepicker_layout)
     {
         elm_object_content_unset(popup_mod->popup);
         if (content) evas_object_hide(content);
         elm_object_content_set(popup_mod->popup, popup_mod->datepicker_layout);
         evas_object_show(popup_mod->datepicker_layout);
         elm_object_part_text_set(popup_mod->popup, "title,text", "Set date");
     }

   elm_datetime_value_get(popup_mod->mod_data.base, &curr_time);
   year = curr_time.tm_year + STRUCT_TM_YEAR_BASE_VALUE;
   elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_YEAR], year);
   month = curr_time.tm_mon + 1;
   elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_MONTH], month);
   elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_DATE], curr_time.tm_mday);

   for (idx = 0; idx <= ELM_DATETIME_DATE; idx++)
     {
         field_type = (Elm_Datetime_Field_Type)idx;
         popup_mod->mod_data.field_limit_get(popup_mod->mod_data.base, field_type, &min, &max);
         if (idx == ELM_DATETIME_YEAR)
           elm_spinner_min_max_set(popup_mod->popup_field[idx], (STRUCT_TM_YEAR_BASE_VALUE + min), (STRUCT_TM_YEAR_BASE_VALUE + max));
         else if (idx == ELM_DATETIME_MONTH)
           elm_spinner_min_max_set(popup_mod->popup_field[idx], (1+min), (1+max));
         else
           elm_spinner_min_max_set(popup_mod->popup_field[idx], min, max);
     }
}

static void
_show_timepicker_layout(Popup_Module_Data *popup_mod)
{
   Evas_Object *content;
   struct tm curr_time;
   int idx, min, max;
   int ampm_min = 0, ampm_max = 1;
   Elm_Datetime_Field_Type  field_type = ELM_DATETIME_YEAR;
   Eina_Bool is_pm = EINA_FALSE;

   if (!popup_mod || !popup_mod->mod_data.base) return;

   content = elm_object_content_get(popup_mod->popup);
   if (content != popup_mod->timepicker_layout)
     {
         elm_object_content_unset(popup_mod->popup);
         if (content) evas_object_hide(content);
         elm_object_content_set(popup_mod->popup, popup_mod->timepicker_layout);
         evas_object_show(popup_mod->timepicker_layout);
         elm_object_part_text_set(popup_mod->popup, "title,text", "Set time");
     }

   elm_datetime_value_get(popup_mod->mod_data.base, &curr_time);
   if (!popup_mod->time_24hr)
     {
        if (curr_time.tm_hour >= STRUCT_TM_TIME_12HRS_MAX_VALUE)
          is_pm = EINA_TRUE;
        if (curr_time.tm_hour >= STRUCT_TM_TIME_12HRS_MAX_VALUE)
          curr_time.tm_hour -= STRUCT_TM_TIME_12HRS_MAX_VALUE;
        curr_time.tm_hour = (curr_time.tm_hour == 0) ? STRUCT_TM_TIME_12HRS_MAX_VALUE : curr_time.tm_hour;
     }
    elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_HOUR], curr_time.tm_hour);
    elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_MINUTE], curr_time.tm_min);
    elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_AMPM], is_pm);
    popup_mod->time_hours = curr_time.tm_hour;

    for (idx = ELM_DATETIME_HOUR; idx < ELM_DATETIME_AMPM; idx++)
      {
          field_type = (Elm_Datetime_Field_Type)idx;
          popup_mod->mod_data.field_limit_get(popup_mod->mod_data.base, field_type, &min, &max);
          if (idx == ELM_DATETIME_HOUR && !popup_mod->time_24hr)
            {
               if (min < 1) min = 1;
               else if (min > STRUCT_TM_TIME_12HRS_MAX_VALUE)
                {
                   min = min - STRUCT_TM_TIME_12HRS_MAX_VALUE;
                   ampm_min = 1;
                }
               if (max > STRUCT_TM_TIME_12HRS_MAX_VALUE)
                 max = STRUCT_TM_TIME_12HRS_MAX_VALUE;
               else if (max < STRUCT_TM_TIME_12HRS_MAX_VALUE) ampm_max = 0;
               elm_spinner_min_max_set(popup_mod->popup_field[idx], min, max);
             }
          else
            elm_spinner_min_max_set(popup_mod->popup_field[idx], min, max);
      }
    elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_AMPM], ampm_min, ampm_max);
}

static void
_set_month_special_values(Popup_Module_Data *popup_mod)
{
   struct tm curr_time;
   const char *fmt = NULL;
   int month;

   if (!popup_mod) return;

   elm_datetime_value_get(popup_mod->mod_data.base, &curr_time);
   fmt = popup_mod->mod_data.field_format_get(popup_mod->mod_data.base, ELM_DATETIME_MONTH);
   for (month = 0; month < TOTAL_NUMBER_OF_MONTHS; month++)
     {
         curr_time.tm_mon = month;
         strftime(month_arr[month], MONTH_STRING_MAX_SIZE, fmt, &curr_time);
     }
   for (month = 0; month < 12; month++)
     elm_spinner_special_value_add(popup_mod->popup_field[ELM_DATETIME_MONTH],
                                   month + 1, month_arr[month]);
}

static void
_set_ampm_special_values(Popup_Module_Data *popup_mod)
{
   struct tm curr_time;
   char am_str[BUFF_SIZE] = {0,};
   char pm_str[BUFF_SIZE] = {0,};
   const char *fmt = NULL;

   if (!popup_mod) return;

   elm_datetime_value_get(popup_mod->mod_data.base, &curr_time);
   fmt = popup_mod->mod_data.field_format_get(popup_mod->mod_data.base, ELM_DATETIME_AMPM);
   // if AM/PM format exists, set its string as spinner special value.
   if (fmt && strcmp(fmt, "%"))
     {
        curr_time.tm_hour = 0;
        strftime(am_str, BUFF_SIZE, fmt, &curr_time);
        curr_time.tm_hour = 12;
        strftime(pm_str, BUFF_SIZE, fmt, &curr_time);
     }
   if (am_str[0])
     elm_spinner_special_value_add(popup_mod->popup_field[ELM_DATETIME_AMPM], 0, am_str);
   else
     elm_spinner_special_value_add(popup_mod->popup_field[ELM_DATETIME_AMPM], 0, E_("AM"));

   if (pm_str[0])
     elm_spinner_special_value_add(popup_mod->popup_field[ELM_DATETIME_AMPM], 1, pm_str);
   else
     elm_spinner_special_value_add(popup_mod->popup_field[ELM_DATETIME_AMPM], 1, E_("PM"));
}

static void
_create_datetime_popup(Popup_Module_Data *popup_mod)
{
   Evas_Object *set_btn, *cancel_btn;
   Evas_Object *parent, *widget, *conformant = NULL;
   const char *widget_type;

   if (!popup_mod) return;

   // add popup to the content of elm_conformant
   widget = popup_mod->mod_data.base;
   while(widget != NULL)
     {
        parent = elm_widget_parent_get(widget);
        widget_type = elm_widget_type_get(widget);
        if (!strcmp(widget_type, "elm_conformant")) conformant = widget;
        widget = parent;
     }
   if (conformant)
     popup_mod->popup = elm_popup_add(elm_object_content_get(conformant));
   else
     popup_mod->popup = elm_popup_add(elm_widget_top_get(popup_mod->mod_data.base));

   evas_object_size_hint_weight_set(popup_mod->popup, EVAS_HINT_EXPAND,
                           EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(popup_mod->popup, EVAS_HINT_FILL, 0.5);

   cancel_btn = elm_button_add(popup_mod->popup);
   elm_object_style_set(cancel_btn, "popup_button/default");
   elm_object_text_set(cancel_btn, E_("Cancel"));
   elm_object_part_content_set(popup_mod->popup, "button1", cancel_btn);
   evas_object_smart_callback_add(cancel_btn, "clicked", _popup_cancel_btn_clicked_cb, popup_mod);

   set_btn = elm_button_add(popup_mod->popup);
   elm_object_style_set(set_btn, "popup_button/default");
   elm_object_text_set(set_btn, E_("Set"));
   elm_object_part_content_set(popup_mod->popup, "button2", set_btn);
   evas_object_smart_callback_add(set_btn, "clicked", _popup_set_btn_clicked_cb, popup_mod);
}

static void
_create_datepicker_layout(Popup_Module_Data *popup_mod)
{
   Evas_Object *spinner;
   char buf[BUFF_SIZE];
   int idx;

   if (!popup_mod) return;

   popup_mod->datepicker_layout = elm_layout_add(popup_mod->popup);
   elm_layout_theme_set(popup_mod->datepicker_layout, "layout", "datetime_popup", "date_layout");

   for (idx = 0; idx <= ELM_DATETIME_DATE; idx++)
     {
       spinner = elm_spinner_add(popup_mod->popup);
       snprintf(buf, sizeof(buf), "datetime_popup/%s", field_styles[idx]);
       elm_object_style_set(spinner, buf);
       elm_spinner_step_set(spinner, 1);
       elm_spinner_wrap_set(spinner, EINA_TRUE);
       elm_spinner_label_format_set(spinner, "%02.0f");
       snprintf(buf, sizeof(buf), "field%d", idx);
       elm_object_part_content_set(popup_mod->datepicker_layout, buf, spinner);
       if (idx < ELM_DATETIME_DATE)
         evas_object_smart_callback_add(spinner, "changed", _adjust_max_days, popup_mod);
       popup_mod->popup_field[idx] = spinner;
     }

   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_YEAR], 1902, 2037);
   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_MONTH], 1, 12);
   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_DATE], 1, 31);

   _set_month_special_values(popup_mod);
   _set_datepicker_entry_filter(popup_mod);
}

static void
_create_timepicker_layout(Popup_Module_Data *popup_mod)
{
   Evas_Object *spinner;
   char buf[BUFF_SIZE];
   const char *fmt = NULL;
   int idx;

   if (!popup_mod) return;

   popup_mod->timepicker_layout = elm_layout_add(popup_mod->popup);
   fmt = popup_mod->mod_data.field_format_get(popup_mod->mod_data.base, ELM_DATETIME_AMPM);
   if (fmt && strcmp(fmt, "%"))
     {
        popup_mod->time_24hr = EINA_FALSE;
        elm_layout_theme_set(popup_mod->timepicker_layout, "layout", "datetime_popup", "time_layout");
     }
   else
     {
        popup_mod->time_24hr = EINA_TRUE;
        elm_layout_theme_set(popup_mod->timepicker_layout, "layout", "datetime_popup", "time_layout_24hr");
     }

   for (idx = ELM_DATETIME_HOUR; idx < DATETIME_FIELD_COUNT; idx++)
     {
       spinner = elm_spinner_add(popup_mod->popup);
       snprintf(buf, sizeof(buf), "datetime_popup/%s", field_styles[idx]);
       elm_object_style_set(spinner, buf);
       elm_spinner_step_set(spinner, 1);
       elm_spinner_wrap_set(spinner, EINA_TRUE);
       elm_spinner_label_format_set(spinner, "%02.0f");
       snprintf(buf, sizeof(buf), "field%d", (idx - ELM_DATETIME_HOUR));
       elm_object_part_content_set(popup_mod->timepicker_layout, buf, spinner);
       popup_mod->popup_field[idx] = spinner;
    }

   if (popup_mod->time_24hr)
     elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_HOUR], 0, 23);
   else
     elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_HOUR], 1, 12);
   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_MINUTE], 0, 59);
   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_AMPM], 0, 1);
   evas_object_smart_callback_add(popup_mod->popup_field[ELM_DATETIME_HOUR],
                                  "changed", _adjust_ampm, popup_mod);
   elm_object_signal_callback_add(popup_mod->popup_field[ELM_DATETIME_HOUR],
       "elm,action,increment,start", "",_hour_increment_start_cb, popup_mod);
   elm_object_signal_callback_add(popup_mod->popup_field[ELM_DATETIME_HOUR],
       "elm,action,increment,stop", "",_hour_increment_stop_cb, popup_mod);
   elm_object_signal_callback_add(popup_mod->popup_field[ELM_DATETIME_HOUR],
       "elm,action,decrement,start", "",_hour_decrement_start_cb, popup_mod);
   elm_object_signal_callback_add(popup_mod->popup_field[ELM_DATETIME_HOUR],
       "elm,action,decrement,stop", "",_hour_decrement_stop_cb, popup_mod);

   _set_ampm_special_values(popup_mod);
   _set_timepicker_entry_filter(popup_mod);
}

static void
_datepicker_show_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   if (!popup_mod->popup)
     _create_datetime_popup(popup_mod);

   if (!popup_mod->datepicker_layout)
     _create_datepicker_layout(popup_mod);

   _show_datepicker_layout(popup_mod);
   evas_object_show(popup_mod->popup);
}

static void
_timepicker_show_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   if (!popup_mod->popup)
     _create_datetime_popup(popup_mod);

   if (!popup_mod->timepicker_layout)
     _create_timepicker_layout(popup_mod);

   _show_timepicker_layout(popup_mod);
   evas_object_show(popup_mod->popup);
}

static void
_access_set(Evas_Object *obj, Elm_Datetime_Field_Type field_type)
{
   const char* type = NULL;

   switch (field_type)
     {
       case ELM_DATETIME_YEAR:
        type = "datetime field, year";
        break;

      case ELM_DATETIME_MONTH:
        type = "datetime field, month";
        break;

     case ELM_DATETIME_DATE:
        type = "datetime field, date";
        break;

      case ELM_DATETIME_HOUR:
        type = "datetime field, hour";
        break;

      case ELM_DATETIME_MINUTE:
        type = "datetime field, minute";
        break;

      case ELM_DATETIME_AMPM:
        type = "datetime field, AM PM";
        break;

      default:
        break;
     }

   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, type);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, NULL, NULL);
}

// module functions for the specific module type
EAPI void
field_value_display(Elm_Datetime_Module_Data *module_data, Evas_Object *obj)
{
   Popup_Module_Data *popup_mod;
   Elm_Datetime_Field_Type  field_type;
   struct tm curr_time;
   char buf[BUFF_SIZE] = {0,};
   const char *fmt;

   popup_mod = (Popup_Module_Data *)module_data;
   if (!popup_mod || !obj) return;

   elm_datetime_value_get(popup_mod->mod_data.base, &curr_time);
   field_type = (Elm_Datetime_Field_Type )evas_object_data_get(obj, "_field_type");
   fmt = popup_mod->mod_data.field_format_get(popup_mod->mod_data.base, field_type);
   strftime(buf, sizeof(buf), fmt, &curr_time);

   if ((!buf[0]) && ((!strcmp(fmt, "%p")) || (!strcmp(fmt, "%P"))))
     {
        if (curr_time.tm_hour < STRUCT_TM_TIME_12HRS_MAX_VALUE) strcpy(buf, E_("AM"));
        else strcpy(buf, E_("PM"));
     }
   elm_object_text_set(obj, buf);

   if (field_type == ELM_DATETIME_DATE)
     {
        mktime(&curr_time);
        strftime(buf, sizeof(buf), "%a", &curr_time);
        elm_layout_text_set(popup_mod->mod_data.base, "elm.text.weekday", buf);
     }
}

EAPI Evas_Object *
field_create(Elm_Datetime_Module_Data *module_data, Elm_Datetime_Field_Type field_type)
{
   Popup_Module_Data *popup_mod;
   Evas_Object *field_obj;
   char buf[BUFF_SIZE];

   popup_mod = (Popup_Module_Data *)module_data;
   if (!popup_mod) return NULL;

   field_obj = elm_label_add(popup_mod->mod_data.base);
   snprintf(buf, sizeof(buf), "datetime/%s/default", field_styles[field_type]);
   elm_object_style_set(field_obj, buf);
   evas_object_data_set(field_obj, "_field_type", (void *)field_type);

   _access_set(field_obj, field_type);

   return field_obj;
}

EAPI Elm_Datetime_Module_Data *
obj_hook(Evas_Object *obj __UNUSED__)
{
   Popup_Module_Data *popup_mod;

   popup_mod = ELM_NEW(Popup_Module_Data);
   if (!popup_mod) return NULL;

   elm_object_signal_callback_add(obj, "datetime,datepicker,show", "",
                                  _datepicker_show_cb, popup_mod);
   elm_object_signal_callback_add(obj, "datetime,datepicker,hide", "",
                                  _datepicker_hide_cb, popup_mod);
   elm_object_signal_callback_add(obj, "datetime,timepicker,show", "",
                                  _timepicker_show_cb, popup_mod);
   elm_object_signal_callback_add(obj, "datetime,timepicker,hide", "",
                                  _timepicker_hide_cb, popup_mod);

   popup_mod->popup = NULL;
   popup_mod->datepicker_layout = NULL;
   popup_mod->timepicker_layout = NULL;
   popup_mod->hour_incr = EINA_FALSE;
   popup_mod->hour_decr = EINA_FALSE;

   return ((Elm_Datetime_Module_Data*)popup_mod);
}

EAPI void
obj_unhook(Elm_Datetime_Module_Data *module_data)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)module_data;
   if (!popup_mod) return;

   if (popup_mod->popup)
     evas_object_del(popup_mod->popup);

   if (popup_mod)
     {
        free(popup_mod);
        popup_mod = NULL;
     }
}

EAPI void
obj_language_change(Elm_Datetime_Module_Data *module_data __UNUSED__)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)module_data;
   if (!popup_mod || popup_mod->popup) return;

   _set_month_special_values(popup_mod);
   _set_ampm_special_values(popup_mod);
}

EAPI void
obj_hide(Elm_Datetime_Module_Data *module_data __UNUSED__)
{
   // TODO: Default focus - enhance this func. for obj_show/obj_hide like below
#if 0
   Popup_Module_Data *popup_mod;
   int idx;
   Eina_Bool is_focused = EINA_FALSE; //init with i/p argument

   popup_mod = (Popup_Module_Data *)module_data;
   if (!popup_mod || popup_mod->popup) return;

   for (idx = 0; idx < DATETIME_FIELD_COUNT; idx++)
      if (elm_object_focus_get(popup_mod->popup_field[idx]))
        is_focused = EINA_TRUE;

   if (is_focused)
     evas_object_show(popup_mod->popup);
   else
     evas_object_hide(popup_mod->popup);
#endif
}

// module api funcs needed
EAPI int
elm_modapi_init(void *m __UNUSED__)
{
   return 1; // succeed always
}

EAPI int
elm_modapi_shutdown(void *m __UNUSED__)
{
   return 1; // succeed always
}
