#include <Elementary.h>
#include "elm_priv.h"

#define DATETIME_FIELD_COUNT            6
#define BUFF_SIZE                       1024
#define MONTH_STRING_MAX_SIZE           32
#define TOTAL_NUMBER_OF_MONTHS          12
#define STRUCT_TM_YEAR_BASE_VALUE       1900
#define STRUCT_TM_TIME_12HRS_MAX_VALUE  12
#define STRUCT_TM_TIME_24HRS_MAX_VALUE  23

/* struct tm does not define the fields in the order year, month,
 * date, hour, minute. values are reassigned to an array for easy
 * handling.
 */
#define DATETIME_MODULE_TM_ARRAY(intptr, tmptr) \
  int *intptr[] = {                             \
     &(tmptr)->tm_year,                         \
     &(tmptr)->tm_mon,                          \
     &(tmptr)->tm_mday,                         \
     &(tmptr)->tm_hour,                         \
     &(tmptr)->tm_min}

static const char *field_styles[] = {
                         "year", "month", "date", "hour", "minute", "ampm" };

static char month_arr[TOTAL_NUMBER_OF_MONTHS][MONTH_STRING_MAX_SIZE];

typedef struct _Popup_Module_Data Popup_Module_Data;

struct _Popup_Module_Data
{
   Elm_Datetime_Module_Data mod_data;
   Evas_Object *popup;
   Evas_Object *datepicker_layout, *timepicker_layout;
   Evas_Object *popup_field[DATETIME_FIELD_COUNT];
   struct tm set_time;
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
   int idx = 0;
   Evas_Object *spinner, *entry;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;
   evas_object_hide(popup_mod->popup);
   content = elm_object_content_get(popup_mod->popup);
   if (content == popup_mod->datepicker_layout)
     {
        for (idx = 0; idx <= ELM_DATETIME_DATE; idx++)
          {
             spinner = popup_mod->popup_field[idx];
             entry = elm_object_part_content_get(spinner, "elm.swallow.entry");
             if (!entry) continue;
             if (elm_object_focus_get(entry))
               {
                  elm_object_focus_set(entry, EINA_FALSE);
                  elm_layout_signal_emit(spinner, "elm,action,entry,toggle", "elm");
                  edje_object_message_signal_process(elm_layout_edje_get(spinner));
               }
          }
        _set_datepicker_value(popup_mod);
     }
   else if (content == popup_mod->timepicker_layout)
     {
        for (idx = ELM_DATETIME_HOUR; idx < ELM_DATETIME_AMPM; idx++)
          {
             spinner = popup_mod->popup_field[idx];
             entry = elm_object_part_content_get(spinner, "elm.swallow.entry");
             if (!entry) continue;
             if (elm_object_focus_get(entry))
               {
                  elm_object_focus_set(entry, EINA_FALSE);
                  elm_layout_signal_emit(spinner, "elm,action,entry,toggle", "elm");
                  edje_object_message_signal_process(elm_layout_edje_get(spinner));
               }
          }
        _set_timepicker_value(popup_mod);
     }
}

static void
_popup_cancel_btn_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   Evas_Object *content;
   int idx = 0;
   Evas_Object *spinner, *entry;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   evas_object_hide(popup_mod->popup);
   content = elm_object_content_get(popup_mod->popup);
   if (content == popup_mod->datepicker_layout)
     {
        for (idx = 0; idx <= ELM_DATETIME_DATE; idx++)
          {
             spinner = popup_mod->popup_field[idx];
             entry = elm_object_part_content_get(spinner, "elm.swallow.entry");
             if (!entry) continue;
             if (elm_object_focus_get(entry))
               {
                  elm_object_focus_set(entry, EINA_FALSE);
                  elm_layout_signal_emit(spinner, "elm,action,entry,toggle", "elm");
               }
          }
     }
   else if (content == popup_mod->timepicker_layout)
     {
        for (idx = ELM_DATETIME_HOUR; idx < ELM_DATETIME_AMPM; idx++)
          {
             spinner = popup_mod->popup_field[idx];
             entry = elm_object_part_content_get(spinner, "elm.swallow.entry");
             if (!entry) continue;
             if (elm_object_focus_get(entry))
               {
                  elm_object_focus_set(entry, EINA_FALSE);
                  elm_layout_signal_emit(spinner, "elm,action,entry,toggle", "elm");
               }
          }
     }
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

void
_set_datepicker_popup_title_text(Popup_Module_Data *popup_mod)
{
   struct tm set_time;
   time_t t;
   char title[BUFF_SIZE];
   if (!popup_mod) return;

   t = time(NULL);
   localtime_r(&t, &set_time);
   set_time.tm_year = (popup_mod->set_time).tm_year;
   set_time.tm_mon = (popup_mod->set_time).tm_mon;
   set_time.tm_mday = (popup_mod->set_time).tm_mday;
   set_time.tm_hour = 0;
   mktime(&set_time);
   strftime(title, BUFF_SIZE, "%x, %a", &set_time);
   elm_object_part_text_set(popup_mod->popup, "title,text", title);
}

void
_set_timepicker_popup_title_text(Popup_Module_Data *popup_mod)
{
   char title[BUFF_SIZE];
   if (!popup_mod) return;

   if (popup_mod->time_24hr)
     strftime(title, BUFF_SIZE, "%H:%M", &(popup_mod->set_time));
   else
     strftime(title, BUFF_SIZE, "%I:%M %p", &(popup_mod->set_time));
   elm_object_part_text_set(popup_mod->popup, "title,text", title);
}

static void
_datepicker_value_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   struct tm min_values, max_values;
   int idx, field_idx, min, max;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   for (idx = 0; idx < DATETIME_FIELD_COUNT; idx++)
     if ((obj == popup_mod->popup_field[idx])) break;

   if (idx > ELM_DATETIME_DATE) return;

   field_idx = idx;
   DATETIME_MODULE_TM_ARRAY(set_val_arr, &popup_mod->set_time);
   if (field_idx == ELM_DATETIME_YEAR)
     *set_val_arr[field_idx] = (int)elm_spinner_value_get(obj) + STRUCT_TM_YEAR_BASE_VALUE;
   else if (field_idx == ELM_DATETIME_MONTH)
     *set_val_arr[field_idx] = (int)elm_spinner_value_get(obj) - 1;
   else
     *set_val_arr[field_idx] = (int)elm_spinner_value_get(obj);

   popup_mod->mod_data.fields_min_max_get(popup_mod->mod_data.base,
                       &(popup_mod->set_time), &min_values, &max_values);

   DATETIME_MODULE_TM_ARRAY(min_val_arr, &min_values);
   DATETIME_MODULE_TM_ARRAY(max_val_arr, &max_values);

   for (idx = field_idx; idx <= ELM_DATETIME_DATE; idx++)
     {
        min = *min_val_arr[idx];
        max = *max_val_arr[idx];
        if (idx == ELM_DATETIME_YEAR)
          {
             min += STRUCT_TM_YEAR_BASE_VALUE;
             max += STRUCT_TM_YEAR_BASE_VALUE;
          }
        else if (idx == ELM_DATETIME_MONTH)
          {
             min += 1;
             max += 1;
          }
        elm_spinner_min_max_set(popup_mod->popup_field[idx], min, max);
     }
   for (idx = field_idx; idx <= ELM_DATETIME_DATE; idx++)
     {
        if (idx == ELM_DATETIME_YEAR)
          *set_val_arr[idx] = (int)elm_spinner_value_get(popup_mod->popup_field[idx]) - STRUCT_TM_YEAR_BASE_VALUE;
        else if (idx == ELM_DATETIME_MONTH)
          *set_val_arr[idx] = (int)elm_spinner_value_get(popup_mod->popup_field[idx]) - 1;
        else
          *set_val_arr[idx] = (int)elm_spinner_value_get(popup_mod->popup_field[idx]);
     }

   _set_datepicker_popup_title_text(popup_mod);
}

static void
_timepicker_value_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   struct tm min_values, max_values;
   int hour, hour_min, hour_max;
   Eina_Bool is_pm;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   if (obj == popup_mod->popup_field[ELM_DATETIME_HOUR])
     {
        hour = (int)elm_spinner_value_get(obj);
        if (popup_mod->time_24hr)
          (popup_mod->set_time).tm_hour = hour;
        else
          {
             is_pm = !!elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_AMPM]);
             if (is_pm)
               {
                  hour += 12;
                  if (hour == 24) hour = 0;
               }
             (popup_mod->set_time).tm_hour = hour;
             if (hour == 0 || hour == 12)
               elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_AMPM], !is_pm);
          }
     }
   else
     {
        (popup_mod->set_time).tm_min = (int)elm_spinner_value_get(obj);
     }

   popup_mod->mod_data.fields_min_max_get(popup_mod->mod_data.base,
                       &(popup_mod->set_time), &min_values, &max_values);

   hour_min = min_values.tm_hour;
   hour_max = max_values.tm_hour;
   if (!popup_mod->time_24hr)
     {
        hour_min = 1;
        hour_max = 12;
     }
   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_HOUR], hour_min, hour_max);
   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_MINUTE], min_values.tm_min, max_values.tm_min);

   (popup_mod->set_time).tm_min = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_MINUTE]);

   _set_timepicker_popup_title_text(popup_mod);
}

static void
_ampm_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Popup_Module_Data *popup_mod;
   int hour;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod) return;

   hour = (int)elm_spinner_value_get(popup_mod->popup_field[ELM_DATETIME_HOUR]);
   if (hour > 12) hour -= 12;
   elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_HOUR], hour);

   _set_timepicker_popup_title_text(popup_mod);
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
   char new_str[BUFF_SIZE] = {0,};
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
   strncat(new_str, insert, 1);
   if (new_str[0]) val = atoi(new_str) - 1;

   popup_mod->mod_data.field_limit_get(popup_mod->mod_data.base, ELM_DATETIME_MONTH, &min, &max);

   if ((val >= min) && (val <= max))
     {
       elm_spinner_value_set(popup_mod->popup_field[ELM_DATETIME_MONTH], val + 1);
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
     }
   elm_datetime_value_get(popup_mod->mod_data.base, &(popup_mod->set_time));
   _set_datepicker_popup_title_text(popup_mod);

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
     }
   elm_datetime_value_get(popup_mod->mod_data.base, &(popup_mod->set_time));
   _set_timepicker_popup_title_text(popup_mod);

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
       elm_spinner_editable_set(spinner, EINA_TRUE);
       snprintf(buf, sizeof(buf), "datetime_popup/%s", field_styles[idx]);
       elm_object_style_set(spinner, buf);
       elm_spinner_step_set(spinner, 1);
       elm_spinner_wrap_set(spinner, EINA_TRUE);
       elm_spinner_label_format_set(spinner, "%02.0f");
       snprintf(buf, sizeof(buf), "field%d", idx);
       elm_object_part_content_set(popup_mod->datepicker_layout, buf, spinner);
       evas_object_smart_callback_add(spinner, "changed", _datepicker_value_changed_cb, popup_mod);
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
       elm_spinner_editable_set(spinner, EINA_TRUE);
       snprintf(buf, sizeof(buf), "datetime_popup/%s", field_styles[idx]);
       elm_object_style_set(spinner, buf);
       elm_spinner_step_set(spinner, 1);
       elm_spinner_wrap_set(spinner, EINA_TRUE);
       elm_spinner_label_format_set(spinner, "%02.0f");
       snprintf(buf, sizeof(buf), "field%d", (idx - ELM_DATETIME_HOUR));
       elm_object_part_content_set(popup_mod->timepicker_layout, buf, spinner);
       if (idx != ELM_DATETIME_AMPM)
         evas_object_smart_callback_add(spinner, "changed", _timepicker_value_changed_cb, popup_mod);
       else
         evas_object_smart_callback_add(spinner, "changed", _ampm_clicked_cb, popup_mod);
       popup_mod->popup_field[idx] = spinner;
    }

   if (popup_mod->time_24hr)
     elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_HOUR], 0, 23);
   else
     elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_HOUR], 1, 12);
   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_MINUTE], 0, 59);
   elm_spinner_min_max_set(popup_mod->popup_field[ELM_DATETIME_AMPM], 0, 1);

   _set_ampm_special_values(popup_mod);
   _set_timepicker_entry_filter(popup_mod);
}

static void
_module_format_change(Popup_Module_Data *popup_mod)
{
   Evas_Object *datetime;
   int idx;
   Eina_Bool datepicker_bg_show = EINA_FALSE;
   Eina_Bool timepicker_bg_show = EINA_FALSE;

   if (!popup_mod) return;

   datetime = popup_mod->mod_data.base;
   for (idx = 0; idx <= ELM_DATETIME_DATE; idx++)
     datepicker_bg_show |= popup_mod->mod_data.field_location_get(datetime, idx, NULL);
   if (!datepicker_bg_show)
     elm_layout_signal_emit(datetime, "datetime,datepicker,bg,hide", "elm");

   for (idx = ELM_DATETIME_HOUR; idx < ELM_DATETIME_AMPM; idx++)
     timepicker_bg_show |= popup_mod->mod_data.field_location_get(datetime, idx, NULL);
   if (!timepicker_bg_show)
     elm_layout_signal_emit(datetime, "datetime,timepicker,bg,hide", "elm");
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
_module_language_changed_cb(void *data,
                           Evas_Object *obj __UNUSED__,
                           const char *emission __UNUSED__,
                           const char *source __UNUSED__)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)data;
   if (!popup_mod || popup_mod->popup) return;

   _set_month_special_values(popup_mod);
   _set_ampm_special_values(popup_mod);
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
   elm_object_signal_callback_add(obj, "language,changed", "",
                                  _module_language_changed_cb, popup_mod);

   popup_mod->popup = NULL;
   popup_mod->datepicker_layout = NULL;
   popup_mod->timepicker_layout = NULL;

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
obj_format_hook(Elm_Datetime_Module_Data *module_data)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)module_data;
   if (!popup_mod) return;

   _module_format_change(popup_mod);
}

EAPI void
obj_theme_hook(Elm_Datetime_Module_Data *module_data)
{
   Popup_Module_Data *popup_mod;

   popup_mod = (Popup_Module_Data *)module_data;
   if (!popup_mod) return;

   // TODO: function can be improved to provide different popup styles.
}

EAPI void
obj_focus_hook(Elm_Datetime_Module_Data *module_data __UNUSED__)
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
