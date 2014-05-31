#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_panel.h"

#define THRESHOLD 10

EAPI const char ELM_PANEL_SMART_NAME[] = "elm_panel";

static const char ACCESS_OUTLINE_PART[] = "elm.swallow.content";

static const char SIG_SCROLL[] = "scroll";

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_SCROLL, ""}
};

static const Evas_Smart_Interface *_smart_interfaces[] =
{
   (Evas_Smart_Interface *)&ELM_SCROLLABLE_IFACE, NULL
};

EVAS_SMART_SUBCLASS_IFACE_NEW
  (ELM_PANEL_SMART_NAME, _elm_panel, Elm_Panel_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks,
  _smart_interfaces);

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   ELM_PANEL_DATA_GET(obj, sd);

   elm_widget_mirrored_set(sd->bx, rtl);
   elm_panel_orient_set(obj, elm_panel_orient_get(obj));
}

static void
_hidden_content_region_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h,
                           Eina_Bool anim)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->hidden)
     {
        //Move the content to the outside of the panel in order to be hidden.
        if (sd->orient == ELM_PANEL_ORIENT_TOP ||
            sd->orient == ELM_PANEL_ORIENT_LEFT)
          {
             if (anim) sd->s_iface->region_bring_in(obj, sd->panel_w, 0, w, h);
             else
               {
                  sd->s_iface->content_region_show(obj, sd->panel_w, 0, w, h);
                  if (!sd->hold)
                    {
                       sd->s_iface->hold_set(obj, EINA_TRUE);
                       sd->hold = EINA_TRUE;
                       elm_layout_signal_emit(sd->ly, "elm,state,content,hidden", "elm");
                       elm_object_signal_emit(obj, "elm,state,hold", "elm");
                       sd->scrolled = EINA_FALSE;
                    }
               }
          }
        else
          {
             //FIXME: Set position properly in case of panel visible.
             if (anim) sd->s_iface->region_bring_in(obj, 0, 0, w, h);
             else
               {
                  sd->s_iface->content_region_show(obj, 0, 0, w, h);
                  if (!sd->hold)
                    {
                       sd->s_iface->hold_set(obj, EINA_TRUE);
                       sd->hold = EINA_TRUE;
                       elm_layout_signal_emit(sd->ly, "elm,state,content,hidden", "elm");
                       elm_object_signal_emit(obj, "elm,state,hold", "elm");
                       sd->scrolled = EINA_FALSE;
                    }
               }
          }
     }
   else
     {
        if (sd->orient == ELM_PANEL_ORIENT_TOP ||
            sd->orient == ELM_PANEL_ORIENT_LEFT)
          {
             if (anim) sd->s_iface->region_bring_in(obj, 0, 0, w, h);
             else
               {
                  sd->s_iface->content_region_show(obj, 0, 0, w, h);
                  if (sd->hold)
                    {
                       sd->s_iface->hold_set(obj, EINA_FALSE);
                       sd->hold = EINA_FALSE;
                       elm_layout_signal_emit(sd->ly, "elm,state,content,visible", "elm");
                       elm_object_signal_emit(obj, "elm,state,unhold", "elm");
                    }
               }
          }
        else
          {
             if (anim) sd->s_iface->region_bring_in(obj, sd->panel_w, 0, w, h);
             else
               {
                  sd->s_iface->content_region_show(obj, sd->panel_w, 0, w, h);
                  if (sd->hold)
                    {
                       sd->s_iface->hold_set(obj, EINA_FALSE);
                       sd->hold = EINA_FALSE;
                       elm_layout_signal_emit(sd->ly, "elm,state,content,visible", "elm");
                       elm_object_signal_emit(obj, "elm,state,unhold", "elm");
                    }
               }
          }
     }
}

static void
_elm_panel_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord mw = -1, mh = -1;

   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->on_deletion) return;

   evas_object_smart_calculate(sd->bx);
   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &mw, &mh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static char *
_access_state_cb(void *data, Evas_Object *obj __UNUSED__)
{
   ELM_PANEL_DATA_GET(data, sd);

   if (!sd->hidden) return strdup(E_("state: opened"));
   else return strdup(E_("state: closed"));

   return NULL;
}

static void
_orient_set_do_internal(Evas_Object *obj)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if (!sd->scrollable) return;

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
        elm_layout_theme_set(sd->ly, "panel", "top/internal",
                             elm_widget_style_get(obj));
        break;
      case ELM_PANEL_ORIENT_BOTTOM:
        elm_layout_theme_set(sd->ly, "panel", "bottom/internal",
                             elm_widget_style_get(obj));
        break;
      case ELM_PANEL_ORIENT_LEFT:
        if (!elm_widget_mirrored_get(obj))
          {
             elm_layout_theme_set(sd->ly, "panel", "left/internal",
                                  elm_widget_style_get(obj));
          }
        else
          {
             elm_layout_theme_set(sd->ly, "panel", "right/internal",
                                  elm_widget_style_get(obj));
          }
        break;
      case ELM_PANEL_ORIENT_RIGHT:
        if (!elm_widget_mirrored_get(obj))
          {
             elm_layout_theme_set(sd->ly, "panel", "right/internal",
                                  elm_widget_style_get(obj));
          }
        else
          {
             elm_layout_theme_set(sd->ly, "panel", "left/internal",
                                 elm_widget_style_get(obj));
          }
        break;
     }
   const char *panel_w = edje_object_data_get(elm_layout_edje_get(sd->ly),
                                              "content_width");
   const char *panel_h = edje_object_data_get(elm_layout_edje_get(sd->ly),
                                              "content_height");
   sd->panel_w = (int) (elm_object_scale_get(obj) * (double) atoi(panel_w));
   sd->panel_h = (int) (elm_object_scale_get(obj) * (double) atoi(panel_h));
}

static Evas_Object *
_access_object_get(const Evas_Object *obj, const char *part)
{
   Evas_Object *po, *ao;
   ELM_PANEL_DATA_GET(obj, sd);

   po = (Evas_Object *)edje_object_part_object_get
      (elm_layout_edje_get(sd->ly), part);
   ao = evas_object_data_get(po, "_part_access_obj");

   return ao;
}

static void
_access_activate_cb(void *data,
                    Evas_Object *part_obj __UNUSED__,
                    Elm_Object_Item *item __UNUSED__)
{
   elm_panel_hidden_set(data, EINA_TRUE);
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Evas_Object *ao;
   ELM_PANEL_DATA_GET(obj, sd);

   if (is_access)
     {
        ao = _elm_access_edje_object_part_object_register
           (obj, elm_layout_edje_get(sd->ly), ACCESS_OUTLINE_PART);
        _elm_access_text_set(_elm_access_object_get(ao),
                             ELM_ACCESS_TYPE, E_("A drawer is open, double tap to close it"));
        _elm_access_activate_callback_set
           (_elm_access_object_get(ao), _access_activate_cb, obj);
     }
   else
     {
        _elm_access_edje_object_part_object_unregister
           (obj, elm_layout_edje_get(sd->ly), ACCESS_OUTLINE_PART);
     }
}

static void
_orient_set_do(Evas_Object *obj)
{
   Evas_Coord handler_w = 0, handler_h = 0;

   ELM_PANEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET(obj, wsd);

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
        elm_layout_theme_set(obj, "panel", "top", elm_widget_style_get(obj));
        break;
      case ELM_PANEL_ORIENT_BOTTOM:
        elm_layout_theme_set
          (obj, "panel", "bottom", elm_widget_style_get(obj));
        break;
      case ELM_PANEL_ORIENT_LEFT:
        if (!elm_widget_mirrored_get(obj))
          {
             elm_layout_theme_set
                (obj, "panel", "left", elm_widget_style_get(obj));
          }
        else
          {
             elm_layout_theme_set
                (obj, "panel", "right", elm_widget_style_get(obj));
          }
        break;
      case ELM_PANEL_ORIENT_RIGHT:
        if (!elm_widget_mirrored_get(obj))
          {
             elm_layout_theme_set
                (obj, "panel", "right", elm_widget_style_get(obj));
          }
        else
          {
             elm_layout_theme_set
                (obj, "panel", "left", elm_widget_style_get(obj));
          }
        break;
     }
   edje_object_part_geometry_get(wsd->resize_obj, "handler", NULL, NULL, &handler_w, &handler_h);
   sd->handler_w = (int) (elm_object_scale_get(obj) * (double) (int)(handler_w));
   sd->handler_h = (int) (elm_object_scale_get(obj) * (double) (int)(handler_h));

   _orient_set_do_internal(obj);

   /* drawer access*/
   if (sd->scrollable)
     {
        if (_elm_config->access_mode) _access_obj_process(obj, EINA_TRUE);
        return;
     }
   /* drawer access*/

   /* access */
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        Evas_Object *ao;
        ao = _elm_access_edje_object_part_object_register
               (obj, ELM_WIDGET_DATA(sd)->resize_obj, "btn_icon");
        _elm_access_text_set(_elm_access_object_get(ao),
                             ELM_ACCESS_TYPE, E_("panel button"));
        _elm_access_callback_set
          (_elm_access_object_get(ao), ELM_ACCESS_STATE, _access_state_cb, obj);
     }
}

static Eina_Bool
_elm_panel_smart_theme(Evas_Object *obj)
{
   const char *str;

   ELM_PANEL_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_panel_parent_sc)->theme(obj))
     return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   str = edje_object_data_get
       (ELM_WIDGET_DATA(sd)->resize_obj, "focus_highlight");
   if ((str) && (!strcmp(str, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);

   _orient_set_do(obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_panel_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   Evas_Object *cur;
   Eina_List *items = NULL;
   Eina_Bool ret = EINA_FALSE;

   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->scrollable)
     {
        if (sd->hidden)
          return EINA_FALSE;

        if (_elm_config->access_mode)
          {
             Evas_Object *ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
             if (ao) items = eina_list_append(items, ao);
          }

        items = eina_list_append(items, sd->ly);

        if (_elm_config->access_mode)
          {
             ret = elm_widget_focus_list_next_get
                (obj, items, eina_list_data_get, dir, next);
             eina_list_free(items);

             return ret;
          }
        else
          return elm_widget_focus_next_get(sd->ly, dir, next);
     }

   if (!sd->content)
     return EINA_FALSE;

   cur = sd->content;

   /* Try to Focus cycle in subitem */
   if (!sd->hidden)
     return elm_widget_focus_next_get(cur, dir, next);

   /* access */
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     {
        Evas_Object *ao, *po;
        po = (Evas_Object *)edje_object_part_object_get
               (ELM_WIDGET_DATA(sd)->resize_obj, "btn_icon");
        ao = evas_object_data_get(po, "_part_access_obj");
        _elm_access_highlight_set(ao, EINA_FALSE);
     }

   /* Return */
   *next = (Evas_Object *)obj;
   return !elm_widget_focus_get(obj);
}

static void
_box_layout_cb(Evas_Object *o,
               Evas_Object_Box_Data *priv,
               void *data __UNUSED__)
{
   _els_box_layout(o, priv, EINA_TRUE, EINA_FALSE, EINA_FALSE);
}

static void
_panel_toggle(void *data __UNUSED__,
              Evas_Object *obj,
              const char *emission __UNUSED__,
              const char *source __UNUSED__)
{
   int w, h;
   ELM_PANEL_DATA_GET(obj, sd);

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   sd->hidden = !sd->hidden;

   if (sd->hidden)
     {
        if (sd->scrollable)
          elm_object_signal_emit(obj, "elm,state,inactive", "elm");
        else
          {
             elm_layout_signal_emit(obj, "elm,action,hide", "elm");

             if (elm_widget_focus_get(sd->content))
               {
                  elm_widget_focused_object_clear(obj);
                  elm_widget_focus_steal(obj);
               }
          }
     }
   else
     {
        if (sd->scrollable)
          {
             if (sd->hold)
               {
                  sd->s_iface->hold_set(obj, EINA_FALSE);
                  sd->hold = EINA_FALSE;
                  elm_layout_signal_emit(sd->ly, "elm,state,content,visible", "elm");
                  elm_object_signal_emit(obj, "elm,state,unhold", "elm");
               }

             elm_object_signal_emit(obj, "elm,state,active", "elm");
          }
        else
          elm_layout_signal_emit(obj, "elm,action,show", "elm");
     }

   _hidden_content_region_set(obj, w, h, EINA_TRUE);

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
}

static Eina_Bool
_elm_panel_smart_event(Evas_Object *obj,
                       Evas_Object *src,
                       Evas_Callback_Type type,
                       void *event_info)
{
   // TIZEN ONLY (2013.11.27): when panel is scrollable, ignore smart event
   ELM_PANEL_DATA_GET(obj, sd);
   if (sd->scrollable) return EINA_FALSE;
   // TIZEN ONLY

   Evas_Event_Key_Down *ev = event_info;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if ((src != obj) || (type != EVAS_CALLBACK_KEY_DOWN)) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if ((strcmp(ev->keyname, "Return")) &&
       (strcmp(ev->keyname, "KP_Enter")))
     return EINA_FALSE;

   _panel_toggle(NULL, obj, NULL, NULL);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static Eina_Bool
_elm_panel_smart_content_set(Evas_Object *obj,
                             const char *part,
                             Evas_Object *content)
{
   int w, h;

   ELM_PANEL_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS(_elm_panel_parent_sc)->content_set
              (obj, part, content);

   if (sd->content == content) return EINA_TRUE;

   if (!sd->scrollable && sd->content)
     evas_object_box_remove_all(sd->bx, EINA_TRUE);

   sd->content = content;

   if (content)
     {
        if (sd->scrollable)
          {
             elm_layout_content_set(sd->ly, "elm.swallow.content", sd->content);
             sd->s_iface->content_set(obj, sd->bx);
             evas_object_geometry_get(obj, NULL, NULL, &w, &h);
             _hidden_content_region_set(obj, w, h, EINA_FALSE);
          }
        else
          {
             evas_object_box_append(sd->bx, sd->content);
             evas_object_show(sd->content);
          }
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object *
_elm_panel_smart_content_get(const Evas_Object *obj,
                             const char *part)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS(_elm_panel_parent_sc)->content_get(obj, part);

   return sd->content;
}

static Evas_Object *
_elm_panel_smart_content_unset(Evas_Object *obj,
                               const char *part)
{
   Evas_Object *content;

   ELM_PANEL_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS
              (_elm_panel_parent_sc)->content_unset(obj, part);

   if (!sd->content) return NULL;
   content = sd->content;

   sd->content = NULL;

   if (sd->scrollable) elm_layout_content_unset(sd->ly, "elm.swallow.content");
   else evas_object_box_remove_all(sd->bx, EINA_FALSE);

   return content;
}

static void
_hidden_state_change(Evas_Object *obj, Eina_Bool handler_bring_in)
{
   Evas_Object *ao;
   int x, w, h;
   ELM_PANEL_DATA_GET(obj, sd);

   if (!sd->scrollable) return;
   if (sd->hold) return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   sd->s_iface->content_pos_get(obj, &x, NULL);

   if (sd->orient == ELM_PANEL_ORIENT_TOP ||
       sd->orient == ELM_PANEL_ORIENT_LEFT)
     {
        //1. handler open
        if (x == (sd->panel_w - sd->handler_w))
          {
             if (!handler_bring_in) //_anim_stop_cb function
               {
                  sd->scroll_dir = 0;
                  return;
               }
             else if (sd->scroll_dir == 0) //_on_mouse_up function -> handler close
               {
                  sd->s_iface->region_bring_in(obj, sd->panel_w, 0, w, h);
                  return;
               }
          }
        //2. drawer open
        else if (x == 0)
          {
             if (sd->hidden)
               {
                  elm_object_signal_emit(obj, "elm,state,active", "elm");
                  sd->hidden = EINA_FALSE;
               }
             sd->scroll_dir = 0;

             // drawer focus & access
             elm_object_tree_focus_allow_set(obj, EINA_TRUE);
             if (_elm_config->access_mode)
               {
                  ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
                  evas_object_show(ao);
               }
             elm_object_focus_set(obj, EINA_TRUE);

             return;
          }
        //3. drawer close
        else if (x == sd->panel_w)
          {
             sd->s_iface->hold_set(obj, EINA_TRUE);
             sd->hold = EINA_TRUE;
             elm_layout_signal_emit(sd->ly, "elm,state,content,hidden", "elm");
             elm_object_signal_emit(obj, "elm,state,hold", "elm");
             sd->scrolled = EINA_FALSE;

             if (!sd->hidden)
               {
                  elm_object_signal_emit(obj, "elm,state,inactive", "elm");
                  sd->hidden = EINA_TRUE;
               }
             sd->scroll_dir = 0;

             // drawer focus & access
             elm_object_tree_focus_allow_set(obj, EINA_FALSE);
             if (_elm_config->access_mode)
               {
                  ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
                  evas_object_hide(ao);
               }

             return;
          }
     }
   else
     {
        //1. handler open
        if (x == sd->handler_w)
          {
             if (!handler_bring_in) //_anim_stop_cb function
               {
                  sd->scroll_dir = 0;
                  return;
               }
             else if (sd->scroll_dir == 0) //_on_mouse_up function -> handler close
               {
                  sd->s_iface->region_bring_in(obj, 0, 0, w, h);
                  return;
               }
          }
        //2. drawer open
        else if (x == sd->panel_w)
          {
             if (sd->hidden)
               {
                  elm_object_signal_emit(obj, "elm,state,active", "elm");
                  sd->hidden = EINA_FALSE;
               }
             sd->scroll_dir = 0;

             // drawer focus & access
             elm_object_tree_focus_allow_set(obj, EINA_TRUE);
             if (_elm_config->access_mode)
               {
                  ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
                  evas_object_show(ao);
               }
             elm_object_focus_set(obj, EINA_TRUE);

             return;
          }
        //3. drawer close
        else if (x == 0)
          {
             sd->s_iface->hold_set(obj, EINA_TRUE);
             sd->hold = EINA_TRUE;
             elm_layout_signal_emit(sd->ly, "elm,state,content,hidden", "elm");
             elm_object_signal_emit(obj, "elm,state,hold", "elm");
             sd->scrolled = EINA_FALSE;

             if (!sd->hidden)
               {
                  elm_object_signal_emit(obj, "elm,state,inactive", "elm");
                  sd->hidden = EINA_TRUE;
               }
             sd->scroll_dir = 0;

             // drawer focus & access
             elm_object_tree_focus_allow_set(obj, EINA_FALSE);
             if (_elm_config->access_mode)
               {
                  ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
                  evas_object_hide(ao);
               }

             return;
          }
     }

   if (sd->scroll_dir == 1)
     sd->s_iface->region_bring_in(obj, 0, 0, w, h);
   else if (sd->scroll_dir == 2)
     sd->s_iface->region_bring_in(obj, sd->panel_w, 0, w, h);
}

static void
_on_mouse_up(void *data __UNUSED__,
             Evas *e __UNUSED__,
             Evas_Object *obj,
             void *event_info __UNUSED__)
{
   _hidden_state_change(obj, EINA_TRUE);
}

static void
_on_spacer_mouse_down(void *data,
                      Evas *e __UNUSED__,
                      Evas_Object *obj __UNUSED__,
                      void *event_info)
{
   ELM_PANEL_DATA_GET(data, sd);
   Evas_Event_Mouse_Down *ev = event_info;

   sd->down_x = ev->output.x;
   sd->down_y = ev->output.y;
}

static void
_on_spacer_mouse_up(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info)
{
   ELM_PANEL_DATA_GET(data, sd);
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord up_x, up_y, minw = 0, minh = 0;

   up_x = ev->output.x;
   up_y = ev->output.y;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);

   if ((abs(up_x - sd->down_x) < minw) && (abs(up_y - sd->down_y) < minh))
     elm_panel_hidden_set(data, EINA_TRUE);

   sd->down_x = 0;
   sd->down_y = 0;
}

static void
_scroll_left_cb(Evas_Object *obj, void *data __UNUSED__)
{
   ELM_PANEL_DATA_GET(obj, sd);
   int x;

   sd->s_iface->content_pos_get(obj, &x, NULL);

   if (!(sd->scroll_dir == 2 && ((sd->content_pos - x) < THRESHOLD)))
     sd->scroll_dir = 1;

   sd->content_pos = x;
}

static void
_scroll_right_cb(Evas_Object *obj, void *data __UNUSED__)
{
   ELM_PANEL_DATA_GET(obj, sd);
   int x;

   sd->s_iface->content_pos_get(obj, &x, NULL);

   if (!(sd->scroll_dir == 1 && ((x - sd->content_pos) < THRESHOLD)))
     sd->scroll_dir = 2;

   sd->content_pos = x;
}

static void
_scroll_cb(Evas_Object *obj, void *data __UNUSED__)
{
   Evas_Coord x, y;
   Elm_Panel_Scroll_Info event;

   ELM_PANEL_DATA_GET(obj, sd);

   // if drag really fast and mouse up, scroll happens after the mouse up.
   // scroller is set to hold on mouse up, so have to unhold here
   if (sd->hold)
     {
        sd->s_iface->hold_set(obj, EINA_FALSE);
        sd->hold = EINA_FALSE;
        elm_layout_signal_emit(sd->ly, "elm,state,content,visible", "elm");
        elm_object_signal_emit(obj, "elm,state,unhold", "elm");
     }
   // signal emit at the first scrolling
   if (!sd->scrolled)
     {
        sd->scrolled = EINA_TRUE;
        elm_layout_signal_emit(sd->ly, "elm,state,scroll,start", "elm");
     }

   sd->s_iface->content_pos_get(obj, &x, &y);

   if (sd->orient == ELM_PANEL_ORIENT_TOP)
     {
        event.rel_x = 1;
        event.rel_y = 1;
//        event.rel_y = 1 - ((double) y / (double) sd->panel_h);
     }
   else if (sd->orient == ELM_PANEL_ORIENT_LEFT)
     {
        event.rel_x = 1 - ((double) x / (double) sd->panel_w);
        event.rel_y = 1;
     }
   else if (sd->orient == ELM_PANEL_ORIENT_RIGHT)
     {
        event.rel_x = (double) x / (double) sd->panel_w;
        event.rel_y = 1;
     }
   else
     {
        event.rel_x = 1;
        event.rel_y = 1;
//        event.rel_y = 1 - ((double) y / (double) sd->panel_h);
     }
   evas_object_smart_callback_call(obj, SIG_SCROLL, (void *) &event);
}

static void
_active_cb(void *data __UNUSED__,
           Evas_Object *obj,
           const char *emission __UNUSED__,
           const char *source __UNUSED__)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if (!sd->scrollable) return;
   if (!sd->hold) return;

   sd->s_iface->hold_set(obj, EINA_FALSE);
   sd->hold = EINA_FALSE;
   elm_layout_signal_emit(sd->ly, "elm,state,content,visible", "elm");
   elm_object_signal_emit(obj, "elm,state,unhold", "elm");
}

static void
_handler_open_cb(void *data __UNUSED__,
           Evas_Object *obj,
           const char *emission __UNUSED__,
           const char *source __UNUSED__)
{
   int w, h, x;
   ELM_PANEL_DATA_GET(obj, sd);

   if (!sd->scrollable) return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_pointer_output_xy_get(evas_object_evas_get(obj), &x, NULL);

   if (sd->orient == ELM_PANEL_ORIENT_TOP ||
       sd->orient == ELM_PANEL_ORIENT_LEFT)
     {
        if ((x > 0) && (x < sd->handler_w))
          sd->s_iface->region_bring_in
             (obj, (sd->panel_w - sd->handler_w), 0, w, h);
     }
   else
     {
        if ((x > (sd->panel_w - sd->handler_w)) && (x < sd->panel_w))
          sd->s_iface->region_bring_in
             (obj, sd->handler_w, 0, w, h);
     }
}

static void
_anim_start_cb(Evas_Object *obj, void *data __UNUSED__)
{
   ELM_PANEL_DATA_GET(obj, sd);
   sd->animating = EINA_TRUE;
}

static void
_anim_stop_cb(Evas_Object *obj, void *data __UNUSED__)
{
   ELM_PANEL_DATA_GET(obj, sd);
   sd->animating = EINA_FALSE;
   _hidden_state_change(obj, EINA_FALSE);
}

static void
_elm_panel_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Panel_Smart_Data);

   ELM_WIDGET_CLASS(_elm_panel_parent_sc)->base.add(obj);
}

static void
_elm_panel_smart_del(Evas_Object *obj)
{
   Evas_Object *child;
   Eina_List *l;

   ELM_PANEL_DATA_GET(obj, sd);

   sd->on_deletion = EINA_TRUE;

   /* let's make our box object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH(ELM_WIDGET_DATA(sd)->subobjs, l, child)
     {
        if (child == sd->bx)
          {
             ELM_WIDGET_DATA(sd)->subobjs =
               eina_list_demote_list(ELM_WIDGET_DATA(sd)->subobjs, l);
             break;
          }
     }

   ELM_WIDGET_CLASS(_elm_panel_parent_sc)->base.del(obj);
}

static void
_elm_panel_smart_access(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_PANEL_CHECK(obj);
   ELM_PANEL_DATA_GET(obj, sd);

   /* drawer access*/
   if (sd->scrollable)
     {
        _access_obj_process(obj, is_access);
        return;
     }
   /* drawer access*/

   if (is_access)
     _elm_access_edje_object_part_object_register
       (obj, ELM_WIDGET_DATA(sd)->resize_obj, "btn_icon");
   else
     _elm_access_edje_object_part_object_unregister
       (obj, ELM_WIDGET_DATA(sd)->resize_obj, "btn_icon");
}

static void
_elm_panel_smart_resize(Evas_Object *obj,
                        Evas_Coord w,
                        Evas_Coord h)
{
   Evas_Coord handler_w = 0, handler_h = 0;

   ELM_PANEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET(obj, wsd);

   ELM_WIDGET_CLASS(_elm_panel_parent_sc)->base.resize(obj, w, h);

   evas_object_resize(sd->hit_rect, w, h);
   if ((sd->content) && (!sd->animating))
     _hidden_content_region_set(obj, w, h, EINA_FALSE);
   if (sd->spacer)
     {
        if (wsd->orient_mode == 90 || wsd->orient_mode == 270)
          {
             evas_object_size_hint_min_set(sd->spacer, 1860, h);
             evas_object_resize(sd->bx, 1860, 0);
          }
        else
          {
             evas_object_size_hint_min_set(sd->spacer, 1300, h);
             evas_object_resize(sd->bx, 1300, 0);
          }
        edje_object_part_geometry_get(wsd->resize_obj, "handler", NULL, NULL, &handler_w, &handler_h);
        sd->handler_w = (int) (elm_object_scale_get(obj) * (double) (int)(handler_w));
        sd->handler_h = (int) (elm_object_scale_get(obj) * (double) (int)(handler_h));
     }
}

static Eina_Bool
_elm_panel_smart_disable(Evas_Object *obj)
{
   ELM_PANEL_DATA_GET(obj, sd);
   if (sd->scrollable)
     {
        if (elm_widget_disabled_get(obj))
          {
             evas_object_pass_events_set(sd->hit_rect, EINA_TRUE);
             elm_layout_signal_emit(obj, "elm,state,disabled", "elm");
          }
        else
          {
             evas_object_pass_events_set(sd->hit_rect, EINA_FALSE);
             elm_layout_signal_emit(obj, "elm,state,enabled", "elm");
          }
     }
   return EINA_TRUE;
}

static void
_elm_panel_smart_set_user(Elm_Panel_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_panel_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_panel_smart_del;

   ELM_WIDGET_CLASS(sc)->base.resize = _elm_panel_smart_resize;

   ELM_WIDGET_CLASS(sc)->focus_next = _elm_panel_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->theme = _elm_panel_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_panel_smart_event;
   ELM_WIDGET_CLASS(sc)->access = _elm_panel_smart_access;
   ELM_WIDGET_CLASS(sc)->disable = _elm_panel_smart_disable;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_panel_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_panel_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_panel_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_panel_smart_sizing_eval;
}

EAPI const Elm_Panel_Smart_Class *
elm_panel_smart_class_get(void)
{
   static Elm_Panel_Smart_Class _sc =
     ELM_PANEL_SMART_CLASS_INIT_NAME_VERSION(ELM_PANEL_SMART_NAME);
   static const Elm_Panel_Smart_Class *class = NULL;

   if (class) return class;

   _elm_panel_smart_set(&_sc);
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_panel_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_panel_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   ELM_PANEL_DATA_GET(obj, sd);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   _orient_set_do(obj);

   /* box */
   sd->bx = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_size_hint_align_set(sd->bx, 0.5, 0.5);
   evas_object_size_hint_weight_set(sd->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_box_layout_set(sd->bx, _box_layout_cb, sd, NULL);
   elm_layout_content_set(obj, "elm.swallow.content", sd->bx);
   evas_object_show(sd->bx);

   elm_layout_signal_callback_add
     (obj, "elm,action,panel,toggle", "*", _panel_toggle, obj);
   elm_layout_signal_callback_add(obj, "elm,state,event,active", "elm", _active_cb,
                                  NULL);
   elm_layout_signal_callback_add(obj, "elm,state,handler,open", "elm", _handler_open_cb,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up,
                                  NULL);

   elm_layout_sizing_eval(obj);

   /* scrollable interface */
   sd->s_iface = evas_object_smart_interface_get
     (obj, ELM_SCROLLABLE_IFACE_NAME);

   /* hit rect */
   sd->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(sd->hit_rect, obj);
   elm_widget_sub_object_add(obj, sd->hit_rect);
   evas_object_color_set(sd->hit_rect, 0, 0, 0, 0);
   evas_object_pass_events_set(sd->hit_rect, EINA_TRUE);
   evas_object_show(sd->hit_rect);

   sd->s_iface->objects_set
      (obj, ELM_WIDGET_DATA(sd)->resize_obj, sd->hit_rect);

   sd->s_iface->single_direction_set(obj, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
   sd->s_iface->scroll_left_cb_set(obj, _scroll_left_cb);
   sd->s_iface->scroll_right_cb_set(obj, _scroll_right_cb);
   sd->s_iface->scroll_cb_set(obj, _scroll_cb);
   sd->s_iface->animate_start_cb_set(obj, _anim_start_cb);
   sd->s_iface->animate_stop_cb_set(obj, _anim_stop_cb);

   sd->s_iface->hold_set(obj, EINA_TRUE);
   sd->hold = EINA_TRUE;

   //Tizen Only: This should be removed when eo is applied.
   ELM_WIDGET_DATA_GET(obj, wsd);
   wsd->on_create = EINA_FALSE;
   wsd->highlight_root = EINA_TRUE;

   return obj;
}

EAPI void
elm_panel_orient_set(Evas_Object *obj,
                     Elm_Panel_Orient orient)
{
   ELM_PANEL_CHECK(obj);
   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->orient == orient) return;
   sd->orient = orient;

   _orient_set_do(obj);

   elm_layout_sizing_eval(obj);
}

EAPI Elm_Panel_Orient
elm_panel_orient_get(const Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj) ELM_PANEL_ORIENT_LEFT;
   ELM_PANEL_DATA_GET(obj, sd);

   return sd->orient;
}

EAPI void
elm_panel_hidden_set(Evas_Object *obj,
                     Eina_Bool hidden)
{
   ELM_PANEL_CHECK(obj);
   ELM_PANEL_DATA_GET(obj, sd);

   if (sd->hidden == hidden)
     {
        if (sd->hidden && !sd->hold)
          {
             int w, h;
             evas_object_geometry_get(obj, NULL, NULL, &w, &h);
             _hidden_content_region_set(obj, w, h, EINA_TRUE);
          }

        return;
     }


   _panel_toggle(NULL, obj, NULL, NULL);
}

EAPI Eina_Bool
elm_panel_hidden_get(const Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj) EINA_FALSE;
   ELM_PANEL_DATA_GET(obj, sd);

   return sd->hidden;
}

EAPI void
elm_panel_toggle(Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj);

   _panel_toggle(NULL, obj, NULL, NULL);
}

EAPI void
elm_panel_scrollable_set(Evas_Object *obj, Eina_Bool scrollable)
{
   ELM_PANEL_CHECK(obj);
   ELM_PANEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET(obj, wsd);

   Evas_Coord h;

   Evas_Object *content;

   scrollable = !!scrollable;

   if (sd->scrollable == scrollable) return;
   sd->scrollable = scrollable;

   //TODO: scrollable set
   if (scrollable)
     {
        /* internal layout */
        sd->ly = elm_layout_add(obj);
        evas_object_smart_member_add(sd->ly, obj);
        elm_widget_sub_object_add(obj, sd->ly);
        _orient_set_do_internal(obj);
        evas_object_show(sd->ly);

        content = sd->content;
        evas_object_box_remove_all(sd->bx, EINA_FALSE);
        evas_object_box_append(sd->bx, sd->ly);
        elm_layout_content_unset(obj, "elm.swallow.content");
        sd->content = NULL;
        if (content)
          elm_layout_content_set(obj, "elm.swallow.content", content);

        sd->spacer = evas_object_rectangle_add(evas_object_evas_get(obj));
        evas_object_color_set(sd->spacer, 0, 0, 0, 0);
        evas_object_event_callback_add(sd->spacer, EVAS_CALLBACK_MOUSE_DOWN,
                                       _on_spacer_mouse_down, obj);
        evas_object_event_callback_add(sd->spacer, EVAS_CALLBACK_MOUSE_UP,
                                       _on_spacer_mouse_up, obj);

        evas_object_geometry_get(obj, NULL, NULL, NULL, &h);
        if (wsd->orient_mode == 90 || wsd->orient_mode == 270)
          evas_object_size_hint_min_set(sd->spacer, 1860, h);
        else
          evas_object_size_hint_min_set(sd->spacer, 1300, h);

        elm_layout_content_set(sd->ly, "elm.swallow.spacer", sd->spacer);

        if (sd->hidden)
          elm_layout_signal_emit(sd->ly, "elm,state,content,hidden", "elm");
        else
          elm_layout_signal_emit(sd->ly, "elm,state,content,visible", "elm");

        evas_object_pass_events_set(sd->hit_rect, EINA_FALSE);
     }
   else
     {
        elm_layout_content_unset(sd->ly, "elm.swallow.content");
        evas_object_box_append(sd->bx, sd->content);
        elm_layout_content_set(obj, "elm.swallow.content", sd->bx);
        evas_object_pass_events_set(sd->hit_rect, EINA_TRUE);
        evas_object_del(sd->spacer);
        evas_object_del(sd->ly);
        sd->spacer = NULL;
        sd->ly = NULL;
     }
}

EAPI Eina_Bool
elm_panel_scrollable_get(const Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj) EINA_FALSE;
   ELM_PANEL_DATA_GET(obj, sd);
   return sd->scrollable;
}
