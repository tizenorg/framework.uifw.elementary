#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_multibuttonentry.h"

//#define _VI_EFFECT 0
//#define _BOX_VI
//#define _TB_BUG

#ifdef _VI_EFFECT
#define TRANSIT_DURATION 0.167
#define ANIMATE_FRAME 10
#endif

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
static const char SIG_LONGPRESSED[] = "longpressed";

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
   {SIG_LONGPRESSED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_MULTIBUTTONENTRY_SMART_NAME, _elm_multibuttonentry,
  Elm_Multibuttonentry_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, _smart_callbacks);

static Eina_Bool
_elm_multibuttonentry_smart_translate(Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);
   Elm_Multibuttonentry_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_widget_item_translate(it);

   return EINA_TRUE;
}

static Eina_Bool
_elm_multibuttonentry_smart_focus_next(const Evas_Object *obj,
                               Elm_Focus_Direction dir,
                               Evas_Object **next)
{
   Eina_List *items = NULL;
   Eina_List *elist = NULL;
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!sd)
     return EINA_FALSE;

   if (!elm_widget_focus_next_get(sd->box, dir, next))
     {
        elm_widget_focused_object_clear(sd->box);
        elm_widget_focus_next_get(sd->box, dir, next);
     }

   EINA_LIST_FOREACH(sd->items, elist, it)
     items = eina_list_append(items, it->base.access_obj);

   return elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

static char *
_access_label_info_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Evas_Object *mbe = (Evas_Object *)data;
   const char *txt = NULL;

   ELM_MULTIBUTTONENTRY_DATA_GET(mbe, sd);

   if (!mbe) return NULL;

   if (!txt)
     {
        txt = _elm_util_mkup_to_text(elm_object_part_text_get(sd->label, "mbe.label"));
        return strdup(txt);
     }
   else return strdup(txt);

   return NULL;
}

static char *
_access_shrink_info_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Evas_Object *mbe = (Evas_Object *)data;
   const char *txt = NULL;
   Eina_Strbuf *buf = NULL;
   char *str = NULL;

   ELM_MULTIBUTTONENTRY_DATA_GET(mbe, sd);

   if (!mbe) return NULL;

   if (!txt) txt = _elm_util_mkup_to_text(elm_object_text_get(sd->end));
   if (txt)
     {
        buf = eina_strbuf_new();
        eina_strbuf_append(buf, E_("And "));
        eina_strbuf_append(buf, txt);
        eina_strbuf_append(buf, E_(" more"));
        str = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);
        return str;
     }
   return NULL;
}

static Eina_Bool
_entry_packed(Evas_Object *obj)
{
   Eina_List *children;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!sd->entry) return EINA_FALSE;

   children = elm_box_children_get(sd->box);
   if (sd->entry == eina_list_data_get(eina_list_last(children)))
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
           (sd->guide, "multibuttonentry", "guidetext", elm_widget_style_get(obj));
        evas_object_size_hint_weight_set
           (sd->guide, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set
           (sd->guide, EVAS_HINT_FILL, EVAS_HINT_FILL);
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
   elm_object_part_text_set(sd->label, "mbe.label", text);

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

   // ACCESS
   if (_elm_config->access_mode)
     {
        _elm_access_object_register(obj, ELM_WIDGET_DATA(sd)->resize_obj);
        _elm_access_callback_set(_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_label_info_cb, obj);
     }
}

#ifdef _VI_EFFECT
static void
_box_layout_pre_calculate(Evas_Object *obj,
                          Elm_Multibuttonentry_Item *except_it,
                          Evas_Coord *minh_wo_entry,
                          Evas_Coord *minh)
{
   Evas_Coord mnw, mnh, x, w, hpad = 0, vpad = 0;
   Evas_Coord linew = 0, lineh = 0;
   int line_num;
   Eina_List *l;
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   evas_object_geometry_get(sd->box, &x, NULL, &w, NULL);
   elm_box_padding_get(sd->box, &hpad, &vpad);

   line_num = 1;

   if (sd->label && _label_packed(obj))
     {
        evas_object_size_hint_min_get(sd->label, &mnw, &mnh);

        linew = mnw;
        linew += hpad;
        lineh = mnh;
     }

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it == except_it) continue;

        evas_object_size_hint_min_get(VIEW(it), &mnw, &mnh);

        linew += mnw;
        if (lineh < mnh) lineh = mnh;

        if (linew > w * 2 / 3)
          {
             if (linew > w) linew = mnw;
             else linew = 0;
             line_num++;
          }
        if (linew != 0)
          linew += hpad;
     }

   if (minh_wo_entry)
     *minh_wo_entry = lineh * line_num + (line_num - 1) * vpad;

   if (sd->editable)
     {
        // get entry size after text is reset
        elm_object_text_set(sd->entry, "");
        elm_layout_sizing_eval(sd->entry);
        evas_object_size_hint_min_get(sd->entry, &mnw, &mnh);

        linew += mnw;
        if (lineh < mnh) lineh = mnh;

        if (!sd->expanded_state)
          line_num = 1;
        else if (linew > w * 2 / 3)
          line_num++;
     }

   if (minh)
     *minh = lineh * line_num + (line_num - 1) * vpad;
}

static void
_after_rect_expand(Evas_Object *rect,
                   Elm_Multibuttonentry_Item *it)
{
   Eina_List *children;
   Elm_Multibuttonentry_Item *last_item;

   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   evas_object_del(rect);
   evas_object_data_set(VIEW(it), "effect_rect", NULL);

   // if last item is unpacked, add it
   children = elm_box_children_get(sd->box);
   last_item = eina_list_data_get(eina_list_last(sd->items));
   if (VIEW(last_item) != eina_list_data_get(eina_list_last(children)))
     {
        elm_box_pack_end(sd->box, VIEW(last_item));
        evas_object_show(VIEW(last_item));
     }
   eina_list_free(children);

   if (sd->editable)
     {
        elm_box_pack_end(sd->box, sd->entry);
        evas_object_show(sd->entry);
        if (elm_object_focus_get(WIDGET(it)))
          elm_object_focus_set(sd->entry, EINA_TRUE);
     }
}

static void
_on_item_expanding_transit_del(void *data,
                               Elm_Transit *transit __UNUSED__)
{
   Elm_Multibuttonentry_Item *it = data;
   Evas_Object *rect;

   evas_object_data_set(VIEW(it), "transit", NULL);
   evas_object_smart_callback_call(WIDGET(it), SIG_ITEM_ADDED, it);

   rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
   if (!rect) return;
}
#ifdef _BOX_VI
static Eina_Bool
_rect_expanding_animate(void *data)
{
   Evas_Coord w, h, eh;
   Evas_Object *rect;
   Elm_Multibuttonentry_Item *it = data;

   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
   if (!rect) return ECORE_CALLBACK_CANCEL;

   evas_object_geometry_get(sd->entry, NULL, NULL, NULL, &eh);
   evas_object_size_hint_min_get(rect, &w, &h);

   if (h >= eh)
     {
        _after_rect_expand(rect, it);
        return ECORE_CALLBACK_CANCEL;
     }
   else
     evas_object_size_hint_min_set(rect, w, h + eh / ANIMATE_FRAME);

   return ECORE_CALLBACK_RENEW;
}
#endif
static void
_item_adding_effect_add(Evas_Object *obj,
                        Elm_Multibuttonentry_Item *it)
{
   Elm_Transit *trans;
   Evas_Coord minh_wo_entry, minh;
   Eina_Bool floating = EINA_FALSE;;
   Evas_Object *rect = NULL;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   _box_layout_pre_calculate(obj, NULL, &minh_wo_entry, &minh);

   // if box will be expanded, add resizing effect
   if (sd->boxh < minh)
     {
        Evas_Coord bx, by, bw, bh;
#ifdef _BOX_VI
        Ecore_Animator *anim;
#endif
        evas_object_geometry_get(sd->box, &bx, &by, &bw, &bh);

        // if box will be expanded with item and entry
        if (minh_wo_entry == minh)
          {
             Elm_Multibuttonentry_Item *last_it;

             last_it = eina_list_data_get(eina_list_last(sd->items));
             if (it == last_it)
               {
#ifdef _BOX_VI
                  Evas_Coord iw, ih, vpad;

                  evas_object_size_hint_min_get(VIEW(it), &iw, &ih);
                  elm_box_padding_get(sd->box, NULL, &vpad);

                  evas_object_move(VIEW(it), bx, by + bh + vpad);
                  evas_object_resize(VIEW(it), iw, ih);
                  evas_object_show(VIEW(it));

                  floating = EINA_TRUE;
#endif
               }
             else
               {
                  elm_box_unpack(sd->box, VIEW(last_it));
                  evas_object_hide(VIEW(last_it));
               }
          }

        if (sd->editable)
          {
             if (elm_object_focus_get(sd->entry))
               elm_object_focus_set(sd->entry, EINA_FALSE);
             elm_box_unpack(sd->box, sd->entry);
             evas_object_hide(sd->entry);
             if (elm_object_focus_get(WIDGET(it)))
               elm_entry_input_panel_show(sd->entry);
          }

        rect = evas_object_rectangle_add(evas_object_evas_get(obj));
        evas_object_color_set(rect, 0, 0, 0, 0);
#ifdef _BOX_VI
        evas_object_size_hint_min_set(rect, bw, 0);
        evas_object_data_set(VIEW(it), "effect_rect", rect);
        elm_box_pack_end(sd->box, rect);
        evas_object_show(rect);

        anim = ecore_animator_add(_rect_expanding_animate, it);
        evas_object_data_set(rect, "animator", anim);
#else
        Evas_Coord eh;
        evas_object_geometry_get(sd->entry, NULL, NULL, NULL, &eh);

        evas_object_size_hint_min_set(rect, bw, eh);
        evas_object_data_set(VIEW(it), "effect_rect", rect);
#endif
     }

   if (!floating)
     {
        Eina_List *cur;
        cur = eina_list_data_find_list(sd->items, it);
        if (cur == sd->items)
          {
             if (sd->label && _label_packed(obj))
               elm_box_pack_after(sd->box, VIEW(it), sd->label);
             else
               elm_box_pack_start(sd->box, VIEW(it));
          }
        else
          {
             Elm_Multibuttonentry_Item *prev_it;
             prev_it = eina_list_data_get(eina_list_prev(cur));
             elm_box_pack_after(sd->box, VIEW(it), VIEW(prev_it));
          }
        evas_object_show(VIEW(it));
     }
   if (rect) _after_rect_expand(rect, it);

   trans = elm_transit_add();
   elm_transit_object_add(trans, VIEW(it));
   elm_transit_effect_zoom_add(trans, 0.9, 1.0);
   elm_transit_effect_color_add(trans, 0, 0, 0, 0, 255, 255, 255, 255);
   elm_transit_del_cb_set(trans, _on_item_expanding_transit_del, it);
   elm_transit_duration_set(trans, TRANSIT_DURATION);
   evas_object_data_set(VIEW(it), "transit", trans);
   elm_transit_go(trans);
}

static void
_on_item_contracting_transit_del(void *data,
                                 Elm_Transit *transit __UNUSED__)
{
   Evas_Object *rect;
   Elm_Multibuttonentry_Item *it = data;

   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   evas_object_data_set(VIEW(it), "transit", NULL);

   rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
   if (rect)
     {
        evas_object_hide(VIEW(it));
        return;
     }

   // delete item and set focus to entry
   if (sd->editable && elm_object_focus_get(WIDGET(it)))
     elm_object_focus_set(sd->entry, EINA_TRUE);

   elm_object_item_del((Elm_Object_Item *)it);
}

static void
_after_rect_contract(Evas_Object *rect,
                          Elm_Multibuttonentry_Item *it)
{
   Elm_Multibuttonentry_Item *last_item;

   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   evas_object_del(rect);
   evas_object_data_set(VIEW(it), "effect_rect", NULL);

   // if last item is unpacked, add it
   last_item = eina_list_data_get(eina_list_last(sd->items));
   if (it != last_item)
     {
        if (!evas_object_visible_get(VIEW(last_item)))
          {
             elm_box_pack_end(sd->box, VIEW(last_item));
             evas_object_show(VIEW(last_item));
          }
     }

   if (sd->editable)
     {
        elm_box_pack_end(sd->box, sd->entry);
        evas_object_show(sd->entry);
        if (elm_object_focus_get(WIDGET(it)))
          elm_object_focus_set(sd->entry, EINA_TRUE);
     }
}
#ifdef _BOX_VI
static Eina_Bool
_rect_contracting_animate(void *data)
{
   Evas_Coord w, h, eh;
   Evas_Object *rect;
   Elm_Multibuttonentry_Item *it = data;

   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
   if (!rect) return ECORE_CALLBACK_CANCEL;

   evas_object_geometry_get(sd->entry, NULL, NULL, NULL, &eh);
   evas_object_size_hint_min_get(rect, &w, &h);

   if (h <= 0)
     {
        Elm_Transit *trans;
        _after_rect_contract(rect, it);

        // delete the button
        trans = (Elm_Transit *)evas_object_data_get(VIEW(it), "transit");
        if (!trans)
          _on_item_contracting_transit_del(it, NULL);

        return ECORE_CALLBACK_CANCEL;
     }
   else
     evas_object_size_hint_min_set(rect, w, h - eh / ANIMATE_FRAME);

   return ECORE_CALLBACK_RENEW;
}
#endif
static void
_item_deleting_effect_add(Evas_Object *obj,
                          Elm_Multibuttonentry_Item *it)
{
   Elm_Transit *trans;
   Evas_Coord minh;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   _box_layout_pre_calculate(obj, it, NULL, &minh);

   // if box will be contracted, add resizing effect
   if (sd->boxh > minh)
     {
        Evas_Coord bx, bw, ix;
#ifdef _BOX_VI
        Evas_Coord eh;
        Ecore_Animator *anim;
#endif
        Evas_Object *rect;
        Elm_Multibuttonentry_Item *last_it;

        last_it = eina_list_data_get(eina_list_last(sd->items));
        evas_object_geometry_get(sd->box, &bx, NULL, &bw, NULL);
        evas_object_geometry_get(VIEW(last_it), &ix, NULL, NULL, NULL);

        // if box will be contracted with item and entry
        if (ix == bx)
          {
             if (it == last_it)
               elm_box_unpack(sd->box, VIEW(it));
             else
               {
                  elm_box_unpack(sd->box, VIEW(last_it));
                  evas_object_hide(VIEW(last_it));
               }
          }

        if (sd->editable)
          {
             if (elm_object_focus_get(sd->entry))
               elm_object_focus_set(sd->entry, EINA_FALSE);
             elm_box_unpack(sd->box, sd->entry);
             evas_object_hide(sd->entry);
             if (elm_object_focus_get(WIDGET(it)))
               elm_entry_input_panel_show(sd->entry);
          }

        rect = evas_object_rectangle_add(evas_object_evas_get(obj));
        evas_object_color_set(rect, 0, 0, 0, 0);
#ifdef _BOX_VI
        evas_object_geometry_get(sd->entry, NULL, NULL, NULL, &eh);
        evas_object_size_hint_min_set(rect, bw, eh);
        evas_object_data_set(VIEW(it), "effect_rect", rect);
        elm_box_pack_end(sd->box, rect);
        evas_object_show(rect);

        anim = ecore_animator_add(_rect_contracting_animate, it);
        evas_object_data_set(rect, "animator", anim);
#else
        evas_object_size_hint_min_set(rect, bw, 0);
        evas_object_data_set(VIEW(it), "effect_rect", rect);
        elm_box_pack_end(sd->box, rect);
        evas_object_show(rect);

        _after_rect_contract(rect, it);
#endif
     }

   trans = elm_transit_add();
   elm_transit_object_add(trans, VIEW(it));
   elm_transit_effect_zoom_add(trans, 1.0, 0.9);
   elm_transit_effect_color_add(trans, 255, 255, 255, 255, 0, 0, 0, 0);
   elm_transit_del_cb_set(trans, _on_item_contracting_transit_del, it);
   elm_transit_duration_set(trans, TRANSIT_DURATION);
   evas_object_data_set(VIEW(it), "transit", trans);
   elm_transit_go(trans);

}
#endif

static void
_layout_expand(Evas_Object *obj)
{
   int count, items_count, i;
   Eina_List *children;
   Elm_Multibuttonentry_Item *it, *it1;
   Evas_Coord itemw = 0, mnh = 0;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (sd->expanded_state) return;
   else if (!sd->items)
     {
        sd->expanded_state = EINA_TRUE;
        return;
     }

   children = elm_box_children_get(sd->box);
   count = eina_list_count(children);
   if (sd->end)
     {
        evas_object_del(sd->end);
        sd->end = NULL;
        count--;
     }
   if (sd->label && _label_packed(obj)) count--;
   eina_list_free(children);

   // to check whether 1st item is shrinked (ex. multibutton@entry... +N)
   it1 = eina_list_nth(sd->items, 0);

   itemw = (Evas_Coord)evas_object_data_get(VIEW(it1), "shrinked_item");
   if (itemw)
     {
        evas_object_data_set(VIEW(it1), "shrinked_item", NULL);
        evas_object_size_hint_min_get(VIEW(it1), NULL, &mnh);

        elm_layout_signal_emit(VIEW(it1), "elm,state,text,ellipsis", "");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(it1)));
        evas_object_size_hint_min_set(VIEW(it1), itemw, mnh);
#ifdef _TB_BUG
        elm_layout_sizing_eval(VIEW(it1));
#else
        evas_object_resize(VIEW(it1), itemw, mnh);
#endif
     }

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
   Evas_Coord w, mnw, mnh, linew = 0, hpad = 0, labelw, endw;
   int count = 0, items_count, i;
   Eina_List *l, *children;
   Evas_Object *child;
   Elm_Multibuttonentry_Item *it;
#ifdef _VI_EFFECT
   Elm_Transit *trans;
   Evas_Object *rect;
   Ecore_Animator *anim;
#endif

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
        evas_object_size_hint_min_get(VIEW(it), &mnw, &mnh);
        linew += mnw;

        if (linew > w)
          {
             linew -= mnw;
             if (count)
               break;
             else if (sd->label && _label_packed(obj))
               {
                  // if 1st item is removed, next item should be recalculated
                  evas_object_size_hint_min_get(sd->label, &labelw, NULL);
                  if (sd->end)
                    {
                       evas_object_size_hint_min_get(sd->end, &endw, NULL);
                       if (mnw > w - (labelw + hpad + endw + hpad))
                         mnw = w - (labelw + hpad + endw + hpad);
                    }
                  else if (mnw > w - (labelw + hpad))
                    mnw = w - (labelw + hpad);

                  elm_layout_signal_emit(VIEW(it), "elm,state,text,ellipsis", "");
                  edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));
                  evas_object_size_hint_min_set(VIEW(it), mnw, mnh);
#ifdef _TB_BUG
                  elm_layout_sizing_eval(VIEW(it));
#else
                  evas_object_resize(VIEW(it), mnw, mnh);
#endif
               }
          }

        count++;
        linew += hpad;
     }

   if (!count) return;

   items_count = eina_list_count(sd->items);
   if (count < items_count)
     {
        const char *str = NULL;
        char buf[16];

        str = elm_layout_data_get(obj, "closed_button_type");
        if (!sd->end)
          {
             sd->end = elm_layout_add(obj);
             if (str && !strcmp(str, "image"))
               elm_layout_theme_set(sd->end, "multibuttonentry",
                                    "closedbutton", elm_widget_style_get(obj));
             else
               elm_layout_theme_set(sd->end, "multibuttonentry",
                                    "number", elm_widget_style_get(obj));
          }
        if (!str || strcmp(str, "image"))
          {
             snprintf(buf, sizeof(buf), "+%d", items_count - count);
             elm_object_text_set(sd->end, buf);

             // ACCESS
             if (_elm_config->access_mode && sd->end)
               {
                  _elm_access_object_register(obj, ELM_WIDGET_DATA(sd)->resize_obj);
                  _elm_access_callback_set(_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_shrink_info_cb, obj);
               }
          }
        evas_object_smart_calculate(sd->end);
        evas_object_size_hint_min_get(sd->end, &mnw, NULL);

        Elm_Multibuttonentry_Item *item;
        Evas_Coord itemw = 0;

        item = eina_list_nth(sd->items, 0);
        itemw = (Evas_Coord)evas_object_data_get(VIEW(item), "maximum_width");
        if (itemw)
          evas_object_size_hint_min_get(VIEW(item), NULL, &mnh);
        else
          evas_object_size_hint_min_get(VIEW(item), &itemw, &mnh);

        linew -= itemw;
        if (sd->label && _label_packed(obj))
          {
             evas_object_size_hint_min_get(sd->label, &labelw, NULL);
             if (itemw > w - (labelw + hpad + mnw + hpad))
               {
                  evas_object_data_set(VIEW(item), "shrinked_item", itemw);
                  itemw = w - (labelw + hpad + mnw + hpad);
               }
          }
        else if (itemw > w - (mnw + hpad))
          {
             evas_object_data_set(VIEW(item), "shrinked_item", itemw);
             itemw = w - (mnw + hpad);
          }
        linew += itemw;
        elm_layout_signal_emit(VIEW(item), "elm,state,text,ellipsis", "");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(item)));
        evas_object_size_hint_min_set(VIEW(item), itemw, mnh);
#ifdef _TB_BUG
        elm_layout_sizing_eval(VIEW(item));
#else
        evas_object_resize(VIEW(item), itemw, mnh);
#endif

        evas_object_size_hint_min_get(sd->end, &mnw, NULL);
        linew += mnw;

        if (linew > w)
          {
             count--;
             if (!str || strcmp(str, "image"))
               {
                  snprintf(buf, sizeof(buf), "+%d", items_count - count);
                  elm_object_text_set(sd->end, buf);

                  // ACCESS
                  if (_elm_config->access_mode && sd->end)
                   {
                      _elm_access_object_register(obj, ELM_WIDGET_DATA(sd)->resize_obj);
                      _elm_access_callback_set(_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_shrink_info_cb, obj);
                   }
               }
          }

        if (!force)
          {
#ifdef _VI_EFFECT
             // reset last inserted item's effect_rect
             it = eina_list_nth(sd->items, count - 1);
             rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
             if (rect)
               {
                  anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
                  if (anim) ecore_animator_del(anim);

                  evas_object_del(rect);
                  evas_object_data_set(VIEW(it), "effect_rect", NULL);
               }
#endif

             for (i = count; i < items_count; i++)
               {
                  it = eina_list_nth(sd->items, i);
#ifdef _VI_EFFECT
                  // reset all effects
                  trans = (Elm_Transit *)evas_object_data_get(VIEW(it), "transit");
                  if (trans) elm_transit_del(trans);

                  rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
                  if (rect)
                    {
                       anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
                       if (anim) ecore_animator_del(anim);

                       evas_object_del(rect);
                       evas_object_data_set(VIEW(it), "effect_rect", NULL);
                    }
#endif
                  elm_box_unpack(sd->box, VIEW(it));
                  evas_object_hide(VIEW(it));
               }

             if (sd->editable)
               {
                  if (elm_object_focus_get(sd->entry))
                    elm_object_focus_set(sd->entry, EINA_FALSE);
                  elm_box_unpack(sd->box, sd->entry);
                  evas_object_hide(sd->entry);
               }
          }
        else
          {
             // if it is called from item_append_xxx, item_del functions,
             // all items are unpacked and packed again
#ifdef _VI_EFFECT
             // reset last inserted item's effect_rect
             it = eina_list_nth(sd->items, count - 1);
             rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
             if (rect)
               {
                  anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
                  if (anim) ecore_animator_del(anim);

                  evas_object_del(rect);
                  evas_object_data_set(VIEW(it), "effect_rect", NULL);
               }

             for (i = count; i < items_count; i++)
               {
                  it = eina_list_nth(sd->items, i);
                  // reset all effects
                  trans = (Elm_Transit *)evas_object_data_get(VIEW(it), "transit");
                  if (trans) elm_transit_del(trans);

                  rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
                  if (rect)
                    {
                       anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
                       if (anim) ecore_animator_del(anim);

                       evas_object_del(rect);
                       evas_object_data_set(VIEW(it), "effect_rect", NULL);
                    }
               }
#endif

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

        elm_box_pack_end(sd->box, sd->end);
        evas_object_show(sd->end);

        sd->expanded_state = EINA_FALSE;
     }
   else
     {
        if (!force)
          {
#ifdef _VI_EFFECT
             // reset last inserted item's effect_rect
             it = eina_list_nth(sd->items, count - 1);
             rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
             if (rect)
               {
                  anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
                  if (anim) ecore_animator_del(anim);

                  evas_object_del(rect);
                  evas_object_data_set(VIEW(it), "effect_rect", NULL);

                  if (sd->editable)
                    {
                       elm_box_pack_end(sd->box, sd->entry);
                       evas_object_show(sd->entry);
                       if (elm_object_focus_get(WIDGET(it)))
                         elm_object_focus_set(sd->entry, EINA_TRUE);
                    }
               }
#endif
             if (sd->editable)
               {
                  evas_object_size_hint_min_get(sd->entry, &mnw, NULL);
                  linew += mnw;
                  if (linew > (w * (2 / 3)))
                    {
                       if (elm_object_focus_get(sd->entry))
                         elm_object_focus_set(sd->entry, EINA_FALSE);
                       elm_box_unpack(sd->box, sd->entry);
                       evas_object_hide(sd->entry);
                       sd->expanded_state = EINA_FALSE;
                    }
               }
          }
        else
          {
             if (sd->end)
               {
                  evas_object_del(sd->end);
                  sd->end = NULL;
               }

             // if it is called from item_append_xxx, item_del functions,
             // all items are unpacked and packed again
#ifdef _VI_EFFECT
             // reset last inserted item's effect_rect
             it = eina_list_nth(sd->items, count - 1);
             rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
             if (rect)
               {
                  anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
                  if (anim) ecore_animator_del(anim);

                  evas_object_del(rect);
                  evas_object_data_set(VIEW(it), "effect_rect", NULL);
               }
#endif

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
                  if (linew > (w * (2 / 3)))
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
                        int *line_height,
                        void *data)
{
   Evas_Coord mnw, mnh, w, minw, minh = 0, linew = 0, lineh = 0, labelw = 0, endw = 0, itemw;
   int line_num, cnt = 0;
   Eina_List *l, *l_next;
   Evas_Object_Box_Option *opt;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   evas_object_geometry_get(box, NULL, NULL, &w, NULL);
   evas_object_size_hint_min_get(box, &minw, NULL);
   if (!w) return EINA_FALSE;

   line_num = 1;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        evas_object_size_hint_min_get(opt->obj, &mnw, &mnh);

        if ((cnt == 0) && !sd->label && sd->end)
          {
             evas_object_size_hint_min_get(sd->end, &endw, NULL);
             if (mnw > w - (endw + priv->pad.h))
               mnw = w - (endw + priv->pad.h);
          }
        if ((cnt == 1) && sd->label && _label_packed(data))
          {
             evas_object_size_hint_min_get(sd->label, &labelw, NULL);
             if (sd->end)
               {
                  evas_object_size_hint_min_get(sd->end, &endw, NULL);
                  if (mnw > w - (labelw + priv->pad.h + endw + priv->pad.h))
                    mnw = w - (labelw + priv->pad.h + endw + priv->pad.h);
               }
             else
               {
                  itemw = (Evas_Coord)evas_object_data_get(opt->obj, "shrinked_item");
                  if (itemw)
                    mnw = itemw;
                  else if (mnw > w - (labelw + priv->pad.h))
                    mnw = w - (labelw + priv->pad.h);
               }
          }
#ifdef _VI_EFFECT
        if (mnw == w &&
            !strcmp(evas_object_type_get(opt->obj), "rectangle"))
          break;
#endif

        linew += mnw;
        if (lineh < mnh) lineh = mnh;

        if (linew > w * 2 / 3)
          {
             if (linew > w)
               {
                  linew = mnw;
                  line_num++;
                  if (linew > w * 2 / 3)
                    {
                       l_next = eina_list_next(l);
                       opt = eina_list_data_get(l_next);
                       if (l_next && opt && opt->obj &&
                           !strcmp(elm_widget_type_get(opt->obj), "elm_entry"))
                         {
                            linew = 0;
                            line_num++;
                         }
                    }
               }
             else
               {
                  l_next = eina_list_next(l);
                  opt = eina_list_data_get(l_next);
                  if (l_next && opt && opt->obj &&
                      !strcmp(elm_widget_type_get(opt->obj), "elm_entry"))
                    {
                       linew = 0;
                       line_num++;
                    }
               }
          }
        if ((linew != 0) && (l != eina_list_last(priv->children)))
          linew += priv->pad.h;
        cnt++;
     }

   minh = lineh * line_num + (line_num - 1) * priv->pad.v;

#ifdef _VI_EFFECT
   // if last item is effect_rect
   if (opt && opt->obj && mnw == w &&
       !strcmp(evas_object_type_get(opt->obj), "rectangle"))
     {
        minh += priv->pad.v;
        minh += mnh;
     }
#endif

   evas_object_size_hint_min_set(box, minw, minh);
   *line_height = lineh;

   return EINA_TRUE;
}

static void
_box_layout(Evas_Object *o,
            Evas_Object_Box_Data *priv,
            void *data)
{
   Evas_Coord x, y, w, h, xx, yy, xxx, yyy;
   Evas_Coord minw, minh, linew = 0, lineh = 0, lineww = 0, itemw;
   double ax, ay;
   Eina_Bool rtl;
   Eina_List *l, *l_next;
   Evas_Object *obj;
   Evas_Object_Box_Option *opt;
   int cnt = 0;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   if (!_box_min_size_calculate(o, priv, &lineh, data)) return;

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
        Evas_Coord mnw, mnh, ww, hh, ow, oh, labelw, endw;
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

        if ((cnt == 0) && !sd->label && sd->end)
          {
             evas_object_data_set(obj, "maximum_width", mnw);
             evas_object_size_hint_min_get(sd->end, &endw, NULL);
             if (mnw > w - (endw + priv->pad.h))
               mnw = w - (endw + priv->pad.h);
             elm_layout_signal_emit(obj, "elm,state,text,ellipsis", "");
             edje_object_message_signal_process(elm_layout_edje_get(obj));
          }
        if ((cnt == 1) && sd->label && _label_packed(data))
          {
             evas_object_size_hint_min_get(sd->label, &labelw, NULL);
             if (sd->end)
               {
                  evas_object_size_hint_min_get(sd->end, &endw, NULL);
                  if (mnw > w - (labelw + priv->pad.h + endw + priv->pad.h))
                    mnw = w - (labelw + priv->pad.h + endw + priv->pad.h);
               }
             else
               {
                  evas_object_data_set(obj, "maximum_width", mnw);
                  itemw = (Evas_Coord)evas_object_data_get(obj, "shrinked_item");
                  if (itemw)
                    mnw = itemw;
                  else if (mnw > w - (labelw + priv->pad.h))
                    mnw = w - (labelw + priv->pad.h);
               }
             elm_layout_signal_emit(obj, "elm,state,text,ellipsis", "");
             edje_object_message_signal_process(elm_layout_edje_get(obj));
          }

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
        if (linew > w && l != priv->children)
          {
             xx = x;
             yy += hh;
             yy += priv->pad.v;
             linew = ww;
          }

        evas_object_move(obj,
                         ((!rtl) ? (xx) : (x + (w - (xx - x) - ww)))
                         + (Evas_Coord)(((double)(ww - ow)) * ax),
                         yy + (Evas_Coord)(((double)(hh - oh)) * ay));
        evas_object_resize(obj, ow, oh);

        xx += ww;
        xx += priv->pad.h;
        xxx = xx;
        yyy = yy;
        lineww = linew;

        // if linew is bigger than 2/3 of boxw, entry goes to next line.
        if (linew > w * 2 / 3)
          {
             l_next = eina_list_next(l);
             opt = eina_list_data_get(l_next);
             if (l_next && opt && opt->obj && !strcmp(elm_widget_type_get(opt->obj), "elm_entry"))
               {
                  xx = x;
                  yy += hh;
                  yy += priv->pad.v;
                  linew = 0;
               }
          }

        // whether there is "+%d" at the end of the box or entry is invisible,
        // box doesn't have to be larger.
        if (!sd->expanded_state)
          {
             xx = xxx;
             yy = yyy;
             linew = lineww;
          }
        if ((linew != 0) && (l != eina_list_last(priv->children)))
          linew += priv->pad.h;
        cnt++;
     }
}

static void
_on_box_resize(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Evas_Coord w, h, mnw, mnh, labelw, hpad;
   int items_count = 0, i;
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   elm_box_padding_get(obj, &hpad, NULL);

   if (sd->boxh < h)
     evas_object_smart_callback_call(data, SIG_EXPANDED, NULL);
   else if (sd->boxh > h)
     evas_object_smart_callback_call(data, SIG_CONTRACTED, NULL);

   // on rotation, items should be packed again in the shrinked layout
   if (sd->boxw && sd->boxw != w)
     {
        // recalculate width of all items
        if (sd->items)
          {
             items_count = eina_list_count(sd->items);
             for(i = 0; i < items_count; i++)
               {
                  it = eina_list_nth(sd->items, i);

                  if (evas_object_data_get(VIEW(it), "shrinked_item"))
                    evas_object_data_set(VIEW(it), "shrinked_item", NULL);
#ifdef _TB_BUG
                  elm_layout_sizing_eval(VIEW(it));
#else
                  Evas_Object *textblock = NULL;
                  Evas_Coord tb_w = 0, minh = 0;
                  const char *str_left = NULL, *str_right = NULL;
                  int leftw = 0, rightw = 0;

                  str_left = elm_layout_data_get(VIEW(it), "button_left_pad");
                  str_right = elm_layout_data_get(VIEW(it), "button_right_pad");

                  textblock = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(VIEW(it)), "elm.btn.text");
                  if (textblock)
                    {
                       if (str_left && str_right)
                         {
                            leftw = atoi(str_left);
                            rightw = atoi(str_right);
                         }
                       evas_object_size_hint_min_get(VIEW(it), NULL, &minh);
                       evas_object_textblock_size_native_get(textblock, &tb_w, NULL);
                       tb_w += (Evas_Coord)(leftw + rightw);
                       evas_object_size_hint_min_set(VIEW(it), tb_w, minh);
                    }
#endif
                  evas_object_size_hint_min_get(VIEW(it), &mnw, &mnh);

                  if (i == 0)
                    {
                       if (sd->label && _label_packed(data))
                         {
                            evas_object_size_hint_min_get(sd->label, &labelw, NULL);
                            if (mnw > w - (labelw + hpad))
                              {
                                 mnw = w - (labelw + hpad);
                                 elm_layout_signal_emit(VIEW(it), "elm,state,text,ellipsis", "");
                                 edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));
                                 evas_object_size_hint_min_set(VIEW(it), mnw, mnh);
#ifdef _TB_BUG
                                 elm_layout_sizing_eval(VIEW(it));
#else
                                 evas_object_resize(VIEW(it), mnw, mnh);
#endif
                              }
                         }
                       evas_object_data_set(VIEW(it), "maximum_width", mnw);
                    }
                  if (mnw > w)
                    {
                       mnw = w;
                       elm_layout_signal_emit(VIEW(it), "elm,state,text,ellipsis", "");
                       edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));
                       evas_object_size_hint_min_set(VIEW(it), mnw, mnh);
#ifdef _TB_BUG
                       elm_layout_sizing_eval(VIEW(it));
#else
                       evas_object_resize(VIEW(it), mnw, mnh);
#endif
                    }
               }
          }
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
        elm_layout_signal_emit(VIEW(it), "focused", "");

        if (sd->editable)
          elm_entry_input_panel_show(sd->entry);
     }
   else
     {
        sd->selected_item = NULL;
        elm_layout_signal_emit(VIEW(it), "default", "");

        if (sd->editable)
          elm_entry_input_panel_hide(sd->entry);
     }
}

static Eina_Bool
_long_press_cb(void *data)
{
   Elm_Multibuttonentry_Item *it = data;

   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   sd->longpress_timer = NULL;
   sd->long_pressed = EINA_TRUE;

   evas_object_smart_callback_call(WIDGET(it), SIG_LONGPRESSED, it);

   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(Elm_Multibuttonentry_Item *it,
               Evas *evas __UNUSED__,
               Evas_Object *obj __UNUSED__,
               Evas_Event_Mouse_Down *ev)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   if (ev->button != 1) return;

   sd->long_pressed = EINA_FALSE;

   if (sd->longpress_timer) ecore_timer_del(sd->longpress_timer);
   sd->longpress_timer = ecore_timer_add
       (_elm_config->longpress_timeout, _long_press_cb, it);
}

static void
_mouse_up_cb(Elm_Multibuttonentry_Item *it,
             Evas *evas __UNUSED__,
             Evas_Object *obj __UNUSED__,
             Evas_Event_Mouse_Down *ev __UNUSED__)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   if (sd->longpress_timer)
     {
        ecore_timer_del(sd->longpress_timer);
        sd->longpress_timer = NULL;
     }
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *text)
{
   if (part && strcmp(part, "elm.btn.text")) return;
   if (!text) return;

   elm_object_part_text_set(VIEW(it), "elm.btn.text", text);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "elm.btn.text")) return NULL;

   return elm_object_part_text_get(VIEW(it), "elm.btn.text");
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   sd->items = eina_list_remove(sd->items, it);

   if (!elm_object_focus_get(WIDGET(it)) && !sd->expanded)
     _layout_shrink(WIDGET(it), EINA_TRUE);

#ifdef _VI_EFFECT
   Elm_Transit *trans;
   Evas_Object *rect;

   trans = (Elm_Transit *)evas_object_data_get(VIEW(it), "transit");
   if (trans)
     {
        elm_transit_del_cb_set(trans, NULL, NULL);
        elm_transit_del(trans);
     }

   rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
   if (rect)
     {
        Ecore_Animator *anim;
        anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
        if (anim) ecore_animator_del(anim);

        evas_object_del(rect);
        evas_object_data_set(VIEW(it), "effect_rect", NULL);

        if (sd->editable)
          {
             elm_box_pack_end(sd->box, sd->entry);
             evas_object_show(sd->entry);
             if (elm_object_focus_get(WIDGET(it)))
               elm_object_focus_set(sd->entry, EINA_TRUE);
          }
     }
#endif

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
   else if (!sd->items && !elm_object_focus_get(WIDGET(it)) &&
      sd->entry && !_entry_packed(WIDGET(it)) && sd->editable)
     {
        elm_box_pack_end(sd->box, sd->entry);
        evas_object_show(sd->entry);
     }

   evas_object_smart_callback_call(WIDGET(it), SIG_ITEM_DELETED, it);

   return EINA_TRUE;
}

static char *
_access_info_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Multibuttonentry_Item *it = (Elm_Multibuttonentry_Item *)data;
   const char *txt = NULL;
   Eina_Strbuf *buf = NULL;
   char *str = NULL;

   if (!it) return NULL;

   if (!txt)txt = elm_object_item_text_get((Elm_Object_Item *)it);
   if (txt)
     {
        buf = eina_strbuf_new();
        eina_strbuf_append(buf, E_(" double tap to edit"));
        eina_strbuf_prepend(buf, txt);
        str = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);
        return str;
     }

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Multibuttonentry_Item *it = (Elm_Multibuttonentry_Item *)data;
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
   _on_item_selected(item, NULL, NULL, NULL);
}

static void
_access_widget_item_register(Elm_Multibuttonentry_Item *it, Eina_Bool is_access)
{
   Elm_Access_Info *ai;

   if (!is_access) _elm_access_widget_item_unregister((Elm_Widget_Item *)it);
   else
     {
        _elm_access_widget_item_register((Elm_Widget_Item *)it);

        ai = _elm_access_object_get(it->base.access_obj);

        _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
        _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);
        _elm_access_activate_callback_set(ai, _access_activate_cb, it);
     }
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
   Evas_Coord minw, minh, boxw = 0;

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
      (VIEW(it), "multibuttonentry", "btn", elm_widget_style_get(obj));
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
   evas_object_event_callback_add
      (elm_layout_edje_get(VIEW(it)),
       EVAS_CALLBACK_MOUSE_DOWN, (Evas_Object_Event_Cb)_mouse_down_cb, it);
   evas_object_event_callback_add
      (elm_layout_edje_get(VIEW(it)),
       EVAS_CALLBACK_MOUSE_UP, (Evas_Object_Event_Cb)_mouse_up_cb, it);
   elm_object_part_text_set(VIEW(it), "elm.btn.text", text);

   it->func = func;
   it->base.data = data;

   // adjust item size if item is longer than maximum size
   evas_object_smart_calculate(VIEW(it));
   evas_object_size_hint_min_get(VIEW(it), &minw, &minh);
   evas_object_geometry_get(sd->box, NULL, NULL, &boxw, NULL);

   if (minw > boxw)
     {
        elm_layout_signal_emit(VIEW(it), "elm,state,text,ellipsis", "");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));
        evas_object_size_hint_min_set(VIEW(it), boxw, minh);
#ifdef _TB_BUG
        elm_layout_sizing_eval(VIEW(it));
#else
        evas_object_resize(VIEW(it), boxw, minh);
#endif
     }

   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);

   sd->item_be_selected = EINA_TRUE;

   // ACCESS
   if (_elm_config->access_mode) _access_widget_item_register(it, EINA_TRUE);

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
#ifdef _VI_EFFECT
        _item_adding_effect_add(data, it);
#else
        elm_box_pack_before(sd->box, VIEW(it), obj);
        evas_object_show(VIEW(it));
        evas_object_smart_callback_call(data, SIG_ITEM_ADDED, it);
#endif

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

   // cancels item_be_selected when text inserting is started
   if (strcmp(*text, ";") && strcmp(*text, ","))
     {
        sd->item_be_selected = EINA_FALSE;
        return;
     }

   str = elm_entry_markup_to_utf8(elm_object_text_get(entry));
   if (strlen(str))
     {
        Elm_Multibuttonentry_Item *it;

        it = _item_new(data, str, NULL, NULL);
        if (!it) return;

        sd->items = eina_list_append(sd->items, it);
#ifdef _VI_EFFECT
        _item_adding_effect_add(data, it);
#else
        elm_box_pack_before(sd->box, VIEW(it), entry);
        evas_object_show(VIEW(it));
        evas_object_smart_callback_call(data, SIG_ITEM_ADDED, it);
#endif

        elm_object_text_set(entry, "");
     }
   free(str);

   free(*text);
   *text = NULL;
}

// handles enter key
static void
_on_entry_key_down(void *data,
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
#ifdef _VI_EFFECT
             _item_adding_effect_add(data, it);
#else
             elm_box_pack_before(sd->box, VIEW(it), obj);
             evas_object_show(VIEW(it));
             evas_object_smart_callback_call(data, SIG_ITEM_ADDED, it);
#endif

             elm_object_text_set(obj, "");
          }
        free(str);
     }
}

// handles delete key
// it can be pressed when button is selected, so it is handled on layout_key_down
static void
_on_layout_key_down(void *data __UNUSED__,
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
        if (sd->item_be_selected) // 2nd delete
          {
             Elm_Multibuttonentry_Item *it;

             it = eina_list_data_get(eina_list_last(sd->items));
             evas_object_focus_set(elm_layout_edje_get(VIEW(it)), EINA_TRUE);
          }
        else // 1st delete
          sd->item_be_selected = EINA_TRUE;
     }
   else // 3rd delete
     {
#ifdef _VI_EFFECT
        Elm_Transit *trans;
        Evas_Object *rect;

        trans = (Elm_Transit *)evas_object_data_get(VIEW(sd->selected_item), "transit");
        rect = (Evas_Object *)evas_object_data_get(VIEW(sd->selected_item), "effect_rect");

        if (!trans && !rect)
          _item_deleting_effect_add
             (obj, (Elm_Multibuttonentry_Item *)sd->selected_item);
#else
        elm_object_item_del(sd->selected_item);

        if (sd->editable)
          elm_object_focus_set(sd->entry, EINA_TRUE);
#endif
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
             elm_layout_signal_emit(obj, "elm,state,event,allow", "");
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
          elm_layout_signal_emit(obj, "elm,state,event,block", "");

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
     return elm_object_part_text_get(sd->label, "mbe.label");
   else if (!strcmp(part, "guide"))
     return elm_object_text_get(sd->guide);
   else return _elm_multibuttonentry_parent_sc->text_get(obj, part);
}

static void
_elm_multibuttonentry_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
      (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
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
      (sd->guide, "multibuttonentry", "guidetext", elm_widget_style_get(obj));
   elm_layout_theme_set
      (sd->end, "multibuttonentry", "number", elm_widget_style_get(obj));
   EINA_LIST_FOREACH(sd->items, l, it)
      elm_layout_theme_set
      (VIEW(it), "multibuttonentry", "btn", elm_widget_style_get(obj));

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
      (obj, EVAS_CALLBACK_KEY_DOWN, _on_layout_key_down, NULL);

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
   elm_layout_content_set(obj, "box.swallow", priv->box);
   evas_object_event_callback_add
      (priv->box, EVAS_CALLBACK_RESIZE, _on_box_resize, obj);

   priv->entry = elm_entry_add(obj);
   elm_entry_single_line_set(priv->entry, EINA_TRUE);
   elm_entry_scrollable_set(priv->entry, EINA_TRUE);
   elm_entry_cnp_mode_set(priv->entry, ELM_CNP_MODE_PLAINTEXT);
   evas_object_size_hint_weight_set
      (priv->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
      (priv->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add
      (priv->entry, EVAS_CALLBACK_KEY_DOWN, _on_entry_key_down, obj);
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

#ifdef _VI_EFFECT
   EINA_LIST_FREE(sd->items, it)
     {
        Elm_Transit *trans;
        Evas_Object *rect;
        Evas_Coord maxw, shrinkw;

        trans = (Elm_Transit *)evas_object_data_get(VIEW(it), "transit");
        if (trans)
          {
             elm_transit_del_cb_set(trans, NULL, NULL);
             elm_transit_del(trans);
          }

        rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
        if (rect)
          {
             Ecore_Animator *anim;
             anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
             if (anim) ecore_animator_del(anim);

             evas_object_del(rect);
             evas_object_data_set(VIEW(it), "effect_rect", NULL);
          }

        maxw = (Evas_Coord)evas_object_data_get(VIEW(it), "maximum_width");
        if (maxw)
          evas_object_data_set(VIEW(it), "maximum_width", NULL);

        shrinkw = (Evas_Coord)evas_object_data_get(VIEW(it), "shrinked_item");
        if (shrinkw)
          evas_object_data_set(VIEW(it), "shrinked_item", NULL);
        elm_widget_item_free(it);
     }
#else
   EINA_LIST_FREE(sd->items, it)
     {
        Evas_Coord maxw, shrinkw;

        maxw = (Evas_Coord)evas_object_data_get(VIEW(it), "maximum_width");
        if (maxw)
          evas_object_data_set(VIEW(it), "maximum_width", NULL);

        shrinkw = (Evas_Coord)evas_object_data_get(VIEW(it), "shrinked_item");
        if (shrinkw)
          evas_object_data_set(VIEW(it), "shrinked_item", NULL);

        elm_widget_item_free(it);
     }
#endif
   EINA_LIST_FREE(sd->filters, ft)
      free(ft);

   if (sd->end) evas_object_del(sd->end);
   if (sd->guide) evas_object_del(sd->guide);
   if (sd->label) evas_object_del(sd->label);
   if (sd->entry) evas_object_del(sd->entry);
   if (sd->box) evas_object_del(sd->box);
   if (sd->longpress_timer) ecore_timer_del(sd->longpress_timer);

   ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->base.del(obj);
}

static void
_elm_multibuttonentry_smart_access(Evas_Object *obj, Eina_Bool is_access)
{
   Eina_List *elist = NULL;
   Elm_Multibuttonentry_Item *it;

   ELM_MULTIBUTTONENTRY_CHECK(obj);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->items, elist, it)
     _access_widget_item_register(it, is_access);
}

static void
_elm_multibuttonentry_smart_set_user(Elm_Multibuttonentry_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_multibuttonentry_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_multibuttonentry_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_multibuttonentry_smart_theme;
   ELM_WIDGET_CLASS(sc)->translate = _elm_multibuttonentry_smart_translate;
   ELM_WIDGET_CLASS(sc)->on_focus = _elm_multibuttonentry_smart_on_focus;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->text_set = _elm_multibuttonentry_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_multibuttonentry_smart_text_get;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_multibuttonentry_smart_sizing_eval;

   // ACCESS
   if (_elm_config->access_mode)
      ELM_WIDGET_CLASS(sc)->focus_next = _elm_multibuttonentry_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _elm_multibuttonentry_smart_access;
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
          elm_layout_signal_emit(obj, "elm,state,event,block", "");
     }
   else
     {
        if (!(sd->guide && _guide_packed(obj)))
          {
             elm_box_unpack(sd->box, sd->entry);
             evas_object_hide(sd->entry);
          }

        if (!elm_object_focus_get(obj))
          elm_layout_signal_emit(obj, "elm,state,event,allow", "");
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

   if (!elm_object_focus_get(obj) && !sd->expanded && sd->boxw)
     {
#ifdef _VI_EFFECT
        _item_adding_effect_add(obj, it);
#endif
        _layout_shrink(obj, EINA_TRUE);
     }
   else
     {
#ifdef _VI_EFFECT
        if (sd->boxh && sd->boxw)
          _item_adding_effect_add(obj, it);
        else
          {
             if (sd->label && _label_packed(obj))
               elm_box_pack_after(sd->box, VIEW(it), sd->label);
             else
               elm_box_pack_start(sd->box, VIEW(it));
             evas_object_show(VIEW(it));
          }
#else
        if (sd->label && _label_packed(obj))
          elm_box_pack_after(sd->box, VIEW(it), sd->label);
        else
          elm_box_pack_start(sd->box, VIEW(it));
        evas_object_show(VIEW(it));
#endif
     }

#ifdef _VI_EFFECT
   if (!sd->boxh || !sd->boxw)
     evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);
#else
   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);
#endif

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

   if (!elm_object_focus_get(obj) && !sd->expanded && sd->boxw)
     {
#ifdef _VI_EFFECT
        _item_adding_effect_add(obj, it);
#endif
        _layout_shrink(obj, EINA_TRUE);
     }
   else
     {
#ifdef _VI_EFFECT
        if (sd->boxh && sd->boxw)
          _item_adding_effect_add(obj, it);
        else
          {
             if (sd->editable)
               elm_box_pack_before(sd->box, VIEW(it), sd->entry);
             else
               elm_box_pack_end(sd->box, VIEW(it));
             evas_object_show(VIEW(it));
          }
#else
        if (sd->editable)
          elm_box_pack_before(sd->box, VIEW(it), sd->entry);
        else
          elm_box_pack_end(sd->box, VIEW(it));
        evas_object_show(VIEW(it));
#endif
     }

#ifdef _VI_EFFECT
   if (!sd->boxh || !sd->boxw)
     evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);
#else
   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);
#endif

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

   if (!elm_object_focus_get(obj) && !sd->expanded && sd->boxw)
     {
#ifdef _VI_EFFECT
        _item_adding_effect_add(obj, it);
#endif
        _layout_shrink(obj, EINA_TRUE);
     }
   else
     {
#ifdef _VI_EFFECT
        if (sd->boxh && sd->boxw)
          _item_adding_effect_add(obj, it);
        else
          {
             elm_box_pack_before(sd->box, VIEW(it), VIEW(before));
             evas_object_show(VIEW(it));
          }
#else
        elm_box_pack_before(sd->box, VIEW(it), VIEW(before));
        evas_object_show(VIEW(it));
#endif
     }

#ifdef _VI_EFFECT
   if (!sd->boxh || !sd->boxw)
     evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);
#else
   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);
#endif

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

   if (!elm_object_focus_get(obj) && !sd->expanded && sd->boxw)
     {
#ifdef _VI_EFFECT
        _item_adding_effect_add(obj, it);
#endif
        _layout_shrink(obj, EINA_TRUE);
     }
   else
     {
#ifdef _VI_EFFECT
        if (sd->boxw && sd->boxh)
          _item_adding_effect_add(obj, it);
        else
          {
             elm_box_pack_after(sd->box, VIEW(it), VIEW(after));
             evas_object_show(VIEW(it));
          }
#else
        elm_box_pack_after(sd->box, VIEW(it), VIEW(after));
        evas_object_show(VIEW(it));
#endif
     }

#ifdef _VI_EFFECT
   if (!sd->boxh || !sd->boxw)
     evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);
#else
   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, it);
#endif

   return (Elm_Object_Item *)it;
}

EAPI const Eina_List *
elm_multibuttonentry_items_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->items;
}

EAPI Evas_Object *
elm_multibuttonentry_item_object_get(const Elm_Object_Item *it)
{
   ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it, NULL);

   return VIEW(it);
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

#ifdef _VI_EFFECT
   EINA_LIST_FREE(sd->items, it)
     {
        Elm_Transit *trans;
        Evas_Object *rect;
        Evas_Coord maxw, shrinkw;

        trans = (Elm_Transit *)evas_object_data_get(VIEW(it), "transit");
        if (trans)
          {
             elm_transit_del_cb_set(trans, NULL, NULL);
             elm_transit_del(trans);
          }

        rect = (Evas_Object *)evas_object_data_get(VIEW(it), "effect_rect");
        if (rect)
          {
             Ecore_Animator *anim;

             anim = (Ecore_Animator*)evas_object_data_get(rect, "animator");
             if (anim) ecore_animator_del(anim);

             evas_object_del(rect);
             evas_object_data_set(VIEW(it), "effect_rect", NULL);

             if (sd->editable)
               {
                  elm_box_pack_end(sd->box, sd->entry);
                  evas_object_show(sd->entry);
                  if (elm_object_focus_get(WIDGET(it)))
                    elm_object_focus_set(sd->entry, EINA_TRUE);
               }
          }

        maxw = (Evas_Coord)evas_object_data_get(VIEW(it), "maximum_width");
        if (maxw)
          evas_object_data_set(VIEW(it), "maximum_width", NULL);

        shrinkw = (Evas_Coord)evas_object_data_get(VIEW(it), "shrinked_item");
        if (shrinkw)
          evas_object_data_set(VIEW(it), "shrinked_item", NULL);
        elm_widget_item_free(it);
     }
#else
   EINA_LIST_FREE(sd->items, it)
     {
        Evas_Coord maxw, shrinkw;

        maxw = (Evas_Coord)evas_object_data_get(VIEW(it), "maximum_width");
        if (maxw)
          evas_object_data_set(VIEW(it), "maximum_width", NULL);

        shrinkw = (Evas_Coord)evas_object_data_get(VIEW(it), "shrinked_item");
        if (shrinkw)
          evas_object_data_set(VIEW(it), "shrinked_item", NULL);

        elm_widget_item_free(it);
     }
#endif
   sd->items = NULL;
   sd->item_be_selected = EINA_FALSE;
   sd->expanded_state = EINA_TRUE;

   if (sd->end)
     {
        evas_object_del(sd->end);
        sd->end = NULL;
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
