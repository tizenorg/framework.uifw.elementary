#include <Elementary.h>
#include "elm_module_priv.h"
#include "elm_priv.h"
#include <appsvc/appsvc.h>
#include "cbhm_helper.h"

#define MULTI_(id) dgettext("sys_string", #id)
#define S_SELECT MULTI_(IDS_COM_SK_SELECT)
#define S_SELECT_ALL MULTI_(IDS_COM_BODY_SELECT_ALL)
#define S_COPY MULTI_(IDS_COM_BODY_COPY)
#define S_CUT MULTI_(IDS_COM_BODY_CUT)
#define S_PASTE MULTI_(IDS_COM_BODY_PASTE)
#define S_CLIPBOARD MULTI_(IDS_COM_BODY_CLIPBOARD)
#define S_COPIED MULTI_(IDS_COM_POP_COPIED_TO_CLIPBOARD)

// TEMP: Will be done by clipboard - start
#define NOTIFY_TIMEOUT 2.0
#define DATA_KEY "_data"
// TEMP - end

Elm_Entry_Extension_data *ext_mod;
static int _mod_hook_count = 0;

typedef struct _Elm_Entry_Context_Menu_Item Elm_Entry_Context_Menu_Item;
typedef struct _Tickernoti_Data Tickernoti_Data;

struct _Elm_Entry_Context_Menu_Item
{
   Evas_Object *obj;
   const char *label;
   const char *icon_file;
   const char *icon_group;
   Elm_Icon_Type icon_type;
   Evas_Smart_Cb func;
   void *data;
};

// TEMP: Will be done by clipboard - start
struct _Tickernoti_Data
{
   Ecore_Timer *timer;
};
// TEMP - end

static void _ctxpopup_hide(Evas_Object *popup);
static void _ctxpopup_position(Evas_Object *obj);

static char *
_remove_tags(const char *str)
{
   char *ret;
   if (!str)
     return NULL;

   Eina_Strbuf *buf = eina_strbuf_new();
   if (!buf)
     return NULL;

   if (!eina_strbuf_append(buf, str))
     {
        eina_strbuf_free(buf);
        return NULL;
     }

   eina_strbuf_replace_all(buf, "<br>", " ");
   eina_strbuf_replace_all(buf, "<br/>", " ");
   eina_strbuf_replace_all(buf, "<ps>", " ");
   eina_strbuf_replace_all(buf, "<ps/>", " ");

   while (EINA_TRUE)
     {
        const char *temp = eina_strbuf_string_get(buf);

        char *startTag = NULL;
        char *endTag = NULL;

        startTag = strstr(temp, "<");
        if (startTag)
          endTag = strstr(startTag, ">");
        else
          break;
        if (!endTag || startTag > endTag)
          break;

        size_t sindex = startTag - temp;
        size_t eindex = endTag - temp + 1;
        if (!eina_strbuf_remove(buf, sindex, eindex))
          break;
     }
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_entry_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_del(data);
}

static void
_entry_hide_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_hide(data);
}

static void
_entry_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info __UNUSED__)
{
   if (evas_pointer_button_down_mask_get(e))
     _ctxpopup_hide(data);
   else
     {
        /*update*/
        elm_entry_extension_module_data_get(obj, ext_mod);
        _ctxpopup_position(data);
     }
}

static void
_entry_resize_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _ctxpopup_hide(data);
}

static void
_ctxpopup_hide(Evas_Object *popup)
{
   evas_object_hide(popup);
   evas_object_event_callback_del(ext_mod->caller, EVAS_CALLBACK_DEL, _entry_del_cb);
   evas_object_event_callback_del(ext_mod->caller, EVAS_CALLBACK_HIDE, _entry_hide_cb);
   evas_object_event_callback_del(ext_mod->caller, EVAS_CALLBACK_MOVE, _entry_move_cb);
   evas_object_event_callback_del(ext_mod->caller, EVAS_CALLBACK_RESIZE, _entry_resize_cb);
}

static void
_ctxpopup_position(Evas_Object *obj __UNUSED__)
{
   if(!ext_mod) return;

   Evas_Coord ex, ey;
   Evas_Coord sx, sy, sw, sh;
   Evas_Coord x, y, w, h;
   int gap = 22; //in GUI

   evas_object_geometry_get(ext_mod->ent, &ex, &ey, NULL, NULL);
   elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_RIGHT);
   if (!edje_object_part_text_selection_geometry_get(ext_mod->ent, "elm.text", &sx, &sy, &sw, &sh))
     { //cannot get selection shape
        Evas_Coord cx, cy, cw, ch;

        edje_object_part_text_cursor_geometry_get(ext_mod->ent, "elm.text",
                                                  &cx, &cy, &cw, &ch);

        Elm_Ctxpopup_Direction dir = elm_ctxpopup_direction_get(ext_mod->popup);
        if (dir == ELM_CTXPOPUP_DIRECTION_DOWN)
          {
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_RIGHT);
             x = ex + cx;
             y = ey + cy + ch;
          }
        else
          {
             x = ex + cx;
             y = ey + cy;
          }

        //limit ctx in viewport
        if (ext_mod->viewport_rect.x != -1 || ext_mod->viewport_rect.y != -1
            || ext_mod->viewport_rect.w != -1 || ext_mod->viewport_rect.h != -1)
          {
             if (ext_mod->viewport_rect.x > x)
               x = ext_mod->viewport_rect.x;
             else if (x > ext_mod->viewport_rect.x + ext_mod->viewport_rect.w)
               x = ext_mod->viewport_rect.x + ext_mod->viewport_rect.w;

             if (ext_mod->viewport_rect.y > y)
               y = ext_mod->viewport_rect.y;
             else if (y > ext_mod->viewport_rect.y + ext_mod->viewport_rect.h)
               y = ext_mod->viewport_rect.y + ext_mod->viewport_rect.h;
          }

        evas_object_move(ext_mod->popup, x, y);
        evas_object_resize(ext_mod->popup, cw, ch);
     }
   else //get selection shape
     {
        Evas_Coord shx, shy, shw, shh;
        Evas_Coord ehx, ehy, ehw, ehh;

        edje_object_part_text_selection_handler_geometry_get(ext_mod->ent, "elm.text", EDJE_SELECTION_HANDLER_START, &shx, &shy, &shw, &shh);
        edje_object_part_text_selection_handler_geometry_get(ext_mod->ent, "elm.text", EDJE_SELECTION_HANDLER_END, &ehx, &ehy, &ehw, &ehh);

        if (ext_mod->viewport_rect.x != -1 || ext_mod->viewport_rect.y != -1
            || ext_mod->viewport_rect.w != -1 || ext_mod->viewport_rect.h != -1)
          {
             Evas_Coord vx, vy, vw, vh, x2, y2;
             x2 = sx + sw;
             if ((ehh > 0) && (ey + ehy + ehh > sy + sh))
               {
                  //y2 = ey + ehy + ehh;
                  y2 = ey + ehy + gap;
               }
             else
               y2 = sy + sh;
             vx = ext_mod->viewport_rect.x;
             vy = ext_mod->viewport_rect.y;
             vw = ext_mod->viewport_rect.w;
             vh = ext_mod->viewport_rect.h;

             //limit ctx in viewport
             x = sx;
             if (sx < vx) x = vx;
             if (sy < vy)
               {
                  y = vy; //case: start of selection is behind the viewport
               }
             else
               {
                  if (ey + shy < sy) //case: start handler is upside
                    {
                       //y = ey + shy;
                       y = sy - gap;
                    }
                  else
                    y = sy;
               }
             if (x2 > vx + vw) x2 = vx + vw;
             if (y2 > vy + vh) y2 = vy + vh;
             w = x2 - x;
             h = y2 - y;
          }
        else //get selection & cannot get viewport
          {
             Evas_Coord ww, wh, x2, y2;
             x2 = sx + sw;
             if (ey + ehy + ehh > sy + sh)
               y2 = ey + ehy + ehh; //end handler is downside
             else
               y2 = sy + sh;

             ecore_x_window_size_get(ecore_x_window_root_first_get(), &ww, &wh);

             x = sx;
             if (sx < 0) x = 0;
             if (sy < 0)
               {
                  y = 0; //start of selection is negative
               }
             else
               {
                  if (ey + shy < sy) //start handler is upside
                    {
                       //y = ey + shy;
                       y = sy - gap;
                    }
                  else
                    y = sy;
               }
             if (x2 > ww) x2 = ww;
             if (y2 > wh) y2 = wh;
             w = x2 - x;
             h = y2 - y;
          }
        x = x + (w / 2);
        Elm_Ctxpopup_Direction dir = elm_ctxpopup_direction_get(ext_mod->popup);
        if (dir != ELM_CTXPOPUP_DIRECTION_UNKNOWN)
          {
             if (dir == ELM_CTXPOPUP_DIRECTION_DOWN)
               {
                  elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_RIGHT);
                  y = sy + sh;
                  if (y < ey + ehy + ehh)
                    {
                       //y = ey + ehy + ehh;
                       y =  sy + sh + gap; //ey + ehy + 8 + 22;
                    }
               }
          }
        evas_object_move(ext_mod->popup, x, y);
     }
}

// TEMP: Will be done by clipboard - start
static Eina_Bool
_hide_notify_cb(void *data __UNUSED__)
{
   Tickernoti_Data *td;
   if (!ext_mod) return ECORE_CALLBACK_CANCEL;
   if (!ext_mod->notify) return ECORE_CALLBACK_CANCEL;
   evas_object_hide(ext_mod->notify);
   elm_win_indicator_mode_set(ext_mod->notify, ELM_WIN_INDICATOR_SHOW);

   td = evas_object_data_get(ext_mod->notify, DATA_KEY);
   if (!td) return ECORE_CALLBACK_CANCEL;
   if (td->timer)
     ecore_timer_del(td->timer);
   td->timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_notify_create(Evas_Object *parent)
{
   Evas_Object *content;
   Tickernoti_Data *td;
   Evas_Object *content_edje;
   Evas_Coord nx, ny, nw, nh;
   Evas_Coord x, y, h;
   int angle;

   if (!ext_mod) return;

   if (ext_mod->notify)
     {
        free(evas_object_data_get(ext_mod->notify, DATA_KEY));
        evas_object_del(ext_mod->notify);
     }

   //win
   ext_mod->notify = elm_win_add(parent, NULL, ELM_WIN_NOTIFICATION);
   elm_win_borderless_set(ext_mod->notify, EINA_TRUE);
   elm_win_autodel_set(ext_mod->notify, EINA_TRUE);
   elm_win_alpha_set(ext_mod->notify, EINA_TRUE);
   evas_object_size_hint_weight_set(ext_mod->notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   //content
   content = elm_layout_add(ext_mod->notify);
   elm_layout_theme_set(content, "tickernoti", "base", "textonly");
   elm_object_text_set(content, S_COPIED);
   evas_object_show(content);
   evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(ext_mod->notify, content);

   //position
   content_edje = elm_layout_edje_get(content);
   edje_object_part_geometry_get(content_edje, "bg", NULL, NULL, NULL, &h);
   ecore_x_window_size_get(ecore_x_window_root_first_get(), &x, &y);

   angle = elm_win_rotation_get(elm_widget_top_get(parent));
   switch (angle)
     {
      case 0:
         nx = ny = 0;
         nw = x;
         nh = h;
         break;

      case 90:
         nx = ny = 0;
         nw = h;
         nh = y;
         break;

      case 180:
         nx = 0;
         ny = y - h;
         nw = x;
         nh = h;
         break;

      case 270:
      case -90:
         nx = x -h;
         ny = 0;
         nw = h;
         nh = y;
         angle = 270;
         break;

      default:
         nx = ny = 0;
         nw = x;
         nh = h;
         angle = 0;
         break;
     }
   evas_object_move(ext_mod->notify, nx, ny);
   evas_object_resize(ext_mod->notify, nw, nh);
   elm_win_rotation_set(ext_mod->notify, angle);

   //timer to hide notify
   td = (Tickernoti_Data *)calloc(1, sizeof(Tickernoti_Data));
   if (!td) return;
   td->timer = ecore_timer_add(NOTIFY_TIMEOUT, _hide_notify_cb, NULL);
   evas_object_data_set(ext_mod->notify, DATA_KEY, td);

   elm_win_prop_focus_skip_set(ext_mod->notify, EINA_TRUE);
   evas_object_show(ext_mod->notify);
   elm_win_indicator_mode_set(ext_mod->notify, ELM_WIN_INDICATOR_HIDE);
}
// TEMP - end

static void
_select_all(void *data, Evas_Object *obj, void *event_info)
{
   if(!ext_mod) return;

   ext_mod->selectall(data,obj,event_info);
   _ctxpopup_hide(obj);
}

static void
_select(void *data, Evas_Object *obj, void *event_info)
{
   if(!ext_mod) return;

   ext_mod->select(data,obj,event_info);
   _ctxpopup_hide(obj);
}

static void
_paste(void *data, Evas_Object *obj, void *event_info)
{
   if(!ext_mod) return;

   ext_mod->paste(data,obj,event_info);
   _ctxpopup_hide(obj);
}

static void
_cut(void *data, Evas_Object *obj, void *event_info)
{
   if(!ext_mod) return;

   ext_mod->cut(data,obj,event_info);
   _ctxpopup_hide(obj);

   _notify_create((Evas_Object *)data); // TEMP: will be done by clipboard

   //elm_object_scroll_freeze_pop(ext_mod->popup);
}

static void
_copy(void *data, Evas_Object *obj, void *event_info)
{
   if(!ext_mod) return;

   ext_mod->copy(data,obj,event_info);
   _ctxpopup_hide(obj);
   _notify_create((Evas_Object *) data); // TEMP: will be done by clipboard

   //elm_object_scroll_freeze_pop(ext_mod->popup);
}

static void
_cancel(void *data, Evas_Object *obj, void *event_info)
{
   if(!ext_mod) return;

   ext_mod->cancel(data,obj,event_info);
   _ctxpopup_hide(obj);
   //elm_object_scroll_freeze_pop(ext_mod->popup);
}

static void
_search_menu(void *data, Evas_Object *obj, void *event_info)
{
   if(!ext_mod) return;

   int ret;
   bundle *b = bundle_create();
   if (!b)
     {
        //printf("bundle_create() failed\n");
        return;
     }

   appsvc_set_operation(b, APPSVC_OPERATION_SEARCH);
   if (ext_mod->selmode)
     {
        const char *selection = elm_entry_selection_get(ext_mod->caller);
        if (selection)
          {
             char *str = _remove_tags(selection);
             if (str)
               {
                  appsvc_add_data(b, APPSVC_DATA_KEYWORD, str);
                  free(str);
               }
             else
               appsvc_add_data(b, APPSVC_DATA_KEYWORD, selection);
          }
     }
   appsvc_run_service(b, 0, NULL, NULL);
   bundle_free(b);
   _ctxpopup_hide(obj);
}

static void
_clipboard_menu(void *data, Evas_Object *obj, void *event_info)
{
   if(!ext_mod) return;

   // start for cbhm
#ifdef HAVE_ELEMENTARY_X
   ecore_x_selection_secondary_set(elm_win_xwindow_get(obj), "",1);
#endif
   if (ext_mod->cnp_mode != ELM_CNP_MODE_MARKUP)
     _cbhm_msg_send(data, "show0");
   else
     _cbhm_msg_send(data, "show1");
   _ctxpopup_hide(obj);
   // end for cbhm
}

static void
_item_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Entry_Context_Menu_Item *it = data;
   Evas_Object *obj2 = it->obj;

   if (it->func) it->func(it->data, obj2, NULL);
   _ctxpopup_hide(obj);
}

static void
_ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Ctxpopup_Direction dir;
   if (!ext_mod) return;

   dir = elm_ctxpopup_direction_get(obj);
   //clear selection if ctxpopup is dismissed by clicking (not parent resizing)
   if (dir != ELM_CTXPOPUP_DIRECTION_UNKNOWN)
     {
        elm_entry_select_none(data);
     }

   //elm_object_scroll_freeze_pop(ext_mod->popup);
}

// module api funcs needed
EAPI int
elm_modapi_init(void *m)
{
   return 1; // succeed always
}

EAPI int
elm_modapi_shutdown(void *m)
{
   return 1; // succeed always
}

// module funcs for the specific module type
EAPI void
obj_hook(Evas_Object *obj)
{
   _mod_hook_count++;
   //if(_mod_hook_count > 1) return;

   if(!ext_mod)
     {
        ext_mod = ELM_NEW(Elm_Entry_Extension_data);
        elm_entry_extension_module_data_get(obj,ext_mod);
     }
}

EAPI void
obj_unhook(Evas_Object *obj)
{
   Tickernoti_Data *td;

   _mod_hook_count--;
   if(_mod_hook_count > 0) return;

   if(ext_mod)
     {
        // TEMP: Will be done by clipboard - start
        if (ext_mod->notify)
          {
             evas_object_hide(ext_mod->notify);
             elm_win_indicator_mode_set(ext_mod->notify, ELM_WIN_INDICATOR_SHOW);
             td = evas_object_data_get(ext_mod->notify, DATA_KEY);
             if (td)
               {
                  if (td->timer)
                    ecore_timer_del(td->timer);
                  td->timer = NULL;
                  free(td);
                  td = NULL;
               }
          }
        // TEMP: Will be done by clipboard - end
        free(ext_mod);
        ext_mod = NULL;
     }
}

EAPI void
obj_longpress(Evas_Object *obj)
{
   if(!ext_mod) return;

   Evas_Object *top;
   const Eina_List *l;
   const Elm_Entry_Context_Menu_Item *it;
   const char *context_menu_orientation;
   char buf[255];
   Evas_Object* icon;
   Elm_Object_Item *added_item = NULL;

   /*update*/
   elm_entry_extension_module_data_get(obj,ext_mod);
   if (ext_mod->context_menu)
     {
#ifdef HAVE_ELEMENTARY_X
        int cbhm_count = _cbhm_item_count_get(obj);
#endif
        if (ext_mod->popup) evas_object_del(ext_mod->popup);
        //else elm_widget_scroll_freeze_push(obj);
        top = elm_widget_top_get(obj);
        if(top)
          {
             ext_mod->popup = elm_ctxpopup_add(top);
             elm_object_tree_focus_allow_set(ext_mod->popup, EINA_FALSE);
             evas_object_smart_callback_add(ext_mod->popup, "dismissed", _ctxpopup_dismissed_cb, obj);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _entry_del_cb, ext_mod->popup);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _entry_hide_cb, ext_mod->popup);
          }
        elm_object_style_set(ext_mod->popup,"copypaste");

        context_menu_orientation = edje_object_data_get
           (ext_mod->ent, "context_menu_orientation");
        if ((context_menu_orientation) &&
            (!strcmp(context_menu_orientation, "horizontal")))
          elm_ctxpopup_horizontal_set(ext_mod->popup, EINA_TRUE);

        if (!ext_mod->selmode)
          {
             if (!ext_mod->password)
               {
                  if (!elm_entry_is_empty(obj))
                    {
                       elm_ctxpopup_item_append(ext_mod->popup, S_SELECT, NULL, _select, obj );
                       added_item = elm_ctxpopup_item_append(ext_mod->popup, S_SELECT_ALL, NULL, _select_all, obj );
                    }
               }

#ifdef HAVE_ELEMENTARY_X
             if (cbhm_count)
#else
             if (1) // need way to detect if someone has a selection
#endif
               {
                  if (ext_mod->editable)
                    added_item = elm_ctxpopup_item_append(ext_mod->popup, S_PASTE, NULL, _paste, obj );
               }
             //elm_ctxpopup_item_append(wd->ctxpopup, NULL, "Selectall",_select_all, obj );
             // start for cbhm
#ifdef HAVE_ELEMENTARY_X
             if ((!ext_mod->password) && (ext_mod->editable) && (cbhm_count))
#else
             if ((!ext_mod->password) && (ext_mod->editable))
#endif
               {
                  added_item = elm_ctxpopup_item_append(ext_mod->popup, S_CLIPBOARD, NULL, _clipboard_menu, obj);  // Clipboard
                  //elm_ctxpopup_item_append(ext_mod->popup, "More", NULL, _clipboard_menu, obj );
               }
             // end for cbhm

             const char *entry_str;
             entry_str = elm_entry_selection_get(obj);
             if (entry_str)
               {
                  icon = elm_icon_add(ext_mod->popup);
                  snprintf(buf, sizeof(buf), "%s/images/copy&paste_icon_search.png", PACKAGE_DATA_DIR);
                  elm_icon_file_set(icon, buf, NULL);
                  added_item = elm_ctxpopup_item_append(ext_mod->popup, NULL, icon, _search_menu, obj);  // Search
               }
          }
        else
          {
             if (!ext_mod->password)
               {
                  if (ext_mod->have_selection)
                    {
                       added_item = elm_ctxpopup_item_append(ext_mod->popup, S_COPY, NULL, _copy, obj );
                       if (ext_mod->editable)
                         added_item = elm_ctxpopup_item_append(ext_mod->popup, S_CUT, NULL, _cut, obj );
#ifdef HAVE_ELEMENTARY_X
                       if (ext_mod->editable && cbhm_count)
#else
                       if (ext_mod->editable)
#endif
                         added_item = elm_ctxpopup_item_append(ext_mod->popup, S_PASTE, NULL, _paste, obj );
                    }
                  else
                    {
                       _cancel(obj,ext_mod->popup,NULL);
                       if (!elm_entry_is_empty(obj))
                         {
                            elm_ctxpopup_item_append(ext_mod->popup, S_SELECT, NULL, _select, obj );
                            added_item = elm_ctxpopup_item_append(ext_mod->popup, S_SELECT_ALL, NULL, _select_all, obj );
                         }
#ifdef HAVE_ELEMENTARY_X
                       if (cbhm_count)
#else
                       if (1) // need way to detect if someone has a selection
#endif
                         {
                            if (ext_mod->editable)
                              added_item = elm_ctxpopup_item_append(ext_mod->popup, S_PASTE, NULL, _paste, obj );
                         }
                    }
                  // start for cbhm
#ifdef HAVE_ELEMENTARY_X
                  if (ext_mod->editable && cbhm_count)
#else
                  if (ext_mod->editable)
#endif
                    {
                       added_item = elm_ctxpopup_item_append(ext_mod->popup, S_CLIPBOARD, NULL, _clipboard_menu, obj);  // Clipboard
                       //elm_ctxpopup_item_append(ext_mod->popup, "More", NULL, _clipboard_menu, obj );
                    }
                  // end for cbhm

                  const char *entry_str;
                  char *str;
                  entry_str = edje_object_part_text_get(ext_mod->ent, "elm.text");
                  str = _remove_tags(entry_str);
                  if (strcmp(str, "") != 0)
                    {
                       icon = elm_icon_add(ext_mod->popup);
                       snprintf(buf, sizeof(buf), "%s/images/copy&paste_icon_search.png", PACKAGE_DATA_DIR);
                       elm_icon_file_set(icon, buf, NULL);
                       added_item = elm_ctxpopup_item_append(ext_mod->popup, NULL, icon, _search_menu, obj);  // Search
                    }
              }
          }
        EINA_LIST_FOREACH(ext_mod->items, l, it)
          {
             added_item = elm_ctxpopup_item_append(ext_mod->popup, it->label, NULL, _item_clicked, it );
          }
        if (ext_mod->popup && added_item)
          {
             //elm_object_scroll_freeze_push(ext_mod->popup);
             _ctxpopup_position(obj);
             evas_object_show(ext_mod->popup);
             _ctxpopup_position(obj);
             ext_mod->caller = obj;
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _entry_move_cb, ext_mod->popup);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _entry_resize_cb, ext_mod->popup);
          }
        else
          ext_mod->caller = NULL;
     }
}

EAPI void
obj_mouseup(Evas_Object *obj)
{
   if (!obj || !ext_mod)
     return;
}


EAPI void
obj_hidemenu(Evas_Object *obj)
{
   if (!obj || !ext_mod || obj != ext_mod->caller)
     return;

   _ctxpopup_hide(ext_mod->popup);
   // if (ext_mod->popup) evas_object_del(ext_mod->popup);
}
