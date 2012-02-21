#include <assert.h>
#include <fnmatch.h>
#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "els_scroller.h"

#define SWIPE_MOVES         12
#define MAX_ITEMS_PER_BLOCK 32
#define REORDER_EFFECT_TIME 0.5

typedef struct _Widget_Data Widget_Data;
typedef struct _Item_Block  Item_Block;
typedef struct _Pan         Pan;
typedef struct _Item_Cache  Item_Cache;
typedef enum _Elm_Genlist_Item_Move_effect_Mode
{
   ELM_GENLIST_ITEM_MOVE_EFFECT_NONE         = 0,
   ELM_GENLIST_ITEM_MOVE_EFFECT_EXPAND       = (1 << 0),
   ELM_GENLIST_ITEM_MOVE_EFFECT_CONTRACT     = (1 << 1),
   ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE       = (1 << 2),
} Elm_Genlist_Item_Move_effect_Mode;

typedef enum _Elm_Genlist_Item_Scrollto_Type
{
   ELM_GENLIST_ITEM_SCROLLTO_NONE = 0, /**< no scrollto */
   ELM_GENLIST_ITEM_SCROLLTO_IN = (1 << 0), /**< show, bring in */
   ELM_GENLIST_ITEM_SCROLLTO_TOP = (1 << 1), /**< top show, top bring in */
   ELM_GENLIST_ITEM_SCROLLTO_MIDDLE = (1 << 2) /**< middle show, middle bring in */
} Elm_Genlist_Item_Scrollto_Type;

struct _Widget_Data
{
   Evas_Object      *obj, *scr, *pan_smart;
   Eina_Inlist      *items, *blocks;
   Eina_List        *group_items;
   Pan              *pan;
   Evas_Coord        pan_x, pan_y, old_pan_y, w, h, minw, minh, realminw, prev_viewport_w;
   Ecore_Job        *calc_job, *update_job;
   Ecore_Idle_Enterer *queue_idle_enterer;
   Ecore_Idler        *must_recalc_idler;
   Eina_List        *queue, *selected;
   Elm_Object_Item  *last_selected_item;
   Elm_Genlist_Item *show_item, *anchor_item, *mode_item, *reorder_it, *reorder_rel, *expanded_item;
   Eina_Inlist      *item_cache;
   Evas_Coord        anchor_y, reorder_start_y;
   Elm_List_Mode     mode;
   Ecore_Timer      *multi_timer, *scr_hold_timer;
   Ecore_Animator   *reorder_move_animator;
   const char       *mode_type;
   unsigned int      start_time;
   Evas_Coord        prev_x, prev_y, prev_mx, prev_my;
   Evas_Coord        cur_x, cur_y, cur_mx, cur_my;
   Eina_Bool         mouse_down : 1;
   Eina_Bool         multi_down : 1;
   Eina_Bool         multi_timeout : 1;
   Eina_Bool         multitouched : 1;
   Eina_Bool         on_hold : 1;
   Eina_Bool         multi : 1;
   Eina_Bool         always_select : 1;
   Eina_Bool         longpressed : 1;
   Eina_Bool         wasselected : 1;
   Eina_Bool         no_select : 1;
   Eina_Bool         bring_in : 1; /* a flag to describe the scroll animation. (show, bring in) */
   Eina_Bool         compress : 1;
   Eina_Bool         height_for_width : 1;
   Eina_Bool         pan_resize : 1;
   Eina_Bool         homogeneous : 1;
   Eina_Bool         clear_me : 1;
   Eina_Bool         swipe : 1;
   Eina_Bool         reorder_mode : 1;
   Eina_Bool         reorder_pan_move : 1;
   struct
   {
      Evas_Coord x, y;
   } history[SWIPE_MOVES];
   int               multi_device;
   int               item_cache_count;
   int               item_cache_max;
   int               movements;
   int               walking;
   int               item_width;
   int               item_height;
   int               group_item_width;
   int               group_item_height;
   int               max_items_per_block;
   double            longpress_timeout;

   // TODO : refactoring
   Eina_Bool         reorder_deleted : 1;
   Eina_Bool         effect_mode : 1;
   Eina_Bool         auto_scroll_enabled : 1;
   Eina_Bool         pan_changed : 1;
   Eina_Bool         drag_started : 1;
   Eina_Bool         check_scroll : 1; /* this flag means genlist is supposed to be scrolled. if this flag is set to EINA_TRUE, genlist checks whether it's ok to scroll genlist now or not. */
   Eina_Bool         expanded_move_effect_set : 1;
   int               edit_mode;
   Elm_Genlist_Item_Scrollto_Type scrollto_type; /* a scrollto type which remembers where to scroll ex) in, top, middle */
   Ecore_Animator   *item_moving_effect_timer;
   Evas_Object      *alpha_bg;
   Evas_Coord        expand_item_end;
   Evas_Coord        expand_item_gap;
   int               move_effect_mode;
   Ecore_Job        *changed_job;
   Elm_Genlist_Item *rename_it, *expanded_next_item;
};

struct _Item_Block
{
   EINA_INLIST;
   int          count;
   int          num;
   int          reorder_offset;
   Widget_Data *wd;
   Eina_List   *items;
   Evas_Coord   x, y, w, h, minw, minh;
   Eina_Bool    want_unrealize : 1;
   Eina_Bool    realized : 1;
   Eina_Bool    changed : 1;
   Eina_Bool    updateme : 1;
   Eina_Bool    changeme : 1;
   Eina_Bool    showme : 1;
   Eina_Bool    must_recalc : 1;
};

struct _Elm_Genlist_Item
{
   ELM_WIDGET_ITEM;
   EINA_INLIST;
   Widget_Data                  *wd;
   Item_Block                   *block;
   Eina_List                    *items;
   Evas_Coord                    x, y, w, h, minw, minh;
   const Elm_Genlist_Item_Class *itc;
   Elm_Genlist_Item             *parent;
   Elm_Genlist_Item             *group_item;
   Elm_Genlist_Item_Flags        flags;
   struct
   {
      Evas_Smart_Cb func;
      const void   *data;
   } func;

   Evas_Object                  *spacer;
   Eina_List                    *texts, *contents, *states, *content_objs;
   Eina_List                    *mode_texts, *mode_contents, *mode_states, *mode_content_objs;
   Ecore_Timer                  *long_timer;
   Ecore_Timer                  *swipe_timer;
   Evas_Coord                    dx, dy;
   Evas_Coord                    scrl_x, scrl_y;

   Elm_Genlist_Item             *rel;
   Evas_Object                  *mode_view;

   struct
   {
      const void                 *data;
      Elm_Tooltip_Item_Content_Cb content_cb;
      Evas_Smart_Cb               del_cb;
      const char                 *style;
   } tooltip;

   const char                   *mouse_cursor;

   int                           relcount;
   int                           walking;
   int                           expanded_depth;
   int                           order_num_in;

   Eina_Bool                     before : 1;

   Eina_Bool                     want_unrealize : 1;
   Eina_Bool                     realized : 1;
   Eina_Bool                     selected : 1;
   Eina_Bool                     highlighted : 1;
   Eina_Bool                     expanded : 1;
   Eina_Bool                     disabled : 1;
   Eina_Bool                     display_only : 1;
   Eina_Bool                     mincalcd : 1;
   Eina_Bool                     queued : 1;
   Eina_Bool                     showme : 1;
   Eina_Bool                     delete_me : 1;
   Eina_Bool                     down : 1;
   Eina_Bool                     dragging : 1;
   Eina_Bool                     updateme : 1;
   Eina_Bool                     nocache : 1;
   Eina_Bool                     move_effect_enabled : 1;
   Eina_Bool                     defer_unrealize : 1;
   Eina_Bool                     can_focus : 1;
   Eina_Bool                     no_select : 1;


   // TODO: refactoring
   Eina_Bool   effect_done : 1;
   Eina_Bool   reordering : 1;
   Eina_Bool   renamed : 1;
   Eina_Bool   effect_item_realized : 1;
   Eina_List  *edit_content_objs;
   Evas_Object *edit_obj;
   int         num;
   Ecore_Animator *item_moving_effect_timer;
   Evas_Coord  old_scrl_y;
};

struct _Item_Cache
{
   EINA_INLIST;

   Evas_Object *base_view, *spacer;

   const char  *item_style; // it->itc->item_style
   Eina_Bool    tree : 1; // it->flags & ELM_GENLIST_ITEM_SUBITEMS
   Eina_Bool    compress : 1; // it->wd->compress
   Eina_Bool    odd : 1; // in & 0x1

   Eina_Bool    selected : 1; // it->selected
   Eina_Bool    disabled : 1; // it->disabled
   Eina_Bool    expanded : 1; // it->expanded
};

#define ELM_GENLIST_ITEM_FROM_INLIST(item) \
  ((item) ? EINA_INLIST_CONTAINER_GET(item, Elm_Genlist_Item) : NULL)

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data                   *wd;
   Ecore_Job                     *resize_job;
};

static const char *widtype = NULL;
static void      _item_cache_zero(Widget_Data *wd);
static void      _del_hook(Evas_Object *obj);
static void      _mirrored_set(Evas_Object *obj,
                               Eina_Bool    rtl);
static void      _theme_hook(Evas_Object *obj);
static void      _show_region_hook(void        *data,
                                   Evas_Object *obj);
static void      _sizing_eval(Evas_Object *obj);
static void      _item_realize(Elm_Genlist_Item *it,
                               int               in,
                               Eina_Bool         calc);
static void      _item_unrealize(Elm_Genlist_Item *it,
                                 Eina_Bool         calc);
static void      _item_block_unrealize(Item_Block *itb);
static void      _calc_job(void *data);
static void      _on_focus_hook(void        *data,
                                Evas_Object *obj);
static void      _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void      _changed_job(void *data);
static Eina_Bool _item_multi_select_up(Widget_Data *wd);
static Eina_Bool _item_multi_select_down(Widget_Data *wd);
static Eina_Bool _item_single_select_up(Widget_Data *wd);
static Eina_Bool _item_single_select_down(Widget_Data *wd);
static Eina_Bool _event_hook(Evas_Object       *obj,
                             Evas_Object       *src,
                             Evas_Callback_Type type,
                             void              *event_info);
static void      _signal_emit_hook(Evas_Object *obj,
                                   const char *emission,
                                   const char *source);
static Eina_Bool _deselect_all_items(Widget_Data *wd);
static void      _pan_calculate(Evas_Object *obj);
static void      _pan_max_get(Evas_Object *obj,
                              Evas_Coord  *x,
                              Evas_Coord  *y);
static void      _item_position(Elm_Genlist_Item *it,
                                Evas_Object      *obj,
                                Evas_Coord        it_x,
                                Evas_Coord        it_y);
static void      _mode_item_realize(Elm_Genlist_Item *it);
static void      _mode_item_unrealize(Elm_Genlist_Item *it);
static void      _item_mode_set(Elm_Genlist_Item *it);
static void      _item_mode_unset(Widget_Data *wd);
static void      _item_move_after(Elm_Genlist_Item *it,
                                  Elm_Genlist_Item *after);
static void      _item_move_before(Elm_Genlist_Item *it,
                                   Elm_Genlist_Item *before);

// TODO : refactoring
static Evas_Object* _create_tray_alpha_bg(const Evas_Object *obj);
static unsigned int current_time_get();
static Eina_Bool _item_moving_effect_timer_cb(void *data);
static int _item_flip_effect_show(Elm_Genlist_Item *it);
static void _effect_item_controls(Elm_Genlist_Item *it, int itx, int ity);
static void _effect_item_realize(Elm_Genlist_Item *it, Eina_Bool effect_on);
static void _effect_item_unrealize(Elm_Genlist_Item *it);
static void      _item_auto_scroll(Widget_Data *wd);
static void      _pan_child_size_get(Evas_Object *obj,
                                     Evas_Coord  *w,
                                     Evas_Coord  *h);

static Evas_Smart_Class _pan_sc = EVAS_SMART_CLASS_INIT_VERSION;

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
static const char SIG_SCROLL_EDGE_TOP[] = "scroll,edge,top";
static const char SIG_SCROLL_EDGE_BOTTOM[] = "scroll,edge,bottom";
static const char SIG_SCROLL_EDGE_LEFT[] = "scroll,edge,left";
static const char SIG_SCROLL_EDGE_RIGHT[] = "scroll,edge,right";
static const char SIG_MULTI_SWIPE_LEFT[] = "multi,swipe,left";
static const char SIG_MULTI_SWIPE_RIGHT[] = "multi,swipe,right";
static const char SIG_MULTI_SWIPE_UP[] = "multi,swipe,up";
static const char SIG_MULTI_SWIPE_DOWN[] = "multi,swipe,down";
static const char SIG_MULTI_PINCH_OUT[] = "multi,pinch,out";
static const char SIG_MULTI_PINCH_IN[] = "multi,pinch,in";
static const char SIG_SWIPE[] = "swipe";

static const Evas_Smart_Cb_Description _signals[] = {
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
   {SIG_SCROLL_EDGE_TOP, ""},
   {SIG_SCROLL_EDGE_BOTTOM, ""},
   {SIG_SCROLL_EDGE_LEFT, ""},
   {SIG_SCROLL_EDGE_RIGHT, ""},
   {SIG_MULTI_SWIPE_LEFT, ""},
   {SIG_MULTI_SWIPE_RIGHT, ""},
   {SIG_MULTI_SWIPE_UP, ""},
   {SIG_MULTI_SWIPE_DOWN, ""},
   {SIG_MULTI_PINCH_OUT, ""},
   {SIG_MULTI_PINCH_IN, ""},
   {SIG_SWIPE, ""},
   {NULL, NULL}
};

static Eina_Bool
_event_hook(Evas_Object       *obj,
            Evas_Object       *src __UNUSED__,
            Evas_Callback_Type type,
            void              *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord pan_max_x = 0, pan_max_y = 0;
   if (!wd) return EINA_FALSE;
   if (!wd->items) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   Elm_Object_Item *it = NULL;
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   elm_smart_scroller_child_pos_get(wd->scr, &x, &y);
   elm_smart_scroller_step_size_get(wd->scr, &step_x, &step_y);
   elm_smart_scroller_page_size_get(wd->scr, &page_x, &page_y);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &v_w, &v_h);

   if ((!strcmp(ev->keyname, "Left")) || (!strcmp(ev->keyname, "KP_Left")))
     {
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            (!strcmp(ev->keyname, "KP_Right")))
     {
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) || (!strcmp(ev->keyname, "KP_Up")))
     {
        if (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
             (_item_multi_select_up(wd)))
            || (_item_single_select_up(wd)))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) || (!strcmp(ev->keyname, "KP_Down")))
     {
        if (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
             (_item_multi_select_down(wd)))
            || (_item_single_select_down(wd)))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Home")) || (!strcmp(ev->keyname, "KP_Home")))
     {
        it = elm_genlist_first_item_get(obj);
        elm_genlist_item_bring_in(it);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "End")) || (!strcmp(ev->keyname, "KP_End")))
     {
        it = elm_genlist_last_item_get(obj);
        elm_genlist_item_bring_in(it);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Prior")) || (!strcmp(ev->keyname, "KP_Prior")))
     {
        if (page_y < 0)
          y -= -(page_y * v_h) / 100;
        else
          y -= page_y;
     }
   else if ((!strcmp(ev->keyname, "Next")) || (!strcmp(ev->keyname, "KP_Next")))
     {
        if (page_y < 0)
          y += -(page_y * v_h) / 100;
        else
          y += page_y;
     }
   else if (!strcmp(ev->keyname, "Escape"))
     {
        if (!_deselect_all_items(wd)) return EINA_FALSE;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if (((!strcmp(ev->keyname, "Return")) ||
             (!strcmp(ev->keyname, "KP_Enter")) ||
             (!strcmp(ev->keyname, "space")))
            && (!wd->multi) && (wd->selected))
     {
        it = elm_genlist_selected_item_get(obj);
        elm_genlist_item_expanded_set(it,
                                      !elm_genlist_item_expanded_get(it));
        evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   _pan_max_get(wd->pan_smart, &pan_max_x, &pan_max_y);
   if (x < 0) x = 0;
   if (x > pan_max_x) x = pan_max_x;
   if (y < 0) y = 0;
   if (y > pan_max_y) y = pan_max_y;
   elm_smart_scroller_child_pos_set(wd->scr, x, y);
   return EINA_TRUE;
}

static Eina_Bool
_deselect_all_items(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;
   while (wd->selected)
     elm_genlist_item_selected_set(wd->selected->data, EINA_FALSE);

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_up(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;
   if (!wd->multi) return EINA_FALSE;

   Elm_Object_Item *prev = elm_genlist_item_prev_get(wd->last_selected_item);
   if (!prev) return EINA_TRUE;

   if (elm_genlist_item_selected_get(prev))
     {
        elm_genlist_item_selected_set(wd->last_selected_item, EINA_FALSE);
        wd->last_selected_item = prev;
        elm_genlist_item_show(wd->last_selected_item);
     }
   else
     {
        elm_genlist_item_selected_set(prev, EINA_TRUE);
        elm_genlist_item_show(prev);
     }
   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_down(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;
   if (!wd->multi) return EINA_FALSE;

   Elm_Object_Item *next = elm_genlist_item_next_get(wd->last_selected_item);
   if (!next) return EINA_TRUE;

   if (elm_genlist_item_selected_get(next))
     {
        elm_genlist_item_selected_set(wd->last_selected_item, EINA_FALSE);
        wd->last_selected_item = next;
        elm_genlist_item_show(wd->last_selected_item);
     }
   else
     {
        elm_genlist_item_selected_set(next, EINA_TRUE);
        elm_genlist_item_show(next);
     }
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_up(Widget_Data *wd)
{
   Elm_Genlist_Item *prev;
   if (!wd->selected)
     {
        prev = ELM_GENLIST_ITEM_FROM_INLIST(wd->items->last);
        while ((prev) && (prev->delete_me))
          prev = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
     }
   else prev = (Elm_Genlist_Item *)elm_genlist_item_prev_get(wd->last_selected_item);

   if (!prev) return EINA_FALSE;

   _deselect_all_items(wd);

   elm_genlist_item_selected_set((Elm_Object_Item *)prev, EINA_TRUE);
   elm_genlist_item_show((Elm_Object_Item *)prev);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_down(Widget_Data *wd)
{
   Elm_Genlist_Item *next;
   if (!wd->selected)
     {
        next = ELM_GENLIST_ITEM_FROM_INLIST(wd->items);
        while ((next) && (next->delete_me))
          next = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
     }
   else next = (Elm_Genlist_Item *) elm_genlist_item_next_get(wd->last_selected_item);

   if (!next) return EINA_FALSE;

   _deselect_all_items(wd);

   elm_genlist_item_selected_set((Elm_Object_Item *) next, EINA_TRUE);
   elm_genlist_item_show((Elm_Object_Item *) next);
   return EINA_TRUE;
}

static void
_on_focus_hook(void        *data __UNUSED__,
               Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        elm_object_signal_emit(wd->obj, "elm,action,focus", "elm");
        evas_object_focus_set(wd->obj, EINA_TRUE);
        if ((wd->items) && (wd->selected) && (!wd->last_selected_item))
          wd->last_selected_item = eina_list_data_get(wd->selected);
     }
   else
     {
        elm_object_signal_emit(wd->obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->obj, EINA_FALSE);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _item_cache_zero(wd);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   if (wd->update_job) ecore_job_del(wd->update_job);
   if (wd->queue_idle_enterer) ecore_idle_enterer_del(wd->queue_idle_enterer);
   if (wd->changed_job) ecore_job_del(wd->changed_job);
   if (wd->must_recalc_idler) ecore_idler_del(wd->must_recalc_idler);
   if (wd->multi_timer) ecore_timer_del(wd->multi_timer);
   if (wd->mode_type) eina_stringshare_del(wd->mode_type);
   if (wd->scr_hold_timer) ecore_timer_del(wd->scr_hold_timer);
   free(wd);
}

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->edit_mode) elm_genlist_edit_mode_set(wd->obj, EINA_FALSE);
   elm_genlist_clear(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool    rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _item_cache_zero(wd);
   elm_smart_scroller_mirrored_set(wd->scr, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item_Block *itb;
   if (!wd) return;
   evas_event_freeze(evas_object_evas_get(wd->obj));
   _item_cache_zero(wd);
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   elm_smart_scroller_object_theme_set(obj, wd->scr, "genlist", "base",
                                       elm_widget_style_get(obj));
   //   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   wd->item_width = wd->item_height = 0;
   wd->group_item_width = wd->group_item_height = 0;
   wd->minw = wd->minh = wd->realminw = 0;
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Eina_List *l;
        Elm_Genlist_Item *it;

        if (itb->realized) _item_block_unrealize(itb);
        EINA_LIST_FOREACH(itb->items, l, it)
          it->mincalcd = EINA_FALSE;

        itb->changed = EINA_TRUE;
     }
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
   _sizing_eval(obj);
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_show_region_hook(void        *data,
                  Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   if (!wd) return;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   //x & y are screen coordinates, Add with pan coordinates
   x += wd->pan_x;
   y += wd->pan_y;
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_translate_hook(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "language,changed", NULL);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   evas_object_size_hint_min_get(wd->scr, &minw, &minh);
   evas_object_size_hint_max_get(wd->scr, &maxw, &maxh);
   minh = -1;
   if (wd->height_for_width)
     {
        Evas_Coord vw, vh;

        elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
        if ((vw != 0) && (vw != wd->prev_viewport_w))
          {
             Item_Block *itb;

             wd->prev_viewport_w = vw;
             EINA_INLIST_FOREACH(wd->blocks, itb)
               {
                  itb->must_recalc = EINA_TRUE;
               }
             if (wd->calc_job) ecore_job_del(wd->calc_job);
             wd->calc_job = ecore_job_add(_calc_job, wd);
          }
     }
   if (wd->mode == ELM_LIST_LIMIT)
     {
        Evas_Coord vmw, vmh;

        minw = wd->realminw;
        maxw = -1;
        elm_smart_scroller_child_viewport_size_get(wd->scr, &vmw, &vmh);
        edje_object_size_min_calc
          (elm_smart_scroller_edje_object_get(wd->scr), &vmw, &vmh);
        minw = vmw + minw;
     }
   else
     {
        Evas_Coord vmw, vmh;

        edje_object_size_min_calc
          (elm_smart_scroller_edje_object_get(wd->scr), &vmw, &vmh);
        minw = vmw;
        minh = vmh;
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_signal_emit_hook(Evas_Object *obj,
                  const char  *emission,
                  const char  *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
                           emission, source);
}

static void
_item_highlight(Elm_Genlist_Item *it)
{
   const char *selectraise;
   if ((it->wd->no_select) || (it->no_select) || (it->delete_me) || (it->highlighted) ||
       (it->disabled) || (it->display_only))
     return;
   edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
   if (it->edit_obj) edje_object_signal_emit(it->edit_obj, "elm,state,selected", "elm");
   selectraise = edje_object_data_get(VIEW(it), "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     {
        if (it->edit_obj) evas_object_raise(it->edit_obj);
        else evas_object_raise(VIEW(it));
        if ((it->group_item) && (it->group_item->realized))
          evas_object_raise(it->VIEW(group_item));
     }
   it->highlighted = EINA_TRUE;
}

static void
_item_block_del(Elm_Genlist_Item *it)
{
   Eina_Inlist *il;
   Item_Block *itb = it->block;
   Eina_Bool block_changed;

   itb->items = eina_list_remove(itb->items, it);
   itb->count--;
   itb->changed = EINA_TRUE;
   if (!it->wd->reorder_deleted)
     {
        if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
        it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
     }
   if (itb->count < 1)
     {
        il = EINA_INLIST_GET(itb);
        Item_Block *itbn = (Item_Block *)(il->next);
        if (it->parent)
          it->parent->items = eina_list_remove(it->parent->items, it);
        else
          it->wd->blocks = eina_inlist_remove(it->wd->blocks, il);
        free(itb);
        if (itbn) itbn->changed = EINA_TRUE;
     }
   else
     {
        if (itb->count < itb->wd->max_items_per_block/2)
          {
             il = EINA_INLIST_GET(itb);
             Item_Block *itbp = (Item_Block *)(il->prev);
             Item_Block *itbn = (Item_Block *)(il->next);
             if ((itbp) && ((itbp->count + itb->count) < itb->wd->max_items_per_block + itb->wd->max_items_per_block/2))
               {
                  Elm_Genlist_Item *it2;

                  EINA_LIST_FREE(itb->items, it2)
                    {
                       it2->block = itbp;
                       itbp->items = eina_list_append(itbp->items, it2);
                       itbp->count++;
                       itbp->changed = EINA_TRUE;
                    }
                  it->wd->blocks = eina_inlist_remove(it->wd->blocks,
                                                      EINA_INLIST_GET(itb));
                  free(itb);
                  block_changed = EINA_TRUE;
               }
             else if ((itbn) && ((itbn->count + itb->count) < itb->wd->max_items_per_block + itb->wd->max_items_per_block/2))
               {
                  while (itb->items)
                    {
                       Eina_List *last = eina_list_last(itb->items);
                       Elm_Genlist_Item *it2 = last->data;

                       it2->block = itbn;
                       itb->items = eina_list_remove_list(itb->items, last);
                       itbn->items = eina_list_prepend(itbn->items, it2);
                       itbn->count++;
                       itbn->changed = EINA_TRUE;
                    }
                  it->wd->blocks =
                    eina_inlist_remove(it->wd->blocks, EINA_INLIST_GET(itb));
                  free(itb);
                  block_changed = EINA_TRUE;
               }
          }
     }
   if (block_changed)
     {
        it->wd->pan_changed = EINA_TRUE;
        evas_object_smart_changed(it->wd->pan_smart);
        if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
        it->wd->calc_job = NULL;
     }
}

static void
_item_subitems_clear(Elm_Genlist_Item *it)
{
   if (!it) return;
   Eina_List *tl = NULL, *l;
   Elm_Object_Item *it2;

   EINA_LIST_FOREACH(it->items, l, it2)
      tl = eina_list_append(tl, it2);

   EINA_LIST_FREE(tl, it2)
     elm_object_item_del(it2);
}

static void
_item_del(Elm_Genlist_Item *it)
{
   Evas_Object *tob = it->wd->obj;

   evas_event_freeze(evas_object_evas_get(tob));
   elm_widget_item_pre_notify_del(it);
   elm_genlist_item_subitems_clear((Elm_Object_Item *) it);
   it->wd->walking -= it->walking;
   if (it->wd->show_item == it) it->wd->show_item = NULL;
   if (it->realized) _item_unrealize(it, EINA_FALSE);
   if (it->effect_item_realized) _effect_item_unrealize(it);
   if (it->block) _item_block_del(it);
   if ((!it->delete_me) && (it->itc->func.del))
     it->itc->func.del((void *)it->base.data, WIDGET(it));
   it->delete_me = EINA_TRUE;
   if (it->queued)
     it->wd->queue = eina_list_remove(it->wd->queue, it);
#ifdef ANCHOR_ITEM
   if (it->wd->anchor_item == it)
     {
        it->wd->anchor_item = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        if (!it->wd->anchor_item)
          it->wd->anchor_item = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
     }
#endif
   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   if (it->parent)
     it->parent->items = eina_list_remove(it->parent->items, it);
   if (it->flags & ELM_GENLIST_ITEM_GROUP)
     it->wd->group_items = eina_list_remove(it->wd->group_items, it);
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->swipe_timer) ecore_timer_del(it->swipe_timer);

   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);

   evas_event_thaw(evas_object_evas_get(tob));
   evas_event_thaw_eval(evas_object_evas_get(tob));

   elm_widget_item_del(it);
}

static void
_item_select(Elm_Genlist_Item *it)
{
   Eina_List *l;
   Evas_Object *obj;
   Evas_Object *parent = WIDGET(it);

   if ((it->wd->no_select) || (it->no_select) || (it->delete_me)) return;
   if (it->selected)
     {
        if (it->wd->always_select) goto call;
        return;
     }
   it->selected = EINA_TRUE;
   it->wd->selected = eina_list_append(it->wd->selected, it);
call:
   evas_object_ref(parent);
   it->walking++;
   it->wd->walking++;
   if (it->wd->last_selected_item &&
       (it != (Elm_Genlist_Item *) it->wd->last_selected_item))
     {
        EINA_LIST_FOREACH(((Elm_Genlist_Item *)it->wd->last_selected_item)->content_objs, l, obj)
          {
             elm_widget_focused_object_clear(obj);
             elm_widget_tree_unfocusable_set(obj, EINA_TRUE);
          }
        ((Elm_Genlist_Item *)it->wd->last_selected_item)->can_focus = EINA_FALSE;
     }
   if (it->func.func) it->func.func((void *)it->func.data, parent, it);
   if (!it->delete_me)
     evas_object_smart_callback_call(parent, SIG_SELECTED, it);
   it->walking--;
   it->wd->walking--;
   if ((it->wd->clear_me) && (!it->wd->walking))
     {
        elm_genlist_clear(parent);
        goto end;
     }
   else
     {
        if ((!it->walking) && (it->delete_me))
          {
             if (!it->relcount) _item_del(it);
             goto end;
          }
     }
   // TODO: Remove 'if'?
   if (it && it->wd) it->wd->last_selected_item = (Elm_Object_Item *) it;
end:
   evas_object_unref(parent);
}

static void
_item_unselect(Elm_Genlist_Item *it)
{
   const char *stacking, *selectraise;

   if ((it->delete_me) || ((!it->highlighted) && (!it->selected))) return;
   edje_object_signal_emit(VIEW(it), "elm,state,unselected", "elm");
   if (it->edit_obj) edje_object_signal_emit(it->edit_obj, "elm,state,unselected", "elm");
   stacking = edje_object_data_get(VIEW(it), "stacking");
   selectraise = edje_object_data_get(VIEW(it), "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     {
        if ((stacking) && (!strcmp(stacking, "below")))
          evas_object_lower(VIEW(it));
     }
   it->highlighted = EINA_FALSE;
   if (it->selected)
     {
        it->selected = EINA_FALSE;
        it->wd->selected = eina_list_remove(it->wd->selected, it);
        evas_object_smart_callback_call(WIDGET(it), SIG_UNSELECTED, it);
     }
}

static Evas_Object *
_item_content_get_hook(Elm_Genlist_Item *it, const char *part)
{
   return edje_object_part_swallow_get(VIEW(it), part);
}

static void
_item_content_set_hook(Elm_Genlist_Item *it, const char *part, Evas_Object *content)
{
   edje_object_part_swallow(VIEW(it), part, content);
}

static Evas_Object *
_item_content_unset_hook(Elm_Genlist_Item *it, const char *part)
{
   Evas_Object *obj;

   obj = edje_object_part_swallow_get(VIEW(it), part);
   if (!obj) return NULL;
   edje_object_part_unswallow(VIEW(it), obj);
   return obj;
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Eina_List *l;
   Evas_Object *obj;
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if (_it->delete_me) return;

   if (_it->selected)
     elm_genlist_item_selected_set(it, EINA_FALSE);

   if (_it->realized)
     {
        if (elm_widget_item_disabled_get(it))
          edje_object_signal_emit(VIEW(_it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(_it), "elm,state,enabled", "elm");
        EINA_LIST_FOREACH(_it->content_objs, l, obj)
          elm_widget_disabled_set(obj, elm_widget_item_disabled_get(_it));
     }
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if ((_it->relcount > 0) || (_it->walking > 0))
     {
        elm_genlist_item_subitems_clear(it);
        if (_it->wd->show_item == _it) _it->wd->show_item = NULL;
        if (_it->block)
          {
             if (_it->realized) _elm_genlist_item_unrealize(_it, EINA_FALSE);
             _it->block->changed = EINA_TRUE;
             if (_it->wd->calc_job) ecore_job_del(_it->wd->calc_job);
             _it->wd->calc_job = ecore_job_add(_calc_job, _it->wd);
          }
        if (_it->parent)
          {
             _it->parent->items =
                eina_list_remove(_it->parent->items, it);
             _it->parent = NULL;
          }
        return EINA_FALSE;
     }
   _item_del(_it);
   return EINA_TRUE;
}

static void
_mouse_move(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *obj,
            void        *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;
   Evas_Coord ox, oy, ow, oh, it_scrl_y, y_pos;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        if (!it->wd->on_hold)
          {
             it->wd->on_hold = EINA_TRUE;
             if ((!it->wd->wasselected) && (!it->renamed))
               _item_unselect(it);
          }
     }
   if (it->wd->multitouched)
     {
        it->wd->cur_x = ev->cur.canvas.x;
        it->wd->cur_y = ev->cur.canvas.y;
        return;
     }
   if ((it->dragging) && (it->down))
     {
        if (it->wd->movements == SWIPE_MOVES) it->wd->swipe = EINA_TRUE;
        else
          {
             it->wd->history[it->wd->movements].x = ev->cur.canvas.x;
             it->wd->history[it->wd->movements].y = ev->cur.canvas.y;
             if (abs((it->wd->history[it->wd->movements].x -
                      it->wd->history[0].x)) > 40)
               it->wd->swipe = EINA_TRUE;
             else
               it->wd->movements++;
          }
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG, it);
        return;
     }
   if ((!it->down) /* || (it->wd->on_hold)*/ || (it->wd->longpressed))
     {
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if ((it->wd->reorder_mode) && (it->wd->reorder_it))
          {
             evas_object_geometry_get(it->wd->pan_smart, &ox, &oy, &ow, &oh);
             it_scrl_y = ev->cur.canvas.y - it->wd->reorder_it->dy;

             if (!it->wd->reorder_start_y)
               it->wd->reorder_start_y = it->block->y + it->y;

             if (it_scrl_y < oy)
               y_pos = oy;
             else if (it_scrl_y + it->wd->reorder_it->h > oy + oh)
               y_pos = oy + oh - it->wd->reorder_it->h;
             else
               y_pos = it_scrl_y;

             if (it->edit_obj)
               _item_position(it, it->edit_obj, it->scrl_x, y_pos);
             else
               _item_position(it, VIEW(it), it->scrl_x, y_pos);

             if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
             it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
          }
        return;
     }
   if (!it->display_only)
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
   minw /= 2;
   minh /= 2;
   if ((adx > minw) || (ady > minh))
     {
        it->dragging = EINA_TRUE;
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if ((!it->wd->wasselected) && (!it->renamed))
          _item_unselect(it);
        if (dy < 0)
          {
             if ((ady > adx) && (!it->wd->drag_started))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_DRAG_START_UP, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(WIDGET(it),
                                                    SIG_DRAG_START_LEFT, it);
                  else
                    evas_object_smart_callback_call(WIDGET(it),
                                                    SIG_DRAG_START_RIGHT, it);
               }
          }
        else
          {
             if ((ady > adx) && (!it->wd->drag_started))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_DRAG_START_DOWN, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(WIDGET(it),
                                                    SIG_DRAG_START_LEFT, it);
                  else
                    evas_object_smart_callback_call(WIDGET(it),
                                                    SIG_DRAG_START_RIGHT, it);
               }
          }
     }
}

static Eina_Bool
_long_press(void *data)
{
   Elm_Genlist_Item *it = data, *it_tmp;
   Eina_List *list, *l;

   it->long_timer = NULL;
   if ((it->disabled) || (it->dragging) || (it->display_only) || (it->wd->rename_it))
     return ECORE_CALLBACK_CANCEL;
   it->wd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(WIDGET(it), SIG_LONGPRESSED, it);
   if ((it->wd->reorder_mode) && (it->flags != ELM_GENLIST_ITEM_GROUP))
     {
        it->wd->reorder_it = it;
        it->wd->reorder_start_y = 0;
        if (it->edit_obj)
          evas_object_raise(it->edit_obj);
        else
          evas_object_raise(VIEW(it));

        elm_smart_scroller_hold_set(it->wd->scr, EINA_TRUE);
        elm_smart_scroller_bounce_allow_set(it->wd->scr, EINA_FALSE, EINA_FALSE);

        list = elm_genlist_realized_items_get(it->wd->obj);
        EINA_LIST_FOREACH(list, l, it_tmp)
          {
             if (it != it_tmp) _item_unselect(it_tmp);
          }
        if (it->items)
          {
             EINA_LIST_FOREACH(it->items, l, it_tmp)
               {
                  if (elm_genlist_item_expanded_get((Elm_Object_Item *) it_tmp))
                    {
                       elm_genlist_item_expanded_set((Elm_Object_Item *)it_tmp,
                                                     EINA_FALSE);
                       return ECORE_CALLBACK_RENEW;
                    }
               }
          }
        if (elm_genlist_item_expanded_get((Elm_Object_Item *) it))
          {
             elm_genlist_item_expanded_set((Elm_Object_Item *) it, EINA_FALSE);
             return ECORE_CALLBACK_RENEW;
          }
        if (it->edit_obj)
          edje_object_signal_emit(it->edit_obj, "elm,action,item,reorder_start", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,reorder,enabled", "elm");
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_swipe(Elm_Genlist_Item *it)
{
   int i, sum = 0;

   if (!it) return;
   if ((it->display_only) || (it->disabled)) return;
   it->wd->swipe = EINA_FALSE;
   for (i = 0; i < it->wd->movements; i++)
     {
        sum += it->wd->history[i].x;
        if (abs(it->wd->history[0].y - it->wd->history[i].y) > 10) return;
     }

   sum /= it->wd->movements;
   if (abs(sum - it->wd->history[0].x) <= 10) return;
   evas_object_smart_callback_call(WIDGET(it), SIG_SWIPE, it);
}

static Eina_Bool
_swipe_cancel(void *data)
{
   Elm_Genlist_Item *it = data;

   if (!it) return ECORE_CALLBACK_CANCEL;
   it->wd->swipe = EINA_FALSE;
   it->wd->movements = 0;
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_multi_cancel(void *data)
{
   Widget_Data *wd = data;

   if (!wd) return ECORE_CALLBACK_CANCEL;
   wd->multi_timeout = EINA_TRUE;
   return ECORE_CALLBACK_RENEW;
}

static void
_multi_touch_gesture_eval(void *data)
{
   Elm_Genlist_Item *it = data;

   it->wd->multitouched = EINA_FALSE;
   if (it->wd->multi_timer)
     {
        ecore_timer_del(it->wd->multi_timer);
        it->wd->multi_timer = NULL;
     }
   if (it->wd->multi_timeout)
     {
        it->wd->multi_timeout = EINA_FALSE;
        return;
     }

   Evas_Coord minw = 0, minh = 0;
   Evas_Coord off_x, off_y, off_mx, off_my;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   off_x = abs(it->wd->cur_x - it->wd->prev_x);
   off_y = abs(it->wd->cur_y - it->wd->prev_y);
   off_mx = abs(it->wd->cur_mx - it->wd->prev_mx);
   off_my = abs(it->wd->cur_my - it->wd->prev_my);

   if (((off_x > minw) || (off_y > minh)) && ((off_mx > minw) || (off_my > minh)))
     {
        if ((off_x + off_mx) > (off_y + off_my))
          {
             if ((it->wd->cur_x > it->wd->prev_x) && (it->wd->cur_mx > it->wd->prev_mx))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_SWIPE_RIGHT, it);
             else if ((it->wd->cur_x < it->wd->prev_x) && (it->wd->cur_mx < it->wd->prev_mx))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_SWIPE_LEFT, it);
             else if (abs(it->wd->cur_x - it->wd->cur_mx) > abs(it->wd->prev_x - it->wd->prev_mx))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_PINCH_OUT, it);
             else
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_PINCH_IN, it);
          }
        else
          {
             if ((it->wd->cur_y > it->wd->prev_y) && (it->wd->cur_my > it->wd->prev_my))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_SWIPE_DOWN, it);
             else if ((it->wd->cur_y < it->wd->prev_y) && (it->wd->cur_my < it->wd->prev_my))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_SWIPE_UP, it);
             else if (abs(it->wd->cur_y - it->wd->cur_my) > abs(it->wd->prev_y - it->wd->prev_my))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_PINCH_OUT, it);
             else
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_PINCH_IN, it);
          }
     }
   it->wd->multi_timeout = EINA_FALSE;
}

static void
_multi_down(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *obj __UNUSED__,
            void        *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Multi_Down *ev = event_info;

   if ((it->wd->multi_device != 0) || (it->wd->multitouched) || (it->wd->multi_timeout)) return;
   it->wd->multi_device = ev->device;
   it->wd->multi_down = EINA_TRUE;
   it->wd->multitouched = EINA_TRUE;
   it->wd->prev_mx = ev->canvas.x;
   it->wd->prev_my = ev->canvas.y;
   if (!it->wd->wasselected) _item_unselect(it);
   it->wd->wasselected = EINA_FALSE;
   it->wd->longpressed = EINA_FALSE;
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
   if (it->swipe_timer)
     {
        ecore_timer_del(it->swipe_timer);
        it->swipe_timer = NULL;
     }
   if (it->wd->on_hold)
     {
        it->wd->swipe = EINA_FALSE;
        it->wd->movements = 0;
        it->wd->on_hold = EINA_FALSE;
     }
}

static void
_multi_up(void        *data,
          Evas        *evas __UNUSED__,
          Evas_Object *obj __UNUSED__,
          void        *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Multi_Up *ev = event_info;

   if (it->wd->multi_device != ev->device) return;
   it->wd->multi_device = 0;
   it->wd->multi_down = EINA_FALSE;
   if (it->wd->mouse_down) return;
   _multi_touch_gesture_eval(data);
}

static void
_multi_move(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *obj __UNUSED__,
            void        *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Multi_Move *ev = event_info;

   if (it->wd->multi_device != ev->device) return;
   it->wd->cur_mx = ev->cur.canvas.x;
   it->wd->cur_my = ev->cur.canvas.y;
}

static void
_mouse_down(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *obj,
            void        *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y;
   Eina_List *l;
   Evas_Object *iobj;

   if (ev->button != 1) return;
   if (!it->can_focus)
     {
        EINA_LIST_FOREACH(it->content_objs, l, iobj)
          {
             elm_widget_tree_unfocusable_set(iobj, EINA_FALSE);
          }
        it->can_focus = EINA_TRUE;
     }
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        it->wd->on_hold = EINA_TRUE;
     }

   it->down = EINA_TRUE;
   it->dragging = EINA_FALSE;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   it->dx = ev->canvas.x - x;
   it->dy = ev->canvas.y - y;
   it->wd->mouse_down = EINA_TRUE;
   if (!it->wd->multitouched)
     {
        it->wd->prev_x = ev->canvas.x;
        it->wd->prev_y = ev->canvas.y;
        it->wd->multi_timeout = EINA_FALSE;
        if (it->wd->multi_timer) ecore_timer_del(it->wd->multi_timer);
        it->wd->multi_timer = ecore_timer_add(1, _multi_cancel, it->wd);
     }
   it->wd->longpressed = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = EINA_TRUE;
   else it->wd->on_hold = EINA_FALSE;
   if (it->wd->on_hold) return;
   it->wd->wasselected = it->selected;
   _item_highlight(it);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     if ((!it->disabled) && (!it->display_only))
       {
          evas_object_smart_callback_call(WIDGET(it), SIG_CLICKED_DOUBLE, it);
          evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
       }
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->swipe_timer) ecore_timer_del(it->swipe_timer);
   it->swipe_timer = ecore_timer_add(0.4, _swipe_cancel, it);
   if (it->realized)
     it->long_timer = ecore_timer_add(it->wd->longpress_timeout, _long_press,
                                      it);
   else
     it->long_timer = NULL;
   it->wd->swipe = EINA_FALSE;
   it->wd->movements = 0;
}

static void
_mouse_up(void        *data,
          Evas        *evas __UNUSED__,
          Evas_Object *obj __UNUSED__,
          void        *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool dragged = EINA_FALSE;

   if (ev->button != 1) return;
   it->down = EINA_FALSE;
   it->wd->mouse_down = EINA_FALSE;
   if (it->wd->multitouched)
     {
        // in single selection modes, some multi touching can make multi highlighted items.
        // if a item not selected and highlighted, it should be unhighlighted.
        if ((!it->wd->multi) && (!it->selected) && (it->highlighted)) _item_unselect(it);
        if (it->wd->multi_down) return;
        _multi_touch_gesture_eval(data);
        return;
     }
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = EINA_TRUE;
   else it->wd->on_hold = EINA_FALSE;
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
   if (it->swipe_timer)
     {
        ecore_timer_del(it->swipe_timer);
        it->swipe_timer = NULL;
     }
   if (it->wd->multi_timer)
     {
        ecore_timer_del(it->wd->multi_timer);
        it->wd->multi_timer = NULL;
        it->wd->multi_timeout = EINA_FALSE;
     }
   if (it->wd->on_hold)
     {
        if (it->wd->swipe) _swipe(data);
        it->wd->longpressed = EINA_FALSE;
        it->wd->on_hold = EINA_FALSE;
        return;
     }
   if ((it->wd->reorder_mode) && (it->wd->reorder_it))
     {
        Evas_Coord it_scrl_y = ev->canvas.y - it->wd->reorder_it->dy;

        if (it->wd->reorder_rel && (it->wd->reorder_it->parent == it->wd->reorder_rel->parent))
          {
             if (it_scrl_y <= it->wd->reorder_rel->scrl_y)
               _item_move_before(it->wd->reorder_it, it->wd->reorder_rel);
             else
               _item_move_after(it->wd->reorder_it, it->wd->reorder_rel);
          }
        else
          {
             if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
             it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
          }
        it->wd->reorder_deleted = EINA_FALSE;
        edje_object_signal_emit(it->edit_obj, "elm,action,item,reorder_end", "elm");
        it->wd->reorder_it = it->wd->reorder_rel = NULL;
        elm_smart_scroller_hold_set(it->wd->scr, EINA_FALSE);
        elm_smart_scroller_bounce_allow_set(it->wd->scr, EINA_FALSE, EINA_TRUE);

     }
   if (it->wd->longpressed)
     {
        it->wd->longpressed = EINA_FALSE;
        if ((!it->wd->wasselected) && (!it->renamed))
          _item_unselect(it);
        it->wd->wasselected = EINA_FALSE;
        return;
     }
   if (dragged)
     {
        if (it->want_unrealize)
          {
             _item_unrealize(it, EINA_FALSE);
             if (it->block->want_unrealize)
               _item_block_unrealize(it->block);
          }
     }
   if ((it->disabled) || (dragged) || (it->display_only)) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (it->wd->multi)
     {
        if (!it->selected)
          {
             _item_highlight(it);
             _item_select(it);
          }
        else _item_unselect(it);
     }
   else
     {
        if (!it->selected)
          {
             Widget_Data *wd = it->wd;
             if (wd)
               {
                  while (wd->selected) _item_unselect(wd->selected->data);
               }
          }
        else
          {
             const Eina_List *l, *l_next;
             Elm_Genlist_Item *it2;

             EINA_LIST_FOREACH_SAFE(it->wd->selected, l, l_next, it2)
               if (it2 != it) _item_unselect(it2);
             //_item_highlight(it);
             //_item_select(it);
          }
        _item_highlight(it);
        _item_select(it);
     }
}

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

static void
_signal_expand_toggle(void        *data,
                      Evas_Object *obj __UNUSED__,
                      const char  *emission __UNUSED__,
                      const char  *source __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (it->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_CONTRACT_REQUEST, it);
   else
     evas_object_smart_callback_call(WIDGET(it), SIG_EXPAND_REQUEST, it);
}

static void
_signal_expand(void        *data,
               Evas_Object *obj __UNUSED__,
               const char  *emission __UNUSED__,
               const char  *source __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (!it->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_EXPAND_REQUEST, it);
}

static void
_signal_contract(void        *data,
                 Evas_Object *obj __UNUSED__,
                 const char  *emission __UNUSED__,
                 const char  *source __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (it->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_CONTRACT_REQUEST, it);
}

static Eina_Bool
_scr_hold_timer_cb(void *data)
{
   if (!data) return ECORE_CALLBACK_CANCEL;
   Widget_Data *wd = data;
   elm_smart_scroller_hold_set(wd->scr, EINA_FALSE);
   wd->scr_hold_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_mode_finished_signal_cb(void        *data,
                         Evas_Object *obj,
                         const char  *emission __UNUSED__,
                         const char  *source __UNUSED__)
{
   if (!data) return;
   if (!obj) return;
   Elm_Genlist_Item *it = data;
   if ((it->delete_me) || (!it->realized) || (!it->mode_view)) return;
   char buf[1024];
   Evas *te = evas_object_evas_get(obj);

   evas_event_freeze(te);
   it->nocache = EINA_FALSE;
   _mode_item_unrealize(it);
   snprintf(buf, sizeof(buf), "elm,state,%s,passive,finished", it->wd->mode_type);
   edje_object_signal_callback_del_full(obj, buf, "elm", _mode_finished_signal_cb, it);
   evas_event_thaw(te);
   evas_event_thaw_eval(te);
}

static void
_item_cache_clean(Widget_Data *wd)
{
   evas_event_freeze(evas_object_evas_get(wd->obj));
   while ((wd->item_cache) && (wd->item_cache_count > wd->item_cache_max))
     {
        Item_Cache *itc;

        itc = EINA_INLIST_CONTAINER_GET(wd->item_cache->last, Item_Cache);
        wd->item_cache = eina_inlist_remove(wd->item_cache,
                                            wd->item_cache->last);
        wd->item_cache_count--;
        if (itc->spacer) evas_object_del(itc->spacer);
        if (itc->base_view) evas_object_del(itc->base_view);
        if (itc->item_style) eina_stringshare_del(itc->item_style);
        free(itc);
     }
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_item_cache_zero(Widget_Data *wd)
{
   int pmax = wd->item_cache_max;
   wd->item_cache_max = 0;
   _item_cache_clean(wd);
   wd->item_cache_max = pmax;
}

static void
_item_cache_add(Elm_Genlist_Item *it)
{
   Item_Cache *itc;

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   if (it->wd->item_cache_max <= 0)
     {
        evas_object_del(VIEW(it));
        VIEW(it) = NULL;
        evas_object_del(it->spacer);
        it->spacer = NULL;
        evas_event_thaw(evas_object_evas_get(it->wd->obj));
        evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
        return;
     }

   it->wd->item_cache_count++;
   itc = calloc(1, sizeof(Item_Cache));
   if (!itc)
     {
        evas_event_thaw(evas_object_evas_get(it->wd->obj));
        evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
        return;
     }
   it->wd->item_cache = eina_inlist_prepend(it->wd->item_cache,
                                            EINA_INLIST_GET(itc));
   itc->spacer = it->spacer;
   it->spacer = NULL;
   itc->base_view = VIEW(it);
   VIEW(it) = NULL;
   evas_object_hide(itc->base_view);
   evas_object_move(itc->base_view, -9999, -9999);
   itc->item_style = eina_stringshare_add(it->itc->item_style);
   if (it->flags & ELM_GENLIST_ITEM_SUBITEMS) itc->tree = 1;
   itc->compress = (it->wd->compress);
   itc->odd = (it->order_num_in & 0x1);
   itc->selected = it->selected;
   itc->disabled = it->disabled;
   itc->expanded = it->expanded;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->swipe_timer)
     {
        ecore_timer_del(it->swipe_timer);
        it->swipe_timer = NULL;
     }
   // FIXME: other callbacks?
   edje_object_signal_callback_del_full(itc->base_view,
                                        "elm,action,expand,toggle",
                                        "elm", _signal_expand_toggle, it);
   edje_object_signal_callback_del_full(itc->base_view, "elm,action,expand",
                                        "elm",
                                        _signal_expand, it);
   edje_object_signal_callback_del_full(itc->base_view, "elm,action,contract",
                                        "elm", _signal_contract, it);
   evas_object_event_callback_del_full(itc->base_view, EVAS_CALLBACK_MOUSE_DOWN,
                                       _mouse_down, it);
   evas_object_event_callback_del_full(itc->base_view, EVAS_CALLBACK_MOUSE_UP,
                                       _mouse_up, it);
   evas_object_event_callback_del_full(itc->base_view, EVAS_CALLBACK_MOUSE_MOVE,
                                       _mouse_move, it);
   evas_object_event_callback_del_full(itc->base_view, EVAS_CALLBACK_MULTI_DOWN,
                                       _multi_down, it);
   evas_object_event_callback_del_full(itc->base_view, EVAS_CALLBACK_MULTI_UP,
                                       _multi_up, it);
   evas_object_event_callback_del_full(itc->base_view, EVAS_CALLBACK_MULTI_MOVE,
                                       _multi_move, it);
   _item_cache_clean(it->wd);
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
}

static Item_Cache *
_item_cache_find(Elm_Genlist_Item *it)
{
   Item_Cache *itc;
   Eina_Bool tree = 0, odd;

   if (it->flags & ELM_GENLIST_ITEM_SUBITEMS) tree = 1;
   odd = (it->order_num_in & 0x1);
   EINA_INLIST_FOREACH(it->wd->item_cache, itc)
     {
        if ((itc->selected) || (itc->disabled) || (itc->expanded))
          continue;
        if ((itc->tree == tree) &&
            (itc->odd == odd) &&
            (itc->compress == it->wd->compress) &&
            (!strcmp(it->itc->item_style, itc->item_style)))
          {
             it->wd->item_cache = eina_inlist_remove(it->wd->item_cache,
                                                     EINA_INLIST_GET(itc));
             it->wd->item_cache_count--;
             return itc;
          }
     }
   return NULL;
}

static void
_item_cache_free(Item_Cache *itc)
{
   if (itc->spacer) evas_object_del(itc->spacer);
   if (itc->base_view) evas_object_del(itc->base_view);
   if (itc->item_style) eina_stringshare_del(itc->item_style);
   free(itc);
}

static const char *
_item_text_hook(Elm_Genlist_Item *it, const char *part)
{
   if (!it->itc->func.text_get) return NULL;
   return edje_object_part_text_get(VIEW(it), part);
}

static void
_content_focused(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;
   if (it) it->defer_unrealize = EINA_TRUE;
}

static void
_content_unfocused(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;
   if (it) it->defer_unrealize = EINA_FALSE;
}

static void
_item_del_hook(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   if (!it) return;
   if (it->selected) it->wd->selected = eina_list_remove(it->wd->selected, it);
   if ((Elm_Genlist_Item *)it->wd->last_selected_item == it)
     it->wd->last_selected_item = NULL;
}

static void
_item_text_realize(Elm_Genlist_Item *it,
                    Evas_Object *target,
                    Eina_List **source,
                    const char *parts)
{
   if (it->itc->func.text_get)
     {
        const Eina_List *l;
        const char *key;

        *source = elm_widget_stringlist_get(edje_object_data_get(target, "texts"));
        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             char *s = it->itc->func.text_get
                ((void *)it->base.data, WIDGET(it), key);

             if (s)
               {
                  edje_object_part_text_set(target, key, s);
                  free(s);
               }
             else
               {
                  edje_object_part_text_set(target, key, "");
               }

          }
     }
}

static Eina_List *
_item_mode_content_unrealize(Elm_Genlist_Item *it,
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
_item_mode_content_realize(Elm_Genlist_Item *it,
                        Evas_Object *target,
                        Eina_List **source,
                        const char *parts,
                        Eina_List **contents_list,
                        const char *contents_part)

{
   Eina_List *res = NULL;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;

        *source = elm_widget_stringlist_get(edje_object_data_get(target, contents_part));
        if (parts && (eina_list_count(*source) != eina_list_count(*contents_list)))
             res = *contents_list;
        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             Evas_Object *ic = it->itc->func.content_get
                ((void *)it->base.data, WIDGET(it), l->data);

             if (ic)
               {
                  res = eina_list_append(res, ic);
                  edje_object_part_swallow(target, key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(WIDGET(it), ic);
                  if (it->mode_view || it->wd->edit_mode)
                    {
                       if (it->disabled)
                         elm_widget_disabled_set(ic, EINA_TRUE);
                    }
                  else if (it->renamed)
                    {
                       // FIXME: if entry calcuates its cursor position correctly and conformant works,
                       //        genlist does not need to handle this focus thing.
                       evas_object_smart_callback_add(ic, "focused", _content_focused, it);
                       evas_object_smart_callback_add(ic, "unfocused", _content_unfocused, it);
                    }
               }
          }
     }

   return res;
}

static Eina_List *
_item_content_unrealize(Elm_Genlist_Item *it,
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
                  evas_object_del(ic);
               }
          }
     }

   return res;
}

static Eina_List *
_item_content_realize(Elm_Genlist_Item *it,
                   Evas_Object *target,
                   Eina_List **source,
                   const char *parts)
{
   Eina_List *res = NULL;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;

        *source = elm_widget_stringlist_get(edje_object_data_get(target, "contents"));
        if (parts && (eina_list_count(*source) != eina_list_count(it->content_objs)))
             res = it->content_objs;
        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             Evas_Object *ic = it->itc->func.content_get
                ((void *)it->base.data, WIDGET(it), key);

             if (ic)
               {
                  res = eina_list_append(res, ic);
                  edje_object_part_swallow(target, key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(WIDGET(it), ic);
                  evas_object_event_callback_add(ic, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, it);
                  if (it->disabled)
                    elm_widget_disabled_set(ic, EINA_TRUE);
               }
          }
     }

   return res;
}

static void
_item_state_realize(Elm_Genlist_Item *it,
                    Evas_Object *target,
                    Eina_List **source,
                    const char *parts)
{
   if (it->itc->func.state_get)
     {
        const Eina_List *l;
        const char *key;
        char buf[4096];

        *source = elm_widget_stringlist_get(edje_object_data_get(target, "states"));
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
_item_realize(Elm_Genlist_Item *it,
              int               in,
              Eina_Bool         calc)
{
   const char *stacking;
   const char *treesize;
   char buf[1024];
   int tsize = 20;
   Item_Cache *itc = NULL;

   if ((it->realized) || (it->delete_me)) return;
   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   it->order_num_in = in;
   if (it->wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE) calc = EINA_FALSE;
   if ((it->nocache) && (!it->renamed))
     it->nocache = EINA_FALSE;
   else
     itc = _item_cache_find(it);
   if (itc && (!it->wd->effect_mode))
     {
        VIEW(it) = itc->base_view;
        itc->base_view = NULL;
        it->spacer = itc->spacer;
        itc->spacer = NULL;
     }
   else
     {
        VIEW(it) = edje_object_add(evas_object_evas_get(WIDGET(it)));
        edje_object_scale_set(VIEW(it),
                              elm_widget_scale_get(WIDGET(it)) *
                              _elm_config->scale);
        evas_object_smart_member_add(VIEW(it), it->wd->pan_smart);
        elm_widget_sub_object_add(WIDGET(it), VIEW(it));

        if (it->flags & ELM_GENLIST_ITEM_SUBITEMS)
          strncpy(buf, "tree", sizeof(buf));
        else strncpy(buf, "item", sizeof(buf));
        if (it->wd->compress)
          strncat(buf, "_compress", sizeof(buf) - strlen(buf));

        if (in & 0x1) strncat(buf, "_odd", sizeof(buf) - strlen(buf));
        strncat(buf, "/", sizeof(buf) - strlen(buf));
        strncat(buf, it->itc->item_style, sizeof(buf) - strlen(buf));

        _elm_theme_object_set(WIDGET(it), VIEW(it), "genlist", buf,
                              elm_widget_style_get(WIDGET(it)));
        edje_object_mirrored_set(VIEW(it),
                                 elm_widget_mirrored_get(WIDGET(it)));
        it->spacer =
          evas_object_rectangle_add(evas_object_evas_get(WIDGET(it)));
        evas_object_color_set(it->spacer, 0, 0, 0, 0);
        elm_widget_sub_object_add(WIDGET(it), it->spacer);
     }

   treesize = edje_object_data_get(VIEW(it), "treesize");
   if (treesize) tsize = atoi(treesize);
   evas_object_size_hint_min_set(it->spacer,
                                 (it->expanded_depth * tsize) * _elm_config->scale, 1);
   edje_object_part_swallow(VIEW(it), "elm.swallow.pad", it->spacer);
   if (!calc)
     {
        edje_object_signal_callback_add(VIEW(it),
                                        "elm,action,expand,toggle",
                                        "elm", _signal_expand_toggle, it);
        edje_object_signal_callback_add(VIEW(it), "elm,action,expand",
                                        "elm", _signal_expand, it);
        edje_object_signal_callback_add(VIEW(it), "elm,action,contract",
                                        "elm", _signal_contract, it);
        stacking = edje_object_data_get(VIEW(it), "stacking");
        if (stacking)
          {
             if (!strcmp(stacking, "below")) evas_object_lower(VIEW(it));
             else if (!strcmp(stacking, "above"))
               evas_object_raise(VIEW(it));
          }
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_DOWN,
                                       _mouse_down, it);
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_UP,
                                       _mouse_up, it);
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_MOVE,
                                       _mouse_move, it);
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MULTI_DOWN,
                                       _multi_down, it);
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MULTI_UP,
                                       _multi_up, it);
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MULTI_MOVE,
                                       _multi_move, it);

        if ((!it->edit_obj) &&(it->wd->edit_mode) &&
            (it->flags != ELM_GENLIST_ITEM_GROUP) && (it->itc->edit_item_style))
          _effect_item_realize(it, EINA_FALSE);

        if (itc)
          {
             if ((it->selected) && (it->selected != itc->selected))
               {
                  edje_object_signal_emit(VIEW(it),
                                          "elm,state,selected", "elm");
                  if (it->edit_obj)
                    edje_object_signal_emit(it->edit_obj,
                                            "elm,state,selected", "elm");
               }
             if ((it->disabled) && (it->disabled != itc->disabled))
               {
                  edje_object_signal_emit(VIEW(it),
                                          "elm,state,disabled", "elm");
                  if (it->edit_obj)
                    edje_object_signal_emit(it->edit_obj,
                                            "elm,state,disabled", "elm");
               }
             if ((it->expanded) && (it->expanded != itc->expanded))
               {
                  edje_object_signal_emit(VIEW(it),
                                          "elm,state,expanded", "elm");
                  if (it->edit_obj)
                    edje_object_signal_emit(it->edit_obj,
                                            "elm,state,expanded", "elm");
               }
          }
        else
          {
             if (it->selected)
               {
                  edje_object_signal_emit(VIEW(it),
                                          "elm,state,selected", "elm");
                  if (it->edit_obj)
                    edje_object_signal_emit(it->edit_obj,
                                            "elm,state,selected", "elm");
               }
             if (it->disabled)
               {
                  edje_object_signal_emit(VIEW(it),
                                          "elm,state,disabled", "elm");
                  if (it->edit_obj)
                    edje_object_signal_emit(it->edit_obj,
                                            "elm,state,disabled", "elm");
               }
             if (it->expanded)
               {
                  edje_object_signal_emit(VIEW(it),
                                          "elm,state,expanded", "elm");
                  if (it->edit_obj)
                    edje_object_signal_emit(it->edit_obj,
                                            "elm,state,expanded", "elm");
               }
          }
     }

   if ((calc) && (it->wd->homogeneous) &&
       ((it->wd->item_width) ||
        ((it->wd->item_width) && (it->wd->group_item_width))))
     {
        /* homogenous genlist shortcut */
        if (!it->mincalcd)
          {
             if (it->flags & ELM_GENLIST_ITEM_GROUP)
               {
                  it->w = it->minw = it->wd->group_item_width;
                  it->h = it->minh = it->wd->group_item_height;
               }
             else
               {
                  it->w = it->minw = it->wd->item_width;
                  it->h = it->minh = it->wd->item_height;
               }
             it->mincalcd = EINA_TRUE;
          }
     }
   else
     {
        _item_text_realize(it, VIEW(it), &it->texts, NULL);
        it->content_objs = _item_content_realize(it, VIEW(it), &it->contents, NULL);
        _item_state_realize(it, VIEW(it), &it->states, NULL);
        if (it->itc->func.content_get) // for rename mode
          {
             const Eina_List *l;
             const char *key;
             if (it->renamed)
               {
                  edje_object_signal_emit(VIEW(it), "elm,state,rename,enabled", "elm");
                  it->contents =
                     elm_widget_stringlist_get(edje_object_data_get(VIEW(it), "renames"));
                  EINA_LIST_FOREACH(it->contents, l, key)
                    {
                       Evas_Object *ic = it->itc->func.content_get
                          ((void *)it->base.data, WIDGET(it), l->data);
                       if (ic)
                         {
                            it->content_objs = eina_list_append(it->content_objs, ic);
                            edje_object_part_swallow(VIEW(it), key, ic);
                            evas_object_show(ic);
                            elm_widget_sub_object_add(WIDGET(it), ic);
                            // FIXME: if entry calcuates its cursor position correctly and conformant works,
                            //        genlist does not need to handle this focus thing.
                            evas_object_smart_callback_add(ic, "focused", _content_focused, it);
                            evas_object_smart_callback_add(ic, "unfocused", _content_unfocused, it);
                         }
                    }
               }
          }
        if (!it->mincalcd || it->wd->pan_resize || ((it->wd->height_for_width) && (it->w != it->minw)))
          {
             Evas_Coord mw = -1, mh = -1;

             if (it->wd->height_for_width) mw = it->wd->w; // TODO: Removed this. This was removed from upstream.

             if (!it->display_only)
               elm_coords_finger_size_adjust(1, &mw, 1, &mh);
             if (it->wd->height_for_width) mw = it->wd->prev_viewport_w;
             edje_object_size_min_restricted_calc(VIEW(it), &mw, &mh, mw,
                                                  mh);
             if (!it->display_only)
               elm_coords_finger_size_adjust(1, &mw, 1, &mh);
             it->w = it->minw = mw;
             it->h = it->minh = mh;
             it->mincalcd = EINA_TRUE;

             if ((!it->wd->group_item_width) && (it->flags == ELM_GENLIST_ITEM_GROUP))
               {
                  it->wd->group_item_width = mw;
                  it->wd->group_item_height = mh;
               }
             else if ((!it->wd->item_width) && (it->flags == ELM_GENLIST_ITEM_NONE))
               {
                  it->wd->item_width = mw;
                  it->wd->item_height = mh;
               }
          }
        if (!calc) evas_object_show(VIEW(it));
     }

   if (it->tooltip.content_cb)
     {
        elm_widget_item_tooltip_content_cb_set(it,
                                               it->tooltip.content_cb,
                                               it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, it->tooltip.style);
     }

   if (it->mouse_cursor)
     elm_widget_item_cursor_set(it, it->mouse_cursor);

   it->realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;

   if (itc) _item_cache_free(itc);
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
   if (!calc)
     evas_object_smart_callback_call(WIDGET(it), SIG_REALIZED, it);
   if ((!calc) && (it->wd->edit_mode) && (it->flags != ELM_GENLIST_ITEM_GROUP))
     {
        if (it->itc->edit_item_style)
          {
             if (!it->edit_obj) _effect_item_realize(it, EINA_FALSE);
             edje_object_message_signal_process(it->edit_obj);
          }
     }
   edje_object_message_signal_process(VIEW(it));
}

static void
_item_unrealize(Elm_Genlist_Item *it,
                Eina_Bool         calc)
{
   Evas_Object *content;

   if (!it->realized) return;
   if (it->wd->reorder_it == it) return;
   if (it->defer_unrealize && !it->updateme) return;
   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   if (!calc)
     evas_object_smart_callback_call(WIDGET(it), SIG_UNREALIZED, it);
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->nocache)
     {
        evas_object_del(VIEW(it));
        VIEW(it) = NULL;
        evas_object_del(it->spacer);
        it->spacer = NULL;
     }
   else
     {
        edje_object_mirrored_set(VIEW(it),
                                 elm_widget_mirrored_get(WIDGET(it)));
        edje_object_scale_set(VIEW(it),
                              elm_widget_scale_get(WIDGET(it))
                              * _elm_config->scale);
        _item_cache_add(it);
     }
   elm_widget_stringlist_free(it->texts);
   it->texts = NULL;
   elm_widget_stringlist_free(it->contents);
   it->contents = NULL;
   elm_widget_stringlist_free(it->states);

   EINA_LIST_FREE(it->content_objs, content)
     evas_object_del(content);

   _mode_item_unrealize(it);
   it->states = NULL;
   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
   if (it->wd->edit_mode) _effect_item_unrealize(it);
}

static Eina_Bool
_item_block_recalc(Item_Block *itb,
                   int         in,
                   Eina_Bool   qadd)
{
   const Eina_List *l;
   Elm_Genlist_Item *it;
   Evas_Coord minw = 0, minh = 0;
   Eina_Bool showme = EINA_FALSE, changed = EINA_FALSE;
   Evas_Coord y = 0;

   evas_event_freeze(evas_object_evas_get(itb->wd->obj));
   itb->num = in;
   EINA_LIST_FOREACH(itb->items, l, it)
     {
        if (it->delete_me) continue;
        showme |= it->showme;
        if (!itb->realized)
          {
             if (qadd)
               {
                  if (!it->mincalcd) changed = EINA_TRUE;
                  if (changed)
                    {
                       _item_realize(it, in, EINA_TRUE);
                       _item_unrealize(it, EINA_TRUE);
                    }
               }
             else
               {
                  _item_realize(it, in, EINA_TRUE);
                  _item_unrealize(it, EINA_TRUE);
               }
          }
        else
          _item_realize(it, in, EINA_FALSE);
        minh += it->minh;
        if (minw < it->minw) minw = it->minw;
        in++;
        it->x = 0;
        it->y = y;
        y += it->h;
     }
   itb->minw = minw;
   itb->minh = minh;
   itb->changed = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(itb->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(itb->wd->obj));
   return showme;
}

static void
_item_block_realize(Item_Block *itb)
{
   if (itb->realized) return;
   itb->realized = EINA_TRUE;
   itb->want_unrealize = EINA_FALSE;
}

static void
_item_block_unrealize(Item_Block *itb)
{
   const Eina_List *l;
   Elm_Genlist_Item *it;
   Eina_Bool dragging = EINA_FALSE;

   if (!itb->realized) return;
   evas_event_freeze(evas_object_evas_get(itb->wd->obj));
   EINA_LIST_FOREACH(itb->items, l, it)
     {
        if (itb->must_recalc || it->flags != ELM_GENLIST_ITEM_GROUP)
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
   evas_event_thaw(evas_object_evas_get(itb->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(itb->wd->obj));
}

static int
_get_space_for_reorder_item(Elm_Genlist_Item *it)
{
   Evas_Coord rox, roy, row, roh, oy, oh;
   Eina_Bool top = EINA_FALSE;
   Elm_Genlist_Item *reorder_it = it->wd->reorder_it;
   if (!reorder_it) return 0;

   evas_object_geometry_get(it->wd->pan_smart, NULL, &oy, NULL, &oh);
   evas_object_geometry_get(it->wd->VIEW(reorder_it), &rox, &roy, &row, &roh);

   if ((it->wd->reorder_start_y < it->block->y) &&
       (roy - oy + (roh / 2) >= it->block->y - it->wd->pan_y))
     {
        it->block->reorder_offset = it->wd->reorder_it->h * -1;
        if (it->block->count == 1)
          it->wd->reorder_rel = it;
     }
   else if ((it->wd->reorder_start_y >= it->block->y) &&
            (roy - oy + (roh / 2) <= it->block->y - it->wd->pan_y))
     {
        it->block->reorder_offset = it->wd->reorder_it->h;
     }
   else
     it->block->reorder_offset = 0;

   it->scrl_y += it->block->reorder_offset;

   top = (ELM_RECTS_INTERSECT(it->scrl_x, it->scrl_y, it->w, it->h,
                              rox, roy + (roh / 2), row, 1));
   if (top)
     {
        it->wd->reorder_rel = it;
        it->scrl_y += it->wd->reorder_it->h;
        return it->wd->reorder_it->h;
     }
   else
     return 0;
}

static Eina_Bool
_reorder_move_animator_cb(void *data)
{
   Elm_Genlist_Item *it = data;
   Eina_Bool down = EINA_FALSE;
   double t;
   int y, dy = it->h / 10 * _elm_config->scale, diff;

   t = ((0.0 > (t = ecore_loop_time_get()-it->wd->start_time)) ? 0.0 : t);

   if (t <= REORDER_EFFECT_TIME) y = (1 * sin((t / REORDER_EFFECT_TIME) * (M_PI / 2)) * dy);
   else y = dy;

   diff = abs(it->old_scrl_y - it->scrl_y);
   if (diff > it->h) y = diff / 2;
   if (it->old_scrl_y < it->scrl_y)
     {
        it->old_scrl_y += y;
        down = EINA_TRUE;
     }
   else if (it->old_scrl_y > it->scrl_y)
     {
        it->old_scrl_y -= y;
        down = EINA_FALSE;
     }

   if (it->edit_obj)
     _item_position(it, it->edit_obj, it->scrl_x, it->old_scrl_y);
   else
     _item_position(it, VIEW(it), it->scrl_x, it->old_scrl_y);
   if ((it->wd->reorder_pan_move) ||
       (down && it->old_scrl_y >= it->scrl_y) ||
       (!down && it->old_scrl_y <= it->scrl_y))
     {
        it->old_scrl_y = it->scrl_y;
        it->move_effect_enabled = EINA_FALSE;
        it->wd->reorder_move_animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_item_position(Elm_Genlist_Item *it,
               Evas_Object      *view,
               Evas_Coord        it_x,
               Evas_Coord        it_y)
{
   if (!it) return;
   if (!view) return;

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   evas_object_resize(view, it->w, it->h);
   evas_object_move(view, it_x, it_y);
   evas_object_show(view);
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
}

static void
_item_block_position(Item_Block *itb,
                     int         in)
{
   const Eina_List *l;
   Elm_Genlist_Item *it;
   Evas_Coord y = 0, ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   Evas_Coord minh = 0;
   int vis = 0;

   evas_event_freeze(evas_object_evas_get(itb->wd->obj));
   evas_object_geometry_get(itb->wd->pan_smart, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(itb->wd->obj), &cvx, &cvy,
                            &cvw, &cvh);
   EINA_LIST_FOREACH(itb->items, l, it)
     {
        if (it->delete_me) continue;
        else if (it->wd->reorder_it == it) continue;

        it->x = 0;
        it->y = y;
        it->w = itb->w;
        it->scrl_x = itb->x + it->x - it->wd->pan_x + ox;
        it->scrl_y = itb->y + it->y - it->wd->pan_y + oy;

        vis = (ELM_RECTS_INTERSECT(it->scrl_x, it->scrl_y, it->w, it->h,
                                   cvx, cvy, cvw, cvh));
        if ((itb->realized))
          {
             if (vis)
               {
                  if (!it->realized) _item_realize(it, in, EINA_FALSE);
               }
          }
        if (it->realized)
          {
             if (vis)
               {
                  if (it->wd->reorder_mode)
                    y += _get_space_for_reorder_item(it);
                  if ((it->wd->reorder_it) && (it->old_scrl_y != it->scrl_y))
                    {
                       if (!it->move_effect_enabled)
                         {
                            it->move_effect_enabled = EINA_TRUE;
                            it->wd->reorder_move_animator =
                               ecore_animator_add(
                                  _reorder_move_animator_cb, it);
                         }
                    }
                  if (!it->move_effect_enabled)
                    if (!it->wd->effect_mode || it->wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_NONE || ((it->wd->move_effect_mode != ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE) && it->parent == it->wd->expanded_item))
                      {
                         if (it->wd->edit_mode && it->itc->edit_item_style)
                           {
                              _item_position(it, it->edit_obj, it->scrl_x, it->scrl_y);
                           }
                         else
                           {
                              if((!it->wd->effect_mode || it->wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_NONE) || ((it->wd->move_effect_mode != ELM_GENLIST_ITEM_MOVE_EFFECT_NONE) && (it->old_scrl_y == it->scrl_y)))
                                {
                                   if (it->mode_view)
                                     _item_position(it, it->mode_view, it->scrl_x,
                                                    it->scrl_y);
                                   else
                                     _item_position(it, VIEW(it), it->scrl_x,
                                                    it->scrl_y);
                                }
                              else
                                {
                                   evas_object_resize(VIEW(it), it->w, it->h);
                                   evas_object_move(VIEW(it), it->scrl_x, it->scrl_y);
                                   evas_object_hide(VIEW(it));
                                }
                           }
                      }
               }
             else
               {
                  if (!it->dragging && (!it->wd->item_moving_effect_timer) || (it->wd->expanded_item->block != itb)) _item_unrealize(it, EINA_FALSE);
               }
          }
        in++;
        if (!it->wd->effect_mode || it->wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_NONE || ((it->wd->move_effect_mode != ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE) && it->parent == it->wd->expanded_item))
          {
             it->old_scrl_y = it->scrl_y;
          }
        y += it->h;
        minh += it->minh;
     }
   if (!itb->wd->reorder_it) itb->minh = minh;
   evas_event_thaw(evas_object_evas_get(itb->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(itb->wd->obj));
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;
   if (!it) return;
   it->mincalcd = EINA_FALSE;
   it->block->changeme = EINA_TRUE;
   if (it->wd->changed_job) ecore_job_del(it->wd->changed_job);
   it->wd->changed_job = ecore_job_add(_changed_job, it->wd);
}

static Eina_Bool
_must_recalc_idler(void *data)
{
   Widget_Data *wd = data;
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
   wd->must_recalc_idler = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_scroll_item(Widget_Data *wd)
{
   Elm_Genlist_Item *it = NULL;
   Evas_Coord ow, oh, dx = 0, dy = 0, dw = 0, dh = 0;

   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &ow, &oh);
   it = wd->show_item;
   dx = it->x + it->block->x;
   dy = it->y + it->block->y;
   dw = it->block->w;
   dh = oh;
   switch (wd->scrollto_type)
     {
      case ELM_GENLIST_ITEM_SCROLLTO_TOP:
         break;
      case ELM_GENLIST_ITEM_SCROLLTO_MIDDLE:
         dy += (it->h / 2 - oh / 2);
         break;
      case ELM_GENLIST_ITEM_SCROLLTO_IN:
      default:
         if ((wd->expanded_item) &&
             ((wd->show_item->y + wd->show_item->block->y + wd->show_item->h)
              - (wd->expanded_item->y + wd->expanded_item->block->y) > oh))
           {
              it = wd->expanded_item;
              dx = it->x + it->block->x;
              dy = it->y + it->block->y;
              dw = it->block->w;
           }
         else
           {
              dh = it->h;
           }
         break;
     }
   if (wd->bring_in)
     elm_smart_scroller_region_bring_in(wd->scr, dx, dy, dw, dh);
   else
     elm_smart_scroller_child_region_show(wd->scr, dx, dy, dw, dh);

   it->showme = EINA_FALSE;
   wd->show_item = NULL;
   wd->auto_scroll_enabled = EINA_FALSE;
   wd->check_scroll = EINA_FALSE;
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Item_Block *itb, *chb = NULL;
   Evas_Coord minw = -1, minh = 0, y = 0, ow, dy = 0;
   Evas_Coord pan_w = 0, pan_h = 0;
   int in = 0;
   double t0, t;
   Eina_Bool minw_change = EINA_FALSE;
   Eina_Bool did_must_recalc = EINA_FALSE;
   if (!wd) return;

   t0 = ecore_time_get();
   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &ow, &wd->h);
   if (wd->w != ow)
     wd->w = ow;

   evas_event_freeze(evas_object_evas_get(wd->obj));
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Eina_Bool showme = EINA_FALSE;

        itb->num = in;
        showme = itb->showme;
        itb->showme = EINA_FALSE;
        if (chb)
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
        if ((itb->changed) || ((itb->must_recalc) && (!did_must_recalc)))
          {
             if (itb->must_recalc)
               {
                  Eina_List *l;
                  Elm_Genlist_Item *it;
                  EINA_LIST_FOREACH(itb->items, l, it)
                    if (it->mincalcd) it->mincalcd = EINA_FALSE;
                  itb->changed = EINA_TRUE;
                  if (itb->must_recalc) did_must_recalc = EINA_TRUE;
                  if (itb->realized) _item_block_unrealize(itb);
                  itb->must_recalc = EINA_FALSE;
               }
             showme = _item_block_recalc(itb, in, EINA_FALSE);
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
        if (minw > wd->w)
          {
             minw = wd->w;
             minw_change = EINA_TRUE;
          }
        itb->w = minw;
        itb->h = itb->minh;
        y += itb->h;
        in += itb->count;
        if ((showme) && (wd->show_item) && (!wd->show_item->queued))
          wd->check_scroll = EINA_TRUE;
     }
   if (minw_change)
     {
        EINA_INLIST_FOREACH(wd->blocks, itb)
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
   wd->realminw = minw;
   if (minw < wd->w) minw = wd->w;
   if ((minw != wd->minw) || (minh != wd->minh))
     {
        wd->minw = minw;
        wd->minh = minh;
        if (wd->move_effect_mode != ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE)
           evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
        _sizing_eval(wd->obj);
#ifdef ANCHOR_ITEM
        if ((wd->anchor_item) && (wd->anchor_item->block) && (wd->auto_scroll_enabled))
          {
             Elm_Genlist_Item *it;
             Evas_Coord it_y;

             it = wd->anchor_item;
             it_y = wd->anchor_y;
             elm_smart_scroller_child_pos_set(wd->scr, wd->pan_x,
                                              it->block->y + it->y + it_y);
             wd->anchor_item = it;
             wd->anchor_y = it_y;
          }
#endif
     }
   t = ecore_time_get();
   if (did_must_recalc)
     {
        if (!wd->must_recalc_idler)
          wd->must_recalc_idler = ecore_idler_add(_must_recalc_idler, wd);
     }
   if (wd->check_scroll)
     {
        _pan_child_size_get(wd->pan_smart, &pan_w, &pan_h);
        if (EINA_INLIST_GET(wd->show_item) == wd->items->last)
          wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_IN;
       switch (wd->scrollto_type)
          {
           case ELM_GENLIST_ITEM_SCROLLTO_TOP:
              dy = wd->h;
              break;
           case ELM_GENLIST_ITEM_SCROLLTO_MIDDLE:
              dy = wd->h / 2;
              break;
           case ELM_GENLIST_ITEM_SCROLLTO_IN:
           default:
              dy = 0;
              break;
          }
        if ((pan_w > (wd->show_item->x + wd->show_item->block->x)) &&
            (pan_h > (wd->show_item->y + wd->show_item->block->y + dy)))
          {
             _scroll_item(wd);
          }
     }
   wd->calc_job = NULL;
   evas_object_smart_changed(wd->pan_smart);
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_update_job(void *data)
{
   Widget_Data *wd = data;
   Eina_List *l2;
   Item_Block *itb;
   int num, num0;
   Eina_Bool position = EINA_FALSE, recalc = EINA_FALSE;
   if (!wd) return;
   wd->update_job = NULL;
   num = 0;

   evas_event_freeze(evas_object_evas_get(wd->obj));
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Evas_Coord itminw, itminh;
        Elm_Genlist_Item *it;

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
             if (it->updateme)
               {
                  itminw = it->minw;
                  itminh = it->minh;

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
                  if ((it->minw != itminw) || (it->minh != itminh))
                    recalc = EINA_TRUE;
                  it->updateme = EINA_FALSE;
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
        if (wd->calc_job) ecore_job_del(wd->calc_job);
        wd->calc_job = ecore_job_add(_calc_job, wd);
     }
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_changed_job(void *data)
{
   Widget_Data *wd = data; Eina_List *l2;
   Item_Block *itb;
   int num, num0, position = 0;
   Eina_Bool width_changed = EINA_FALSE, height_changed = EINA_FALSE;
   if (!wd) return;
   wd->changed_job = NULL;
   num = 0;
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Evas_Coord itminw, itminh;
        Elm_Genlist_Item *it;

        if (!itb->changeme)
          {
             num += itb->count;
             if (position)
               _item_block_position(itb, num);
             continue;
          }
        num0 = num;
        width_changed = height_changed = EINA_FALSE;
        EINA_LIST_FOREACH(itb->items, l2, it)
          {
             if ((!it->mincalcd) && (it->realized))
               {
                  Evas_Coord mw = -1, mh = -1;
                  itminw = it->minw;
                  itminh = it->minh;

                  if (it->wd->height_for_width) mw = it->wd->w;
                  if (!it->display_only)
                    elm_coords_finger_size_adjust(1, &mw, 1, &mh);
                  if (it->wd->height_for_width) mw = it->wd->prev_viewport_w;
                  edje_object_size_min_restricted_calc(VIEW(it), &mw, &mh, mw, mh);
                  if (!it->display_only)
                    elm_coords_finger_size_adjust(1, &mw, 1, &mh);
                  if (mw > itb->w)
                    width_changed = EINA_TRUE;
                  else
                    mw = itb->w;
                  it->w = it->minw = mw;
                  it->h = it->minh = mh;
                  it->mincalcd = EINA_TRUE;

                  //if ((it->minw != itminw) || (it->minh != itminh))
                  //if ((it->minh != itminh))
                  //  recalc = 1;
                  if ((it->minw != itminw))
                    width_changed = EINA_TRUE;
                  if ((it->minh != itminh))
                    height_changed = EINA_TRUE;

                  if ((!it->wd->group_item_width) && (it->flags == ELM_GENLIST_ITEM_GROUP))
                    {
                       it->wd->group_item_width = mw;
                       it->wd->group_item_height = mh;
                    }
                  else if ((!it->wd->item_width) && (it->flags == ELM_GENLIST_ITEM_NONE))
                    {
                       it->wd->item_width = mw;
                       it->wd->item_height = mh;
                    }
               }
             num++;
          }
        itb->changeme = EINA_FALSE;
        // TODO: why this is separated.
        if (height_changed)
          {
             position = 1;
             itb->changed = EINA_TRUE;
             _item_block_recalc(itb, num0, 0);
             _item_block_position(itb, num0);
          }
        else if (width_changed)
          {
             _item_block_position(itb, num0);
          }
     }
   if (position)
     {
        if (wd->calc_job) ecore_job_del(wd->calc_job);
        wd->calc_job = ecore_job_add(_calc_job, wd);
     }
}

static void
_pan_set(Evas_Object *obj,
         Evas_Coord   x,
         Evas_Coord   y)
{
   Pan *sd = evas_object_smart_data_get(obj);
#ifdef ANCHOR_ITEM
   Item_Block *itb;
#endif

   if (!sd) return;
   //   Evas_Coord ow, oh;
   //   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   //   ow = sd->wd->minw - ow;
   //   if (ow < 0) ow = 0;
   //   oh = sd->wd->minh - oh;
   //   if (oh < 0) oh = 0;
   //   if (x < 0) x = 0;
   //   if (y < 0) y = 0;
   //   if (x > ow) x = ow;
   //   if (y > oh) y = oh;
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;

#ifdef ANCHOR_ITEM
   EINA_INLIST_FOREACH(sd->wd->blocks, itb)
     {
        if ((itb->y + itb->h) > y)
          {
             Elm_Genlist_Item *it;
             Eina_List *l2;

             EINA_LIST_FOREACH(itb->items, l2, it)
               {
                  if ((itb->y + it->y) >= y)
                    {
                       sd->wd->anchor_item = it;
                       sd->wd->anchor_y = -(itb->y + it->y - y);
                       goto done;
                    }
               }
          }
     }
done:
#endif
   if (!sd->wd->reorder_move_animator) evas_object_smart_changed(obj);
}

static void
_pan_get(Evas_Object *obj,
         Evas_Coord  *x,
         Evas_Coord  *y)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_max_get(Evas_Object *obj,
             Evas_Coord  *x,
             Evas_Coord  *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   if (!sd) return;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = sd->wd->minw - ow;
   if (ow < 0) ow = 0;
   oh = sd->wd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

static void
_pan_min_get(Evas_Object *obj __UNUSED__,
             Evas_Coord  *x,
             Evas_Coord  *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

static void
_pan_child_size_get(Evas_Object *obj,
                    Evas_Coord  *w,
                    Evas_Coord  *h)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = ELM_NEW(Pan);
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}

static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   if (sd->resize_job)
     {
        ecore_job_del(sd->resize_job);
        sd->resize_job = NULL;
     }
   _pan_sc.del(obj);
}

static void
_pan_resize_job(void *data)
{
   Pan *sd = data;
   if (!sd) return;
   _sizing_eval(sd->wd->obj);
   sd->resize_job = NULL;
}

static void
_pan_resize(Evas_Object *obj,
            Evas_Coord   w,
            Evas_Coord   h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   if (!sd) return;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if ((sd->wd->height_for_width) && (ow != w))
     {
        sd->wd->pan_resize = EINA_TRUE;
        if (sd->resize_job) ecore_job_del(sd->resize_job);
        sd->resize_job = ecore_job_add(_pan_resize_job, sd);
     }
   sd->wd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(obj);
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = NULL;
}

static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Item_Block *itb;
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   int in = 0;
   Elm_Genlist_Item *git;
   Eina_List *l;

   if (!sd) return;
   evas_event_freeze(evas_object_evas_get(obj));

   if (sd->wd->pan_changed)
     {
        _calc_job(sd->wd);
        sd->wd->pan_changed = EINA_FALSE;
     }

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   sd->wd->prev_viewport_w = ow;
   evas_output_viewport_get(evas_object_evas_get(obj), &cvx, &cvy, &cvw, &cvh);
   EINA_INLIST_FOREACH(sd->wd->blocks, itb)
     {
        itb->w = sd->wd->minw;
        if (ELM_RECTS_INTERSECT(itb->x - sd->wd->pan_x + ox,
                                itb->y - sd->wd->pan_y + oy,
                                itb->w, itb->h,
                                cvx, cvy, cvw, cvh))
          {
             if ((!itb->realized) || (itb->changed))
               _item_block_realize(itb);
             _item_block_position(itb, in);
          }
        else
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
        in += itb->count;
     }

   if ((sd->wd->reorder_mode) && (sd->wd->reorder_it))
     {
        if (sd->wd->pan_y != sd->wd->old_pan_y)
          sd->wd->reorder_pan_move = EINA_TRUE;
        else sd->wd->reorder_pan_move = EINA_FALSE;
        evas_object_raise(sd->wd->reorder_it->edit_obj);
        sd->wd->old_pan_y = sd->wd->pan_y;
        sd->wd->start_time = ecore_loop_time_get();
     }

      if (sd->wd->effect_mode &&
          ((sd->wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_EXPAND) ||
           (sd->wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_CONTRACT) ||
           (sd->wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE)))
        {
           if (!sd->wd->item_moving_effect_timer)
             {
                if (sd->wd->move_effect_mode != ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE)
                   _item_flip_effect_show(sd->wd->expanded_item);

                evas_object_raise(sd->wd->alpha_bg);
                evas_object_show(sd->wd->alpha_bg);
                elm_smart_scroller_bounce_animator_disabled_set(sd->wd->scr, EINA_TRUE);
                sd->wd->start_time = current_time_get();
                sd->wd->item_moving_effect_timer = ecore_animator_add(_item_moving_effect_timer_cb, sd->wd);
             }
        }
      else _item_auto_scroll(sd->wd);
   sd->wd->pan_resize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_pan_move(Evas_Object *obj,
          Evas_Coord   x __UNUSED__,
          Evas_Coord   y __UNUSED__)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;

   sd->wd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(obj);
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = NULL;
}

static void
_hold_on(void        *data __UNUSED__,
         Evas_Object *obj,
         void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void        *data __UNUSED__,
          Evas_Object *obj,
          void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void        *data __UNUSED__,
           Evas_Object *obj,
           void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void        *data __UNUSED__,
            Evas_Object *obj,
            void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_scr_anim_start(void        *data,
                Evas_Object *obj __UNUSED__,
                void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scr_anim_stop(void        *data,
               Evas_Object *obj __UNUSED__,
               void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scr_drag_start(void            *data,
                Evas_Object     *obj __UNUSED__,
                void            *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scr_drag_stop(void            *data,
               Evas_Object     *obj __UNUSED__,
               void            *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_scroll_edge_left(void        *data,
                  Evas_Object *scr __UNUSED__,
                  void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_SCROLL_EDGE_LEFT, NULL);
}

static void
_scroll_edge_right(void        *data,
                   Evas_Object *scr __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_SCROLL_EDGE_RIGHT, NULL);
}

static void
_scroll_edge_top(void        *data,
                 Evas_Object *scr __UNUSED__,
                 void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_SCROLL_EDGE_TOP, NULL);
}

static void
_scroll_edge_bottom(void        *data,
                    Evas_Object *scr __UNUSED__,
                    void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_SCROLL_EDGE_BOTTOM, NULL);
}

static void
_mode_item_realize(Elm_Genlist_Item *it)
{
   char buf[1024];

   if ((it->mode_view) || (it->delete_me)) return;

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   it->mode_view = edje_object_add(evas_object_evas_get(WIDGET(it)));
   edje_object_scale_set(it->mode_view,
                         elm_widget_scale_get(WIDGET(it)) *
                         _elm_config->scale);
   evas_object_smart_member_add(it->mode_view, it->wd->pan_smart);
   elm_widget_sub_object_add(WIDGET(it), it->mode_view);

   strncpy(buf, "item", sizeof(buf));
   if (it->wd->compress)
     strncat(buf, "_compress", sizeof(buf) - strlen(buf));

   if (it->order_num_in & 0x1) strncat(buf, "_odd", sizeof(buf) - strlen(buf));
   strncat(buf, "/", sizeof(buf) - strlen(buf));
   strncat(buf, it->itc->mode_item_style, sizeof(buf) - strlen(buf));

   _elm_theme_object_set(WIDGET(it), it->mode_view, "genlist", buf,
                         elm_widget_style_get(WIDGET(it)));
   edje_object_mirrored_set(it->mode_view,
                            elm_widget_mirrored_get(WIDGET(it)));

   /* signal callback add */
   evas_object_event_callback_add(it->mode_view, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, it);
   evas_object_event_callback_add(it->mode_view, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, it);
   evas_object_event_callback_add(it->mode_view, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move, it);

   /* text_get, content_get, state_get */
   _item_text_realize(it, it->mode_view, &it->mode_texts, NULL);
   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;

        it->mode_contents =
           elm_widget_stringlist_get(edje_object_data_get(it->mode_view,
                                                          "contents"));
        EINA_LIST_FOREACH(it->mode_contents, l, key)
         {
           Evas_Object *ic = it->itc->func.content_get
                ((void *)it->base.data, WIDGET(it), l->data);

             if (ic)
               {
                  it->mode_content_objs = eina_list_append(it->mode_content_objs, ic);
                  edje_object_part_swallow(it->mode_view, key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(WIDGET(it), ic);
                  if (it->disabled)
                    elm_widget_disabled_set(ic, EINA_TRUE);
               }
          }
     }
   _item_state_realize(it, it->mode_view, &it->mode_states, NULL);
   edje_object_part_swallow(it->mode_view,
                            edje_object_data_get(it->mode_view, "mode_part"),
                            VIEW(it));

   it->want_unrealize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
}

static void
_mode_item_unrealize(Elm_Genlist_Item *it)
{
   Widget_Data *wd = it->wd;
   Evas_Object *content;
   if (!it->mode_view) return;

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   elm_widget_stringlist_free(it->mode_texts);
   it->mode_texts = NULL;
   elm_widget_stringlist_free(it->mode_contents);
   it->mode_contents = NULL;
   elm_widget_stringlist_free(it->mode_states);

   EINA_LIST_FREE(it->mode_content_objs, content)
     evas_object_del(content);

   edje_object_part_unswallow(it->mode_view, VIEW(it));
   evas_object_smart_member_add(VIEW(it), wd->pan_smart);
   evas_object_del(it->mode_view);
   it->mode_view = NULL;

   if (wd->mode_item == it)
     wd->mode_item = NULL;
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
}

static void
_item_mode_set(Elm_Genlist_Item *it)
{
   if (!it) return;
   Widget_Data *wd = it->wd;
   if (!wd) return;
   char buf[1024];

   wd->mode_item = it;
   it->nocache = EINA_TRUE;

   if (wd->scr_hold_timer)
     {
        ecore_timer_del(wd->scr_hold_timer);
        wd->scr_hold_timer = NULL;
     }
   elm_smart_scroller_hold_set(wd->scr, EINA_TRUE);
   wd->scr_hold_timer = ecore_timer_add(0.1, _scr_hold_timer_cb, wd);

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   _mode_item_realize(it);
   if (it->group_item)
     evas_object_raise(it->VIEW(group_item));
   _item_position(it, it->mode_view, it->scrl_x, it->scrl_y);
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));

   snprintf(buf, sizeof(buf), "elm,state,%s,active", wd->mode_type);
   edje_object_signal_emit(it->mode_view, buf, "elm");
}

static void
_item_mode_unset(Widget_Data *wd)
{
   if (!wd) return;
   if (!wd->mode_item) return;
   char buf[1024], buf2[1024];
   Elm_Genlist_Item *it;

   it = wd->mode_item;
   it->nocache = EINA_TRUE;

   snprintf(buf, sizeof(buf), "elm,state,%s,passive", wd->mode_type);
   snprintf(buf2, sizeof(buf2), "elm,state,%s,passive,finished", wd->mode_type);

   edje_object_signal_emit(it->mode_view, buf, "elm");
   edje_object_signal_callback_add(it->mode_view, buf2, "elm", _mode_finished_signal_cb, it);

   wd->mode_item = NULL;
}

static void
_item_auto_scroll(Widget_Data *wd)
{
   if (!wd) return;
   Elm_Genlist_Item  *tmp_item = NULL;

   if ((wd->expanded_item) && (wd->auto_scroll_enabled))
     {
        tmp_item = eina_list_data_get(eina_list_last(wd->expanded_item->items));
        if (!tmp_item) return;
        wd->show_item = tmp_item;
        wd->bring_in = EINA_TRUE;
        wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_IN;
        if ((wd->show_item->queued) || (!wd->show_item->mincalcd))
          {
             wd->show_item->showme = EINA_TRUE;
             wd->auto_scroll_enabled = EINA_FALSE;
          }
        else
          _scroll_item(wd);
     }
}

EAPI Evas_Object *
elm_genlist_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;
   static Evas_Smart *smart = NULL;

   if (!smart)
     {
        static Evas_Smart_Class sc;

        evas_object_smart_clipped_smart_set(&_pan_sc);
        sc = _pan_sc;
        sc.name = "elm_genlist_pan";
        sc.version = EVAS_SMART_CLASS_VERSION;
        sc.add = _pan_add;
        sc.del = _pan_del;
        sc.resize = _pan_resize;
        sc.move = _pan_move;
        sc.calculate = _pan_calculate;
        if (!(smart = evas_smart_class_new(&sc))) return NULL;
     }

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "genlist");
   elm_widget_type_set(obj, "genlist");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_on_show_region_hook_set(obj, _show_region_hook, obj);
   elm_widget_translate_hook_set(obj, _translate_hook);

   wd->scr = elm_smart_scroller_add(e);
   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_scroller, obj);
   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up_scroller, obj);
   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move_scroller, obj);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "genlist", "base",
                                       elm_widget_style_get(obj));
   elm_smart_scroller_bounce_allow_set(wd->scr, EINA_FALSE,
                                       _elm_config->thumbscroll_bounce_enable);
   elm_widget_resize_object_set(obj, wd->scr);

   evas_object_smart_callback_add(wd->scr, "animate,start", _scr_anim_start, obj);
   evas_object_smart_callback_add(wd->scr, "animate,stop", _scr_anim_stop, obj);
   evas_object_smart_callback_add(wd->scr, "drag,start", _scr_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scr_drag_stop, obj);
   evas_object_smart_callback_add(wd->scr, "edge,left", _scroll_edge_left, obj);
   evas_object_smart_callback_add(wd->scr, "edge,right", _scroll_edge_right,
                                  obj);
   evas_object_smart_callback_add(wd->scr, "edge,top", _scroll_edge_top, obj);
   evas_object_smart_callback_add(wd->scr, "edge,bottom", _scroll_edge_bottom,
                                  obj);

   wd->obj = obj;
   wd->mode = ELM_LIST_SCROLL;
   wd->max_items_per_block = MAX_ITEMS_PER_BLOCK;
   wd->item_cache_max = wd->max_items_per_block * 2;
   wd->longpress_timeout = _elm_config->longpress_timeout;
   wd->effect_mode = _elm_config->effect_enable;

   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   wd->pan_smart = evas_object_smart_add(e, smart);
   wd->pan = evas_object_smart_data_get(wd->pan_smart);
   wd->pan->wd = wd;

   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
                                     _pan_set, _pan_get, _pan_max_get,
                                     _pan_min_get, _pan_child_size_get);

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr),
                             &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

static Elm_Genlist_Item *
_item_new(Widget_Data                  *wd,
          const Elm_Genlist_Item_Class *itc,
          const void                   *data,
          Elm_Genlist_Item             *parent,
          Elm_Genlist_Item_Flags        flags,
          Evas_Smart_Cb                 func,
          const void                   *func_data)
{
   Elm_Genlist_Item *it, *it2;
   int depth = 0;

   it = elm_widget_item_new(wd->obj, Elm_Genlist_Item);
   if (!it) return NULL;
   it->wd = wd;
   it->itc = itc;
   it->base.data = data;
   it->parent = parent;
   it->flags = flags;
   it->func.func = func;
   it->func.data = func_data;
   it->mouse_cursor = NULL;
   it->expanded_depth = 0;
   it->can_focus = EINA_TRUE;
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_hook);
   elm_widget_item_del_cb_set(it, _item_del_hook);

   if (it->parent)
     {
        if (it->parent->flags & ELM_GENLIST_ITEM_GROUP)
          it->group_item = parent;
        else if (it->parent->group_item)
          it->group_item = it->parent->group_item;
     }
   for (it2 = it, depth = 0; it2->parent; it2 = it2->parent)
     {
        if (it2->parent->flags != ELM_GENLIST_ITEM_GROUP) depth += 1;
     }
   it->expanded_depth = depth;
   return it;
}

static void
_item_block_add(Widget_Data      *wd,
                Elm_Genlist_Item *it)
{
   Item_Block *itb = NULL;

   if (!it->rel)
     {
newblock:
        if (it->rel)
          {
             itb = calloc(1, sizeof(Item_Block));
             if (!itb) return;
             itb->wd = wd;
             if (!it->rel->block)
               {
                  wd->blocks =
                    eina_inlist_append(wd->blocks, EINA_INLIST_GET(itb));
                  itb->items = eina_list_append(itb->items, it);
               }
             else
               {
                  if (it->before)
                    {
                       wd->blocks = eina_inlist_prepend_relative
                           (wd->blocks, EINA_INLIST_GET(itb),
                           EINA_INLIST_GET(it->rel->block));
                       itb->items =
                         eina_list_prepend_relative(itb->items, it, it->rel);
                    }
                  else
                    {
                       wd->blocks = eina_inlist_append_relative
                           (wd->blocks, EINA_INLIST_GET(itb),
                           EINA_INLIST_GET(it->rel->block));
                       itb->items =
                         eina_list_append_relative(itb->items, it, it->rel);
                    }
               }
          }
        else
          {
             if (it->before)
               {
                  if (wd->blocks)
                    {
                       itb = (Item_Block *)(wd->blocks);
                       if (itb->count >= wd->max_items_per_block)
                         {
                            itb = calloc(1, sizeof(Item_Block));
                            if (!itb) return;
                            itb->wd = wd;
                            wd->blocks =
                              eina_inlist_prepend(wd->blocks,
                                                  EINA_INLIST_GET(itb));
                         }
                    }
                  else
                    {
                       itb = calloc(1, sizeof(Item_Block));
                       if (!itb) return;
                       itb->wd = wd;
                       wd->blocks =
                         eina_inlist_prepend(wd->blocks, EINA_INLIST_GET(itb));
                    }
                  itb->items = eina_list_prepend(itb->items, it);
               }
             else
               {
                  if (wd->blocks)
                    {
                       itb = (Item_Block *)(wd->blocks->last);
                       if (itb->count >= wd->max_items_per_block)
                         {
                            itb = calloc(1, sizeof(Item_Block));
                            if (!itb) return;
                            itb->wd = wd;
                            wd->blocks =
                              eina_inlist_append(wd->blocks,
                                                 EINA_INLIST_GET(itb));
                         }
                    }
                  else
                    {
                       itb = calloc(1, sizeof(Item_Block));
                       if (!itb) return;
                       itb->wd = wd;
                       wd->blocks =
                         eina_inlist_append(wd->blocks, EINA_INLIST_GET(itb));
                    }
                  itb->items = eina_list_append(itb->items, it);
               }
          }
     }
   else
     {
        itb = it->rel->block;
        if (!itb) goto newblock;
        if (it->before)
          itb->items = eina_list_prepend_relative(itb->items, it, it->rel);
        else
          itb->items = eina_list_append_relative(itb->items, it, it->rel);
     }
   itb->count++;
   itb->changed = EINA_TRUE;
   it->block = itb;
   if (itb->wd->calc_job) ecore_job_del(itb->wd->calc_job);
   itb->wd->calc_job = ecore_job_add(_calc_job, itb->wd);
   if (it->rel)
     {
        it->rel->relcount--;
        if ((it->rel->delete_me) && (!it->rel->relcount))
          _item_del(it->rel);
        it->rel = NULL;
     }
   if (itb->count > itb->wd->max_items_per_block)
     {
        int newc;
        Item_Block *itb2;
        Elm_Genlist_Item *it2;

        newc = itb->count / 2;
        itb2 = calloc(1, sizeof(Item_Block));
        if (!itb2) return;
        itb2->wd = wd;
        wd->blocks =
          eina_inlist_append_relative(wd->blocks, EINA_INLIST_GET(itb2),
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
             it2->block = itb2;
             itb2->count++;
          }
     }
}

static int
_queue_process(Widget_Data *wd)
{
   int n;
   Eina_Bool showme = EINA_FALSE;
   double t0, t;

   t0 = ecore_time_get();
   //evas_event_freeze(evas_object_evas_get(wd->obj));
   for (n = 0; (wd->queue) && (n < 128); n++)
     {
        Elm_Genlist_Item *it;

        it = wd->queue->data;
        wd->queue = eina_list_remove_list(wd->queue, wd->queue);
        it->queued = EINA_FALSE;
        _item_block_add(wd, it);
        t = ecore_time_get();
        if (it->block->changed)
          {
             showme = _item_block_recalc(it->block, it->block->num, EINA_TRUE);
             it->block->changed = 0;
             if(wd->pan_changed)
               {
                  if (wd->calc_job) ecore_job_del(wd->calc_job);
                  wd->calc_job = NULL;
                  _calc_job(wd);
                  wd->pan_changed = EINA_FALSE;
               }
          }
        if (showme) it->block->showme = EINA_TRUE;
        if (eina_inlist_count(wd->blocks) > 1)
          {
             if ((t - t0) > (ecore_animator_frametime_get())) break;
          }
     }
   //evas_event_thaw(evas_object_evas_get(wd->obj));
   //evas_event_thaw_eval(evas_object_evas_get(wd->obj));
   return n;
}

static Eina_Bool
_idle_process(void *data, Eina_Bool *wakeup)
{
   Widget_Data *wd = data;

   //xxx
   //static double q_start = 0.0;
   //if (q_start == 0.0) q_start = ecore_time_get();
   //xxx
   if (_queue_process(wd) > 0) *wakeup = EINA_TRUE;
   if (!wd->queue)
     {
        //xxx
        //printf("PROCESS TIME: %3.3f\n", ecore_time_get() - q_start);
        //xxx
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_item_idle_enterer(void *data)
{
   Widget_Data *wd = data;
   Eina_Bool wakeup = EINA_FALSE;
   Eina_Bool ok = _idle_process(data, &wakeup);

   if (wakeup)
     {
        // wake up mainloop
        if (wd->calc_job) ecore_job_del(wd->calc_job);
        wd->calc_job = ecore_job_add(_calc_job, wd);
     }
   if (ok == ECORE_CALLBACK_CANCEL) wd->queue_idle_enterer = NULL;
   return ok;
}

static void
_item_queue(Widget_Data      *wd,
            Elm_Genlist_Item *it)
{
   if (it->queued) return;
   it->queued = EINA_TRUE;
   wd->queue = eina_list_append(wd->queue, it);
// FIXME: why does a freeze then thaw here cause some genlist
// elm_genlist_item_append() to be much much slower?
//   evas_event_freeze(evas_object_evas_get(wd->obj));
   while ((wd->queue) && ((!wd->blocks) || (!wd->blocks->next)))
     {
        if (wd->queue_idle_enterer)
          {
             ecore_idle_enterer_del(wd->queue_idle_enterer);
             wd->queue_idle_enterer = NULL;
          }
        _queue_process(wd);
     }
//   evas_event_thaw(evas_object_evas_get(wd->obj));
//   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
   if (!wd->queue_idle_enterer)
     wd->queue_idle_enterer = ecore_idle_enterer_add(_item_idle_enterer, wd);
}

static void
_item_move_after(Elm_Genlist_Item *it, Elm_Genlist_Item *after)
{
   if (!it) return;
   if (!after) return;
   Elm_Object_Item *_it = (Elm_Object_Item *) it;
   Elm_Object_Item *_after = (Elm_Object_Item *) after;

   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   it->wd->reorder_deleted = EINA_TRUE;
   _item_block_del(it);

   it->wd->items = eina_inlist_append_relative(it->wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(after));
   it->rel = after;
   it->rel->relcount++;
   it->before = EINA_FALSE;
   if (after->group_item) it->group_item = after->group_item;
   _item_queue(it->wd, it);

   // TODO: change this to smart callback
   if (it->itc->func.moved)
     it->itc->func.moved(WIDGET(it), _it, _after, EINA_TRUE);
}

static void
_item_move_before(Elm_Genlist_Item *it, Elm_Genlist_Item *before)
{
   if (!it) return;
   if (!before) return;
   Elm_Object_Item *_it = (Elm_Object_Item *) it;
   Elm_Object_Item *_before = (Elm_Object_Item *) before;

   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   it->wd->reorder_deleted = EINA_TRUE;
   _item_block_del(it);
   it->wd->items = eina_inlist_prepend_relative(it->wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(before));
   it->rel = before;
   it->rel->relcount++;
   it->before = EINA_TRUE;
   if (before->group_item) it->group_item = before->group_item;
   _item_queue(it->wd, it);

   // TODO: change this to smart callback
   if (it->itc->func.moved)
     it->itc->func.moved(WIDGET(it), _it, _before, EINA_FALSE);
}

EAPI Elm_Object_Item *
elm_genlist_item_append(Evas_Object                  *obj,
                        const Elm_Genlist_Item_Class *itc,
                        const void                   *data,
                        Elm_Object_Item              *parent,
                        Elm_Genlist_Item_Flags        flags,
                        Evas_Smart_Cb                 func,
                        const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Elm_Genlist_Item *it = _item_new(wd, itc, data, (Elm_Genlist_Item *)parent,
                                    flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     {
        if (flags & ELM_GENLIST_ITEM_GROUP)
          wd->group_items = eina_list_append(wd->group_items, it);
        wd->items = eina_inlist_append(wd->items, EINA_INLIST_GET(it));
        it->rel = NULL;
     }
   else
     {
        Elm_Genlist_Item *it2 = NULL;
        Eina_List *ll = eina_list_last(it->parent->items);
        if (ll) it2 = ll->data;
        it->parent->items = eina_list_append(it->parent->items, it);
        if (!it2) it2 = it->parent;
        if (it2->delete_me)
           it2 = (Elm_Genlist_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it2);
        wd->items =
          eina_inlist_append_relative(wd->items, EINA_INLIST_GET(it),
                                      EINA_INLIST_GET(it2));
        it->rel = it2;
        it->rel->relcount++;
     }
   it->before = EINA_FALSE;
   _item_queue(wd, it);
   return (Elm_Object_Item *) it;
}

EAPI Elm_Object_Item *
elm_genlist_item_prepend(Evas_Object                  *obj,
                         const Elm_Genlist_Item_Class *itc,
                         const void                   *data,
                         Elm_Object_Item              *parent,
                         Elm_Genlist_Item_Flags        flags,
                         Evas_Smart_Cb                 func,
                         const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Elm_Genlist_Item *it = _item_new(wd, itc, data, (Elm_Genlist_Item *)parent,
                                    flags, func, func_data);

   if (!it) return NULL;
   if (!it->parent)
     {
        if (flags & ELM_GENLIST_ITEM_GROUP)
          wd->group_items = eina_list_prepend(wd->group_items, it);
        wd->items = eina_inlist_prepend(wd->items, EINA_INLIST_GET(it));
        it->rel = NULL;
     }
   else
     {
        Elm_Genlist_Item *it2 = NULL;
        Eina_List *ll = it->parent->items;
        if (ll) it2 = ll->data;
        it->parent->items = eina_list_prepend(it->parent->items, it);
        if (!it2) it2 = it->parent;
        if (it2->delete_me)
           it2 = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it2);
        wd->items =
          eina_inlist_prepend_relative(wd->items, EINA_INLIST_GET(it),
                                       EINA_INLIST_GET(it2));
        it->rel = it2;
        it->rel->relcount++;
     }
   it->before = EINA_TRUE;
   _item_queue(wd, it);
   return (Elm_Object_Item *) it;
}

EAPI Elm_Object_Item *
elm_genlist_item_insert_after(Evas_Object                  *obj,
                              const Elm_Genlist_Item_Class *itc,
                              const void                   *data,
                              Elm_Object_Item              *parent,
                              Elm_Object_Item              *after,
                              Elm_Genlist_Item_Flags        flags,
                              Evas_Smart_Cb                 func,
                              const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(after, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Genlist_Item *_after = (Elm_Genlist_Item *) after;
   if (!wd) return NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd->items, NULL);
   Elm_Genlist_Item *it = _item_new(wd, itc, data, (Elm_Genlist_Item *)parent,
                                    flags, func, func_data);
   if (!it) return NULL;

   if (!it->parent)
     {
        if ((flags & ELM_GENLIST_ITEM_GROUP) &&
            (((Elm_Genlist_Item * )after)->flags & ELM_GENLIST_ITEM_GROUP))
          wd->group_items = eina_list_append_relative(wd->group_items, it,
                                                      after);
     }
   else
     {
        it->parent->items = eina_list_append_relative(it->parent->items, it,
                                                      after);
     }
   wd->items = eina_inlist_append_relative(wd->items, EINA_INLIST_GET(it),
                                           EINA_INLIST_GET(_after));
   it->rel = _after;
   it->rel->relcount++;
   it->before = EINA_FALSE;
   _item_queue(wd, it);
   return (Elm_Object_Item *) it;
}

EAPI Elm_Object_Item *
elm_genlist_item_insert_before(Evas_Object                  *obj,
                               const Elm_Genlist_Item_Class *itc,
                               const void                   *data,
                               Elm_Object_Item              *parent,
                               Elm_Object_Item              *before,
                               Elm_Genlist_Item_Flags        flags,
                               Evas_Smart_Cb                 func,
                               const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(before, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Genlist_Item *_before = (Elm_Genlist_Item *) before;
   if (!wd) return NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(wd->items, NULL);
   Elm_Genlist_Item *it = _item_new(wd, itc, data, (Elm_Genlist_Item *)parent,
                                    flags, func, func_data);
   if (!it) return NULL;

   if (!it->parent)
     {
        if ((flags & ELM_GENLIST_ITEM_GROUP) &&
            (_before->flags & ELM_GENLIST_ITEM_GROUP))
          wd->group_items = eina_list_prepend_relative(wd->group_items, it,
                                                       _before);
     }
   else
     {
        it->parent->items = eina_list_prepend_relative(it->parent->items, it,
                                                       _before);
     }
   wd->items = eina_inlist_prepend_relative(wd->items, EINA_INLIST_GET(it),
                                            EINA_INLIST_GET(_before));
   it->rel = _before;
   it->rel->relcount++;
   it->before = EINA_TRUE;
   _item_queue(wd, it);
   return (Elm_Object_Item *) it;
}

EAPI void
elm_genlist_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->walking > 0)
     {
        Elm_Genlist_Item *it;

        wd->clear_me = EINA_TRUE;
        EINA_INLIST_FOREACH(wd->items, it)
          {
             it->delete_me = EINA_TRUE;
          }
        return;
     }
   evas_event_freeze(evas_object_evas_get(wd->obj));
   while (wd->items)
     {
        Elm_Genlist_Item *it = ELM_GENLIST_ITEM_FROM_INLIST(wd->items);
        it->nocache = EINA_TRUE;
        it->highlighted = EINA_FALSE;
#ifdef ANCHOR_ITEM
        if (wd->anchor_item == it)
          {
             wd->anchor_item = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
             if (!wd->anchor_item)
               wd->anchor_item =
                 ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
          }
#endif
        wd->items = eina_inlist_remove(wd->items, wd->items);
        if (it->flags & ELM_GENLIST_ITEM_GROUP)
          it->wd->group_items = eina_list_remove(it->wd->group_items, it);
        elm_widget_item_pre_notify_del(it);
        if (it->realized) _item_unrealize(it, EINA_FALSE);
        if (((wd->clear_me) || (!it->delete_me)) && (it->itc->func.del))
          it->itc->func.del((void *)it->base.data, WIDGET(it));
        if (it->long_timer) ecore_timer_del(it->long_timer);
        if (it->swipe_timer) ecore_timer_del(it->swipe_timer);
        elm_widget_item_del(it);
     }
   wd->clear_me = EINA_FALSE;
   wd->anchor_item = NULL;
   while (wd->blocks)
     {
        Item_Block *itb = (Item_Block *)(wd->blocks);

        wd->blocks = eina_inlist_remove(wd->blocks, wd->blocks);
        if (itb->items) eina_list_free(itb->items);
        free(itb);
     }
   wd->pan_changed = EINA_TRUE;
   if (wd->calc_job)
     {
        ecore_job_del(wd->calc_job);
        wd->calc_job = NULL;
     }
   if (wd->queue_idle_enterer)
     {
        ecore_idle_enterer_del(wd->queue_idle_enterer);
        wd->queue_idle_enterer = NULL;
     }
   if (wd->must_recalc_idler)
     {
        ecore_idler_del(wd->must_recalc_idler);
        wd->must_recalc_idler = NULL;
     }
   if (wd->queue)
     {
        eina_list_free(wd->queue);
        wd->queue = NULL;
     }
   if (wd->selected)
     {
        eina_list_free(wd->selected);
        wd->selected = NULL;
     }
   if (wd->reorder_move_animator)
     {
        ecore_animator_del(wd->reorder_move_animator);
        wd->reorder_move_animator = NULL;
     }
   wd->show_item = NULL;
   wd->pan_x = 0;
   wd->pan_y = 0;
   wd->old_pan_y = 0;
   wd->minw = 0;
   wd->minh = 0;

   if (wd->alpha_bg)
      evas_object_del(wd->alpha_bg);
   wd->alpha_bg = NULL;

   if (wd->pan_smart)
     {
        evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
        evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
     }
   _sizing_eval(obj);
   elm_smart_scroller_child_region_show(wd->scr, 0, 0, 0, 0);
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

EAPI void
elm_genlist_multi_select_set(Evas_Object *obj,
                             Eina_Bool    multi)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->multi = multi;
}

EAPI Eina_Bool
elm_genlist_multi_select_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->multi;
}

EAPI Elm_Object_Item *
elm_genlist_selected_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (wd->selected) return wd->selected->data;
   return NULL;
}

EAPI const Eina_List *
elm_genlist_selected_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->selected;
}

EAPI Eina_List *
elm_genlist_realized_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *list = NULL;
   Item_Block *itb;
   Eina_Bool done = EINA_FALSE;
   if (!wd) return NULL;
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        if (itb->realized)
          {
             Eina_List *l;
             Elm_Genlist_Item *it;

             done = 1;
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
                           Evas_Coord         x,
                           Evas_Coord         y,
                           int               *posret)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord ox, oy, ow, oh;
   Item_Block *itb;
   Evas_Coord lasty;
   if (!wd) return NULL;
   evas_object_geometry_get(wd->pan_smart, &ox, &oy, &ow, &oh);
   lasty = oy;
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Eina_List *l;
        Elm_Genlist_Item *it;

        if (!ELM_RECTS_INTERSECT(ox + itb->x - itb->wd->pan_x,
                                 oy + itb->y - itb->wd->pan_y,
                                 itb->w, itb->h, x, y, 1, 1))
          continue;
        EINA_LIST_FOREACH(itb->items, l, it)
          {
             Evas_Coord itx, ity;

             itx = ox + itb->x + it->x - itb->wd->pan_x;
             ity = oy + itb->y + it->y - itb->wd->pan_y;
             if (ELM_RECTS_INTERSECT(itx, ity, it->w, it->h, x, y, 1, 1))
               {
                  if (posret)
                    {
                       if (y <= (ity + (it->h / 4))) *posret = -1;
                       else if (y >= (ity + it->h - (it->h / 4)))
                         *posret = 1;
                       else *posret = 0;
                    }
                  return (Elm_Object_Item *) it;
               }
             lasty = ity + it->h;
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
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Genlist_Item *it = ELM_GENLIST_ITEM_FROM_INLIST(wd->items);
   while ((it) && (it->delete_me))
     it = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
   return (Elm_Object_Item *) it;
}

EAPI Elm_Object_Item *
elm_genlist_last_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Genlist_Item *it = ELM_GENLIST_ITEM_FROM_INLIST(wd->items->last);
   while ((it) && (it->delete_me))
     it = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
   return (Elm_Object_Item *) it;
}

EAPI Elm_Object_Item *
elm_genlist_item_next_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   while (_it)
     {
        _it = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(_it)->next);
        if ((_it) && (!_it->delete_me)) break;
     }
   return (Elm_Object_Item *) _it;
}

EAPI Elm_Object_Item *
elm_genlist_item_prev_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   while (_it)
     {
        _it = ELM_GENLIST_ITEM_FROM_INLIST(EINA_INLIST_GET(_it)->prev);
        if ((_it) && (!_it->delete_me)) break;
     }
   return (Elm_Object_Item *)_it;
}

EAPI Evas_Object *
elm_genlist_item_genlist_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return elm_object_item_widget_get(it);
}

EAPI Elm_Object_Item *
elm_genlist_item_parent_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return (Elm_Object_Item *) ((Elm_Genlist_Item *) it)->parent;
}

EAPI void
elm_genlist_item_subitems_clear(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Widget_Data *wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return;
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   Elm_Genlist_Item *it2, *it3;
   Evas_Coord y, h;

   if (!wd->effect_mode || !wd->move_effect_mode)
     _item_subitems_clear(_it);
   else
     {
        if ((!wd->item_moving_effect_timer) && (_it->flags != ELM_GENLIST_ITEM_GROUP) &&
             wd->move_effect_mode != ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE)
          {
             wd->expanded_item = _it;
             _item_flip_effect_show(_it);
             evas_object_geometry_get(VIEW(it), NULL, &y, NULL, &h);
             wd->expand_item_end = y + h;

             it2 = _it;
             do {
                it2 = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it2);
                if (!it2) break;
             } while (it2->expanded_depth > _it->expanded_depth);
             if (it2)
               {
                  int diff;
                  Evas_Coord vy, vh;
                  evas_output_viewport_get(evas_object_evas_get(wd->pan_smart), NULL, &vy, NULL, &vh);
                  diff = it2->old_scrl_y - (vy + vh);
                  if (diff > 0)
                    {
                       it3 = it2;
                       while (it3)
                         {
                            it3->old_scrl_y = it3->old_scrl_y - diff;
                            it3 = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it3);
                         }
                    }

                  wd->expand_item_gap = wd->expand_item_end - it2->old_scrl_y;
               }
             else
                wd->expand_item_gap = 0;

             evas_object_raise(wd->alpha_bg);
             evas_object_show(wd->alpha_bg);

             wd->start_time = current_time_get();
             wd->item_moving_effect_timer = ecore_animator_add(_item_moving_effect_timer_cb, wd);
          }
        else
           _item_subitems_clear(_it);
     }
}

EAPI void
elm_genlist_item_selected_set(Elm_Object_Item *it,
                              Eina_Bool         selected)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   Widget_Data *wd = elm_widget_data_get(WIDGET(_it));
   if (!wd) return;
   if ((_it->delete_me) || (_it->disabled)) return;
   selected = !!selected;
   if (_it->selected == selected) return;

   if (selected)
     {
        if (!wd->multi)
          {
             while (wd->selected)
               _item_unselect(wd->selected->data);
          }
        _item_highlight(_it);
        _item_select(_it);
     }
   else
     _item_unselect(_it);
}

EAPI Eina_Bool
elm_genlist_item_selected_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   return ((Elm_Genlist_Item *) it)->selected;
}

EAPI void
elm_genlist_item_expanded_set(Elm_Object_Item *it,
                              Eina_Bool         expanded)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if (_it->flags != ELM_GENLIST_ITEM_SUBITEMS) return;
   if (_it->expanded == expanded) return;
   _it->expanded = expanded;
   _it->wd->expanded_item = _it;

   if (_it->wd->effect_mode && !_it->wd->alpha_bg)
      _it->wd->alpha_bg = _create_tray_alpha_bg(WIDGET(it));

   if (_it->expanded)
     {
        _it->wd->move_effect_mode = ELM_GENLIST_ITEM_MOVE_EFFECT_EXPAND;
        _it->wd->expanded_move_effect_set = EINA_TRUE;
        _it->wd->expanded_next_item = (Elm_Genlist_Item *) elm_genlist_item_next_get(it);
        if (_it->realized)
          edje_object_signal_emit(VIEW(it), "elm,state,expanded", "elm");
        evas_object_smart_callback_call(WIDGET(it), SIG_EXPANDED, _it);
        _it->wd->auto_scroll_enabled = EINA_TRUE;
     }
   else
     {
        _it->wd->move_effect_mode = ELM_GENLIST_ITEM_MOVE_EFFECT_CONTRACT;
        _it->wd->expanded_next_item = NULL;
        if (_it->realized)
          edje_object_signal_emit(VIEW(it), "elm,state,contracted", "elm");
        evas_object_smart_callback_call(WIDGET(it), SIG_CONTRACTED, _it);
        _it->wd->auto_scroll_enabled = EINA_FALSE;
     }
}

EAPI Eina_Bool
elm_genlist_item_expanded_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   return ((Elm_Genlist_Item *) it)->expanded;
}

EAPI int
elm_genlist_item_expanded_depth_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, 0);
   return ((Elm_Genlist_Item *) it)->expanded_depth;
}

EAPI void
elm_genlist_item_disabled_set(Elm_Object_Item *it,
                              Eina_Bool         disabled)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Eina_List *l;
   Evas_Object *obj;
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if (_it->disabled == disabled) return;
   if (_it->delete_me) return;
   _it->disabled = !!disabled;
   if (_it->selected)
     elm_genlist_item_selected_set(it, EINA_FALSE);
   if (_it->realized)
     {
        if (_it->disabled)
          {
             edje_object_signal_emit(VIEW(_it), "elm,state,disabled", "elm");
             if (_it->edit_obj)
               edje_object_signal_emit(_it->edit_obj, "elm,state,disabled", "elm");
          }
        else
          {
             edje_object_signal_emit(VIEW(_it), "elm,state,enabled", "elm");
             if (_it->edit_obj)
               edje_object_signal_emit(_it->edit_obj, "elm,state,enabled", "elm");
          }
        EINA_LIST_FOREACH(_it->content_objs, l, obj)
           elm_widget_disabled_set(obj, disabled);
     }
}

EAPI Eina_Bool
elm_genlist_item_disabled_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   if (((Elm_Genlist_Item *) it)->delete_me) return EINA_FALSE;
   return ((Elm_Genlist_Item *) it)->disabled;
}

EAPI void
elm_genlist_item_display_only_set(Elm_Object_Item *it,
                                  Eina_Bool         display_only)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if (_it->display_only == display_only) return;
   if (_it->delete_me) return;
   _it->display_only = display_only;
   _it->mincalcd = EINA_FALSE;
   _it->updateme = EINA_TRUE;
   if (_it->block) _it->block->updateme = EINA_TRUE;
   if (_it->wd->update_job) ecore_job_del(_it->wd->update_job);
   _it->wd->update_job = ecore_job_add(_update_job, _it->wd);
}

EAPI Eina_Bool
elm_genlist_item_display_only_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   if (((Elm_Genlist_Item *) it)->delete_me) return EINA_FALSE;
   return ((Elm_Genlist_Item *) it)->display_only;
}

EAPI void
elm_genlist_item_show(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if (_it->delete_me) return;
   if ((_it->queued) || (!_it->mincalcd))
     {
        _it->wd->show_item = _it;
        _it->wd->bring_in = EINA_FALSE;
        _it->wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_IN;
        _it->showme = EINA_TRUE;
        return;
     }
   if (_it->wd->show_item)
     {
        _it->wd->show_item->showme = EINA_FALSE;
        _it->wd->show_item = NULL;
     }
   elm_smart_scroller_child_region_show(_it->wd->scr,
                                        _it->x + _it->block->x,
                                        _it->y + _it->block->y,
                                        _it->block->w, _it->h);
}

EAPI void
elm_genlist_item_bring_in(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if (_it->delete_me) return;
   if ((_it->queued) || (!_it->mincalcd))
     {
        _it->wd->show_item = _it;
        _it->wd->bring_in = EINA_TRUE;
        _it->wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_IN;
        _it->showme = EINA_TRUE;
        return;
     }
   if (_it->wd->show_item)
     {
        _it->wd->show_item->showme = EINA_FALSE;
        _it->wd->show_item = NULL;
     }
   elm_smart_scroller_region_bring_in(_it->wd->scr,
                                      _it->x + _it->block->x,
                                      _it->y + _it->block->y,
                                      _it->block->w, _it->h);
}

EAPI void
elm_genlist_item_top_show(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Evas_Coord ow, oh;
   Evas_Coord pan_w = 0, pan_h = 0;
   Eina_Bool pan_size_ready = EINA_FALSE;
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if (_it->delete_me) return;
   evas_object_geometry_get(_it->wd->pan_smart, NULL, NULL, &ow, &oh);
   if (_it->block)
     {
        _pan_child_size_get(_it->wd->pan_smart, &pan_w, &pan_h);
        if ((pan_w > (_it->x + _it->block->x)) && (pan_h > (_it->y + _it->block->y + oh)))
          {
             pan_size_ready = EINA_TRUE;
          }
     }
   if ((_it->queued) || (!_it->mincalcd) || (!pan_size_ready))
     {
        _it->wd->show_item = _it;
        _it->wd->bring_in = EINA_FALSE;
        _it->wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_TOP;
        _it->showme = EINA_TRUE;
        return;
     }
   if (_it->wd->show_item)
     {
        _it->wd->show_item->showme = EINA_FALSE;
        _it->wd->show_item = NULL;
     }
   elm_smart_scroller_child_region_show(_it->wd->scr,
                                        _it->x + _it->block->x,
                                        _it->y + _it->block->y,
                                        _it->block->w, oh);
}

EAPI void
elm_genlist_item_top_bring_in(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Evas_Coord ow, oh;
   Evas_Coord pan_w = 0, pan_h = 0;
   Eina_Bool pan_size_ready = EINA_FALSE;
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if (_it->delete_me) return;
   evas_object_geometry_get(_it->wd->pan_smart, NULL, NULL, &ow, &oh);
   if (_it->block)
     {
        _pan_child_size_get(_it->wd->pan_smart, &pan_w, &pan_h);
        if ((pan_w > (_it->x + _it->block->x)) && (pan_h > (_it->y + _it->block->y + oh)))
          {
             pan_size_ready = EINA_TRUE;
          }
     }
   if ((_it->queued) || (!_it->mincalcd) || (!pan_size_ready))
     {
        _it->wd->show_item = _it;
        _it->wd->bring_in = EINA_TRUE;
        _it->wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_TOP;
        _it->showme = EINA_TRUE;
        return;
     }
   if (_it->wd->show_item)
     {
        _it->wd->show_item->showme = EINA_FALSE;
        _it->wd->show_item = NULL;
     }
   elm_smart_scroller_region_bring_in(_it->wd->scr,
                                      _it->x + _it->block->x,
                                      _it->y + _it->block->y,
                                      _it->block->w, oh);
}

EAPI void
elm_genlist_item_middle_show(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Evas_Coord ow, oh;
   Evas_Coord pan_w = 0, pan_h = 0;
   Eina_Bool pan_size_ready = EINA_FALSE;
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if (_it->delete_me) return;
   evas_object_geometry_get(_it->wd->pan_smart, NULL, NULL, &ow, &oh);
   if (_it->block)
     {
        _pan_child_size_get(_it->wd->pan_smart, &pan_w, &pan_h);
        if ((pan_w > (_it->x + _it->block->x)) && (pan_h > (_it->y + _it->block->y + oh / 2)))
          {
             pan_size_ready = EINA_TRUE;
          }
     }
   if ((_it->queued) || (!_it->mincalcd) || (!pan_size_ready))
     {
        _it->wd->show_item = _it;
        _it->wd->bring_in = EINA_FALSE;
        _it->wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_MIDDLE;
        _it->showme = EINA_TRUE;
        return;
     }
   if (_it->wd->show_item)
     {
        _it->wd->show_item->showme = EINA_FALSE;
        _it->wd->show_item = NULL;
     }
   elm_smart_scroller_child_region_show(_it->wd->scr,
                                        _it->x + _it->block->x,
                                        _it->y + _it->block->y - oh / 2 +
                                        _it->h / 2, _it->block->w, oh);
}

EAPI void
elm_genlist_item_middle_bring_in(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Evas_Coord ow, oh;
   Evas_Coord pan_w = 0, pan_h = 0;
   Eina_Bool pan_size_ready = EINA_FALSE;
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if (_it->delete_me) return;
   evas_object_geometry_get(_it->wd->pan_smart, NULL, NULL, &ow, &oh);
   if (_it->block)
     {
        _pan_child_size_get(_it->wd->pan_smart, &pan_w, &pan_h);
        if ((pan_w > (_it->x + _it->block->x)) && (pan_h > (_it->y + _it->block->y + oh / 2)))
          {
             pan_size_ready = EINA_TRUE;
          }
     }
   if ((_it->queued) || (!_it->mincalcd) || (!pan_size_ready))
     {
        _it->wd->show_item = _it;
        _it->wd->bring_in = EINA_TRUE;
        _it->wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_MIDDLE;
        _it->showme = EINA_TRUE;
        return;
     }
   if (_it->wd->show_item)
     {
        _it->wd->show_item->showme = EINA_FALSE;
        _it->wd->show_item = NULL;
     }
   elm_smart_scroller_region_bring_in(_it->wd->scr,
                                      _it->x + _it->block->x,
                                      _it->y + _it->block->y - oh / 2 + _it->h / 2,
                                      _it->block->w, oh);
}

EAPI void
elm_genlist_item_del(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if ((_it->relcount > 0) || (_it->walking > 0))
     {
        elm_widget_item_pre_notify_del(_it);
        elm_genlist_item_subitems_clear(it);
        _it->delete_me = EINA_TRUE;
        if (_it->wd->show_item == _it) _it->wd->show_item = NULL;
        if (_it->selected)
          _it->wd->selected = eina_list_remove(_it->wd->selected,
                                               _it);
        if (_it->block)
          {
             if (_it->realized) _item_unrealize(_it, EINA_FALSE);
             _it->block->changed = EINA_TRUE;
             if (_it->wd->calc_job) ecore_job_del(_it->wd->calc_job);
             _it->wd->calc_job = ecore_job_add(_calc_job, _it->wd);
          }
        if (_it->itc->func.del)
          _it->itc->func.del((void *)_it->base.data, WIDGET(_it));
        return;
     }
   _item_del(_it);
}

EAPI void
elm_genlist_item_data_set(Elm_Object_Item *it,
                          const void       *data)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   elm_widget_item_data_set(it, data);
}

EAPI void *
elm_genlist_item_data_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return elm_widget_item_data_get(it);
}

EAPI void
elm_genlist_item_contents_orphan(Elm_Object_Item *it)
{
   Evas_Object *content;
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   EINA_LIST_FREE(((Elm_Genlist_Item *) it)->content_objs, content)
     {
        elm_widget_sub_object_del(WIDGET(it), content);
        evas_object_smart_member_del(content);
        evas_object_hide(content);
     }
}

EAPI const Evas_Object *
elm_genlist_item_object_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return VIEW(it);
}

EAPI void
elm_genlist_item_update(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if (!_it->block) return;
   if (_it->delete_me) return;
   _it->mincalcd = EINA_FALSE;
   _it->updateme = EINA_TRUE;
   _it->block->updateme = EINA_TRUE;
   if (_it->wd->update_job) ecore_job_del(_it->wd->update_job);
   _it->wd->update_job = ecore_job_add(_update_job, _it->wd);
   _it->defer_unrealize = EINA_FALSE;
}

EAPI void
elm_genlist_item_fields_update(Elm_Object_Item *it,
                               const char *parts,
                               Elm_Genlist_Item_Field_Flags itf)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if (!_it->block) return;
   if (_it->delete_me) return;

   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_TEXT))
     _item_text_realize(_it, VIEW(_it), &_it->texts, parts);
   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_CONTENT))
     {
        _it->content_objs = _item_content_unrealize(_it, VIEW(_it),
                                                    &_it->contents, parts);
        _it->content_objs = _item_content_realize(_it, VIEW(_it),
                                                  &_it->contents, parts);

        if (_it->renamed)
          {
             _it->content_objs = _item_mode_content_unrealize(_it, VIEW(_it),
                                                              &_it->contents, parts, &_it->content_objs);
             _it->content_objs = _item_mode_content_realize(_it, VIEW(_it),
                                                            &_it->contents, parts, &_it->content_objs, "renames");
          }

        if (_it->mode_view)
          {
             _it->mode_content_objs = _item_mode_content_unrealize(_it, _it->mode_view,
                                                                   &_it->mode_contents, parts, &_it->mode_content_objs);
             _it->mode_content_objs = _item_mode_content_realize(_it, _it->mode_view,
                                                                 &_it->mode_contents, parts, &_it->mode_content_objs, "contents");
          }

        if (_it->wd->edit_mode)
          {
             _it->edit_content_objs = _item_mode_content_unrealize(_it, _it->edit_obj,
                                                                   &_it->contents, parts, &_it->edit_content_objs);
             _it->edit_content_objs = _item_mode_content_realize(_it, _it->edit_obj,
                                                                 &_it->contents, parts, &_it->edit_content_objs, "edit_contents");
          }
     }
   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_STATE))
     _item_state_realize(_it, VIEW(_it), &_it->states, parts);
}

EAPI void
elm_genlist_item_item_class_update(Elm_Object_Item             *it,
                                   const Elm_Genlist_Item_Class *itc)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if (!_it->block) return;
   EINA_SAFETY_ON_NULL_RETURN(itc);
   if (_it->delete_me) return;
   _it->itc = itc;
   _it->nocache = EINA_TRUE;
   elm_genlist_item_update(it);
}

EAPI const Elm_Genlist_Item_Class *
elm_genlist_item_item_class_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   if (((Elm_Genlist_Item *) it)->delete_me) return NULL;
   return ((Elm_Genlist_Item *) it)->itc;
}
static Evas_Object *
_elm_genlist_item_label_create(void        *data,
                               Evas_Object *obj __UNUSED__,
                               Evas_Object *tooltip,
                               void        *item __UNUSED__)
{
   Evas_Object *label = elm_label_add(tooltip);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);
   return label;
}

static void
_elm_genlist_item_label_del_cb(void        *data,
                               Evas_Object *obj __UNUSED__,
                               void        *event_info __UNUSED__)
{
   eina_stringshare_del(data);
}

EAPI void
elm_genlist_item_tooltip_text_set(Elm_Object_Item *it,
                                  const char      *text)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   text = eina_stringshare_add(text);
   elm_genlist_item_tooltip_content_cb_set(it, _elm_genlist_item_label_create,
                                           text,
                                           _elm_genlist_item_label_del_cb);
}

EAPI void
elm_genlist_item_tooltip_content_cb_set(Elm_Object_Item            *it,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void                 *data,
                                        Evas_Smart_Cb               del_cb)
{
   ELM_OBJ_ITEM_CHECK_OR_GOTO(it, error);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if ((_it->tooltip.content_cb == func) && (_it->tooltip.data == data))
     return;

   if (_it->tooltip.del_cb)
     _it->tooltip.del_cb((void *)_it->tooltip.data,
                          _it->base.widget, it);

   _it->tooltip.content_cb = func;
   _it->tooltip.data = data;
   _it->tooltip.del_cb = del_cb;

   if (VIEW(_it))
     {
        elm_widget_item_tooltip_content_cb_set(_it,
                                               _it->tooltip.content_cb,
                                               _it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, _it->tooltip.style);
     }

   return;

error:
   if (del_cb) del_cb((void *)data, NULL, NULL);
}

EAPI void
elm_genlist_item_tooltip_unset(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   if ((_it->base.view) && (_it->tooltip.content_cb))
     elm_widget_item_tooltip_unset(_it);

   if (_it->tooltip.del_cb)
     _it->tooltip.del_cb((void *)_it->tooltip.data, _it->base.widget, _it);
   _it->tooltip.del_cb = NULL;
   _it->tooltip.content_cb = NULL;
   _it->tooltip.data = NULL;
   if (_it->tooltip.style)
     elm_genlist_item_tooltip_style_set(it, NULL);
}

EAPI void
elm_genlist_item_tooltip_style_set(Elm_Object_Item *it,
                                   const char       *style)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   eina_stringshare_replace(&_it->tooltip.style, style);
   if (_it->base.view) elm_widget_item_tooltip_style_set(it, style);
}

EAPI const char *
elm_genlist_item_tooltip_style_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return ((Elm_Genlist_Item *) it)->tooltip.style;
}

EAPI Eina_Bool
elm_genlist_item_tooltip_size_restrict_disable(Elm_Object_Item *it, Eina_Bool disable)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   //   return elm_widget_item_tooltip_size_restrict_disable(it, disable);
   return EINA_FALSE;
}

EAPI Eina_Bool
elm_genlist_item_tooltip_size_restrict_disabled_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   //return elm_widget_item_tooltip_size_restrict_disabled_get(it);
   return EINA_FALSE;
}

EAPI void
elm_genlist_item_cursor_set(Elm_Object_Item *it,
                            const char       *cursor)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   eina_stringshare_replace(&_it->mouse_cursor, cursor);
   if (_it->base.view) elm_widget_item_cursor_set(it, cursor);
}

EAPI const char *
elm_genlist_item_cursor_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return elm_widget_item_cursor_get(it);
}

EAPI void
elm_genlist_item_cursor_unset(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if (!_it->mouse_cursor)
     return;

   if (VIEW(_it))
     elm_widget_item_cursor_unset(_it);

   eina_stringshare_del(_it->mouse_cursor);
   _it->mouse_cursor = NULL;
}

EAPI void
elm_genlist_item_cursor_style_set(Elm_Object_Item *it,
                                  const char       *style)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   elm_widget_item_cursor_style_set(it, style);
}

EAPI const char *
elm_genlist_item_cursor_style_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return elm_widget_item_cursor_style_get(it);
}

EAPI void
elm_genlist_item_cursor_engine_only_set(Elm_Object_Item *it,
                                        Eina_Bool         engine_only)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   elm_widget_item_cursor_engine_only_set(it, engine_only);
}

EAPI Eina_Bool
elm_genlist_item_cursor_engine_only_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   return elm_widget_item_cursor_engine_only_get(it);
}

EAPI void
elm_genlist_horizontal_mode_set(Evas_Object  *obj,
                                Elm_List_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->mode == mode) return;
   wd->mode = mode;
   _sizing_eval(obj);
}

EAPI Elm_List_Mode
elm_genlist_horizontal_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_LIST_LAST;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_LIST_LAST;
   return wd->mode;
}

EAPI void
elm_genlist_always_select_mode_set(Evas_Object *obj,
                                   Eina_Bool    always_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->always_select = always_select;
}

EAPI Eina_Bool
elm_genlist_always_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->always_select;
}

EAPI void
elm_genlist_no_select_mode_set(Evas_Object *obj,
                               Eina_Bool    no_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->no_select = no_select;
}

EAPI Eina_Bool
elm_genlist_no_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->no_select;
}

EAPI void
elm_genlist_compress_mode_set(Evas_Object *obj,
                              Eina_Bool    compress)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->compress = compress;
}

EAPI Eina_Bool
elm_genlist_compress_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->compress;
}

EAPI void
elm_genlist_height_for_width_mode_set(Evas_Object *obj,
                                      Eina_Bool    height_for_width)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->height_for_width = !!height_for_width;
   if (wd->height_for_width)
     {
        elm_genlist_homogeneous_set(obj, EINA_FALSE);
        elm_genlist_compress_mode_set(obj, EINA_TRUE);
     }
}

EAPI Eina_Bool
elm_genlist_height_for_width_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->height_for_width;
}

EAPI void
elm_genlist_bounce_set(Evas_Object *obj,
                       Eina_Bool    h_bounce,
                       Eina_Bool    v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_set(wd->scr, h_bounce, v_bounce);
}

EAPI void
elm_genlist_bounce_get(const Evas_Object *obj,
                       Eina_Bool         *h_bounce,
                       Eina_Bool         *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_get(wd->scr, h_bounce, v_bounce);
}

EAPI void
elm_genlist_homogeneous_set(Evas_Object *obj,
                            Eina_Bool    homogeneous)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (homogeneous) elm_genlist_compress_mode_set(obj, EINA_TRUE);
   wd->homogeneous = homogeneous;
}

EAPI Eina_Bool
elm_genlist_homogeneous_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->homogeneous;
}

EAPI void
elm_genlist_block_count_set(Evas_Object *obj,
                            int          n)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->max_items_per_block = n;
   wd->item_cache_max = wd->max_items_per_block * 2;
   _item_cache_clean(wd);
}

EAPI int
elm_genlist_block_count_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->max_items_per_block;
}

EAPI void
elm_genlist_longpress_timeout_set(Evas_Object *obj,
                                  double       timeout)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->longpress_timeout = timeout;
}

EAPI double
elm_genlist_longpress_timeout_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->longpress_timeout;
}

EAPI void
elm_genlist_scroller_policy_set(Evas_Object        *obj,
                                Elm_Scroller_Policy policy_h,
                                Elm_Scroller_Policy policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->scr)) return;
   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;
   elm_smart_scroller_policy_set(wd->scr, policy_h, policy_v);
}

EAPI void
elm_genlist_scroller_policy_get(const Evas_Object   *obj,
                                Elm_Scroller_Policy *policy_h,
                                Elm_Scroller_Policy *policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Smart_Scroller_Policy s_policy_h, s_policy_v;
   if ((!wd) || (!wd->scr)) return;
   elm_smart_scroller_policy_get(wd->scr, &s_policy_h, &s_policy_v);
   if (policy_h) *policy_h = (Elm_Scroller_Policy)s_policy_h;
   if (policy_v) *policy_v = (Elm_Scroller_Policy)s_policy_v;
}

EAPI void
elm_genlist_realized_items_update(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Eina_List *list, *l;
   Elm_Object_Item *it;

   list = elm_genlist_realized_items_get(obj);
   EINA_LIST_FOREACH(list, l, it)
     elm_genlist_item_update(it);
}

EAPI void
elm_genlist_item_mode_set(Elm_Object_Item *it,
                          const char       *mode_type,
                          Eina_Bool         mode_set)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;
   Widget_Data *wd = _it->wd;
   Eina_List *l;
   Elm_Object_Item *it2;

   if (!wd) return;
   if (!mode_type) return;
   if ((_it->delete_me) || (_it->disabled)) return;
   if (wd->edit_mode) return;

   if ((wd->mode_item == _it) &&
       (!strcmp(mode_type, wd->mode_type)) &&
       (mode_set))
      return;
   if (!_it->itc->mode_item_style) return;

   if (wd->multi)
     {
        EINA_LIST_FOREACH(wd->selected, l, it2)
          if (((Elm_Genlist_Item *) it2)->realized)
            elm_genlist_item_selected_set(it2, EINA_FALSE);
     }
   else
     {
        it2 = elm_genlist_selected_item_get(wd->obj);
        if ((it2) && (((Elm_Genlist_Item *) it2)->realized))
          elm_genlist_item_selected_set(it2, EINA_FALSE);
     }

   if (((wd->mode_type) && (strcmp(mode_type, wd->mode_type))) ||
       (mode_set) ||
       ((_it == wd->mode_item) && (!mode_set)))
     _item_mode_unset(wd);

   eina_stringshare_replace(&wd->mode_type, mode_type);
   if (mode_set) _item_mode_set(_it);
}

EAPI const char *
elm_genlist_mode_item_style_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->mode_item->itc->mode_item_style;
}

EAPI void
elm_genlist_mode_item_style_set(Evas_Object *obj, const char *style)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((style == wd->mode_item->itc->mode_item_style) ||
       (style && wd->mode_item->itc->mode_item_style &&
       (!strcmp(style, wd->mode_item->itc->mode_item_style))))
     return;
   eina_stringshare_replace(&wd->mode_item->itc->mode_item_style, style);
   elm_genlist_realized_items_update(obj);
}

EAPI const char *
elm_genlist_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->mode_type;
}

EAPI const Elm_Object_Item *
elm_genlist_mode_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return (Elm_Object_Item *) wd->mode_item;
}

EAPI void
elm_genlist_reorder_mode_set(Evas_Object *obj,
                             Eina_Bool    reorder_mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->reorder_mode = reorder_mode;
}

EAPI Eina_Bool
elm_genlist_reorder_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->reorder_mode;
}

EAPI void
elm_genlist_item_move_after(Elm_Object_Item *it __UNUSED__, Elm_Object_Item *after __UNUSED__)
{
   return;
}

EAPI void
elm_genlist_item_move_before(Elm_Object_Item *it __UNUSED__, Elm_Object_Item *before __UNUSED__)
{
   return;
}

EAPI void
elm_genlist_effect_set(const Evas_Object *obj, Eina_Bool emode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->effect_mode = emode;
   //   wd->point_rect = evas_object_rectangle_add(evas_object_evas_get(wd->obj));
   //   evas_object_resize(wd->point_rect, 10, 25);
   //   evas_object_color_set(wd->point_rect, 255, 0, 0, 130);
   //   evas_object_show(wd->point_rect);
   //   evas_object_hide(wd->point_rect);
}

static Evas_Object*
_create_tray_alpha_bg(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   Evas_Object *bg = NULL;
   Evas_Coord ox, oy, ow, oh;

   evas_object_geometry_get(wd->pan_smart, &ox, &oy, &ow, &oh);
   bg  =  evas_object_rectangle_add(evas_object_evas_get(wd->obj));
   evas_object_color_set(bg,0,0,0,0);
   evas_object_resize(bg , ow, oh);
   evas_object_move(bg , ox, oy);
   evas_object_show(bg);
   evas_object_hide(bg);
   return bg ;
}

static unsigned int
current_time_get()
{
   struct timeval timev;

   gettimeofday(&timev, NULL);
   return ((timev.tv_sec * 1000) + ((timev.tv_usec) / 1000));
}

// added for item moving animation.
static Eina_Bool
_item_moving_effect_timer_cb(void *data)
{
   Widget_Data *wd = data;
   if (!wd) return EINA_FALSE;
   Item_Block *itb;
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   Elm_Genlist_Item *it = NULL, *it2;
   const Eina_List *l;
   double time = 0.5, t;
   int y, dy;
   Eina_Bool check, end = EINA_FALSE, vis = EINA_TRUE;
   int in = 0;

   t = ((0.0 > (t = current_time_get() - wd->start_time)) ? 0.0 : t) / 1000;

   evas_object_geometry_get(wd->pan_smart, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(wd->pan_smart), &cvx, &cvy, &cvw, &cvh);
   if (t > time) end = EINA_TRUE;

   // Below while statement is needed, when the genlist is resized.
   it2 = wd->expanded_item;
   while (it2 && vis)
     {
        evas_object_move(VIEW(it2), it2->scrl_x, it2->scrl_y);
        vis = (ELM_RECTS_INTERSECT(it2->scrl_x, it2->scrl_y, it2->w, it2->h,
                                   cvx, cvy, cvw, cvh));
        it2 = (Elm_Genlist_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it2);
     }
   if (wd->expanded_next_item)
     {
        Item_Block *itb2;
        it2 = wd->expanded_next_item;
        itb2 = wd->expanded_next_item->block;
        if ((wd->expanded_move_effect_set) && (!(ELM_RECTS_INTERSECT(itb2->x - wd->pan_x + ox,
                                                                     itb2->y - wd->pan_y + oy,
                                                                     itb2->w, itb2->h,
                                                                     cvx, cvy, cvw, cvh))))
          {
             _item_block_position(itb2, 0);
             wd->expanded_move_effect_set = EINA_FALSE;
          }
     }
   else
     {
        it2 = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) wd->expanded_item);
     }
   while (it2)
     {
        if (wd->expanded_item->expanded_depth >= it2->expanded_depth) break;
        it2 = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it2);
     }
   dy = 0;
   if (it2)
     {
        if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_EXPAND)
          dy = ((it2->scrl_y > cvy + cvh) ? cvy + cvh : it2->scrl_y) - it2->old_scrl_y;
        else if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_CONTRACT)
          dy = wd->expand_item_gap;

        if (t <= time)
          {
             y = ((1 - (1 - (t / time)) * (1 - (t /time))) * dy);
          }
        else
          {
             end = EINA_TRUE;
             y = dy;
          }

        check = EINA_FALSE;
        EINA_INLIST_FOREACH(wd->blocks, itb)
          {
             itb->w = wd->minw;
             if (ELM_RECTS_INTERSECT(itb->x - wd->pan_x + ox,
                                     itb->y - wd->pan_y + oy,
                                     itb->w, itb->h,
                                     cvx, cvy, cvw, cvh))
               {
                  EINA_LIST_FOREACH(itb->items, l, it)
                    {
                       if (it == it2) check = EINA_TRUE;
                       if (!check) continue;

                       if (!it->old_scrl_y)
                         it->old_scrl_y  = it->scrl_y;
                       if (it->old_scrl_y + y < oy + oh)
                         {
                            if (!it->realized) _item_realize(it, in, 0);
                         }
                       if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE && it->old_scrl_y + y < it->scrl_y)
                         it->old_scrl_y = it->scrl_y - y;
                       in++;

                       if (wd->move_effect_mode != ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE ||
                           (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_DELETE && it->old_scrl_y + y >= it->scrl_y))
                         {
                            if (wd->edit_mode) _effect_item_controls(it, it->scrl_x, it->old_scrl_y + y);
                            else
                              {
                                 evas_object_resize(VIEW(it), it->w, it->h);
                                 evas_object_move(VIEW(it), it->scrl_x, it->old_scrl_y + y);
                                 evas_object_show(VIEW(it));
                                 evas_object_raise(VIEW(it));
                              }

                            if (it->group_item) evas_object_raise(it->group_item->base.view);
                         }
                    }
               }
          }

        if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_EXPAND)
          {
             it = (Elm_Genlist_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it2);
             while (it)
               {
                  if (it->expanded_depth <= it2->expanded_depth) break;
                  if (it->scrl_y && (it->scrl_y < it2->old_scrl_y + y) && (it->expanded_depth > it2->expanded_depth))
                    {
                       if (!it->effect_done)
                         {
                            edje_object_signal_emit(VIEW(it), "flip_item", "");
                            evas_object_show(VIEW(it));
                            it->effect_done = EINA_TRUE;
                         }
                    }
                  it = (Elm_Genlist_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it);
               }
          }
        else if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_CONTRACT)
          {
             it = (Elm_Genlist_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it2);
             while (it)
               {
                  if ((it->scrl_y > it2->old_scrl_y + y) && (it->expanded_depth > it2->expanded_depth))
                    {
                       if (!it->effect_done)
                         {
                            edje_object_signal_emit(VIEW(it), "elm,state,hide", "");
                            it->effect_done = EINA_TRUE;
                         }
                    }
                  else
                    break;
                  it = (Elm_Genlist_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it);
               }
          }
     }
   else
     {
        int expand_num = 0;
        int expand_order = 0;
        if (wd->expanded_item) it = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) wd->expanded_item);
        while (it)
          {
             expand_num++;
             it = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it);
          }
        if (wd->expanded_item) it = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) wd->expanded_item);
        while (it)
          {
             expand_order++;
             if (wd->expanded_item->expanded_depth >= it->expanded_depth) break;
             if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_EXPAND)
               {
                  if (!it->effect_done)
                    {
                       if (t >= (((expand_order - 1) * time) / expand_num))
                         {
                            edje_object_signal_emit(VIEW(it), "flip_item", "");
                            evas_object_show(VIEW(it));
                            it->effect_done = EINA_TRUE;
                         }
                    }
               }
             it = (Elm_Genlist_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it);
          }
     }

   if (end)
     {
        if (wd->item_moving_effect_timer)
          {
             if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_CONTRACT)
                _item_subitems_clear(wd->expanded_item);
             EINA_INLIST_FOREACH(wd->blocks, itb)
               {
                  EINA_LIST_FOREACH(itb->items, l, it)
                    {
                       it->effect_done = EINA_TRUE;
                       it->old_scrl_y = it->scrl_y;
                    }
               }
          }
        wd->item_moving_effect_timer = NULL;
        wd->expanded_move_effect_set = EINA_FALSE;

        _item_auto_scroll(wd);
        evas_object_lower(wd->alpha_bg);
        evas_object_hide(wd->alpha_bg);
        elm_smart_scroller_bounce_animator_disabled_set(wd->scr, EINA_FALSE);
        wd->move_effect_mode = ELM_GENLIST_ITEM_MOVE_EFFECT_NONE;

        evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
        evas_object_smart_callback_call(wd->obj, "effect_done", NULL);
        evas_object_smart_changed(wd->pan_smart);
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_emit_contract(Elm_Genlist_Item *it)
{
   Elm_Genlist_Item *it2;
   Eina_List *l;

   edje_object_signal_emit(VIEW(it), "elm,state,contract_flip", "");
   it->effect_done = EINA_FALSE;

   EINA_LIST_FOREACH(it->items, l, it2)
      if (it2)
         _emit_contract(it2);
}

// added for item moving animation.
static int
_item_flip_effect_show(Elm_Genlist_Item *it)
{
   Elm_Genlist_Item *it2;
   Eina_List *l;
   Widget_Data *wd = it->wd;

   EINA_LIST_FOREACH(it->items, l, it2)
     {
        if (it2->parent && it == it2->parent)
          {
             if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_EXPAND)
               edje_object_signal_emit(it2->base.view, "elm,state,hide", "");
             else if (wd->move_effect_mode == ELM_GENLIST_ITEM_MOVE_EFFECT_CONTRACT)
               _emit_contract(it2);
          }
     }

   return ECORE_CALLBACK_CANCEL;
}

/*
static void
_elm_genlist_pinch_zoom_execute(Evas_Object *obj, Eina_Bool emode)
{
   printf("!!! NOW FIXING \n");
}
*/

/**
 * Set pinch zoom mode
 *
 * @param obj The genlist object
 * @param emode
 * (EINA_TRUE = pinch contract (zoom in), EINA_FALSE = pinch expand (zoom out)
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_pinch_zoom_mode_set(Evas_Object *obj __UNUSED__, Eina_Bool emode __UNUSED__)
{
   printf("!!! NOW FIXING \n");
}

/**
 * Get pinch zoom mode
 *
 * @param obj The genlist object
 * @return The pinch mode
 * (EINA_TRUE = pinch contract (zoom in), EINA_FALSE = pinch expand (zoom out)
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool
elm_genlist_pinch_zoom_mode_get(const Evas_Object *obj __UNUSED__)
{
   printf("!!! NOW FIXING \n");
   return EINA_FALSE;
}

EAPI void
elm_genlist_pinch_zoom_set(Evas_Object *obj __UNUSED__, Eina_Bool emode __UNUSED__)
{
   printf("!!! NOW FIXING \n");
}


////////////////////////////////////////////////////////////////////////
//  EDIT  MODE
////////////////////////////////////////////////////////////////////////

static void
_effect_item_controls(Elm_Genlist_Item *it, int itx, int ity)
{
   if (!it->wd->edit_mode) return;
   evas_object_resize(it->edit_obj,it->w, it->h);
   evas_object_move(it->edit_obj, itx, ity);
}

static void
_effect_item_realize(Elm_Genlist_Item *it, Eina_Bool effect_on)
{
   if ((it->effect_item_realized) || (it->delete_me)) return;
   char buf[1024];

   if (it->mode_view)
     _mode_finished_signal_cb(it, (Evas_Object *)elm_genlist_item_object_get((Elm_Object_Item *) it), NULL, NULL);

   it->edit_obj = edje_object_add(evas_object_evas_get(WIDGET(it)));
   edje_object_scale_set(it->edit_obj, elm_widget_scale_get(WIDGET(it)) *
                         _elm_config->scale);
   evas_object_smart_member_add(it->edit_obj, it->wd->pan_smart);
   elm_widget_sub_object_add(WIDGET(it), it->edit_obj);

   if (it->flags & ELM_GENLIST_ITEM_SUBITEMS) strncpy(buf, "tree", sizeof(buf));
   else strncpy(buf, "item", sizeof(buf));
   if (it->wd->compress) strncat(buf, "_compress", sizeof(buf) - strlen(buf));

   strncat(buf, "/", sizeof(buf) - strlen(buf));

   if (it->itc->edit_item_style && strcmp(it->itc->edit_item_style, "default"))
     {
        strncat(buf, it->itc->edit_item_style, sizeof(buf) - strlen(buf));
        _elm_theme_object_set(WIDGET(it), it->edit_obj, "genlist", buf, elm_widget_style_get(WIDGET(it)));
     }
   else
     {
        _elm_theme_object_set(WIDGET(it), it->edit_obj, "genlist", "item/edit_default", elm_widget_style_get(WIDGET(it)));
     }

   if (it->wd->reorder_mode)
     {
        if (effect_on) edje_object_signal_emit(it->edit_obj, "elm,state,reorder_enabled_effect", "elm");
        else edje_object_signal_emit(it->edit_obj, "elm,state,reorder_enabled", "elm");
     }
   if (effect_on) edje_object_signal_emit(it->edit_obj, "elm,state,emode_enabled_effect", "elm");
   else edje_object_signal_emit(it->edit_obj, "elm,state,emode_enabled", "elm");

   if (it->disabled) edje_object_signal_emit(it->edit_obj, "elm,state,disabled", "elm");
   else edje_object_signal_emit(it->edit_obj, "elm,state,enabled", "elm");


   if ((it->wd->edit_mode))
     {
        if (it->itc->func.content_get)
          {
             const Eina_List *l;
             const char *key;

             if (it->renamed) edje_object_signal_emit(it->edit_obj, "elm,state,rename,enabled", "elm");
             it->contents = elm_widget_stringlist_get(edje_object_data_get(it->edit_obj, "edit_contents"));
             EINA_LIST_FOREACH(it->contents, l, key)
               {
                  Evas_Object *ic = it->itc->func.content_get
                     ((void *)it->base.data, WIDGET(it), l->data);

                  if (ic)
                    {
                       it->edit_content_objs = eina_list_append(it->edit_content_objs, ic);
                       edje_object_part_swallow(it->edit_obj, key, ic);
                       evas_object_show(ic);
                       elm_widget_sub_object_add(WIDGET(it), ic);
                       if (it->disabled)
                         elm_widget_disabled_set(ic, EINA_TRUE);
                    }
               }
          }
     }
   edje_object_part_swallow(it->edit_obj, "original_edc", VIEW(it));
   _effect_item_controls(it,it->scrl_x, it->scrl_y);
   evas_object_show(it->edit_obj);

   it->effect_item_realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;
}

static void
_effect_item_unrealize(Elm_Genlist_Item *it)
{
   Evas_Object *content;

   if (!it->effect_item_realized) return;
   if (it->wd->reorder_it && it->wd->reorder_it == it) return;

   edje_object_signal_emit(it->edit_obj, "elm,state,reorder_disable_effect", "elm");
   edje_object_signal_emit(it->edit_obj, "elm,state,sel,disable", "elm");
   edje_object_message_signal_process(it->edit_obj);
   edje_object_part_unswallow(it->edit_obj, VIEW(it));
   evas_object_smart_member_add(VIEW(it), it->wd->pan_smart);
   elm_widget_sub_object_add(WIDGET(it), VIEW(it));
   //   evas_object_smart_callback_call(it->edit_obj, "unrealized", it);
   //   _item_cache_add(it);
   evas_object_del(it->edit_obj);
   it->edit_obj = NULL;
   EINA_LIST_FREE(it->edit_content_objs, content)
      evas_object_del(content);

//   edje_object_signal_emit(it->edit_obj, "elm,state,edit_end,disable", "elm");
   it->effect_item_realized = EINA_FALSE;
}

/**
 * Get Genlist edit mode
 *
 * @param obj The genlist object
 * @return The edit mode status
 * (EINA_TRUE = edit mode, EINA_FALSE = normal mode
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool
elm_genlist_edit_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   if (wd->edit_mode) return EINA_TRUE;
   else return EINA_FALSE;
}

/**
 * Set Genlist edit mode
 *
 * This sets Genlist edit mode.
 *
 * @param obj The Genlist object
 * @param emode ELM_GENLIST_EDIT_MODE_{NONE & REORDER & INSERT & DELETE & SELECT & SELECT_ALL}
 * @param edit_class Genlist edit class (Elm_Genlist_Edit_Class structure)
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_edit_mode_set(Evas_Object *obj, Eina_Bool edit_mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Item_Block *itb;
   Eina_List *l;
   Elm_Genlist_Item *it;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->edit_mode == edit_mode) return;

   wd->edit_mode = edit_mode;
   if (!wd->edit_mode)
     {
        EINA_INLIST_FOREACH(wd->blocks, itb)
          {
             if (itb->realized)
               {
                  EINA_LIST_FOREACH(itb->items, l, it)
                    {
                       if (it->flags != ELM_GENLIST_ITEM_GROUP && it->realized)
                         {
                            _effect_item_unrealize(it);
                         }
                    }
               }
          }
        _item_cache_zero(wd);
     }
   else
     {

        EINA_INLIST_FOREACH(wd->blocks, itb)
          {
             if (itb->realized)
               {
                  EINA_LIST_FOREACH(itb->items, l, it)
                    {
                       if (it->flags != ELM_GENLIST_ITEM_GROUP && it->realized)
                         {
                            if(it->selected) _item_unselect(it);
                            if (it->itc->edit_item_style) _effect_item_realize(it, EINA_TRUE);
                         }
                    }
                }
           }
      }

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

/**
 * Set a given item's rename mode
 *
 * This renames the item's text from genlist
 *
 * @param it The item
 * @param renamed set if emode is EINA_TRUE, unset if emode is EINA_FALSE
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_rename_mode_set(Elm_Object_Item *it, Eina_Bool renamed)
{
   if (!it) return;
   Elm_Genlist_Item *_it = (Elm_Genlist_Item *) it;

   if (renamed)
     {
        _item_unrealize(_it, EINA_FALSE);
        _it->renamed = EINA_TRUE;
        _it->wd->rename_it = _it;
        _it->nocache = EINA_TRUE;
        if (_it->selected)  _item_unselect(_it);

        if (_it->wd->calc_job) ecore_job_del(_it->wd->calc_job);
        _it->wd->calc_job = ecore_job_add(_calc_job, _it->wd);
     }
   else
     {
        if (_it->renamed)
          {
             _it->renamed = EINA_FALSE;
             _it->nocache = EINA_TRUE;
             _it->wd->rename_it = NULL;
             _item_cache_zero(_it->wd);
             elm_genlist_item_update(it);
          }
     }
}

EAPI Eina_Bool
elm_genlist_item_rename_mode_get(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   return ((Elm_Genlist_Item *) it)->renamed;
}

EAPI void
elm_genlist_item_no_select_mode_set(Elm_Object_Item *it,
                                    Eina_Bool    no_select)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   ((Elm_Genlist_Item *) it)->no_select = no_select;
}

EAPI Eina_Bool
elm_genlist_item_no_select_mode_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   return ((Elm_Genlist_Item *) it)->no_select;
}

void
_elm_genlist_item_del_notserious(Elm_Genlist_Item *it)
{
   elm_widget_item_pre_notify_del(it);
   if (it->selected) it->wd->selected = eina_list_remove(it->wd->selected, it);

   if (it->itc->func.del)
     it->itc->func.del((void *)it->base.data, WIDGET(it));
}

void
_elm_genlist_item_del_serious(Elm_Genlist_Item *it)
{
   _elm_genlist_item_del_notserious(it);
   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->wd->walking -= it->walking;
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->flags & ELM_GENLIST_ITEM_GROUP)
     it->wd->group_items = eina_list_remove(it->wd->group_items, it);

/*
   if (it->wd->state)
     {
        eina_inlist_sorted_state_free(it->wd->state);
        it->wd->state = NULL;
     }
*/
   if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
   it->wd->calc_job = ecore_job_add(_calc_job, it->wd);

   elm_genlist_item_del((Elm_Object_Item *) it);
   if (it->wd->last_selected_item == (Elm_Object_Item *) it)
     it->wd->last_selected_item = NULL;
}


/* for gengrid as of now */
void
_elm_genlist_current_page_get(const Evas_Object *obj,
                              int               *h_pagenumber,
                              int               *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_current_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

/* for gengrid as of now */
void
_elm_genlist_last_page_get(const Evas_Object *obj,
                           int               *h_pagenumber,
                           int               *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_last_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

/* for gengrid as of now */
void
_elm_genlist_page_show(const Evas_Object *obj,
                       int                h_pagenumber,
                       int                v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_page_show(wd->scr, h_pagenumber, v_pagenumber);
}

/* for gengrid as of now */
void
_elm_genlist_page_bring_in(const Evas_Object *obj,
                           int                h_pagenumber,
                           int                v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_page_bring_in(wd->scr, h_pagenumber, v_pagenumber);
}

/* for gengrid as of now */
void
_elm_genlist_page_relative_set(Evas_Object *obj,
                               double       h_pagerel,
                               double       v_pagerel)
{
   Evas_Coord pagesize_h;
   Evas_Coord pagesize_v;

   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, NULL, NULL, &pagesize_h, &pagesize_v);
   elm_smart_scroller_paging_set(wd->scr, h_pagerel, v_pagerel, pagesize_h,
                                 pagesize_v);
}

/* for gengrid as of now */
void
_elm_genlist_page_relative_get(const Evas_Object *obj,
                               double            *h_pagerel,
                               double            *v_pagerel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, h_pagerel, v_pagerel, NULL, NULL);
}

/* for gengrid as of now */
void
_elm_genlist_page_size_set(Evas_Object *obj,
                           Evas_Coord   h_pagesize,
                           Evas_Coord   v_pagesize)
{
   double pagerel_h;
   double pagerel_v;

   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_paging_get(wd->scr, &pagerel_h, &pagerel_v, NULL, NULL);
   elm_smart_scroller_paging_set(wd->scr, pagerel_h, pagerel_v, h_pagesize,
                                 v_pagesize);
}

Elm_Genlist_Item *
_elm_genlist_item_new(Widget_Data              *wd,
                      const Elm_Genlist_Item_Class *itc,
                      const void               *data,
                      Elm_Genlist_Item             *parent,
                      Evas_Smart_Cb             func,
                      const void               *func_data)
{
   Elm_Genlist_Item *it;

   it = elm_widget_item_new(wd->obj, Elm_Genlist_Item);
   if (!it) return NULL;
   it->wd = wd;
   it->itc = itc;
   it->base.data = data;
   it->parent = parent;
   it->func.func = func;
   it->func.data = func_data;
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   /* TEMPORARY */
//   it->sel_cb = (Ecore_Cb)_item_select;

   elm_widget_item_text_get_hook_set(it, _item_text_hook);
   return it;
}

void
_elm_genlist_item_unrealize(Elm_Genlist_Item *it,
                            Eina_Bool     calc)
{
   Evas_Object *content;

   if (!it->realized) return;
   if (it->wd->reorder_it == it) return;
   evas_event_freeze(evas_object_evas_get(WIDGET(it)));
   if (!calc)
     evas_object_smart_callback_call(WIDGET(it), SIG_UNREALIZED, it);
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }

   elm_widget_stringlist_free(it->texts);
   it->texts = NULL;
  elm_widget_stringlist_free(it->contents);
   it->contents = NULL;
   elm_widget_stringlist_free(it->states);
   it->states = NULL;

   EINA_LIST_FREE(it->content_objs, content)
     evas_object_del(content);

   //it->unrealize_cb(it);
   _item_unrealize(it, EINA_FALSE);

   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(WIDGET(it)));
   evas_event_thaw_eval(evas_object_evas_get(WIDGET(it)));
}
