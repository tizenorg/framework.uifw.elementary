#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_multibuttonentry.h"

EAPI const char ELM_MULTIBUTTONENTRY_SMART_NAME[] = "elm_multibuttonentry";

//widget signals
static const char SIG_ITEM_SELECTED[] = "item,selected";
static const char SIG_ITEM_ADDED[] = "item,added";
static const char SIG_ITEM_DELETED[] = "item,deleted";
static const char SIG_ITEM_CLICKED[] = "item,clicked";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_EXPANDED[] = "expanded";
static const char SIG_CONTRACTED[] = "contracted";
static const char SIG_EXPAND_STATE_CHANGED[] = "expand,state,changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_ITEM_SELECTED, ""},
   {SIG_ITEM_ADDED, ""},
   {SIG_ITEM_DELETED, ""},
   {SIG_ITEM_CLICKED, ""},
   {SIG_CLICKED, ""},
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {SIG_EXPANDED, ""},
   {SIG_CONTRACTED, ""},
   {SIG_EXPAND_STATE_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_MULTIBUTTONENTRY_SMART_NAME, _elm_multibuttonentry,
  Elm_Multibuttonentry_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, _smart_callbacks);

static Eina_Bool
_guide_packed(Evas_Object *obj)
{
   Eina_List *children;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!sd->guide) return EINA_FALSE;

   children = elm_box_children_get(sd->box);
   if (sd->guide == eina_list_data_get(eina_list_last(children)))
     {
        eina_list_free(children);
        return EINA_TRUE;
     }
   else
     {
        eina_list_free(children);
        return EINA_FALSE;
     }
}

static Eina_Bool
_label_packed(Evas_Object *obj)
{
   Eina_List *children;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!sd->label) return EINA_FALSE;

   children = elm_box_children_get(sd->box);
   if (sd->label == eina_list_data_get(children))
     {
        eina_list_free(children);
        return EINA_TRUE;
     }
   else
     {
        eina_list_free(children);
        return EINA_FALSE;
     }
}

static void
_guide_set(Evas_Object *obj,
           const char *text)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!sd->guide && !strlen(text)) return;

   if (!sd->guide)
     {
        sd->guide = elm_layout_add(obj);
        elm_layout_theme_set
           (sd->guide, "multibuttonentry", "guide", elm_widget_style_get(obj));
     }
   elm_object_text_set(sd->guide, text);

   if (!sd->items && !elm_object_focus_get(obj) && !_guide_packed(obj))
     {
        if (sd->editable)
          {
             elm_box_unpack(sd->box, sd->entry);
             evas_object_hide(sd->entry);
          }

        elm_box_pack_end(sd->box, sd->guide);
        evas_object_show(sd->guide);
     }
}

static void
_label_set(Evas_Object *obj,
           const char *text)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!sd->label && !strlen(text)) return;

   if (!sd->label)
     {
        sd->label = elm_layout_add(obj);
        elm_layout_theme_set
           (sd->label, "multibuttonentry", "label", elm_widget_style_get(obj));
     }
   elm_object_text_set(sd->label, text);

   if (strlen(text) && !_label_packed(obj))
     {
        elm_box_pack_start(sd->box, sd->label);
        evas_object_show(sd->label);
     }
   else if (!strlen(text) && _label_packed(obj))
     {
        elm_box_unpack(sd->box, sd->label);
        evas_object_hide(sd->label);
     }
}

static void
_layout_expand(Evas_Object *obj)
{
   int count, items_count, i;
   Eina_List *children;
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!sd->items) return;
   if (sd->expanded_state) return;

   children = elm_box_children_get(sd->box);
   count = eina_list_count(children);
   if (sd->number)
     {
        evas_object_del(sd->number);
        sd->number = NULL;
        count--;
     }
   if (sd->label && _label_packed(obj)) count--;
   eina_list_free(children);

   items_count = eina_list_count(sd->items);
   for (i = count; i < items_count; i++)
     {
        it = eina_list_nth(sd->items, i);
        elm_box_pack_end(sd->box, VIEW(it));
        evas_object_show(VIEW(it));
     }

   if (sd->editable)
     {
        elm_box_pack_end(sd->box, sd->entry);
        evas_object_show(sd->entry);
     }

   sd->expanded_state = EINA_TRUE;
   evas_object_smart_callback_call(obj, SIG_EXPAND_STATE_CHANGED, NULL);
}

static void
_layout_shrink(Evas_Object *obj,
               Eina_Bool force)
{
   Evas_Coord w, mnw, linew = 0, hpad = 0;
   int count = 0, items_count, i;
   Eina_List *l, *children;
   Evas_Object *child;
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!sd->items) return;
   if (!sd->expanded_state && !force) return;

   evas_object_geometry_get(sd->box, NULL, NULL, &w, NULL);
   elm_box_padding_get(sd->box, &hpad, NULL);

   if (sd->label && _label_packed(obj))
     {
        evas_object_size_hint_min_get(sd->label, &mnw, NULL);
        linew += mnw;
        linew += hpad;
     }

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        evas_object_size_hint_min_get(VIEW(it), &mnw, NULL);
        linew += mnw;

        if (linew > w)
          {
             linew -= mnw;
             break;
          }

        count++;
        linew += hpad;
     }

   if(!count) return;

   items_count = eina_list_count(sd->items);
   if (count < items_count)
     {
        char buf[16];

        if (!sd->number)
          {
             sd->number = elm_layout_add(obj);
             elm_layout_theme_set
                (sd->number, "multibuttonentry", "number",
                 elm_widget_style_get(obj));
          }
        snprintf(buf, sizeof(buf), "+%d", items_count - count);
        elm_object_text_set(sd->number, buf);
        evas_object_smart_calculate(sd->number);
        evas_object_size_hint_min_get(sd->number, &mnw, NULL);
        linew += mnw;

        if (linew > w)
          {
             count--;
             snprintf(buf, sizeof(buf), "+%d", items_count - count);
             elm_object_text_set(sd->number, buf);
          }

        if (!force)
          {
             for (i = count; i < items_count; i++)
               {
                  it = eina_list_nth(sd->items, i);
                  elm_box_unpack(sd->box, VIEW(it));
                  evas_object_hide(VIEW(it));
               }

             if (sd->editable)
               {
                  elm_box_unpack(sd->box, sd->entry);
                  evas_object_hide(sd->entry);
               }
          }
        else
          {
             // if it is called from item_append_xxx, item_del functions,
             // all items are unpacked and packed again
             children = elm_box_children_get(sd->box);
             EINA_LIST_FREE(children, child)
               {
                  if (child != sd->label)
                    {
                       elm_box_unpack(sd->box, child);
                       evas_object_hide(child);
                    }
               }

             for (i = 0; i < count; i++)
               {
                  it = eina_list_nth(sd->items, i);
                  elm_box_pack_end(sd->box, VIEW(it));
                  evas_object_show(VIEW(it));
               }
          }

        elm_box_pack_end(sd->box, sd->number);
        evas_object_show(sd->number);

        sd->expanded_state = EINA_FALSE;
     }
   else
     {
        if (!force)
          {
             if (sd->editable)
               {
                  evas_object_size_hint_min_get(sd->entry, &mnw, NULL);
                  linew += mnw;
                  if (linew > w)
                    {
                       elm_box_unpack(sd->box, sd->entry);
                       evas_object_hide(sd->entry);
                       sd->expanded_state = EINA_FALSE;
                    }
               }
          }
        else
          {
             if (sd->number)
               {
                  evas_object_del(sd->number);
                  sd->number = NULL;
               }

             // if it is called from item_append_xxx, item_del functions,
             // all items are unpacked and packed again
             children = elm_box_children_get(sd->box);
             EINA_LIST_FREE(children, child)
               {
                  if (child != sd->label)
                    {
                       elm_box_unpack(sd->box, child);
                       evas_object_hide(child);
                    }
               }

             for (i = 0; i < count; i++)
               {
                  it = eina_list_nth(sd->items, i);
                  elm_box_pack_end(sd->box, VIEW(it));
                  evas_object_show(VIEW(it));
               }

             sd->expanded_state = EINA_TRUE;

             if (sd->editable)
               {
                  evas_object_size_hint_min_get(sd->entry, &mnw, NULL);
                  linew += mnw;
                  if (linew > w)
                     sd->expanded_state = EINA_FALSE;
                  else
                    {
                       elm_box_pack_end(sd->box, sd->entry);
                       evas_object_show(sd->entry);
                    }
               }
          }
     }

   if (!sd->expanded_state && !force)
      evas_object_smart_callback_call(obj, SIG_EXPAND_STATE_CHANGED, NULL);
}

static Eina_Bool
_box_min_size_calculate(Evas_Object *box,
                    Evas_Object_Box_Data *priv,
                    int *line_height)
{
   Evas_Coord mnw, mnh, w, minw, minh = 0, linew = 0, lineh = 0;
   int line_num;
   Eina_List *l;
   Evas_Object_Box_Option *opt;

   evas_object_geometry_get(box, NULL, NULL, &w, NULL);
   evas_object_size_hint_min_get(box, &minw, NULL);
   if (!w) return EINA_FALSE;

   line_num = 1;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        evas_object_size_hint_min_get(opt->obj, &mnw, &mnh);

        linew += mnw;
        if (lineh < mnh) lineh = mnh;

        if (linew > w)
          {
             linew = mnw;
             line_num++;
          }

        if (l != eina_list_last(priv->children))
           linew += priv->pad.h;
     }

   minh = lineh * line_num + (line_num - 1) * priv->pad.v;

   evas_object_size_hint_min_set(box, minw, minh);
   *line_height = lineh;

   return EINA_TRUE;
}

static void
_box_layout(Evas_Object *o,
            Evas_Object_Box_Data *priv,
            void *data)
{
   Evas_Coord x, y, w, h, xx, yy, minw, minh, linew = 0, lineh = 0;
   double ax, ay;
   Eina_Bool rtl;
   Eina_List *l;
   Evas_Object *obj;
   Evas_Object_Box_Option *opt;
   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   if (!_box_min_size_calculate(o, priv, &lineh)) return;

   evas_object_geometry_get(o, &x, &y, &w, &h);
   evas_object_size_hint_min_get(o, &minw, &minh);
   evas_object_size_hint_align_get(o, &ax, &ay);

   rtl = elm_widget_mirrored_get(data);
   if (rtl) ax = 1.0 - ax;

   if (w < minw)
     {
        x = x + ((w - minw) * (1.0 - ax));
        w = minw;
     }
   if (h < minh)
     {
        y = y + ((h - minh) * (1.0 - ay));
        h = minh;
     }

   xx = x;
   yy = y;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Coord mnw, mnh, ww, hh, ow, oh;
        double wx, wy;
        Eina_Bool fw, fh, xw, xh;

        obj = opt->obj;
        evas_object_size_hint_align_get(obj, &ax, &ay);
        evas_object_size_hint_weight_get(obj, &wx, &wy);
        evas_object_size_hint_min_get(obj, &mnw, &mnh);

        fw = fh = EINA_FALSE;
        xw = xh = EINA_FALSE;
        if (ax == -1.0) {fw = EINA_TRUE; ax = 0.5;}
        if (ay == -1.0) {fh = EINA_TRUE; ay = 0.5;}
        if (rtl) ax = 1.0 - ax;
        if (wx > 0.0) xw = EINA_TRUE;
        if (wy > 0.0) xh = EINA_TRUE;

        ww = mnw;
        if (xw)
          {
             if (ww <= w - linew) ww = w - linew;
             else ww = w;
          }
        hh = lineh;

        ow = mnw;
        if (fw) ow = ww;
        oh = mnh;
        if (fh) oh = hh;

        linew += ww;
        if (linew > w)
          {
             xx = x;
             yy += hh;
             yy += priv->pad.v;
             linew = ww;
          }
        linew += priv->pad.h;

        evas_object_move(obj,
                         ((!rtl) ? (xx) : (x + (w - (xx - x) - ww)))
                         + (Evas_Coord)(((double)(ww - ow)) * ax),
                         yy + (Evas_Coord)(((double)(hh - oh)) * ay));
	evas_object_resize(obj, ow, oh);

        xx += ww;
        xx += priv->pad.h;
     }

   // when expanded_set is called before box layout is calculated,
   // it should be called again 
   if (!elm_object_focus_get(data) && sd->expanded != sd->expanded_state)
     {
        if (sd->expanded)
           _layout_expand(data);
        else
           _layout_shrink(data, EINA_FALSE);
     }
}

static void
_on_box_resize(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Evas_Coord w, h;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   if (sd->boxh < h)
      evas_object_smart_callback_call(data, SIG_EXPANDED, NULL);
   else if (sd->boxh > h)
      evas_object_smart_callback_call(data, SIG_CONTRACTED, NULL);

   // on rotation, items should be packed again in the shrinked layout
   if (sd->boxw && sd->boxw != w)
     {
        if (!elm_object_focus_get(data) && !sd->expanded)
           _layout_shrink(data, EINA_TRUE);
     }

   sd->boxh = h;
   sd->boxw = w;
}

static Elm_Multibuttonentry_Item_Filter *
_filter_new(Elm_Multibuttonentry_Item_Filter_Cb func,
            const void *data)
{
   Elm_Multibuttonentry_Item_Filter *ft;

   ft = ELM_NEW(Elm_Multibuttonentry_Item_Filter);
   if (!ft) return NULL;

   ft->func = func;
   ft->data = data;

   return ft;
}

static void
_on_item_clicked(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   Elm_Multibuttonentry_Item *it = data;

   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   evas_object_focus_set(elm_layout_edje_get(VIEW(it)), EINA_TRUE);

   // handles input panel because it can be hidden by user
   if (sd->editable)
      elm_entry_input_panel_show(sd->entry);

   evas_object_smart_callback_call(WIDGET(it), SIG_ITEM_CLICKED, it);
}

static void
_on_item_selected(void *data,
                  Evas_Object *obj __UNUSED__,
                  const char *emission __UNUSED__,
                  const char *source __UNUSED__)
{
   Elm_Multibuttonentry_Item *it = data;

   if (it->func) it->func((void *)it->base.data, WIDGET(it), it);

   evas_object_smart_callback_call(WIDGET(it), SIG_ITEM_SELECTED, it);
}

static void
_on_item_focus(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Elm_Multibuttonentry_Item *it = data;

   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   if (evas_object_focus_get(obj))
     {
        if (elm_object_focus_get(sd->entry))
           elm_object_focus_set(sd->entry, EINA_FALSE);

        sd->selected_item = (Elm_Object_Item *)it;
        elm_layout_signal_emit(VIEW(it), "elm,state,bg,focus", "elm");

        if (sd->editable)
           elm_entry_input_panel_show(sd->entry);
     }
   else
     {
        sd->selected_item = NULL;
        elm_layout_signal_emit(VIEW(it), "elm,state,bg,unfocus", "elm");

        if (sd->editable)
           elm_entry_input_panel_hide(sd->entry);
     }
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *text)
{
   if (part && strcmp(part, "elm.text")) return;
   if (!text) return;

   elm_object_text_set(VIEW(it), text);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "elm.text")) return NULL;

   return elm_object_text_get(VIEW(it));
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   sd->items = eina_list_remove(sd->items, it);

   if (!elm_object_focus_get(WIDGET(it)) && !sd->expanded)
      _layout_shrink(WIDGET(it), EINA_TRUE);

   if (!sd->items && !elm_object_focus_get(WIDGET(it)) &&
       sd->guide && !_guide_packed(WIDGET(it)))
     {
        if (sd->editable)
          {
             elm_box_unpack(sd->box, sd->entry);
             evas_object_hide(sd->entry);
          }

        elm_box_pack_end(sd->box, sd->guide);
        evas_object_show(sd->guide);
     }

   evas_object_smart_callback_call(WIDGET(it), SIG_ITEM_DELETED, it);

   return EINA_TRUE;
}

static Elm_Multibuttonentry_Item *
_item_new(Evas_Object *obj,
          const char *text,
          Evas_Smart_Cb func,
          const void *data)
{
   Elm_Multibuttonentry_Item *it;
   Elm_Multibuttonentry_Item_Filter *ft;
   Eina_List *l;
   const char *str;
   Evas_Coord w, maxw = 0;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->filters, l, ft)
     {
        if (!ft->func(obj, text, data, ft->data))
           return NULL;
     }

   it = elm_widget_item_new(obj, Elm_Multibuttonentry_Item);
   if (!it) return NULL;

   VIEW(it) = elm_layout_add(obj);
   elm_layout_theme_set
      (VIEW(it), "multibuttonentry", "button", elm_widget_style_get(obj));
   elm_layout_signal_callback_add
      (VIEW(it), "elm,action,clicked", "", _on_item_clicked, it);
   elm_layout_signal_callback_add
      (VIEW(it), "elm,action,selected", "", _on_item_selected, it);
   evas_object_event_callback_add
      (elm_layout_edje_get(VIEW(it)),
       EVAS_CALLBACK_FOCUS_IN, _on_item_focus, it);
   evas_object_event_callback_add
      (elm_layout_edje_get(VIEW(it)),
       EVAS_CALLBACK_FOCUS_OUT, _on_item_focus, it);
   elm_object_text_set(VIEW(it), text);

   it->func = func;
   it->base.data = data;

   // adjust item size if item is longer than maximum size
   evas_object_smart_calculate(VIEW(it));
   evas_object_size_hint_min_get(VIEW(it), &w, NULL);
   str = elm_layout_data_get(VIEW(it), "max_size");
   if (str) maxw = atoi(str);
   maxw = maxw * elm_widget_scale_get(obj) * elm_config_scale_get();
   if (w > maxw)
     {
        elm_layout_signal_emit(VIEW(it), "elm,state,text,ellipsis", "elm");
        elm_layout_sizing_eval(VIEW(it));
     }

   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);

   sd->item_be_selected = EINA_TRUE;

   return it;
}

static void
_on_entry_unfocused(void *data,
                    Evas_Object *obj,
                    void *event_info __UNUSED__)
{
   char *str;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   str = elm_entry_markup_to_utf8(elm_object_text_get(obj));
   if (strlen(str))
     {
        Elm_Multibuttonentry_Item *it;

        it = _item_new(data, str, NULL, NULL);
        if (!it) return;

        sd->items = eina_list_append(sd->items, it);
        elm_box_pack_before(sd->box, VIEW(it), obj);
        evas_object_show(VIEW(it));
        evas_object_smart_callback_call(data, SIG_ITEM_ADDED, it);

        elm_object_text_set(obj, "");
     }
     free(str);
}

// handles semicolon, comma (before inserting them to the entry)
static void
_entry_filter(void *data,
              Evas_Object *entry,
              char **text)
{
   char *str;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   if (!*text || !strlen(*text)) return;
   if (strcmp(*text, ";") && strcmp(*text, ",")) return;

   str = elm_entry_markup_to_utf8(elm_object_text_get(entry));
   if (strlen(str))
     {
        Elm_Multibuttonentry_Item *it;

        it = _item_new(data, str, NULL, NULL);
        if (!it) return;

        sd->items = eina_list_append(sd->items, it);
        elm_box_pack_before(sd->box, VIEW(it), entry);
        evas_object_show(VIEW(it));
        evas_object_smart_callback_call(data, SIG_ITEM_ADDED, it);

        elm_object_text_set(entry, "");
     }
   free(str);

   free(*text);
   *text = NULL;
}

// handles enter key
static void
_on_entry_key_up(void *data,
                 Evas *e __UNUSED__,
                 Evas_Object *obj,
                 void *event_info)
{
   char *str;
   Evas_Event_Key_Up *ev = (Evas_Event_Key_Up *)event_info;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   // cancels item_be_selected when text inserting is started 
   if (strcmp(ev->keyname, "KP_Enter") && strcmp(ev->keyname, "Return") &&
       strcmp(ev->keyname, "BackSpace") && strcmp(ev->keyname, "Delete") &&
       strcmp(ev->keyname, "semicolon") && strcmp(ev->keyname, "comma"))
     {
        sd->item_be_selected = EINA_FALSE;
        return;
     }

   if (!strcmp(ev->keyname, "KP_Enter") || !strcmp(ev->keyname, "Return"))
     {
        str = elm_entry_markup_to_utf8(elm_object_text_get(obj));
        if (strlen(str))
          {
             Elm_Multibuttonentry_Item *it;

             it = _item_new(data, str, NULL, NULL);
             if (!it) return;

             sd->items = eina_list_append(sd->items, it);
             elm_box_pack_before(sd->box, VIEW(it), obj);
             evas_object_show(VIEW(it));
             evas_object_smart_callback_call(data, SIG_ITEM_ADDED, it);

             elm_object_text_set(obj, "");
          }
        free(str);
     }
}

// handles delete key
// it can be pressed when button is selected, so it is handled on layout_key_up)
static void
_on_layout_key_up(void *data __UNUSED__,
                 Evas *e __UNUSED__,
                 Evas_Object *obj,
                 void *event_info)
{
   char *str;
   Evas_Event_Key_Up *ev = (Evas_Event_Key_Up *)event_info;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (strcmp(ev->keyname, "BackSpace") && strcmp(ev->keyname, "Delete"))
      return;

   str = elm_entry_markup_to_utf8(elm_object_text_get(sd->entry));
   if (strlen(str))
     {
        free(str);
        return;
     }
   free(str);

   if (!sd->items) return;

   if (!sd->selected_item)
     {
        if (sd->item_be_selected)
          {
             Elm_Multibuttonentry_Item *it;

             it = eina_list_data_get(eina_list_last(sd->items));
             evas_object_focus_set(elm_layout_edje_get(VIEW(it)), EINA_TRUE);
          }
        else
           sd->item_be_selected = EINA_TRUE;
     }
   else
     {
        elm_object_item_del(sd->selected_item);

        if (sd->editable)
           elm_object_focus_set(sd->entry, EINA_TRUE);
     }
}

static void
_on_layout_clicked(void *data __UNUSED__,
                   Evas_Object *obj,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
}

static Eina_Bool
_elm_multibuttonentry_smart_on_focus(Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->on_focus(obj))
     return EINA_FALSE;

   if (elm_object_focus_get(obj))
     {
        // ACCESS
        //if ((_elm_config->access_mode == ELM_ACCESS_MODE_ON)) return EINA_TRUE;

        if (sd->guide && _guide_packed(obj))
          {
             elm_box_unpack(sd->box, sd->guide);
             evas_object_hide(sd->guide);

             if (sd->editable)
               {
                  elm_box_pack_end(sd->box, sd->entry);
                  evas_object_show(sd->entry);
               }
          }
        // when object gets focused, it should be expanded layout
        else if (!sd->expanded)
           _layout_expand(obj);

        if (sd->editable)
          {
             elm_layout_signal_emit(obj, "elm,state,event,allow", "elm");
             elm_object_focus_set(sd->entry, EINA_TRUE);
          }

        evas_object_smart_callback_call(obj, SIG_FOCUSED, NULL);
     }
   else
     {
        if (!sd->items && sd->guide)
          {
             if (sd->editable)
               {
                  elm_box_unpack(sd->box, sd->entry);
                  evas_object_hide(sd->entry);
               }

             elm_box_pack_end(sd->box, sd->guide);
             evas_object_show(sd->guide);
          }
        // if shrinked mode was set, it goes back to shrinked layout 
        else if (!sd->expanded)
           _layout_shrink(obj, EINA_FALSE);

        if (sd->editable)
           elm_layout_signal_emit(obj, "elm,state,event,block", "elm");


        if (sd->selected_item)
           evas_object_focus_set
              (elm_layout_edje_get(VIEW(sd->selected_item)), EINA_FALSE);

        evas_object_smart_callback_call(obj, SIG_UNFOCUSED, NULL);
     }

   return EINA_TRUE;
}

static Eina_Bool
_elm_multibuttonentry_smart_text_set(Evas_Object *obj,
                                     const char *part,
                                     const char *text)
{
   if (!part || (part && !strcmp(part, "label")))
     {
        if (text) _label_set(obj, text);
        return EINA_TRUE;
     }
   else if (part && !strcmp(part, "guide"))
     {
        if (text) _guide_set(obj, text);
        return EINA_TRUE;
     }
   else return _elm_multibuttonentry_parent_sc->text_set(obj, part, text);
}

static const char *
_elm_multibuttonentry_smart_text_get(const Evas_Object *obj,
                                     const char *part)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!part || !strcmp(part, "label"))
      return elm_object_text_get(sd->label);
   else if (!strcmp(part, "guide"))
      return elm_object_text_get(sd->guide);
   else return _elm_multibuttonentry_parent_sc->text_get(obj, part);
}

static Eina_Bool
_elm_multibuttonentry_smart_theme(Evas_Object *obj)
{
   const char *str;
   int hpad = 0, vpad = 0;
   Eina_List *l;
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->theme(obj))
     return EINA_FALSE;

   str = elm_layout_data_get(obj, "horizontal_pad");
   if (str) hpad = atoi(str);
   str = elm_layout_data_get(obj, "vertical_pad");
   if (str) vpad = atoi(str);
   elm_box_padding_set
      (sd->box,
       hpad * elm_widget_scale_get(obj) * elm_config_scale_get(),
       vpad * elm_widget_scale_get(obj) * elm_config_scale_get());

   elm_layout_theme_set
      (sd->label, "multibuttonentry", "label", elm_widget_style_get(obj));
   elm_layout_theme_set
      (sd->guide, "multibuttonentry", "guide", elm_widget_style_get(obj));
   elm_layout_theme_set
      (sd->number, "multibuttonentry", "number", elm_widget_style_get(obj));
   EINA_LIST_FOREACH(sd->items, l, it)
      elm_layout_theme_set
      (VIEW(it), "multibuttonentry", "button", elm_widget_style_get(obj));

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_elm_multibuttonentry_smart_add(Evas_Object *obj)
{
   const char *str;
   int hpad = 0, vpad = 0;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Multibuttonentry_Smart_Data);

   ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->base.add(obj);
   elm_layout_theme_set
     (obj, "multibuttonentry", "base", elm_widget_style_get(obj));
   elm_layout_signal_callback_add
      (obj, "elm,action,clicked", "", _on_layout_clicked, NULL);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_KEY_UP, _on_layout_key_up, NULL);

   priv->box = elm_box_add(obj);
   str = elm_layout_data_get(obj, "horizontal_pad");
   if (str) hpad = atoi(str);
   str = elm_layout_data_get(obj, "vertical_pad");
   if (str) vpad = atoi(str);
   elm_box_padding_set
      (priv->box,
       hpad * elm_widget_scale_get(obj) * elm_config_scale_get(),
       vpad * elm_widget_scale_get(obj) * elm_config_scale_get());
   elm_box_layout_set(priv->box, _box_layout, obj, NULL);
   elm_layout_content_set(obj, "elm.swallow.box", priv->box);
   evas_object_event_callback_add
      (priv->box, EVAS_CALLBACK_RESIZE, _on_box_resize, obj);

   priv->entry = elm_entry_add(obj);
   elm_entry_single_line_set(priv->entry, EINA_TRUE);
   elm_entry_cnp_mode_set(priv->entry, ELM_CNP_MODE_PLAINTEXT);
   evas_object_size_hint_weight_set
      (priv->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
      (priv->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add
      (priv->entry, EVAS_CALLBACK_KEY_UP, _on_entry_key_up, obj);
   evas_object_smart_callback_add
      (priv->entry, "unfocused", _on_entry_unfocused, obj);
   elm_entry_markup_filter_append(priv->entry, _entry_filter, obj);
   elm_box_pack_end(priv->box, priv->entry);
   evas_object_show(priv->entry);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->editable = EINA_TRUE;
   priv->expanded = EINA_TRUE;
   priv->expanded_state = EINA_TRUE;

#if 0
   // ACCESS
   _elm_access_object_register(obj, ELM_WIDGET_DATA(priv)->resize_obj);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("multi button entry"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
#endif
}

static void
_elm_multibuttonentry_smart_del(Evas_Object *obj)
{
   Elm_Multibuttonentry_Item *it;
   Elm_Multibuttonentry_Item_Filter *ft;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   EINA_LIST_FREE(sd->items, it)
      elm_widget_item_free(it);

   EINA_LIST_FREE(sd->filters, ft)
      free(ft);

   if (sd->number) evas_object_del(sd->number);
   if (sd->guide) evas_object_del(sd->guide);
   if (sd->label) evas_object_del(sd->label);
   if (sd->entry) evas_object_del(sd->entry);
   if (sd->box) evas_object_del(sd->box);

   ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->base.del(obj);
}

static void
_elm_multibuttonentry_smart_set_user(Elm_Multibuttonentry_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_multibuttonentry_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_multibuttonentry_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_multibuttonentry_smart_theme;
   ELM_WIDGET_CLASS(sc)->on_focus = _elm_multibuttonentry_smart_on_focus;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->text_set = _elm_multibuttonentry_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_multibuttonentry_smart_text_get;

   // ACCESS
//   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
//     ELM_WIDGET_CLASS(sc)->focus_next = _elm_multibuttonentry_smart_focus_next;

//   ELM_WIDGET_CLASS(sc)->access = _elm_multibuttonentry_smart_access;
}

EAPI const Elm_Multibuttonentry_Smart_Class *
elm_multibuttonentry_smart_class_get(void)
{
   static Elm_Multibuttonentry_Smart_Class _sc =
      ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NAME_VERSION
      (ELM_MULTIBUTTONENTRY_SMART_NAME);
   static const Elm_Multibuttonentry_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_multibuttonentry_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_multibuttonentry_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_multibuttonentry_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
      ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Evas_Object *
elm_multibuttonentry_entry_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->entry;
}

EAPI void
elm_multibuttonentry_expanded_set(Evas_Object *obj,
                                  Eina_Bool expanded)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   expanded = !!expanded;
   if (sd->expanded == expanded) return;
   sd->expanded = expanded;

   if (elm_object_focus_get(obj)) return;

   if (sd->expanded)
      _layout_expand(obj);
   else
      _layout_shrink(obj, EINA_FALSE);
}

EAPI Eina_Bool
elm_multibuttonentry_expanded_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) EINA_FALSE;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return (elm_object_focus_get(obj) || sd->expanded);
}

EAPI void
elm_multibuttonentry_editable_set(Evas_Object *obj,
                                  Eina_Bool editable)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   editable = !!editable;
   if (sd->editable == editable) return;
   sd->editable = editable;

   if (sd->editable)
     {
        if (!(sd->guide && _guide_packed(obj)))
          {
             elm_box_pack_end(sd->box, sd->entry);
             evas_object_show(sd->entry);
          }

        if (!elm_object_focus_get(obj))
           elm_layout_signal_emit(obj, "elm,state,event,block", "elm");
     }
   else
     {
        if (!(sd->guide && _guide_packed(obj)))
          {
             elm_box_unpack(sd->box, sd->entry);
             evas_object_hide(sd->entry);
          }

        if (!elm_object_focus_get(obj))
           elm_layout_signal_emit(obj, "elm,state,event,allow", "elm");
     }
}

EAPI Eina_Bool
elm_multibuttonentry_editable_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) EINA_FALSE;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->editable;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_prepend(Evas_Object *obj,
                                  const char *label,
                                  Evas_Smart_Cb func,
                                  const void *data)
{
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!label) return NULL;

   // if guide text was shown, hide it
   if (sd->guide && _guide_packed(obj))
     {
        elm_box_unpack(sd->box, sd->guide);
        evas_object_hide(sd->guide);

        if (sd->editable)
          {
             elm_box_pack_end(sd->box, sd->entry);
             evas_object_show(sd->entry);
          }
     }

   it = _item_new(obj, label, func, data);
   if (!it) return NULL;

   sd->items = eina_list_prepend(sd->items, it);

   if (!elm_object_focus_get(obj) && !sd->expanded)
      _layout_shrink(obj, EINA_TRUE);
   else
     {
        if (sd->label && _label_packed(obj))
           elm_box_pack_after(sd->box, VIEW(it), sd->label);
        else
           elm_box_pack_start(sd->box, VIEW(it));
        evas_object_show(VIEW(it));
     }

   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_append(Evas_Object *obj,
                                 const char *label,
                                 Evas_Smart_Cb func,
                                 const void *data)
{
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!label) return NULL;

   // if guide text was shown, hide it
   if (sd->guide && _guide_packed(obj))
     {
        elm_box_unpack(sd->box, sd->guide);
        evas_object_hide(sd->guide);

        if (sd->editable)
          {
             elm_box_pack_end(sd->box, sd->entry);
             evas_object_show(sd->entry);
          }
     }

   it = _item_new(obj, label, func, data);
   if (!it) return NULL;

   sd->items = eina_list_append(sd->items, it);

   if (!elm_object_focus_get(obj) && !sd->expanded)
      _layout_shrink(obj, EINA_TRUE);
   else
     {
        if (sd->editable)
           elm_box_pack_before(sd->box, VIEW(it), sd->entry);
        else
           elm_box_pack_end(sd->box, VIEW(it));
        evas_object_show(VIEW(it));
     }

   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_insert_before(Evas_Object *obj,
                                        Elm_Object_Item *before,
                                        const char *label,
                                        Evas_Smart_Cb func,
                                        const void *data)
{
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!label) return NULL;
   if (!before) return NULL;

   it = _item_new(obj, label, func, data);
   if (!it) return NULL;

   sd->items = eina_list_prepend_relative(sd->items, it, before);

   if (!elm_object_focus_get(obj) && !sd->expanded)
      _layout_shrink(obj, EINA_TRUE);
   else
     {
        elm_box_pack_before(sd->box, VIEW(it), VIEW(before));
        evas_object_show(VIEW(it));
     }

   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_insert_after(Evas_Object *obj,
                                       Elm_Object_Item *after,
                                       const char *label,
                                       Evas_Smart_Cb func,
                                       const void *data)
{
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!label) return NULL;
   if (!after) return NULL;

   it = _item_new(obj, label, func, data);
   if (!it) return NULL;

   sd->items = eina_list_append_relative(sd->items, it, after);

   if (!elm_object_focus_get(obj) && !sd->expanded)
      _layout_shrink(obj, EINA_TRUE);
   else
     {
        elm_box_pack_after(sd->box, VIEW(it), VIEW(after));
        evas_object_show(VIEW(it));
     }

   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);

   return (Elm_Object_Item *)it;
}

EAPI const Eina_List *
elm_multibuttonentry_items_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->items;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_first_item_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return eina_list_data_get(sd->items);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_last_item_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return eina_list_data_get(eina_list_last(sd->items));
}

EAPI Elm_Object_Item *
elm_multibuttonentry_selected_item_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->selected_item;
}

EAPI void
elm_multibuttonentry_item_selected_set(Elm_Object_Item *it,
                                       Eina_Bool selected)
{
   ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it);
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   if (!elm_object_focus_get(WIDGET(it))) return;

   if (selected && it != sd->selected_item)
     {
        evas_object_focus_set(elm_layout_edje_get(VIEW(it)), EINA_TRUE);
     }
   else if (!selected && it == sd->selected_item)
     {
        evas_object_focus_set(elm_layout_edje_get(VIEW(it)), EINA_FALSE);

        if (sd->editable)
           elm_object_focus_set(sd->entry, EINA_TRUE);
     }
}

EAPI Eina_Bool
elm_multibuttonentry_item_selected_get(const Elm_Object_Item *it)
{
   ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   if (!elm_object_focus_get(WIDGET(it))) return EINA_FALSE;

   if (it == sd->selected_item)
      return EINA_TRUE;

   return EINA_FALSE;
}

EAPI void
elm_multibuttonentry_clear(Evas_Object *obj)
{
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_CHECK(obj);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   EINA_LIST_FREE(sd->items, it)
      elm_widget_item_free(it);

   sd->items = NULL;
   sd->item_be_selected = EINA_FALSE;
   sd->expanded_state = EINA_TRUE;

   if (sd->number)
     {
        evas_object_del(sd->number);
        sd->number = NULL;
     }

   if (!sd->items && !elm_object_focus_get(obj) &&
       sd->guide && !_guide_packed(obj))
     {
        if (sd->editable)
          {
             elm_box_unpack(sd->box, sd->entry);
             evas_object_hide(sd->entry);
          }

        elm_box_pack_end(sd->box, sd->guide);
        evas_object_show(sd->guide);
     }
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_prev_get(const Elm_Object_Item *it)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   EINA_LIST_FOREACH(sd->items, l, item)
     {
        if (item == (Elm_Multibuttonentry_Item *)it)
          {
             l = eina_list_prev(l);
             if (!l) return NULL;
             return eina_list_data_get(l);
          }
     }
   return NULL;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_next_get(const Elm_Object_Item *it)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   EINA_LIST_FOREACH(sd->items, l, item)
     {
        if (item == (Elm_Multibuttonentry_Item *)it)
          {
             l = eina_list_next(l);
             if (!l) return NULL;
             return eina_list_data_get(l);
          }
     }
   return NULL;
}

EAPI void
elm_multibuttonentry_item_filter_append(Evas_Object *obj,
                                        Elm_Multibuttonentry_Item_Filter_Cb func,
                                        const void *data)
{
   Elm_Multibuttonentry_Item_Filter *ft = NULL;

   ELM_MULTIBUTTONENTRY_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(func);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   ft = _filter_new(func, data);
   if (!ft) return;

   sd->filters = eina_list_append(sd->filters, ft);
}

EAPI void
elm_multibuttonentry_item_filter_prepend(Evas_Object *obj,
                                         Elm_Multibuttonentry_Item_Filter_Cb func,
                                         const void *data)
{
   Elm_Multibuttonentry_Item_Filter *ft = NULL;

   ELM_MULTIBUTTONENTRY_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(func);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   ft = _filter_new(func, data);
   if (!ft) return;

   sd->filters = eina_list_prepend(sd->filters, ft);
}

EAPI void
elm_multibuttonentry_item_filter_remove(Evas_Object *obj,
                                        Elm_Multibuttonentry_Item_Filter_Cb func,
                                        const void *data)
{
   Elm_Multibuttonentry_Item_Filter *ft;
   Eina_List *l;

   ELM_MULTIBUTTONENTRY_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(func);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->filters, l, ft)
     {
        if ((ft->func == func) && (!data || ft->data == data))
          {
             sd->filters = eina_list_remove_list(sd->filters, l);
             free(ft);
             return;
          }
     }
}
