#include <fnmatch.h>
#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "elm_widget_genlist.h"

#define GROUP_ITEMS_FEATURE 0
// internally allocated
#define CLASS_ALLOCATED     0x3a70f11f

#define MAX_ITEMS_PER_BLOCK 32
#define REORDER_EFFECT_TIME 0.1
#define FX_MOVE_TIME 0.5

EAPI const char ELM_GENLIST_SMART_NAME[] = "elm_genlist";
EAPI const char ELM_GENLIST_PAN_SMART_NAME[] = "elm_genlist_pan";

#define ERR_ABORT(_msg) \
  ERR(_msg);            \
  if (getenv("ELM_ERROR_ABORT")) abort();

#define GL_IT(_it) (_it->item)

#define IS_ROOT_PARENT_IT(_it) \
   ((_it->group) || ((GL_IT(_it)->items && GL_IT(_it)->expanded_depth == 0)  \
                      &&(!(GL_IT(_it)->type & ELM_GENLIST_ITEM_TREE)))) \

static const Evas_Smart_Interface *_smart_interfaces[] =
{
   (Evas_Smart_Interface *)&ELM_SCROLLABLE_IFACE, NULL
};

static const char SIG_ACTIVATED[] = "activated";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_SELECTED[] = "selected";
static const char SIG_UNSELECTED[] = "unselected";
static const char SIG_EXPANDED[] = "expanded";
static const char SIG_CONTRACTED[] = "contracted";
static const char SIG_EXPAND_REQUEST[] = "expand,request";
static const char SIG_CONTRACT_REQUEST[] = "contract,request";
static const char SIG_REALIZED[] = "realized";
static const char SIG_UNREALIZED[] = "unrealized";
static const char SIG_DRAG_START_UP[] = "drag,start,up";
static const char SIG_DRAG_START_DOWN[] = "drag,start,down";
static const char SIG_DRAG_START_LEFT[] = "drag,start,left";
static const char SIG_DRAG_START_RIGHT[] = "drag,start,right";
static const char SIG_DRAG_STOP[] = "drag,stop";
static const char SIG_DRAG[] = "drag";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_VBAR_DRAG[] = "vbar,drag";
static const char SIG_VBAR_PRESS[] = "vbar,press";
static const char SIG_VBAR_UNPRESS[] = "vbar,unpress";
static const char SIG_HBAR_DRAG[] = "hbar,drag";
static const char SIG_HBAR_PRESS[] = "hbar,press";
static const char SIG_HBAR_UNPRESS[] = "hbar,unpress";
static const char SIG_MULTI_SWIPE_LEFT[] = "multi,swipe,left";
static const char SIG_MULTI_SWIPE_RIGHT[] = "multi,swipe,right";
static const char SIG_MULTI_SWIPE_UP[] = "multi,swipe,up";
static const char SIG_MULTI_SWIPE_DOWN[] = "multi,swipe,down";
static const char SIG_MULTI_PINCH_OUT[] = "multi,pinch,out";
static const char SIG_MULTI_PINCH_IN[] = "multi,pinch,in";
static const char SIG_SWIPE[] = "swipe";
static const char SIG_MOVED[] = "moved";
static const char SIG_MOVED_AFTER[] = "moved,after";
static const char SIG_MOVED_BEFORE[] = "moved,before";
static const char SIG_INDEX_UPDATE[] = "index,update";
static const char SIG_TREE_EFFECT_FINISHED [] = "tree,effect,finished";
static const char SIG_HIGHLIGHTED[] = "highlighted";
static const char SIG_UNHIGHLIGHTED[] = "unhighlighted";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_ACTIVATED, ""},
   {SIG_SELECTED, ""},
   {SIG_UNSELECTED, ""},
   {SIG_EXPANDED, ""},
   {SIG_CONTRACTED, ""},
   {SIG_EXPAND_REQUEST, ""},
   {SIG_CONTRACT_REQUEST, ""},
   {SIG_REALIZED, ""},
   {SIG_UNREALIZED, ""},
   {SIG_DRAG_START_UP, ""},
   {SIG_DRAG_START_DOWN, ""},
   {SIG_DRAG_START_LEFT, ""},
   {SIG_DRAG_START_RIGHT, ""},
   {SIG_DRAG_STOP, ""},
   {SIG_DRAG, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {SIG_EDGE_TOP, ""},
   {SIG_EDGE_BOTTOM, ""},
   {SIG_EDGE_LEFT, ""},
   {SIG_EDGE_RIGHT, ""},
   {SIG_VBAR_DRAG, ""},
   {SIG_VBAR_PRESS, ""},
   {SIG_VBAR_UNPRESS, ""},
   {SIG_HBAR_DRAG, ""},
   {SIG_HBAR_PRESS, ""},
   {SIG_HBAR_UNPRESS, ""},
   {SIG_MULTI_SWIPE_LEFT, ""},
   {SIG_MULTI_SWIPE_RIGHT, ""},
   {SIG_MULTI_SWIPE_UP, ""},
   {SIG_MULTI_SWIPE_DOWN, ""},
   {SIG_MULTI_PINCH_OUT, ""},
   {SIG_MULTI_PINCH_IN, ""},
   {SIG_SWIPE, ""},
   {SIG_MOVED, ""},
   {SIG_MOVED_AFTER, ""},
   {SIG_MOVED_BEFORE, ""},
   {SIG_TREE_EFFECT_FINISHED, ""},
   {SIG_HIGHLIGHTED, ""},
   {SIG_UNHIGHLIGHTED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_IFACE_NEW
  (ELM_GENLIST_SMART_NAME, _elm_genlist, Elm_Genlist_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks,
  _smart_interfaces);

ELM_INTERNAL_SMART_SUBCLASS_NEW
  (ELM_GENLIST_PAN_SMART_NAME, _elm_genlist_pan, Elm_Genlist_Pan_Smart_Class,
  Elm_Pan_Smart_Class, elm_pan_smart_class_get, NULL);

static void      _calc_job(void *);
static Eina_Bool _item_block_recalc(Item_Block *, int, Eina_Bool);
static void      _item_mouse_callbacks_add(Elm_Gen_Item *, Evas_Object *);
static void      _item_mouse_callbacks_del(Elm_Gen_Item *, Evas_Object *);
static void      _access_activate_cb(void *data __UNUSED__,
                                     Evas_Object *part_obj __UNUSED__,
                                     Elm_Object_Item *item);
static void _item_select(Elm_Gen_Item *it);

static void     _expand_toggle_signal_cb(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__);
static void     _expand_signal_cb(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__);
static void     _contract_signal_cb(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__);
static void _elm_genlist_item_state_update(Elm_Gen_Item *it);
static void _decorate_item_unrealize(Elm_Gen_Item *it);
static void _decorate_all_item_unrealize(Elm_Gen_Item *it);
static void _decorate_item_set(Elm_Gen_Item *it);
static void _changed_job(Elm_Genlist_Smart_Data *sd);

#if GENLIST_FX_SUPPORT
static Eina_Bool      _elm_genlist_fx_capture(Evas_Object *obj, int level);
static void           _elm_genlist_fx_play(Evas_Object *obj);
static void           _elm_genlist_fx_clear(Evas_Object *obj, Eina_Bool force);
static void           _elm_genlist_proxy_item_del(const Elm_Object_Item *item);
#endif

#if GENLIST_PINCH_ZOOM_SUPPORT
EAPI Eina_Bool          elm_genlist_pinch_zoom_mode_set(Evas_Object *obj, Elm_Gen_Pinch_Zoom_Mode mode);
#endif

static void
_item_cache_all_free(Elm_Genlist_Smart_Data *sd)
{
   // It would be better not to use
   // EINA_INLIST_FOREACH or EINA_INLIST_FOREACH_SAFE
   while (sd->item_cache)
     {
        Item_Cache *ic = EINA_INLIST_CONTAINER_GET(sd->item_cache->last, Item_Cache);
        if (ic->spacer) evas_object_del(ic->spacer);
        if (ic->base_view) evas_object_del(ic->base_view);
        if (ic->item_style) eina_stringshare_del(ic->item_style);
        sd->item_cache = eina_inlist_remove(sd->item_cache, EINA_INLIST_GET(ic));
        // Free should be performed after inlist is poped
        free(ic);
     }
   sd->item_cache = NULL;
   sd->item_cache_count = 0;
}

static void
_item_cache_push(Elm_Gen_Item *it)
{
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   Item_Cache *ic = NULL;

   if (sd->item_cache_count >= sd->item_cache_max)
    {
        ic = EINA_INLIST_CONTAINER_GET(sd->item_cache->last, Item_Cache);
        if (ic->spacer) evas_object_del(ic->spacer);
        if (ic->base_view) evas_object_del(ic->base_view);
        eina_stringshare_del(ic->item_style);
        sd->item_cache = eina_inlist_remove(sd->item_cache,
                                             sd->item_cache->last);
        sd->item_cache_count--;
        // Free should be performed after inlist is poped
        free(ic);
     }
   //evas_event_freeze(evas_object_evas_get(obj));

   edje_object_mirrored_set(VIEW(it),
                            elm_widget_mirrored_get(WIDGET(it)));
   edje_object_scale_set(VIEW(it),
                         elm_widget_scale_get(WIDGET(it))
                         * elm_config_scale_get());

   // FIXME: other callbacks?
   edje_object_signal_callback_del_full(VIEW(it), "elm,action,expand,toggle",
                                        "elm", _expand_toggle_signal_cb, it);
   edje_object_signal_callback_del_full(VIEW(it), "elm,action,expand", "elm",
                                        _expand_signal_cb, it);
   edje_object_signal_callback_del_full(VIEW(it), "elm,action,contract", "elm",
                                        _contract_signal_cb, it);
   _item_mouse_callbacks_del(it, VIEW(it));

#if GENLIST_FX_SUPPORT
   if ((it->item->nocache_once) || (it->item->nocache) || (sd->pending_del_items))
#else
   if ((it->item->nocache_once) || (it->item->nocache))
#endif
     {
        if (VIEW(it)) evas_object_del(VIEW(it));
        if (it->spacer) evas_object_del(it->spacer);
        goto _ITEM_CLEAN;
     }
   ic = ELM_NEW(Item_Cache);
   if (!ic)
     {
        if (VIEW(it)) evas_object_del(VIEW(it));
        if (it->spacer) evas_object_del(it->spacer);
        goto _ITEM_CLEAN;
     }
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->item->swipe_timer)
     {
        ecore_timer_del(it->item->swipe_timer);
        it->item->swipe_timer = NULL;
     }

   ic->base_view = VIEW(it);
   ic->spacer = it->spacer;
   ic->item_style = eina_stringshare_add(it->itc->item_style);
   if (it->item->type & ELM_GENLIST_ITEM_TREE) ic->tree = 1;

   ic->selected = it->selected;
   ic->disabled = elm_widget_item_disabled_get(it);
   ic->expanded = it->item->expanded;
   if (it->item->type & ELM_GENLIST_ITEM_TREE) ic->tree = 1;

   edje_object_signal_emit(ic->base_view, "elm,state,unselected", "elm");
   evas_object_hide(ic->base_view);

   sd->item_cache = eina_inlist_prepend(sd->item_cache, EINA_INLIST_GET(ic));
   sd->item_cache_count++;

_ITEM_CLEAN:
   VIEW(it) = NULL;
   it->spacer = NULL;
   it->states = NULL;
   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;

   //evas_event_thaw(evas_object_evas_get(obj));
   //evas_event_thaw_eval(evas_object_evas_get(obj));
}

static Eina_Bool
_item_cache_pop(Elm_Gen_Item *it)
{
   Item_Cache *ic = NULL;
   Eina_Inlist *l;
   Eina_Bool tree = 0;
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;

   if (it->item->nocache_once) {
        it->item->nocache_once = EINA_FALSE;
        return EINA_FALSE;
   }
   if (it->item->nocache) return EINA_FALSE;

   if (it->item->type & ELM_GENLIST_ITEM_TREE) tree = 1;
   EINA_INLIST_FOREACH_SAFE(sd->item_cache, l, ic)
     {
        if ((ic->selected) || (ic->disabled) || (ic->expanded))
          continue;

        if ((ic->tree == tree) &&
            (((!it->itc->item_style) && (!ic->item_style)) ||
             (it->itc->item_style && ic->item_style &&
              (!strcmp(it->itc->item_style, ic->item_style)))))
          {
             sd->item_cache =
                eina_inlist_remove(sd->item_cache, EINA_INLIST_GET(ic));
             sd->item_cache_count--;

             VIEW(it) = ic->base_view;
             it->spacer = ic->spacer;

#if 1 // FIXME: different from upstream
             // Because of item cache, set state as default after cache item
             // is popped.
             edje_object_signal_emit(VIEW(it),"elm,state,default", "elm");
#endif
             edje_object_signal_callback_add
                (VIEW(it), "elm,action,expand,toggle", "elm",
                 _expand_toggle_signal_cb, it);
             edje_object_signal_callback_add
                (VIEW(it), "elm,action,expand", "elm", _expand_signal_cb, it);
             edje_object_signal_callback_add
                (VIEW(it), "elm,action,contract", "elm", _contract_signal_cb, it);
             _item_mouse_callbacks_add(it, VIEW(it));

             eina_stringshare_del(ic->item_style);
             free(ic);

             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static void
_elm_genlist_pan_smart_pos_set(Evas_Object *obj,
                               Evas_Coord x,
                               Evas_Coord y)
{
   Item_Block *itb;

   ELM_GENLIST_PAN_DATA_GET(obj, psd);

   if ((x == psd->wsd->pan_x) && (y == psd->wsd->pan_y)) return;
   psd->wsd->pan_x = x;
   psd->wsd->pan_y = y;

   EINA_INLIST_FOREACH(psd->wsd->blocks, itb)
     {
        if ((itb->y + itb->h) > y)
          {
             Elm_Gen_Item *it;
             Eina_List *l2;

             EINA_LIST_FOREACH(itb->items, l2, it)
               {
                  if ((itb->y + it->y) >= y)
                    {
                       psd->wsd->anchor_item = it;
                       psd->wsd->anchor_y = -(itb->y + it->y - y);
                       goto done;
                    }
               }
          }
     }
done:
   if (!psd->wsd->reorder_move_animator) evas_object_smart_changed(obj);
}

static void
_elm_genlist_pan_smart_pos_get(const Evas_Object *obj,
                               Evas_Coord *x,
                               Evas_Coord *y)
{
   ELM_GENLIST_PAN_DATA_GET(obj, psd);

   if (x) *x = psd->wsd->pan_x;
   if (y) *y = psd->wsd->pan_y;
}

static void
_elm_genlist_pan_smart_pos_max_get(const Evas_Object *obj,
                                   Evas_Coord *x,
                                   Evas_Coord *y)
{
   Evas_Coord ow, oh;

   ELM_GENLIST_PAN_DATA_GET(obj, psd);

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = psd->wsd->minw - ow;
   if (ow < 0) ow = 0;
   oh = psd->wsd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;

#if GENLIST_PINCH_ZOOM_SUPPORT
   if (psd->wsd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT)
     {
        evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
        if (psd->wsd->pinch_zoom_h > oh)
          *y = psd->wsd->pinch_zoom_h - oh;
        else *y = 0;
     }
#endif
}

static void
_elm_genlist_pan_smart_pos_min_get(const Evas_Object *obj __UNUSED__,
                                   Evas_Coord *x,
                                   Evas_Coord *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

static void
_elm_genlist_pan_smart_content_size_get(const Evas_Object *obj,
                                        Evas_Coord *w,
                                        Evas_Coord *h)
{
   ELM_GENLIST_PAN_DATA_GET(obj, psd);

   if (w) *w = psd->wsd->minw;
   if (h) *h = psd->wsd->minh;
}

static void
_elm_genlist_pan_smart_add(Evas_Object *obj)
{
   /* here just to allocate our extended data */
   EVAS_SMART_DATA_ALLOC(obj, Elm_Genlist_Pan_Smart_Data);

   ELM_PAN_CLASS(_elm_genlist_pan_parent_sc)->base.add(obj);
}

static void
_elm_genlist_pan_smart_del(Evas_Object *obj)
{
   ELM_GENLIST_PAN_DATA_GET(obj, psd);

   if (psd->resize_job)
     {
        ecore_job_del(psd->resize_job);
        psd->resize_job = NULL;
     }

   _elm_genlist_pan_parent_sc->base.del(obj);
}

static void
_elm_genlist_pan_smart_move(Evas_Object *obj,
                            Evas_Coord x __UNUSED__,
                            Evas_Coord y __UNUSED__)
{
   ELM_GENLIST_PAN_DATA_GET(obj, psd);

   psd->wsd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(obj);
   if (psd->wsd->calc_job) ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = NULL;
}

static void
_elm_genlist_pan_smart_resize(Evas_Object *obj,
                              Evas_Coord w,
                              Evas_Coord h)
{
   ELM_GENLIST_PAN_DATA_GET(obj, psd);

   if (psd->wsd->mode == ELM_LIST_COMPRESS &&
       psd->wsd->prev_viewport_w != w)
     {
        psd->wsd->size_changed = EINA_TRUE;
     }
   psd->wsd->prev_viewport_w = w;

   psd->wsd->pan_changed = EINA_TRUE;
   if (psd->wsd->calc_job) ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job, psd->wsd);
}

static void
_item_scroll(Elm_Genlist_Smart_Data *sd)
{
   Evas_Coord gith = 0;
   Elm_Gen_Item *it = NULL;
   Evas_Coord ow, oh, dx = 0, dy = 0, dw = 0, dh = 0;

   if (!sd->show_item) return;

   evas_object_geometry_get(sd->pan_obj, NULL, NULL, &ow, &oh);
   it = sd->show_item;
   dx = it->x + it->item->block->x;
   dy = it->y + it->item->block->y;
   dw = it->item->block->w;
   dh = oh;

   switch (sd->scroll_to_type)
     {
      case ELM_GENLIST_ITEM_SCROLLTO_TOP:
        if (it->item->group_item) gith = it->item->group_item->item->h;
        dy -= gith;
        break;

      case ELM_GENLIST_ITEM_SCROLLTO_MIDDLE:
        dy += ((it->item->h / 2) - (oh / 2));
        break;

      case ELM_GENLIST_ITEM_SCROLLTO_IN:
      default:
        if ((sd->expanded_item) &&
            ((sd->show_item->y + sd->show_item->item->block->y
              + sd->show_item->item->h) -
             (sd->expanded_item->y + sd->expanded_item->item->block->y) > oh))
          {
             it = sd->expanded_item;
             if (it->item->group_item) gith = it->item->group_item->item->h;
             dx = it->x + it->item->block->x;
             dy = it->y + it->item->block->y - gith;
             dw = it->item->block->w;
          }
        else
          {
             if ((it->item->group_item) &&
                 (sd->pan_y > (it->y + it->item->block->y)))
               gith = it->item->group_item->item->h;
             dy -= gith;
             dh = it->item->h;
          }
        break;
     }
   if (sd->bring_in)
     sd->s_iface->region_bring_in(ELM_WIDGET_DATA(sd)->obj, dx, dy, dw, dh);
   else
     sd->s_iface->content_region_show
       (ELM_WIDGET_DATA(sd)->obj, dx, dy, dw, dh);

   it->item->show_me = EINA_FALSE;
   sd->show_item = NULL;
   sd->auto_scroll_enabled = EINA_FALSE;
   sd->check_scroll = EINA_FALSE;
}

static void
_item_unrealize(Elm_Gen_Item *it,
                Eina_Bool calc)
{
   Evas_Object *content;
   Eina_List *l;
   const char *part;

   if (!it->realized) return;
   if (GL_IT(it)->wsd->reorder_it == it) return;

#if GENLIST_FX_SUPPORT
   if ((GL_IT(it)->has_proxy_it) && (!GL_IT(it)->wsd->genlist_clearing))
     {
        if (!eina_list_data_find(GL_IT(it)->wsd->pending_unrealized_items, it))
          GL_IT(it)->wsd->pending_unrealized_items =
             eina_list_append(GL_IT(it)->wsd->pending_unrealized_items, it);
        return;
     }
#endif
#if GENLIST_ENTRY_SUPPORT
   if (it->item->unrealize_disabled)
     {
        evas_object_hide(VIEW(it));
        return;
     }
#endif

   //evas_event_freeze(evas_object_evas_get(WIDGET(it)));
   if (!calc)
     evas_object_smart_callback_call(WIDGET(it), SIG_UNREALIZED, it);
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->item->swipe_timer)
     {
        ecore_timer_del(it->item->swipe_timer);
        it->item->swipe_timer = NULL;
     }

   //Forcing the edje signal process for item's content.
   EINA_LIST_FOREACH(it->content_objs, l, content)
     {
        if (evas_object_smart_type_check(content, "elm_layout"))
          edje_object_message_signal_process(elm_layout_edje_get(content));
     }

   // Free view item
   EINA_LIST_FOREACH(it->texts, l, part)
     edje_object_part_text_set(VIEW(it), part, NULL);

   elm_widget_stringlist_free(it->texts);
   it->texts = NULL;
   elm_widget_stringlist_free(it->contents);
   it->contents = NULL;
   elm_widget_stringlist_free(it->states);
   it->states = NULL;
   EINA_LIST_FREE(it->content_objs, content)
     {
        // FIXME: If parent-child relationship was broken before 'ic'
        // is deleted, freeze_pop will not be called. ex) elm_slider
        // If layout is used instead of edje, this problme can be
        // solved.
        if (0 != elm_widget_scroll_freeze_get(content))
          elm_widget_scroll_freeze_pop(content);
        evas_object_del(content);
     }

   // Free flip item
   elm_widget_stringlist_free(it->item->flip_contents);
   it->item->flip_contents = NULL;
   EINA_LIST_FREE(it->item->flip_content_objs, content)
     evas_object_del(content);

   /* access */
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _elm_access_widget_item_unregister((Elm_Widget_Item *)it);

   // unswallow VIEW(it) first then manipulate VIEW(it)
   _decorate_item_unrealize(it);
   _decorate_all_item_unrealize(it);

   edje_object_signal_emit
     (VIEW(it), "elm,state,decorate,disabled", "elm");
   edje_object_signal_emit
     (VIEW(it), "elm,state,reorder,mode_unset", "elm");
   edje_object_message_signal_process(VIEW(it));

   it->realized = EINA_FALSE;
   _item_cache_push(it);

#if GENLIST_ENTRY_SUPPORT
   it->item->unrealize_disabled = EINA_FALSE;
#endif
   it->want_unrealize = EINA_FALSE;

   //evas_event_thaw(evas_object_evas_get(WIDGET(it)));
   //evas_event_thaw_eval(evas_object_evas_get(WIDGET(it)));
}

static void
_item_block_unrealize(Item_Block *itb)
{
   Elm_Gen_Item *it;
   const Eina_List *l;
   Eina_Bool dragging = EINA_FALSE;

   if (!itb->realized) return;
   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(itb->sd)->obj));

   EINA_LIST_FOREACH(itb->items, l, it)
     {
        if (itb->must_recalc || !it->group)
          {
             if (it->dragging)
               {
                  dragging = EINA_TRUE;
                  it->want_unrealize = EINA_TRUE;
               }
             else
               _item_unrealize(it, EINA_FALSE);
          }
     }
   if (!dragging)
     {
        itb->realized = EINA_FALSE;
        itb->want_unrealize = EINA_TRUE;
     }
   else
     itb->want_unrealize = EINA_FALSE;
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(itb->sd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(itb->sd)->obj));
}

static Eina_Bool
_must_recalc_idler(void *data)
{
   Elm_Genlist_Smart_Data *sd = data;

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);
   sd->must_recalc_idler = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_calc_job(void *data)
{
   int in = 0;
   Item_Block *itb, *chb = NULL;
   Evas_Coord pan_w = 0, pan_h = 0;
   Elm_Genlist_Smart_Data *sd = data;
   Eina_Bool minw_change = EINA_FALSE;
   Eina_Bool did_must_recalc = EINA_FALSE;
   Evas_Coord minw = -1, minh = 0, y = 0, dy = 0, vw = 0;

   sd->s_iface->content_viewport_size_get(ELM_WIDGET_DATA(sd)->obj, &sd->w, &sd->h);

   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        Eina_Bool show_me = EINA_FALSE;

        itb->num = in;
        show_me = itb->show_me;
        itb->show_me = EINA_FALSE;
        if (chb)
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
        if ((itb->changed) || ((itb->must_recalc) && (!did_must_recalc)))
          {
             if (itb->must_recalc)
               {
                  Eina_List *l;
                  Elm_Gen_Item *it;

                  EINA_LIST_FOREACH(itb->items, l, it)
                    if (it->item->mincalcd) it->item->mincalcd = EINA_FALSE;
                  itb->changed = EINA_TRUE;
                  did_must_recalc = EINA_TRUE;
                  if (itb->realized) _item_block_unrealize(itb);
                  itb->must_recalc = EINA_FALSE;
               }
             show_me = _item_block_recalc(itb, in, EINA_FALSE);
             chb = itb;
          }
        itb->y = y;
        itb->x = 0;
        minh += itb->minh;
        if (minw == -1) minw = itb->minw;
        else if ((!itb->must_recalc) && (minw < itb->minw))
          {
             minw = itb->minw;
             minw_change = EINA_TRUE;
          }
        if ((sd->mode == ELM_LIST_COMPRESS) && (minw > vw))
          {
             minw = vw;
             minw_change = EINA_TRUE;
          }
        itb->w = minw;
        itb->h = itb->minh;
        y += itb->h;
        in += itb->count;
        if ((show_me) && (sd->show_item) && (!sd->show_item->item->queued))
          sd->check_scroll = EINA_TRUE;
     }
   if (minw_change)
     {
        EINA_INLIST_FOREACH(sd->blocks, itb)
          {
             itb->minw = minw;
             itb->w = itb->minw;
          }
     }
   if ((chb) && (EINA_INLIST_GET(chb)->next))
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(chb)->next, itb)
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
     }
   sd->realminw = minw;
   if (minw < sd->w) minw = sd->w;
   if ((minw != sd->minw) || (minh != sd->minh))
     {
        sd->minw = minw;
        sd->minh = minh;
        evas_object_smart_callback_call(sd->pan_obj, "changed", NULL);
        elm_layout_sizing_eval(ELM_WIDGET_DATA(sd)->obj);
        if (sd->reorder_it)
          {
              Elm_Gen_Item *it;
              it = sd->reorder_it;
              it->item->w = minw;
          }
        if ((sd->anchor_item) && (sd->anchor_item->item->block)
            && (!sd->auto_scroll_enabled))
          {
             Elm_Gen_Item *it;
             Evas_Coord it_y;

             it = sd->anchor_item;
             it_y = sd->anchor_y;
             sd->s_iface->content_pos_set
               (ELM_WIDGET_DATA(sd)->obj, sd->pan_x, it->item->block->y
               + it->y + it_y, EINA_TRUE);
             sd->anchor_item = it;
             sd->anchor_y = it_y;
          }
     }
   if (did_must_recalc)
     {
        if (!sd->must_recalc_idler)
          sd->must_recalc_idler = ecore_idler_add(_must_recalc_idler, sd);
     }
   if (sd->check_scroll)
     {
        _elm_genlist_pan_smart_content_size_get(sd->pan_obj, &pan_w, &pan_h);
        if (EINA_INLIST_GET(sd->show_item) == sd->items->last)
          sd->scroll_to_type = ELM_GENLIST_ITEM_SCROLLTO_IN;

        switch (sd->scroll_to_type)
          {
           case ELM_GENLIST_ITEM_SCROLLTO_TOP:
             dy = sd->h;
             break;

           case ELM_GENLIST_ITEM_SCROLLTO_MIDDLE:
             dy = sd->h / 2;
             break;

           case ELM_GENLIST_ITEM_SCROLLTO_IN:
           default:
             dy = 0;
             break;
          }
        if ((sd->show_item) && (sd->show_item->item->block))
          {
             if ((pan_w > (sd->show_item->x + sd->show_item->item->block->x))
                 && (pan_h > (sd->show_item->y + sd->show_item->item->block->y
                              + dy)))
               {
                  _item_scroll(sd);
               }
          }
     }

   sd->calc_job = NULL;
   evas_object_smart_changed(sd->pan_obj);
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
}

static void
_elm_genlist_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = 0, minh = 0, maxw = -1, maxh = -1, vw = 0;

   ELM_GENLIST_DATA_GET(obj, sd);

   /* parent class' early call */
   if (!sd->s_iface) return;
   if (sd->on_sub_del) return;;

   evas_object_size_hint_min_get(obj, &minw, NULL);
   evas_object_size_hint_max_get(obj, &maxw, &maxh);
   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);

   sd->s_iface->content_viewport_size_get(obj, &vw, NULL);
   if (vw != 0) sd->prev_viewport_w = vw;

   if (sd->mode == ELM_LIST_LIMIT)
     {
        maxw = -1;
        minw = minw + sd->realminw;
     }

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_item_position(Elm_Gen_Item *it,
               Evas_Object *view,
               Evas_Coord it_x,
               Evas_Coord it_y)
{
   if (!it) return;
   if (!view) return;

   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(GL_IT(it)->wsd)->obj));
   evas_object_resize(view, it->item->w, it->item->h);
   evas_object_move(view, it_x, it_y);
   evas_object_show(view);
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(GL_IT(it)->wsd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(GL_IT(it)->wsd)->obj));
}

static void
_item_sub_items_clear(Elm_Gen_Item *it)
{
   Eina_List *l, *ll;
   Elm_Object_Item *it2;

   EINA_LIST_FOREACH_SAFE(it->item->items, l, ll, it2)
     elm_widget_item_del(it2);
}

static void
_item_auto_scroll(Elm_Genlist_Smart_Data *sd)
{
   Elm_Gen_Item *tmp_item = NULL;

#if GENLIST_FX_SUPPORT
   if (sd->fx_first_captured) return;
#endif
   if ((sd->expanded_item) && (sd->auto_scroll_enabled))
     {
        tmp_item = eina_list_data_get
            (eina_list_last(sd->expanded_item->item->items));
        if (!tmp_item) return;
        sd->show_item = tmp_item;
        sd->bring_in = EINA_TRUE;
        sd->scroll_to_type = ELM_GENLIST_ITEM_SCROLLTO_IN;
        if ((sd->show_item->item->queued) || (!sd->show_item->item->mincalcd))
          {
             sd->show_item->item->show_me = EINA_TRUE;
             sd->auto_scroll_enabled = EINA_FALSE;
          }
        else
          _item_scroll(sd);
     }
}

static void
_elm_genlist_item_odd_even_update(Elm_Gen_Item *it)
{
   if (!it->item->nostacking)
     {
        if ((it->item->order_num_in & 0x1) ^ it->item->stacking_even)
          {
             if (it->deco_all_view) evas_object_lower(it->deco_all_view);
             else evas_object_lower(VIEW(it));
          }
        else
          {
             if (it->deco_all_view) evas_object_raise(it->deco_all_view);
             else evas_object_raise(VIEW(it));
          }
     }

   if (it->item->order_num_in & 0x1)
     {
        edje_object_signal_emit(VIEW(it), "elm,state,odd", "elm");
        if (it->deco_all_view)
          edje_object_signal_emit(it->deco_all_view, "elm,state,odd", "elm");
     }
   else
     {
        edje_object_signal_emit(VIEW(it), "elm,state,even", "elm");
        if (it->deco_all_view)
          edje_object_signal_emit(it->deco_all_view, "elm,state,even", "elm");
     }
}

static void
_item_order_update(const Eina_Inlist *l,
                   int start)
{
   Elm_Gen_Item *it, *it2;

   for (it = ELM_GEN_ITEM_FROM_INLIST(l); l; l = l->next,
        it = ELM_GEN_ITEM_FROM_INLIST(l))
     {
        it->item->order_num_in = start++;
        _elm_genlist_item_odd_even_update(it);
        it2 = ELM_GEN_ITEM_FROM_INLIST(l->next);
        if (it2 && (it->item->order_num_in != it2->item->order_num_in))
          return;
     }
}

static void
_elm_genlist_item_state_update(Elm_Gen_Item *it)
{
   if (it->selected)
     {
        if (it->deco_all_view)
           edje_object_signal_emit
              (it->deco_all_view, "elm,state,selected", "elm");
        edje_object_signal_emit
           (VIEW(it), "elm,state,selected", "elm");
        evas_object_smart_callback_call(WIDGET(it), SIG_HIGHLIGHTED, it);
     }
   if (elm_widget_item_disabled_get(it))
     {
        edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        if (it->deco_all_view)
           edje_object_signal_emit
              (it->deco_all_view, "elm,state,disabled", "elm");
     }
   if (it->item->expanded)
     {
        edje_object_signal_emit(VIEW(it), "elm,state,expanded", "elm");
        if (it->deco_all_view)
           edje_object_signal_emit
              (it->deco_all_view, "elm,state,expanded", "elm");
     }
}

static void
_elm_genlist_item_index_update(Elm_Gen_Item *it)
{
   if (it->position_update || it->item->block->position_update)
     {
        evas_object_smart_callback_call(WIDGET(it), SIG_INDEX_UPDATE, it);
        it->position_update = EINA_FALSE;
     }
}

static void
_item_text_realize(Elm_Gen_Item *it,
                   Evas_Object *target,
                   Eina_List **source,
                   const char *parts)
{
   if (it->itc->func.text_get)
     {
        const Eina_List *l;
        const char *key;

        if (!(*source))
          *source = elm_widget_stringlist_get
              (edje_object_data_get(target, "texts"));
        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             char *s = it->itc->func.text_get
                 ((void *)it->base.data, WIDGET(it), key);

             if (s)
               {
                  edje_object_part_text_escaped_set(target, key, s);
                  free(s);
               }
             else
               {
                  edje_object_part_text_set(target, key, "");
               }
          }
     }
}

#if 0
// FIXME: difference from upstream
static void
_content_focused(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Gen_Item *it = data;
   if (it) it->defer_unrealize = EINA_TRUE;
}

static void
_content_unfocused(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Gen_Item *it = data;
   if (it) it->defer_unrealize = EINA_FALSE;
}
#endif

static Eina_List *
_item_mode_content_realize(Elm_Gen_Item *it,
                           Evas_Object *target,
                           Eina_List **source,
                           const char *parts,
                           Eina_List **contents_list)
{
   Eina_List *res = *contents_list;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;
        Evas_Object *ic;

        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             ic = it->itc->func.content_get
                 ((void *)it->base.data, WIDGET(it), key);

             if (ic)
               {
                  res = eina_list_append(res, ic);
                  edje_object_part_swallow(target, key, ic);
                  evas_object_show(ic);
#if GENLIST_ENTRY_SUPPORT
                  if (it->flipped)
                    {
                       // For entry or editfield
                       // 1. Add resize callback for multiline entry.
                       // 2. Do not unrealize it for focus issue
                       // ps. Only for entry because of performnace
                       if (!strcmp("elm_entry", evas_object_type_get(ic)))
                         it->item->unrealize_disabled = EINA_TRUE;
                    }
#endif
#if 0
                  // FIXME: difference from upstream
                  if (it->flipped)
                    {
                       // FIXME: if entry calcuates its cursor position correctly and conformant works,
                       //        genlist does not need to handle this focus thing.
                       evas_object_smart_callback_add(ic, "focused", _content_focused, it);
                       evas_object_smart_callback_add(ic, "unfocused", _content_unfocused, it);
                    }
#endif
                  if (elm_widget_item_disabled_get(it))
                    elm_widget_disabled_set(ic, EINA_TRUE);
               }
          }
     }

   return res;
}

static void
_item_state_realize(Elm_Gen_Item *it,
                    Evas_Object *target,
                    Eina_List **source,
                    const char *parts)
{
   if (it->itc->func.state_get)
     {
        const Eina_List *l;
        const char *key;
        char buf[4096];

        if (!(*source))
          *source = elm_widget_stringlist_get
              (edje_object_data_get(target, "states"));
        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             Eina_Bool on = it->itc->func.state_get
                 ((void *)it->base.data, WIDGET(it), key);

             if (on)
               {
                  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
                  edje_object_signal_emit(target, buf, "elm");
               }
             else
               {
                  snprintf(buf, sizeof(buf), "elm,state,%s,passive", key);
                  edje_object_signal_emit(target, buf, "elm");
               }
          }
     }
}

static void
_decorate_item_unrealize(Elm_Gen_Item *it)
{
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   Evas_Object *content;

   if (!it->item->deco_it_view) return;

   //evas_event_freeze(evas_object_evas_get(obj));
   elm_widget_stringlist_free(it->item->deco_it_texts);
   it->item->deco_it_texts = NULL;
   elm_widget_stringlist_free(it->item->deco_it_contents);
   it->item->deco_it_contents = NULL;
   elm_widget_stringlist_free(it->item->deco_it_states);

   EINA_LIST_FREE (it->item->deco_it_content_objs, content)
     evas_object_del(content);

   edje_object_part_unswallow(it->item->deco_it_view, VIEW(it));
   evas_object_smart_member_add(VIEW(it), sd->pan_obj);
   evas_object_del(it->item->deco_it_view);
   it->item->deco_it_view = NULL;

   if (sd->mode_item == it) sd->mode_item = NULL;
   //evas_event_thaw(evas_object_evas_get(obj));
   //evas_event_thaw_eval(evas_object_evas_get(obj));
}


static void
_decorate_all_item_position(Elm_Gen_Item *it,
                            int itx,
                            int ity)
{
   evas_object_resize(it->deco_all_view, it->item->w, it->item->h);
   evas_object_move(it->deco_all_view, itx, ity);
}

static void
_decorate_all_item_realize(Elm_Gen_Item *it,
                           Eina_Bool effect_on)
{
   char buf[1024];
   const char *stacking;
   const char *stacking_even;

   if ((!it) || (it->item->decorate_all_item_realized)) return;

   if (it->item->deco_it_view) _decorate_item_unrealize(it);

   it->deco_all_view = edje_object_add(evas_object_evas_get(WIDGET(it)));
   edje_object_scale_set(it->deco_all_view, elm_widget_scale_get(WIDGET(it)) *
                         elm_config_scale_get());
   evas_object_smart_member_add(it->deco_all_view, GL_IT(it)->wsd->pan_obj);
   elm_widget_sub_object_add(WIDGET(it), it->deco_all_view);

   if (it->item->type & ELM_GENLIST_ITEM_TREE)
     strncpy(buf, "tree", sizeof(buf));
   else strncpy(buf, "item", sizeof(buf));
   if (GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS)
     strncat(buf, "_compress", sizeof(buf) - strlen(buf));

   strncat(buf, "/", sizeof(buf) - strlen(buf));
   strncat(buf, it->itc->decorate_all_item_style, sizeof(buf) - strlen(buf));

   elm_widget_theme_object_set(WIDGET(it), it->deco_all_view, "genlist", buf,
                               elm_widget_style_get(WIDGET(it)));

   stacking_even = edje_object_data_get(VIEW(it), "stacking_even");
   if (!stacking_even) stacking_even = "above";
   it->item->stacking_even = !!strcmp("above", stacking_even);

   stacking = edje_object_data_get(VIEW(it), "stacking");
   if (!stacking) stacking = "yes";
   it->item->nostacking = !!strcmp("yes", stacking);

   edje_object_mirrored_set
     (it->deco_all_view, elm_widget_mirrored_get(WIDGET(it)));

   _elm_genlist_item_odd_even_update(it);
   _elm_genlist_item_state_update(it);

#if 1 // FIXME: difference from upstream
   if (GL_IT(it)->wsd->reorder_mode)
     edje_object_signal_emit
       (it->deco_all_view, "elm,state,reorder,mode_set", "elm");
   else
     edje_object_signal_emit
       (it->deco_all_view, "elm,state,reorder,mode_unset", "elm");
#endif
   if (effect_on)
     {
        edje_object_signal_emit
           (it->deco_all_view, "elm,state,decorate,enabled,effect", "elm");
        edje_object_signal_emit
           (VIEW(it), "elm,state,decorate,enabled,effect", "elm");
     }
   else
     {
        edje_object_signal_emit
           (it->deco_all_view, "elm,state,decorate,enabled", "elm");
        edje_object_signal_emit
           (VIEW(it), "elm,state,decorate,enabled", "elm");
     }
   _item_mouse_callbacks_del(it, VIEW(it));
   _item_mouse_callbacks_add(it, it->deco_all_view);

   _item_text_realize(it, it->deco_all_view, &it->item->deco_all_texts, NULL);
   if (it->flipped)
     edje_object_signal_emit
       (it->deco_all_view, "elm,state,flip,enabled", "elm");
   if (!it->item->deco_all_contents)
     it->item->deco_all_contents = elm_widget_stringlist_get
         (edje_object_data_get(it->deco_all_view, "contents"));
   it->item->deco_all_content_objs =
     _item_mode_content_realize(it, it->deco_all_view,
                                &it->item->deco_all_contents, NULL,
                                &it->item->deco_all_content_objs);
   _item_state_realize
     (it, it->deco_all_view, &it->item->deco_all_states, NULL);
   edje_object_part_swallow
     (it->deco_all_view, "elm.swallow.decorate.content", VIEW(it));

   _decorate_all_item_position(it, it->item->scrl_x, it->item->scrl_y);
   evas_object_show(it->deco_all_view);

   if (it->selected)
     {
        edje_object_signal_emit(it->deco_all_view, "elm,state,selected", "elm");
        evas_object_smart_callback_call(WIDGET(it), SIG_HIGHLIGHTED, it);
     }

   it->item->decorate_all_item_realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;
}

static void
_expand_toggle_signal_cb(void *data,
                         Evas_Object *obj __UNUSED__,
                         const char *emission __UNUSED__,
                         const char *source __UNUSED__)
{
   Elm_Gen_Item *it = data;

   if (it->item->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_CONTRACT_REQUEST, it);
   else
     evas_object_smart_callback_call(WIDGET(it), SIG_EXPAND_REQUEST, it);
}

static void
_expand_signal_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  const char *emission __UNUSED__,
                  const char *source __UNUSED__)
{
   Elm_Gen_Item *it = data;

   if (!it->item->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_EXPAND_REQUEST, it);
}

static void
_contract_signal_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    const char *emission __UNUSED__,
                    const char *source __UNUSED__)
{
   Elm_Gen_Item *it = data;

   if (it->item->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_CONTRACT_REQUEST, it);
}

#if GENLIST_ENTRY_SUPPORT
static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Gen_Item *it = data;
   if (!it) return;
   if (it->want_unrealize) return;
   GL_IT(it)->wsd->size_changed = EINA_TRUE;
   evas_object_smart_changed(GL_IT(it)->wsd->pan_obj);
}
#endif

static Eina_List *
_item_content_realize(Elm_Gen_Item *it,
                      Evas_Object *target,
                      Eina_List **source,
                      const char *parts)
{
   Eina_List *res = it->content_objs;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;
        Evas_Object *ic = NULL;

        if (!(*source))
          *source = elm_widget_stringlist_get
              (edje_object_data_get(target, "contents"));

        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             if (it->itc->func.content_get)
               ic = it->itc->func.content_get
                   ((void *)it->base.data, WIDGET(it), key);
             if (ic)
               {
                  res = eina_list_append(res, ic);
                  edje_object_part_swallow(target, key, ic);
                  evas_object_show(ic);

#if GENLIST_ENTRY_SUPPORT
                  // For entry or editfield
                  // 1. Add resize callback for multiline entry.
                  // 2. Do not unrealize it for focus issue
                  // ps. Only for entry because of performnace
                  if (!strcmp("elm_entry", evas_object_type_get(ic)))
                    {
                       if (!elm_entry_single_line_get(ic))
                         evas_object_event_callback_add
                            (ic, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                             _changed_size_hints, it);
                       it->item->unrealize_disabled = EINA_TRUE;
                    }
#endif
#if 0 // FIXME: difference from upstream
                  if (it->flipped)
                    {
                       // FIXME: if entry calcuates its cursor position correctly and conformant works,
                       //        genlist does not need to handle this focus thing.
                       evas_object_smart_callback_add(ic, "focused", _content_focused, it);
                       evas_object_smart_callback_add(ic, "unfocused", _content_unfocused, it);
                    }
#endif
                  if (elm_widget_item_disabled_get(it))
                    elm_widget_disabled_set(ic, EINA_TRUE);
               }
          }
     }

   return res;
}

static char *
_access_info_cb(void *data, Evas_Object *obj __UNUSED__)
{
   char *ret;
   Eina_Strbuf *buf;

   Elm_Gen_Item *it = (Elm_Gen_Item *)data;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(it, NULL);

   buf = eina_strbuf_new();

   if (it->itc->func.text_get)
     {
        const Eina_List *l;
        const char *key;

        if (!(it->texts)) it->texts =
          elm_widget_stringlist_get(edje_object_data_get(VIEW(it), "texts"));

        EINA_LIST_FOREACH(it->texts, l, key)
          {
             char *s = it->itc->func.text_get
                ((void *)it->base.data, WIDGET(it), key);

             s = _elm_util_mkup_to_text(s);

             if (s)
               {
                  if (eina_strbuf_length_get(buf) > 0) eina_strbuf_append(buf, ", ");
                  eina_strbuf_append(buf, s);
                  free(s);
               }
          }
     }

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static char *
_access_state_cb(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)data;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(it, NULL);

   if (it->base.disabled)
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_on_highlight_cb(void *data)
{
   Evas_Coord x, y, w, h;
   Evas_Coord sx, sy, sw, sh;
   Elm_Gen_Item *it = (Elm_Gen_Item *)data;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(it);

   ELM_GENLIST_DATA_GET(it->base.widget, sd);

   evas_object_geometry_get(it->base.view, &x, &y, &w, &h);
   // XXX There would be a reason.
   if ((w == 0) && (h == 0)) return;

   evas_object_geometry_get(ELM_WIDGET_DATA(sd)->obj, &sx, &sy, &sw, &sh);
   if ((x < sx) || (y < sy) || ((x + w) > (sx + sw)) || ((y + h) > (sy + sh)))
     elm_genlist_item_bring_in((Elm_Object_Item *)it,
                               ELM_GENLIST_ITEM_SCROLLTO_IN);
}

static void
_access_widget_item_register(Elm_Gen_Item *it)
{
   Elm_Access_Info *ai;

   _elm_access_widget_item_register((Elm_Widget_Item *)it);

   ai = _elm_access_object_get(it->base.access_obj);

   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);
   _elm_access_on_highlight_hook_set(ai, _access_on_highlight_cb, it);
   _elm_access_activate_callback_set(ai, _access_activate_cb, it);
}

static void
_item_realize(Elm_Gen_Item *it,
              int in,
              Eina_Bool calc)
{
   const char *treesize;
   char buf[1024];
   int tsize = 20;

   if (it->realized)
     {
        if (it->item->order_num_in != in)
          {
             _item_order_update(EINA_INLIST_GET(it), in);
             _elm_genlist_item_state_update(it);
             _elm_genlist_item_index_update(it);
          }
        return;
     }
   it->item->order_num_in = in;

   if (it->item->nocache_once | it->item->nocache ||
       it->item->type == ELM_GENLIST_ITEM_TREE || it->parent ||
       !_item_cache_pop(it))
     {
        const char *stacking_even;
        const char *stacking;

        it->item->nocache_once = EINA_FALSE;

        VIEW(it) = edje_object_add(evas_object_evas_get(WIDGET(it)));
        edje_object_scale_set(VIEW(it),
                              elm_widget_scale_get(WIDGET(it)) *
                              elm_config_scale_get());
        evas_object_smart_member_add(VIEW(it), GL_IT(it)->wsd->pan_obj);
        elm_widget_sub_object_add(WIDGET(it), VIEW(it));

        if (it->item->type & ELM_GENLIST_ITEM_TREE)
          snprintf(buf, sizeof(buf), "tree%s/%s",
                   GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS ? "_compress" :
                   "", it->itc->item_style ? : "default");
        else
          snprintf(buf, sizeof(buf), "item%s/%s",
                   GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS ? "_compress" :
                   "", it->itc->item_style ? : "default");

        elm_widget_theme_object_set
          (WIDGET(it), VIEW(it), "genlist", buf,
          elm_widget_style_get(WIDGET(it)));

#if 1 // FIXME: difference from upstream
        if (it->item->expanded_depth > 0)
          edje_object_signal_emit(VIEW(it), "bg_color_change", "elm");
#endif

        stacking_even = edje_object_data_get(VIEW(it), "stacking_even");
        if (!stacking_even) stacking_even = "above";
        it->item->stacking_even = !!strcmp("above", stacking_even);

        stacking = edje_object_data_get(VIEW(it), "stacking");
        if (!stacking) stacking = "yes";
        it->item->nostacking = !!strcmp("yes", stacking);

        edje_object_mirrored_set
          (VIEW(it), elm_widget_mirrored_get(WIDGET(it)));

        edje_object_signal_callback_add
          (VIEW(it), "elm,action,expand,toggle", "elm",
          _expand_toggle_signal_cb, it);
        edje_object_signal_callback_add
          (VIEW(it), "elm,action,expand", "elm", _expand_signal_cb, it);
        edje_object_signal_callback_add
          (VIEW(it), "elm,action,contract", "elm", _contract_signal_cb, it);
        _item_mouse_callbacks_add(it, VIEW(it));
     }

   /* access */
   if (_elm_config->access_mode) _access_widget_item_register(it);

   _item_order_update(EINA_INLIST_GET(it), in);
#if 1 // FIXME: difference from upstream
   if (it->item->type != ELM_GENLIST_ITEM_GROUP)
     {
        if (GL_IT(it)->wsd->reorder_mode)
          edje_object_signal_emit
            (VIEW(it), "elm,state,reorder,mode_set", "elm");
        else
          edje_object_signal_emit
            (VIEW(it), "elm,state,reorder,mode_unset", "elm");
    }
#endif
   treesize = edje_object_data_get(VIEW(it), "treesize");
   if (treesize) tsize = atoi(treesize);
   if (it->parent && GL_IT(it->parent)->type == ELM_GENLIST_ITEM_TREE &&
       !it->spacer && treesize)
     {
        it->spacer =
          evas_object_rectangle_add(evas_object_evas_get(WIDGET(it)));
        evas_object_color_set(it->spacer, 0, 0, 0, 0);
     }
   if (it->spacer)
     {
        evas_object_size_hint_min_set
          (it->spacer, (it->item->expanded_depth * tsize) *
          elm_config_scale_get(), 1);
        edje_object_part_swallow(VIEW(it), "elm.swallow.pad", it->spacer);
     }
   if (!calc)
     {
        if ((GL_IT(it)->wsd->decorate_all_mode) && (!it->deco_all_view) &&
            (it->item->type != ELM_GENLIST_ITEM_GROUP) &&
            (it->itc->decorate_all_item_style))
          _decorate_all_item_realize(it, EINA_FALSE);

        _elm_genlist_item_state_update(it);
        _elm_genlist_item_index_update(it);
     }

   /* homogenous genlist shortcut */
   if ((calc) && (GL_IT(it)->wsd->homogeneous) && (!it->item->mincalcd) &&
       ((it->group && GL_IT(it)->wsd->group_item_width) ||
        (!it->group && GL_IT(it)->wsd->item_width)))
     {
        if (it->group)
          {
             it->item->w = it->item->minw = GL_IT(it)->wsd->group_item_width;
             it->item->h = it->item->minh = GL_IT(it)->wsd->group_item_height;
          }
        else
          {
             it->item->w = it->item->minw = GL_IT(it)->wsd->item_width;
             it->item->h = it->item->minh = GL_IT(it)->wsd->item_height;
          }
        it->item->mincalcd = EINA_TRUE;
     }
   else
     {
        if (eina_list_count(it->content_objs) != 0)
          ERR_ABORT("If you see this error, please notify us and we"
                    "will fix it");

        _item_text_realize(it, VIEW(it), &it->texts, NULL);
        it->content_objs =
          _item_content_realize(it, VIEW(it), &it->contents, NULL);
        _item_state_realize(it, VIEW(it), &it->states, NULL);
        if (it->flipped)
          {
             edje_object_signal_emit
               (VIEW(it), "elm,state,flip,enabled", "elm");
             // This is needed before contents are swallowed
             edje_object_message_signal_process(VIEW(it));

             if (!(it->item->flip_contents))
               it->item->flip_contents = elm_widget_stringlist_get
                   (edje_object_data_get(VIEW(it), "flips"));
             it->item->flip_content_objs = _item_mode_content_realize
                 (it, VIEW(it), &it->item->flip_contents, NULL,
                 &it->item->flip_content_objs);
          }

        /* access: unregister item which have no text and content */
        if (_elm_config->access_mode && !it->texts && !it->contents)
          _elm_access_widget_item_unregister((Elm_Widget_Item *)it);

#if 0
        // FIXME: difference from upstream
        if (!it->item->mincalcd || ((GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS) && (it->item->w != it->item->minw)))
#else
        if (!it->item->mincalcd)
#endif
          {
             Evas_Coord mw = 0, mh = 0;

             if (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
               elm_coords_finger_size_adjust(1, &mw, 1, &mh);
             if ((GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS) &&
                 (GL_IT(it)->wsd->prev_viewport_w != 0) &&
                 (mw < GL_IT(it)->wsd->prev_viewport_w))
                mw = GL_IT(it)->wsd->prev_viewport_w;
             edje_object_size_min_restricted_calc(VIEW(it), &mw, &mh, mw, mh);
             if ((GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS) &&
                 (GL_IT(it)->wsd->prev_viewport_w != 0) &&
                 (mw > GL_IT(it)->wsd->prev_viewport_w))
                mw = GL_IT(it)->wsd->prev_viewport_w;


             it->item->w = it->item->minw = mw;
             it->item->h = it->item->minh = mh;
             it->item->mincalcd = EINA_TRUE;

             if ((!GL_IT(it)->wsd->group_item_width) && (it->group))
               {
                  GL_IT(it)->wsd->group_item_width = mw;
                  GL_IT(it)->wsd->group_item_height = mh;
               }
             else if ((!GL_IT(it)->wsd->item_width) &&
                      (it->item->type == ELM_GENLIST_ITEM_NONE))
               {
                  GL_IT(it)->wsd->item_width = mw;
                  GL_IT(it)->wsd->item_height = mh;
               }
          }
        if (!calc) evas_object_show(VIEW(it));
     }

   if (it->tooltip.content_cb)
     {
        elm_widget_item_tooltip_content_cb_set
          (it, it->tooltip.content_cb, it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, it->tooltip.style);
        elm_widget_item_tooltip_window_mode_set(it, it->tooltip.free_size);
     }

   if (it->mouse_cursor)
     elm_widget_item_cursor_set(it, it->mouse_cursor);

   it->realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;

   if (!calc || it->item->unrealize_disabled)
     evas_object_smart_callback_call(WIDGET(it), SIG_REALIZED, it);

   if ((!calc) && (GL_IT(it)->wsd->decorate_all_mode) &&
       (it->item->type != ELM_GENLIST_ITEM_GROUP))
     {
        if (it->itc->decorate_all_item_style)
          {
             if (!it->deco_all_view)
               _decorate_all_item_realize(it, EINA_FALSE);
             edje_object_message_signal_process(it->deco_all_view);
          }
     }
   if ((!calc) && (it->decorate_it_set) &&
       (it->item->type != ELM_GENLIST_ITEM_GROUP))
     {
        if (it->itc->decorate_item_style)
          {
             if (!it->item->deco_it_view)
               _decorate_item_set(it);
             edje_object_message_signal_process(it->item->deco_it_view);
          }
     }
   edje_object_message_signal_process(VIEW(it));
}

#if GENLIST_PINCH_ZOOM_SUPPORT
static void
_pinch_items_recalc(void *data)
{
   Elm_Genlist_Smart_Data *sd = data;
   Item_Block *itb;
   Eina_List *l;
   Elm_Gen_Item *it;

   Evas_Coord tot_y = 0, ox, oy;
   evas_object_geometry_get(sd->pan_obj, &ox, &oy, NULL, NULL);

   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        EINA_LIST_FOREACH(itb->items, l, it)
          {

             if (IS_ROOT_PARENT_IT(it))
               {
                  if (sd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT)
                    {
                       if (!it->realized) _item_realize(it, 0, EINA_FALSE);
                       it->item->scrl_y = oy + tot_y - sd->pan_y;
                       evas_object_resize(VIEW(it), sd->minw, it->item->h);
                       evas_object_move(VIEW(it), it->item->scrl_x, it->item->scrl_y);
                       evas_object_show(VIEW(it));
                       evas_object_raise(VIEW(it));
                       tot_y += it->item->h;
                       sd->pinch_zoom_h = tot_y;
                    }

                  else if (sd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_EXPAND)
                    {
                       evas_object_resize(VIEW(it), sd->minw, it->item->h);
                       it->item->scrl_y = oy + it->item->pan_scrl_y - sd->pan_y;
                       evas_object_move(VIEW(it), it->item->scrl_x, it->item->scrl_y);
                    }

               }
          }
     }
}
#endif

#if GROUP_ITEMS_FEATURE
static void
_group_items_recalc(void *data)
{
   Eina_List *l;
   Elm_Gen_Item *git;
   Elm_Genlist_Smart_Data *sd = data;

   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   EINA_LIST_FOREACH(sd->group_items, l, git)
     {
        if (git->item->want_realize)
          {
             if (!git->realized) _item_realize(git, 0, EINA_FALSE);
             evas_object_resize(VIEW(git), sd->minw, git->item->h);
             evas_object_move(VIEW(git), git->item->scrl_x, git->item->scrl_y);
             evas_object_show(VIEW(git));
             evas_object_raise(VIEW(git));
          }
        else if (!git->item->want_realize)
          {
             if (!git->dragging)
               _item_unrealize(git, EINA_FALSE);
          }
     }
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
}
#endif

static Eina_Bool
_reorder_move_animator_cb(void *data)
{
   double t;
   Elm_Gen_Item *it = data;
   Eina_Bool down = EINA_FALSE;
   int y, dy, diff;

   dy = 20 * elm_config_scale_get();
   t = ((0.0 > (t = ecore_loop_time_get()
                  - GL_IT(it)->wsd->start_time)) ? 0.0 : t);

   if (t <= REORDER_EFFECT_TIME)
     y = (1 * sin((t / REORDER_EFFECT_TIME) * (M_PI / 2)) * dy);
   else y = dy;

   diff = abs(it->item->old_scrl_y - it->item->scrl_y);
   if (diff < dy) y = diff;

   if (it->item->old_scrl_y < it->item->scrl_y)
     {
        it->item->old_scrl_y += y;
        down = EINA_TRUE;
     }
   else if (it->item->old_scrl_y > it->item->scrl_y)
     {
        it->item->old_scrl_y -= y;
        down = EINA_FALSE;
     }

   if (it->deco_all_view)
     _item_position
       (it, it->deco_all_view, it->item->scrl_x, it->item->old_scrl_y);
   else if (it->item->deco_it_view)
      _item_position
         (it, it->item->deco_it_view, it->item->scrl_x, it->item->old_scrl_y);
   else
     _item_position(it, VIEW(it), it->item->scrl_x, it->item->old_scrl_y);

   if ((GL_IT(it)->wsd->reorder_pan_move) ||
       (down && it->item->old_scrl_y >= it->item->scrl_y) ||
       (!down && it->item->old_scrl_y <= it->item->scrl_y))
     {
        it->item->old_scrl_y = it->item->scrl_y;
        it->item->move_effect_enabled = EINA_FALSE;
        GL_IT(it)->wsd->reorder_move_animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static int
_reorder_item_space_get(Elm_Gen_Item *it)
{
   Elm_Gen_Item *reorder_it = GL_IT(it)->wsd->reorder_it;
   Evas_Coord rox, roy, row, roh, oy, oh;
   Eina_Bool top = EINA_FALSE;

   if (!reorder_it) return 0;
   if(it->deco_all_view) evas_object_lower(it->deco_all_view);

   evas_object_geometry_get(GL_IT(it)->wsd->pan_obj, NULL, &oy, NULL, &oh);
   evas_object_geometry_get
     (GL_IT(it)->wsd->VIEW(reorder_it), &rox, &roy, &row, &roh);

   if ((GL_IT(it)->wsd->reorder_start_y < it->item->block->y) &&
       (roy - oy + (roh / 2) >= it->item->block->y - GL_IT(it)->wsd->pan_y))
     {
        it->item->block->reorder_offset =
          GL_IT(it)->wsd->reorder_it->item->h * -1;
        if (it->item->block->count == 1)
          GL_IT(it)->wsd->reorder_rel = it;
     }
   else if ((GL_IT(it)->wsd->reorder_start_y >= it->item->block->y) &&
            (roy - oy + (roh / 2) <= it->item->block->y -
             GL_IT(it)->wsd->pan_y))
     {
        it->item->block->reorder_offset = GL_IT(it)->wsd->reorder_it->item->h;
     }
   else
     it->item->block->reorder_offset = 0;

   it->item->scrl_y += it->item->block->reorder_offset;

   top = (ELM_RECTS_INTERSECT
            (it->item->scrl_x, it->item->scrl_y, it->item->w, it->item->h,
            rox, roy + (roh / 2), row, 1));
   if (top)
     {
        GL_IT(it)->wsd->reorder_rel = it;
        it->item->scrl_y += GL_IT(it)->wsd->reorder_it->item->h;
        return GL_IT(it)->wsd->reorder_it->item->h;
     }
   else
     return 0;
}

static void
_item_block_position(Item_Block *itb,
                     int in)
{
   Elm_Gen_Item *it;
   Elm_Gen_Item *git;
   const Eina_List *l;
   Eina_Bool vis = EINA_FALSE;
   Evas_Coord y = 0, ox, oy, ow, oh, cvx, cvy, cvw, cvh;

   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(itb->sd)->obj));
   evas_object_geometry_get(itb->sd->pan_obj, &ox, &oy, &ow, &oh);
   evas_output_viewport_get
     (evas_object_evas_get(ELM_WIDGET_DATA(itb->sd)->obj),
     &cvx, &cvy, &cvw, &cvh);

   EINA_LIST_FOREACH(itb->items, l, it)
     {
        if ((GL_IT(it)->wsd->reorder_it == it) && it->highlighted) continue;

        it->x = 0;
        it->y = y;
        it->item->w = itb->w;
        it->item->scrl_x = itb->x + it->x - GL_IT(it)->wsd->pan_x + ox;
        it->item->scrl_y = itb->y + it->y - GL_IT(it)->wsd->pan_y + oy;

        vis = (ELM_RECTS_INTERSECT
                 (it->item->scrl_x, it->item->scrl_y, it->item->w, it->item->h,
                 cvx, cvy, cvw, cvh));
        if (1)
          {
             if ((itb->realized) && (!it->realized))
               {
                  if (vis) _item_realize(it, in, EINA_FALSE);
               }
             if (it->realized)
               {
                  if (vis || it->dragging)
                    {
#if GENLIST_ENTRY_SUPPORT
                       evas_object_show(VIEW(it));
#endif
                       if (GL_IT(it)->wsd->reorder_mode &&
                                GL_IT(it)->wsd->reorder_it &&
                                  GL_IT(it)->wsd->reorder_it->highlighted)
                         y += _reorder_item_space_get(it);
                       git = it->item->group_item;
                       if (git)
                         {
                            if (git->item->scrl_y < oy)
                              git->item->scrl_y = oy;
                            if ((git->item->scrl_y + git->item->h) >
                                (it->item->scrl_y + it->item->h))
                              git->item->scrl_y = (it->item->scrl_y +
                                                   it->item->h) - git->item->h;
                            git->item->scrl_x = it->item->scrl_x;
                            git->item->want_realize = EINA_TRUE;
                         }
                       if ((GL_IT(it)->wsd->reorder_it) &&
                                GL_IT(it)->wsd->reorder_it->highlighted &&
                                (it->item->old_scrl_y != it->item->scrl_y))
                         {
                            if (!it->item->move_effect_enabled)
                              {
                                 it->item->move_effect_enabled = EINA_TRUE;
                                 GL_IT(it)->wsd->reorder_move_animator =
                                   ecore_animator_add(
                                      _reorder_move_animator_cb, it);
                              }
                         }
                       if (!it->item->move_effect_enabled)
                         {
                            if ((GL_IT(it)->wsd->decorate_all_mode) &&
                                (it->itc->decorate_all_item_style))
                              _decorate_all_item_position
                                 (it, it->item->scrl_x, it->item->scrl_y);
                            else
                              {
                                 if (it->item->deco_it_view)
                                   _item_position
                                      (it, it->item->deco_it_view,
                                       it->item->scrl_x,
                                       it->item->scrl_y);
                                 else
                                   _item_position
                                      (it, VIEW(it), it->item->scrl_x,
                                       it->item->scrl_y);
                              }
                            it->item->old_scrl_y = it->item->scrl_y;
#if GENLIST_PINCH_ZOOM_SUPPORT
                            if (((GL_IT(it)->wsd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT)
                                 && (!IS_ROOT_PARENT_IT(it))) || (GL_IT(it)->wsd->sorting))
                              {
                                 if (it->deco_all_view) evas_object_hide(it->deco_all_view);
                                 else evas_object_hide(VIEW(it));
                              }
#endif

                         }
                    }
                  else
                    {
                       if ((GL_IT(it)->wsd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT)
                           && (!IS_ROOT_PARENT_IT(it)))
                         {
                            if (it->deco_all_view) evas_object_hide(it->deco_all_view);
                            else evas_object_hide(VIEW(it));
                         }
                       else
                         _item_unrealize(it, EINA_FALSE);
                    }
               }
             in++;
          }
        else
          {
             if (vis) it->item->want_realize = EINA_TRUE;
          }
        y += it->item->h;
#if 0
        // FIXME: diffrence from upstream
        minh += it->item->minh;
#endif

     }
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(itb->sd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(itb->sd)->obj));
}

static void
_item_block_realize(Item_Block *itb)
{
   if (itb->realized) return;

   itb->realized = EINA_TRUE;
   itb->want_unrealize = EINA_FALSE;
}

static void
_changed_job(Elm_Genlist_Smart_Data *sd)
{
   Eina_List *l2;

   Item_Block *itb;
   int num = 0, num0;
   Eina_Bool anything_changed = EINA_FALSE;
   Eina_Bool width_changed = EINA_FALSE;
   Eina_Bool height_changed = EINA_FALSE;

   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        Elm_Gen_Item *it;

        num0 = num;
        width_changed = height_changed = EINA_FALSE;
        EINA_LIST_FOREACH(itb->items, l2, it)
          {
             if (it->realized)
               {
                  Evas_Coord mw = 0, mh = 0;

                  if (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
                     elm_coords_finger_size_adjust(1, &mw, 1, &mh);
                  if ((GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS) &&
                      (GL_IT(it)->wsd->prev_viewport_w != 0) &&
                      (mw < GL_IT(it)->wsd->prev_viewport_w))
                     mw = GL_IT(it)->wsd->prev_viewport_w;
                  edje_object_size_min_restricted_calc(VIEW(it), &mw, &mh, mw, mh);
                  if ((GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS) &&
                      (GL_IT(it)->wsd->prev_viewport_w != 0) &&
                      (mw > GL_IT(it)->wsd->prev_viewport_w))
                     mw = GL_IT(it)->wsd->prev_viewport_w;

                  if (it->item->minw != mw)
                    {
                       it->item->w = it->item->minw = mw;
                       width_changed = EINA_TRUE;
                    }
                  if (it->item->minh != mh)
                    {
                       it->item->h = it->item->minh = mh;
                       height_changed = EINA_TRUE;
                    }
                  if (mw > itb->w)
                    {
                       itb->w = mw;
                    }

                  if ((!sd->group_item_width) && (it->group))
                    {
                       sd->group_item_width = mw;
                       sd->group_item_height = mh;
                    }
                  else if ((!sd->item_width) &&
                           (it->item->type == ELM_GENLIST_ITEM_NONE))
                    {
                       sd->item_width = mw;
                       sd->item_height = mh;
                    }
               }
             num++;
          }
        if (height_changed)
          {
             anything_changed = EINA_TRUE;
             itb->changed = EINA_TRUE;
             _item_block_recalc(itb, num0, 0);
             _item_block_position(itb, num0);
          }
        else if (width_changed)
          {
             _item_block_position(itb, num0);
          }
     }
   if (anything_changed)
     {
        if (sd->calc_job) ecore_job_del(sd->calc_job);
        sd->calc_job = ecore_job_add(_calc_job, sd);
     }
}

static void
_elm_genlist_pan_smart_calculate(Evas_Object *obj)
{
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   Evas_Coord vx = 0, vy = 0, vw = 0, vh = 0;
   Elm_Gen_Item *git;
   Item_Block *itb;
   Eina_List *l;
   int in = 0;

   ELM_GENLIST_PAN_DATA_GET(obj, psd);

   //evas_event_freeze(evas_object_evas_get(obj));

#if GENLIST_FX_SUPPORT
   if (psd->wsd->fx_playing) return;
#endif
   if (psd->wsd->pan_changed)
     {
        _calc_job(psd->wsd);
        psd->wsd->pan_changed = EINA_FALSE;
     }

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(obj), &cvx, &cvy, &cvw, &cvh);
   EINA_LIST_FOREACH(psd->wsd->group_items, l, git)
    {
#if GROUP_ITEMS_FEATURE
       git->item->want_realize = EINA_FALSE;
#else
       evas_object_hide(VIEW(git));
#endif
    }

   EINA_INLIST_FOREACH(psd->wsd->blocks, itb)
     {
        itb->w = psd->wsd->minw;
        if (ELM_RECTS_INTERSECT(itb->x - psd->wsd->pan_x + ox,
                                itb->y - psd->wsd->pan_y + oy,
                                itb->w, itb->h,
                                cvx, cvy, cvw, cvh))
          {
             if ((!itb->realized) || (itb->changed))
               {
                  _item_block_realize(itb);
#if 0
                  // FIXME: difference from opensource
                  if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
                  sd->wd->calc_job = NULL;
                  _calc_job(sd->wd);
#endif
               }
             _item_block_position(itb, in);
          }
        else
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
        in += itb->count;
     }
#if GENLIST_PINCH_ZOOM_SUPPORT
   if (psd->wsd->pinch_zoom_mode) _pinch_items_recalc(psd->wsd);
#endif

#if GROUP_ITEMS_FEATURE
   if ((!psd->wsd->reorder_it) || (psd->wsd->reorder_pan_move))
     _group_items_recalc(psd->wsd);
#endif
   if ((psd->wsd->reorder_mode) && (psd->wsd->reorder_it))
     {
        if (psd->wsd->pan_y != psd->wsd->reorder_old_pan_y)
          psd->wsd->reorder_pan_move = EINA_TRUE;
        else psd->wsd->reorder_pan_move = EINA_FALSE;

        evas_object_raise(psd->wsd->VIEW(reorder_it));
        psd->wsd->reorder_old_pan_y = psd->wsd->pan_y;
        psd->wsd->start_time = ecore_loop_time_get();
     }
     _item_auto_scroll(psd->wsd);

   psd->wsd->s_iface->content_pos_get(ELM_WIDGET_DATA(psd->wsd)->obj, &vx, &vy);
   psd->wsd->s_iface->content_viewport_size_get(ELM_WIDGET_DATA(psd->wsd)->obj,
                                                &vw, &vh);

   if (psd->wsd->reorder_fast == 1)
     psd->wsd->s_iface->content_region_show(ELM_WIDGET_DATA(psd->wsd)->obj,
                                            vx,vy - 10, vw, vh);
   else if (psd->wsd->reorder_fast == -1)
     psd->wsd->s_iface->content_region_show(ELM_WIDGET_DATA(psd->wsd)->obj,
                                            vx, vy + 10, vw, vh);

   if (psd->wsd->size_changed)
     {
        _changed_job(psd->wsd);
        psd->wsd->size_changed = EINA_FALSE;
     }

#if GENLIST_FX_SUPPORT
   psd->wsd->rendered = EINA_TRUE;
   if ((psd->wsd->fx_mode) && (!psd->wsd->queue))
     {
        if (_elm_genlist_fx_capture(ELM_WIDGET_DATA(psd->wsd)->obj, 1))
          _elm_genlist_fx_play(ELM_WIDGET_DATA(psd->wsd)->obj);
     }
#endif
#if GENLIST_PINCH_ZOOM_SUPPORT
   if (psd->wsd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_EXPAND)
     psd->wsd->pinch_zoom_mode = ELM_GEN_PINCH_ZOOM_NONE;
#endif
   //evas_event_thaw(evas_object_evas_get(obj));
   //evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_elm_genlist_pan_smart_set_user(Elm_Genlist_Pan_Smart_Class *sc)
{
   ELM_PAN_CLASS(sc)->base.add = _elm_genlist_pan_smart_add;
   ELM_PAN_CLASS(sc)->base.del = _elm_genlist_pan_smart_del;
   ELM_PAN_CLASS(sc)->base.move = _elm_genlist_pan_smart_move;
   ELM_PAN_CLASS(sc)->base.resize = _elm_genlist_pan_smart_resize;
   ELM_PAN_CLASS(sc)->base.calculate = _elm_genlist_pan_smart_calculate;

   ELM_PAN_CLASS(sc)->pos_set = _elm_genlist_pan_smart_pos_set;
   ELM_PAN_CLASS(sc)->pos_get = _elm_genlist_pan_smart_pos_get;
   ELM_PAN_CLASS(sc)->pos_max_get = _elm_genlist_pan_smart_pos_max_get;
   ELM_PAN_CLASS(sc)->pos_min_get = _elm_genlist_pan_smart_pos_min_get;
   ELM_PAN_CLASS(sc)->content_size_get =
     _elm_genlist_pan_smart_content_size_get;
}

static Eina_Bool
_item_multi_select_up(Elm_Genlist_Smart_Data *sd)
{
   Elm_Object_Item *prev;

   if (!sd->selected) return EINA_FALSE;
   if (!sd->multi) return EINA_FALSE;

   prev = elm_genlist_item_prev_get(sd->last_selected_item);
   if (!prev) return EINA_TRUE;

   if (elm_genlist_item_selected_get(prev))
     {
        elm_genlist_item_selected_set(sd->last_selected_item, EINA_FALSE);
        sd->last_selected_item = prev;
        elm_genlist_item_show
          (sd->last_selected_item, ELM_GENLIST_ITEM_SCROLLTO_IN);
     }
   else
     {
        elm_genlist_item_selected_set(prev, EINA_TRUE);
        elm_genlist_item_show(prev, ELM_GENLIST_ITEM_SCROLLTO_IN);
     }
   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_down(Elm_Genlist_Smart_Data *sd)
{
   Elm_Object_Item *next;

   if (!sd->selected) return EINA_FALSE;
   if (!sd->multi) return EINA_FALSE;

   next = elm_genlist_item_next_get(sd->last_selected_item);
   if (!next) return EINA_TRUE;

   if (elm_genlist_item_selected_get(next))
     {
        elm_genlist_item_selected_set(sd->last_selected_item, EINA_FALSE);
        sd->last_selected_item = next;
        elm_genlist_item_show
          (sd->last_selected_item, ELM_GENLIST_ITEM_SCROLLTO_IN);
     }
   else
     {
        elm_genlist_item_selected_set(next, EINA_TRUE);
        elm_genlist_item_show(next, ELM_GENLIST_ITEM_SCROLLTO_IN);
     }

   return EINA_TRUE;
}

static Eina_Bool
_all_items_deselect(Elm_Genlist_Smart_Data *sd)
{
   if (!sd->selected) return EINA_FALSE;

   while (sd->selected)
     elm_genlist_item_selected_set(sd->selected->data, EINA_FALSE);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_up(Elm_Genlist_Smart_Data *sd)
{
   Elm_Gen_Item *prev;

   if (!sd->selected)
     {
        prev = ELM_GEN_ITEM_FROM_INLIST(sd->items->last);
        while (prev)
          prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
     }
   else
     prev = (Elm_Gen_Item *)elm_genlist_item_prev_get
         (sd->last_selected_item);

   if (!prev) return EINA_FALSE;

   _all_items_deselect(sd);

   elm_genlist_item_selected_set((Elm_Object_Item *)prev, EINA_TRUE);
   elm_genlist_item_show((Elm_Object_Item *)prev, ELM_GENLIST_ITEM_SCROLLTO_IN);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_down(Elm_Genlist_Smart_Data *sd)
{
   Elm_Gen_Item *next;

   if (!sd->selected)
     {
        next = ELM_GEN_ITEM_FROM_INLIST(sd->items);
        while (next)
          next = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
     }
   else
     next = (Elm_Gen_Item *)elm_genlist_item_next_get
         (sd->last_selected_item);

   if (!next) return EINA_FALSE;

   _all_items_deselect(sd);

   elm_genlist_item_selected_set((Elm_Object_Item *)next, EINA_TRUE);
   elm_genlist_item_show
     ((Elm_Object_Item *)next, ELM_GENLIST_ITEM_SCROLLTO_IN);

   return EINA_TRUE;
}

static Eina_Bool
_elm_genlist_smart_event(Evas_Object *obj,
                         Evas_Object *src __UNUSED__,
                         Evas_Callback_Type type,
                         void *event_info)
{
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;
   Elm_Object_Item *it;
   Evas_Event_Key_Down *ev = event_info;
   Evas_Coord pan_max_x = 0, pan_max_y = 0;

   ELM_GENLIST_DATA_GET(obj, sd);

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (!sd->items) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   sd->s_iface->content_pos_get(obj, &x, &y);
   sd->s_iface->step_size_get(obj, &step_x, &step_y);
   sd->s_iface->page_size_get(obj, &page_x, &page_y);
   sd->s_iface->content_viewport_size_get(obj, &v_w, &v_h);

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        if (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
             (_item_multi_select_up(sd)))
            || (_item_single_select_up(sd)))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        if (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
             (_item_multi_select_down(sd)))
            || (_item_single_select_down(sd)))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Home")) ||
            ((!strcmp(ev->keyname, "KP_Home")) && (!ev->string)))
     {
        it = elm_genlist_first_item_get(obj);
        elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
        elm_genlist_item_selected_set(it, EINA_TRUE);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "End")) ||
            ((!strcmp(ev->keyname, "KP_End")) && (!ev->string)))
     {
        it = elm_genlist_last_item_get(obj);
        elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
        elm_genlist_item_selected_set(it, EINA_TRUE);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Prior")) ||
            ((!strcmp(ev->keyname, "KP_Prior")) && (!ev->string)))
     {
        if (page_y < 0)
          y -= -(page_y * v_h) / 100;
        else
          y -= page_y;
     }
   else if ((!strcmp(ev->keyname, "Next")) ||
            ((!strcmp(ev->keyname, "KP_Next")) && (!ev->string)))
     {
        if (page_y < 0)
          y += -(page_y * v_h) / 100;
        else
          y += page_y;
     }
   else if (!strcmp(ev->keyname, "Escape"))
     {
        if (!_all_items_deselect(sd)) return EINA_FALSE;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if (((!strcmp(ev->keyname, "Return")) ||
             (!strcmp(ev->keyname, "KP_Enter")) ||
             (!strcmp(ev->keyname, "space")))
            && (!sd->multi) && (sd->selected))
     {
        it = elm_genlist_selected_item_get(obj);
        elm_genlist_item_expanded_set(it, !elm_genlist_item_expanded_get(it));
        evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   _elm_genlist_pan_smart_pos_max_get(sd->pan_obj, &pan_max_x, &pan_max_y);
   if (x < 0) x = 0;
   if (x > pan_max_x) x = pan_max_x;
   if (y < 0) y = 0;
   if (y > pan_max_y) y = pan_max_y;

   sd->s_iface->content_pos_set(obj, x, y, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_elm_genlist_smart_sub_object_add(Evas_Object *obj,
                                  Evas_Object *sobj)
{
   Elm_Widget_Smart_Class *parent_parent;

   parent_parent = (Elm_Widget_Smart_Class *)((Evas_Smart_Class *)
                                              _elm_genlist_parent_sc)->parent;

   /* skipping layout's code, which registers size hint changing
    * callback on sub objects. this is here because items'
    * content_get() routines may change hints on the objects after
    * creation, thus issuing TOO MANY sizing_eval()'s here. they are
    * not needed at here anyway, so let's skip listening to those
    * hints changes */
   if (!parent_parent->sub_object_add(obj, sobj))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_elm_genlist_smart_sub_object_del(Evas_Object *obj,
                                  Evas_Object *sobj)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   /* XXX: hack -- also skipping sizing recalculation on
    * sub-object-del. genlist's crazy code paths (like groups and
    * such) seem to issue a whole lot of deletions and Evas bitches
    * about too many recalculations */
   sd->on_sub_del = EINA_TRUE;

   if (!ELM_WIDGET_CLASS(_elm_genlist_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   sd->on_sub_del = EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_elm_genlist_smart_on_focus(Evas_Object *obj)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_genlist_parent_sc)->on_focus(obj))
     return EINA_FALSE;

   if (elm_widget_focus_get(obj) && (sd->items) && (sd->selected) &&
       (!sd->last_selected_item))
     sd->last_selected_item = eina_list_data_get(sd->selected);

   return EINA_TRUE;
}

static Eina_Bool
_elm_genlist_smart_focus_next(const Evas_Object *obj,
                           Elm_Focus_Direction dir,
                           Evas_Object **next)
{
   Evas_Coord x, y, w, h;
   Evas_Coord sx, sy, sw, sh;
   Item_Block *itb;
   Eina_List *items = NULL;
   Eina_Bool done = EINA_FALSE;

   ELM_GENLIST_CHECK(obj) EINA_FALSE;
   ELM_GENLIST_DATA_GET(obj, sd);

   evas_object_geometry_get(ELM_WIDGET_DATA(sd)->obj, &sx, &sy, &sw, &sh);

   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        if (itb->realized)
          {
             Eina_List *l;
             Elm_Gen_Item *it;

             done = EINA_TRUE;
             EINA_LIST_FOREACH(itb->items, l, it)
               {
                  if (it->realized)
                    {
                       evas_object_geometry_get(it->base.view, &x, &y, &w, &h);

                       /* check item which displays more than half of its size */
                       if (it->base.access_obj &&
                           ELM_RECTS_INTERSECT
                             (x + (w / 2), y + (h / 2), 0, 0, sx, sy, sw, sh))
                         items = eina_list_append(items, it->base.access_obj);

                       if (!it->base.access_order) continue;

                       Eina_List *subl;
                       Evas_Object *subo;
                       EINA_LIST_FOREACH(it->base.access_order, subl, subo)
                         items = eina_list_append(items, subo);
                    }
               }
          }
        else if (done) break;
     }

   return elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   sd->s_iface->mirrored_set(obj, rtl);
}

static Eina_Bool
_elm_genlist_smart_theme(Evas_Object *obj)
{
   Item_Block *itb;

   ELM_GENLIST_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_genlist_parent_sc)->theme(obj))
     return EINA_FALSE;

   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   _item_cache_all_free(sd);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

#if 0
   // FIXME: difference from opensource
   elm_layout_theme_set(obj, "genlist", "base", elm_widget_style_get(obj));
#endif

   sd->item_width = sd->item_height = 0;
   sd->group_item_width = sd->group_item_height = 0;
   sd->minw = sd->minh = sd->realminw = 0;
   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        Eina_List *l;
        Elm_Gen_Item *it;

        if (itb->realized) _item_block_unrealize(itb);
        EINA_LIST_FOREACH(itb->items, l, it)
          it->item->mincalcd = EINA_FALSE;

        itb->changed = EINA_TRUE;
     }
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = NULL;
   elm_layout_sizing_eval(obj);
   sd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(sd->pan_obj);
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));

   return EINA_TRUE;
}

/* FIXME: take off later. maybe this show region coords belong in the
 * interface (new api functions, set/get)? */
static void
_show_region_hook(void *data,
                  Evas_Object *obj)
{
   Evas_Coord x, y, w, h;

   ELM_GENLIST_DATA_GET(data, sd);

   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   //x & y are screen coordinates, Add with pan coordinates
   x += sd->pan_x;
   y += sd->pan_y;
   sd->s_iface->content_region_show(obj, x, y, w, h);
}

static Eina_Bool
_elm_genlist_smart_translate(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "language,changed", NULL);

   return EINA_TRUE;
}

static void
_item_highlight(Elm_Gen_Item *it)
{
   const char *selectraise;
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;

   if (elm_widget_item_disabled_get(it)) return;
   if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY ) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY ))
     return;

   if (!sd->highlight || it->highlighted) return;
#if GENLIST_PINCH_ZOOM_SUPPORT
   if (sd->pinch_zoom_mode) return;
#endif

   selectraise = edje_object_data_get(VIEW(it), "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     {
        if (it->deco_all_view) evas_object_raise(it->deco_all_view);
        else evas_object_raise(VIEW(it));
        if ((it->item->group_item) && (it->item->group_item->realized))
          evas_object_raise(it->item->VIEW(group_item));
     }
   it->highlighted = EINA_TRUE;

   if (it->deco_all_view)
     edje_object_signal_emit(it->deco_all_view, "elm,state,selected", "elm");
   edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
   evas_object_smart_callback_call(WIDGET(it), SIG_HIGHLIGHTED, it);
}

static void
_item_unhighlight(Elm_Gen_Item *it)
{
   if (!it->highlighted) return;

   if (!it->item->nostacking)
     {
        if ((it->item->order_num_in & 0x1) ^ it->item->stacking_even)
          {
             if (it->deco_all_view) evas_object_lower(it->deco_all_view);
             else evas_object_lower(VIEW(it));
          }
        else
          {
             if (it->deco_all_view) evas_object_raise(it->deco_all_view);
             else evas_object_raise(VIEW(it));
          }
     }
   it->highlighted = EINA_FALSE;

   if (it->deco_all_view)
     edje_object_signal_emit(it->deco_all_view, "elm,state,unselected", "elm");
   edje_object_signal_emit(VIEW(it), "elm,state,unselected", "elm");
   evas_object_smart_callback_call(WIDGET(it), SIG_UNHIGHLIGHTED, it);
}

static void
_item_unselect(Elm_Gen_Item *it)
{
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   _item_unhighlight(it);
   if (!it->selected) return;

   it->selected = EINA_FALSE;
   sd->selected = eina_list_remove(sd->selected, it);
   evas_object_smart_callback_call(WIDGET(it), SIG_UNSELECTED, it);
}

static void
_item_select(Elm_Gen_Item *it)
{
   Evas_Object *obj = WIDGET(it);
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   if (elm_widget_item_disabled_get(it)) return;
   if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     return;

#if GENLIST_PINCH_ZOOM_SUPPORT
   if (IS_ROOT_PARENT_IT(it) && (sd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT))
     {
        elm_genlist_pinch_zoom_mode_set(obj, ELM_GEN_PINCH_ZOOM_EXPAND);
        elm_genlist_item_show((Elm_Object_Item *)it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
        return;
     }
#endif

#if 0
   // FIXME: difference from upstream
   if ((GL_IT(it)->wsd->last_selected_item) &&
       (it != (Elm_Gen_Item *) GL_IT(it)->wsd->last_selected_item))
     _item_unfocusable_set
       ((Elm_Gen_Item *)GL_IT(it)->wsd->last_selected_item, EINA_TRUE);
#endif

   // Do not check selected because always select mode can be used
   _item_highlight(it);
   it->selected = EINA_TRUE;
   sd->selected = eina_list_append(sd->selected, it);
   sd->last_selected_item = (Elm_Object_Item *)it;

   if (it->func.func) it->func.func((void *)it->func.data, obj, it);
   if (!EINA_MAGIC_CHECK((Elm_Widget_Item *)it, ELM_WIDGET_ITEM_MAGIC))
     return;
   evas_object_smart_callback_call(WIDGET(it), SIG_SELECTED, it);
}

static void
_item_select_unselect(Elm_Gen_Item *it, Eina_Bool selected)
{
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   Evas_Object *obj = ELM_WIDGET_DATA(sd)->obj;
   if (elm_widget_item_disabled_get(it)) return;

   evas_object_ref(obj);
   if (!sd->multi)
     {
        const Eina_List *l, *ll;
        Elm_Gen_Item *it2;
        EINA_LIST_FOREACH_SAFE(sd->selected, l, ll, it2)
           if (it2 != it) _item_unselect(it2);
     }
   if (selected) _item_select(it);
   else _item_unselect(it);
   evas_object_unref(obj);
}

static void
_item_block_position_update(Eina_Inlist *list,
                            int idx)
{
   Item_Block *tmp;

   EINA_INLIST_FOREACH(list, tmp)
     {
        tmp->position = idx++;
        tmp->position_update = EINA_TRUE;
     }
}

static void
_item_position_update(Eina_List *list,
                      int idx)
{
   Elm_Gen_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(list, l, it)
     {
        it->position = idx++;
        it->position_update = EINA_TRUE;
     }
}

static void
_item_block_merge(Item_Block *left,
                  Item_Block *right)
{
   Eina_List *l;
   Elm_Gen_Item *it2;

   EINA_LIST_FOREACH(right->items, l, it2)
     {
        it2->item->block = left;
        left->count++;
        left->changed = EINA_TRUE;
     }
   left->items = eina_list_merge(left->items, right->items);
}

static void
_item_block_del(Elm_Gen_Item *it)
{
   Eina_Inlist *il;
   Item_Block *itb = it->item->block;
   Eina_Bool block_changed = EINA_FALSE;

   itb->items = eina_list_remove(itb->items, it);
   itb->count--;
   itb->changed = EINA_TRUE;
   if (GL_IT(it)->wsd->calc_job) ecore_job_del(GL_IT(it)->wsd->calc_job);
   GL_IT(it)->wsd->calc_job = ecore_job_add(_calc_job, GL_IT(it)->wsd);
   if (itb->count < 1)
     {
        Item_Block *itbn;

        il = EINA_INLIST_GET(itb);
        itbn = (Item_Block *)(il->next);
        if (it->parent)
          it->parent->item->items =
            eina_list_remove(it->parent->item->items, it);
        else
          {
             _item_block_position_update(il->next, itb->position);
          }
        GL_IT(it)->wsd->blocks =
          eina_inlist_remove(GL_IT(it)->wsd->blocks, il);
        free(itb);
        itb = NULL;
        if (itbn) itbn->changed = EINA_TRUE;
     }
   else
     {
        if (itb->count < (itb->sd->max_items_per_block / 2))
          {
             Item_Block *itbp;
             Item_Block *itbn;

             il = EINA_INLIST_GET(itb);
             itbp = (Item_Block *)(il->prev);
             itbn = (Item_Block *)(il->next);

             /* merge block with previous */
             if ((itbp) &&
                 ((itbp->count + itb->count) <
                  (itb->sd->max_items_per_block +
                   (itb->sd->max_items_per_block / 2))))
               {
                  _item_block_merge(itbp, itb);
                  _item_block_position_update
                    (EINA_INLIST_GET(itb)->next, itb->position);
                  GL_IT(it)->wsd->blocks = eina_inlist_remove
                      (GL_IT(it)->wsd->blocks, EINA_INLIST_GET(itb));
                  free(itb);
                  block_changed = EINA_TRUE;
               }
             /* merge block with next */
             else if ((itbn) &&
                      ((itbn->count + itb->count) <
                       (itb->sd->max_items_per_block +
                        (itb->sd->max_items_per_block / 2))))
               {
                  _item_block_merge(itb, itbn);
                  _item_block_position_update
                    (EINA_INLIST_GET(itbn)->next, itbn->position);
                  GL_IT(it)->wsd->blocks =
                    eina_inlist_remove(GL_IT(it)->wsd->blocks,
                                       EINA_INLIST_GET(itbn));
                  free(itbn);
                  block_changed = EINA_TRUE;
               }
          }
     }

   if (block_changed)
     {
        GL_IT(it)->wsd->pan_changed = EINA_TRUE;
        evas_object_smart_changed(GL_IT(it)->wsd->pan_obj);
        if (GL_IT(it)->wsd->calc_job) ecore_job_del(GL_IT(it)->wsd->calc_job);
        GL_IT(it)->wsd->calc_job = NULL;
     }
#if GENLIST_FX_SUPPORT
   it->item->block = NULL;
#endif
}

static void
_decorate_all_item_unrealize(Elm_Gen_Item *it)
{
   Evas_Object *icon;

   if ((!it) || (!it->item->decorate_all_item_realized)) return;

#if GENLIST_FX_SUPPORT
   if ((GL_IT(it)->has_proxy_it) && (!GL_IT(it)->wsd->genlist_clearing))
     {
        if (!eina_list_data_find(GL_IT(it)->wsd->pending_unrealized_decorate_all_items, it))
          GL_IT(it)->wsd->pending_unrealized_decorate_all_items =
             eina_list_append(GL_IT(it)->wsd->pending_unrealized_decorate_all_items, it);
        return;
     }
#endif

   edje_object_part_unswallow(it->deco_all_view, VIEW(it));
   evas_object_smart_member_add(VIEW(it), GL_IT(it)->wsd->pan_obj);
   elm_widget_sub_object_add(WIDGET(it), VIEW(it));
   _elm_genlist_item_odd_even_update(it);
   _elm_genlist_item_state_update(it);

   edje_object_signal_emit
     (it->deco_all_view, "elm,state,decorate,disabled", "elm");
   edje_object_signal_emit
     (VIEW(it), "elm,state,decorate,disabled", "elm");
   edje_object_signal_emit
     (it->deco_all_view, "elm,state,reorder,mode_unset", "elm");
   edje_object_message_signal_process(VIEW(it));
   edje_object_message_signal_process(it->deco_all_view);

   evas_object_del(it->deco_all_view);
   it->deco_all_view = NULL;
   elm_widget_stringlist_free(it->item->deco_all_texts);
   it->item->deco_all_texts = NULL;
   elm_widget_stringlist_free(it->item->deco_all_contents);
   it->item->deco_all_contents = NULL;
   elm_widget_stringlist_free(it->item->deco_all_states);
   it->item->deco_all_states = NULL;
   EINA_LIST_FREE (it->item->deco_all_content_objs, icon)
     evas_object_del(icon);

   _item_mouse_callbacks_del(it, it->deco_all_view);
   _item_mouse_callbacks_add(it, VIEW(it));

   it->item->decorate_all_item_realized = EINA_FALSE;
}

static void
_item_free_common(Elm_Gen_Item *it)
{
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   elm_widget_item_pre_notify_del(it);
   if (it->itc->func.del)
     it->itc->func.del((void *)it->base.data, WIDGET(it));
   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   if (it->item->block) _item_block_del(it);
   if (it->parent)
     it->parent->item->items =
        eina_list_remove(it->parent->item->items, it);
   if (it->item->queued) sd->queue = eina_list_remove(sd->queue, it);
   if (it->group) sd->group_items = eina_list_remove(sd->group_items, it);

   // FIXME: relative will be better to be fixed. it is too harsh.
   if (it->item->rel)
     it->item->rel->item->rel_revs =
        eina_list_remove(it->item->rel->item->rel_revs, it);
   if (it->item->rel_revs)
     {
        Elm_Gen_Item *tmp;
        EINA_LIST_FREE(it->item->rel_revs, tmp) tmp->item->rel = NULL;
     }
   _item_sub_items_clear(it);

#if GENLIST_ENTRY_SUPPORT
   it->item->unrealize_disabled = EINA_FALSE;
#endif

   if (it->selected) sd->selected = eina_list_remove(sd->selected, it);
   if (sd->show_item == it) sd->show_item = NULL;
   if (sd->anchor_item == it)
     {
        sd->anchor_item =
           ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        if (!sd->anchor_item)          sd->anchor_item =
          ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
     }
   if (sd->expanded_item == it) sd->expanded_item = NULL;
   if (sd->expanded_next_item == it) sd->expanded_next_item = NULL;
   if (sd->state) eina_inlist_sorted_state_free(sd->state);

   if (sd->last_selected_item == (Elm_Object_Item *)it)
     sd->last_selected_item = NULL;

   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->item->swipe_timer) ecore_timer_del(it->item->swipe_timer);

   sd->items = eina_inlist_remove(sd->items, EINA_INLIST_GET(it));
   sd->item_count--;
}

static void
_item_mouse_move_cb(void *data,
                    Evas *evas __UNUSED__,
                    Evas_Object *obj,
                    void *event_info)
{
   EINA_SAFETY_ON_NULL_RETURN(data);
   Elm_Gen_Item *it = data;
   Elm_Genlist_Smart_Data *sd;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord ox, oy, ow, oh, it_scrl_y, y_pos;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;

   sd = GL_IT(it)->wsd;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        if (!sd->on_hold) _item_unselect(it);
        sd->on_hold = EINA_TRUE;
     }
   if (sd->multi_touched)
     {
        sd->cur_x = ev->cur.canvas.x;
        sd->cur_y = ev->cur.canvas.y;
        return;
     }
   if ((it->dragging) && (it->down))
     {
        if (sd->movements == SWIPE_MOVES)
          sd->swipe = EINA_TRUE;
        else
          {
             sd->history[sd->movements].x = ev->cur.canvas.x;
             sd->history[sd->movements].y = ev->cur.canvas.y;
             if (abs((sd->history[sd->movements].x -
                      sd->history[0].x)) > 40)
               sd->swipe = EINA_TRUE;
             else
               sd->movements++;
          }
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG, it);
        return;
     }
   if ((!it->down) || (sd->longpressed))
     {
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if ((sd->reorder_mode) && (sd->reorder_it))
          {
             evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);

             if (ev->cur.canvas.y < (oy + (sd->reorder_it->item->h / 2)))
                sd->reorder_fast = 1;
             else if (ev->cur.canvas.y > (oy + oh - (sd->reorder_it->item->h  / 2)))
                sd->reorder_fast = -1;
             else sd->reorder_fast = 0;

             it_scrl_y = ev->cur.canvas.y - sd->reorder_it->dy;

             if (!sd->reorder_start_y)
               sd->reorder_start_y = it->item->block->y + it->y;

             if (it_scrl_y < oy)
               y_pos = oy;
             else if (it_scrl_y + sd->reorder_it->item->h > oy + oh)
               y_pos = oy + oh - sd->reorder_it->item->h;
             else
               y_pos = it_scrl_y;

             if (it->deco_all_view)
               _item_position(it, it->deco_all_view, it->item->scrl_x, y_pos);
             else if (it->item->deco_it_view)
               _item_position
                  (it, it->item->deco_it_view, it->item->scrl_x, y_pos);
             else
               _item_position(it, VIEW(it), it->item->scrl_x, y_pos);

             if (sd->calc_job) ecore_job_del(sd->calc_job);
             sd->calc_job = ecore_job_add(_calc_job, sd);
          }
        return;
     }
   if (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
     elm_coords_finger_size_adjust(1, &minw, 1, &minh);

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - it->dx;
   adx = dx;
   if (adx < 0) adx = -dx;

   dy = y - it->dy;
   ady = dy;
   if (ady < 0) ady = -dy;

   minw *= 2;
   minh *= 2;
   if ((adx > minw) || (ady > minh))
     {
        it->dragging = EINA_TRUE;
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if (dy < 0)
          {
             if (ady > adx)
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_DRAG_START_UP, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call
                      (WIDGET(it), SIG_DRAG_START_LEFT, it);
                  else
                    evas_object_smart_callback_call
                      (WIDGET(it), SIG_DRAG_START_RIGHT, it);
               }
          }
        else
          {
             if (ady > adx)
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_DRAG_START_DOWN, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call
                      (WIDGET(it), SIG_DRAG_START_LEFT, it);
                  else
                    evas_object_smart_callback_call
                      (WIDGET(it), SIG_DRAG_START_RIGHT, it);
               }
          }
     }
}

static Eina_Bool
_long_press_cb(void *data)
{
   Elm_Gen_Item *it = data, *it2;
   Elm_Genlist_Smart_Data *sd;
   Eina_List *l, *ll;

   sd = GL_IT(it)->wsd;

   it->long_timer = NULL;
   if (elm_widget_item_disabled_get(it) || (it->dragging) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     return ECORE_CALLBACK_CANCEL;

   GL_IT(it)->wsd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(WIDGET(it), SIG_LONGPRESSED, it);
   if ((GL_IT(it)->wsd->reorder_mode) && (!it->group))
     {
        GL_IT(it)->wsd->reorder_it = it;
        GL_IT(it)->wsd->reorder_start_y = 0;
        if (it->deco_all_view)
          evas_object_raise(it->deco_all_view);
        else
          evas_object_raise(VIEW(it));

        sd->s_iface->hold_set(ELM_WIDGET_DATA(sd)->obj, EINA_TRUE);
        sd->s_iface->bounce_allow_set
          (ELM_WIDGET_DATA(sd)->obj, EINA_FALSE, EINA_FALSE);

        EINA_LIST_FOREACH_SAFE(sd->selected, l, ll, it2)
          {
             if (it != it2) _item_unselect(it2);
          }
#if 0
        // FIXME: difference from opensource
        if (it->item->items)
          {
             EINA_LIST_FOREACH(it->item->items, l, it_tmp)
               {
                  if (elm_genlist_item_expanded_get((Elm_Object_Item *) it_tmp))
                    {
                       elm_genlist_item_expanded_set((Elm_Object_Item *)it_tmp,
                                                     EINA_FALSE);
                       return ECORE_CALLBACK_RENEW;
                    }
               }
          }
#endif
        if (elm_genlist_item_expanded_get((Elm_Object_Item *)it))
          {
             elm_genlist_item_expanded_set((Elm_Object_Item *)it, EINA_FALSE);
             return ECORE_CALLBACK_RENEW;
          }

        if (!GL_IT(it)->wsd->decorate_all_mode)
          edje_object_signal_emit
            (VIEW(it), "elm,state,reorder,enabled", "elm");
     }

   return ECORE_CALLBACK_CANCEL;
}

static void
_swipe_do(Elm_Gen_Item *it)
{
   int i, sum = 0;

   if (!it) return;
   if ((it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       elm_widget_item_disabled_get(it)) return;

   GL_IT(it)->wsd->swipe = EINA_FALSE;
   for (i = 0; i < GL_IT(it)->wsd->movements; i++)
     {
        sum += GL_IT(it)->wsd->history[i].x;
        if (abs(GL_IT(it)->wsd->history[0].y -
                GL_IT(it)->wsd->history[i].y) > 10) return;
     }

   sum /= GL_IT(it)->wsd->movements;
   if (abs(sum - GL_IT(it)->wsd->history[0].x) <= 10) return;
   evas_object_smart_callback_call(WIDGET(it), SIG_SWIPE, it);
}

static Eina_Bool
_swipe_cancel(void *data)
{
   Elm_Gen_Item *it = data;

   if (!it) return ECORE_CALLBACK_CANCEL;
   GL_IT(it)->wsd->swipe = EINA_FALSE;
   GL_IT(it)->wsd->movements = 0;

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_multi_cancel(void *data)
{
   Elm_Genlist_Smart_Data *sd = data;

   if (!sd) return ECORE_CALLBACK_CANCEL;
   sd->multi_timeout = EINA_TRUE;

   return ECORE_CALLBACK_RENEW;
}

static void
_multi_touch_gesture_eval(void *data)
{
   Elm_Gen_Item *it = data;
   Evas_Coord minw = 0, minh = 0;
   Evas_Coord off_x, off_y, off_mx, off_my;

   GL_IT(it)->wsd->multi_touched = EINA_FALSE;
   if (GL_IT(it)->wsd->multi_timer)
     {
        ecore_timer_del(GL_IT(it)->wsd->multi_timer);
        GL_IT(it)->wsd->multi_timer = NULL;
     }
   if (GL_IT(it)->wsd->multi_timeout)
     {
        GL_IT(it)->wsd->multi_timeout = EINA_FALSE;
        return;
     }

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   off_x = abs(GL_IT(it)->wsd->cur_x - GL_IT(it)->wsd->prev_x);
   off_y = abs(GL_IT(it)->wsd->cur_y - GL_IT(it)->wsd->prev_y);
   off_mx = abs(GL_IT(it)->wsd->cur_mx - GL_IT(it)->wsd->prev_mx);
   off_my = abs(GL_IT(it)->wsd->cur_my - GL_IT(it)->wsd->prev_my);

   if (((off_x > minw) || (off_y > minh)) && ((off_mx > minw)
                                              || (off_my > minh)))
     {
        if ((off_x + off_mx) > (off_y + off_my))
          {
             if ((GL_IT(it)->wsd->cur_x > GL_IT(it)->wsd->prev_x)
                 && (GL_IT(it)->wsd->cur_mx > GL_IT(it)->wsd->prev_mx))
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_MULTI_SWIPE_RIGHT, it);
             else if ((GL_IT(it)->wsd->cur_x < GL_IT(it)->wsd->prev_x)
                      && (GL_IT(it)->wsd->cur_mx < GL_IT(it)->wsd->prev_mx))
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_MULTI_SWIPE_LEFT, it);
             else if (abs(GL_IT(it)->wsd->cur_x - GL_IT(it)->wsd->cur_mx) >
                      abs(GL_IT(it)->wsd->prev_x - GL_IT(it)->wsd->prev_mx))
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_MULTI_PINCH_OUT, it);
             else
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_MULTI_PINCH_IN, it);
          }
        else
          {
             if ((GL_IT(it)->wsd->cur_y > GL_IT(it)->wsd->prev_y)
                 && (GL_IT(it)->wsd->cur_my > GL_IT(it)->wsd->prev_my))
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_MULTI_SWIPE_DOWN, it);
             else if ((GL_IT(it)->wsd->cur_y < GL_IT(it)->wsd->prev_y)
                      && (GL_IT(it)->wsd->cur_my < GL_IT(it)->wsd->prev_my))
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_MULTI_SWIPE_UP, it);
             else if (abs(GL_IT(it)->wsd->cur_y - GL_IT(it)->wsd->cur_my) >
                      abs(GL_IT(it)->wsd->prev_y - GL_IT(it)->wsd->prev_my))
               {
                  evas_object_smart_callback_call
                     (WIDGET(it), SIG_MULTI_PINCH_OUT, it);
#if GENLIST_PINCH_ZOOM_SUPPORT
                  if (IS_ROOT_PARENT_IT(it) && (GL_IT(it)->wsd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT))
                    {
                       elm_genlist_pinch_zoom_mode_set(ELM_WIDGET_DATA(GL_IT(it)->wsd)->obj, ELM_GEN_PINCH_ZOOM_EXPAND);
                       elm_genlist_item_show((Elm_Object_Item *)it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
                    }
#endif
               }
             else
               {
                  evas_object_smart_callback_call
                     (WIDGET(it), SIG_MULTI_PINCH_IN, it);
#if GENLIST_PINCH_ZOOM_SUPPORT
                  elm_genlist_pinch_zoom_mode_set(ELM_WIDGET_DATA(GL_IT(it)->wsd)->obj,
                                                  ELM_GEN_PINCH_ZOOM_CONTRACT);
#endif
               }
          }
     }

   GL_IT(it)->wsd->multi_timeout = EINA_FALSE;
}

static void
_item_multi_down_cb(void *data,
                    Evas *evas __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Multi_Down *ev = event_info;

   if ((GL_IT(it)->wsd->multi_device != 0) || (GL_IT(it)->wsd->multi_touched)
       || (GL_IT(it)->wsd->multi_timeout))
     return;

   GL_IT(it)->wsd->multi_device = ev->device;
   GL_IT(it)->wsd->multi_down = EINA_TRUE;
   GL_IT(it)->wsd->multi_touched = EINA_TRUE;
   GL_IT(it)->wsd->prev_mx = ev->canvas.x;
   GL_IT(it)->wsd->prev_my = ev->canvas.y;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->dragging)
     {
        it->dragging = EINA_FALSE;
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG_STOP, it);
     }
   if (it->item->swipe_timer)
     {
        ecore_timer_del(it->item->swipe_timer);
        it->item->swipe_timer = NULL;
     }
   GL_IT(it)->wsd->longpressed = EINA_FALSE;
   if (GL_IT(it)->wsd->on_hold)
     {
        GL_IT(it)->wsd->swipe = EINA_FALSE;
        GL_IT(it)->wsd->movements = 0;
        GL_IT(it)->wsd->on_hold = EINA_FALSE;
     }
}

static void
_item_multi_up_cb(void *data,
                  Evas *evas __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Multi_Up *ev = event_info;

   if (GL_IT(it)->wsd->multi_device != ev->device) return;
   GL_IT(it)->wsd->multi_device = 0;
   GL_IT(it)->wsd->multi_down = EINA_FALSE;
   if (GL_IT(it)->wsd->mouse_down) return;
   _multi_touch_gesture_eval(data);
}

static void
_item_multi_move_cb(void *data,
                    Evas *evas __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Multi_Move *ev = event_info;

   if (GL_IT(it)->wsd->multi_device != ev->device) return;
   GL_IT(it)->wsd->cur_mx = ev->cur.canvas.x;
   GL_IT(it)->wsd->cur_my = ev->cur.canvas.y;
}

#if 0
// FIXME: difference from upstream
static void
_item_unfocusable_set(Elm_Gen_Item *it, Eina_Bool unfocus)
{
   Eina_List *l;
   Evas_Object *item_obj;
   Evas_Object *obj = WIDGET(it);

   if (!it->can_focus)
     {
        EINA_LIST_FOREACH(it->content_objs, l, item_obj)
          {
            if (unfocus) elm_widget_focused_object_clear(item_obj);
            elm_widget_tree_unfocusable_set(item_obj, unfocus);
          }
        EINA_LIST_FOREACH(it->item->flip_content_objs, l, item_obj)
          {
             if (unfocus) elm_widget_focused_object_clear(item_obj);
             elm_widget_tree_unfocusable_set(item_obj, unfocus);
          }
        EINA_LIST_FOREACH(it->item->deco_all_content_objs, l, item_obj)
          {
             if (unfocus) elm_widget_focused_object_clear(item_obj);
             elm_widget_tree_unfocusable_set(item_obj, unfocus);
          }
        it->can_focus = !unfocus;
     }
}
#endif

static void
_item_mouse_down_cb(void *data,
                    Evas *evas __UNUSED__,
                    Evas_Object *obj,
                    void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Elm_Gen_Item *it = data;
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   Evas_Coord x, y;

   if (ev->button != 1) return;
#if 0
   _item_unfocusable_set(it, EINA_FALSE);
#endif
   it->down = EINA_TRUE;
   it->dragging = EINA_FALSE;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   it->dx = ev->canvas.x - x;
   it->dy = ev->canvas.y - y;
   sd->mouse_down = EINA_TRUE;
   if (!sd->multi_touched)
     {
        sd->prev_x = ev->canvas.x;
        sd->prev_y = ev->canvas.y;
        sd->multi_timeout = EINA_FALSE;
        if (sd->multi_timer) ecore_timer_del(sd->multi_timer);
        sd->multi_timer = ecore_timer_add(1, _multi_cancel, sd);
     }
   sd->longpressed = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        sd->on_hold = EINA_TRUE;
        return;
     }
   else sd->on_hold = EINA_FALSE;
   _item_highlight(it);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     if ((!elm_widget_item_disabled_get(it)) &&
         (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
       {
          evas_object_smart_callback_call(WIDGET(it), SIG_CLICKED_DOUBLE, it);
          evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
       }
   if (it->item->swipe_timer) ecore_timer_del(it->item->swipe_timer);
   it->item->swipe_timer = ecore_timer_add(0.4, _swipe_cancel, it);
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->realized)
     it->long_timer = ecore_timer_add
         (sd->longpress_timeout, _long_press_cb, it);
   else
     it->long_timer = NULL;
   sd->swipe = EINA_FALSE;
   sd->movements = 0;
}

static Item_Block *
_item_block_new(Elm_Genlist_Smart_Data *sd,
                Eina_Bool prepend)
{
   Item_Block *itb;

   itb = calloc(1, sizeof(Item_Block));
   if (!itb) return NULL;
   itb->sd = sd;
   if (prepend)
     {
        sd->blocks = eina_inlist_prepend(sd->blocks, EINA_INLIST_GET(itb));
        _item_block_position_update(sd->blocks, 0);
     }
   else
     {
        sd->blocks = eina_inlist_append(sd->blocks, EINA_INLIST_GET(itb));
        itb->position_update = EINA_TRUE;
        if (sd->blocks != EINA_INLIST_GET(itb))
          {
             itb->position =
               ((Item_Block *)(EINA_INLIST_GET(itb)->prev))->position + 1;
          }
        else
          {
             itb->position = 0;
          }
     }

   return itb;
}

static Eina_Bool
_item_block_add(Elm_Genlist_Smart_Data *sd,
                Elm_Gen_Item *it)
{
   Item_Block *itb = NULL;

   if (!it->item->rel)
     {
newblock:
        if (it->item->rel)
          {
             itb = calloc(1, sizeof(Item_Block));
             if (!itb) return EINA_FALSE;
             itb->sd = sd;
             if (!it->item->rel->item->block)
               {
                  sd->blocks =
                    eina_inlist_append(sd->blocks, EINA_INLIST_GET(itb));
                  itb->items = eina_list_append(itb->items, it);
                  itb->position_update = EINA_TRUE;
                  it->position = eina_list_count(itb->items);
                  it->position_update = EINA_TRUE;

                  if (sd->blocks != EINA_INLIST_GET(itb))
                    {
                       itb->position =
                         ((Item_Block *)
                          (EINA_INLIST_GET(itb)->prev))->position + 1;
                    }
                  else
                    {
                       itb->position = 0;
                    }
               }
             else
               {
                  Eina_List *tmp;

                  tmp = eina_list_data_find_list(itb->items, it->item->rel);
                  if (it->item->before)
                    {
                       sd->blocks = eina_inlist_prepend_relative
                           (sd->blocks, EINA_INLIST_GET(itb),
                           EINA_INLIST_GET(it->item->rel->item->block));
                       itb->items =
                         eina_list_prepend_relative_list(itb->items, it, tmp);

                       /* Update index from where we prepended */
                       _item_position_update
                         (eina_list_prev(tmp), it->item->rel->position);
                       _item_block_position_update
                         (EINA_INLIST_GET(itb),
                         it->item->rel->item->block->position);
                    }
                  else
                    {
                       sd->blocks = eina_inlist_append_relative
                           (sd->blocks, EINA_INLIST_GET(itb),
                           EINA_INLIST_GET(it->item->rel->item->block));
                       itb->items =
                         eina_list_append_relative_list(itb->items, it, tmp);

                       /* Update block index from where we appended */
                       _item_position_update
                         (eina_list_next(tmp), it->item->rel->position + 1);
                       _item_block_position_update
                         (EINA_INLIST_GET(itb),
                         it->item->rel->item->block->position + 1);
                    }
               }
          }
        else
          {
             if (it->item->before)
               {
                  if (sd->blocks)
                    {
                       itb = (Item_Block *)(sd->blocks);
                       if (itb->count >= sd->max_items_per_block)
                         {
                            itb = _item_block_new(sd, EINA_TRUE);
                            if (!itb) return EINA_FALSE;
                         }
                    }
                  else
                    {
                       itb = _item_block_new(sd, EINA_TRUE);
                       if (!itb) return EINA_FALSE;
                    }
                  itb->items = eina_list_prepend(itb->items, it);

                  _item_position_update(itb->items, 0);
               }
             else
               {
                  if (sd->blocks)
                    {
                       itb = (Item_Block *)(sd->blocks->last);
                       if (itb->count >= sd->max_items_per_block)
                         {
                            itb = _item_block_new(sd, EINA_FALSE);
                            if (!itb) return EINA_FALSE;
                         }
                    }
                  else
                    {
                       itb = _item_block_new(sd, EINA_FALSE);
                       if (!itb) return EINA_FALSE;
                    }
                  itb->items = eina_list_append(itb->items, it);
                  it->position = eina_list_count(itb->items);
               }
          }
     }
   else
     {
        Eina_List *tmp;

        if (it->item->rel->item->queued)
          {
             /* NOTE: for a strange reason eina_list and eina_inlist
                don't have the same property on sorted insertion
                order, so the queue is not always ordered like the
                item list.  This lead to issue where we depend on an
                item that is not yet created. As a quick work around,
                we reschedule the calc of the item and stop reordering
                the list to prevent any nasty issue to show up here.
              */
             sd->queue = eina_list_append(sd->queue, it);
             sd->requeued = EINA_TRUE;
             it->item->queued = EINA_TRUE;

             return EINA_FALSE;
          }
        itb = it->item->rel->item->block;
        if (!itb) goto newblock;
        tmp = eina_list_data_find_list(itb->items, it->item->rel);
        if (it->item->before)
          {
             itb->items = eina_list_prepend_relative_list(itb->items, it, tmp);
             _item_position_update
               (eina_list_prev(tmp), it->item->rel->position);
          }
        else
          {
             itb->items = eina_list_append_relative_list(itb->items, it, tmp);
             _item_position_update
               (eina_list_next(tmp), it->item->rel->position + 1);
          }
     }

   itb->count++;
   itb->changed = EINA_TRUE;
   it->item->block = itb;
   if (itb->sd->calc_job) ecore_job_del(itb->sd->calc_job);
   itb->sd->calc_job = ecore_job_add(_calc_job, itb->sd);

   if (itb->count > itb->sd->max_items_per_block)
     {
        int newc;
        Item_Block *itb2;
        Elm_Gen_Item *it2;
        Eina_Bool done = EINA_FALSE;

        newc = itb->count / 2;

        if (EINA_INLIST_GET(itb)->prev)
          {
             Item_Block *itbp = (Item_Block *)(EINA_INLIST_GET(itb)->prev);

             if (itbp->count + newc < sd->max_items_per_block / 2)
               {
                  /* moving items to previous block */
                  while ((itb->count > newc) && (itb->items))
                    {
                       it2 = eina_list_data_get(itb->items);
                       itb->items = eina_list_remove_list
                           (itb->items, itb->items);
                       itb->count--;

                       itbp->items = eina_list_append(itbp->items, it2);
                       it2->item->block = itbp;
                       itbp->count++;
                    }

                  done = EINA_TRUE;
               }
          }

        if (!done && EINA_INLIST_GET(itb)->next)
          {
             Item_Block *itbn = (Item_Block *)(EINA_INLIST_GET(itb)->next);

             if (itbn->count + newc < sd->max_items_per_block / 2)
               {
                  /* moving items to next block */
                  while ((itb->count > newc) && (itb->items))
                    {
                       Eina_List *l;

                       l = eina_list_last(itb->items);
                       it2 = eina_list_data_get(l);
                       itb->items = eina_list_remove_list(itb->items, l);
                       itb->count--;

                       itbn->items = eina_list_prepend(itbn->items, it2);
                       it2->item->block = itbn;
                       itbn->count++;
                    }

                  done = EINA_TRUE;
               }
          }

        if (!done)
          {
             /* moving items to new block */
             itb2 = calloc(1, sizeof(Item_Block));
             if (!itb2) return EINA_FALSE;
             itb2->sd = sd;
             sd->blocks =
               eina_inlist_append_relative(sd->blocks, EINA_INLIST_GET(itb2),
                                           EINA_INLIST_GET(itb));
             itb2->changed = EINA_TRUE;
             while ((itb->count > newc) && (itb->items))
               {
                  Eina_List *l;

                  l = eina_list_last(itb->items);
                  it2 = l->data;
                  itb->items = eina_list_remove_list(itb->items, l);
                  itb->count--;

                  itb2->items = eina_list_prepend(itb2->items, it2);
                  it2->item->block = itb2;
                  itb2->count++;
               }
          }
     }

   return EINA_TRUE;
}

static int
_item_process(Elm_Genlist_Smart_Data *sd,
              Elm_Gen_Item *it)
{
   if (!_item_block_add(sd, it)) return 1;
   if (!sd->blocks)
     _item_block_realize(it->item->block);

   return 0;
}

static void
_item_process_post(Elm_Genlist_Smart_Data *sd,
                   Elm_Gen_Item *it,
                   Eina_Bool qadd)
{
   Eina_Bool show_me = EINA_FALSE;

   if (it->item->block->changed)
     {
        show_me = _item_block_recalc
            (it->item->block, it->item->block->num, qadd);
        it->item->block->changed = 0;
        if (sd->pan_changed)
          {
             if (sd->calc_job) ecore_job_del(sd->calc_job);
             sd->calc_job = NULL;
             _calc_job(sd);
             sd->pan_changed = EINA_FALSE;
          }
     }
   if (show_me) it->item->block->show_me = EINA_TRUE;

   /* when prepending, move the scroller along with the first selected
    * item to create the illusion that we're watching the selected
    * item this prevents the selected item being scrolled off the
    * viewport
    */
   if (sd->selected && it->item->before)
     {
        int y, h;
        Elm_Gen_Item *it2;

        it2 = sd->selected->data;
        if (!it2->item->block) return;
        sd->s_iface->content_pos_get(ELM_WIDGET_DATA(sd)->obj, NULL, &y);
        evas_object_geometry_get(sd->pan_obj, NULL, NULL, NULL, &h);
        if ((it->y + it->item->block->y > y + h) ||
            (it->y + it->item->block->y + it->item->h < y))
          /* offscreen, just update */
          sd->s_iface->content_region_show
            (ELM_WIDGET_DATA(sd)->obj, it2->x + it2->item->block->x, y,
            it2->item->block->w, h);
        else
          sd->s_iface->content_region_show
            (ELM_WIDGET_DATA(sd)->obj, it->x + it->item->block->x,
            y + it->item->h, it->item->block->w, h);
     }
}

static int
_queue_process(Elm_Genlist_Smart_Data *sd)
{
   int n;
   double t0, t;

   t0 = ecore_loop_time_get();
   for (n = 0; (sd->queue) && (n < 128); n++)
     {
        Elm_Gen_Item *it;

        it = eina_list_data_get(sd->queue);
        sd->queue = eina_list_remove_list(sd->queue, sd->queue);
        it->item->queued = EINA_FALSE;
        if (_item_process(sd, it)) continue;
        t = ecore_time_get();
        _item_process_post(sd, it, EINA_TRUE);
        /* same as eina_inlist_count > 1 */
        if (sd->blocks && sd->blocks->next)
          {
             if ((t - t0) > (ecore_animator_frametime_get())) break;
          }
     }
   return n;
}

static Eina_Bool
_item_idle_enterer(void *data)
{
   Elm_Genlist_Smart_Data *sd = data;

   if (sd->prev_viewport_w == 0) return ECORE_CALLBACK_RENEW;
   if (_queue_process(sd) > 0)
     {
        // wake up mainloop
        // Job always be alive because idle_enterer should be alive
        if (sd->calc_job) ecore_job_del(sd->calc_job);
        sd->calc_job = ecore_job_add(_calc_job, sd);
     }
   if (!sd->queue)
     {
        sd->queue_idle_enterer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

#if GENLIST_FX_SUPPORT
   _elm_genlist_fx_clear(ELM_WIDGET_DATA(sd)->obj, EINA_FALSE);
#endif
   return ECORE_CALLBACK_RENEW;
}

static void
_item_queue(Elm_Genlist_Smart_Data *sd,
            Elm_Gen_Item *it,
            Eina_Compare_Cb cb)
{
   if (it->item->queued) return;

   it->item->queued = EINA_TRUE;
   if (cb && !sd->requeued)
     sd->queue = eina_list_sorted_insert(sd->queue, cb, it);
   else
     sd->queue = eina_list_append(sd->queue, it);

   if (sd->queue_idle_enterer)
      ecore_idle_enterer_del(sd->queue_idle_enterer);
   sd->queue_idle_enterer = ecore_idle_enterer_add(_item_idle_enterer, sd);
}

/* If the application wants to know the relative item, use
 * elm_genlist_item_prev_get(it)*/
static void
_item_move_after(Elm_Gen_Item *it,
                 Elm_Gen_Item *after)
{
   if (!it) return;
   if (!after) return;
   if (it == after) return;

   GL_IT(it)->wsd->items =
     eina_inlist_remove(GL_IT(it)->wsd->items, EINA_INLIST_GET(it));
   if (it->item->block) _item_block_del(it);

   GL_IT(it)->wsd->items = eina_inlist_append_relative
       (GL_IT(it)->wsd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(after));
   it->item->rel = after;
   after->item->rel_revs = eina_list_append(after->item->rel_revs, it);
   it->item->before = EINA_FALSE;
   if (after->item->group_item) it->item->group_item = after->item->group_item;
   _item_queue(GL_IT(it)->wsd, it, NULL);

   evas_object_smart_callback_call(WIDGET(it), SIG_MOVED_AFTER, it);
}

static void
_access_activate_cb(void *data __UNUSED__,
                    Evas_Object *part_obj __UNUSED__,
                    Elm_Object_Item *item)
{
   _item_select_unselect((Elm_Gen_Item *)item, EINA_TRUE);
}

/* If the application wants to know the relative item, use
 * elm_genlist_item_next_get(it)*/
static void
_item_move_before(Elm_Gen_Item *it,
                  Elm_Gen_Item *before)
{
   if (!it) return;
   if (!before) return;
   if (it == before) return;

   GL_IT(it)->wsd->items =
     eina_inlist_remove(GL_IT(it)->wsd->items, EINA_INLIST_GET(it));
   if (it->item->block) _item_block_del(it);
   GL_IT(it)->wsd->items = eina_inlist_prepend_relative
       (GL_IT(it)->wsd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(before));
   it->item->rel = before;
   before->item->rel_revs = eina_list_append(before->item->rel_revs, it);
   it->item->before = EINA_TRUE;
   if (before->item->group_item)
     it->item->group_item = before->item->group_item;
   _item_queue(GL_IT(it)->wsd, it, NULL);

   evas_object_smart_callback_call(WIDGET(it), SIG_MOVED_BEFORE, it);
}

static void
_item_mouse_up_cb(void *data,
                  Evas *evas __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool dragged = EINA_FALSE;
   Elm_Genlist_Smart_Data *sd;
   Elm_Gen_Item *it = data;

   if (ev->button != 1) return;
   it->down = EINA_FALSE;
   sd = GL_IT(it)->wsd;

   sd->mouse_down = EINA_FALSE;
   if (sd->multi_touched)
     {
        if ((!sd->multi) && (!it->selected) && (it->highlighted))
          _item_unhighlight(it);
        if (sd->multi_down) return;
        _multi_touch_gesture_eval(data);
        return;
     }
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->dragging)
     {
        it->dragging = EINA_FALSE;
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG_STOP, it);
        dragged = 1;
     }
   if (it->item->swipe_timer)
     {
        ecore_timer_del(it->item->swipe_timer);
        it->item->swipe_timer = NULL;
     }
   if (sd->multi_timer)
     {
        ecore_timer_del(sd->multi_timer);
        sd->multi_timer = NULL;
        sd->multi_timeout = EINA_FALSE;
     }
   sd->on_hold = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        if (sd->swipe) _swipe_do(data);
        sd->longpressed = EINA_FALSE;
        return;
     }
   if ((sd->reorder_mode) && (sd->reorder_it))
     {
        Evas_Coord it_scrl_y = ev->canvas.y - sd->reorder_it->dy;
        sd->reorder_fast = 0;

        if (sd->reorder_rel &&
            (sd->reorder_it->parent == sd->reorder_rel->parent))
          {
             if (it_scrl_y <= sd->reorder_rel->item->scrl_y)
               _item_move_before(sd->reorder_it, sd->reorder_rel);
             else
               _item_move_after(sd->reorder_it, sd->reorder_rel);
             evas_object_smart_callback_call(WIDGET(it), SIG_MOVED, it);
          }
        else
          {
             if (sd->calc_job) ecore_job_del(sd->calc_job);
             sd->calc_job = ecore_job_add(_calc_job, sd);
          }
        edje_object_signal_emit(VIEW(it), "elm,state,reorder,disabled", "elm");
        sd->reorder_it = sd->reorder_rel = NULL;
        sd->s_iface->hold_set(ELM_WIDGET_DATA(sd)->obj, EINA_FALSE);
        sd->s_iface->bounce_allow_set
          (ELM_WIDGET_DATA(sd)->obj, sd->h_bounce, sd->v_bounce);
     }
   if (dragged)
     {
        if (it->want_unrealize && !(it->item->unrealize_disabled))
          {
             _item_unrealize(it, EINA_FALSE);
             if (it->item->block->want_unrealize)
               _item_block_unrealize(it->item->block);
          }
        return;
     }

   if (!sd->longpressed)
     if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS) ||
              (it->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS))
              _item_select_unselect(it, EINA_TRUE);
     else _item_select_unselect(it, !it->selected);
   else
     _item_unhighlight(it);
}

static void
_item_mouse_callbacks_add(Elm_Gen_Item *it,
                          Evas_Object *view)
{
   evas_object_event_callback_add
     (view, EVAS_CALLBACK_MOUSE_DOWN, _item_mouse_down_cb, it);
   evas_object_event_callback_add
     (view, EVAS_CALLBACK_MOUSE_UP, _item_mouse_up_cb, it);
   evas_object_event_callback_add
     (view, EVAS_CALLBACK_MOUSE_MOVE, _item_mouse_move_cb, it);
   evas_object_event_callback_add
     (view, EVAS_CALLBACK_MULTI_DOWN, _item_multi_down_cb, it);
   evas_object_event_callback_add
     (view, EVAS_CALLBACK_MULTI_UP, _item_multi_up_cb, it);
   evas_object_event_callback_add
     (view, EVAS_CALLBACK_MULTI_MOVE, _item_multi_move_cb, it);
}

static void
_item_mouse_callbacks_del(Elm_Gen_Item *it,
                          Evas_Object *view)
{
   evas_object_event_callback_del_full
     (view, EVAS_CALLBACK_MOUSE_DOWN, _item_mouse_down_cb, it);
   evas_object_event_callback_del_full
     (view, EVAS_CALLBACK_MOUSE_UP, _item_mouse_up_cb, it);
   evas_object_event_callback_del_full
     (view, EVAS_CALLBACK_MOUSE_MOVE, _item_mouse_move_cb, it);
   evas_object_event_callback_del_full
     (view, EVAS_CALLBACK_MULTI_DOWN, _item_multi_down_cb, it);
   evas_object_event_callback_del_full
     (view, EVAS_CALLBACK_MULTI_UP, _item_multi_up_cb, it);
   evas_object_event_callback_del_full
     (view, EVAS_CALLBACK_MULTI_MOVE, _item_multi_move_cb, it);
}

static Eina_Bool
_scroll_hold_timer_cb(void *data)
{
   Elm_Genlist_Smart_Data *sd = data;

   if (!data) return ECORE_CALLBACK_CANCEL;

   sd->s_iface->hold_set(ELM_WIDGET_DATA(sd)->obj, EINA_FALSE);
   sd->scr_hold_timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_decorate_item_finished_signal_cb(void *data,
                                  Evas_Object *obj,
                                  const char *emission __UNUSED__,
                                  const char *source __UNUSED__)
{
   Elm_Gen_Item *it = data;
   char buf[1024];

   if (!data || !obj) return;
   if ((!it->realized) || (!it->item->deco_it_view)) return;

   it->item->nocache_once = EINA_FALSE;
   _decorate_item_unrealize(it);
   if (it->item->group_item)
     evas_object_raise(it->item->VIEW(group_item));

   snprintf(buf, sizeof(buf), "elm,state,%s,passive,finished",
            GL_IT(it)->wsd->decorate_it_type);
   edje_object_signal_callback_del_full
     (obj, buf, "elm", _decorate_item_finished_signal_cb, it);
}

static Eina_List *
_item_mode_content_unrealize(Elm_Gen_Item *it,
                             Evas_Object *target,
                             Eina_List **source,
                             const char *parts,
                             Eina_List **contents_list)
{
   Eina_List *res = *contents_list;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;
        Evas_Object *ic = NULL;

        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             ic = edje_object_part_swallow_get(target, key);
             if (ic)
               {
                  res = eina_list_remove(res, ic);
                  edje_object_part_unswallow(target, ic);
                  evas_object_del(ic);
               }
          }
     }

   return res;
}

static Eina_List *
_item_content_unrealize(Elm_Gen_Item *it,
                        Evas_Object *target,
                        Eina_List **source,
                        const char *parts)
{
   Eina_List *res = it->content_objs;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;
        Evas_Object *ic = NULL;

        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             ic = edje_object_part_swallow_get(target, key);
             if (ic)
               {
                  res = eina_list_remove(res, ic);
                  edje_object_part_unswallow(target, ic);
                  // FIXME: If parent-child relationship was broken before 'ic'
                  // is deleted, freeze_pop will not be called. ex) elm_slider
                  // If layout is used instead of edje, this problme can be
                  // solved.
                  if (0 != elm_widget_scroll_freeze_get(ic))
                    elm_widget_scroll_freeze_pop(ic);
                  evas_object_del(ic);
               }
          }
     }

   return res;
}

static Eina_Bool
_item_block_recalc(Item_Block *itb,
                   int in,
                   Eina_Bool qadd)
{
   const Eina_List *l, *ll;
   Elm_Gen_Item *it;
   Evas_Coord minw = 0, minh = 0;
   Eina_Bool show_me = EINA_FALSE, changed = EINA_FALSE;
   Evas_Coord y = 0;

   itb->num = in;
   EINA_LIST_FOREACH_SAFE(itb->items, l, ll, it)
     {
        show_me |= it->item->show_me;
        if (!itb->realized)
          {
             if (qadd)
               {
                  if (!it->item->mincalcd) changed = EINA_TRUE;
                  if (changed)
                    {
                       _item_realize(it, in, EINA_TRUE);
                       _item_unrealize(it, EINA_TRUE);
                    }
               }
             else
               {
                  if ((itb->sd->homogeneous) &&
                      (itb->sd->mode == ELM_LIST_COMPRESS))
                    {
                       if (it->group)
                         {
                            it->item->w = it->item->minw =
                                GL_IT(it)->wsd->group_item_width;
                            it->item->h = it->item->minh =
                                GL_IT(it)->wsd->group_item_height;
                         }
                       else
                         {
                            it->item->w = it->item->minw =
                                GL_IT(it)->wsd->item_width;
                            it->item->h = it->item->minh =
                                GL_IT(it)->wsd->item_height;
                         }
                    }
                  else
                    {
                       _item_realize(it, in, EINA_TRUE);
                       _item_unrealize(it, EINA_TRUE);
                    }
               }
          }
        else
          {
             if (!it->item->mincalcd) changed = EINA_TRUE;
             _item_realize(it, in, EINA_FALSE);
          }
        minh += it->item->minh;
        if (minw < it->item->minw) minw = it->item->minw;
        in++;
        it->x = 0;
        it->y = y;
#if GENLIST_PINCH_ZOOM_SUPPORT
        if (IS_ROOT_PARENT_IT(it))
          it->item->pan_scrl_y = itb->y + it->y;
#endif
        y += it->item->h;
     }
   if (changed) itb->sd->pan_changed = changed;
   itb->minw = minw;
   itb->minh = minh;
   itb->changed = EINA_FALSE;
   itb->position_update = EINA_FALSE;

   return show_me;
}

static void
_update_job(void *data)
{
   Eina_Bool position = EINA_FALSE, recalc = EINA_FALSE;
   Elm_Genlist_Smart_Data *sd = data;
   Item_Block *itb;
   Eina_List *l2;
   int num, num0;

   sd->update_job = NULL;
   num = 0;

   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        Evas_Coord itminw, itminh;
        Elm_Gen_Item *it;

        if (!itb->updateme)
          {
             num += itb->count;
             if (position)
               _item_block_position(itb, num);
             continue;
          }
        num0 = num;
        recalc = EINA_FALSE;
        EINA_LIST_FOREACH(itb->items, l2, it)
          {
             if (it->item->updateme)
               {
                  itminw = it->item->minw;
                  itminh = it->item->minh;

                  it->item->updateme = EINA_FALSE;
                  if (it->realized)
                    {
                       _item_unrealize(it, EINA_FALSE);
                       _item_realize(it, num, EINA_FALSE);
                       position = EINA_TRUE;
                    }
                  else
                    {
                       _item_realize(it, num, EINA_TRUE);
                       _item_unrealize(it, EINA_TRUE);
                    }
                  if ((it->item->minw != itminw) || (it->item->minh != itminh))
                    recalc = EINA_TRUE;
               }
             num++;
          }
        itb->updateme = EINA_FALSE;
        if (recalc)
          {
             position = EINA_TRUE;
             itb->changed = EINA_TRUE;
             _item_block_recalc(itb, num0, EINA_FALSE);
             _item_block_position(itb, num0);
          }
     }
   if (position)
     {
        if (sd->calc_job) ecore_job_del(sd->calc_job);
        sd->calc_job = ecore_job_add(_calc_job, sd);
     }
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data __UNUSED__)
{
#if GENLIST_FX_SUPPORT
   _elm_genlist_fx_clear(obj, EINA_FALSE);
#endif
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data __UNUSED__)
{
#if GENLIST_FX_SUPPORT
   _elm_genlist_fx_clear(obj, EINA_FALSE);
#endif
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_edge_left_cb(Evas_Object *obj,
              void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_LEFT, NULL);
}

static void
_edge_right_cb(Evas_Object *obj,
               void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_RIGHT, NULL);
}

static void
_edge_top_cb(Evas_Object *obj,
             void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_TOP, NULL);
}

static void
_edge_bottom_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_BOTTOM, NULL);
}

static void
_vbar_drag_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_VBAR_DRAG, NULL);
}

static void
_vbar_press_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_VBAR_PRESS, NULL);
}

static void
_vbar_unpress_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_VBAR_UNPRESS, NULL);
}

static void
_hbar_drag_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_HBAR_DRAG, NULL);
}

static void
_hbar_press_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_HBAR_PRESS, NULL);
}

static void
_hbar_unpress_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_HBAR_UNPRESS, NULL);
}

static void
_decorate_item_realize(Elm_Gen_Item *it)
{
   char buf[1024];
   if (it->item->deco_it_view) return;

   //evas_event_freeze(evas_object_evas_get(obj));
   it->item->deco_it_view = edje_object_add(evas_object_evas_get(WIDGET(it)));
   edje_object_scale_set
     (it->item->deco_it_view, elm_widget_scale_get(WIDGET(it)) *
     elm_config_scale_get());
   evas_object_smart_member_add
     (it->item->deco_it_view, GL_IT(it)->wsd->pan_obj);
   elm_widget_sub_object_add(WIDGET(it), it->item->deco_it_view);

   strncpy(buf, "item", sizeof(buf));
   if (GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS)
     strncat(buf, "_compress", sizeof(buf) - strlen(buf));

   if (it->item->order_num_in & 0x1)
     strncat(buf, "_odd", sizeof(buf) - strlen(buf));
   strncat(buf, "/", sizeof(buf) - strlen(buf));
   strncat(buf, it->itc->decorate_item_style, sizeof(buf) - strlen(buf));

   elm_widget_theme_object_set
     (WIDGET(it), it->item->deco_it_view, "genlist", buf,
     elm_widget_style_get(WIDGET(it)));
   edje_object_mirrored_set
     (it->item->deco_it_view, elm_widget_mirrored_get(WIDGET(it)));

   /* signal callback add */
   evas_object_event_callback_add
     (it->item->deco_it_view, EVAS_CALLBACK_MOUSE_DOWN, _item_mouse_down_cb,
     it);
   evas_object_event_callback_add
     (it->item->deco_it_view, EVAS_CALLBACK_MOUSE_UP, _item_mouse_up_cb, it);
   evas_object_event_callback_add
     (it->item->deco_it_view, EVAS_CALLBACK_MOUSE_MOVE, _item_mouse_move_cb,
     it);

   if (eina_list_count(it->item->deco_it_content_objs) != 0)
     ERR_ABORT("If you see this error, please notify us and we"
               "will fix it");

   /* text_get, content_get, state_get */
   _item_text_realize
     (it, it->item->deco_it_view, &it->item->deco_it_texts, NULL);
   if (!it->item->deco_it_contents)
     it->item->deco_it_contents =
       elm_widget_stringlist_get
         (edje_object_data_get(it->item->deco_it_view, "contents"));
   it->item->deco_it_content_objs =
     _item_mode_content_realize
       (it, it->item->deco_it_view, &it->item->deco_it_contents, NULL,
       &it->item->deco_it_content_objs);
   _item_state_realize
     (it, it->item->deco_it_view, &it->item->deco_it_states, NULL);
   edje_object_part_swallow
     (it->item->deco_it_view,
     edje_object_data_get(it->item->deco_it_view, "mode_part"), VIEW(it));

   it->want_unrealize = EINA_FALSE;
   //evas_event_thaw(evas_object_evas_get(obj));
   //evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_decorate_item_set(Elm_Gen_Item *it)
{
   Elm_Genlist_Smart_Data *sd;
   char buf[1024];

   if (!it) return;

   sd = GL_IT(it)->wsd;

   sd->mode_item = it;
   it->item->nocache_once = EINA_TRUE;

   if (sd->scr_hold_timer)
     {
        ecore_timer_del(sd->scr_hold_timer);
        sd->scr_hold_timer = NULL;
     }
   sd->s_iface->hold_set(ELM_WIDGET_DATA(sd)->obj, EINA_TRUE);
   sd->scr_hold_timer = ecore_timer_add(0.1, _scroll_hold_timer_cb, sd);

   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   _decorate_item_realize(it);
   if (it->item->group_item)
     evas_object_raise(it->item->VIEW(group_item));
   _item_position
     (it, it->item->deco_it_view, it->item->scrl_x, it->item->scrl_y);
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));

   snprintf(buf, sizeof(buf), "elm,state,%s,active", sd->decorate_it_type);
   edje_object_signal_emit(it->item->deco_it_view, buf, "elm");
}

static void
_decorate_item_unset(Elm_Genlist_Smart_Data *sd)
{
   char buf[1024], buf2[1024];
   Elm_Gen_Item *it;

   if (!sd->mode_item) return;

   it = sd->mode_item;
   it->item->nocache_once = EINA_TRUE;

   snprintf(buf, sizeof(buf), "elm,state,%s,passive", sd->decorate_it_type);
   snprintf(buf2, sizeof(buf2), "elm,state,%s,passive,finished",
            sd->decorate_it_type);
   edje_object_signal_emit(it->item->deco_it_view, buf, "elm");
   edje_object_signal_callback_add
     (it->item->deco_it_view, buf2, "elm", _decorate_item_finished_signal_cb,
     it);
   sd->mode_item = NULL;
}

#if 0
// FIXME: difference from upstream
static void
_mouse_down_scroller(void        *data,
                     Evas        *evas __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;
   wd->drag_started = EINA_FALSE;
}

static void
_mouse_up_scroller(void        *data,
                   Evas        *evas __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;
   wd->drag_started = EINA_FALSE;
}

static void
_mouse_move_scroller(void        *data,
                     Evas        *evas __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void        *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, dx, dy, adx, ady;

   if (!wd) return;
   if (wd->drag_started) return;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   dx = ev->cur.canvas.x - ev->prev.canvas.x;
   dy = ev->cur.canvas.y - ev->prev.canvas.y;
   adx = dx;
   ady = dy;
   if (adx < 0) adx = -dx;
   if (ady < 0) ady = -dy;
   if (((ady < minh) && (ady > minh / 2)) && (ady > adx))
     {
        if (dy < 0)
          {
             evas_object_smart_callback_call(data, SIG_DRAG_START_UP, NULL);
             wd->drag_started = EINA_TRUE;
          }
        else
          {
             evas_object_smart_callback_call(data, SIG_DRAG_START_DOWN, NULL);
             wd->drag_started = EINA_TRUE;
          }
     }
}
#endif

static void
_elm_genlist_smart_add(Evas_Object *obj)
{
   Evas_Coord minw, minh;
   Elm_Genlist_Pan_Smart_Data *pan_data;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Genlist_Smart_Data);

   ELM_WIDGET_CLASS(_elm_genlist_parent_sc)->base.add(obj);

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_on_show_region_hook_set(obj, _show_region_hook, obj);

   elm_layout_theme_set(obj, "genlist", "base", elm_widget_style_get(obj));

   /* interface's add() routine issued AFTER the object's smart_add() */
   priv->s_iface = evas_object_smart_interface_get
       (obj, ELM_SCROLLABLE_IFACE_NAME);

   priv->s_iface->objects_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, priv->hit_rect);

   priv->s_iface->bounce_allow_set
     (obj, EINA_FALSE, _elm_config->thumbscroll_bounce_enable);
   priv->v_bounce = _elm_config->thumbscroll_bounce_enable;

   priv->s_iface->animate_start_cb_set(obj, _scroll_animate_start_cb);
   priv->s_iface->animate_stop_cb_set(obj, _scroll_animate_stop_cb);
   priv->s_iface->drag_start_cb_set(obj, _scroll_drag_start_cb);
   priv->s_iface->drag_stop_cb_set(obj, _scroll_drag_stop_cb);
   priv->s_iface->edge_left_cb_set(obj, _edge_left_cb);
   priv->s_iface->edge_right_cb_set(obj, _edge_right_cb);
   priv->s_iface->edge_top_cb_set(obj, _edge_top_cb);
   priv->s_iface->edge_bottom_cb_set(obj, _edge_bottom_cb);
   priv->s_iface->vbar_drag_cb_set(obj, _vbar_drag_cb);
   priv->s_iface->vbar_press_cb_set(obj, _vbar_press_cb);
   priv->s_iface->vbar_unpress_cb_set(obj, _vbar_unpress_cb);
   priv->s_iface->hbar_drag_cb_set(obj, _hbar_drag_cb);
   priv->s_iface->hbar_press_cb_set(obj, _hbar_press_cb);
   priv->s_iface->hbar_unpress_cb_set(obj, _hbar_unpress_cb);

   priv->mode = ELM_LIST_SCROLL;
   priv->max_items_per_block = MAX_ITEMS_PER_BLOCK;
   priv->item_cache_max = priv->max_items_per_block * 2;
   priv->longpress_timeout = _elm_config->longpress_timeout;
   priv->highlight = EINA_TRUE;

#if GENLIST_FX_SUPPORT
   priv->fx_mode = EINA_FALSE;
   priv->rendered = EINA_FALSE;
   priv->fx_first_captured = EINA_FALSE;
   priv->fx_playing = EINA_FALSE;
   priv->fx_items_deleted = EINA_FALSE;
   priv->genlist_clearing = EINA_FALSE;
#endif

   priv->pan_obj = evas_object_smart_add
       (evas_object_evas_get(obj), _elm_genlist_pan_smart_class_new());
   pan_data = evas_object_smart_data_get(priv->pan_obj);
   pan_data->wsd = priv;

#if 0
   // FIXME: difference from upstream
   evas_object_event_callback_add(pan_obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_scroller, obj);
   evas_object_event_callback_add(pan_obj, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up_scroller, obj);
   evas_object_event_callback_add(pan_obj, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move_scroller, obj);
#endif

   priv->s_iface->extern_pan_set(obj, priv->pan_obj);

   edje_object_size_min_calc(ELM_WIDGET_DATA(priv)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   _item_cache_all_free(priv);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   elm_layout_sizing_eval(obj);
}

static void
_elm_genlist_smart_del(Evas_Object *obj)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   elm_genlist_clear(obj);
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   if (sd->update_job) ecore_job_del(sd->update_job);
   if (sd->queue_idle_enterer) ecore_idle_enterer_del(sd->queue_idle_enterer);
   if (sd->must_recalc_idler) ecore_idler_del(sd->must_recalc_idler);
   if (sd->multi_timer) ecore_timer_del(sd->multi_timer);
   if (sd->decorate_it_type) eina_stringshare_del(sd->decorate_it_type);
   if (sd->scr_hold_timer) ecore_timer_del(sd->scr_hold_timer);

   evas_object_del(sd->pan_obj);
   ELM_WIDGET_CLASS(_elm_genlist_parent_sc)->base.del(obj);
}

static void
_elm_genlist_smart_move(Evas_Object *obj,
                        Evas_Coord x,
                        Evas_Coord y)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_genlist_parent_sc)->base.move(obj, x, y);

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_genlist_smart_resize(Evas_Object *obj,
                          Evas_Coord w,
                          Evas_Coord h)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_genlist_parent_sc)->base.resize(obj, w, h);

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_genlist_smart_member_add(Evas_Object *obj,
                              Evas_Object *member)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_genlist_parent_sc)->base.member_add(obj, member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_access_obj_process(Elm_Genlist_Smart_Data *sd, Eina_Bool is_access)
{
   Item_Block *itb;
   Eina_Bool done = EINA_FALSE;

   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        if (itb->realized)
          {
             Eina_List *l;
             Elm_Gen_Item *it;

             done = EINA_TRUE;
             EINA_LIST_FOREACH(itb->items, l, it)
               {
                  if (!it->realized) continue;
                  if (is_access) _access_widget_item_register(it);
                  else
                    _elm_access_widget_item_unregister((Elm_Widget_Item *)it);
               }
          }
        else if (done) break;
     }
}

static void
_elm_genlist_smart_access(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   if (is_access)
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next =
     _elm_genlist_smart_focus_next;
   else
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next = NULL;
   _access_obj_process(sd, is_access);
}

static void
_elm_genlist_smart_set_user(Elm_Genlist_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_genlist_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_genlist_smart_del;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_genlist_smart_move;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_genlist_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.member_add = _elm_genlist_smart_member_add;

   ELM_WIDGET_CLASS(sc)->sub_object_add = _elm_genlist_smart_sub_object_add;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_genlist_smart_sub_object_del;
   ELM_WIDGET_CLASS(sc)->on_focus = _elm_genlist_smart_on_focus;
   ELM_WIDGET_CLASS(sc)->theme = _elm_genlist_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_genlist_smart_event;
   ELM_WIDGET_CLASS(sc)->translate = _elm_genlist_smart_translate;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_genlist_smart_sizing_eval;

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     ELM_WIDGET_CLASS(sc)->focus_next = _elm_genlist_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _elm_genlist_smart_access;
}

EAPI const Elm_Genlist_Smart_Class *
elm_genlist_smart_class_get(void)
{
   static Elm_Genlist_Smart_Class _sc =
     ELM_GENLIST_SMART_CLASS_INIT_NAME_VERSION(ELM_GENLIST_SMART_NAME);
   static const Elm_Genlist_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_genlist_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_genlist_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_genlist_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}


static Evas_Object *
_item_content_get_hook(Elm_Gen_Item *it,
                       const char *part)
{
   Evas_Object *ret = NULL;
   if (it->deco_all_view)
     ret = edje_object_part_swallow_get(it->deco_all_view, part);
   else if (it->decorate_it_set)
     ret = edje_object_part_swallow_get(it->item->deco_it_view, part);
   if (!ret)
     ret = edje_object_part_swallow_get(VIEW(it), part);
   return ret;
}

static const char *
_item_text_get_hook(Elm_Gen_Item *it,
                    const char *part)
{
   if (!it->itc->func.text_get) return NULL;
   const char *ret = NULL;
   if (it->deco_all_view)
     ret = edje_object_part_text_get(it->deco_all_view, part);
   else if (it->decorate_it_set)
     ret = edje_object_part_text_get(it->item->deco_it_view, part);
   if (!ret)
     ret = edje_object_part_text_get(VIEW(it), part);
   return ret;
}

static void
_item_disable_hook(Elm_Object_Item *item)
{
   Eina_List *l;
   Evas_Object *obj;
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   if (it->selected)
     elm_genlist_item_selected_set(item, EINA_FALSE);

   if (it->realized)
     {
        if (elm_widget_item_disabled_get(it))
          {
             edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
             if (it->deco_all_view)
               edje_object_signal_emit
                 (it->deco_all_view, "elm,state,disabled", "elm");
          }
        else
          {
             edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");
             if (it->deco_all_view)
               edje_object_signal_emit
                 (it->deco_all_view, "elm,state,enabled", "elm");
          }
        EINA_LIST_FOREACH(it->content_objs, l, obj)
          elm_widget_disabled_set(obj, elm_widget_item_disabled_get(it));
     }
}

static void
_item_free(Elm_Gen_Item *it)
{
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;

#if GENLIST_FX_SUPPORT
   if (sd->fx_mode) GL_IT(it)->has_proxy_it = EINA_FALSE;
   _elm_genlist_proxy_item_del((Elm_Object_Item *)it);
   if ((!sd->fx_mode) || (sd->genlist_clearing))
#endif
     {
        _item_free_common(it);
     }
   _item_unrealize(it, EINA_FALSE);
   elm_genlist_item_class_unref((Elm_Genlist_Item_Class *)it->itc);
   free(it->item);

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);
}

#if GENLIST_FX_SUPPORT
static void
_item_del_pre_fx_process(Elm_Gen_Item *it)
{
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;

   sd->fx_items_deleted = EINA_TRUE;
   _elm_genlist_fx_capture(ELM_WIDGET_DATA(sd)->obj, 0);
   if (!eina_list_data_find(sd->pending_del_items, it))
     sd->pending_del_items = eina_list_append(sd->pending_del_items, it);

   _item_free_common(it);

}
#endif

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

#if GENLIST_FX_SUPPORT
   if ((GL_IT(it)->wsd->fx_mode) && (!GL_IT(it)->wsd->genlist_clearing))
     {
        _item_del_pre_fx_process(it);
        return EINA_FALSE;
     }
#endif
   _item_free(it);
   return EINA_TRUE;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   edje_object_signal_emit(VIEW(it), emission, source);
}

static Elm_Gen_Item *
_elm_genlist_item_new(Elm_Genlist_Smart_Data *sd,
                      const Elm_Genlist_Item_Class *itc,
                      const void *data,
                      Elm_Gen_Item *parent,
                      Elm_Genlist_Item_Type type,
                      Evas_Smart_Cb func,
                      const void *func_data)
{
   Elm_Gen_Item *it, *it2;
   int depth = 0;

   if (!itc) return NULL;

   it = elm_widget_item_new(ELM_WIDGET_DATA(sd)->obj, Elm_Gen_Item);
   if (!it) return NULL;

   it->itc = itc;
   elm_genlist_item_class_ref((Elm_Genlist_Item_Class *)itc);

   it->base.data = data;
   it->parent = parent;
   it->func.func = func;
   it->func.data = func_data;

   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);

#if 0 // FIXME: difference from upstream
   it->can_focus = EINA_TRUE;
#endif
   //it->del_cb = (Ecore_Cb)_item_free;
   //it->highlight_cb = (Ecore_Cb)_item_highlight;
   //it->unhighlight_cb = (Ecore_Cb)_item_unhighlight;
   //it->sel_cb = (Ecore_Cb)_item_select;
   //it->unsel_cb = (Ecore_Cb)_item_unselect;
   //it->unrealize_cb = (Ecore_Cb)_item_unrealize_cb;

   GL_IT(it) = ELM_NEW(Elm_Gen_Item_Type);
   GL_IT(it)->wsd = sd;
   GL_IT(it)->type = type;
   if (type & ELM_GENLIST_ITEM_GROUP) it->group++;

   if (it->parent)
     {
        if (it->parent->group)
          GL_IT(it)->group_item = parent;
        else if (GL_IT(it->parent)->group_item)
          GL_IT(it)->group_item = GL_IT(it->parent)->group_item;
     }
   for (it2 = it, depth = 0; it2->parent; it2 = it2->parent)
     {
        if (!it2->parent->group) depth += 1;
     }
   GL_IT(it)->expanded_depth = depth;
   sd->item_count++;

#if GENLIST_FX_SUPPORT
   it->item->num = sd->item_count;
   if (sd->fx_mode)
     {
        sd->fx_items_deleted = EINA_FALSE;
        _elm_genlist_fx_capture(ELM_WIDGET_DATA(sd)->obj, 0);
     }
#endif
   return it;
}

static int
_elm_genlist_item_compare(const void *data,
                          const void *data1)
{
   const Elm_Gen_Item *it, *item1;

   it = ELM_GEN_ITEM_FROM_INLIST(data);
   item1 = ELM_GEN_ITEM_FROM_INLIST(data1);
   return GL_IT(it)->wsd->item_compare_cb(it, item1);
}

static int
_elm_genlist_item_list_compare(const void *data,
                               const void *data1)
{
   const Elm_Gen_Item *it = data;
   const Elm_Gen_Item *item1 = data1;

   return GL_IT(it)->wsd->item_compare_cb(it, item1);
}

EAPI unsigned int
elm_genlist_items_count(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) 0;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->item_count;
}

EAPI Elm_Object_Item *
elm_genlist_item_append(Evas_Object *obj,
                        const Elm_Genlist_Item_Class *itc,
                        const void *data,
                        Elm_Object_Item *parent,
                        Elm_Genlist_Item_Type type,
                        Evas_Smart_Cb func,
                        const void *func_data)
{
   Elm_Gen_Item *it;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);
   if (parent)
     ELM_WIDGET_ITEM_RETURN_IF_ONDEL(((Elm_Widget_Item *)parent),NULL);

   it = _elm_genlist_item_new
       (sd, itc, data, (Elm_Gen_Item *)parent, type, func, func_data);
   if (!it) return NULL;

   if (!it->parent)
     {
        if (it->group)
          sd->group_items = eina_list_append(sd->group_items, it);
        sd->items = eina_inlist_append(sd->items, EINA_INLIST_GET(it));
        it->item->rel = NULL;
     }
   else
     {
        Elm_Gen_Item *it2 = NULL;
        Eina_List *ll = eina_list_last(it->parent->item->items);

        if (ll) it2 = ll->data;
        it->parent->item->items =
          eina_list_append(it->parent->item->items, it);
        if (!it2) it2 = it->parent;
        sd->items = eina_inlist_append_relative
          (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(it2));
        it->item->rel = it2;
        it2->item->rel_revs = eina_list_append(it2->item->rel_revs, it);
     }
   it->item->before = EINA_FALSE;
   _item_queue(sd, it, NULL);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_prepend(Evas_Object *obj,
                         const Elm_Genlist_Item_Class *itc,
                         const void *data,
                         Elm_Object_Item *parent,
                         Elm_Genlist_Item_Type type,
                         Evas_Smart_Cb func,
                         const void *func_data)
{
   Elm_Gen_Item *it;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);
   it = _elm_genlist_item_new
       (sd, itc, data, (Elm_Gen_Item *)parent, type, func, func_data);
   if (!it) return NULL;

   if (!it->parent)
     {
        if (it->group)
          sd->group_items = eina_list_prepend(sd->group_items, it);
        sd->items = eina_inlist_prepend(sd->items, EINA_INLIST_GET(it));
        it->item->rel = NULL;
     }
   else
     {
        Elm_Gen_Item *it2 = NULL;
        Eina_List *ll = it->parent->item->items;

        if (ll) it2 = ll->data;
        it->parent->item->items =
          eina_list_prepend(it->parent->item->items, it);
        if (!it2) it2 = it->parent;
        sd->items = eina_inlist_prepend_relative
            (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(it2));
        it->item->rel = it2;
        it2->item->rel_revs = eina_list_append(it2->item->rel_revs, it);
     }
   it->item->before = EINA_TRUE;
   _item_queue(sd, it, NULL);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_insert_after(Evas_Object *obj,
                              const Elm_Genlist_Item_Class *itc,
                              const void *data,
                              Elm_Object_Item *parent,
                              Elm_Object_Item *after_it,
                              Elm_Genlist_Item_Type type,
                              Evas_Smart_Cb func,
                              const void *func_data)
{
   Elm_Gen_Item *after = (Elm_Gen_Item *)after_it;
   Elm_Gen_Item *it;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(after_it, NULL);
   ELM_GENLIST_DATA_GET(obj, sd);

   /* It makes no sense to insert after in an empty list with after !=
    * NULL, something really bad is happening in your app. */
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->items, NULL);

   it = _elm_genlist_item_new
       (sd, itc, data, (Elm_Gen_Item *)parent, type, func, func_data);
   if (!it) return NULL;

   if (!it->parent)
     {
        if ((it->group) && (after->group))
          sd->group_items = eina_list_append_relative
              (sd->group_items, it, after);
     }
   else
     {
        it->parent->item->items =
          eina_list_append_relative(it->parent->item->items, it, after);
     }
   sd->items = eina_inlist_append_relative
       (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(after));

   it->item->rel = after;
   after->item->rel_revs = eina_list_append(after->item->rel_revs, it);
   it->item->before = EINA_FALSE;
   _item_queue(sd, it, NULL);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_insert_before(Evas_Object *obj,
                               const Elm_Genlist_Item_Class *itc,
                               const void *data,
                               Elm_Object_Item *parent,
                               Elm_Object_Item *before_it,
                               Elm_Genlist_Item_Type type,
                               Evas_Smart_Cb func,
                               const void *func_data)
{
   Elm_Gen_Item *before = (Elm_Gen_Item *)before_it;
   Elm_Gen_Item *it;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(before_it, NULL);
   ELM_GENLIST_DATA_GET(obj, sd);

   /* It makes no sense to insert before in an empty list with before
    * != NULL, something really bad is happening in your app. */
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->items, NULL);

   it = _elm_genlist_item_new
       (sd, itc, data, (Elm_Gen_Item *)parent, type, func, func_data);
   if (!it) return NULL;

   if (!it->parent)
     {
        if (it->group && before->group)
          sd->group_items =
            eina_list_prepend_relative(sd->group_items, it, before);
     }
   else
     {
        it->parent->item->items =
          eina_list_prepend_relative(it->parent->item->items, it, before);
     }
   sd->items = eina_inlist_prepend_relative
       (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(before));

   it->item->rel = before;
   before->item->rel_revs = eina_list_append(before->item->rel_revs, it);
   it->item->before = EINA_TRUE;
   _item_queue(sd, it, NULL);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_sorted_insert(Evas_Object *obj,
                               const Elm_Genlist_Item_Class *itc,
                               const void *data,
                               Elm_Object_Item *parent,
                               Elm_Genlist_Item_Type type,
                               Eina_Compare_Cb comp,
                               Evas_Smart_Cb func,
                               const void *func_data)
{
   Elm_Gen_Item *rel = NULL;
   Elm_Gen_Item *it;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   it = _elm_genlist_item_new
       (sd, itc, data, (Elm_Gen_Item *)parent, type, func, func_data);
   if (!it) return NULL;

   sd->item_compare_cb = comp;

   if (it->parent)
     {
        Eina_List *l;
        int cmp_result;

        l = eina_list_search_sorted_near_list
            (it->parent->item->items, _elm_genlist_item_list_compare, it,
            &cmp_result);

        if (l)
          rel = eina_list_data_get(l);
        else
          rel = it->parent;

        if (cmp_result >= 0)
          {
             it->parent->item->items = eina_list_prepend_relative_list
                 (it->parent->item->items, it, l);
             sd->items = eina_inlist_prepend_relative
                 (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(rel));
             it->item->before = EINA_FALSE;
          }
        else if (cmp_result < 0)
          {
             it->parent->item->items = eina_list_append_relative_list
                 (it->parent->item->items, it, l);
             sd->items = eina_inlist_append_relative
                 (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(rel));
             it->item->before = EINA_TRUE;
          }
     }
   else
     {
        if (!sd->state)
          {
             sd->state = eina_inlist_sorted_state_new();
             eina_inlist_sorted_state_init(sd->state, sd->items);
             sd->requeued = EINA_FALSE;
          }

        if (it->group)
          sd->group_items = eina_list_append(sd->group_items, it);

        sd->items = eina_inlist_sorted_state_insert
            (sd->items, EINA_INLIST_GET(it), _elm_genlist_item_compare,
            sd->state);

        if (EINA_INLIST_GET(it)->next)
          {
             rel = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
             it->item->before = EINA_TRUE;
          }
        else if (EINA_INLIST_GET(it)->prev)
          {
             rel = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
             it->item->before = EINA_FALSE;
          }
     }

   if (rel)
     {
        it->item->rel = rel;
        rel->item->rel_revs = eina_list_append(rel->item->rel_revs, it);
     }

   _item_queue(sd, it, _elm_genlist_item_list_compare);

   return (Elm_Object_Item *)it;
}

static void
_elm_genlist_fx_clear(Evas_Object *obj, Eina_Bool force)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   Elm_Gen_FX_Item *fi;
   Proxy_Item *pi;
   Elm_Gen_Item *it;

   if ((!force) && (!sd->fx_first_captured)) return;

   EINA_LIST_FREE(sd->capture_before_items, pi)
     {
        if ((pi->it) && (GL_IT(pi->it))) GL_IT(pi->it)->has_proxy_it = EINA_FALSE;
        if (pi->proxy) evas_object_smart_member_del(pi->proxy);
        if (pi->proxy) evas_object_del(pi->proxy);
        free(pi);
     }
   EINA_LIST_FREE(sd->capture_after_items, pi)
     {
        if ((pi->it) && (GL_IT(pi->it))) GL_IT(pi->it)->has_proxy_it = EINA_FALSE;
        if (pi->proxy) evas_object_smart_member_del(pi->proxy);
        if (pi->proxy) evas_object_del(pi->proxy);
        free(pi);
     }

   EINA_LIST_FREE(sd->fx_items, fi)
     {
        if (fi->trans) elm_transit_del(fi->trans);
        free(fi);
     }

   EINA_LIST_FREE (sd->pending_unrealized_items, it)
      _item_unrealize(it, EINA_FALSE);

   EINA_LIST_FREE (sd->pending_unrealized_decorate_all_items, it)
      _decorate_all_item_unrealize(it);

   EINA_LIST_FREE (sd->pending_del_items, it)
     {
        _item_free(it);
        _elm_widget_item_free((Elm_Widget_Item *)it);
     }
   if (sd->alpha_bg) evas_object_del(sd->alpha_bg);
   sd->alpha_bg = NULL;
   if (sd->fx_timer) ecore_timer_del(sd->fx_timer);
   sd->fx_timer = NULL;

   sd->genlist_clearing = EINA_TRUE;
   sd->fx_playing = EINA_FALSE;
   sd->sorting = EINA_FALSE;
   sd->fx_first_captured = EINA_FALSE;
   sd->fx_items_deleted = EINA_FALSE;
}

EAPI void
elm_genlist_clear(Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);
   Elm_Gen_Item *it;

   if (sd->state)
     {
        eina_inlist_sorted_state_free(sd->state);
        sd->state = NULL;
     }

#if GENLIST_FX_SUPPORT
   if (sd->fx_mode)
     {
        _elm_genlist_fx_clear(obj, EINA_TRUE);
        sd->genlist_clearing = EINA_TRUE;
        sd->rendered = EINA_FALSE;
     }
#endif
#if GENLIST_PINCH_ZOOM_SUPPORT
   sd->pinch_zoom_h = 0;
   sd->realized_top_item = NULL;
   sd->pinch_zoom_mode = ELM_GEN_PINCH_ZOOM_NONE;
#endif

   //evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));

   // Do not use EINA_INLIST_FOREACH or EINA_INLIST_FOREACH_SAFE
   // because sd->items can be modified inside elm_widget_item_del()
   while (sd->items)
     {
        it = EINA_INLIST_CONTAINER_GET(sd->items->last, Elm_Gen_Item);
        it->item->nocache_once = EINA_TRUE;
        elm_widget_item_del(it);
     }
   sd->items = NULL;
   sd->blocks = NULL;
   _item_cache_all_free(sd);

   sd->pan_changed = EINA_TRUE;
   if (sd->selected) sd->selected = eina_list_free(sd->selected);
   sd->selected = NULL;

   sd->anchor_item = NULL;
   if (sd->multi_timer)
     {
        ecore_timer_del(sd->multi_timer);
        sd->multi_timer = NULL;
     }
   if (sd->update_job)
     {
        ecore_job_del(sd->update_job);
        sd->update_job = NULL;
     }
   if (sd->calc_job)
     {
        ecore_job_del(sd->calc_job);
        sd->calc_job = NULL;
     }
   if (sd->queue_idle_enterer)
     {
        ecore_idle_enterer_del(sd->queue_idle_enterer);
        sd->queue_idle_enterer = NULL;
     }
   if (sd->must_recalc_idler)
     {
        ecore_idler_del(sd->must_recalc_idler);
        sd->must_recalc_idler = NULL;
     }
   if (sd->queue) sd->queue = eina_list_free(sd->queue);
   if (sd->reorder_move_animator)
     {
        ecore_animator_del(sd->reorder_move_animator);
        sd->reorder_move_animator = NULL;
     }
   sd->show_item = NULL;
   sd->reorder_old_pan_y = 0;

   sd->pan_x = 0;
   sd->pan_y = 0;
   sd->minw = 0;
   sd->minh = 0;

   if (sd->alpha_bg) evas_object_del(sd->alpha_bg);
   sd->alpha_bg = NULL;

   if (sd->pan_obj)
     {
        evas_object_size_hint_min_set(sd->pan_obj, sd->minw, sd->minh);
        evas_object_smart_callback_call(sd->pan_obj, "changed", NULL);
     }
   elm_layout_sizing_eval(ELM_WIDGET_DATA(sd)->obj);
   sd->s_iface->content_region_show(obj, 0, 0, 0, 0);

#if GENLIST_FX_SUPPORT
   sd->genlist_clearing = EINA_FALSE;
#endif
   //evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   //evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
}

EAPI void
elm_genlist_multi_select_set(Evas_Object *obj,
                             Eina_Bool multi)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   sd->multi = !!multi;
}

EAPI Eina_Bool
elm_genlist_multi_select_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) EINA_FALSE;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->multi;
}

EAPI Elm_Object_Item *
elm_genlist_selected_item_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   if (sd->selected)
     return sd->selected->data;

   return NULL;
}

EAPI Eina_List *
elm_genlist_selected_items_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->selected;
}

EAPI Eina_List *
elm_genlist_realized_items_get(const Evas_Object *obj)
{
   Item_Block *itb;
   Eina_List *list = NULL;
   Eina_Bool done = EINA_FALSE;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        if (itb->realized)
          {
             Eina_List *l;
             Elm_Gen_Item *it;

             done = EINA_TRUE;
             EINA_LIST_FOREACH(itb->items, l, it)
               {
                  if (it->realized) list = eina_list_append(list, it);
               }
          }
        else
          {
             if (done) break;
          }
     }
   return list;
}

EAPI Elm_Object_Item *
elm_genlist_at_xy_item_get(const Evas_Object *obj,
                           Evas_Coord x,
                           Evas_Coord y,
                           int *posret)
{
   Evas_Coord ox, oy, ow, oh;
   Evas_Coord lasty;
   Item_Block *itb;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);
   lasty = oy;
   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        Eina_List *l;
        Elm_Gen_Item *it;

        if (!ELM_RECTS_INTERSECT(ox + itb->x - itb->sd->pan_x,
                                 oy + itb->y - itb->sd->pan_y,
                                 itb->w, itb->h, x, y, 1, 1))
          continue;
        EINA_LIST_FOREACH(itb->items, l, it)
          {
             Evas_Coord itx, ity;

             itx = ox + itb->x + it->x - itb->sd->pan_x;
             ity = oy + itb->y + it->y - itb->sd->pan_y;
             if (ELM_RECTS_INTERSECT
                   (itx, ity, it->item->w, it->item->h, x, y, 1, 1))
               {
                  if (posret)
                    {
                       if (y <= (ity + (it->item->h / 4))) *posret = -1;
                       else if (y >= (ity + it->item->h - (it->item->h / 4)))
                         *posret = 1;
                       else *posret = 0;
                    }

                  return (Elm_Object_Item *)it;
               }
             lasty = ity + it->item->h;
          }
     }
   if (posret)
     {
        if (y > lasty) *posret = 1;
        else *posret = -1;
     }

   return NULL;
}

EAPI Elm_Object_Item *
elm_genlist_first_item_get(const Evas_Object *obj)
{
   Elm_Gen_Item *tit, *it = NULL;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   EINA_INLIST_REVERSE_FOREACH(sd->items, tit) it = tit;

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_last_item_get(const Evas_Object *obj)
{
   Elm_Gen_Item *it;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);
   if (!sd->items) return NULL;

   it = ELM_GEN_ITEM_FROM_INLIST(sd->items->last);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_next_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, NULL);

   it = (Elm_Gen_Item *)item;
   while (it)
     {
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        if (it) break;
     }

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_prev_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, NULL);

   it = (Elm_Gen_Item *)item;
   while (it)
     {
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
        if (it) break;
     }

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_parent_get(const Elm_Object_Item *it)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(it, NULL);

   return (Elm_Object_Item *)((Elm_Gen_Item *)it)->parent;
}

EAPI void
elm_genlist_item_subitems_clear(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   _item_sub_items_clear(it);
}

EAPI void
elm_genlist_item_selected_set(Elm_Object_Item *item,
                              Eina_Bool selected)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);

   if (elm_widget_item_disabled_get(it)) return;
   selected = !!selected;
   if (it->selected == selected) return;

   _item_select_unselect(it, selected);
}

EAPI Eina_Bool
elm_genlist_item_selected_get(const Elm_Object_Item *it)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_Gen_Item *)it)->selected;
}

static Elm_Gen_Item *
_elm_genlist_expanded_next_item_get(Elm_Gen_Item *it)
{
   Elm_Gen_Item *it2;

   if (it->item->expanded)
     {
        it2 = (Elm_Gen_Item *)elm_genlist_item_next_get((Elm_Object_Item *)it);
     }
   else
     {
        it2 = (Elm_Gen_Item *)elm_genlist_item_next_get((Elm_Object_Item *)it);
        while (it2)
          {
             if (it->item->expanded_depth >= it2->item->expanded_depth) break;
             it2 = (Elm_Gen_Item *)
               elm_genlist_item_next_get((Elm_Object_Item *)it2);
          }
     }
   return it2;
}

static Evas_Object *
_tray_alpha_bg_create(const Evas_Object *obj)
{
   Evas_Object *bg = NULL;
   Evas_Coord ox, oy, ow, oh;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);
   bg = evas_object_rectangle_add
       (evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   evas_object_color_set(bg, 0, 0, 0, 0);
   evas_object_resize(bg, ow, oh);
   evas_object_move(bg, ox, oy);

   return bg;
}

EAPI void
elm_genlist_item_expanded_set(Elm_Object_Item *item,
                              Eina_Bool expanded)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);

   expanded = !!expanded;
   if (it->item->expanded == expanded) return;
   if (it->item->type != ELM_GENLIST_ITEM_TREE) return;
   it->item->expanded = expanded;
   GL_IT(it)->wsd->expanded_item = it;
   GL_IT(it)->wsd->expanded_next_item =
     _elm_genlist_expanded_next_item_get(it);

   if (it->item->expanded)
     {
        if (it->realized)
          edje_object_signal_emit(VIEW(it), "elm,state,expanded", "elm");
        evas_object_smart_callback_call(WIDGET(it), SIG_EXPANDED, it);
        GL_IT(it)->wsd->auto_scroll_enabled = EINA_TRUE;
     }
   else
     {
        if (it->realized)
          edje_object_signal_emit(VIEW(it), "elm,state,contracted", "elm");
        evas_object_smart_callback_call(WIDGET(it), SIG_CONTRACTED, it);
        GL_IT(it)->wsd->auto_scroll_enabled = EINA_FALSE;
     }
}

EAPI Eina_Bool
elm_genlist_item_expanded_get(const Elm_Object_Item *it)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_Gen_Item *)it)->item->expanded;
}

EAPI int
elm_genlist_item_expanded_depth_get(const Elm_Object_Item *it)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(it, 0);

   return ((Elm_Gen_Item *)it)->item->expanded_depth;
}

static Eina_Bool
_elm_genlist_item_coordinates_calc(Elm_Object_Item *item,
                                   Elm_Genlist_Item_Scrollto_Type type,
                                   Eina_Bool bring_in,
                                   Evas_Coord *x,
                                   Evas_Coord *y,
                                   Evas_Coord *w,
                                   Evas_Coord *h)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   Evas_Coord gith = 0;

   if (!((GL_IT(it)->wsd->homogeneous) &&
         (GL_IT(it)->wsd->mode == ELM_LIST_COMPRESS)))
     {
        if ((it->item->queued) || (!it->item->mincalcd))
          {
             GL_IT(it)->wsd->show_item = it;
             GL_IT(it)->wsd->bring_in = bring_in;
             GL_IT(it)->wsd->scroll_to_type = type;
             it->item->show_me = EINA_TRUE;
             return EINA_FALSE;
          }
     }
   if (GL_IT(it)->wsd->show_item)
     {
        GL_IT(it)->wsd->show_item->item->show_me = EINA_FALSE;
        GL_IT(it)->wsd->show_item = NULL;
     }

   evas_object_geometry_get(GL_IT(it)->wsd->pan_obj, NULL, NULL, w, h);
   switch (type)
     {
      case ELM_GENLIST_ITEM_SCROLLTO_IN:
        if ((it->item->group_item) &&
            (GL_IT(it)->wsd->pan_y > (it->y + it->item->block->y)))
          gith = it->item->group_item->item->h;
        *h = it->item->h;
        *y = it->y + it->item->block->y - gith;
        break;

      case ELM_GENLIST_ITEM_SCROLLTO_TOP:
        if (it->item->group_item) gith = it->item->group_item->item->h;
        *y = it->y + it->item->block->y - gith;
        break;

      case ELM_GENLIST_ITEM_SCROLLTO_MIDDLE:
        *y = it->y + it->item->block->y - (*h / 2) + (it->item->h / 2);
        break;

      default:
        return EINA_FALSE;
     }

   *x = it->x + it->item->block->x;
   *w = it->item->block->w;

   return EINA_TRUE;
}

EAPI void
elm_genlist_item_promote(Elm_Object_Item *item)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   _item_move_before
     (it, (Elm_Gen_Item *)elm_genlist_first_item_get(WIDGET(it)));
}

EAPI void
elm_genlist_item_demote(Elm_Object_Item *item)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   _item_move_after(it, (Elm_Gen_Item *)elm_genlist_last_item_get(WIDGET(it)));
}

EAPI void
elm_genlist_item_show(Elm_Object_Item *item,
                      Elm_Genlist_Item_Scrollto_Type type)
{
   Evas_Coord x, y, w, h;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   if (_elm_genlist_item_coordinates_calc
         (item, type, EINA_FALSE, &x, &y, &w, &h))
     GL_IT(it)->wsd->s_iface->content_region_show(WIDGET(item), x, y, w, h);
}

EAPI void
elm_genlist_item_bring_in(Elm_Object_Item *item,
                          Elm_Genlist_Item_Scrollto_Type type)
{
   Evas_Coord x, y, w, h;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   if (_elm_genlist_item_coordinates_calc
         (item, type, EINA_TRUE, &x, &y, &w, &h))
     GL_IT(it)->wsd->s_iface->region_bring_in(WIDGET(item), x, y, w, h);
}

EAPI void
elm_genlist_item_all_contents_unset(Elm_Object_Item *item,
                                    Eina_List **l)
{
   Evas_Object *content;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   EINA_LIST_FREE (it->content_objs, content)
     {
        evas_object_smart_member_del(content);
        evas_object_hide(content);
        if (l) *l = eina_list_append(*l, content);
     }
}

EAPI void
elm_genlist_item_update(Elm_Object_Item *item)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   if (!it->item->block) return;
   it->item->mincalcd = EINA_FALSE;
   it->item->updateme = EINA_TRUE;
   it->item->block->updateme = EINA_TRUE;
#if GENLIST_ENTRY_SUPPORT
   it->item->unrealize_disabled = EINA_FALSE;
#endif

#if GENLIST_FX_SUPPORT
     _elm_genlist_fx_clear(ELM_WIDGET_DATA(GL_IT(it)->wsd)->obj, EINA_FALSE);
#endif

   if (GL_IT(it)->wsd->update_job) ecore_job_del(GL_IT(it)->wsd->update_job);
   GL_IT(it)->wsd->update_job = ecore_job_add(_update_job, GL_IT(it)->wsd);
}

EAPI void
elm_genlist_item_fields_update(Elm_Object_Item *item,
                               const char *parts,
                               Elm_Genlist_Item_Field_Type itf)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   if (!it->item->block) return;

#if GENLIST_ENTRY_SUPPORT
   it->item->unrealize_disabled = EINA_FALSE;
#endif
   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_TEXT))
     {
        _item_text_realize(it, VIEW(it), &it->texts, parts);
     }
   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_CONTENT))
     {
        it->content_objs = _item_content_unrealize(it, VIEW(it),
                                                   &it->contents, parts);
        it->content_objs = _item_content_realize(it, VIEW(it),
                                                 &it->contents, parts);

        if (it->flipped)
          {
             it->item->flip_content_objs =
               _item_mode_content_unrealize(it, VIEW(it),
                                            &it->item->flip_contents, parts,
                                            &it->item->flip_content_objs);
             it->item->flip_content_objs =
               _item_mode_content_realize(it, VIEW(it),
                                          &it->item->flip_contents, parts,
                                          &it->item->flip_content_objs);
          }
        if (it->item->deco_it_view)
          {
             it->item->deco_it_content_objs =
               _item_mode_content_unrealize(it, it->item->deco_it_view,
                                            &it->item->deco_it_contents, parts,
                                            &it->item->deco_it_content_objs);
             it->item->deco_it_content_objs =
               _item_mode_content_realize(it, it->item->deco_it_view,
                                          &it->item->deco_it_contents, parts,
                                          &it->item->deco_it_content_objs);
          }
        if (GL_IT(it)->wsd->decorate_all_mode)
          {
             it->item->deco_all_content_objs =
               _item_mode_content_unrealize
                 (it, it->deco_all_view, &it->item->deco_all_contents, parts,
                 &it->item->deco_all_content_objs);
             it->item->deco_all_content_objs =
               _item_mode_content_realize(it, it->deco_all_view,
                                          &it->item->deco_all_contents, parts,
                                          &it->item->deco_all_content_objs);
          }
     }

   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_STATE))
     _item_state_realize(it, VIEW(it), &it->states, parts);
}

EAPI void
elm_genlist_item_item_class_update(Elm_Object_Item *item,
                                   const Elm_Genlist_Item_Class *itc)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   EINA_SAFETY_ON_NULL_RETURN(itc);
   it->itc = itc;

   if (!it->item->block) return;
   it->item->nocache_once = EINA_TRUE;

   elm_widget_stringlist_free(it->texts);
   it->texts = NULL;
   elm_widget_stringlist_free(it->contents);
   it->contents = NULL;
   elm_widget_stringlist_free(it->states);
   it->states = NULL;

   if (it->flipped)
     {
        elm_widget_stringlist_free(it->item->flip_contents);
        it->item->flip_contents = NULL;
     }
   if (it->item->deco_it_view)
     {
        elm_widget_stringlist_free(it->item->deco_it_texts);
        it->item->deco_it_texts = NULL;
        elm_widget_stringlist_free(it->item->deco_it_contents);
        it->item->deco_it_contents = NULL;
     }
   if (GL_IT(it)->wsd->decorate_all_mode)
     {
        elm_widget_stringlist_free(it->item->deco_all_texts);
        it->item->deco_all_texts = NULL;
        elm_widget_stringlist_free(it->item->deco_all_contents);
        it->item->deco_all_contents = NULL;
     }

   elm_genlist_item_update(item);
}

EAPI const Elm_Genlist_Item_Class *
elm_genlist_item_item_class_get(const Elm_Object_Item *item)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, NULL);
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   return it->itc;
}

static Evas_Object *
_elm_genlist_item_label_create(void *data,
                               Evas_Object *obj __UNUSED__,
                               Evas_Object *tooltip,
                               void *it __UNUSED__)
{
   Evas_Object *label = elm_label_add(tooltip);

   if (!label)
     return NULL;

   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);

   return label;
}

static void
_elm_genlist_item_label_del_cb(void *data,
                               Evas_Object *obj __UNUSED__,
                               void *event_info __UNUSED__)
{
   eina_stringshare_del(data);
}

EAPI void
elm_genlist_item_tooltip_text_set(Elm_Object_Item *it,
                                  const char *text)
{
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(it);

   text = eina_stringshare_add(text);
   elm_genlist_item_tooltip_content_cb_set
     (it, _elm_genlist_item_label_create, text,
     _elm_genlist_item_label_del_cb);
}

EAPI void
elm_genlist_item_tooltip_content_cb_set(Elm_Object_Item *item,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void *data,
                                        Evas_Smart_Cb del_cb)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_GOTO(item, error);

   if ((it->tooltip.content_cb == func) && (it->tooltip.data == data))
     return;

   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);

   it->tooltip.content_cb = func;
   it->tooltip.data = data;
   it->tooltip.del_cb = del_cb;

   if (VIEW(it))
     {
        elm_widget_item_tooltip_content_cb_set
          (it, it->tooltip.content_cb, it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, it->tooltip.style);
        elm_widget_item_tooltip_window_mode_set(it, it->tooltip.free_size);
     }

   return;

error:
   if (del_cb) del_cb((void *)data, NULL, NULL);
}

EAPI void
elm_genlist_item_tooltip_unset(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);

   if ((VIEW(it)) && (it->tooltip.content_cb))
     elm_widget_item_tooltip_unset(it);

   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->tooltip.del_cb = NULL;
   it->tooltip.content_cb = NULL;
   it->tooltip.data = NULL;
   it->tooltip.free_size = EINA_FALSE;
   if (it->tooltip.style)
     elm_genlist_item_tooltip_style_set(item, NULL);
}

EAPI void
elm_genlist_item_tooltip_style_set(Elm_Object_Item *item,
                                   const char *style)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);

   eina_stringshare_replace(&it->tooltip.style, style);
   if (VIEW(it)) elm_widget_item_tooltip_style_set(it, style);
}

EAPI const char *
elm_genlist_item_tooltip_style_get(const Elm_Object_Item *it)
{
   return elm_object_item_tooltip_style_get(it);
}

EAPI Eina_Bool
elm_genlist_item_tooltip_window_mode_set(Elm_Object_Item *item,
                                         Eina_Bool disable)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);

   it->tooltip.free_size = disable;
   if (VIEW(it)) return elm_widget_item_tooltip_window_mode_set(it, disable);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_genlist_item_tooltip_window_mode_get(const Elm_Object_Item *it)
{
   return elm_object_tooltip_window_mode_get(VIEW(it));
}

EAPI void
elm_genlist_item_cursor_set(Elm_Object_Item *item,
                            const char *cursor)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   eina_stringshare_replace(&it->mouse_cursor, cursor);
   if (VIEW(it)) elm_widget_item_cursor_set(it, cursor);
}

EAPI const char *
elm_genlist_item_cursor_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_get(it);
}

EAPI void
elm_genlist_item_cursor_unset(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);

   if (!it->mouse_cursor) return;

   if (VIEW(it)) elm_widget_item_cursor_unset(it);

   eina_stringshare_del(it->mouse_cursor);
   it->mouse_cursor = NULL;
}

EAPI void
elm_genlist_item_cursor_style_set(Elm_Object_Item *it,
                                  const char *style)
{
   elm_widget_item_cursor_style_set(it, style);
}

EAPI const char *
elm_genlist_item_cursor_style_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_style_get(it);
}

EAPI void
elm_genlist_item_cursor_engine_only_set(Elm_Object_Item *it,
                                        Eina_Bool engine_only)
{
   elm_widget_item_cursor_engine_only_set(it, engine_only);
}

EAPI Eina_Bool
elm_genlist_item_cursor_engine_only_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_engine_only_get(it);
}

EAPI int
elm_genlist_item_index_get(const Elm_Object_Item *item)
{
   int cnt = 0;
   Elm_Gen_Item *tmp;
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, -1);

   EINA_INLIST_FOREACH(GL_IT(it)->wsd->items, tmp)
     {
        if (tmp == it) break;
        cnt++;
     }
   return cnt;
}

EAPI void
elm_genlist_mode_set(Evas_Object *obj,
                     Elm_List_Mode mode)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   if (sd->mode == mode) return;
   sd->mode = mode;
   elm_layout_sizing_eval(obj);
}

EAPI Elm_List_Mode
elm_genlist_mode_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) ELM_LIST_LAST;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->mode;
}

EAPI void
elm_genlist_bounce_set(Evas_Object *obj,
                       Eina_Bool h_bounce,
                       Eina_Bool v_bounce)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   sd->h_bounce = !!h_bounce;
   sd->v_bounce = !!v_bounce;
   sd->s_iface->bounce_allow_set(obj, sd->h_bounce, sd->v_bounce);
}

EAPI void
elm_genlist_bounce_get(const Evas_Object *obj,
                       Eina_Bool *h_bounce,
                       Eina_Bool *v_bounce)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   if (h_bounce) *h_bounce = sd->h_bounce;
   if (v_bounce) *v_bounce = sd->v_bounce;
}

EAPI void
elm_genlist_homogeneous_set(Evas_Object *obj,
                            Eina_Bool homogeneous)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   sd->homogeneous = !!homogeneous;
}

EAPI Eina_Bool
elm_genlist_homogeneous_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) EINA_FALSE;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->homogeneous;
}

EAPI void
elm_genlist_block_count_set(Evas_Object *obj,
                            int count)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);
   EINA_SAFETY_ON_TRUE_RETURN(count < 1);

   sd->max_items_per_block = count;
   sd->item_cache_max = sd->max_items_per_block * 2;
   _item_cache_all_free(sd);
}

EAPI int
elm_genlist_block_count_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) 0;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->max_items_per_block;
}

EAPI void
elm_genlist_longpress_timeout_set(Evas_Object *obj,
                                  double timeout)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   sd->longpress_timeout = timeout;
}

EAPI double
elm_genlist_longpress_timeout_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) 0;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->longpress_timeout;
}

EAPI void
elm_genlist_scroller_policy_set(Evas_Object *obj,
                                Elm_Scroller_Policy policy_h,
                                Elm_Scroller_Policy policy_v)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   sd->s_iface->policy_set(obj, policy_h, policy_v);
}

EAPI void
elm_genlist_scroller_policy_get(const Evas_Object *obj,
                                Elm_Scroller_Policy *policy_h,
                                Elm_Scroller_Policy *policy_v)
{
   Elm_Scroller_Policy s_policy_h, s_policy_v;

   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   sd->s_iface->policy_get(obj, &s_policy_h, &s_policy_v);
   if (policy_h) *policy_h = (Elm_Scroller_Policy)s_policy_h;
   if (policy_v) *policy_v = (Elm_Scroller_Policy)s_policy_v;
}

EAPI void
elm_genlist_realized_items_update(Evas_Object *obj)
{
   Eina_List *list, *l;
   Elm_Object_Item *it;

   ELM_GENLIST_CHECK(obj);

   list = elm_genlist_realized_items_get(obj);
   EINA_LIST_FOREACH(list, l, it)
     elm_genlist_item_update(it);
}

EAPI void
elm_genlist_item_decorate_mode_set(Elm_Object_Item *item,
                                   const char *decorate_it_type,
                                   Eina_Bool decorate_it_set)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   Elm_Genlist_Smart_Data *sd;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   sd = GL_IT(it)->wsd;

   if (elm_widget_item_disabled_get(it)) return;
   if (!decorate_it_type) return;
   if (sd->decorate_all_mode) return;

   if ((sd->mode_item == it) &&
       (!strcmp(decorate_it_type, sd->decorate_it_type)) &&
       (decorate_it_set))
     return;
   if (!it->itc->decorate_item_style) return;
   it->decorate_it_set = decorate_it_set;

   _item_unselect(it);
   if (((sd->decorate_it_type)
        && (strcmp(decorate_it_type, sd->decorate_it_type))) ||
       (decorate_it_set) || ((it == sd->mode_item) && (!decorate_it_set)))
     _decorate_item_unset(sd);

   eina_stringshare_replace(&sd->decorate_it_type, decorate_it_type);
   if (decorate_it_set) _decorate_item_set(it);
}

EAPI const char *
elm_genlist_item_decorate_mode_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *i = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, NULL);
   return GL_IT(i)->wsd->decorate_it_type;
}

EAPI Elm_Object_Item *
elm_genlist_decorated_item_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   return (Elm_Object_Item *)sd->mode_item;
}

EAPI Eina_Bool
elm_genlist_decorate_mode_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) EINA_FALSE;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->decorate_all_mode;
}

EAPI void
elm_genlist_decorate_mode_set(Evas_Object *obj,
                              Eina_Bool decorated)
{
   Elm_Gen_Item *it;
   Eina_List *list, *l;

   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   decorated = !!decorated;
   if (sd->decorate_all_mode == decorated) return;
   // decorate_all_mode should be set first
   // because content_get func. will be called after this
   // and user can check whether deocrate_all_mode is enabeld.
   sd->decorate_all_mode = decorated;

   list = elm_genlist_realized_items_get(obj);
   if (!sd->decorate_all_mode)
     {
        EINA_LIST_FOREACH(list, l, it)
          {
             if (it->item->type != ELM_GENLIST_ITEM_GROUP)
               _decorate_all_item_unrealize(it);
          }
        _item_cache_all_free(sd);
     }
   else
     {
        EINA_LIST_FOREACH(list, l, it)
          {
             if (it->item->type != ELM_GENLIST_ITEM_GROUP)
               {
                  if (it->itc->decorate_all_item_style)
                    _decorate_all_item_realize(it, EINA_TRUE);
               }
          }
     }
#if GENLIST_FX_SUPPORT
     _elm_genlist_fx_clear(ELM_WIDGET_DATA(sd)->obj, EINA_FALSE);
#endif
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);
}

EAPI void
elm_genlist_reorder_mode_set(Evas_Object *obj,
                             Eina_Bool reorder_mode)
{
#if 1 // FIXME: difference from upstream
   Eina_List *list, *l;
   Elm_Gen_Item *it;
#endif
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   if (sd->reorder_mode == !!reorder_mode) return;
   sd->reorder_mode = !!reorder_mode;
#if 1 // FIXME: difference from upstream
   list = elm_genlist_realized_items_get(obj);
   EINA_LIST_FOREACH(list, l, it)
     {
        if (it->item->type != ELM_GENLIST_ITEM_GROUP)
          {
             if (sd->reorder_mode)
               edje_object_signal_emit
                 (VIEW(it), "elm,state,reorder,mode_set", "elm");
             else
               edje_object_signal_emit
                 (VIEW(it), "elm,state,reorder,mode_unset", "elm");
          }
        if (sd->decorate_all_mode)
          {
             if (sd->reorder_mode)
               edje_object_signal_emit
                  (it->deco_all_view, "elm,state,reorder,mode_set", "elm");
             else
               edje_object_signal_emit
                  (it->deco_all_view, "elm,state,reorder,mode_unset", "elm");
          }
     }
   eina_list_free(list);
#endif
}

EAPI Eina_Bool
elm_genlist_reorder_mode_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) EINA_FALSE;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->reorder_mode;
}

EAPI Elm_Genlist_Item_Type
elm_genlist_item_type_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, ELM_GENLIST_ITEM_MAX);

   return it->item->type;
}

EAPI Elm_Genlist_Item_Class *
elm_genlist_item_class_new(void)
{
   Elm_Genlist_Item_Class *itc = ELM_NEW(Elm_Genlist_Item_Class);
   EINA_SAFETY_ON_NULL_RETURN_VAL(itc, NULL);

   itc->version = CLASS_ALLOCATED;
   itc->refcount = 1;
   itc->delete_me = EINA_FALSE;

   return itc;
}

EAPI void
elm_genlist_item_class_free(Elm_Genlist_Item_Class *itc)
{
   if (itc && (itc->version == CLASS_ALLOCATED))
     {
        if (!itc->delete_me) itc->delete_me = EINA_TRUE;
        if (itc->refcount > 0) elm_genlist_item_class_unref(itc);
        else
          {
             itc->version = 0;
             free(itc);
          }
     }
}

EAPI void
elm_genlist_item_class_ref(Elm_Genlist_Item_Class *itc)
{
   if (itc && (itc->version == CLASS_ALLOCATED))
     {
        itc->refcount++;
        if (itc->refcount == 0) itc->refcount--;
     }
}

EAPI void
elm_genlist_item_class_unref(Elm_Genlist_Item_Class *itc)
{
   if (itc && (itc->version == CLASS_ALLOCATED))
     {
        if (itc->refcount > 0) itc->refcount--;
        if (itc->delete_me && (!itc->refcount))
          elm_genlist_item_class_free(itc);
     }
}

EAPI void
elm_genlist_item_flip_set(Elm_Object_Item *item,
                          Eina_Bool flip)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;

   flip = !!flip;
   if (it->flipped == flip) return;

   it->item->unrealize_disabled = EINA_FALSE;
   if (flip)
     {
        _item_unselect(it);
        _item_unrealize(it, EINA_FALSE);
        it->flipped = EINA_TRUE;
        it->item->nocache = EINA_TRUE;
     }
   else
     {
        _item_unrealize(it, EINA_FALSE);
        it->flipped = EINA_FALSE;
        it->item->nocache = EINA_FALSE;
     }
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);
}

EAPI Eina_Bool
elm_genlist_item_flip_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);

   return it->flipped;
}

EAPI void
elm_genlist_select_mode_set(Evas_Object *obj,
                            Elm_Object_Select_Mode mode)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   if (mode >= ELM_OBJECT_SELECT_MODE_MAX || (sd->select_mode == mode))
     return;

   sd->select_mode = mode;
   if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY ))
     {
        Eina_List *l, *ll;
        Elm_Gen_Item *it;
        EINA_LIST_FOREACH_SAFE(sd->selected, l, ll, it)
          _item_unselect(it);
     }
}

EAPI Elm_Object_Select_Mode
elm_genlist_select_mode_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) ELM_OBJECT_SELECT_MODE_MAX;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->select_mode;
}

EAPI void
elm_genlist_highlight_mode_set(Evas_Object *obj,
                               Eina_Bool highlight)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   sd->highlight = !!highlight;
}

EAPI Eina_Bool
elm_genlist_highlight_mode_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) EINA_FALSE;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->highlight;
}

EAPI void
elm_genlist_item_select_mode_set(Elm_Object_Item *item,
                                 Elm_Object_Select_Mode mode)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item);
   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;
   if (it->select_mode != mode)
     it->select_mode = mode;

   if ((it->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY ))
      _item_unselect(it);

   if (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
     {
        it->item->mincalcd = EINA_FALSE;
        it->item->updateme = EINA_TRUE;
        if (it->item->block) it->item->block->updateme = EINA_TRUE;
        if (GL_IT(it)->wsd->update_job)
          ecore_job_del(GL_IT(it)->wsd->update_job);
        GL_IT(it)->wsd->update_job =
          ecore_job_add(_update_job, GL_IT(it)->wsd);

        // reset homogeneous item size
        if (GL_IT(it)->wsd->homogeneous)
          {
             if (it->group)
               GL_IT(it)->wsd->group_item_width =
                 GL_IT(it)->wsd->group_item_height = 0;
             else
               GL_IT(it)->wsd->item_width = GL_IT(it)->wsd->item_height = 0;
          }
     }
}

EAPI Elm_Object_Select_Mode
elm_genlist_item_select_mode_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENLIST_ITEM_CHECK_OR_RETURN(item, ELM_OBJECT_SELECT_MODE_MAX);

   return it->select_mode;
}

// will be deprecated
EAPI void
elm_genlist_tree_effect_enabled_set(Evas_Object *obj __UNUSED__,
                                    Eina_Bool enabled __UNUSED__)
{
   ELM_GENLIST_CHECK(obj);
}

// will be deprecated
EAPI Eina_Bool
elm_genlist_tree_effect_enabled_get(const Evas_Object *obj __UNUSED__)
{
   ELM_GENLIST_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Elm_Object_Item *
elm_genlist_nth_item_get(const Evas_Object *obj, unsigned int nth)
{
   Elm_Gen_Item *it = NULL;
   Eina_Accessor *a;
   void *data;

   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);

   if (!sd->items) return NULL;

   a = eina_inlist_accessor_new(sd->items);
   if (!a) return NULL;
   if (eina_accessor_data_get(a, nth, &data))
     it = ELM_GEN_ITEM_FROM_INLIST(data);
   eina_accessor_free(a);
   return (Elm_Object_Item *)it;
}

#if GENLIST_FX_SUPPORT
EAPI void
elm_genlist_fx_mode_set(Evas_Object *obj, Eina_Bool mode)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   sd->fx_mode = mode;
}

EAPI Eina_Bool
elm_genlist_fx_mode_get(const Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) EINA_FALSE;
   ELM_GENLIST_DATA_GET(obj, sd);

   return sd->fx_mode;
}

static void
_elm_genlist_proxy_item_del(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   if ((!it) || (!it->item)) return;

   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   Proxy_Item *pi;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->capture_before_items, l, pi)
      if (pi->it == it) pi->it = NULL;

   EINA_LIST_FOREACH(sd->capture_after_items, l, pi)
      if (pi->it == it) pi->it = NULL;
}

static Proxy_Item *
_elm_genlist_proxy_item_new(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   if ((!it) || (!it->item)) return EINA_FALSE;
   int w, h;

   Proxy_Item *pi = NULL;
   pi = calloc(1, sizeof(Proxy_Item));
   if (!pi) return NULL;

   pi->proxy = evas_object_image_filled_add
      (evas_object_evas_get(ELM_WIDGET_DATA(GL_IT(it)->wsd)->obj));
   if (!pi->proxy)
     {
        free(pi);
        return NULL;
     }
   evas_object_clip_set(pi->proxy, evas_object_clip_get(GL_IT(it)->wsd->pan_obj));
   evas_object_smart_member_add(pi->proxy, GL_IT(it)->wsd->pan_obj);
   evas_object_hide(pi->proxy);

   if ((GL_IT(it)->wsd->decorate_all_mode) && (it->deco_all_view))
     evas_object_image_source_set(pi->proxy, it->deco_all_view);
   else
     evas_object_image_source_set(pi->proxy, VIEW(it));

   GL_IT(it)->has_proxy_it = EINA_TRUE;
   pi->it = it;
   pi->num = it->item->num;
   pi->x = it->item->scrl_x;
   pi->y = it->item->scrl_y;
   pi->w = it->item->w;
   pi->h = it->item->h;

   evas_object_geometry_get(VIEW(it), NULL, NULL, &w, &h);

   if (w <= 0 || h <= 0)
     {
        evas_object_size_hint_min_get(VIEW(it), &w, &h);
        evas_object_size_hint_min_set(pi->proxy, w, h);
     }
   else evas_object_resize(pi->proxy, w, h);

   return pi;
}

static Eina_Bool
_elm_genlist_fx_capture(Evas_Object *obj, int level)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   Item_Block *itb;
   Eina_List *l;
   Eina_Bool done = EINA_FALSE;
   Elm_Gen_Item *it;
   Proxy_Item *pi;
   Evas_Coord ox, oy, ow, oh;

   if (!sd->sorting)
     {
        if ((!sd->rendered) || (sd->fx_playing)) return EINA_FALSE;
        if ((!level) && (sd->fx_first_captured)) return EINA_FALSE;
        if ((level) && (!sd->fx_first_captured)) return EINA_FALSE;
     }
   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);

   if (!level)
     {
        sd->fx_first_captured = EINA_TRUE;
        EINA_LIST_FREE(sd->capture_before_items, pi)
          {
             if ((pi->it) && (GL_IT(pi->it))) GL_IT(pi->it)->has_proxy_it = EINA_FALSE;
             if (pi->proxy) evas_object_smart_member_del(pi->proxy);
             if (pi->proxy) evas_object_del(pi->proxy);
             free(pi);
          }
     }
   else
     {
        EINA_LIST_FREE(sd->capture_after_items, pi)
          {
             if ((pi->it) && (GL_IT(pi->it))) GL_IT(pi->it)->has_proxy_it = EINA_FALSE;
             if (pi->proxy) evas_object_smart_member_del(pi->proxy);
             if (pi->proxy) evas_object_del(pi->proxy);
             free(pi);
          }

        EINA_INLIST_FOREACH(sd->blocks, itb)
          {
             if (itb->realized)
               {
                  EINA_LIST_FOREACH(itb->items, l, it)
                    {
                       if (it->realized && it->item->scrl_y >= oy)
                         {
                            sd->realized_top_item = it;
                            break;
                         }
                    }
               }
             if (sd->realized_top_item) break;
          }
     }

#if GENLIST_PINCH_ZOOM_SUPPORT
   if (sd->pinch_zoom_mode)
     {
        EINA_INLIST_FOREACH(sd->blocks, itb)
          {
             EINA_LIST_FOREACH(itb->items, l, it)
               {

                  if (IS_ROOT_PARENT_IT(it) && it->realized
                      && (GL_IT(it)->scrl_y + GL_IT(it)->h >= oy && GL_IT(it)->scrl_y <= oy + oh))
                    {
                       pi = _elm_genlist_proxy_item_new((Elm_Object_Item *)it);
                       if (!level) sd->capture_before_items = eina_list_append(sd->capture_before_items, pi);
                       else sd->capture_after_items = eina_list_append(sd->capture_after_items, pi);
                    }
               }
          }
        if ((sd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT) && (level)) return EINA_TRUE;
        if ((sd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_EXPAND) && (!level)) return EINA_TRUE;
     }
#endif

   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        if (itb->realized)
          {
             done = EINA_TRUE;
             EINA_LIST_FOREACH(itb->items, l, it)
               {
#if GENLIST_PINCH_ZOOM_SUPPORT
                  if ((sd->pinch_zoom_mode) && (IS_ROOT_PARENT_IT(it))) continue;
#endif
                  if (it->realized)
                    {
                       pi = _elm_genlist_proxy_item_new((Elm_Object_Item *)it);
                       if (!pi) continue;
                       if (!level)
                         sd->capture_before_items = eina_list_append(sd->capture_before_items, pi);
                       else
                         sd->capture_after_items = eina_list_append(sd->capture_after_items, pi);
                    }
               }
          }
        else if (done) break;
     }
   return EINA_TRUE;
}

static Elm_Gen_FX_Item *
_elm_genlist_fx_item_find(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   if (!it) return EINA_FALSE;

   Elm_Genlist_Smart_Data *sd = GL_IT(it)->wsd;
   if (!sd) return NULL;

   Elm_Gen_FX_Item *fi = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->fx_items, l, fi)
     {
        if (fi->it == it) return fi;
     }
   return NULL;
}

static Elm_Gen_FX_Item *
_elm_genlist_fx_item_new(const Proxy_Item *pi)
{
   if ((!pi) || (!pi->it)) return NULL;

   Elm_Gen_FX_Item *fi = NULL;
   fi = calloc(1, sizeof(Elm_Gen_FX_Item));
   if (!fi) return NULL;

   fi->it = pi->it;
   GL_IT(fi->it)->fi = fi;
   fi->proxy = pi->proxy;
   fi->num = pi->num;
   fi->from.x = fi->to.x = pi->x;
   fi->from.y = fi->to.y = pi->y;
   fi->from.w = fi->to.w = pi->w;
   fi->from.h = fi->to.h = pi->h;
   fi->update = EINA_FALSE;

   return fi;
}

static Elm_Object_Item *
_elm_genlist_fx_item_prev_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it;
   it = (Elm_Gen_Item *)item;
   if (!it) return NULL;
   if (GL_IT(it)->wsd->pinch_zoom_mode) return NULL;

   while (it)
     {
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
        if (it) break;
     }

   return (Elm_Object_Item *)it;
}

EAPI Eina_Bool
_elm_genlist_fx_items_make(Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj) EINA_FALSE;
   ELM_GENLIST_DATA_GET(obj, sd);

   Elm_Gen_FX_Item *fi;
   Proxy_Item *pi;
   Eina_List *l;
   Elm_Gen_Item *prev = NULL;
   Evas_Coord ox, oy, ow, oh, mv_dist, max_mv_dist = 0;

   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);

   EINA_LIST_FOREACH(sd->capture_before_items, l, pi)
     {
        fi = _elm_genlist_fx_item_new(pi);
        if (fi) sd->fx_items = eina_list_append(sd->fx_items, fi);
     }

   EINA_LIST_FOREACH(sd->capture_after_items, l, pi)
     {
        fi = _elm_genlist_fx_item_find((Elm_Object_Item *)pi->it);

        if (fi) // pi exists in both before and after capture items
          {
             fi->type = ELM_GEN_ITEM_FX_TYPE_SAME;
             fi->update = EINA_TRUE;
             fi->to.x = pi->x;
             fi->to.y = pi->y;
             fi->to.w = pi->w;
             fi->to.h = pi->h;

             // find the max distance between before and after capture items
             mv_dist = abs(fi->to.y - fi->from.y);
             if (max_mv_dist < mv_dist) max_mv_dist = mv_dist;
          }
        else  // pi only exists in after capture items
          {
             fi = _elm_genlist_fx_item_new(pi);
             if (fi)
               {
                  fi->type = ELM_GEN_ITEM_FX_TYPE_ADD;
                  fi->update = EINA_TRUE;
                  sd->fx_items = eina_list_append(sd->fx_items, fi);
               }
          }
     }

   EINA_LIST_FOREACH(sd->fx_items, l, fi)
     {
        // find deleted items
        if (!fi->update) // pi only exists in before capture items
          {
             fi->type = ELM_GEN_ITEM_FX_TYPE_DEL;
             fi->update = EINA_TRUE;

             prev = (Elm_Gen_Item *)_elm_genlist_fx_item_prev_get((Elm_Object_Item *)fi->it);
             if ((prev) && (GL_IT(prev)->fi))
               {
                  fi->to.y = GL_IT(prev)->fi->to.y + GL_IT(prev)->fi->to.h;
               }
          }
        else if (fi->type == ELM_GEN_ITEM_FX_TYPE_ADD)
          {
             prev = (Elm_Gen_Item *)_elm_genlist_fx_item_prev_get((Elm_Object_Item *)fi->it);
             if ((prev) && (prev->realized) && (GL_IT(prev)->fi))
               {
                  fi->from.y = GL_IT(prev)->fi->from.y + GL_IT(prev)->fi->from.h;
               }
             else
               {
                  if (sd->realized_top_item)
                    {

                       if (fi->num <= sd->realized_top_item->item->num)
                         fi->from.y -= max_mv_dist;
                       else
                         fi->from.y += max_mv_dist;
                    }
               }
          }
     }

   EINA_LIST_FOREACH(sd->fx_items, l, fi)
     {
        if ((GL_IT(fi->it)->items) && (GL_IT(fi->it)->expanded_depth == 0))
          {
#if GENLIST_PINCH_ZOOM_SUPPORT
             if (sd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT)
               {
                  fi->from.y = oy + GL_IT(fi->it)->pan_scrl_y - sd->pinch_pan_y;
               }
#endif
             if (fi->type == ELM_GEN_ITEM_FX_TYPE_DEL)
               {
                  if (sd->realized_top_item)
                    {
                       if (fi->num <= sd->realized_top_item->item->num)
                         fi->to.y = oy - fi->from.h;
                       else
                         fi->to.y = oy + oh;
                    }
               }
          }
     }

   return EINA_TRUE;
}

static Elm_Gen_FX_Item *
_fx_cover_item_get(const Elm_Gen_FX_Item *fi)
{
   Elm_Gen_FX_Item *cover_fi;
   Eina_List *l;
   if (!fi) return NULL;

   Elm_Genlist_Smart_Data *sd = GL_IT(fi->it)->wsd;
   Evas_Coord ox, oy, ow, oh;
   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);

   EINA_LIST_FOREACH(sd->fx_items, l, cover_fi)
     {
        if (fi == cover_fi) continue;
        if (sd->fx_items_deleted)
          {
             if ((cover_fi->from.y >= oy+oh || cover_fi->from.y > fi->to.y) && (cover_fi->from.y - cover_fi->to.y > 0))
               return cover_fi;
          }
        else
          {
             if ((cover_fi->to.y > fi->to.y) && (cover_fi->from.y - cover_fi->to.y < 0))
               return cover_fi;
          }
     }
   return NULL;
}

static void
_item_fx_op(Elm_Transit_Effect *data, Elm_Transit *transit __UNUSED__, double progress __UNUSED__)
{
   Elm_Gen_FX_Item *fi = data, *cover_it;
   Elm_Genlist_Smart_Data *sd = GL_IT(fi->it)->wsd;
   Evas_Coord fi_ox, fi_oy, cover_it_ox, cover_it_oy, ox, oy, ow, oh;

   evas_object_show(VIEW(fi->it));
   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);
   evas_object_geometry_get(fi->proxy, &fi_ox, &fi_oy, NULL, NULL);
   evas_object_move(fi->proxy, ox, fi_oy);
   evas_object_show(fi->proxy);
#if GENLIST_PINCH_ZOOM_SUPPORT
   if ((sd->pinch_zoom_mode) && (IS_ROOT_PARENT_IT(fi->it))) evas_object_raise(fi->proxy);
#endif

   if ((!sd->expanded_next_item) || (sd->expanded_next_item == fi->it)) goto raise_event_block;
   if (fi->type == ELM_GEN_ITEM_FX_TYPE_SAME) goto raise_event_block;

   // find the closest item that cover current item when deleting
   cover_it = _fx_cover_item_get(fi);
   if (cover_it)
     {
        evas_object_geometry_get(cover_it->proxy, &cover_it_ox, &cover_it_oy, NULL, NULL);

        if (sd->fx_items_deleted)
          {
             if (cover_it_oy
                 && (fi_oy + fi->to.h >= cover_it_oy + cover_it->to.h ) && (fi->from.y < cover_it->from.y))
               evas_object_hide(fi->proxy);
          }
        else if (cover_it_oy && (fi_oy >= cover_it_oy))
          evas_object_hide(fi->proxy);
        else
          evas_object_show(fi->proxy);
     }

raise_event_block:
   evas_object_raise(sd->alpha_bg);

   return;
}


static void
_item_fx_done(Elm_Transit_Effect *data, Elm_Transit *transit __UNUSED__)
{
   Elm_Gen_FX_Item *fx_done_it = data;
   Elm_Genlist_Smart_Data *sd;

   if (!fx_done_it || !fx_done_it->it) return;

   sd = GL_IT(fx_done_it->it)->wsd;
   if (!sd) return;

   evas_object_image_source_visible_set(fx_done_it->proxy, EINA_TRUE);
   evas_object_lower(fx_done_it->proxy);

   if ((sd->decorate_all_mode) && (fx_done_it->it->deco_all_view))
     evas_object_move(fx_done_it->it->deco_all_view, fx_done_it->to.x, fx_done_it->to.y);
   else
     evas_object_move(VIEW(fx_done_it->it), fx_done_it->to.x, fx_done_it->to.y);
   GL_IT(fx_done_it->it)->has_proxy_it = EINA_FALSE;

   evas_object_clip_unset(fx_done_it->proxy);
}

static void
_item_fx_del_cb(void *data, Elm_Transit *transit __UNUSED__)
{
   Elm_Gen_FX_Item *fx_done_it = data;
   Elm_Gen_Item *it = NULL;
   Proxy_Item *pi = NULL;
   Eina_List *l;
   Elm_Genlist_Smart_Data *sd;

   if (!fx_done_it || !fx_done_it->it) return;

   sd = GL_IT(fx_done_it->it)->wsd;
   if (!sd) return;

   sd->fx_items = eina_list_remove(sd->fx_items, fx_done_it);
   GL_IT(fx_done_it->it)->fi = NULL;
   free(fx_done_it);

   if (!eina_list_count(sd->fx_items))
     {
        EINA_LIST_FREE (sd->pending_unrealized_items, it)
          {
             if (GL_IT(it)) GL_IT(it)->has_proxy_it = EINA_FALSE;
#if GENLIST_PINCH_ZOOM_SUPPORT
             if ((sd->pinch_zoom_mode) && (IS_ROOT_PARENT_IT(it))) continue;
#endif
             _item_unrealize(it, EINA_FALSE);
          }
        EINA_LIST_FREE (sd->pending_unrealized_decorate_all_items, it)
          {
             if (GL_IT(it)) GL_IT(it)->has_proxy_it = EINA_FALSE;
#if GENLIST_PINCH_ZOOM_SUPPORT
             if ((sd->pinch_zoom_mode) && (IS_ROOT_PARENT_IT(it))) continue;
#endif
             _decorate_all_item_unrealize(it);
          }

        EINA_LIST_FREE (sd->pending_del_items, it)
          {
             _item_free(it);
             _elm_widget_item_free((Elm_Widget_Item *)it);
          }

        EINA_LIST_FOREACH(sd->capture_before_items, l, pi)
           evas_object_hide(pi->proxy);
        EINA_LIST_FOREACH(sd->capture_after_items, l, pi)
           evas_object_hide(pi->proxy);

        sd->fx_playing = EINA_FALSE;
        sd->sorting = EINA_FALSE;
        sd->fx_first_captured = EINA_FALSE;
        evas_object_hide(sd->alpha_bg);

        sd->realized_top_item = NULL;
        sd->pan_changed = EINA_TRUE;
        evas_object_smart_changed(sd->pan_obj);
     }
}

static Eina_Bool
_sorting_effect_animator_cb(void *data)
{
   Elm_Genlist_Smart_Data *sd = data;
   Elm_Gen_FX_Item *fi;
   Eina_List *l;

   Evas_Coord ox, oy, ow, oh;
   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);

   sd->fx_timer = NULL;
   EINA_LIST_FOREACH(sd->fx_items, l, fi)
     {
        if (!fi->changed)
          {
             fi->changed = EINA_TRUE;
             evas_object_resize(fi->proxy, ow, fi->to.h);

             fi->trans = elm_transit_add();
             elm_transit_object_add(fi->trans, fi->proxy);
             evas_object_image_source_visible_set(fi->proxy, EINA_FALSE);
             elm_transit_tween_mode_set(fi->trans, ELM_TRANSIT_TWEEN_MODE_DECELERATE);

             elm_transit_effect_translation_add(fi->trans, fi->to.x, fi->to.y - 30 * elm_config_scale_get(), fi->to.x, fi->to.y);
             elm_transit_effect_color_add(fi->trans,0, 0, 0, 0, 255,255,255,255);

             elm_transit_effect_add(fi->trans, _item_fx_op, fi, _item_fx_done);
             elm_transit_del_cb_set(fi->trans, _item_fx_del_cb, fi);
             elm_transit_duration_set(fi->trans,0.3);
             elm_transit_objects_final_state_keep_set(fi->trans, EINA_FALSE);
             elm_transit_go(fi->trans);

             return ECORE_CALLBACK_RENEW;
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_elm_genlist_fx_play(Evas_Object *obj)
{
   ELM_GENLIST_CHECK(obj);
   ELM_GENLIST_DATA_GET(obj, sd);

   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   Elm_Gen_FX_Item *fi;
   Eina_List *l;

   if (!sd->fx_mode) return;

   EINA_LIST_FREE(sd->fx_items, fi)
     {
        if (fi->trans) elm_transit_del(fi->trans);
        free(fi);
     }
   _item_cache_all_free(sd);

   _elm_genlist_fx_items_make(obj);
   if (!eina_list_count(sd->fx_items) || (sd->queue))
     {
        _elm_genlist_fx_clear(obj, EINA_TRUE);
        return;
     }

   sd->fx_playing = EINA_TRUE;

   if (!sd->alpha_bg) sd->alpha_bg = _tray_alpha_bg_create(obj);
   evas_object_show(sd->alpha_bg);

   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(obj), &cvx, &cvy, &cvw, &cvh);

   if (sd->sorting)
     {
        EINA_LIST_FOREACH(sd->fx_items, l, fi)
           evas_object_image_source_visible_set(fi->proxy, EINA_FALSE);
        if (sd->fx_timer) ecore_timer_del(sd->fx_timer);
        sd->fx_timer = ecore_timer_add(0.05, _sorting_effect_animator_cb, sd);
        return;
     }

   EINA_LIST_FOREACH(sd->fx_items, l, fi)
     {
        if (!fi->proxy) continue;

        if ((fi->from.y <= oy) || (fi->from.y + fi->from.h >= oy + oh))
          {
             if ((sd->decorate_all_mode) && (fi->it->deco_all_view))
               evas_object_move(fi->it->deco_all_view, cvx, fi->to.y);
             else
               evas_object_move(VIEW(fi->it), cvx, fi->to.y);
          }
        else if ((fi->to.y <= oy) || (fi->to.y + fi->to.h >= oy + oh))
          {
             if ((sd->decorate_all_mode) && (fi->it->deco_all_view))
               evas_object_move(fi->it->deco_all_view, cvx, fi->from.y);
             else
               evas_object_move(VIEW(fi->it), cvx, fi->from.y);
          }

        evas_object_resize(fi->proxy, ow, fi->to.h);
        evas_object_show(fi->proxy);

        fi->trans = elm_transit_add();
        elm_transit_object_add(fi->trans, fi->proxy);

        evas_object_image_source_visible_set(fi->proxy, EINA_FALSE);
        elm_transit_tween_mode_set(fi->trans, ELM_TRANSIT_TWEEN_MODE_DECELERATE);

        if (fi->type == ELM_GEN_ITEM_FX_TYPE_SAME)
          {
             evas_object_raise(fi->proxy);
             elm_transit_effect_translation_add(fi->trans, fi->from.x, fi->from.y, fi->to.x, fi->to.y);
          }

        else if (fi->type == ELM_GEN_ITEM_FX_TYPE_ADD)
          {
             elm_transit_effect_translation_add(fi->trans, fi->from.x, fi->from.y, fi->to.x, fi->to.y);
             elm_transit_effect_color_add(fi->trans, 0, 0, 0, 0, 255, 255, 255, 255);
             if ((!sd->pinch_zoom_mode) && (!sd->expanded_item))
               elm_transit_effect_zoom_add(fi->trans, 0.8, 1.0);
          }
        else if (fi->type == ELM_GEN_ITEM_FX_TYPE_DEL)
          {
             _item_unhighlight(fi->it);
             elm_transit_effect_translation_add(fi->trans, fi->from.x, fi->from.y, fi->to.x, fi->to.y);
             elm_transit_effect_color_add(fi->trans, 255, 255, 255, 255, 0, 0, 0, 0);
             if ((!sd->pinch_zoom_mode) && (!sd->expanded_item))
               elm_transit_effect_zoom_add(fi->trans, 1.0, 0.8);
          }
        elm_transit_effect_add(fi->trans, _item_fx_op, fi, _item_fx_done);
        elm_transit_del_cb_set(fi->trans, _item_fx_del_cb, fi);

        elm_transit_duration_set(fi->trans, FX_MOVE_TIME);
        elm_transit_objects_final_state_keep_set(fi->trans, EINA_FALSE);
        elm_transit_go(fi->trans);
     }
}
#endif

#if GENLIST_PINCH_ZOOM_SUPPORT
EAPI Eina_Bool
elm_genlist_pinch_zoom_mode_set(Evas_Object *obj, Elm_Gen_Pinch_Zoom_Mode mode)
{
   ELM_GENLIST_DATA_GET(obj, sd);

   Item_Block *itb;
   Eina_List *l;
   Elm_Gen_Item *it;
   Eina_Bool done = EINA_FALSE;

   if ((sd->queue) || (!sd->rendered)
       || (sd->queue_idle_enterer) || (!sd->fx_mode)) return EINA_FALSE;

   EINA_INLIST_FOREACH(sd->blocks, itb)
     {
        EINA_LIST_FOREACH(itb->items, l, it)
          {

             if (IS_ROOT_PARENT_IT(it))
               {
                  done = EINA_TRUE;
                  break;
               }
          }
        if (done) break;
     }
   if (!done) return EINA_FALSE;

   sd->pinch_zoom_mode = mode;

   _item_cache_all_free(sd);
   _elm_genlist_fx_capture(obj, 0);

   sd->pinch_pan_y = sd->pan_y;

   if (sd->pinch_zoom_mode == ELM_GEN_PINCH_ZOOM_CONTRACT)
     sd->s_iface->content_region_show(obj, 0,0,0,0);

   sd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(sd->pan_obj);

   return EINA_TRUE;
}
#endif

static Eina_List *
eina_list_sort_merge(Eina_List *a, Eina_List *b, Eina_Compare_Cb func)
{
   Eina_List *first, *last;

   if (func(a->data, b->data) > 0)
     {
        _item_move_after(a->data, b->data);
        a = (last = first = a)->next;

     }
   else
     b = (last = first = b)->next;

   while (a && b)
     if (func(a->data, b->data) > 0)
       {
          _item_move_after(a->data, b->data);
          a = (last = last->next = a)->next;
       }
     else
       b = (last = last->next = b)->next;

   last->next = a ? a : b;

   return first;
}

EAPI Eina_List *
elm_genlist_sort(Evas_Object *obj, Eina_Compare_Cb func)
{
   ELM_GENLIST_CHECK(obj) NULL;
   ELM_GENLIST_DATA_GET(obj, sd);
   Eina_List *list = NULL;
   Elm_Gen_Item  *it, *next;
   unsigned int limit = 0, i = 0, n = 0;
   Eina_List *tail = NULL, *unsort = NULL, *stack[32], *prev = NULL;

   sd->sorting = EINA_TRUE;

   it = (Elm_Gen_Item*)elm_genlist_first_item_get(obj);
   while(1)
     {
        list = eina_list_append(list, it);
        next = (Elm_Gen_Item*)elm_genlist_item_next_get((Elm_Object_Item *)it);
        if (!next) break;
        it = next;
     }

   if (!list)
     return NULL;

   limit = eina_list_count(list);
   tail = list;

   if ((limit == 0) ||
       (limit > list->accounting->count))
     limit = list->accounting->count;

   if (limit != list->accounting->count)
     {
        unsort = eina_list_nth_list(list, limit);
        if (unsort)
          unsort->prev->next = NULL;
     }

   while (tail)
     {
        unsigned int idx, tmp;

        Eina_List *a = tail;
        Eina_List *b = tail->next;

        if (!b)
          {
             stack[i++] = a;
             break;
          }

        tail = b->next;

        if (func(a->data, b->data) > 0)
          ((stack[i++] = a)->next = b)->next = 0;
        else
             ((stack[i++] = b)->next = a)->next = 0;

        tmp = n++;
        for (idx = n ^ tmp; idx &= idx - 1; i--)
          stack[i - 2] = eina_list_sort_merge(stack[i - 2], stack[i - 1], func);
     }

   while (i-- > 1)
     stack[i - 1] = eina_list_sort_merge(stack[i - 1], stack[i], func);

   list = stack[0];

   for (; stack[0]; stack[0] = stack[0]->next)
     {
        stack[0]->prev = prev;
        prev = stack[0];
     }
   tail = prev;

   if (unsort)
     {
        tail->next = unsort;
        unsort->prev = tail;
     }
   else
     list->accounting->last = tail;

   if (!sd->fx_mode) sd->sorting = EINA_FALSE;
   if (sd->decorate_all_mode) sd->sorting = EINA_FALSE;
   sd->s_iface->content_region_show(obj, 0,0,0,0);
   sd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(sd->pan_obj);

   return list;
}
