#include <Elementary.h>
#include "elm_priv.h"
#include <unicode/udat.h>
#include <unicode/ustring.h>

#define DATETIME_FIELD_COUNT            6
#define BUFF_SIZE                       1024
#define STRUCT_TM_YEAR_BASE_VALUE       1900
#define STRUCT_TM_TIME_12HRS_MAX_VALUE  12
#define STRUCT_TM_TIME_24HRS_MAX_VALUE  23
#define PICKER_POPUP_FIELD_COUNT        3

#define ROTATE_TIME_NORMAL 0.2

typedef struct _Button_Module_Data Button_Module_Data;

struct _Button_Module_Data
{
   Elm_Datetime_Module_Data mod_data;
   Evas_Object *datetime_field[DATETIME_FIELD_COUNT];
   Evas_Object *group;
   Evas_Object *wheel_button_field, *ampm_button_field;
   Ecore_Timer *longpress_timer;
   Ecore_Timer *spin_timer;
   struct tm set_time;
   Eina_Bool time_12hr_fmt;
   Eina_Bool weekday_show;
   Eina_Bool weekday_loc_first;
   Eina_Bool timepicker;
};

EAPI void field_value_display(Elm_Datetime_Module_Data *module_data, Evas_Object *obj);

static void
_fields_visible_set(Button_Module_Data *button_mod)
{
   Evas_Object *datetime;

   if (!button_mod) return;
   datetime = button_mod->mod_data.base;

   if (button_mod->timepicker)
     {
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_YEAR, EINA_FALSE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_MONTH, EINA_FALSE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_DATE, EINA_FALSE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_HOUR, EINA_TRUE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_MINUTE, EINA_TRUE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_AMPM, EINA_TRUE);
     }
   else
     {
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_YEAR, EINA_TRUE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_MONTH, EINA_TRUE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_DATE, EINA_TRUE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_HOUR, EINA_FALSE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_MINUTE, EINA_FALSE);
        elm_datetime_field_visible_set(datetime, ELM_DATETIME_AMPM, EINA_FALSE);
     }
}

static void _ampm_button_show(Button_Module_Data *button_mod)
{
   Evas_Object *datetime = (Evas_Object *)button_mod->mod_data.base;
   Evas_Object *ampm_group = elm_object_part_content_get(button_mod->ampm_button_field, "elm.icon.right");

   elm_datetime_value_get(datetime, &(button_mod->set_time));
   if (button_mod->set_time.tm_hour >= STRUCT_TM_TIME_12HRS_MAX_VALUE)
     elm_radio_value_set(ampm_group, 1);
   else
     elm_radio_value_set(ampm_group, 0);

   elm_object_signal_emit(datetime, "elm,action,show,ampm_button", "");
}

static void
_select_first_field(Button_Module_Data *button_mod)
{
   Evas_Object *datetime;
   Evas_Object *group;
   Evas_Object *first_obj;
   int value, loc;
   Eina_Bool field_exist;

   if (!button_mod) return;
   datetime = button_mod->mod_data.base;
   group = button_mod->group;

   if (!button_mod->timepicker)
     {
        first_obj = elm_object_part_content_get(datetime, "field0");
        value = elm_radio_state_value_get(first_obj);
        elm_radio_value_set(group, value);
     }
   else
     {
        field_exist = button_mod->mod_data.field_location_get(datetime, ELM_DATETIME_AMPM, &loc);
        first_obj = elm_object_part_content_get(datetime, "field3");

        if (field_exist && (loc == 3))
          {
             _ampm_button_show(button_mod);
          }
        else if (!field_exist && (loc == 3))
          {
             first_obj = elm_object_part_content_get(datetime, "field4");
             elm_object_signal_emit(datetime, "elm,action,hide,ampm_button", "");
          }
        else
          {
             elm_object_signal_emit(datetime, "elm,action,hide,ampm_button", "");
          }

        value = elm_radio_state_value_get(first_obj);
        elm_radio_value_set(group, value);
     }
}

static void
_adjust_layout(Button_Module_Data *button_mod)
{
   Evas_Object *datetime;
   char buf[BUFF_SIZE];
   int idx, loc;
   Eina_Bool field_exist;

   if (!button_mod) return;
   datetime = button_mod->mod_data.base;

   for (idx = 0; idx <= ELM_DATETIME_AMPM; idx++)
     {
        field_exist = button_mod->mod_data.field_location_get(datetime, idx, &loc);
        switch (idx)
          {
           case ELM_DATETIME_YEAR:
              snprintf(buf, sizeof(buf), "elm,state,field%d,year", loc);
              elm_layout_signal_emit(datetime, buf, "");
              break;
           case ELM_DATETIME_MONTH:
              snprintf(buf, sizeof(buf), "elm,state,field%d,month", loc);
              elm_layout_signal_emit(datetime, buf, "");
              break;
           case ELM_DATETIME_DATE:
              snprintf(buf, sizeof(buf), "elm,state,field%d,date", loc);
              elm_layout_signal_emit(datetime, buf, "");
              break;
           case ELM_DATETIME_HOUR:
              snprintf(buf, sizeof(buf), "elm,state,field%d,hour", loc);
              elm_layout_signal_emit(datetime, buf, "");
              break;
           case ELM_DATETIME_MINUTE:
              snprintf(buf, sizeof(buf), "elm,state,field%d,minute", loc);
              elm_layout_signal_emit(datetime, buf, "");
              break;
           case ELM_DATETIME_AMPM:
              snprintf(buf, sizeof(buf), "elm,state,field%d,ampm", loc);
              elm_layout_signal_emit(datetime, buf, "");
              break;
          }
     }
   field_exist = button_mod->mod_data.field_location_get(datetime, ELM_DATETIME_AMPM, &loc);
   if (!field_exist)
     {
        snprintf(buf, sizeof(buf), "elm,state,field%d,ampm,hide", loc);
        elm_layout_signal_emit(datetime, buf, "");
     }
}

static Eina_Bool
_value_increment_cb(void *data)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;
   Evas_Object *datetime = button_mod->mod_data.base;
   struct tm min_time, max_time;
   int on_field = elm_radio_value_get(button_mod->group);

   elm_datetime_value_get(datetime, &(button_mod->set_time));
   button_mod->mod_data.fields_min_max_get(datetime, &(button_mod->set_time), &min_time, &max_time);

   switch (on_field)
     {
      case ELM_DATETIME_YEAR:
         button_mod->set_time.tm_year++;
         if (button_mod->set_time.tm_year > max_time.tm_year)
           button_mod->set_time.tm_year = min_time.tm_year;
         break;
      case ELM_DATETIME_MONTH:
         button_mod->set_time.tm_mon++;
         if (button_mod->set_time.tm_mon > max_time.tm_mon)
           button_mod->set_time.tm_mon = min_time.tm_mon;
         break;
      case ELM_DATETIME_DATE:
         button_mod->set_time.tm_mday++;
         if (button_mod->set_time.tm_mday > max_time.tm_mday)
           button_mod->set_time.tm_mday = min_time.tm_mday;
         break;
      case ELM_DATETIME_HOUR:
         button_mod->set_time.tm_hour++;
         if (button_mod->set_time.tm_hour > max_time.tm_hour)
           button_mod->set_time.tm_hour = min_time.tm_hour;
         break;
      case ELM_DATETIME_MINUTE:
         button_mod->set_time.tm_min++;
         if (button_mod->set_time.tm_min > max_time.tm_min)
           button_mod->set_time.tm_min = min_time.tm_min;
         break;
      case ELM_DATETIME_AMPM:
      default:
         break;
     }
   elm_datetime_value_set(datetime, &(button_mod->set_time));
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_value_decrement_cb(void *data)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;
   Evas_Object *datetime = button_mod->mod_data.base;
   struct tm min_time, max_time;

   int on_field = elm_radio_value_get(button_mod->group);

   elm_datetime_value_get(datetime, &(button_mod->set_time));
   button_mod->mod_data.fields_min_max_get(datetime, &(button_mod->set_time), &min_time, &max_time);
   switch (on_field)
     {
      case ELM_DATETIME_YEAR:
         button_mod->set_time.tm_year--;
         if (button_mod->set_time.tm_year < min_time.tm_year)
           button_mod->set_time.tm_year = max_time.tm_year;
         break;
      case ELM_DATETIME_MONTH:
         button_mod->set_time.tm_mon--;
         if (button_mod->set_time.tm_mon < min_time.tm_mon)
           button_mod->set_time.tm_mon = max_time.tm_mon;
         break;
      case ELM_DATETIME_DATE:
         button_mod->set_time.tm_mday--;
         if (button_mod->set_time.tm_mday < min_time.tm_mday)
           button_mod->set_time.tm_mday = max_time.tm_mday;
         break;
      case ELM_DATETIME_HOUR:
         button_mod->set_time.tm_hour--;
         if (button_mod->set_time.tm_hour < min_time.tm_hour)
           button_mod->set_time.tm_hour = max_time.tm_hour;
         break;
      case ELM_DATETIME_MINUTE:
         button_mod->set_time.tm_min--;
         if (button_mod->set_time.tm_min < min_time.tm_min)
           button_mod->set_time.tm_min = max_time.tm_min;
         break;
      case ELM_DATETIME_AMPM:
      default:
         break;
     }
   elm_datetime_value_set(datetime, &(button_mod->set_time));
   return ECORE_CALLBACK_RENEW;
}

static void
_wheel_right(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _value_increment_cb(data);
}

static void
_wheel_left(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _value_decrement_cb(data);
}

static void _pm_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;
   Evas_Object *datetime = button_mod->mod_data.base;

   elm_datetime_value_get(datetime, &(button_mod->set_time));
   if (button_mod->set_time.tm_hour >= STRUCT_TM_TIME_12HRS_MAX_VALUE)
     return;

   button_mod->set_time.tm_hour += STRUCT_TM_TIME_12HRS_MAX_VALUE;
   elm_datetime_value_set(datetime, &(button_mod->set_time));
}

static void _am_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;
   Evas_Object *datetime = button_mod->mod_data.base;

   elm_datetime_value_get(datetime, &(button_mod->set_time));
   if (button_mod->set_time.tm_hour < STRUCT_TM_TIME_12HRS_MAX_VALUE)
     return;

   button_mod->set_time.tm_hour -= STRUCT_TM_TIME_12HRS_MAX_VALUE;
   elm_datetime_value_set(datetime, &(button_mod->set_time));
}

static void _normal_radio_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;
   Evas_Object *datetime = button_mod->mod_data.base;

   elm_object_signal_emit(datetime, "elm,action,hide,ampm_button", "");
}

static void _ampm_radio_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;
   _ampm_button_show(button_mod);
}

static Eina_Bool
_longpress_right_button_timer_cb(void *data)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;

   button_mod->longpress_timer = NULL;
   if (button_mod->spin_timer) ecore_timer_del(button_mod->spin_timer);

   button_mod->spin_timer = ecore_timer_add(ROTATE_TIME_NORMAL, _value_increment_cb, button_mod);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_longpress_left_button_timer_cb(void *data)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;

   button_mod->longpress_timer = NULL;
   if (button_mod->spin_timer) ecore_timer_del(button_mod->spin_timer);

   button_mod->spin_timer = ecore_timer_add(ROTATE_TIME_NORMAL, _value_decrement_cb, button_mod);
   return ECORE_CALLBACK_CANCEL;
}

static void
_button_right_pressed_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;

   if (button_mod->longpress_timer) ecore_timer_del(button_mod->longpress_timer);

   button_mod->longpress_timer = ecore_timer_add(_elm_config->longpress_timeout, _longpress_right_button_timer_cb, button_mod);
}

static void
_button_left_pressed_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;

   if (button_mod->longpress_timer) ecore_timer_del(button_mod->longpress_timer);

   button_mod->longpress_timer = ecore_timer_add(_elm_config->longpress_timeout, _longpress_left_button_timer_cb, button_mod);
}

static void
_released_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   Button_Module_Data *button_mod = (Button_Module_Data *)data;
   if (button_mod->longpress_timer)
     {
        ecore_timer_del(button_mod->longpress_timer);
        button_mod->longpress_timer = NULL;
     }
   if (button_mod->spin_timer)
     {
        ecore_timer_del(button_mod->spin_timer);
        button_mod->spin_timer = NULL;
     }
}

static void
_button_left_clicked_cb(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   _value_decrement_cb(data);
}

static void
_button_right_clicked_cb(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   _value_increment_cb(data);
}

static Evas_Object *
_wheel_button_field_create(Button_Module_Data *button_mod)
{
   Evas_Object *ly, *icon;
   Evas_Object *obj = button_mod->mod_data.base;

   ly = elm_layout_add(obj);
   elm_layout_theme_set(ly, "layout", "wheel_controller", "default");
   evas_object_size_hint_weight_set (ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_signal_callback_add(ly, "elm,state,wheel,right", "", _wheel_right, button_mod);
   elm_layout_signal_callback_add(ly, "elm,state,wheel,left", "", _wheel_left, button_mod);

   icon = elm_button_add(ly);
   elm_object_style_set(icon, "controller_minus");
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(icon, "clicked", _button_left_clicked_cb, button_mod);
   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_MOUSE_DOWN, _button_left_pressed_cb, button_mod);
   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_MOUSE_UP, _released_cb, button_mod);
   elm_object_part_content_set(ly, "elm.icon.left", icon);

   icon = elm_button_add(ly);
   elm_object_style_set(icon, "controller_plus");
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(icon, "clicked", _button_right_clicked_cb, button_mod);
   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_MOUSE_DOWN, _button_right_pressed_cb, button_mod);
   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_MOUSE_UP, _released_cb, button_mod);
   elm_object_part_content_set(ly, "elm.icon.right", icon);

   elm_object_part_content_set(obj, "button.field.wheel", ly);
   return ly;
}

static Evas_Object *
_ampm_button_field_create(Button_Module_Data *button_mod)
{
#ifdef BUILD_B2
   Evas_Object *ly, *rd1, *rd2;
   Evas_Object *obj = button_mod->mod_data.base;

   ly = elm_layout_add(obj);
   elm_layout_theme_set(ly, "layout", "AM_PM", "default");
   evas_object_size_hint_weight_set (ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

   rd1 = elm_radio_add(ly);
   elm_object_style_set(rd1, "AM");
   elm_radio_state_value_set(rd1, 0);
   elm_object_domain_translatable_text_set(rd1, PACKAGE, E_("AM"));
   evas_object_size_hint_align_set(rd1, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(rd1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(rd1, "changed", _am_clicked_cb, button_mod);
   elm_object_part_content_set(ly, "elm.icon.left", rd1);

   rd2 = elm_radio_add(ly);
   elm_object_style_set(rd2, "PM");
   elm_radio_state_value_set(rd2, 1);
   elm_object_domain_translatable_text_set(rd2, PACKAGE, E_("PM"));
   evas_object_size_hint_align_set(rd2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(rd2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(rd2, "changed", _pm_clicked_cb, button_mod);
   elm_object_part_content_set(ly, "elm.icon.right", rd2);
   elm_radio_group_add(rd2, rd1);
#else
   Evas_Object *ly, *icon;
   Evas_Object *obj = button_mod->mod_data.base;

   ly = elm_layout_add(obj);
   elm_layout_theme_set(ly, "layout", "AM_PM", "default");
   evas_object_size_hint_weight_set (ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

   icon = elm_button_add(ly);
   elm_object_style_set(icon, "AM");
   elm_object_domain_translatable_text_set(icon, PACKAGE, E_("AM"));
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(icon, "clicked", _am_clicked_cb, button_mod);
   elm_object_part_content_set(ly, "elm.icon.left", icon);

   icon = elm_button_add(ly);
   elm_object_style_set(icon, "PM");
   elm_object_domain_translatable_text_set(icon, PACKAGE, E_("PM"));
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(icon, "clicked", _pm_clicked_cb, button_mod);
   elm_object_part_content_set(ly, "elm.icon.right", icon);
#endif
   elm_object_part_content_set(obj, "button.field.ampm", ly);
   return ly;
}

static void
_weekday_loc_update(Button_Module_Data *button_mod)
{
   char *fmt;

   if (!button_mod) return;

   fmt = nl_langinfo(D_T_FMT);
   if (!strncmp(fmt, "%a", 2) || !strncmp(fmt, "%A", 2))
     button_mod->weekday_loc_first = EINA_TRUE;
   else
     button_mod->weekday_loc_first = EINA_FALSE;
}

static void
_module_format_change(Button_Module_Data *button_mod)
{
   if (!button_mod) return;

   _adjust_layout(button_mod);
   _select_first_field(button_mod);
   _weekday_loc_update(button_mod);
}

static void
_module_language_changed_cb(void *data,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   Button_Module_Data *button_mod;
   button_mod = (Button_Module_Data *)data;
   if (!button_mod) return;
}

static void
_weekday_show_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   Button_Module_Data *button_mod;
   int idx, loc, weekday_loc;

   button_mod = (Button_Module_Data *)data;
   if (!button_mod) return;

   button_mod->weekday_show = EINA_TRUE;

   weekday_loc = (button_mod->weekday_loc_first) ? 0 : 2;
   for (idx = 0; idx <= ELM_DATETIME_DATE; idx++)
     {
        button_mod->mod_data.field_location_get(button_mod->mod_data.base, idx, &loc);
        if (loc == weekday_loc)
         {
            field_value_display((Elm_Datetime_Module_Data *)button_mod,
                                button_mod->datetime_field[idx]);
            break;
         }
     }
}

static void
_weekday_hide_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   Button_Module_Data *button_mod;
   int idx, loc, weekday_loc;

   button_mod = (Button_Module_Data *)data;
   if (!button_mod) return;

   button_mod->weekday_show = EINA_FALSE;
   weekday_loc = (button_mod->weekday_loc_first) ? 0 : 2;
   for (idx = 0; idx <= ELM_DATETIME_DATE; idx++)
     {
        button_mod->mod_data.field_location_get(button_mod->mod_data.base, idx, &loc);
        if (loc == weekday_loc)
         {
            field_value_display((Elm_Datetime_Module_Data *)button_mod,
                                button_mod->datetime_field[idx]);
            break;
         }
     }
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
   Button_Module_Data *button_mod;
   Elm_Datetime_Field_Type  field_type;
   struct tm curr_time;
   char buf[BUFF_SIZE] = {0,};
   char weekday[BUFF_SIZE], label[BUFF_SIZE];
   int loc, weekday_loc;
   const char *fmt;
   Eina_Bool is_weekday_shown;
   UDateFormat *dt_formatter = NULL;
   UErrorCode status = U_ZERO_ERROR;
   UChar str[256];
   char *locale_tmp;
   char locale[32] = {0,};
   char *p = NULL;
   Evas_Object *ampm_group = NULL;
   UChar Pattern[256] = {0, };
   time_t time_val;
   UDate date;

   button_mod = (Button_Module_Data *)module_data;
   if (!button_mod || !obj) return;

   elm_datetime_value_get(button_mod->mod_data.base, &curr_time);
   field_type = (Elm_Datetime_Field_Type )evas_object_data_get(obj, "_field_type");
   fmt = button_mod->mod_data.field_format_get(button_mod->mod_data.base, field_type);
   button_mod->time_12hr_fmt = button_mod->mod_data.field_location_get(button_mod->mod_data.base, ELM_DATETIME_AMPM, NULL);

   locale_tmp = setlocale(LC_ALL, NULL);
   strcpy(locale, locale_tmp);

   if (locale[0] != '\0')
     {
        p = strstr(locale, ".UTF-8");
        if (p) *p = 0;
     }

   curr_time.tm_isdst = -1;
   time_val = (time_t)mktime(&curr_time);
   date = (UDate)time_val * 1000;

   switch (field_type)
     {
      case ELM_DATETIME_YEAR:
        if (!strcmp(fmt, "%y"))
          u_uastrncpy(Pattern, "yy", strlen("yy"));
        else
          u_uastrncpy(Pattern, "yyyy", strlen("yyyy"));
        break;

      case ELM_DATETIME_MONTH:
        if (!strcmp(fmt, "%B"))
          u_uastrncpy(Pattern, "MMMM", strlen("MMMM"));
        else if (!strcmp(fmt, "%m"))
          u_uastrncpy(Pattern, "MM", strlen("MM"));
        else
          u_uastrncpy(Pattern, "MMM", strlen("MMM"));
        break;

      case ELM_DATETIME_DATE:
        if (!strcmp(fmt, "%e"))
          u_uastrncpy(Pattern, "d", strlen("d"));
        else
          u_uastrncpy(Pattern, "dd", strlen("dd"));
         break;

      case ELM_DATETIME_HOUR:
        if (button_mod->time_12hr_fmt)
          u_uastrncpy(Pattern, "hh", strlen("hh"));
        else
          u_uastrncpy(Pattern, "HH", strlen("HH"));
        break;

      case ELM_DATETIME_MINUTE:
        u_uastrncpy(Pattern, "mm", strlen("mm"));
        break;

      case ELM_DATETIME_AMPM:
        break;

      default:
        break;
     }

   dt_formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, locale, NULL, -1, Pattern, -1, &status);
   udat_format(dt_formatter, date, str, sizeof(str), NULL, &status);
   u_austrcpy(buf, str);
   udat_close(dt_formatter);

   if ((!buf[0]) && ((!strcmp(fmt, "%p")) || (!strcmp(fmt, "%P"))))
     {
        if (button_mod->ampm_button_field)
          ampm_group = elm_object_part_content_get(button_mod->ampm_button_field, "elm.icon.right");
        if (curr_time.tm_hour < STRUCT_TM_TIME_12HRS_MAX_VALUE)
          {
             elm_object_domain_translatable_text_set(obj, PACKAGE, E_("AM"));
             if (ampm_group)
               elm_radio_value_set(ampm_group, 0);
          }
        else
          {
             elm_object_domain_translatable_text_set(obj, PACKAGE, E_("PM"));
             if (ampm_group)
               elm_radio_value_set(ampm_group, 1);
          }
     }
   else if (!button_mod->weekday_show)
     elm_object_text_set(obj, buf);
   else
     {
        /* FIXME: To restrict month wrapping because of summer time in some locales,
         * ignore day light saving mode in mktime(). */
        curr_time.tm_isdst = -1;
        mktime(&curr_time);
        strftime(weekday, sizeof(weekday), "%a", &curr_time);
        weekday_loc = (button_mod->weekday_loc_first) ? 0 : 2;
        is_weekday_shown = button_mod->mod_data.field_location_get(
                           button_mod->mod_data.base, field_type, &loc);
        if (!is_weekday_shown || (loc != weekday_loc))
          elm_object_text_set(obj, buf);
        else if (loc == 0)
          {
             snprintf(label, sizeof(label), "%s %s", weekday, buf);
             elm_object_text_set(obj, label);
          }
        else
          {
             snprintf(label, sizeof(label), "%s %s", buf, weekday);
             elm_object_text_set(obj, label);
          }
     }
}

EAPI Evas_Object *
field_create(Elm_Datetime_Module_Data *module_data, Elm_Datetime_Field_Type field_type)
{
   Button_Module_Data *button_mod;
   Evas_Object *field_obj;

   button_mod = (Button_Module_Data *)module_data;
   if (!button_mod) return NULL;

   field_obj = elm_radio_add(button_mod->mod_data.base);
   if (field_type < ELM_DATETIME_HOUR)
     elm_object_style_set(field_obj, "datetime/style3");
   else if (field_type < ELM_DATETIME_AMPM)
     elm_object_style_set(field_obj, "datetime/style1");
   else if (field_type == ELM_DATETIME_AMPM)
     elm_object_style_set(field_obj, "datetime/style2");
   elm_radio_state_value_set(field_obj, field_type);
   evas_object_data_set(field_obj, "_field_type", (void *)field_type);
   button_mod->datetime_field[field_type] = field_obj;
   if (field_type == ELM_DATETIME_AMPM)
     evas_object_smart_callback_add(field_obj, "changed", _ampm_radio_changed_cb, button_mod);
   else
     evas_object_smart_callback_add(field_obj, "changed", _normal_radio_changed_cb, button_mod);
   if (!button_mod->group)
     {
        button_mod->group = field_obj;
        elm_radio_value_set(button_mod->group, 0);
     }
   else
     {
        elm_radio_group_add(field_obj, button_mod->group);
     }
   if (!button_mod->wheel_button_field)
     button_mod->wheel_button_field = _wheel_button_field_create(button_mod);
   if (!button_mod->ampm_button_field)
     button_mod->ampm_button_field = _ampm_button_field_create(button_mod);

   _access_set(field_obj, field_type);

   return field_obj;
}

EAPI Elm_Datetime_Module_Data *
obj_hook(Evas_Object *obj)
{
   Button_Module_Data *button_mod;

   button_mod = ELM_NEW(Button_Module_Data);
   if (!button_mod) return NULL;

   evas_object_smart_callback_add(obj, "language,changed",
                                  _module_language_changed_cb, button_mod);
   elm_object_signal_callback_add(obj, "weekday,show", "",
                                  _weekday_show_cb, button_mod);
   elm_object_signal_callback_add(obj, "weekday,hide", "",
                                  _weekday_hide_cb, button_mod);

   button_mod->weekday_show = EINA_FALSE;
   button_mod->weekday_loc_first = EINA_TRUE;
   button_mod->group = NULL;
   button_mod->wheel_button_field = NULL;
   button_mod->ampm_button_field = NULL;
   button_mod->longpress_timer = NULL;
   button_mod->spin_timer = NULL;

   return ((Elm_Datetime_Module_Data*)button_mod);
}

EAPI void
obj_unhook(Elm_Datetime_Module_Data *module_data)
{
   Button_Module_Data *button_mod;

   button_mod = (Button_Module_Data *)module_data;
   if (!button_mod) return;

   if (button_mod)
     {
        if (button_mod->longpress_timer)
          ecore_timer_del(button_mod->longpress_timer);
        if (button_mod->spin_timer)
          ecore_timer_del(button_mod->spin_timer);
        free(button_mod);
        button_mod = NULL;
     }
}

EAPI void
obj_format_hook(Elm_Datetime_Module_Data *module_data)
{
   Button_Module_Data *button_mod;

   button_mod = (Button_Module_Data *)module_data;
   if (!button_mod) return;

   _module_format_change(button_mod);
}

EAPI void
obj_theme_hook(Elm_Datetime_Module_Data *module_data)
{
   Button_Module_Data *button_mod;
   Evas_Object *datetime;
   const char *style;

   button_mod = (Button_Module_Data *)module_data;
   if (!button_mod) return;
   datetime = button_mod->mod_data.base;
   style = elm_object_style_get(datetime);

   if (style)
     {
        if (!strcmp(style, "timepicker_layout"))
          button_mod->timepicker = EINA_TRUE;
        else
          button_mod->timepicker = EINA_FALSE;
      }

   _fields_visible_set(button_mod);
   _adjust_layout(button_mod);
   _select_first_field(button_mod);
}

EAPI void
obj_focus_hook(Elm_Datetime_Module_Data *module_data __UNUSED__)
{
   return;
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
