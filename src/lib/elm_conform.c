#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_conform.h"

#ifndef MIN
# define MIN(a, b) ((a) < (b)) ? (a) : (b)
#endif

#ifndef MAX
# define MAX(a, b) ((a) < (b)) ? (b) : (a)
#endif

EAPI const char ELM_CONFORMANT_SMART_NAME[] = "elm_conformant";
static char CONFORMANT_KEY[] = "_elm_conform_key";

#ifdef HAVE_ELEMENTARY_X
#define SUB_TYPE_COUNT 2
static char *sub_type[SUB_TYPE_COUNT] = { "elm_scroller", "elm_genlist" };
#endif


static const char INDICATOR_PART[] = "elm.swallow.indicator";
static const char VIRTUALKEYPAD_PART[] = "elm.swallow.virtualkeypad";
static const char CLIPBOARD_PART[] = "elm.swallow.clipboard";
static const char SOFTKEY_PART[] = "elm.swallow.softkey";

static const char SIG_VIRTUALKEYPAD_STATE_ON[] = "virtualkeypad,state,on";
static const char SIG_VIRTUALKEYPAD_STATE_OFF[] = "virtualkeypad,state,off";
static const char SIG_CLIPBOARD_STATE_ON[] = "clipboard,state,on";
static const char SIG_CLIPBOARD_STATE_OFF[] = "clipboard,state,off";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_VIRTUALKEYPAD_STATE_ON, ""},
   {SIG_VIRTUALKEYPAD_STATE_OFF, ""},
   {SIG_CLIPBOARD_STATE_ON, ""},
   {SIG_CLIPBOARD_STATE_OFF, ""},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.content"},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_CONFORMANT_SMART_NAME, _elm_conformant, Elm_Conformant_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

/* Example of env vars:
 * ILLUME_KBD="0, 0, 800, 301"
 * ILLUME_IND="0, 0, 800, 32"
 * ILLUME_STK="0, 568, 800, 32"
 */
static Eina_Bool
_conformant_part_geometry_get_from_env(const char *part,
                                       int *sx,
                                       int *sy,
                                       int *sw,
                                       int *sh)
{
   const char delimiters[] = " ,;";
   char *env_val, *token;
   char buf[PATH_MAX];
   int tsx, tsy, tsw;

   if (!(env_val = getenv(part))) return EINA_FALSE;

   /* strtok would modify env var if not copied to a buffer */
   strncpy(buf, env_val, sizeof(buf));
   buf[PATH_MAX - 1] = '\0';

   token = strtok(buf, delimiters);
   if (!token) return EINA_FALSE;
   tsx = atoi(token);

   token = strtok(NULL, delimiters);
   if (!token) return EINA_FALSE;
   tsy = atoi(token);

   token = strtok(NULL, delimiters);
   if (!token) return EINA_FALSE;
   tsw = atoi(token);

   token = strtok(NULL, delimiters);
   if (!token) return EINA_FALSE;
   *sh = atoi(token);

   *sx = tsx;
   *sy = tsy;
   *sw = tsw;

   return EINA_TRUE;
}

static void
_conformant_part_size_hints_set(Evas_Object *obj,
                                Evas_Object *sobj,
                                Evas_Coord sx,
                                Evas_Coord sy,
                                Evas_Coord sw,
                                Evas_Coord sh)
{
   Evas_Coord cx, cy, cw, ch;
   Evas_Coord part_height = 0, part_width = 0;

   evas_object_geometry_get(obj, &cx, &cy, &cw, &ch);

   /* Part overlapping with conformant */
   if ((cx < (sx + sw)) && ((cx + cw) > sx)
       && (cy < (sy + sh)) && ((cy + ch) > sy))
     {
        part_height = MIN((cy + ch), (sy + sh)) - MAX(cy, sy);
        part_width = MIN((cx + cw), (sx + sw)) - MAX(cx, sx);
     }

   evas_object_size_hint_min_set(sobj, part_width, part_height);
   evas_object_size_hint_max_set(sobj, part_width, part_height);
}

static void
_conformant_part_sizing_eval(Evas_Object *obj,
                             Conformant_Part_Type part_type)
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window zone = 0;
   Evas_Object *top;
   Ecore_X_Window xwin;
#endif
   int sx = -1, sy = -1, sw = -1, sh = -1;

   ELM_CONFORMANT_DATA_GET(obj, sd);

#ifdef HAVE_ELEMENTARY_X
   top = elm_widget_top_get(obj);
   xwin = elm_win_xwindow_get(top);

   if (xwin)
     zone = ecore_x_e_illume_zone_get(xwin);
#endif

   if (part_type & ELM_CONFORMANT_INDICATOR_PART)
     {
        if ((!_conformant_part_geometry_get_from_env
               ("ILLUME_IND", &sx, &sy, &sw, &sh)) && (xwin))
          {
#ifdef HAVE_ELEMENTARY_X
             //No information of the indicator geometry, reset the geometry.
             if (!ecore_x_e_illume_indicator_geometry_get
                   (zone, &sx, &sy, &sw, &sh))
               sx = sy = sw = sh = 0;
#else
             ;
#endif
          }
        if (((sd->rot == 90) || (sd->rot == 270)) && sd->landscape_indicator)
          _conformant_part_size_hints_set(obj, sd->landscape_indicator, sx, sy, sw, sh);
        else if (((sd->rot == 0) || (sd->rot == 180)) && sd->portrait_indicator)
          _conformant_part_size_hints_set(obj, sd->portrait_indicator, sx, sy, sw, sh);
     }

   if ((part_type & ELM_CONFORMANT_VIRTUAL_KEYPAD_PART) && sd->virtualkeypad)
     {
        if ((!_conformant_part_geometry_get_from_env
               ("ILLUME_KBD", &sx, &sy, &sw, &sh)) && (xwin))
          {
#ifdef HAVE_ELEMENTARY_X
             //No information of the keyboard geometry, reset the geometry.
#ifdef __linux__
	         DBG("[KEYPAD]:pid=%d, xwin=0x%x, zone=0x%x: no env value and check window property.", getpid(), xwin, zone);
#endif
             if (!ecore_x_e_illume_keyboard_geometry_get(xwin, &sx, &sy, &sw, &sh))
               {
		          DBG("[KEYPAD]:no window property, check zone property.");
				  if (!ecore_x_e_illume_keyboard_geometry_get(zone, &sx, &sy, &sw, &sh))
                    {
		               DBG("[KEYPAD]:no zone property, reset value.");
                       sx = sy = sw = sh = 0;
				    }
               }
#else
             ;
#endif
          }
		DBG("[KEYPAD]: size(%d,%d, %dx%d).", sx, sy, sw, sh);
        _conformant_part_size_hints_set
          (obj, sd->virtualkeypad, sx, sy, sw, sh);
     }

   if ((part_type & ELM_CONFORMANT_SOFTKEY_PART) && sd->softkey)
     {
        if ((!_conformant_part_geometry_get_from_env
               ("ILLUME_STK", &sx, &sy, &sw, &sh)) && (xwin))
          {
#ifdef HAVE_ELEMENTARY_X
             //No information of the softkey geometry, reset the geometry.
             if (!ecore_x_e_illume_softkey_geometry_get
                   (zone, &sx, &sy, &sw, &sh))
               sx = sy = sw = sh = 0;
#else
             ;
#endif
          }
        _conformant_part_size_hints_set(obj, sd->softkey, sx, sy, sw, sh);
     }
   if ((part_type & ELM_CONFORMANT_CLIPBOARD_PART) && sd->clipboard)
     {
        if ((!_conformant_part_geometry_get_from_env
               ("ILLUME_CB", &sx, &sy, &sw, &sh)) && (xwin))
          {
#ifdef HAVE_ELEMENTARY_X
             //No information of the clipboard geometry, reset the geometry.
             if (!ecore_x_e_illume_clipboard_geometry_get
                   (zone, &sx, &sy, &sw, &sh))
               sx = sy = sw = sh = 0;
#else
             ;
#endif
          }
        _conformant_part_size_hints_set(obj, sd->clipboard, sx, sy, sw, sh);
     }
}

static void
_conformant_parts_swallow(Evas_Object *obj)
{
   Evas *e;
   Elm_Widget_Smart_Data *wd;

   ELM_CONFORMANT_DATA_GET(obj, sd);

   wd = ELM_WIDGET_DATA(sd);
   e = evas_object_evas_get(obj);

   sd->scroller = NULL;

   //Virtual Keyboard
   if (edje_object_part_exists(wd->resize_obj, VIRTUALKEYPAD_PART))
     {
        if (!sd->virtualkeypad)
          {
             sd->virtualkeypad = evas_object_rectangle_add(e);
             elm_widget_sub_object_add(obj, sd->virtualkeypad);
             evas_object_size_hint_max_set(sd->virtualkeypad, -1, 0);
          }
        else
          _conformant_part_sizing_eval(obj, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);

        evas_object_color_set(sd->virtualkeypad, 0, 0, 0, 0);
        elm_layout_content_set(obj, VIRTUALKEYPAD_PART, sd->virtualkeypad);
     }
   else if (sd->virtualkeypad)
     {
        evas_object_del(sd->virtualkeypad);
        sd->virtualkeypad = NULL;
     }

   //Clipboard
   if (edje_object_part_exists(wd->resize_obj, CLIPBOARD_PART))
     {
        if (!sd->clipboard)
          {
             sd->clipboard = evas_object_rectangle_add(e);
             evas_object_size_hint_min_set(sd->clipboard, -1, 0);
             evas_object_size_hint_max_set(sd->clipboard, -1, 0);
          }
        else
          _conformant_part_sizing_eval(obj, ELM_CONFORMANT_CLIPBOARD_PART);

        evas_object_color_set(sd->clipboard, 0, 0, 0, 0);
        elm_layout_content_set(obj, CLIPBOARD_PART, sd->clipboard);
     }
   else if (sd->clipboard)
     {
        evas_object_del(sd->clipboard);
        sd->clipboard = NULL;
     }

   //Softkey
   if (edje_object_part_exists(wd->resize_obj, SOFTKEY_PART))
     {
        if (!sd->softkey)
          {
             sd->softkey = evas_object_rectangle_add(e);
             evas_object_size_hint_min_set(sd->softkey, -1, 0);
             evas_object_size_hint_max_set(sd->softkey, -1, 0);
          }
        else
          _conformant_part_sizing_eval(obj, ELM_CONFORMANT_SOFTKEY_PART);

        evas_object_color_set(sd->softkey, 0, 0, 0, 0);
        elm_layout_content_set(obj, SOFTKEY_PART, sd->softkey);
     }
   else if (sd->softkey)
     {
        evas_object_del(sd->softkey);
        sd->softkey = NULL;
     }
}

static Eina_Bool
_port_indicator_connect_cb(void *data)
{
   const char   *indicator_serv_name;
   Elm_Conformant_Smart_Data *sd = data;
   int rot;

   if (!sd) return ECORE_CALLBACK_CANCEL;
   if (sd->indmode != ELM_WIN_INDICATOR_SHOW)
     {
        sd->port_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   rot = (int) evas_object_data_get(sd->portrait_indicator, CONFORMANT_KEY);
   indicator_serv_name = elm_config_indicator_service_get(rot);
   if (!indicator_serv_name)
     {
        DBG("[INDICATOR]Conformant cannot find indicator service name: Rotation=%d\n",rot);
        sd->port_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   if (elm_plug_connect(sd->portrait_indicator, indicator_serv_name, 0, EINA_FALSE))
     {
        DBG("[INDICATOR]Conformant connect to server[%s]\n", indicator_serv_name);
        sd->port_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}


static Eina_Bool
_land_indicator_connect_cb(void *data)
{
   const char   *indicator_serv_name;
   Elm_Conformant_Smart_Data *sd = data;
   int rot;

   if (!sd) return ECORE_CALLBACK_CANCEL;
   if (sd->indmode != ELM_WIN_INDICATOR_SHOW)
     {
        sd->land_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   rot = (int) evas_object_data_get(sd->landscape_indicator, CONFORMANT_KEY);
   indicator_serv_name = elm_config_indicator_service_get(rot);
   if (!indicator_serv_name)
     {
        DBG("[INDICATOR]Conformant cannot find indicator service name: Rotation=%d\n",rot);
        sd->land_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   if (elm_plug_connect(sd->landscape_indicator, indicator_serv_name, 0, EINA_FALSE))
     {
        DBG("[INDICATOR]Conformant connect to server[%s]\n", indicator_serv_name);
        sd->land_indi_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_land_indicator_disconnected(void *data,
                             Evas_Object *obj __UNUSED__,
                             void *event_info __UNUSED__)
{
   Evas_Object *conform = data;

   ELM_CONFORMANT_DATA_GET(conform, sd);

   if (sd->land_indi_timer) ecore_timer_del(sd->land_indi_timer);
   sd->land_indi_timer = ecore_timer_add(1, _land_indicator_connect_cb, sd);
}

static void
_port_indicator_disconnected(void *data,
                             Evas_Object *obj __UNUSED__,
                             void *event_info __UNUSED__)
{
   Evas_Object *conform = data;

   ELM_CONFORMANT_DATA_GET(conform, sd);

   if (sd->port_indi_timer) ecore_timer_del(sd->port_indi_timer);
   sd->port_indi_timer = ecore_timer_add(1, _port_indicator_connect_cb, sd);
}


static const char PLUG_KEY[] = "__Plug_Ecore_Evas";
// procotol version - change this as needed
#define MSG_DOMAIN_CONTROL_INDICATOR 0x10001
#define MSG_ID_INDICATOR_REPEAT_EVENT 0x10002
#define MSG_ID_INDICATOR_ROTATION 0x10003
#define MSG_ID_INDICATOR_OPACITY 0X1004
#define MSG_ID_INDICATOR_TYPE 0X1005

static void
_plug_msg_handle(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *conformant = data;
   Elm_Plug_Message *pm = event_info;
   int msg_domain = 0;
   int msg_id = 0;
   void *msg_data = NULL;
   int msg_data_size = 0;

   if (!conformant) return;
   ELM_CONFORMANT_CHECK(conformant) ;
   ELM_CONFORMANT_DATA_GET(conformant, sd);
   msg_domain = pm->msg_domain;
   msg_id = pm->msg_id;
   msg_data = pm->data;
   msg_data_size = pm->size;

   DBG("[INDICATOR]Receive msg from plug msg_domain=%x msg_id=%x size=%d", msg_domain, msg_id, msg_data_size);
   //get plug object form ee
   if (msg_domain == MSG_DOMAIN_CONTROL_INDICATOR)
     {
        if (msg_id == MSG_ID_INDICATOR_REPEAT_EVENT)
          {
              int *repeat = msg_data;
              DBG("[INDICATOR]Receive repeat event change message:(%d)", *repeat);
              if (1 == *repeat)
                {
                   evas_object_repeat_events_set(sd->landscape_indicator, EINA_TRUE);
                   evas_object_repeat_events_set(sd->portrait_indicator, EINA_TRUE);
                }
              else
                {
                   evas_object_repeat_events_set(sd->landscape_indicator, EINA_FALSE);
                   evas_object_repeat_events_set(sd->portrait_indicator, EINA_FALSE);
                }
          }
        if (msg_id == MSG_ID_INDICATOR_TYPE)
          {
             Elm_Win_Indicator_Type_Mode *indi_t_mode = msg_data;
             Evas_Object *win = elm_widget_top_get(conformant);
             DBG("[INDICATOR]Receive indicator type change message:(%d)", *indi_t_mode);
             elm_win_indicator_type_set(win, *indi_t_mode);
          }
     }
}

static Evas_Object *
_create_portrait_indicator(Evas_Object *obj)
{
   Evas_Object *port_indicator = NULL;
   const char *port_indicator_serv_name;

   ELM_CONFORMANT_DATA_GET(obj, sd);

   port_indicator_serv_name = elm_config_indicator_service_get(sd->rot);
   if (!port_indicator_serv_name)
     {
        DBG("[INDICATOR]Conformant cannot get portrait indicator service name\n");
        return NULL;
     }

   port_indicator = elm_plug_add(obj);
   if (!port_indicator)
     {
        DBG("[INDICATOR]Conformant cannot create plug to server[%s]\n", port_indicator_serv_name);
        return NULL;
     }

   if (!elm_plug_connect(port_indicator, port_indicator_serv_name, 0, EINA_FALSE))
     {
        DBG("[INDICATOR]Conformant cannot connect to server[%s]\n", port_indicator_serv_name);
        return NULL;
     }

   //callback to deal with extn socket message
   evas_object_smart_callback_add(port_indicator, "message.received", _plug_msg_handle, obj);

   DBG("[INDICATOR]Send msg to portrait indicator: rot(%d), opacity(%d)", sd->rot, sd->ind_o_mode);
   elm_plug_msg_send(port_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_ROTATION, &(sd->rot), sizeof(int));
   elm_plug_msg_send(port_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_OPACITY, &(sd->ind_o_mode), sizeof(Elm_Win_Indicator_Opacity_Mode));

   elm_widget_sub_object_add(obj, port_indicator);
   evas_object_smart_callback_add(port_indicator, "image.deleted", _port_indicator_disconnected, obj);

   evas_object_size_hint_min_set(port_indicator, -1, 0);
   evas_object_size_hint_max_set(port_indicator, -1, 0);

   return port_indicator;
}

static Evas_Object *
_create_landscape_indicator(Evas_Object *obj)
{
   Evas_Object *land_indicator = NULL;
   const char *land_indicator_serv_name;

   ELM_CONFORMANT_DATA_GET(obj, sd);

   land_indicator_serv_name = elm_config_indicator_service_get(sd->rot);
   if (!land_indicator_serv_name)
     {
        DBG("[INDICATOR]Conformant cannot get portrait indicator service name\n");
        return NULL;
     }

   land_indicator = elm_plug_add(obj);
   if (!land_indicator)
     {
        DBG("[INDICATOR]Conformant cannot create plug to server[%s]\n", land_indicator_serv_name);
        return NULL;
     }

   if (!elm_plug_connect(land_indicator, land_indicator_serv_name, 0, EINA_FALSE))
     {
        DBG("[INDICATOR]Conformant cannot connect to server[%s]\n", land_indicator_serv_name);
        return NULL;
     }

   //callback to deal with extn socket message
   evas_object_smart_callback_add(land_indicator, "message.received", _plug_msg_handle, obj);

   DBG("[INDICATOR]Send msg to landscape indicator: rot(%d), opacity(%d)", sd->rot, sd->ind_o_mode);
   elm_plug_msg_send(land_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_ROTATION, &(sd->rot), sizeof(int));
   elm_plug_msg_send(land_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_OPACITY, &(sd->ind_o_mode), sizeof(Elm_Win_Indicator_Opacity_Mode));

   elm_widget_sub_object_add(obj, land_indicator);
   evas_object_smart_callback_add(land_indicator, "image.deleted",_land_indicator_disconnected, obj);

   evas_object_size_hint_min_set(land_indicator, -1, 0);
   evas_object_size_hint_max_set(land_indicator, -1, 0);
   return land_indicator;
}

static void
_indicator_mode_set(Evas_Object *conformant, Elm_Win_Indicator_Mode indmode)
{
   Evas_Object *old_indi = NULL;
   Elm_Widget_Smart_Data *wd;
   ELM_CONFORMANT_DATA_GET(conformant, sd);
   sd->indmode = indmode;

   wd = ELM_WIDGET_DATA(sd);

   if (!edje_object_part_exists(wd->resize_obj, INDICATOR_PART))
     return;

   if (indmode == ELM_WIN_INDICATOR_SHOW)
     {
        old_indi = elm_layout_content_get(conformant, INDICATOR_PART);

        //create new indicator
        if (!old_indi)
          {
             if ((sd->rot == 90)||(sd->rot == 270))
               {
                  if (!sd->landscape_indicator)
                    sd->landscape_indicator = _create_landscape_indicator(conformant);

                  if (!sd->landscape_indicator) return;

                  evas_object_show(sd->landscape_indicator);
                  elm_layout_content_set(conformant, INDICATOR_PART, sd->landscape_indicator);
               }
             else
               {
                  if (!sd->portrait_indicator)
                    sd->portrait_indicator = _create_portrait_indicator(conformant);

                  if (!sd->portrait_indicator) return;

                  evas_object_show(sd->portrait_indicator);
                  elm_layout_content_set(conformant, INDICATOR_PART, sd->portrait_indicator);
               }

          }
        elm_object_signal_emit(conformant, "elm,state,indicator,show", "elm");
     }
   else
     {
        old_indi = elm_layout_content_get(conformant, INDICATOR_PART);
        if (old_indi)
          {
             evas_object_hide(old_indi);
          }
        elm_object_signal_emit(conformant, "elm,state,indicator,hide", "elm");
     }
}

static void
_indicator_opacity_set(Evas_Object *conformant, Elm_Win_Indicator_Opacity_Mode ind_o_mode)
{
   ELM_CONFORMANT_DATA_GET(conformant, sd);
   sd->ind_o_mode = ind_o_mode;
   //TODO: opacity change
   //send indicator information
   DBG("[INDICATOR]The opacity mode of indicator was changed:(%d) rot=%d", ind_o_mode, sd->rot);
   if (((sd->rot == 90) || (sd->rot == 270))
        && sd->landscape_indicator)
     {
        elm_plug_msg_send(sd->landscape_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_OPACITY, &(sd->ind_o_mode), sizeof(Elm_Win_Indicator_Opacity_Mode));
     }
   if (sd->portrait_indicator)
     {
        elm_plug_msg_send(sd->portrait_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_OPACITY, &(sd->ind_o_mode), sizeof(Elm_Win_Indicator_Opacity_Mode));
     }
   if (ind_o_mode == ELM_WIN_INDICATOR_TRANSLUCENT)
      elm_object_signal_emit(conformant, "elm,state,indicator,translucent", "elm");
   else if (ind_o_mode == ELM_WIN_INDICATOR_TRANSPARENT)
      elm_object_signal_emit(conformant, "elm,state,indicator,transparent", "elm");
   else
      elm_object_signal_emit(conformant, "elm,state,indicator,opaque", "elm");
}

static void
_on_indicator_mode_changed(void *data,
                    Evas_Object *obj,
                    void *event_info __UNUSED__)
{
   Evas_Object *conformant = data;
   Evas_Object *win = obj;

   Elm_Win_Indicator_Mode indmode;
   Elm_Win_Indicator_Opacity_Mode ind_o_mode;

   ELM_CONFORMANT_DATA_GET(conformant, sd);

   indmode = elm_win_indicator_mode_get(win);
   ind_o_mode = elm_win_indicator_opacity_get(win);
   if (indmode != sd->indmode)
     _indicator_mode_set(conformant, indmode);
   if (ind_o_mode != sd->ind_o_mode)
     _indicator_opacity_set(conformant, ind_o_mode);
}

static void
_on_rotation_changed(void *data,
              Evas_Object *obj,
              void *event_info __UNUSED__)
{
   int rot = 0;
   Evas_Object *win = obj;
   Evas_Object *conformant = data;
   Evas_Object *old_indi = NULL;

   ELM_CONFORMANT_DATA_GET(data, sd);

   rot = elm_win_rotation_get(win);

   if (rot == sd->rot) return;

   sd->rot = rot;

   //send indicator information
   if (((rot == 90) || (rot == 270))
       && sd->landscape_indicator)
     {
        DBG("[INDICATOR]The rotation value of landscape indicator was changed:(%d)", rot);
        elm_plug_msg_send(sd->landscape_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_ROTATION, &(sd->rot), sizeof(int));
     }
   if (((rot == 0) || (rot == 180))
       && sd->portrait_indicator)
     {
        DBG("[INDICATOR]The rotation value of portrait indicator was changed:(%d)", rot);
        elm_plug_msg_send(sd->portrait_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_ROTATION, &(sd->rot), sizeof(int));
     }
   old_indi = elm_layout_content_unset(conformant, INDICATOR_PART);
   /* this means ELM_WIN_INDICATOR_SHOW never be set.we don't need to change indicator type*/
   if (!old_indi) return;
   evas_object_hide(old_indi);

   if ((rot == 90) || (rot == 270))
     {
        if (!sd->landscape_indicator)
          sd->landscape_indicator = _create_landscape_indicator(conformant);

        if (!sd->landscape_indicator) return;

        evas_object_show(sd->landscape_indicator);
        evas_object_data_set(sd->landscape_indicator, CONFORMANT_KEY, (void *) rot);
        elm_layout_content_set(conformant, INDICATOR_PART, sd->landscape_indicator);
     }
   else
     {
        if (!sd->portrait_indicator)
          sd->portrait_indicator = _create_portrait_indicator(conformant);

        if (!sd->portrait_indicator) return;

        evas_object_show(sd->portrait_indicator);
        evas_object_data_set(sd->portrait_indicator, CONFORMANT_KEY, (void *) rot);
        elm_layout_content_set(conformant, INDICATOR_PART, sd->portrait_indicator);
     }
}

static void
_on_iconified(void *data,
              Evas_Object *obj,
              void *event_info __UNUSED__)
{
   Evas_Object *win = obj;
   Evas_Object *conformant = data;

   ELM_CONFORMANT_DATA_GET(data, sd);
   DBG("[INDICATOR]Window is iconified rot=%d indmode=%d ind_o_mode=%d", sd->rot, sd->indmode, sd->ind_o_mode);
   //TODO: send message related with iconified.

}

static void
_on_normal(void *data,
           Evas_Object *obj,
           void *event_info __UNUSED__)
{
   Evas_Object *win = obj;
   Evas_Object *conformant = data;

   ELM_CONFORMANT_DATA_GET(data, sd);
   DBG("[INDICATOR]Window is normal rot=%d indmode=%d ind_o_mode=%d", sd->rot, sd->indmode, sd->ind_o_mode);
   //TODO: send message related with iconified.
   if ((sd->rot == 90) || (sd->rot == 270))
     {
        if(!sd->landscape_indicator) return;
        elm_plug_msg_send(sd->landscape_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_ROTATION, &(sd->rot), sizeof(int));
        elm_plug_msg_send(sd->landscape_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_OPACITY, &(sd->ind_o_mode), sizeof(Elm_Win_Indicator_Opacity_Mode));
     }
   else
     {
        if(!sd->portrait_indicator) return;
        elm_plug_msg_send(sd->portrait_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_ROTATION, &(sd->rot), sizeof(int));
        elm_plug_msg_send(sd->portrait_indicator, MSG_DOMAIN_CONTROL_INDICATOR, MSG_ID_INDICATOR_OPACITY, &(sd->ind_o_mode), sizeof(Elm_Win_Indicator_Opacity_Mode));
     }
}
static Eina_Bool
_elm_conformant_smart_theme(Evas_Object *obj)
{
   if (!ELM_WIDGET_CLASS(_elm_conformant_parent_sc)->theme(obj))
     return EINA_FALSE;

   _conformant_parts_swallow(obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

// unused now - but meant to be for making sure the focused widget is always
// visible when the vkbd comes and goes by moving the conformant obj (and thus
// its children) to  show the focused widget (and if focus changes follow)
/*
   static Evas_Object *
   _focus_object_get(const Evas_Object *obj)
   {
   Evas_Object *win, *foc;

   win = elm_widget_top_get(obj);
   if (!win) return NULL;
   foc = elm_widget_top_get(win);
   }

   static void
   _focus_object_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
   {
   evas_object_geometry_get(obj, x, y, w, h);
   }

   static void
   _focus_change_del(void *data, Evas_Object *obj, void *event_info)
   {
   // called from toplevel when the focused window shanges
   }

   static void
   _autoscroll_move(Evas_Object *obj)
   {
   // move conformant edje by delta to show focused widget
   }

   static void
   _autoscroll_mode_enable(Evas_Object *obj)
   {
   // called when autoscroll mode should be on - content area smaller than
   // its min size
   // 1. get focused object
   // 2. if not in visible conformant area calculate delta needed to
   //    get it in
   // 3. store delta and call _autoscroll_move() which either asanimates
   //    or jumps right there
   }

   static void
   _autoscroll_mode_disable(Evas_Object *obj)
   {
   // called when autoscroll mode should be off - set delta to 0 and
   // call _autoscroll_move()
   }
 */

static void
_move_resize_cb(void *data __UNUSED__,
                Evas *e __UNUSED__,
                Evas_Object *obj,
                void *event_info __UNUSED__)
{
   Conformant_Part_Type part_type;

   part_type = (ELM_CONFORMANT_INDICATOR_PART |
                ELM_CONFORMANT_SOFTKEY_PART |
                ELM_CONFORMANT_VIRTUAL_KEYPAD_PART |
                ELM_CONFORMANT_CLIPBOARD_PART);

   _conformant_part_sizing_eval(obj, part_type);
}

static void
_show_region_job(void *data)
{
   Evas_Object *focus_obj;

   ELM_CONFORMANT_DATA_GET(data, sd);

   focus_obj = elm_widget_focused_object_get(data);
   if (focus_obj)
     {
        Evas_Coord x, y, w, h;

        elm_widget_show_region_get(focus_obj, &x, &y, &w, &h);


        if (h < _elm_config->finger_size)
          h = _elm_config->finger_size;

        elm_widget_show_region_set(focus_obj, x, y, w, h, EINA_TRUE);
     }

   sd->show_region_job = NULL;
}

// showing the focused/important region.
#ifdef HAVE_ELEMENTARY_X
static void
_add_show_region_job(void *data)
{
   ELM_CONFORMANT_DATA_GET(data, sd);

   if (sd->show_region_job) ecore_job_del(sd->show_region_job);
   sd->show_region_job = ecore_job_add(_show_region_job, data);
}

static void
_on_content_resize(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   ELM_CONFORMANT_DATA_GET(data, sd);

   if ((sd->vkb_state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF) &&
       (sd->clipboard_state == ECORE_X_ILLUME_CLIPBOARD_STATE_OFF))
     return;

   _add_show_region_job(data);
}

static void
_on_del(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   ELM_CONFORMANT_DATA_GET(data, sd);
   EINA_SAFETY_ON_NULL_RETURN(sd);

   if (sd->scroller)
     {
        evas_object_event_callback_del
               (sd->scroller, EVAS_CALLBACK_RESIZE, _on_content_resize);
        evas_object_event_callback_del
               (sd->scroller, EVAS_CALLBACK_DEL, _on_del);
     }
   sd->scroller = NULL;
}

static void
_autoscroll_objects_update(void *data)
{
   int i;
   const char *type;
   Evas_Object *sub, *top_scroller = NULL;

   ELM_CONFORMANT_DATA_GET(data, sd);

   sub = elm_widget_focused_object_get(data);
   //Look up for top most scroller in the focus object hierarchy
   //inside Conformant.

   while (sub)
     {
        type = elm_widget_type_get(sub);
        if (!strcmp(type, ELM_CONFORMANT_SMART_NAME)) break;

        for (i = 0; i < SUB_TYPE_COUNT; i++)
          if (!strcmp(type, sub_type[i]))
            {
               top_scroller = sub;
               break;
            }
        sub = elm_object_parent_widget_get(sub);
     }

   //If the scroller got changed by app, replace it.
   if (top_scroller != sd->scroller)
     {
        if (sd->scroller)
          evas_object_event_callback_del
            (sd->scroller, EVAS_CALLBACK_RESIZE, _on_content_resize);
        sd->scroller = top_scroller;

        if (sd->scroller)
          {
             evas_object_event_callback_add
               (sd->scroller, EVAS_CALLBACK_RESIZE, _on_content_resize, data);
             // We are getting sd->scroller provided by app, so we need to keep updating it accordingly.
             evas_object_event_callback_add
               (sd->scroller, EVAS_CALLBACK_DEL, _on_del, data);
          }
     }
}

static void
_virtualkeypad_state_change(Evas_Object *obj, Ecore_X_Event_Window_Property *ev)
{
   ELM_CONFORMANT_DATA_GET(obj, sd);

   Ecore_X_Window zone = ecore_x_e_illume_zone_get(ev->win);
   Ecore_X_Virtual_Keyboard_State prevstate = sd->vkb_state;
   Ecore_X_Virtual_Keyboard_State state =
      ecore_x_e_virtual_keyboard_state_get(ev->win);

   DBG("[KEYPAD]:window's state win=0x%x, state=%d.", ev->win, state);
   if (state == ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN)
     {
        state = ecore_x_e_virtual_keyboard_state_get(zone);
        DBG("[KEYPAD]:zone's state zone=0x%x, state=%d.", zone, state);
     }

   if (sd->vkb_state == state) return;
   sd->vkb_state = state;

   if (state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF)
     {
        DBG("[KEYPAD]:ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF");
        evas_object_size_hint_min_set(sd->virtualkeypad, -1, 0);
        evas_object_size_hint_max_set(sd->virtualkeypad, -1, 0);
        // Tizen Only - SIP regions for virtual keypad and clipboard are the same in Tizen
        if (sd->clipboard_state == ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN || sd->clipboard_state == ECORE_X_ILLUME_CLIPBOARD_STATE_OFF)
          elm_widget_display_mode_set(obj, EVAS_DISPLAY_MODE_NONE);
        evas_object_smart_callback_call(obj, SIG_VIRTUALKEYPAD_STATE_OFF, NULL);
     }
   else if (state == ECORE_X_VIRTUAL_KEYBOARD_STATE_ON)
     {
        DBG("[KEYPAD]:ECORE_X_VIRTUAL_KEYBOARD_STATE_ON");
        _conformant_part_sizing_eval(obj, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);
        elm_widget_display_mode_set(obj, EVAS_DISPLAY_MODE_COMPRESS);
        _autoscroll_objects_update(obj);
        if (prevstate == ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN)
            _add_show_region_job(obj);
        evas_object_smart_callback_call(obj, SIG_VIRTUALKEYPAD_STATE_ON, NULL);
     }
}

static void
_clipboard_state_change(Evas_Object *obj, Ecore_X_Event_Window_Property *ev)
{
   ELM_CONFORMANT_DATA_GET(obj, sd);

   Ecore_X_Illume_Clipboard_State prevstate = sd->clipboard_state;
   Ecore_X_Window zone = ecore_x_e_illume_zone_get(ev->win);
   Ecore_X_Illume_Clipboard_State state =
      ecore_x_e_illume_clipboard_state_get(ev->win);

   DBG("[CLIPBOARD]:window's state win=0x%x, state=%d.", ev->win, state);

   if (state == ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN)
     {
        state = ecore_x_e_illume_clipboard_state_get(ev->win);
        DBG("[CLIPBOARD]:zone's state zone=0x%x, state=%d.", zone, state);
     }

   if (sd->clipboard_state == state) return;
   sd->clipboard_state = state;

   if (state == ECORE_X_ILLUME_CLIPBOARD_STATE_OFF)
     {
        // Tizen Only - SIP regions for virtual keypad and clipboard are the same in Tizen
        edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj, "elm,state,clipboard,off", "elm");
        evas_object_size_hint_min_set(sd->clipboard, -1, 0);
        evas_object_size_hint_max_set(sd->clipboard, -1, 0);
        // Tizen Only - SIP regions for virtual keypad and clipboard are the same in Tizen
        if (sd->vkb_state == ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN || sd->vkb_state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF)
          elm_widget_display_mode_set(obj, EVAS_DISPLAY_MODE_NONE);
        evas_object_smart_callback_call(obj, SIG_CLIPBOARD_STATE_OFF, NULL);
     }
   else if(state == ECORE_X_ILLUME_CLIPBOARD_STATE_ON)
     {
        elm_widget_display_mode_set(obj, EVAS_DISPLAY_MODE_COMPRESS);
        _autoscroll_objects_update(obj);
        if (prevstate == ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN)
            _add_show_region_job(obj);
        // Tizen Only - SIP regions for virtual keypad and clipboard are the same in Tizen
        edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj, "elm,state,clipboard,on", "elm");
        evas_object_smart_callback_call(obj, SIG_CLIPBOARD_STATE_ON, NULL);
     }
}

static Eina_Bool
_on_prop_change(void *data,
                int type __UNUSED__,
                void *event)
{
   Ecore_X_Event_Window_Property *ev = event;

   int pid = 0;

#ifdef __linux__
   pid = (int)getpid();
#endif

   if (ev->atom == ECORE_X_ATOM_E_ILLUME_ZONE)
     {
		DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_ZONE.\n", pid, ev->win);
        Conformant_Part_Type part_type;

        part_type = (ELM_CONFORMANT_INDICATOR_PART |
                     ELM_CONFORMANT_SOFTKEY_PART |
                     ELM_CONFORMANT_VIRTUAL_KEYPAD_PART |
                     ELM_CONFORMANT_CLIPBOARD_PART);

        _conformant_part_sizing_eval(data, part_type);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY)
     {
		DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY.", pid, ev->win);
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_INDICATOR_PART);
	 }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY)
     {
		DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY.", pid, ev->win);
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_SOFTKEY_PART);
	 }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY)
     {
		DBG("[KEYPAD]:pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY.", pid, ev->win);
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_VIRTUAL_KEYPAD_PART);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY)
     {
		DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY.", pid, ev->win);
        _conformant_part_sizing_eval(data, ELM_CONFORMANT_CLIPBOARD_PART);
     }
   else if (ev->atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE)
     {
		DBG("[KEYPAD]:pid=%d, win=0x%x, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE.", pid, ev->win);
        _virtualkeypad_state_change(data, ev);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_CLIPBOARD_STATE)
     {
		DBG("pid=%d, win=0x%x, ECORE_X_ATOM_E_ILLUME_CLIPBOARD_STATE.", pid, ev->win);
        _clipboard_state_change(data, ev);
     }

   return ECORE_CALLBACK_PASS_ON;
}

#endif

static void
_elm_conformant_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Conformant_Smart_Data);

   ELM_WIDGET_CLASS(_elm_conformant_parent_sc)->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   elm_layout_theme_set(obj, "conformant", "base", elm_widget_style_get(obj));

   _conformant_parts_swallow(obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _move_resize_cb, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOVE, _move_resize_cb, obj);

   elm_layout_sizing_eval(obj);
}

static void
_elm_conformant_smart_del(Evas_Object *obj)
{
   Evas_Object *top;
   ELM_CONFORMANT_DATA_GET(obj, sd);

#ifdef HAVE_ELEMENTARY_X
   if (sd->prop_hdl) ecore_event_handler_del(sd->prop_hdl);
#endif
   if (sd->scroller)
     {
        evas_object_event_callback_del
               (sd->scroller, EVAS_CALLBACK_RESIZE, _on_content_resize);
        evas_object_event_callback_del
               (sd->scroller, EVAS_CALLBACK_DEL, _on_del);
     }

   if (sd->show_region_job) ecore_job_del(sd->show_region_job);
   if (sd->port_indi_timer) ecore_timer_del(sd->port_indi_timer);
   if (sd->land_indi_timer) ecore_timer_del(sd->land_indi_timer);
   if (sd->portrait_indicator)
     {
        evas_object_smart_callback_del(sd->portrait_indicator, "message.received", _plug_msg_handle);
        evas_object_del(sd->portrait_indicator);
     }
   if (sd->landscape_indicator)
     {
        evas_object_smart_callback_del(sd->landscape_indicator, "message.received", _plug_msg_handle);
        evas_object_del(sd->landscape_indicator);
     }
   top = sd->win;
   evas_object_data_set(top, "\377 elm,conformant", NULL);

   evas_object_smart_callback_del(top, "indicator,prop,changed",
                                  _on_indicator_mode_changed);
   evas_object_smart_callback_del(top, "rotation,changed",
                                  _on_rotation_changed);
   evas_object_smart_callback_del(top, "iconified", _on_iconified);
   evas_object_smart_callback_del(top, "normal", _on_normal);

   ELM_WIDGET_CLASS(_elm_conformant_parent_sc)->base.del(obj);
}

static void
_elm_conformant_smart_parent_set(Evas_Object *obj,
                                 Evas_Object *parent)
{
#ifdef HAVE_ELEMENTARY_X
   Evas_Object *top = elm_widget_top_get(parent);
   Ecore_X_Window xwin = elm_win_xwindow_get(parent);

   if ((xwin) && (!elm_win_inlined_image_object_get(top)))
     {
        ELM_CONFORMANT_DATA_GET(obj, sd);

        sd->prop_hdl = ecore_event_handler_add
            (ECORE_X_EVENT_WINDOW_PROPERTY, _on_prop_change, obj);
        sd->vkb_state = ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF;
        sd->clipboard_state = ECORE_X_ILLUME_CLIPBOARD_STATE_OFF;
     }
   // FIXME: get kbd region prop
#endif
}

static Eina_Bool
_elm_conformant_smart_content_set(Evas_Object *obj,
                                 const char *part,
                                 Evas_Object *content)
{
    ELM_CONFORMANT_DATA_GET(obj, sd);

    if (!ELM_CONTAINER_CLASS(_elm_conformant_parent_sc)->content_set
         (obj, part, content))
    return EINA_FALSE;

    /* The internal state of keyboard/clipboard is reset since the state may be set already earlier in which case
       the _show_region_job will not be called, causing the focused object to be in invisible area (in some scenarios)
    */
    sd->vkb_state = ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN;
    sd->clipboard_state = ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN;
    return EINA_TRUE;
}

static void
_elm_conformant_smart_set_user(Elm_Conformant_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_conformant_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_conformant_smart_del;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_conformant_smart_content_set;
   ELM_WIDGET_CLASS(sc)->parent_set = _elm_conformant_smart_parent_set;
   ELM_WIDGET_CLASS(sc)->theme = _elm_conformant_smart_theme;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
}

EAPI const Elm_Conformant_Smart_Class *
elm_conformant_smart_class_get(void)
{
   static Elm_Conformant_Smart_Class _sc =
     ELM_CONFORMANT_SMART_CLASS_INIT_NAME_VERSION(ELM_CONFORMANT_SMART_NAME);
   static const Elm_Conformant_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_conformant_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_conformant_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas_Object *top;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_conformant_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   ELM_CONFORMANT_DATA_GET(obj, sd);

   top = elm_widget_top_get(obj);
   _on_indicator_mode_changed(obj, top, NULL);
   _on_rotation_changed(obj, top, NULL);

   sd->indmode = elm_win_indicator_mode_get(top);
   sd->ind_o_mode = elm_win_indicator_opacity_get(top);
   sd->rot = elm_win_rotation_get(top);
   evas_object_data_set(top, "\377 elm,conformant", obj);

   evas_object_smart_callback_add
     (top, "indicator,prop,changed", _on_indicator_mode_changed, obj);
   evas_object_smart_callback_add
     (top, "rotation,changed", _on_rotation_changed, obj);
   evas_object_smart_callback_add
     (top, "iconified", _on_iconified, obj);
   evas_object_smart_callback_add
     (top, "normal", _on_normal, obj);

   sd->win = top;

   return obj;
}
