#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_ctxpopup.h"

#define OUTLINE_TEXT "Center popup is opened, double tap to close the popup"

EAPI const char ELM_CTXPOPUP_SMART_NAME[] = "elm_ctxpopup";

static const char ACCESS_OUTLINE_PART[] = "access.outline";

static const char SIG_DISMISSED[] = "dismissed";
static const char SIG_LANG_CHANGED[] = "language,changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DISMISSED, ""},
   {SIG_LANG_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_CTXPOPUP_SMART_NAME, _elm_ctxpopup, Elm_Ctxpopup_Smart_Class,
   Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static Eina_Bool
_elm_ctxpopup_smart_translate(Evas_Object *obj)
{
   evas_object_hide(obj);

   return EINA_TRUE;
}

static Evas_Object *
_access_object_get(const Evas_Object *obj, const char* part)
{
   Evas_Object *po, *ao;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   po = (Evas_Object *)edje_object_part_object_get(ELM_WIDGET_DATA(sd)->resize_obj, part);
   ao = evas_object_data_get(po, "_part_access_obj");

   return ao;
}

static Eina_Bool
_elm_ctxpopup_smart_focus_next(const Evas_Object *obj,
                               Elm_Focus_Direction dir,
                               Evas_Object **next)
{
   Eina_List *items = NULL;
   Eina_List *elist = NULL;
   Elm_Ctxpopup_Item *it;
   Evas_Object *ao;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd)
     return EINA_FALSE;

   if (_elm_config->access_mode)
     {
        ao = _access_object_get(obj, ACCESS_OUTLINE_PART);
        if (ao) items = eina_list_append(items, ao);
     }

   if (eina_list_count(sd->items))
     {
        EINA_LIST_FOREACH(sd->items, elist, it)
          items = eina_list_append(items, it->base.access_obj);

        return elm_widget_focus_list_next_get
                 (obj, items, eina_list_data_get, dir, next);
     }
   else
     {
        return elm_widget_focus_next_get(sd->box, dir, next);
     }
}

static Eina_Bool
_elm_ctxpopup_smart_focus_direction(const Evas_Object *obj,
                                    const Evas_Object *base,
                                    double degree,
                                    Evas_Object **direction,
                                    double *weight)
{
   Eina_Bool ret;
   Eina_List *l = NULL;
   void *(*list_data_get)(const Eina_List *list);

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd)
     return EINA_FALSE;

   list_data_get = eina_list_data_get;

   l = eina_list_append(l, sd->box);
   ret = elm_widget_focus_list_direction_get
      (obj, base, l, list_data_get, degree, direction, weight);
   eina_list_free(l);

   return ret;
}

static Eina_Bool
_elm_ctxpopup_smart_event(Evas_Object *obj,
                          Evas_Object *src __UNUSED__,
                          Evas_Callback_Type type,
                          void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!strcmp(ev->keyname, "Tab"))
     {
        if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
          elm_widget_focus_cycle(sd->box, ELM_FOCUS_PREVIOUS);
        else
          elm_widget_focus_cycle(sd->box, ELM_FOCUS_NEXT);
        return EINA_TRUE;
     }

   if (strcmp(ev->keyname, "Escape")) return EINA_FALSE;

   evas_object_hide(obj);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_freeze_on_cb(void *data __UNUSED__,
              Evas_Object *obj,
              void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((!sd) || (!sd->scr)) return;
   elm_object_scroll_freeze_push(sd->scr);
}

static void
_freeze_off_cb(void *data __UNUSED__,
               Evas_Object *obj,
               void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((!sd) || (!sd->scr)) return;
   elm_object_scroll_freeze_pop(sd->scr);
}

static void
_hold_on_cb(void *data __UNUSED__,
            Evas_Object *obj,
            void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((!sd) || (!sd->scr)) return;
   elm_object_scroll_hold_push(sd->scr);
}

static void
_hold_off_cb(void *data __UNUSED__,
             Evas_Object *obj,
             void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((!sd) || (!sd->scr)) return;
   elm_object_scroll_hold_pop(sd->scr);
}

static void
_x_pos_adjust(Evas_Coord_Point *pos,
              Evas_Coord_Point *base_size,
              Evas_Coord_Rectangle *hover_area)
{
   pos->x -= (base_size->x / 2);

   if (pos->x < hover_area->x)
     pos->x = hover_area->x;
   else if ((pos->x + base_size->x) > (hover_area->x + hover_area->w))
     pos->x = (hover_area->x + hover_area->w) - base_size->x;

   if (base_size->x > hover_area->w)
     base_size->x -= (base_size->x - hover_area->w);

   if (pos->x < hover_area->x)
     pos->x = hover_area->x;
}

static void
_y_pos_adjust(Evas_Coord_Point *pos,
              Evas_Coord_Point *base_size,
              Evas_Coord_Rectangle *hover_area)
{
   pos->y -= (base_size->y / 2);

   if (pos->y < hover_area->y)
     pos->y = hover_area->y;
   else if ((pos->y + base_size->y) > (hover_area->y + hover_area->h))
     pos->y = hover_area->y + hover_area->h - base_size->y;

   if (base_size->y > hover_area->h)
     base_size->y -= (base_size->y - hover_area->h);

   if (pos->y < hover_area->y)
     pos->y = hover_area->y;
}

static void
_item_select_cb(void *data, Evas_Object *obj __UNUSED__,
                const char *emission __UNUSED__,
                const char *source __UNUSED__)
{
   Elm_Ctxpopup_Item *item = data;

   if (!item) return;
   if (elm_widget_item_disabled_get(item)) return;

   if (item->func)
     item->func((void*)item->base.data, WIDGET(item), data);
}

static char *
_access_info_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Ctxpopup_Item *it = (Elm_Ctxpopup_Item *)data;
   const char *txt = NULL;
   Evas_Object *icon = NULL;
   Eina_Strbuf *buf = NULL;
   char *str = NULL;

   if (!it) return NULL;

   txt = it->label;
   icon = it->icon;

   if (txt && icon)
     {
        buf = eina_strbuf_new();
        eina_strbuf_append(buf, E_("icon "));
        eina_strbuf_append(buf, txt);
        str = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);
        return str;
     }
   else if ((!txt) && icon) return strdup(E_("icon"));
   else if (txt && (!icon)) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Ctxpopup_Item *it = (Elm_Ctxpopup_Item *)data;
   if (!it) return NULL;

   if (it->base.disabled)
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_activate_cb(void *data __UNUSED__,
                    Evas_Object *part_obj __UNUSED__,
                    Elm_Object_Item *item)
{
   _item_select_cb(item, NULL, NULL, NULL);
}

static void
_access_widget_item_register(Elm_Ctxpopup_Item *it, Eina_Bool is_access)
{
   Elm_Access_Info *ai;

   if (!is_access) _elm_access_widget_item_unregister((Elm_Widget_Item *)it);
   else
     {
        _elm_access_widget_item_register((Elm_Widget_Item *)it);

        ai = _elm_access_object_get(it->base.access_obj);

        _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
        _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Contextual Popup"));
        _elm_access_activate_callback_set(ai, _access_activate_cb, it);
     }
}

static void
_item_new(Elm_Ctxpopup_Item *item,
          char *group_name)
{
   ELM_CTXPOPUP_DATA_GET(WIDGET(item), sd);
   if (!sd) return;

   VIEW(item) = edje_object_add(evas_object_evas_get(sd->box));
   edje_object_mirrored_set(VIEW(item), elm_widget_mirrored_get(WIDGET(item)));
   _elm_theme_object_set(WIDGET(item), VIEW(item), "ctxpopup", group_name,
                         elm_widget_style_get(WIDGET(item)));
   edje_object_signal_callback_add(VIEW(item), "elm,action,click", "",
                                   _item_select_cb, item);
   evas_object_size_hint_align_set(VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(VIEW(item));

   /* access */
   if (_elm_config->access_mode) _access_widget_item_register(item, EINA_TRUE);
}

static void
_item_icon_set(Elm_Ctxpopup_Item *item,
               Evas_Object *icon)
{
   if (item->icon)
     evas_object_del(item->icon);

   item->icon = icon;
   if (!icon) return;

   edje_object_part_swallow(VIEW(item), "elm.swallow.icon", item->icon);
   edje_object_message_signal_process(VIEW(item));
}

static void
_item_label_set(Elm_Ctxpopup_Item *item,
                const char *label)
{
   if (!eina_stringshare_replace(&item->label, label))
     return;

   edje_object_part_text_set(VIEW(item), "elm.text", label);
   edje_object_message_signal_process(VIEW(item));
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   ELM_CTXPOPUP_DATA_GET(WIDGET(ctxpopup_it), sd);
   if (!sd) return EINA_FALSE;

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (ctxpopup_it->icon)
     evas_object_del(ctxpopup_it->icon);
   if (VIEW(ctxpopup_it))
     evas_object_del(VIEW(ctxpopup_it));

   eina_stringshare_del(ctxpopup_it->label);
   sd->items = eina_list_remove(sd->items, ctxpopup_it);

   if (eina_list_count(sd->items) < 1)
     {
        evas_object_hide(WIDGET(ctxpopup_it));
        return EINA_TRUE;
     }
   if (sd->visible) elm_layout_sizing_eval(WIDGET(ctxpopup_it));

   return EINA_TRUE;
}

static void
_items_remove(Elm_Ctxpopup_Smart_Data *sd)
{
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;

   if (!sd->items) return;

   EINA_LIST_FOREACH(sd->items, elist, item)
     {
         _item_del_pre_hook((Elm_Object_Item *)item);
     }

   sd->items = NULL;
}

static void
_scroller_size_reset(Elm_Ctxpopup_Smart_Data *sd)
{
   sd->finished = EINA_FALSE;
   sd->max_sc_h = -1;
   sd->max_sc_w = -1;
}

static void
_item_sizing_eval(Elm_Ctxpopup_Item *item)
{
   Evas_Coord min_w = -1, min_h = -1, max_w = -1, max_h = -1;

   if (!item) return;

   edje_object_size_min_restricted_calc(VIEW(item), &min_w, &min_h, min_w,
                                        min_h);
   evas_object_size_hint_min_set(VIEW(item), min_w, min_h);
   evas_object_size_hint_max_set(VIEW(item), max_w, max_h);
}

static Elm_Ctxpopup_Direction
_base_geometry_calc(Evas_Object *obj,
                    Evas_Coord_Rectangle *rect)
{
   Elm_Ctxpopup_Direction dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;
   Evas_Coord_Rectangle hover_area;
   Evas_Coord_Point pos = {0, 0};
   Evas_Coord_Point arrow_size;
   Evas_Coord_Point base_size;
   Evas_Coord_Point max_size;
   Evas_Coord_Point min_size;
   Evas_Coord_Point temp;
   int idx;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!rect) return ELM_CTXPOPUP_DIRECTION_DOWN;

   edje_object_part_geometry_get
     (sd->arrow, "ctxpopup_arrow", NULL, NULL, &arrow_size.x, &arrow_size.y);
   evas_object_resize(sd->arrow, arrow_size.x, arrow_size.y);

   //Initialize Area Rectangle.
   evas_object_geometry_get
     (sd->parent, &hover_area.x, &hover_area.y, &hover_area.w,
     &hover_area.h);

   evas_object_geometry_get(obj, &pos.x, &pos.y, NULL, NULL);

   //recalc the edje
   edje_object_size_min_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &base_size.x, &base_size.y);
   evas_object_smart_calculate(ELM_WIDGET_DATA(sd)->resize_obj);

   //Limit to Max Size
   evas_object_size_hint_max_get(obj, &max_size.x, &max_size.y);
   if ((max_size.x == -1) || (max_size.y == -1))
     edje_object_size_max_get(sd->layout, &max_size.x, &max_size.y);

   if ((max_size.y > 0) && (base_size.y > max_size.y))
     base_size.y = max_size.y;

   if ((max_size.x > 0) && (base_size.x > max_size.x))
     base_size.x = max_size.x;

   //Limit to Min Size
   evas_object_size_hint_min_get(obj, &min_size.x, &min_size.y);
   if ((min_size.x == 0) || (min_size.y == 0))
     edje_object_size_min_get(sd->layout, &min_size.x, &min_size.y);

   if ((min_size.y > 0) && (base_size.y < min_size.y))
     base_size.y = min_size.y;

   if ((min_size.x > 0) && (base_size.x < min_size.x))
     base_size.x = min_size.x;

   //Check the Which direction is available.
   //If find a avaialble direction, it adjusts position and size.
   for (idx = 0; idx < 4; idx++)
     {
        switch (sd->dir_priority[idx])
          {
           case ELM_CTXPOPUP_DIRECTION_UNKNOWN:

           case ELM_CTXPOPUP_DIRECTION_UP:
             temp.y = (pos.y - base_size.y);
             if ((temp.y - arrow_size.y) < hover_area.y)
               continue;

             _x_pos_adjust(&pos, &base_size, &hover_area);
             pos.y -= base_size.y;
             dir = ELM_CTXPOPUP_DIRECTION_UP;
             break;

           case ELM_CTXPOPUP_DIRECTION_LEFT:
             temp.x = (pos.x - base_size.x);
             if ((temp.x - arrow_size.x) < hover_area.x)
               continue;

             _y_pos_adjust(&pos, &base_size, &hover_area);
             pos.x -= base_size.x;
             dir = ELM_CTXPOPUP_DIRECTION_LEFT;
             break;

           case ELM_CTXPOPUP_DIRECTION_RIGHT:
             temp.x = (pos.x + base_size.x);
             if ((temp.x + arrow_size.x) >
                 (hover_area.x + hover_area.w))
               continue;

             _y_pos_adjust(&pos, &base_size, &hover_area);
             dir = ELM_CTXPOPUP_DIRECTION_RIGHT;
             break;

           case ELM_CTXPOPUP_DIRECTION_DOWN:
             temp.y = (pos.y + base_size.y);
             if ((temp.y + arrow_size.y) >
                 (hover_area.y + hover_area.h))
               continue;

             _x_pos_adjust(&pos, &base_size, &hover_area);
             dir = ELM_CTXPOPUP_DIRECTION_DOWN;
             break;

           default:
             continue;
          }
        break;
     }

   //In this case, all directions are invalid because of lack of space.
   if (idx == 4)
     {
        Evas_Coord length[2];

        if (!sd->horizontal)
          {
             length[0] = pos.y - hover_area.y;
             length[1] = (hover_area.y + hover_area.h) - pos.y;

             // ELM_CTXPOPUP_DIRECTION_UP
             if (length[0] > length[1])
               {
                  _x_pos_adjust(&pos, &base_size, &hover_area);
                  pos.y -= base_size.y;
                  dir = ELM_CTXPOPUP_DIRECTION_UP;
                  if (pos.y < (hover_area.y + arrow_size.y))
                    {
                       base_size.y -= ((hover_area.y + arrow_size.y) - pos.y);
                       pos.y = hover_area.y + arrow_size.y;
                    }
               }
             //ELM_CTXPOPUP_DIRECTION_DOWN
             else
               {
                  _x_pos_adjust(&pos, &base_size, &hover_area);
                  dir = ELM_CTXPOPUP_DIRECTION_DOWN;
                  if ((pos.y + arrow_size.y + base_size.y) >
                      (hover_area.y + hover_area.h))
                    base_size.y -=
                      ((pos.y + arrow_size.y + base_size.y) -
                       (hover_area.y + hover_area.h));
               }
          }
        else
          {
             length[0] = pos.x - hover_area.x;
             length[1] = (hover_area.x + hover_area.w) - pos.x;

             //ELM_CTXPOPUP_DIRECTION_LEFT
             if (length[0] > length[1])
               {
                  _y_pos_adjust(&pos, &base_size, &hover_area);
                  pos.x -= base_size.x;
                  dir = ELM_CTXPOPUP_DIRECTION_LEFT;
                  if (pos.x < (hover_area.x + arrow_size.x))
                    {
                       base_size.x -= ((hover_area.x + arrow_size.x) - pos.x);
                       pos.x = hover_area.x + arrow_size.x;
                    }
               }
             //ELM_CTXPOPUP_DIRECTION_RIGHT
             else
               {
                  _y_pos_adjust(&pos, &base_size, &hover_area);
                  dir = ELM_CTXPOPUP_DIRECTION_RIGHT;
                  if (pos.x + (arrow_size.x + base_size.x) >
                      hover_area.x + hover_area.w)
                    base_size.x -=
                      ((pos.x + arrow_size.x + base_size.x) -
                       (hover_area.x + hover_area.w));
               }
          }
     }

   //Final position and size.
   rect->x = pos.x;
   rect->y = pos.y;
   rect->w = base_size.x;
   rect->h = base_size.y;

   return dir;
}

static void
_arrow_update(Evas_Object *obj,
              Elm_Ctxpopup_Direction dir,
              Evas_Coord_Rectangle base_size)
{
   Evas_Coord_Rectangle arrow_size;
   Evas_Coord x, y;
   double drag;
   Evas_Coord_Rectangle shadow_left_top, shadow_right_bottom, arrow_padding;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_geometry_get
     (sd->arrow, NULL, NULL, &arrow_size.w, &arrow_size.h);

   /* tizen only : since ctxpopup of tizen has shadow, start and end padding of arrow, it should be put together when updating arrow
    * so there are some differences between open source and tizen */
   edje_object_part_geometry_get(ELM_WIDGET_DATA(sd)->resize_obj, "frame_shadow_left_top_padding", NULL, NULL, &shadow_left_top.w, &shadow_left_top.h);
   edje_object_part_geometry_get(ELM_WIDGET_DATA(sd)->resize_obj, "frame_shadow_right_bottom_padding", NULL, NULL, &shadow_right_bottom.w, &shadow_right_bottom.h);
   edje_object_part_geometry_get(ELM_WIDGET_DATA(sd)->resize_obj, "ctxpopup_frame_left_top", NULL, NULL, &arrow_padding.w, &arrow_padding.h);

   /* arrow is not being kept as sub-object on purpose, here. the
    * design of the widget does not help with the contrary */

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        edje_object_signal_emit(sd->arrow, "elm,state,left", "elm");
        edje_object_part_swallow
           (ELM_WIDGET_DATA(sd)->resize_obj,
            (elm_widget_mirrored_get(obj) ? "elm.swallow.arrow_right" :
             "elm.swallow.arrow_left"), sd->arrow);

        if (base_size.h > 0)
          {
             if (y <= ((arrow_size.h * 0.5) + base_size.y + shadow_left_top.h + arrow_padding.h))
               y = 0;
             else if (y >= (base_size.y + base_size.h - ((arrow_size.h * 0.5) + shadow_right_bottom.h + arrow_padding.h)))
               y = base_size.h - (arrow_size.h + shadow_right_bottom.h + shadow_left_top.h + (arrow_padding.h * 2));
             else
               y = y - base_size.y - ((arrow_size.h * 0.5) + shadow_left_top.h + arrow_padding.h);
             drag = (double)(y) / (double)(base_size.h - (arrow_size.h + shadow_right_bottom.h + shadow_left_top.h + (arrow_padding.h * 2)));
             edje_object_part_drag_value_set
                (ELM_WIDGET_DATA(sd)->resize_obj,
                 (elm_widget_mirrored_get(obj) ? "elm.swallow.arrow_right" :
                  "elm.swallow.arrow_left"), 1, drag);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        edje_object_signal_emit(sd->arrow, "elm,state,right", "elm");
        edje_object_part_swallow
           (ELM_WIDGET_DATA(sd)->resize_obj,
            (elm_widget_mirrored_get(obj) ? "elm.swallow.arrow_left" :
             "elm.swallow.arrow_right"), sd->arrow);

        if (base_size.h > 0)
          {
             if (y <= ((arrow_size.h * 0.5) + base_size.y + shadow_left_top.h + arrow_padding.h))
               y = 0;
             else if (y >= (base_size.y + base_size.h - ((arrow_size.h * 0.5) + shadow_right_bottom.h + arrow_padding.h)))
               y = base_size.h - (arrow_size.h + shadow_right_bottom.h + shadow_left_top.h + (arrow_padding.h * 2));
             else
               y = y - base_size.y - ((arrow_size.h * 0.5) + shadow_left_top.h + arrow_padding.h);
             drag = (double)(y) / (double)(base_size.h - (arrow_size.h + shadow_right_bottom.h + shadow_left_top.h + (arrow_padding.h * 2)));
             edje_object_part_drag_value_set
                (ELM_WIDGET_DATA(sd)->resize_obj,
                 (elm_widget_mirrored_get(obj) ? "elm.swallow.arrow_left" :
                  "elm.swallow.arrow_right"), 0, drag);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        edje_object_signal_emit(sd->arrow, "elm,state,top", "elm");
        edje_object_part_swallow
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_up",
          sd->arrow);

        if (base_size.w > 0)
          {
             if (x <= ((arrow_size.w * 0.5) + base_size.x + shadow_left_top.w + arrow_padding.w))
               x = 0;
             else if (x >= (base_size.x + base_size.w - ((arrow_size.w * 0.5) + shadow_right_bottom.w + arrow_padding.w)))
               x = base_size.w - (arrow_size.w + shadow_right_bottom.w + shadow_left_top.w + (arrow_padding.w * 2));
             else
               x = x - base_size.x - ((arrow_size.w * 0.5) + shadow_left_top.w + arrow_padding.w);
             drag = (double)(x) / (double)(base_size.w - (arrow_size.w + shadow_right_bottom.w + shadow_left_top.w + (arrow_padding.w * 2)));
             edje_object_part_drag_value_set
               (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_up",
               drag, 1);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_UP:
        edje_object_signal_emit(sd->arrow, "elm,state,bottom", "elm");
        edje_object_part_swallow
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_down",
          sd->arrow);

        if (base_size.w > 0)
          {
             if (x <= ((arrow_size.w * 0.5) + base_size.x + shadow_left_top.w + arrow_padding.w))
               x = 0;
             else if (x >= (base_size.x + base_size.w - ((arrow_size.w * 0.5) + shadow_right_bottom.w + arrow_padding.w)))
               x = base_size.w - (arrow_size.w + shadow_right_bottom.w + shadow_left_top.w + (arrow_padding.w * 2));
             else
               x = x - base_size.x - ((arrow_size.w * 0.5) + shadow_left_top.w + arrow_padding.w);
             drag = (double)(x) / (double)(base_size.w - (arrow_size.w + shadow_right_bottom.w + shadow_left_top.w + (arrow_padding.w * 2)));
             edje_object_part_drag_value_set
               (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_down",
               drag, 0);
          }
        break;

      default:
        break;
     }

   //should be here for getting accurate geometry value
   evas_object_smart_calculate(ELM_WIDGET_DATA(sd)->resize_obj);
}

static void
_show_signals_emit(Evas_Object *obj,
                   Elm_Ctxpopup_Direction dir)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->visible) return;

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_UP:
        edje_object_signal_emit(sd->layout, "elm,state,show,up", "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        edje_object_signal_emit(sd->layout, (elm_widget_mirrored_get(obj) ? "elm,state,show,right" :
               "elm,state,show,left"), "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        edje_object_signal_emit(sd->layout, (elm_widget_mirrored_get(obj) ? "elm,state,show,left" :
               "elm,state,show,right"), "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        edje_object_signal_emit(sd->layout, "elm,state,show,down", "elm");
        break;

      default:
        break;
     }

   edje_object_signal_emit(sd->bg, "elm,state,show", "elm");
//   elm_layout_signal_emit(obj, "elm,state,show", "elm");
}

static void
_hide_signals_emit(Evas_Object *obj,
                   Elm_Ctxpopup_Direction dir)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->visible) return;

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_UP:
        edje_object_signal_emit(sd->layout, "elm,state,hide,up", "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        edje_object_signal_emit(sd->layout, (elm_widget_mirrored_get(obj) ? "elm,state,hide,right" :
               "elm,state,hide,left"), "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        edje_object_signal_emit(sd->layout, (elm_widget_mirrored_get(obj) ? "elm,state,hide,left" :
               "elm,state,hide,right"), "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        edje_object_signal_emit(sd->layout, "elm,state,hide,down", "elm");
        break;

      default:
        break;
     }

   edje_object_signal_emit(sd->bg, "elm,state,hide", "elm");
}

static void
_base_shift_by_arrow(Evas_Object *arrow,
                     Elm_Ctxpopup_Direction dir,
                     Evas_Coord_Rectangle *rect)
{
   Evas_Coord arrow_w, arrow_h;

   evas_object_geometry_get(arrow, NULL, NULL, &arrow_w, &arrow_h);

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        rect->x += arrow_w;
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        rect->x -= arrow_w;
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        rect->y += arrow_h;
        break;

      case ELM_CTXPOPUP_DIRECTION_UP:
        rect->y -= arrow_h;
        break;

      default:
         break;
     }
}

//TODO: compress item - different from opensource
static void
_compress_item(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;

   ELM_CTXPOPUP_DATA_GET(obj, sd);
   if (!sd) return;

   EINA_LIST_FOREACH(sd->items, elist, item)
     {
        edje_object_signal_emit(item->base.view, "elm,state,compress", "elm");
     }
}

static Eina_Bool
_elm_ctxpopup_smart_sub_object_add(Evas_Object *obj,
                                   Evas_Object *sobj)
{
   Elm_Widget_Smart_Class *parent_parent;

   parent_parent = (Elm_Widget_Smart_Class *)((Evas_Smart_Class *)
                                              _elm_ctxpopup_parent_sc)->parent;

   /* skipping layout's code, which registers size hint changing
    * callback on sub objects. a hack to make ctxpopup live, as it is,
    * on the new classing schema. this widget needs a total
    * rewrite. */
   if (!parent_parent->sub_object_add(obj, sobj))
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_elm_ctxpopup_smart_sizing_eval(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;
   Evas_Coord_Rectangle rect = { 0, 0, 1, 1 };
   Evas_Coord_Point box_size = { 0, 0 };
   Evas_Coord_Point _box_size = { 0, 0 };
   Evas_Coord maxw = 0;
   const char *str;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   //Box, Scroller
   EINA_LIST_FOREACH(sd->items, elist, item)
     {
        _item_sizing_eval(item);
        evas_object_size_hint_min_get(VIEW(item), &_box_size.x, &_box_size.y);

        str = edje_object_data_get(VIEW(item), "item_max_size");
        if (str)
          {
             maxw = atoi(str);
             maxw = maxw * elm_widget_scale_get(obj) * elm_config_scale_get();

             if (_box_size.x > maxw)
               {
                  edje_object_signal_emit(VIEW(item), "elm,state,text,ellipsis", "elm");
                  edje_object_message_signal_process(VIEW(item));
               }
          }

        if (!sd->horizontal)
          {
             if (_box_size.x > box_size.x)
               box_size.x = _box_size.x;
             if (_box_size.y != -1)
               box_size.y += _box_size.y;
          }
        else
          {
             if (_box_size.x != -1)
               box_size.x += _box_size.x;
             if (_box_size.y > box_size.y)
               box_size.y = _box_size.y;
          }
     }

   if (!sd->arrow) return;  /* simple way to flag "under deletion" */

   if ((!sd->content) && (sd->scr))
     {
        evas_object_size_hint_min_set(sd->box, box_size.x, box_size.y);
        elm_scroller_content_min_limit(sd->scr, EINA_TRUE, EINA_TRUE);
        evas_object_size_hint_min_set(sd->scr, box_size.x, box_size.y);
     }

   //Base
   sd->dir = _base_geometry_calc(obj, &rect);

   //TODO: compress item - different from opensource
   if (!sd->horizontal && !sd->content)
     _compress_item(obj);

   _arrow_update(obj, sd->dir, rect);
   _base_shift_by_arrow(sd->arrow, sd->dir, &rect);

   //resize scroller according to final size
   if ((!sd->content) && (sd->scr))
     {
        elm_scroller_content_min_limit(sd->scr, EINA_FALSE, EINA_FALSE);
        evas_object_smart_calculate(sd->scr);
     }

   evas_object_size_hint_min_set(ELM_WIDGET_DATA(sd)->resize_obj, rect.w, rect.h);
   evas_object_resize(ELM_WIDGET_DATA(sd)->resize_obj, rect.w, rect.h);

   evas_object_move(sd->layout, rect.x, rect.y);
   evas_object_resize(sd->layout, rect.w, rect.h);

   _show_signals_emit(obj, sd->dir);
}

static void
_on_parent_del(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   evas_object_del(data);
}

static void
_on_parent_move(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);


   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible)
     {
        _scroller_size_reset(sd);
        elm_layout_sizing_eval(data);
     }
}

static void
_on_parent_resize(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
}

static void
_parent_detach(Evas_Object *obj)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->parent) return;

   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);
   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_MOVE, _on_parent_move, obj);
   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_RESIZE, _on_parent_resize, obj);
}

static void
_on_content_resized(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   elm_box_recalculate(sd->box);
   elm_layout_sizing_eval(data);
}

//FIXME: lost the content size when theme hook is called.
static Eina_Bool
_elm_ctxpopup_smart_theme(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;
   int idx = 0;
   Eina_Bool rtl;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_ctxpopup_parent_sc)->theme(obj))
     return EINA_FALSE;

   rtl = elm_widget_mirrored_get(obj);

   elm_widget_theme_object_set
     (obj, sd->bg, "ctxpopup", "bg", elm_widget_style_get(obj));
   elm_widget_theme_object_set
     (obj, sd->arrow, "ctxpopup", "arrow", elm_widget_style_get(obj));
   elm_widget_theme_object_set
     (obj, sd->layout, "ctxpopup", "layout", elm_widget_style_get(obj));

   //Items
   EINA_LIST_FOREACH(sd->items, elist, item)
     {
        edje_object_mirrored_set(VIEW(item), rtl);

        if (item->label && item->icon)
          _elm_theme_object_set
             (obj, VIEW(item), "ctxpopup", "icon_text_style_item", elm_widget_style_get(obj));
        else if (item->label)
          {
             if(!sd->horizontal)
               _elm_theme_object_set(obj, VIEW(item), "ctxpopup", "text_style_item",
                                     elm_widget_style_get(obj));
             else
               _elm_theme_object_set(obj, VIEW(item), "ctxpopup", "text_style_item_horizontal",
                                     elm_widget_style_get(obj));
          }
        else if (item->icon)
          _elm_theme_object_set
             (obj, VIEW(item), "ctxpopup", "icon_style_item", elm_widget_style_get(obj));
        if (item->label)
          edje_object_part_text_set(VIEW(item), "elm.text", item->label);

        if (elm_widget_item_disabled_get(item))
          edje_object_signal_emit(VIEW(item), "elm,state,disabled", "elm");

       /*
        *  For separator, if the first item has visible separator,
        *  then it should be aligned with edge of the base part.
        *  In some cases, it gives improper display. Ex) rounded corner
        *  So the first item separator should be invisible.
        */
        if ((idx++) == 0)
          edje_object_signal_emit(VIEW(item), "elm,state,default", "elm");
        else
          edje_object_signal_emit(VIEW(item), "elm,state,separator", "elm");

        edje_object_message_signal_process(VIEW(item));
     }

   if (evas_object_visible_get(sd->bg))
     edje_object_signal_emit(sd->bg, "elm,state,show", "elm");

   if (sd->scr)
     {
        if (!strncmp(elm_object_style_get(obj), "default", strlen("default")))
           elm_object_style_set(sd->scr, "ctxpopup");
        else
           elm_object_style_set(sd->scr, elm_object_style_get(obj));
     }

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible)
     {
        _scroller_size_reset(sd);
        _elm_ctxpopup_smart_sizing_eval(obj);
     }

   return EINA_TRUE;
}

/* kind of a big and tricky override here: an internal box will hold
 * the actual content. content aliases won't be of much help here */
static Eina_Bool
_elm_ctxpopup_smart_content_set(Evas_Object *obj,
                                const char *part,
                                Evas_Object *content)
{
   Evas_Coord min_w = -1, min_h = -1;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((part) && (strcmp(part, "default")))
     return ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_set
              (obj, part, content);

   if (!content) return EINA_FALSE;

   if (content == sd->content) return EINA_TRUE;

   if (sd->items) elm_ctxpopup_clear(obj);
   if (sd->content) evas_object_del(sd->content);

   //box
   if (!sd->box)
     {
        sd->box = elm_box_add(obj);
        evas_object_size_hint_weight_set(sd->box, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        evas_object_event_callback_add
           (sd->box, EVAS_CALLBACK_RESIZE, _on_content_resized, obj);
        ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_set
           (obj, "elm.swallow.content", sd->box);
     }

   evas_object_size_hint_weight_set
     (content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set
     (content, EVAS_HINT_FILL, EVAS_HINT_FILL);

   /* since it's going to be a box content, not a layout's... */
   evas_object_show(content);

   evas_object_size_hint_min_get(content, &min_w, &min_h);
   evas_object_size_hint_min_set(sd->box, min_w, min_h);
   elm_box_pack_end(sd->box, content);

   sd->content = content;
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object *
_elm_ctxpopup_smart_content_get(const Evas_Object *obj,
                                const char *part)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((part) && (strcmp(part, "default")))
     return ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_get
              (obj, part);

   return sd->content;
}

static Evas_Object *
_elm_ctxpopup_smart_content_unset(Evas_Object *obj,
                                  const char *part)
{
   Evas_Object *content;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((part) && (strcmp(part, "default")))
     return ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_unset
              (obj, part);

   content = sd->content;
   if (!content) return NULL;

   evas_object_del(sd->box);
   sd->box = NULL;
   sd->content = NULL;
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);

   return content;
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if ((part) && (strcmp(part, "default"))) return;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   _item_label_set(ctxpopup_it, label);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if (part && strcmp(part, "default")) return NULL;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;
   return ctxpopup_it->label;
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if ((part) && (strcmp(part, "icon"))) return;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   ELM_CTXPOPUP_DATA_GET(WIDGET(ctxpopup_it), sd);

   _item_icon_set(ctxpopup_it, content);
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible)
     {
        _scroller_size_reset(sd);
        _elm_ctxpopup_smart_sizing_eval(WIDGET(ctxpopup_it));
     }
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if (part && strcmp(part, "icon")) return NULL;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;
   return ctxpopup_it->icon;
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   ELM_CTXPOPUP_DATA_GET(WIDGET(it), sd);
   if (!sd) return;

   if (elm_widget_item_disabled_get(it))
     edje_object_signal_emit(VIEW(ctxpopup_it), "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(VIEW(ctxpopup_it), "elm,state,enabled", "elm");
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   edje_object_signal_emit(VIEW(ctxpopup_it), emission, source);
}

static void
_bg_clicked_cb(void *data,
               Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__,
               const char *source __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   _hide_signals_emit(data, sd->dir);
}

static void
_on_show(void *data __UNUSED__,
         Evas *e __UNUSED__,
         Evas_Object *obj,
         void *event_info __UNUSED__)
{
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;
   int idx = 0;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((!sd->items) && (!sd->content)) return;

   sd->visible = EINA_TRUE;

   evas_object_show(sd->bg);
   evas_object_show(sd->arrow);
   evas_object_show(sd->layout);

   edje_object_signal_emit(sd->bg, "elm,state,show", "elm");
   elm_layout_signal_emit(obj, "elm,state,show", "elm");

   EINA_LIST_FOREACH(sd->items, elist, item)
     {
        if (item->label && !item->icon)
          {
             if(!sd->horizontal)
               _elm_theme_object_set(obj, VIEW(item), "ctxpopup", "text_style_item",
                                     elm_widget_style_get(obj));
             else
               _elm_theme_object_set(obj, VIEW(item), "ctxpopup", "text_style_item_horizontal",
                                     elm_widget_style_get(obj));
          }

        if (idx++ == 0)
          edje_object_signal_emit(VIEW(item), "elm,state,default", "elm");
        else
          edje_object_signal_emit(VIEW(item), "elm,state,separator", "elm");
     }

   elm_layout_sizing_eval(obj);

   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_on_hide(void *data __UNUSED__,
         Evas *e __UNUSED__,
         Evas_Object *obj,
         void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->visible) return;

   evas_object_hide(sd->bg);
   evas_object_hide(sd->arrow);
   evas_object_hide(sd->layout);

   _scroller_size_reset(sd);

   sd->visible = EINA_FALSE;
}

static void
_on_move(void *data __UNUSED__,
         Evas *e __UNUSED__,
         Evas_Object *obj,
         void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (sd->visible) evas_object_show(sd->arrow);

   _scroller_size_reset(sd);
   _elm_ctxpopup_smart_sizing_eval(obj);
}

static void
_hide_finished_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  const char *emission __UNUSED__,
                  const char *source __UNUSED__)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
}

static void
_scroller_resize(void *data, Evas *e __UNUSED__,
                 Evas_Object *obj,
                 void *event_info __UNUSED__)
{
   Evas_Coord w, h;

   ELM_CTXPOPUP_DATA_GET(data, sd);
   if (!sd) return;
   if (!sd->visible) return;
   if (sd->finished) return;

   evas_object_geometry_get(obj, 0, 0, &w, &h);

   if (w != 0 && h != 0)
     {
        if ((w <= sd->max_sc_w) && (h <= sd->max_sc_h))
          {
             _elm_ctxpopup_smart_sizing_eval(data);
             sd->finished = EINA_TRUE;
             return;
          }
     }

   if (sd->max_sc_w < w)
     sd->max_sc_w = w;
   if (sd->max_sc_h < h)
     sd->max_sc_h = h;

   _elm_ctxpopup_smart_sizing_eval(data);
}

static void
_ctxpopup_restack_cb(void *data __UNUSED__,
                     Evas *e __UNUSED__,
                     Evas_Object *obj,
                     void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);
   evas_object_stack_below(sd->bg, obj);
   evas_object_stack_below(sd->layout, obj);
}

static void
_list_del(Elm_Ctxpopup_Smart_Data *sd)
{
   if (!sd->scr) return;

   edje_object_part_unswallow(ELM_WIDGET_DATA(sd)->resize_obj, sd->scr);
   evas_object_del(sd->scr);
   sd->scr = NULL;
   evas_object_del(sd->box);
   sd->box = NULL;
}

static void
_list_new(Evas_Object *obj)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);
   if (!sd) return;

   //scroller
   sd->scr = elm_scroller_add(obj);
   elm_object_style_set(sd->scr, "ctxpopup");
   evas_object_size_hint_align_set(sd->scr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add(sd->scr, EVAS_CALLBACK_RESIZE,
                                  _scroller_resize, obj);
   edje_object_part_swallow(ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.content", sd->scr);

   //box
   if (!sd->box)
     {
        sd->box = elm_box_add(obj);
        evas_object_size_hint_weight_set(sd->box, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        evas_object_event_callback_add
           (sd->box, EVAS_CALLBACK_RESIZE, _on_content_resized, obj);
        ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_set
           (obj, "elm.swallow.content", sd->box);
     }

   elm_object_content_set(sd->scr, sd->box);
   elm_ctxpopup_horizontal_set(obj, sd->horizontal);
}

static Eina_Bool
_elm_ctxpopup_smart_disable(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Object_Item *it;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_ctxpopup_parent_sc)->disable(obj))
     return EINA_FALSE;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_object_item_disabled_set(it, elm_widget_disabled_get(obj));

   return EINA_TRUE;
}

static void
_access_outline_activate_cb(void *data,
                        Evas_Object *part_obj __UNUSED__,
                        Elm_Object_Item *item __UNUSED__)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Evas_Object *ao;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (is_access)
     {
        ao = _elm_access_edje_object_part_object_register
               (obj, ELM_WIDGET_DATA(sd)->resize_obj, ACCESS_OUTLINE_PART);
        _elm_access_text_set(_elm_access_object_get(ao),
                             ELM_ACCESS_TYPE, E_(OUTLINE_TEXT));
        _elm_access_activate_callback_set
          (_elm_access_object_get(ao), _access_outline_activate_cb, obj);
     }
   else
     {
        _elm_access_edje_object_part_object_unregister
               (obj, ELM_WIDGET_DATA(sd)->resize_obj, ACCESS_OUTLINE_PART);
     }
}

static void
_elm_ctxpopup_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Ctxpopup_Smart_Data);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_ctxpopup_parent_sc)->base.add(obj);

   elm_layout_theme_set(obj, "ctxpopup", "base", elm_widget_style_get(obj));
   elm_layout_signal_callback_add
     (obj, "elm,action,hide,finished", "", _hide_finished_cb, obj);

   //Background
   priv->bg = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set(obj, priv->bg, "ctxpopup", "bg", "default");
   edje_object_signal_callback_add
     (priv->bg, "elm,action,click", "", _bg_clicked_cb, obj);

   evas_object_stack_below(priv->bg, obj);

   //Arrow
   priv->arrow = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set
     (obj, priv->arrow, "ctxpopup", "arrow", "default");

   priv->dir_priority[0] = ELM_CTXPOPUP_DIRECTION_UP;
   priv->dir_priority[1] = ELM_CTXPOPUP_DIRECTION_LEFT;
   priv->dir_priority[2] = ELM_CTXPOPUP_DIRECTION_RIGHT;
   priv->dir_priority[3] = ELM_CTXPOPUP_DIRECTION_DOWN;
   priv->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   evas_object_event_callback_add
      (obj, EVAS_CALLBACK_RESTACK, _ctxpopup_restack_cb, obj);

   evas_object_smart_callback_add
      (obj, "scroll-freeze-on", _freeze_on_cb, obj);
   evas_object_smart_callback_add
      (obj, "scroll-freeze-off", _freeze_off_cb, obj);
   evas_object_smart_callback_add
      (obj, "scroll-hold-on", _hold_on_cb, obj);
   evas_object_smart_callback_add
      (obj, "scroll-hold-off", _hold_off_cb, obj);

   priv->box = elm_box_add(obj);
   evas_object_size_hint_weight_set
     (priv->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_event_callback_add
     (priv->box, EVAS_CALLBACK_RESIZE, _on_content_resized, obj);

   /* box will be our content placeholder, thus the parent's version call */
   ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_set
     (obj, "elm.swallow.content", priv->box);

   priv->layout = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set(obj, priv->layout, "ctxpopup", "layout", "default");

   edje_object_signal_callback_add
     (priv->layout, "elm,action,hide,finished", "", _hide_finished_cb, obj);
   edje_object_part_swallow(priv->layout, "swallow", ELM_WIDGET_DATA(sd)->resize_obj);
   evas_object_size_hint_weight_set
     (priv->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _on_show, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _on_hide, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _on_move, NULL);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   /* access */
   if (_elm_config->access_mode) _access_obj_process(obj, EINA_TRUE);
}

static void
_elm_ctxpopup_smart_del(Evas_Object *obj)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   evas_object_event_callback_del_full
     (sd->box, EVAS_CALLBACK_RESIZE, _on_content_resized, obj);
   _parent_detach(obj);

   elm_ctxpopup_clear(obj);
   evas_object_del(sd->arrow);
   sd->arrow = NULL; /* stops _sizing_eval() from going on on deletion */

   evas_object_del(sd->bg);
   sd->bg = NULL;

   evas_object_del(sd->layout);
   sd->layout = NULL;

   ELM_WIDGET_CLASS(_elm_ctxpopup_parent_sc)->base.del(obj);
}

static void
_elm_ctxpopup_smart_parent_set(Evas_Object *obj,
                               Evas_Object *parent)
{
   //default parent is to be hover parent
   elm_ctxpopup_hover_parent_set(obj, parent);
}

static void
_elm_ctxpopup_smart_access(Evas_Object *obj, Eina_Bool is_access)
{
   Eina_List *elist = NULL;
   Elm_Ctxpopup_Item *it;

   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   _access_obj_process(obj, is_access);

   EINA_LIST_FOREACH(sd->items, elist, it)
     _access_widget_item_register(it, is_access);
}

static void
_elm_ctxpopup_smart_set_user(Elm_Ctxpopup_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_ctxpopup_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_ctxpopup_smart_del;

   ELM_WIDGET_CLASS(sc)->parent_set = _elm_ctxpopup_smart_parent_set;
   ELM_WIDGET_CLASS(sc)->disable = _elm_ctxpopup_smart_disable;
   ELM_WIDGET_CLASS(sc)->event = _elm_ctxpopup_smart_event;
   ELM_WIDGET_CLASS(sc)->theme = _elm_ctxpopup_smart_theme;
   ELM_WIDGET_CLASS(sc)->sub_object_add = _elm_ctxpopup_smart_sub_object_add;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_ctxpopup_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_ctxpopup_smart_focus_direction;
   ELM_WIDGET_CLASS(sc)->translate = _elm_ctxpopup_smart_translate;

   ELM_CONTAINER_CLASS(sc)->content_get = _elm_ctxpopup_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_set = _elm_ctxpopup_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_ctxpopup_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_ctxpopup_smart_sizing_eval;

   ELM_WIDGET_CLASS(sc)->access = _elm_ctxpopup_smart_access;
}

EAPI const Elm_Ctxpopup_Smart_Class *
elm_ctxpopup_smart_class_get(void)
{
   static Elm_Ctxpopup_Smart_Class _sc =
     ELM_CTXPOPUP_SMART_CLASS_INIT_NAME_VERSION(ELM_CTXPOPUP_SMART_NAME);
   static const Elm_Ctxpopup_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_ctxpopup_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_ctxpopup_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_ctxpopup_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   /* access: parent could be any object such as elm_list which does
      not know elc_ctxpopup as its child object in the focus_next(); */
   ELM_WIDGET_DATA_GET(obj, sd);
   sd->highlight_root = EINA_TRUE;

   return obj;
}

EAPI void
elm_ctxpopup_hover_parent_set(Evas_Object *obj,
                              Evas_Object *parent)
{
   Evas_Coord x, y, w, h;

   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!parent) return;

   _parent_detach(obj);

   evas_object_event_callback_add
     (parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);
   evas_object_event_callback_add
     (parent, EVAS_CALLBACK_MOVE, _on_parent_move, obj);
   evas_object_event_callback_add
     (parent, EVAS_CALLBACK_RESIZE, _on_parent_resize, obj);

   sd->parent = parent;

   //Update Background
   evas_object_geometry_get(parent, &x, &y, &w, &h);
   evas_object_move(sd->bg, x, y);
   evas_object_resize(sd->bg, w, h);

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EAPI Evas_Object *
elm_ctxpopup_hover_parent_get(const Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj) NULL;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   return sd->parent;
}

EAPI void
elm_ctxpopup_clear(Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   _items_remove(sd);
   _list_del(sd);
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;
}

EAPI void
elm_ctxpopup_horizontal_set(Evas_Object *obj,
                            Eina_Bool horizontal)
{
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;
   int idx = 0;

   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   sd->horizontal = !!horizontal;

   if (!sd->scr)
      return;

  if (!horizontal)
     {
        elm_box_horizontal_set(sd->box, EINA_FALSE);
        elm_scroller_bounce_set(sd->scr, EINA_FALSE, EINA_TRUE);
     }
   else
     {
        elm_box_horizontal_set(sd->box, EINA_TRUE);
        elm_scroller_bounce_set(sd->scr, EINA_TRUE, EINA_FALSE);
     }

   EINA_LIST_FOREACH(sd->items, elist, item)
     {
        if (item->label && !item->icon && !horizontal)
          _elm_theme_object_set(obj, VIEW(item), "ctxpopup", "text_style_item",
                                elm_widget_style_get(obj));
        else if (item->label && !item->icon && horizontal)
          _elm_theme_object_set(obj, VIEW(item), "ctxpopup", "text_style_item_horizontal",
                                elm_widget_style_get(obj));

        if (idx++ == 0)
          edje_object_signal_emit(VIEW(item), "elm,state,default", "elm");
        else
          edje_object_signal_emit(VIEW(item), "elm,state,separator", "elm");

        _item_disable_hook((Elm_Object_Item *)item);
     }

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EAPI Eina_Bool
elm_ctxpopup_horizontal_get(const Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj) EINA_FALSE;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   return sd->horizontal;
}

EAPI Elm_Object_Item *
elm_ctxpopup_item_append(Evas_Object *obj,
                         const char *label,
                         Evas_Object *icon,
                         Evas_Smart_Cb func,
                         const void *data)
{
   Elm_Ctxpopup_Item *item;
   Evas_Object *content;

   ELM_CTXPOPUP_CHECK(obj) NULL;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   item = elm_widget_item_new(obj, Elm_Ctxpopup_Item);
   if (!item) return NULL;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_disable_hook_set(item, _item_disable_hook);
   elm_widget_item_text_set_hook_set(item, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(item, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(item, _item_content_get_hook);
   elm_widget_item_signal_emit_hook_set(item, _item_signal_emit_hook);

   //The first item is appended.
   content = elm_object_content_unset(obj);
   if (content) evas_object_del(content);

   if (!sd->items)
     _list_new(obj);

   item->func = func;
   item->base.data = data;

   if (icon && label)
     _item_new(item, "icon_text_style_item");
   else if (label)
     {
        if (!sd->horizontal)
          _item_new(item, "text_style_item");
        else
          _item_new(item, "text_style_item_horizontal");
     }
   else
     _item_new(item, "icon_style_item");

   _item_icon_set(item, icon);
   _item_label_set(item, label);
   elm_box_pack_end(sd->box, VIEW(item));
   sd->items = eina_list_append(sd->items, item);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible)
     {
        _scroller_size_reset(sd);
        _elm_ctxpopup_smart_sizing_eval(obj);
     }


   return (Elm_Object_Item *)item;
}

EAPI void
elm_ctxpopup_direction_priority_set(Evas_Object *obj,
                                    Elm_Ctxpopup_Direction first,
                                    Elm_Ctxpopup_Direction second,
                                    Elm_Ctxpopup_Direction third,
                                    Elm_Ctxpopup_Direction fourth)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   sd->dir_priority[0] = first;
   sd->dir_priority[1] = second;
   sd->dir_priority[2] = third;
   sd->dir_priority[3] = fourth;

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EAPI void
elm_ctxpopup_direction_priority_get(Evas_Object *obj,
                                    Elm_Ctxpopup_Direction *first,
                                    Elm_Ctxpopup_Direction *second,
                                    Elm_Ctxpopup_Direction *third,
                                    Elm_Ctxpopup_Direction *fourth)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (first) *first = sd->dir_priority[0];
   if (second) *second = sd->dir_priority[1];
   if (third) *third = sd->dir_priority[2];
   if (fourth) *fourth = sd->dir_priority[3];
}

EAPI Elm_Ctxpopup_Direction
elm_ctxpopup_direction_get(const Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj) ELM_CTXPOPUP_DIRECTION_UNKNOWN;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   return sd->dir;
}

EAPI void
elm_ctxpopup_dismiss(Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   _hide_signals_emit(obj, sd->dir);
}
