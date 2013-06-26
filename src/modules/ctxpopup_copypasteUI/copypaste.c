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

Elm_Entry_Extension_data *ext_mod;
static int _mod_hook_count = 0;

typedef struct _Elm_Entry_Context_Menu_Item Elm_Entry_Context_Menu_Item;

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

static void _ctxpopup_hide(Evas_Object *popup);
static void _ctxpopup_position(Evas_Object *obj);
static void _ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
void obj_longpress(Evas_Object *obj);

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
_parent_mouse_down_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if ((!ext_mod) || (!ext_mod->popup))
     return;
   evas_object_hide(ext_mod->popup);
   ext_mod->mouse_up = EINA_FALSE;
   ext_mod->entry_move = EINA_FALSE;
}

static void
_entry_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (data)
     {
        if (ext_mod)
          {
             evas_object_event_callback_del(ext_mod->ctx_par, EVAS_CALLBACK_MOUSE_DOWN,
                                            _parent_mouse_down_cb);
          }
        _ctxpopup_hide(data);
        evas_object_del(data);
        data = NULL;
     }
}

static void
_entry_hide_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_hide(data);
}

static void
_entry_mouse_down_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if ((!ext_mod) || (!ext_mod->popup))
     return;
   ext_mod->mouse_up = EINA_FALSE;
   ext_mod->mouse_down = EINA_TRUE;
}

static Eina_Bool
_ctx_show(void *data)
{
   ext_mod->show_timer = NULL;
   if (!data) return ECORE_CALLBACK_CANCEL;
   if (elm_entry_selection_get(data))
     {
        if (ext_mod->popup_showing)
          {
             obj_longpress(data);
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_entry_mouse_up_cb(void *data  __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   if (!ext_mod)
     return;
   if (ext_mod->mouse_down && ext_mod->entry_move)
     {
        evas_object_smart_callback_del(ext_mod->popup, "dismissed", _ctxpopup_dismissed_cb);
        if (ext_mod->show_timer) ecore_timer_del(ext_mod->show_timer);
        ext_mod->show_timer = ecore_timer_add(0.1, _ctx_show, obj);
     }
   ext_mod->mouse_up = EINA_TRUE;
   ext_mod->mouse_down = EINA_FALSE;
}

static void
_entry_move_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   if (!ext_mod)
     return;
   ext_mod->entry_move = EINA_TRUE;
   if (ext_mod->mouse_up)
     {
        evas_object_smart_callback_del(ext_mod->popup, "dismissed", _ctxpopup_dismissed_cb);
        if (ext_mod->show_timer) ecore_timer_del(ext_mod->show_timer);
        ext_mod->show_timer = ecore_timer_add(0.1, _ctx_show, obj);
     }
}

static void
_ctxpopup_hide(Evas_Object *popup)
{
   ext_mod->popup_showing = EINA_FALSE;
   evas_object_hide(popup);
   evas_object_event_callback_del(ext_mod->caller, EVAS_CALLBACK_DEL, _entry_del_cb);
   evas_object_event_callback_del(ext_mod->caller, EVAS_CALLBACK_HIDE, _entry_hide_cb);
   evas_object_event_callback_del(ext_mod->caller, EVAS_CALLBACK_MOVE, _entry_move_cb);
}

static Eina_Bool
_selection_in_viewport_check()
{
   if (!ext_mod) return EINA_FALSE;

   Evas_Coord sx, sy, sw, sh;
   if (edje_object_part_text_selection_geometry_get(ext_mod->ent, "elm.text", &sx, &sy, &sw, &sh))
     {
        if ((ext_mod->viewport_rect.x != -1) || (ext_mod->viewport_rect.y != -1) ||
            (ext_mod->viewport_rect.w != -1) || (ext_mod->viewport_rect.h != -1))
          {
             Eina_Rectangle vp;
             Eina_Rectangle sel;

             vp.x = ext_mod->viewport_rect.x;
             vp.y = ext_mod->viewport_rect.y;
             vp.w = ext_mod->viewport_rect.w;
             vp.h = ext_mod->viewport_rect.h;
             sel.x = sx;
             sel.y = sy;
             sel.w = sw;
             sel.h = sh;
             if (eina_rectangle_intersection(&sel, &vp))
               {
                  return EINA_TRUE;
               }
             return EINA_FALSE;
          }
        return EINA_TRUE;
     }
   return EINA_TRUE;
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
   elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_UP,
                                       ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_LEFT,
                                       ELM_CTXPOPUP_DIRECTION_RIGHT);
   if (!edje_object_part_text_selection_geometry_get(ext_mod->ent, "elm.text", &sx, &sy, &sw, &sh))
     { //cannot get selection shape
        Evas_Coord cx, cy, cw, ch;

        edje_object_part_text_cursor_geometry_get(ext_mod->ent, "elm.text",
                                                  &cx, &cy, &cw, &ch);
        x = ex + cx;
        y = ey + cy;
        evas_object_move(ext_mod->popup, x, y);
        if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_DOWN)
          {
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_DOWN,
                                                 ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_RIGHT);
             x = ex + cx;
             y = ey + cy + ch;
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
        /* Currently, we cannot get size of ctxpopup, we must set hard value for threshold.
           After fixing ctxpopup, we can get correctly threshold */
        Evas_Coord threshold = 300;

        edje_object_part_text_selection_handler_geometry_get(ext_mod->ent, "elm.text",
                                                             EDJE_SELECTION_HANDLER_START,
                                                             &shx, &shy, &shw, &shh);
        edje_object_part_text_selection_handler_geometry_get(ext_mod->ent, "elm.text",
                                                             EDJE_SELECTION_HANDLER_END,
                                                             &ehx, &ehy, &ehw, &ehh);
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
                       y = ey + shy;
                       w = x2 - x;
                       evas_object_move(ext_mod->popup, x + w/2, y);
                       if (elm_ctxpopup_direction_get(ext_mod->popup) != ELM_CTXPOPUP_DIRECTION_UP)
                         {
                            y = sy - gap;
                         }
                    }
                  else
                    y = sy;
               }
             if (x2 > vx + vw) x2 = vx + vw;
             if (y2 > vy + vh) y2 = vy + vh;
             w = x2 - x;
             h = y2 - y;
             evas_object_move(ext_mod->popup, x + w/2, y);
             if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_UP)
               return;
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
                       y = ey + shy;
                       w = x2 - x;
                       evas_object_move(ext_mod->popup, x + w/2, y);
                       if (elm_ctxpopup_direction_get(ext_mod->popup) != ELM_CTXPOPUP_DIRECTION_UP)
                         {
                            y = sy - gap;
                         }
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
        evas_object_move(ext_mod->popup, x, y);

        if (elm_ctxpopup_direction_get(ext_mod->popup) != ELM_CTXPOPUP_DIRECTION_UP)
          {
             Eina_Rectangle vp;
             Eina_Rectangle sel; //selection area which is not covered by start,end sel handlers
             Evas_Coord cry;

             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_DOWN,
                                                 ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_RIGHT);
             vp.x = ext_mod->viewport_rect.x;
             vp.y = ext_mod->viewport_rect.y;
             vp.w = ext_mod->viewport_rect.w;
             vp.h = ext_mod->viewport_rect.h;
             sel.x = sx;
             sel.y = sy > ey + shy + shh ? sy : ey + shy + shh;
             sel.w = sw;
             cry = sy + sh < ey + ehy ? sy + sh : ey + ehy;
             sel.h = cry - sel.y;
             if ((eina_rectangle_intersection(&sel, &vp)) && (sel.h > threshold))
               {
                  elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_UP,
                                                      ELM_CTXPOPUP_DIRECTION_DOWN,
                                                      ELM_CTXPOPUP_DIRECTION_LEFT,
                                                      ELM_CTXPOPUP_DIRECTION_RIGHT);
                  evas_object_move(ext_mod->popup, x, sel.y + sel.h/2);
                  if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_UP)
                    return;
               }

             y = sel.y + sel.h;
             if ((y < ey + ehy + ehh) && (ey + ehy < vp.y + vp.h)) //end handler is downside
               {
                  y = ey + ehy + ehh;
                  evas_object_move(ext_mod->popup, x, y);
                  if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_DOWN)
                    return;
                  y = sy + sh + gap;
                  evas_object_move(ext_mod->popup, x, y);
                  if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_DOWN)
                    return;
               }
             evas_object_move(ext_mod->popup, x, y);
             if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_DOWN)
               return;

             // not enough space and small viewport (like landscape mode)
             y = sel.y;
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_DOWN,
                                                 ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_RIGHT);
             evas_object_move(ext_mod->popup, x, y);
             if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_UP)
               return;

             y = sel.y + sel.h;
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_DOWN,
                                                 ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_RIGHT);
             evas_object_move(ext_mod->popup, x, y);
             if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_DOWN)
               return;

             x = sel.x;
             y = sel.y + sel.h/2;
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_RIGHT,
                                                 ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_DOWN);
             evas_object_move(ext_mod->popup, x, y);
             if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_LEFT)
               return;

             x = sel.x + sel.w;
             evas_object_move(ext_mod->popup, x, y);
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_RIGHT,
                                                 ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_DOWN);
             if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_RIGHT)
               return;

             x = sel.x + sel.w/2;
             evas_object_move(ext_mod->popup, x, y);
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_RIGHT,
                                                 ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_DOWN);
             if ((elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_LEFT) ||
                 (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_RIGHT))
               return;

             x = sel.x + sel.w;
             evas_object_move(ext_mod->popup, x, y);
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_RIGHT,
                                                 ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_DOWN);
             if (elm_ctxpopup_direction_get(ext_mod->popup) == ELM_CTXPOPUP_DIRECTION_LEFT)
               return;

             x = sel.x;
             evas_object_move(ext_mod->popup, x, y);
             elm_ctxpopup_direction_priority_set(ext_mod->popup, ELM_CTXPOPUP_DIRECTION_RIGHT,
                                                 ELM_CTXPOPUP_DIRECTION_LEFT,
                                                 ELM_CTXPOPUP_DIRECTION_UP,
                                                 ELM_CTXPOPUP_DIRECTION_DOWN);
          }
     }
}

static void
_select_all(void *data, Evas_Object *obj, void *event_info)
{
   if((!ext_mod) || (!data)) return;

   ext_mod->selectall(data, obj, event_info);
   _ctxpopup_hide(obj);
}

static void
_select(void *data, Evas_Object *obj, void *event_info)
{
   if((!ext_mod) || (!data)) return;

   ext_mod->select(data, obj, event_info);
   _ctxpopup_hide(obj);
}

static void
_paste(void *data, Evas_Object *obj, void *event_info)
{
   if((!ext_mod) || (!data)) return;

   ext_mod->paste(data, obj, event_info);
   _ctxpopup_hide(obj);
}

static void
_cut(void *data, Evas_Object *obj, void *event_info)
{
   if((!ext_mod) || (!data)) return;

   ext_mod->cut(data, obj, event_info);
   _ctxpopup_hide(obj);
}

static void
_copy(void *data, Evas_Object *obj, void *event_info)
{
   if((!ext_mod) || (!data)) return;

   ext_mod->copy(data, obj, event_info);
   _ctxpopup_hide(obj);
}

static void
_cancel(void *data, Evas_Object *obj, void *event_info)
{
   if((!ext_mod) || (!data)) return;

   ext_mod->cancel(data, obj, event_info);
   _ctxpopup_hide(obj);
}

static void
_search_menu(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   if(!ext_mod) return;

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
_clipboard_menu(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
_item_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
        if (ext_mod->mouse_up)
          {
             _ctxpopup_position(obj);
             evas_object_show(ext_mod->popup);
             _ctxpopup_position(obj);
          }
        else
          {
             elm_entry_select_none(data);
             ext_mod->popup_showing = EINA_FALSE;
          }
     }
   else if (ext_mod->popup_showing)
     {
        if (_selection_in_viewport_check())
          {
             if (ext_mod->show_timer) ecore_timer_del(ext_mod->show_timer);
             ext_mod->show_timer = ecore_timer_add(0.1, _ctx_show, data);

             _ctxpopup_position(obj);
             evas_object_show(ext_mod->popup);
             _ctxpopup_position(obj);
          }
     }
   ext_mod->mouse_up = EINA_FALSE;
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

// module funcs for the specific module type
EAPI void
obj_hook(Evas_Object *obj)
{
   _mod_hook_count++;
   //if(_mod_hook_count > 1) return;

   if(!ext_mod)
     {
        ext_mod = ELM_NEW(Elm_Entry_Extension_data);
        if (!ext_mod) return;
        elm_entry_extension_module_data_get(obj, ext_mod);
        ext_mod->mouse_up = EINA_FALSE;
        ext_mod->mouse_down = EINA_FALSE;
        ext_mod->entry_move = EINA_FALSE;
     }
}

EAPI void
obj_unhook(Evas_Object *obj __UNUSED__)
{
   _mod_hook_count--;
   if(_mod_hook_count > 0) return;

   if(ext_mod)
     {
        evas_object_event_callback_del(ext_mod->ctx_par, EVAS_CALLBACK_MOUSE_DOWN,
                                       _parent_mouse_down_cb);
        if (ext_mod->show_timer)
          {
             ecore_timer_del(ext_mod->show_timer);
             ext_mod->show_timer = NULL;
          }
        if (ext_mod->popup)
          {
             evas_object_del(ext_mod->popup);
             ext_mod->popup = NULL;
          }
        free(ext_mod);
        ext_mod = NULL;
     }
}

EAPI void
obj_longpress(Evas_Object *obj)
{
   if(!ext_mod) return;

   Evas_Object *ctxparent;
   Evas_Object *parent, *child;
   const Eina_List *l;
   const Elm_Entry_Context_Menu_Item *it;
   const char *context_menu_orientation;
   char buf[255];
   Evas_Object* icon;
   Elm_Object_Item *added_item = NULL;

   /*update*/
   elm_entry_extension_module_data_get(obj, ext_mod);
   if (ext_mod->context_menu)
     {
#ifdef HAVE_ELEMENTARY_X
        int cbhm_count = _cbhm_item_count_get(obj);
#endif
        if (ext_mod->popup)
          {
             evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _entry_del_cb);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_HIDE, _entry_hide_cb);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOVE, _entry_move_cb);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_DOWN, _entry_mouse_down_cb);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP, _entry_mouse_up_cb);
             evas_object_event_callback_del(ext_mod->ctx_par, EVAS_CALLBACK_MOUSE_DOWN,
                                            _parent_mouse_down_cb);
             evas_object_del(ext_mod->popup);
             ext_mod->popup = NULL;
          }
        ctxparent = elm_widget_top_get(obj);
        evas_smart_objects_calculate(evas_object_evas_get(obj));
        parent = elm_widget_parent_get(obj);
        child = obj;
        if (parent)
          {
             while(parent)
               {
                  if (!strcmp(elm_widget_type_get(parent), "elm_conformant"))
                    {
                       ctxparent = child;
                       break;
                    }
                  child = parent;
                  parent = elm_widget_parent_get(parent);
               }
          }
        ext_mod->ctx_par = ctxparent;

        if(ctxparent)
          {
             ext_mod->popup = elm_ctxpopup_add(ctxparent);
             elm_object_tree_focus_allow_set(ext_mod->popup, EINA_FALSE);
             evas_object_smart_callback_add(ext_mod->popup, "dismissed", _ctxpopup_dismissed_cb, obj);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _entry_del_cb, ext_mod->popup);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _entry_hide_cb, ext_mod->popup);
          }
        else
          {
             ext_mod->caller = NULL;
             return;
          }
        elm_object_style_set(ext_mod->popup, "copypaste");

        elm_ctxpopup_horizontal_set(ext_mod->popup, EINA_TRUE);
        context_menu_orientation = edje_object_data_get
           (ext_mod->ent, "context_menu_orientation");
        if ((context_menu_orientation) &&
            (!strcmp(context_menu_orientation, "vertical")))
          elm_ctxpopup_horizontal_set(ext_mod->popup, EINA_FALSE);

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
                  elm_image_file_set(icon, buf, NULL);
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
                       elm_image_file_set(icon, buf, NULL);
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
             if (_selection_in_viewport_check())
               {
                  _ctxpopup_position(obj);
                  evas_object_show(ext_mod->popup);
                  ext_mod->popup_showing = EINA_TRUE;
                  _ctxpopup_position(obj);
               }

             ext_mod->caller = obj;
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _entry_move_cb, ext_mod->popup);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                            _entry_mouse_down_cb, ext_mod->popup);
             evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                            _entry_mouse_up_cb, ext_mod->popup);
             evas_object_event_callback_add(ext_mod->ctx_par, EVAS_CALLBACK_MOUSE_DOWN,
                                            _parent_mouse_down_cb, ext_mod->popup);
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
