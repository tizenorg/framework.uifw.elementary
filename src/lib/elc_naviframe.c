#include <Ecore_X.h>               //Tizen Only
#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_naviframe.h"

#define KEY_END "XF86Stop"         //Tizen Only

EAPI const char ELM_NAVIFRAME_SMART_NAME[] = "elm_naviframe";

static const char CONTENT_PART[] = "elm.swallow.content";
static const char PREV_BTN_PART[] = "elm.swallow.prev_btn";
static const char NEXT_BTN_PART[] = "elm.swallow.next_btn";
static const char MORE_BTN_PART[] = "toolbar_more_btn"; //Tizen Only
static const char ICON_PART[] = "elm.swallow.icon";
static const char TITLE_PART[] = "elm.text.title";
static const char SUBTITLE_PART[] = "elm.text.subtitle";
static const char TITLE_ACCESS_PART[] = "access.title";

static const char SIG_TRANSITION_FINISHED[] = "transition,finished";
static const char SIG_TITLE_CLICKED[] = "title,clicked";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_TRANSITION_FINISHED, ""},
   {SIG_TITLE_CLICKED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_NAVIFRAME_SMART_NAME, _elm_naviframe, Elm_Naviframe_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static const char SIG_CLICKED[] = "clicked";

static void _on_item_back_btn_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__);

static Eina_Bool
_elm_naviframe_smart_translate(Evas_Object *obj)
{
   ELM_NAVIFRAME_DATA_GET(obj, sd);
   Elm_Naviframe_Item *it;

   EINA_INLIST_FOREACH(sd->stack, it)
     elm_widget_item_translate(it);

   return EINA_TRUE;
}

static void
_item_content_del_cb(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->content = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");
}

static void
_item_title_prev_btn_del_cb(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_prev_btn = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");
}

static void
_item_title_next_btn_del_cb(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_next_btn = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");
}

static void
_item_title_icon_del_cb(void *data,
                        Evas *e __UNUSED__,
                        Evas_Object *obj __UNUSED__,
                        void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_icon = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");
}

static void
_title_content_del(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   char buf[1024];
   Elm_Naviframe_Content_Item_Pair *pair = data;
   Elm_Naviframe_Item *it = pair->it;
   snprintf(buf, sizeof(buf), "elm,state,%s,hide", pair->part);
   elm_object_signal_emit(VIEW(it), buf, "elm");
   it->content_list = eina_inlist_remove(it->content_list,
                                         EINA_INLIST_GET(pair));
   eina_stringshare_del(pair->part);
   free(pair);
}

static void
_item_free(Elm_Naviframe_Item *it)
{
   Eina_Inlist *l;
   Elm_Naviframe_Content_Item_Pair *content_pair;
   Elm_Naviframe_Text_Item_Pair *text_pair;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   eina_stringshare_del(it->style);
   eina_stringshare_del(it->title_label);
   eina_stringshare_del(it->subtitle_label);

   EINA_INLIST_FOREACH_SAFE(it->content_list, l, content_pair)
     {
        if (content_pair->content)
          {
             evas_object_event_callback_del(content_pair->content,
                                            EVAS_CALLBACK_DEL,
                                            _title_content_del);
             evas_object_del(content_pair->content);
          }
        eina_stringshare_del(content_pair->part);
        free(content_pair);
     }
   EINA_INLIST_FOREACH_SAFE(it->text_list, l, text_pair)
     {
        eina_stringshare_del(text_pair->part);
        free(text_pair);
     }

   if (it->content)
     {
        if ((sd->preserve) && (!sd->on_deletion))
          {
             /* so that elm does not delete the contents with the item's
              * view after the del_pre_hook */
             elm_object_part_content_unset(VIEW(it), CONTENT_PART);
             evas_object_event_callback_del
                (it->content, EVAS_CALLBACK_DEL, _item_content_del_cb);
          }
     }
}

static void
_item_content_signals_emit(Elm_Naviframe_Item *it)
{
   Elm_Naviframe_Content_Item_Pair *content_pair;
   char buf[1024];
   //content
   if (it->content)
     elm_object_signal_emit(VIEW(it), "elm,state,content,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");

   //prev button
   if (it->title_prev_btn)
     elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");

   //next button
   if (it->title_next_btn)
     elm_object_signal_emit(VIEW(it), "elm,state,next_btn,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");

   if (it->title_icon)
     elm_object_signal_emit(VIEW(it), "elm,state,icon,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");

   EINA_INLIST_FOREACH(it->content_list, content_pair)
     {
        if (content_pair->content)
          snprintf(buf, sizeof(buf), "elm,state,%s,show", content_pair->part);
        else
          snprintf(buf, sizeof(buf), "elm,state,%s,hide", content_pair->part);
        elm_object_signal_emit(VIEW(it), buf, "elm");
     }
}

static void
_item_text_signals_emit(Elm_Naviframe_Item *it)
{
   Elm_Naviframe_Text_Item_Pair *text_pair;
   char buf[1024];

   if ((it->title_label) && (it->title_label[0]))
     elm_object_signal_emit(VIEW(it), "elm,state,title_label,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,title_label,hide", "elm");

   if ((it->subtitle_label) && (it->subtitle_label[0]))
     elm_object_signal_emit(VIEW(it), "elm,state,subtitle,show", "elm");
   else
     elm_object_signal_emit(VIEW(it), "elm,state,subtitle,hide", "elm");

   EINA_INLIST_FOREACH(it->text_list, text_pair)
     {
        if (elm_object_part_text_get(VIEW(it), text_pair->part))
          snprintf(buf, sizeof(buf), "elm,state,%s,show", text_pair->part);
        else
          snprintf(buf, sizeof(buf), "elm,state,%s,hide", text_pair->part);
        elm_object_signal_emit(VIEW(it), buf, "elm");
     }
}

static void
_access_focus_set(Elm_Naviframe_Item *it)
{
   Evas_Object *ao;

   if (it->unfocusable) return;

   ao = ((Elm_Widget_Item *)it)->access_obj;

   if (ao && it->title_visible)
     {
        elm_object_focus_set(ao, EINA_TRUE);
     }
   else if ((it->title_icon) &&
            (elm_widget_can_focus_get(it->title_icon) ||
             elm_widget_child_can_focus_get(it->title_icon)))
     {
        elm_object_focus_set(it->title_icon, EINA_TRUE);
     }
   else elm_object_focus_set(it->content, EINA_TRUE);
}

static void
_item_signals_emit(Elm_Naviframe_Item *it)
{
   _item_text_signals_emit(it);
   _item_content_signals_emit(it);
}

/* FIXME: we need to handle the case when this function is called
 * during a transition */
static void
_item_style_set(Elm_Naviframe_Item *it,
                const char *item_style)
{
   char buf[256];

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   if (!item_style)
     {
        strcpy(buf, "item/basic");
        eina_stringshare_replace(&it->style, "basic");
     }
   else
     {
        snprintf(buf, sizeof(buf), "item/%s", item_style);
        eina_stringshare_replace(&it->style, item_style);
     }

   elm_layout_theme_set(VIEW(it), "naviframe", buf,
                        elm_widget_style_get(WIDGET(it)));

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);
}

static void
_item_title_visible_update(Elm_Naviframe_Item *nit)
{
   /* access */
   if (_elm_config->access_mode) _access_focus_set(nit);

   if (nit->title_visible)
     elm_object_signal_emit(VIEW(nit), "elm,state,title,show", "elm");
   else
     elm_object_signal_emit(VIEW(nit), "elm,state,title,hide", "elm");
}

static Eina_Bool
_elm_naviframe_smart_theme(Evas_Object *obj)
{
   Elm_Naviframe_Item *it;
   const char *style, *sstyle;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   style = elm_widget_style_get(obj);

   EINA_INLIST_FOREACH(sd->stack, it)
     {
        /* FIXME: Need to merge with opensource later. smart_theme() is called
           multiple timese since the elm_widget.c is inefficient. If the
           elm_widget is merged with the latest opensouce, it's ok to sync with
           opensource here also */
        sstyle = elm_widget_style_get(VIEW(it));
        if ((style && sstyle) && strcmp(style, sstyle))
          {
             _item_style_set(it, it->style);
             _item_signals_emit(it);
             _item_title_visible_update(it);
          }
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static char *
_access_info_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Evas_Object *layout;
   Eina_Strbuf *buf;
   const char *info;
   char *ret;

   layout = (Evas_Object *)data;
   info = elm_object_part_text_get(layout, TITLE_PART);
   if (!info) return NULL;

   buf = eina_strbuf_new();
   eina_strbuf_append(buf, info);

   info = elm_object_part_text_get(layout, SUBTITLE_PART);
   if (!info) goto end;

   eina_strbuf_append_printf(buf, ", %s", info);

end:
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_access_obj_process(Elm_Naviframe_Item *it, Eina_Bool is_access)
{
   Evas_Object *ao, *eo;

   if (is_access)
     {
        ao = ((Elm_Widget_Item *)it)->access_obj;

        if (!ao)
          {
             eo = elm_layout_edje_get(VIEW(it));
             ao =_elm_access_edje_object_part_object_register(WIDGET(it), eo,
                                                              TITLE_ACCESS_PART);
             _elm_access_text_set(_elm_access_object_get(ao),
                                 ELM_ACCESS_TYPE, E_("title"));
             _elm_access_callback_set(_elm_access_object_get(ao),
                                      ELM_ACCESS_INFO, _access_info_cb, VIEW(it));

             /* to access title access object, any idea? */
             ((Elm_Widget_Item *)it)->access_obj = ao;
          }
     }
   else
     {
        if (it->title_label)
          _elm_access_edje_object_part_object_unregister
                (WIDGET(it), elm_layout_edje_get(VIEW(it)), TITLE_ACCESS_PART);

        /* to access title access object, any idea? */
        ao = ((Elm_Widget_Item *)it)->access_obj;
        if (!ao) return;
        evas_object_del(ao);
     }
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;
   Elm_Naviframe_Text_Item_Pair *pair = NULL;
   char buf[1024];

   if ((!part) || (!strcmp(part, "default")) ||
       (!strcmp(part, TITLE_PART)))
     {
        eina_stringshare_replace(&nit->title_label, label);
        if (label)
          elm_object_signal_emit(VIEW(it), "elm,state,title_label,show", "elm");
        else
          elm_object_signal_emit(VIEW(it), "elm,state,title_label,hide", "elm");
        elm_object_part_text_set(VIEW(it), TITLE_PART, label);
     }
   else if (!strcmp("subtitle", part))
     {
        eina_stringshare_replace(&nit->subtitle_label, label);
        if (label)
          elm_object_signal_emit(VIEW(it), "elm,state,subtitle,show", "elm");
        else
          elm_object_signal_emit(VIEW(it), "elm,state,subtitle,hide", "elm");
        elm_object_part_text_set(VIEW(it), SUBTITLE_PART, label);
     }
   else
     {
        EINA_INLIST_FOREACH(nit->text_list, pair)
          if (!strcmp(part, pair->part)) break;

        if (!pair)
          {
             pair = ELM_NEW(Elm_Naviframe_Text_Item_Pair);
             if (!pair)
               {
                  ERR("Failed to allocate new text part of the item! : naviframe=%p",
                  WIDGET(it));
                  return;
               }
             eina_stringshare_replace(&pair->part, part);
             nit->text_list = eina_inlist_append(nit->text_list,
                                                 EINA_INLIST_GET(pair));
          }
        if (label)
          snprintf(buf, sizeof(buf), "elm,state,%s,show", part);
        else
          snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
        elm_object_signal_emit(VIEW(it), buf, "elm");
        elm_object_part_text_set(VIEW(it), part, label);
     }

   /* access */
   if (_elm_config->access_mode)
     {
        Evas_Object *ao;

        _access_obj_process(nit, EINA_TRUE);

        ao = ((Elm_Widget_Item *)nit)->access_obj;
        if (!elm_object_focus_get(ao)) elm_object_focus_set(ao, EINA_TRUE);
     }

   elm_layout_sizing_eval(WIDGET(nit));
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   char buf[1024];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), TITLE_PART);
   else if (!strcmp("subtitle", part))
     snprintf(buf, sizeof(buf), SUBTITLE_PART);
   else
     snprintf(buf, sizeof(buf), "%s", part);

   return elm_object_part_text_get(VIEW(it), buf);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit, *prev_it = NULL;
   Eina_Bool top;

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   nit->delete_me = EINA_TRUE;
   if (nit->ref > 0) return EINA_FALSE;

   if (nit->animator) ecore_animator_del(nit->animator);

   top = (it == elm_naviframe_top_item_get(WIDGET(nit)));
   if (evas_object_data_get(VIEW(nit), "out_of_list"))
     goto end;

   if (!sd->on_deletion)
     {
        nit->unfocusable = elm_widget_tree_unfocusable_get(VIEW(nit));
        elm_widget_tree_unfocusable_set(VIEW(nit), EINA_TRUE);
     }

   sd->stack = eina_inlist_remove(sd->stack, EINA_INLIST_GET(nit));
   if (!sd->stack) elm_widget_resize_object_set(WIDGET(it), sd->dummy_edje);

   if (top && !sd->on_deletion) /* must raise another one */
     {
        if (sd->stack && sd->stack->last)
          prev_it = EINA_INLIST_CONTAINER_GET(sd->stack->last,
                                              Elm_Naviframe_Item);
        if (!prev_it) goto end;

        if (sd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(prev_it), EINA_FALSE);
          }
        elm_widget_resize_object_set(WIDGET(prev_it), VIEW(prev_it));
        evas_object_show(VIEW(prev_it));
        evas_object_raise(VIEW(prev_it));

        elm_object_signal_emit(VIEW(prev_it), "elm,state,visible", "elm");
     }

end:

   _item_free(nit);

   return EINA_TRUE;
}

static void
_item_content_set(Elm_Naviframe_Item *it,
                  Evas_Object *content)
{
   if (it->content == content) return;

   if (it->content) evas_object_del(it->content);
   it->content = content;

   if (!content) return;

   elm_object_part_content_set(VIEW(it), CONTENT_PART, content);
   elm_object_signal_emit(VIEW(it), "elm,state,content,show", "elm");

   evas_object_event_callback_add
     (content, EVAS_CALLBACK_DEL, _item_content_del_cb, it);
}

//Tizen Only: Temporary code. block the focus for the back button for
//H/W Key event support.
static void
_hide_button_prop_set(Elm_Naviframe_Item *it, Evas_Object *prev_btn)
{
   if (it->dispmode == EVAS_DISPLAY_MODE_COMPRESS)
     {
        elm_object_signal_emit(prev_btn, "elm,state,display,compress", "elm");
        evas_object_propagate_events_set(prev_btn, EINA_FALSE);
        elm_object_focus_allow_set(prev_btn, EINA_FALSE);
     }
   else
     {
        elm_object_signal_emit(prev_btn, "elm,state,display,default", "elm");
        evas_object_propagate_events_set(prev_btn, EINA_TRUE);
        elm_object_focus_allow_set(prev_btn, EINA_TRUE);
     }
}

static void
_item_title_prev_btn_set(Elm_Naviframe_Item *it,
                         Evas_Object *btn)
{
   const char* txt;
   if (it->title_prev_btn == btn) return;
   if (it->title_prev_btn) evas_object_del(it->title_prev_btn);
   it->title_prev_btn = btn;
   if (!btn) return;

   elm_object_part_content_set(VIEW(it), PREV_BTN_PART, btn);
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,show", "elm");
   evas_object_event_callback_add
     (btn, EVAS_CALLBACK_DEL, _item_title_prev_btn_del_cb, it);
   evas_object_smart_callback_add
     (btn, SIG_CLICKED, _on_item_back_btn_clicked, WIDGET(it));

   _hide_button_prop_set(it, btn);
}

static void
_item_title_next_btn_set(Elm_Naviframe_Item *it,
                         Evas_Object *btn)
{
   if (it->title_next_btn == btn) return;
   if (it->title_next_btn) evas_object_del(it->title_next_btn);
   it->title_next_btn = btn;
   if (!btn) return;

   elm_object_part_content_set(VIEW(it), NEXT_BTN_PART, btn);
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,show", "elm");

   evas_object_event_callback_add
     (btn, EVAS_CALLBACK_DEL, _item_title_next_btn_del_cb, it);
}

static void
_item_title_icon_set(Elm_Naviframe_Item *it,
                     Evas_Object *icon)
{
   if (it->title_icon == icon) return;
   if (it->title_icon) evas_object_del(it->title_icon);
   it->title_icon = icon;
   if (!icon) return;

   elm_object_part_content_set(VIEW(it), ICON_PART, icon);
   elm_object_signal_emit(VIEW(it), "elm,state,icon,show", "elm");

   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_DEL, _item_title_icon_del_cb, it);
}

static Evas_Object *
_item_content_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->content;

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), CONTENT_PART);
   elm_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_content_del_cb);

   it->content = NULL;
   return content;
}

static Evas_Object *
_item_title_prev_btn_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_prev_btn;

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), PREV_BTN_PART);
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_prev_btn_del_cb);

   evas_object_smart_callback_del(content, SIG_CLICKED,
                                  _on_item_back_btn_clicked);

   it->title_prev_btn = NULL;
   return content;
}

static Evas_Object *
_item_title_next_btn_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_next_btn;

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), NEXT_BTN_PART);
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_next_btn_del_cb);

   it->title_next_btn = NULL;
   return content;
}

static Evas_Object *
_item_title_icon_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_icon;

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), ICON_PART);
   elm_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_icon_del_cb);

   it->title_icon = NULL;
   return content;
}

/* since we have each item as layout, we can't reusing the layout's
 * aliasing, so let's do it ourselves */
static void
_part_aliasing_eval(const char **part)
{
   if (!*part || !strcmp("default", *part))
     *part = CONTENT_PART;
   else if (!strcmp(*part, "prev_btn"))
     *part = PREV_BTN_PART;
   else if (!strcmp(*part, "next_btn"))
     *part = NEXT_BTN_PART;
   else if (!strcmp(*part, "icon"))
     *part = ICON_PART;
}

static void
_title_content_set(Elm_Naviframe_Item *it,
                   const char *part,
                   Evas_Object *content)
{
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   char buf[1024];

   EINA_INLIST_FOREACH(it->content_list, pair)
     if (!strcmp(part, pair->part)) break;
   if (pair)
     {
        if (pair->content == content) return;
        if (pair->content)
          evas_object_event_callback_del(pair->content,
                                         EVAS_CALLBACK_DEL,
                                         _title_content_del);
       if (content) elm_object_part_content_set(VIEW(it), part, content);
     }
   else
     {
        if (!content) return;

        //Remove the pair if new content was swallowed into other part.
        EINA_INLIST_FOREACH(it->content_list, pair)
          {
             if (pair->content == content)
               {
                  eina_stringshare_del(pair->part);
                  it->content_list = eina_inlist_remove(it->content_list,
                                                        EINA_INLIST_GET(pair));
                  evas_object_event_callback_del(pair->content,
                                                 EVAS_CALLBACK_DEL,
                                                 _title_content_del);
                  free(pair);
                  break;
               }
          }

        //New pair
        pair = ELM_NEW(Elm_Naviframe_Content_Item_Pair);
        if (!pair)
          {
             ERR("Failed to allocate new content part of the item! : naviframe=%p",
             WIDGET(it));
             return;
          }
        pair->it = it;
        eina_stringshare_replace(&pair->part, part);
        it->content_list = eina_inlist_append(it->content_list,
                                              EINA_INLIST_GET(pair));
        elm_object_part_content_set(VIEW(it), part, content);
        snprintf(buf, sizeof(buf), "elm,state,%s,show", part);
        elm_object_signal_emit(VIEW(it), buf, "elm");
     }
   pair->content = content;
   evas_object_event_callback_add(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_content_del,
                                  pair);
   /* access */
   if (_elm_config->access_mode)
     _access_obj_process(it, EINA_TRUE);

   /* Tizen Only */
   if (!strcmp(part, MORE_BTN_PART))
     _elm_access_callback_set
        (_elm_access_object_get(content), ELM_ACCESS_INFO,
         _access_more_btn_info_cb, it);
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     _item_content_set(nit, content);
   else if (!strcmp(part, PREV_BTN_PART))
     _item_title_prev_btn_set(nit, content);
   else if (!strcmp(part, NEXT_BTN_PART))
     _item_title_next_btn_set(nit, content);
   else if (!strcmp(part, ICON_PART))
     _item_title_icon_set(nit, content);
   else
     _title_content_set(nit, part, content);

   elm_layout_sizing_eval(WIDGET(it));
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     return nit->content;
   else if (!strcmp(part, PREV_BTN_PART))
     return nit->title_prev_btn;
   else if (!strcmp(part, NEXT_BTN_PART))
     return nit->title_next_btn;
   else if (!strcmp(part, ICON_PART))
     return nit->title_icon;

   //common parts
   return elm_object_part_content_get(VIEW(nit), part);
}

static Evas_Object *
_title_content_unset(Elm_Naviframe_Item *it, const char *part)
{
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   char buf[1028];
   Evas_Object *content = NULL;

   EINA_INLIST_FOREACH(it->content_list, pair)
     {
        if (!strcmp(part, pair->part))
          {
             content = pair->content;
             eina_stringshare_del(pair->part);
             it->content_list = eina_inlist_remove(it->content_list,
                                                   EINA_INLIST_GET(pair));
             free(pair);
             break;
          }
     }

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), part);
   snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
   elm_object_signal_emit(VIEW(it), buf, "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_content_del);
   return content;
}

static Evas_Object *
_item_content_unset_hook(Elm_Object_Item *it,
                         const char *part)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;
   Evas_Object *o = NULL;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     o = _item_content_unset(nit);
   else if (!strcmp(part, PREV_BTN_PART))
     o = _item_title_prev_btn_unset(nit);
   else if (!strcmp(part, NEXT_BTN_PART))
     o = _item_title_next_btn_unset(nit);
   else if (!strcmp(part, ICON_PART))
     o = _item_title_icon_unset(nit);
   else
     o = _title_content_unset(nit, part);

   elm_layout_sizing_eval(WIDGET(it));

   return o;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   elm_object_signal_emit(VIEW(it), emission, source);
}

static void
_elm_naviframe_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   Elm_Naviframe_Item *it;
   Evas_Coord x, y, w, h;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   if (sd->on_deletion) return;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   EINA_INLIST_FOREACH(sd->stack, it)
     {
        evas_object_move(VIEW(it), x, y);
        evas_object_resize(VIEW(it), w, h);
        edje_object_size_min_calc(elm_layout_edje_get(VIEW(it)),
                                  &it->minw, &it->minh);
        if (it->minw > minw) minw = it->minw;
        if (it->minh > minh) minh = it->minh;
     }

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

//Tizen Only: Original
#if 0
static void
_on_item_back_btn_clicked(void *data,
                          Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   /* Since edje has the event queue, clicked event could be happend
      multiple times on some heavy environment. This callback del will
      prevent those scenario and guarantee only one clicked for it's own
      page. */
   evas_object_smart_callback_del(obj, SIG_CLICKED, _on_item_back_btn_clicked);
   elm_naviframe_item_pop(data);
}
#else

//Tizen Only: Get Clipboard Window
static Ecore_X_Window
_cbhm_window_get()
{
   Ecore_X_Atom cbhm_atom = ecore_x_atom_get("CBHM_ELM_WIN");
   Ecore_X_Window cbhm_win = NULL;
   Ecore_X_Window root = ecore_x_window_root_first_get();
   ecore_x_window_prop_xid_get(root, cbhm_atom, ECORE_X_ATOM_WINDOW, &cbhm_win, 1);

   return cbhm_win;
}

//Tizen Only: Get Virtual Keyboard Window
static Ecore_X_Window
_vkb_window_get()
{
   Ecore_X_Atom vkb_atom = ecore_x_atom_get("_ISF_CONTROL_WINDOW");
   Ecore_X_Window vkb_win = NULL;
   Ecore_X_Window root = ecore_x_window_root_first_get();
   ecore_x_window_prop_xid_get(root, vkb_atom, ECORE_X_ATOM_WINDOW, &vkb_win, 1);

   return vkb_win;
}

//Tizen Only: Customized
static void
_on_item_back_btn_clicked(void *data,
                          Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   static Ecore_X_Window keygrab_win = NULL;
   Ecore_X_Atom type = ecore_x_atom_get("_HWKEY_EMULATION");
   char msg_data[20];

   Ecore_X_Window cbhm_win = _cbhm_window_get();
   Ecore_X_Window zone = ecore_x_e_illume_zone_get(cbhm_win);
   Ecore_X_Illume_Clipboard_State cbhm_state = ecore_x_e_illume_clipboard_state_get(zone);

   Ecore_X_Window vkb_win = _vkb_window_get();
   Ecore_X_Virtual_Keyboard_State vkb_state = ecore_x_e_virtual_keyboard_state_get(vkb_win);

   if ((cbhm_state == ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN || cbhm_state == ECORE_X_ILLUME_CLIPBOARD_STATE_OFF)
       && (vkb_state == ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN || vkb_state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF))
     evas_object_focus_set(obj, EINA_TRUE);

   //Get the keygrab window handle.
   if (!keygrab_win)
     {
        Ecore_X_Window *_keygrab_win = NULL;
        int num;
        ecore_x_window_prop_property_get(NULL, type, ECORE_X_ATOM_WINDOW, 32,
                                         (unsigned char **)&_keygrab_win, &num);
        if (!_keygrab_win)
          {
             ERR("Failed to get the key grabber window, naviframe(%p)", obj);
             return;
          }
        keygrab_win = *_keygrab_win;
        free(_keygrab_win);
     }

   //Press
   snprintf(msg_data, sizeof(msg_data), "Px%s", KEY_END);
   if (!ecore_x_client_message8_send(keygrab_win, type, msg_data,
                                     sizeof(msg_data)))
     {
        ERR("Failed to send message for h/w press, naviframe(%p)", obj);
     }

   //Release
   snprintf(msg_data, sizeof(msg_data), "Rx%s", KEY_END);
   if (!ecore_x_client_message8_send(keygrab_win, type,
                                     msg_data, sizeof(msg_data)))
     {
        ERR("Failed to send message for h/w release, naviframe(%p)", obj);
     }
}
#endif

static Evas_Object *
_back_btn_new(Evas_Object *obj, const char *title_label)
{
   Evas_Object *btn, *ed;
   char buf[1024];

   btn = elm_button_add(obj);

   if (!btn) return NULL;
   snprintf
     (buf, sizeof(buf), "naviframe/back_btn/%s", elm_widget_style_get(obj));
   elm_object_style_set(btn, buf);
   if (title_label)
     elm_layout_text_set(btn, NULL, title_label);
   else
     elm_object_domain_translatable_text_set(btn, PACKAGE, N_("Back"));

   /* HACK NOTE: this explicit check only exists to avoid an ERR()
    * message from elm_layout_content_set().
    *
    * The button was ALWAYS supposed to support an elm.swallow.content, but
    * default naviframe/back_btn/default theme did not provide such, then
    * old themes would emit such error message.
    *
    * Once we can break the theme API, remove this check and always
    * set an icon.
    */
   ed = elm_layout_edje_get(btn);
   if (edje_object_part_exists(ed, CONTENT_PART))
     {
        Evas_Object *ico = elm_icon_add(btn);
        elm_icon_standard_set(ico, "arrow_left");
        elm_layout_content_set(btn, CONTENT_PART, ico);
     }

   return btn;
}

static void
_elm_naviframe_smart_signal_callback_add(Evas_Object *obj,
                                  const char *emission,
                                  const char *source,
                                  Edje_Signal_Cb func_cb,
                                  void *data)
{
   ELM_NAVIFRAME_DATA_GET(obj, sd);
   Elm_Object_Item *it;

   if (!sd->stack) return;

   _elm_naviframe_parent_sc->callback_add(obj, emission, source, func_cb, data);

   it = elm_naviframe_top_item_get(obj);
   if (!it) return;

   elm_object_signal_callback_add(VIEW(it), emission, source, func_cb, data);
}

static void
_elm_naviframe_smart_signal(Evas_Object *obj,
                            const char *emission,
                            const char *source)
{
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   if (!sd->stack) return;

   _elm_naviframe_parent_sc->signal(obj, emission, source);
}

/* content/text smart functions proxying things to the top item, which
 * is the resize object of the layout */
static Eina_Bool
_elm_naviframe_smart_text_set(Evas_Object *obj,
                              const char *part,
                              const char *label)
{
   Elm_Object_Item *it;

   it = elm_naviframe_top_item_get(obj);
   if (!it) return EINA_FALSE;

   elm_object_item_part_text_set(it, part, label);

   return !strcmp(elm_object_item_part_text_get(it, part), label);
}

static const char *
_elm_naviframe_smart_text_get(const Evas_Object *obj,
                              const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);

   if (!it) return NULL;

   return elm_object_item_part_text_get(it, part);
}

/* we have to keep a "manual" set here because of the callbacks on the
 * children */
static Eina_Bool
_elm_naviframe_smart_content_set(Evas_Object *obj,
                                 const char *part,
                                 Evas_Object *content)
{
   Elm_Object_Item *it;

   it = elm_naviframe_top_item_get(obj);
   if (!it) return EINA_FALSE;

   elm_object_item_part_content_set(it, part, content);

   return content == elm_object_item_part_content_get(it, part);
}

static Evas_Object *
_elm_naviframe_smart_content_get(const Evas_Object *obj,
                                 const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);

   if (!it) return NULL;

   return elm_object_item_part_content_get(it, part);
}

static Evas_Object *
_elm_naviframe_smart_content_unset(Evas_Object *obj,
                                   const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);

   if (!it) return NULL;

   return elm_object_item_part_content_unset(it, part);
}

static void
_on_item_title_clicked(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   evas_object_smart_callback_call(WIDGET(it), SIG_TITLE_CLICKED, it);
}

/* "elm,state,cur,pushed"
 */
static void
_on_item_push_finished(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   if (!it) return;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   evas_object_hide(VIEW(it));

   elm_widget_tree_unfocusable_set(VIEW(it), it->unfocusable);

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);
}

/* "elm,state,cur,popped"
 */
static void
_on_item_pop_finished(void *data,
                      Evas_Object *obj __UNUSED__,
                      const char *emission __UNUSED__,
                      const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   if (sd->preserve)
     elm_widget_tree_unfocusable_set(VIEW(it), it->unfocusable);
   sd->popping = eina_list_remove(sd->popping, it);

   elm_widget_item_del(data);
}

/* "elm,state,new,pushed",
 * "elm,state,prev,popped
 */
static void
_on_item_show_finished(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   unsigned int order = 0;
   Evas_Object *newest;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   elm_object_signal_emit(VIEW(it), "elm,state,visible", "elm");

   elm_widget_tree_unfocusable_set(VIEW(it), it->unfocusable);

   newest = elm_widget_newest_focus_order_get(VIEW(it), &order, EINA_TRUE);
   if (newest)
   {
     elm_object_focus_set(newest, EINA_TRUE);
   }
   else
   {
     if (_elm_config->access_mode)
       _access_focus_set(it);
     else
       elm_object_focus_set(VIEW(it), EINA_TRUE);
   }

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);

   evas_object_smart_callback_call(WIDGET(it), SIG_TRANSITION_FINISHED, data);
}

static void
_on_item_size_hints_changed(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_item_dispmode_set(Elm_Naviframe_Item *it, Evas_Display_Mode dispmode)
{
   if (it->dispmode == dispmode) return;
   switch (dispmode)
     {
      case EVAS_DISPLAY_MODE_COMPRESS:
         elm_object_signal_emit(VIEW(it), "elm,state,display,compress", "elm");
         break;
      default:
         elm_object_signal_emit(VIEW(it), "elm,state,display,default", "elm");
         break;
     }
   it->dispmode = dispmode;

   if (it->title_prev_btn)
     _hide_button_prop_set(it, it->title_prev_btn);
}

static Elm_Naviframe_Item *
_item_new(Evas_Object *obj,
          const Elm_Naviframe_Item *prev_it,
          const char *title_label,
          Evas_Object *prev_btn,
          Evas_Object *next_btn,
          Evas_Object *content,
          const char *item_style)
{
   Elm_Naviframe_Item *it;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   it = elm_widget_item_new(obj, Elm_Naviframe_Item);
   if (!it)
     {
        ERR("Failed to allocate new item! : naviframe=%p", obj);
        return NULL;
     }

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);

   //item base layout
   VIEW(it) = elm_layout_add(obj);
   evas_object_smart_member_add(VIEW(it), obj);

   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_item_size_hints_changed, obj);

   elm_object_signal_callback_add
     (VIEW(it), "elm,action,show,finished", "", _on_item_show_finished, it);
   elm_object_signal_callback_add
     (VIEW(it), "elm,action,pushed,finished", "", _on_item_push_finished, it);
   elm_object_signal_callback_add
     (VIEW(it), "elm,action,popped,finished", "", _on_item_pop_finished, it);
   elm_object_signal_callback_add
     (VIEW(it), "elm,action,title,clicked", "", _on_item_title_clicked, it);

   _item_style_set(it, item_style);

   if (title_label)
     _item_text_set_hook((Elm_Object_Item *)it, TITLE_PART, title_label);

   //title buttons
   if ((!prev_btn) && sd->auto_pushed && prev_it)
     {
        const char *prev_title = prev_it->title_label;
        prev_btn = _back_btn_new(obj, prev_title);
     }

   if (prev_btn)
     _item_content_set_hook((Elm_Object_Item *)it, PREV_BTN_PART, prev_btn);

   if (next_btn)
     {
        _item_content_set_hook((Elm_Object_Item *)it, NEXT_BTN_PART, next_btn);

        if (!elm_layout_text_get(next_btn, NULL))
          _elm_access_text_set
            (_elm_access_object_get(next_btn), ELM_ACCESS_INFO, E_("Next"));
     }

   _item_content_set(it, content);
   _item_dispmode_set(it, sd->dispmode);

   it->title_visible = EINA_TRUE;

   return it;
}

static void
_on_obj_size_hints_changed(void *data __UNUSED__, Evas *e __UNUSED__,
                           Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it;
   Evas_Display_Mode dispmode;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   dispmode = evas_object_size_hint_display_mode_get(obj);
   if (sd->dispmode == dispmode) return;

   sd->dispmode = dispmode;

   EINA_INLIST_FOREACH(sd->stack, it)
     _item_dispmode_set(it, dispmode);
}

static Eina_Bool
_elm_naviframe_smart_focus_next(const Evas_Object *obj,
                                Elm_Focus_Direction dir,
                                Evas_Object **next)
{
   Evas_Object *ao;
   Eina_Bool ret;
   Eina_List *l = NULL;
   Elm_Naviframe_Item *top_it;
   void *(*list_data_get)(const Eina_List *list);

   top_it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!top_it) return EINA_FALSE;

   list_data_get = eina_list_data_get;

   l = eina_list_append(l, VIEW(top_it));

   /* access */
   if (_elm_config->access_mode)
     {
        ao = ((Elm_Widget_Item *)top_it)->access_obj;
        if (ao) l = eina_list_append(l, ao);
     }

   ret = elm_widget_focus_list_next_get(obj, l, list_data_get, dir, next);
   eina_list_free(l);

   return ret;
}

static Eina_Bool
_elm_naviframe_smart_focus_direction(const Evas_Object *obj,
                                     const Evas_Object *base,
                                     double degree,
                                     Evas_Object **direction,
                                     double *weight)
{
   Eina_Bool ret;
   Eina_List *l = NULL;
   Elm_Naviframe_Item *top_it;
   void *(*list_data_get)(const Eina_List *list);

   top_it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!top_it) return EINA_FALSE;

   list_data_get = eina_list_data_get;

   l = eina_list_append(l, VIEW(top_it));
   ret = elm_widget_focus_list_direction_get
      (obj, base, l, list_data_get, degree, direction, weight);
   eina_list_free(l);

   return ret;
}

static void
_elm_naviframe_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Naviframe_Smart_Data);

   ELM_WIDGET_CLASS(_elm_naviframe_parent_sc)->base.add(obj);

   priv->dummy_edje = ELM_WIDGET_DATA(priv)->resize_obj;
   priv->auto_pushed = EINA_TRUE;
   priv->freeze_events = EINA_TRUE;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _on_obj_size_hints_changed, obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static Eina_Bool
_pop_transition_cb(void *data)
{
   Elm_Naviframe_Item *prev_it, *it;
   it = (Elm_Naviframe_Item *)data;

   it->animator = NULL;

   prev_it = (Elm_Naviframe_Item *) elm_naviframe_top_item_get(WIDGET(it));
   if (prev_it)
     {
        elm_object_signal_emit(VIEW(prev_it), "elm,state,prev,popped,deferred",
                               "elm");
        //FIXME: Remove the below line once edje_object_message_signal_process is fixed.
        //This API crashes when in the midst of this API processing if edje object passed here is deleted.
        //edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
     }
   elm_object_signal_emit(VIEW(it), "elm,state,cur,popped,deferred", "elm");

   //FIXME: Remove the below line once edje_object_message_signal_process is fixed.
   //This API crashes when in the midst of this API processing if edje object passed here is deleted.
   //edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));

   return ECORE_CALLBACK_CANCEL;
}

static void
_elm_naviframe_smart_del(Evas_Object *obj)
{
   Elm_Naviframe_Item *it;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   sd->on_deletion = EINA_TRUE;

   while (sd->stack)
     {
        it = EINA_INLIST_CONTAINER_GET(sd->stack, Elm_Naviframe_Item);
        elm_widget_item_del(it);
     }

   //All popping items which are not called yet by animator.
   EINA_LIST_FREE(sd->popping, it)
     {
        if (it->animator) ecore_animator_del(it->animator);
        elm_widget_item_del(it);
     }
   eina_list_free(sd->popping);

   evas_object_del(sd->dummy_edje);

   ELM_WIDGET_CLASS(_elm_naviframe_parent_sc)->base.del(obj);
}

static void
_elm_naviframe_smart_access(Evas_Object *obj, Eina_Bool is_access)
{
   Elm_Naviframe_Item *it;

   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->stack, it)
     _access_obj_process(it, is_access);
}

#if 0
//Tizen Only: Original
static Eina_Bool
_elm_naviframe_smart_event(Evas_Object *obj,
                           Evas_Object *src __UNUSED__,
                           Evas_Callback_Type type,
                           void *event_info)
{
   Elm_Naviframe_Item *it;
   Evas_Event_Key_Down *ev = event_info;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (strcmp(ev->keyname, "Escape")) return EINA_FALSE;

   it = (Elm_Naviframe_Item *) elm_naviframe_top_item_get(obj);
   if (!it) return EINA_FALSE;

   if (it->title_prev_btn)
    evas_object_smart_callback_call(it->title_prev_btn, SIG_CLICKED, NULL);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   return EINA_TRUE;
}
#else
//Tizen Only: Customized
static Eina_Bool
_elm_naviframe_smart_event(Evas_Object *obj,
                           Evas_Object *src __UNUSED__,
                           Evas_Callback_Type type,
                           void *event_info)
{
   Elm_Naviframe_Item *it;
   Evas_Event_Key_Down *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (ev->keyname && strcmp(ev->keyname, KEY_END) && strcmp(ev->keyname, "Escape")) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;

   it = (Elm_Naviframe_Item *) elm_naviframe_top_item_get(obj);
   if (!it) return EINA_FALSE;

   elm_naviframe_item_pop(obj);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   return EINA_TRUE;
}
#endif

static Eina_Bool
_elm_naviframe_smart_activate(Evas_Object *obj, Elm_Activate act)
{
   Elm_Naviframe_Item *it;

   if (act != ELM_ACTIVATE_BACK) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!it) return EINA_FALSE;

   if (it->title_prev_btn)
     evas_object_smart_callback_call(it->title_prev_btn, SIG_CLICKED, NULL);

   return EINA_TRUE;
}

static void
_elm_naviframe_smart_set_user(Elm_Naviframe_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_naviframe_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_naviframe_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_naviframe_smart_theme;
   ELM_WIDGET_CLASS(sc)->translate = _elm_naviframe_smart_translate;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_naviframe_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_naviframe_smart_focus_direction;
   ELM_WIDGET_CLASS(sc)->access = _elm_naviframe_smart_access;
   ELM_WIDGET_CLASS(sc)->event = _elm_naviframe_smart_event;
   ELM_WIDGET_CLASS(sc)->activate = _elm_naviframe_smart_activate;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_naviframe_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_naviframe_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_naviframe_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->signal = _elm_naviframe_smart_signal;
   ELM_LAYOUT_CLASS(sc)->callback_add = _elm_naviframe_smart_signal_callback_add;
   ELM_LAYOUT_CLASS(sc)->text_set = _elm_naviframe_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_naviframe_smart_text_get;
   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_naviframe_smart_sizing_eval;
}

static Eina_Bool
_push_transition_cb(void *data)
{
   Elm_Naviframe_Item *prev_it, *it = data;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   it->animator = NULL;

   if (sd->stack->last->prev)
     {
        prev_it = EINA_INLIST_CONTAINER_GET(sd->stack->last->prev,
                                            Elm_Naviframe_Item);
        elm_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed,deferred",
                                "elm");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
     }
   elm_object_signal_emit(VIEW(it), "elm,state,new,pushed,deferred", "elm");
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));

   return ECORE_CALLBACK_CANCEL;
}

EAPI const Elm_Naviframe_Smart_Class *
elm_naviframe_smart_class_get(void)
{
   static Elm_Naviframe_Smart_Class _sc =
     ELM_NAVIFRAME_SMART_CLASS_INIT_NAME_VERSION
       (ELM_NAVIFRAME_SMART_NAME);
   static const Elm_Naviframe_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_naviframe_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_naviframe_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_naviframe_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   _elm_widget_orient_signal_emit(obj);

   return obj;
}

EAPI Elm_Object_Item *
elm_naviframe_item_push(Evas_Object *obj,
                        const char *title_label,
                        Evas_Object *prev_btn,
                        Evas_Object *next_btn,
                        Evas_Object *content,
                        const char *item_style)
{
   Elm_Naviframe_Item *prev_it, *it;

   ELM_NAVIFRAME_CHECK(obj) NULL;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   prev_it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   it = _item_new(obj, prev_it,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return NULL;

   evas_object_show(VIEW(it));
   elm_widget_resize_object_set(obj, VIEW(it));
   if (prev_it)
     elm_widget_sub_object_add(obj, VIEW(prev_it));
   evas_object_smart_member_add(sd->dummy_edje, obj);

   if (prev_it)
     {
        /* re-add as smart member */
        evas_object_smart_member_add(VIEW(prev_it), obj);

        if (sd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
             evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
          }

        elm_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed", "elm");
        elm_object_signal_emit(VIEW(it), "elm,state,new,pushed", "elm");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));

        prev_it->unfocusable = elm_widget_tree_unfocusable_get(VIEW(prev_it));
        elm_widget_tree_unfocusable_set(VIEW(prev_it), EINA_TRUE);

        it->animator = ecore_animator_add(_push_transition_cb, it);
     }
   else
     elm_object_focus_set(VIEW(it), EINA_TRUE);

   sd->stack = eina_inlist_append(sd->stack, EINA_INLIST_GET(it));
   evas_object_raise(VIEW(it));

   /* access */
   if (_elm_config->access_mode) _access_focus_set(it);

   elm_layout_sizing_eval(obj);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_naviframe_item_insert_before(Evas_Object *obj,
                                 Elm_Object_Item *before,
                                 const char *title_label,
                                 Evas_Object *prev_btn,
                                 Evas_Object *next_btn,
                                 Evas_Object *content,
                                 const char *item_style)
{
   Elm_Naviframe_Item *it, *prev_it = NULL;

   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(before, NULL);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   it = (Elm_Naviframe_Item *)before;
   if (EINA_INLIST_GET(it)->prev)
     prev_it = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(it)->prev,
                                         Elm_Naviframe_Item);
   it = _item_new(obj, prev_it,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return NULL;

   sd->stack = eina_inlist_prepend_relative
       (sd->stack, EINA_INLIST_GET(it),
       EINA_INLIST_GET(((Elm_Naviframe_Item *)before)));

   elm_layout_sizing_eval(obj);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_naviframe_item_insert_after(Evas_Object *obj,
                                Elm_Object_Item *after,
                                const char *title_label,
                                Evas_Object *prev_btn,
                                Evas_Object *next_btn,
                                Evas_Object *content,
                                const char *item_style)
{
   Elm_Naviframe_Item *it;
   Eina_Bool top_inserted = EINA_FALSE;

   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(after, NULL);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   it = _item_new(obj, (Elm_Naviframe_Item *)after,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return NULL;

   if (elm_naviframe_top_item_get(obj) == after) top_inserted = EINA_TRUE;

   sd->stack = eina_inlist_append_relative
       (sd->stack, EINA_INLIST_GET(it),
       EINA_INLIST_GET(((Elm_Naviframe_Item *)after)));

   if (top_inserted)
     {
        elm_widget_resize_object_set(obj, VIEW(it));
        elm_widget_sub_object_add(obj, VIEW(after));
        evas_object_smart_member_add(sd->dummy_edje, obj);
        evas_object_show(VIEW(it));
        evas_object_hide(VIEW(after));
        elm_object_focus_set(VIEW(it), EINA_TRUE);
     }

   /* access */
   if (_elm_config->access_mode) _access_focus_set(it);

   elm_layout_sizing_eval(obj);

   return (Elm_Object_Item *)it;
}

EAPI Evas_Object *
elm_naviframe_item_pop(Evas_Object *obj)
{
   Elm_Naviframe_Item *it, *prev_it = NULL;
   Evas_Object *content = NULL;

   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!it) return NULL;

   if (it->animator || it->popping) return NULL;

   it->popping = EINA_TRUE;

   evas_object_ref(obj);
   if (it->pop_cb)
     {
        it->ref++;
        if (!it->pop_cb(it->pop_data, (Elm_Object_Item *)it))
          {
             it->ref--;
             if (it->delete_me)
               {
                  _item_del_pre_hook(it);
                  _elm_widget_item_free(it);
               }
             it->popping = EINA_FALSE;
             evas_object_unref(obj);

             return NULL;
          }
        it->ref--;
     }
   evas_object_unref(obj);

   if (sd->preserve)
     content = it->content;

   evas_object_data_set(VIEW(it), "out_of_list", (void *)1);

   it->unfocusable = elm_widget_tree_unfocusable_get(VIEW(it));
   elm_widget_tree_unfocusable_set(VIEW(it), EINA_TRUE);

   if (sd->stack->last->prev)
     prev_it = EINA_INLIST_CONTAINER_GET
         (sd->stack->last->prev, Elm_Naviframe_Item);

   sd->stack = eina_inlist_remove(sd->stack, EINA_INLIST_GET(it));
   if (!sd->stack) elm_widget_resize_object_set(obj, sd->dummy_edje);

   if (prev_it)
     {
        if (sd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
             evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
          }

        elm_widget_resize_object_set(obj, VIEW(prev_it));
        evas_object_smart_member_add(sd->dummy_edje, obj);
        evas_object_raise(VIEW(prev_it));

        /* access */
        if (_elm_config->access_mode) _access_focus_set(prev_it);

        /* these 2 signals MUST take place simultaneously */
        elm_object_signal_emit(VIEW(it), "elm,state,cur,popped", "elm");
        evas_object_show(VIEW(prev_it));
        elm_object_signal_emit(VIEW(prev_it), "elm,state,prev,popped", "elm");

        edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));

        if (it->animator) ecore_animator_del(it->animator);
        it->animator = ecore_animator_add(_pop_transition_cb, it);
        sd->popping = eina_list_append(sd->popping, it);
     }
   else
     elm_widget_item_del(it);

   return content;
}

EAPI void
elm_naviframe_item_pop_to(Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit;
   Eina_Inlist *l, *prev_l;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   if (it == elm_naviframe_top_item_get(WIDGET(nit))) return;

   l = sd->stack->last->prev;

   sd->on_deletion = EINA_TRUE;

   while (l)
     {
        Elm_Naviframe_Item *iit = EINA_INLIST_CONTAINER_GET
            (l, Elm_Naviframe_Item);

        if (iit == nit) break;

        prev_l = l->prev;
        sd->stack = eina_inlist_remove(sd->stack, l);

        elm_widget_item_del(iit);

        l = prev_l;
     }

   sd->on_deletion = EINA_FALSE;

   elm_naviframe_item_pop(WIDGET(nit));
}

EAPI void
elm_naviframe_item_promote(Elm_Object_Item *it)
{
   Elm_Object_Item *prev_top;
   Elm_Naviframe_Item *nit;
   Elm_Naviframe_Item *prev_it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   prev_top = elm_naviframe_top_item_get(WIDGET(nit));
   if (it == prev_top) return;

   /* remember, last is 1st on the naviframe, push it to last pos. */
   sd->stack = eina_inlist_demote(sd->stack, EINA_INLIST_GET(nit));

   elm_widget_resize_object_set(WIDGET(it), VIEW(nit));
   elm_widget_sub_object_add(WIDGET(it), VIEW(prev_top));
   evas_object_smart_member_add(sd->dummy_edje, WIDGET(it));

   /* this was the previous top one */
   prev_it = EINA_INLIST_CONTAINER_GET
       (sd->stack->last->prev, Elm_Naviframe_Item);

   /* re-add as smart member */
   evas_object_smart_member_add(VIEW(prev_it), WIDGET(it));

   prev_it->unfocusable = elm_widget_tree_unfocusable_get(VIEW(prev_it));
   elm_widget_tree_unfocusable_set(prev_it->content, EINA_TRUE);

   if (sd->freeze_events)
     {
        evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
        evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
     }

   elm_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed", "elm");

   evas_object_show(VIEW(nit));
   evas_object_raise(VIEW(nit));

   elm_object_signal_emit(VIEW(nit), "elm,state,new,pushed", "elm");

   edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(nit)));
   if (nit->animator) ecore_animator_del(nit->animator);
   nit->animator = ecore_animator_add(_push_transition_cb, nit);

   /* access */
   if (_elm_config->access_mode) _access_focus_set(nit);
}

EAPI void
elm_naviframe_item_simple_promote(Evas_Object *obj,
                                  Evas_Object *content)
{
   Elm_Naviframe_Item *itr;

   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->stack, itr)
     {
        if (elm_object_item_content_get((Elm_Object_Item *)itr) == content)
          {
             elm_naviframe_item_promote((Elm_Object_Item *)itr);
             break;
          }
     }
}

EAPI void
elm_naviframe_content_preserve_on_pop_set(Evas_Object *obj,
                                          Eina_Bool preserve)
{
   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   sd->preserve = !!preserve;
}

EAPI Eina_Bool
elm_naviframe_content_preserve_on_pop_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   return sd->preserve;
}

EAPI Elm_Object_Item *
elm_naviframe_top_item_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   if (!sd->stack) return NULL;
   return (Elm_Object_Item *)(EINA_INLIST_CONTAINER_GET
                                (sd->stack->last, Elm_Naviframe_Item));
}

EAPI Elm_Object_Item *
elm_naviframe_bottom_item_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   if (!sd->stack) return NULL;
   return (Elm_Object_Item *)(EINA_INLIST_CONTAINER_GET
                                (sd->stack, Elm_Naviframe_Item));
}

EAPI void
elm_naviframe_item_style_set(Elm_Object_Item *it,
                             const char *item_style)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   if (item_style && !strcmp(item_style, nit->style)) return;

   if (!item_style)
     if (!strcmp("basic", nit->style)) return;

   _item_style_set(nit, item_style);
   _item_signals_emit(nit);
   _item_title_visible_update(nit);
}

EAPI const char *
elm_naviframe_item_style_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it, NULL);

   return nit->style;
}

EAPI void
elm_naviframe_item_title_visible_set(Elm_Object_Item *it,
                                     Eina_Bool visible)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   visible = !!visible;
   if (nit->title_visible == visible) return;

   nit->title_visible = visible;
   _item_title_visible_update(nit);
}

EAPI Eina_Bool
elm_naviframe_item_title_visible_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return nit->title_visible;
}

EAPI void
elm_naviframe_item_pop_cb_set(Elm_Object_Item *it, Elm_Naviframe_Item_Pop_Cb func, void *data)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   nit->pop_cb = func;
   nit->pop_data = data;
}

EAPI void
elm_naviframe_prev_btn_auto_pushed_set(Evas_Object *obj,
                                       Eina_Bool auto_pushed)
{
   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   sd->auto_pushed = !!auto_pushed;
}

EAPI Eina_Bool
elm_naviframe_prev_btn_auto_pushed_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   return sd->auto_pushed;
}

EAPI Eina_List *
elm_naviframe_items_get(const Evas_Object *obj)
{
   Eina_List *ret = NULL;
   Elm_Naviframe_Item *itr;

   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->stack, itr)
     ret = eina_list_append(ret, itr);

   return ret;
}

EAPI void
elm_naviframe_event_enabled_set(Evas_Object *obj,
                                Eina_Bool enabled)
{
   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   enabled = !!enabled;
   if (sd->freeze_events == !enabled) return;
   sd->freeze_events = !enabled;
}

EAPI Eina_Bool
elm_naviframe_event_enabled_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   return !sd->freeze_events;
}
