#include <stdio.h>
#include <math.h>
#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

#define ANI_STEP (14 * elm_scale_get())
#define ANI_TIME (0.005)
#define ANI_TIME_MSEC (12)
#define CLICK_TIME_MAX (180)
#define CLICK_WIDTH_MIN (elm_finger_size_get() >> 1)
#define FLICK_TIME_MAX (200)
#define FLICK_WIDTH_MIN (elm_finger_size_get() >> 2)
#define MOVE_STEP (3)
#define STEP_WEIGHT_DEF (1)
#define STEP_WEIGHT_MAX (2)
#define STEP_WEIGHT_MIN (0)
#define MOVING_IMAGE_SIZE (128)
#define MAX_ZOOM_SIZE (6)
#define INTERVAL_WIDTH (15)
#define MULTITOUCHDEVICE (11)

// Enumeration for layout.
enum
{
   BLOCK_LEFT = 0,
   BLOCK_CENTER,
   BLOCK_RIGHT,
   BLOCK_MAX
};

// Image Slider Item.
struct _Imageslider_Item
{
   Evas_Object *obj;
   const char *photo_file;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   void *data;
};

// Image Slider Widget Data.
struct _Widget_Data
{
   Evas_Object *ly[BLOCK_MAX];
   Evas_Object *clip;
   Eina_List *its;
   Eina_List *cur;
   Evas_Coord x, y, w, h;
   Evas_Object *obj;
   Ecore_Idler *queue_idler;
   Ecore_Timer *anim_timer;

   Evas_Coord_Point down_pos;
   Evas_Coord move_x;
   Evas_Coord move_y;
   Evas_Coord dest_x;
   struct timeval tv;
   unsigned int timestamp;
   int step;
   int move_cnt;
   int ani_lock:1;
   int moving:1;
};

// Global value declaration.
static const char *widtype = NULL;

static const char SIG_CLICKED[] = "clicked";

// Internal function declaration.
static void _del_hook(Evas_Object *obj);

static void _theme_hook(Evas_Object *obj);

static void _sizing_eval(Evas_Object *obj);

static void _imageslider_move(void *data, Evas * e, Evas_Object *obj, void *event_info);
static void _imageslider_resize(void *data, Evas * e, Evas_Object *obj, void *event_info);
static void _imageslider_show(void *data, Evas * e, Evas_Object *obj, void *event_info);
static void _imageslider_hide(void *data, Evas * e, Evas_Object *obj, void *event_info);
static void _imageslider_update(Widget_Data * wd);

static void _imageslider_update_pos(Widget_Data * wd, Evas_Coord x, Evas_Coord y, Evas_Coord w);

static void _imageslider_obj_shift(Widget_Data * wd, Eina_Bool left);

static void _imageslider_obj_move(Widget_Data * wd, Evas_Coord step);

static Eina_Bool _icon_to_image(void *data);

static int _check_drag(int state, void *data);

static void _anim(Widget_Data * wd);

static Eina_Bool _timer_cb(void *data);

static void _ev_imageslider_down_cb(void *data, Evas * e, Evas_Object *obj, void *event_info);
static void _ev_imageslider_up_cb(void *data, Evas * e, Evas_Object *obj, void *event_info);
static void _ev_imageslider_move_cb(void *data, Evas * e, Evas_Object *obj, void *event_info);

// Whenever the Image Slider item is deleted, Call this funtion.
static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);

   if (!wd) return;

   if (wd->its)
     {
        eina_list_free(wd->its);
        wd->its = NULL;
     }

   if (wd->queue_idler)
     {
        ecore_idler_del(wd->queue_idler);
        wd->queue_idler = NULL;
     }

   if (wd->anim_timer)
     {
        ecore_timer_del(wd->anim_timer);
        wd->anim_timer = NULL;
     }

   if (wd) free(wd);
}

// Whenever require processing theme, Call this function
static void
_theme_hook(Evas_Object *obj)
{
   int i = 0;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   for (i = 0; i < BLOCK_MAX; i++)
     {
        if (wd->ly[i])
           elm_layout_theme_set(wd->ly[i], "imageslider", "base",
                                elm_object_style_get(obj));
        evas_object_show(wd->ly[i]);
     }
   _imageslider_update(wd);
   _sizing_eval(obj);
}

// Resize Image Slider item.
static void
_sizing_eval(Evas_Object *obj)
{
   Evas *e;

   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd)
     return;

   e = evas_object_evas_get(wd->obj);

   _imageslider_move(obj, e, obj, NULL);
   _imageslider_resize(obj, e, obj, NULL);
}

// Whenever MOVE event occurs, Call this function.
static void
_imageslider_move(void *data, Evas * e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd;

   Evas_Coord x, y;

   if (!data)
     return;

   wd = elm_widget_data_get((Evas_Object *) data);
   if (!wd)
     return;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   wd->x = x;
   wd->y = y;

   _imageslider_update_pos(wd, wd->x, wd->y, wd->w);
}

// Whenever RESIZE event occurs, Call this fucntion.
static void
_imageslider_resize(void *data, Evas * e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   int i;

   Widget_Data *wd;

   Evas_Coord w, h;

   if (!data)
     return;

   wd = elm_widget_data_get((Evas_Object *) data);
   if (!wd)
     return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   DBG("%d %d -resize\n", w, h);
   wd->w = w;
   wd->h = h;

   for (i = 0; i < BLOCK_MAX; i++)
     {
        evas_object_resize(wd->ly[i], w, h);
     }

   _imageslider_update_pos(wd, wd->x, wd->y, wd->w);

}

// Whenever SHOW event occurs, Call this function.
static void
_imageslider_show(void *data, Evas * e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd;

   if (!data)
     {
        return;
     }

   wd = elm_widget_data_get((Evas_Object *) data);
   if (!wd) return;

   evas_object_show(wd->clip);
}

// Whenever HIDE event occurs, Call this function.
static void
_imageslider_hide(void *data, Evas * e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd;

   if (!data) return;

   wd = elm_widget_data_get((Evas_Object *) data);
   if (!wd) return;
   evas_object_hide(wd->clip);
}

// Update Image Slider item position.
static void
_imageslider_update_pos(Widget_Data * wd, Evas_Coord x, Evas_Coord y, Evas_Coord w)
{
   evas_object_move(wd->ly[BLOCK_LEFT], x - (w + INTERVAL_WIDTH), y);
   evas_object_move(wd->ly[BLOCK_CENTER], x, y);
   evas_object_move(wd->ly[BLOCK_RIGHT], x + (w + INTERVAL_WIDTH), y);
}

// Shift next/previous Image Slider item in layouts.
static void
_imageslider_obj_shift(Widget_Data * wd, Eina_Bool left)
{
   if (!left)
     {
        Evas_Object *ly_temp;
        ly_temp = wd->ly[BLOCK_LEFT];
        wd->ly[BLOCK_LEFT] = wd->ly[BLOCK_CENTER];
        wd->ly[BLOCK_CENTER] = wd->ly[BLOCK_RIGHT];
        wd->ly[BLOCK_RIGHT] = ly_temp;
        elm_object_part_content_set(wd->ly[BLOCK_RIGHT], "swl.photo", NULL);
     }
   else
     {
        Evas_Object *ly_temp;
        ly_temp = wd->ly[BLOCK_RIGHT];
        wd->ly[BLOCK_RIGHT] = wd->ly[BLOCK_CENTER];
        wd->ly[BLOCK_CENTER] = wd->ly[BLOCK_LEFT];
        wd->ly[BLOCK_LEFT] = ly_temp;
        elm_object_part_content_set(wd->ly[BLOCK_LEFT], "swl.photo", NULL);
     }
}

// Move the current Image Slider item and update.
static void
_imageslider_obj_move(Widget_Data * wd, Evas_Coord step)
{
   if (step > 0)
     {
        wd->cur = eina_list_next(wd->cur);
        if (wd->cur == NULL)
          {
             wd->cur = eina_list_last(wd->its);
             wd->step = ANI_STEP;
          }
        else
          {
             wd->step = -ANI_STEP;
             wd->move_x += wd->w;
             _imageslider_obj_shift(wd, EINA_FALSE);
          }
        wd->moving = EINA_TRUE;
     }
   else if (step < 0)
     {
        wd->cur = eina_list_prev(wd->cur);
        if (wd->cur == NULL)
          {
             wd->cur = wd->its;
             wd->step = -ANI_STEP;
          }
        else
          {
             wd->step = ANI_STEP;
             wd->move_x -= wd->w;
             _imageslider_obj_shift(wd, EINA_TRUE);
          }
        wd->moving = EINA_TRUE;
     }
   else
     {
        if (wd->move_x < 0)
          wd->step = ANI_STEP;
        else
          wd->step = -ANI_STEP;
        wd->moving = EINA_FALSE;
     }

   _imageslider_update(wd);
}

// Whenever MOUSE DOWN event occurs, Call this function.
static void
_ev_imageslider_down_cb(void *data, Evas * e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (wd->ani_lock)
      return;

   wd->down_pos = ev->canvas;
   wd->timestamp = ev->timestamp;
   wd->move_cnt = MOVE_STEP;
}

// Whenever MOUSE UP event occurs, Call this function.
// And make Click Event also.
static void
_ev_imageslider_up_cb(void *data, Evas * e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = data;

   Evas_Event_Mouse_Up *ev = event_info;

   Evas_Coord step;

   int interval;

   if (wd->ani_lock) return;

   step = wd->down_pos.x - ev->canvas.x;
   interval = ev->timestamp - wd->timestamp;
   if (step == 0 || interval == 0)
     {
        DBG("case1: emit CLICK event\n");
        evas_object_smart_callback_call(wd->obj, SIG_CLICKED, NULL);
        return;
     }
   if (interval < CLICK_TIME_MAX)
     {
        if (step < CLICK_WIDTH_MIN && step > CLICK_WIDTH_MIN)
          {
             DBG("case2: emit CLICK event\n");
             evas_object_smart_callback_call(wd->obj, SIG_CLICKED, NULL);
             return;
          }
     }

   if (interval < FLICK_TIME_MAX)
     {
        if (step < FLICK_WIDTH_MIN && step > FLICK_WIDTH_MIN)
          {
             DBG("ev_imageslider_up_cb-black zone (1)\n");

             _imageslider_obj_move(wd, 0);
          }
        else
          {
             DBG("ev_imageslider_up_cb-black zone (2)\n");

             _imageslider_obj_move(wd, step);
          }

     }
   else
     {
        step = (wd->x - wd->move_x) << 1;
        if (step <= wd->w && step >= -(wd->w))
          {
             DBG("ev_imageslider_up_cb-white zone (1)\n");

             _imageslider_obj_move(wd, 0);
          }
        else
          {
             DBG("ev_imageslider_up_cb-white zone (2)\n");

             _imageslider_obj_move(wd, step);
          }
     }
}

// Whenever MOUSE MOVE event occurs, Call this
static void
_ev_imageslider_move_cb(void *data, Evas * e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   int idx = 0;
   Evas_Coord step;
   Widget_Data *wd = data;
   Evas_Event_Mouse_Move *ev = event_info;

   if (wd->ani_lock) return;
   if (wd->move_cnt == MOVE_STEP)
     {
        wd->move_cnt = 0;

        if (ev->buttons)
          {
             step = ev->cur.canvas.x - wd->down_pos.x;
             if (step > 0)
               idx = BLOCK_LEFT;
             else
               idx = BLOCK_RIGHT;
             wd->move_x = wd->x + ((ev->cur.canvas.x - wd->down_pos.x));
             wd->move_y = wd->y + ((ev->cur.canvas.y - wd->down_pos.y));
             _imageslider_update_pos(wd, wd->move_x, wd->y, wd->w);
          }
     }
   wd->move_cnt++;
}

static inline double
time_get(Evas_Coord x, Evas_Coord w)
{
   double time;

   time = (-sin(x / w) + 1) / 500;

   if (time == 0)
      time = ANI_TIME;

   return time;
}

static Eina_Bool
_icon_to_image(void *data)
{
   Widget_Data *wd = data;

   wd->moving = 0;
   _imageslider_update(wd);

   if (wd->queue_idler)
     wd->queue_idler = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static int
_check_drag(int state, void *data)
{
   Widget_Data *wd = data;

   Elm_Imageslider_Item *it;

   Evas_Coord ix = 0, iy = 0, iw = 0, ih = 0;

   double dx = 0, dy = 0;

   Eina_List *l[BLOCK_MAX];

   Evas_Object *eo = NULL;

   l[BLOCK_LEFT] = eina_list_prev(wd->cur);
   l[BLOCK_CENTER] = wd->cur;
   l[BLOCK_RIGHT] = eina_list_next(wd->cur);

   it = eina_list_data_get(l[state]);

   eo = elm_object_part_content_get(wd->ly[state], "swl.photo");
   if (eo)
     evas_object_geometry_get(eo, &ix, &iy, &iw, &ih);
   edje_object_part_drag_value_get(elm_layout_edje_get(wd->ly[state]), "swl.photo", &dx, &dy);

   if ((iw != wd->w) || ((dx != 0) || (dy != 0)))
     {
        elm_object_part_content_set(wd->ly[state], "swl.photo", NULL);
     }
   else
     return 1;

   return 0;
}

static Eina_Bool
_timer_cb(void *data)
{
   Widget_Data *wd;

   Elm_Imageslider_Item *it;

   struct timeval tv;

   int t = 0;
   int ret = 0;

   wd = data;
   if (!wd->ani_lock)
     {
        if (wd->anim_timer)
          wd->anim_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   gettimeofday(&tv, NULL);

   t = (tv.tv_sec - wd->tv.tv_sec) * 1000 + (tv.tv_usec - wd->tv.tv_usec) / 1000;
   gettimeofday(&wd->tv, NULL);

   t = t / ANI_TIME_MSEC;
   if (t <= STEP_WEIGHT_MIN)
     t = STEP_WEIGHT_DEF;
   else if (t > STEP_WEIGHT_MAX)
     t = STEP_WEIGHT_MAX;

   wd->move_x += (wd->step) * t;

   if (wd->step < 0 && wd->move_x < wd->x)
     wd->move_x = wd->x;
   else if (wd->step > 0 && wd->move_x > wd->x)
     wd->move_x = wd->x;

   _imageslider_update_pos(wd, wd->move_x, wd->y, wd->w);

   if (wd->move_x == wd->x)
     {
        wd->ani_lock = EINA_FALSE;
        if (wd->cur)
          {
             it = eina_list_data_get(wd->cur);
             if (it->func)
               it->func(it->data, wd->obj, it);
          }
        if (wd->cur)
          {
             it = eina_list_data_get(wd->cur);
             evas_object_smart_callback_call(wd->obj, "changed", it);
          }

        ret = _check_drag(BLOCK_LEFT, wd);
        ret = _check_drag(BLOCK_RIGHT, wd);

        if (!wd->queue_idler)
          wd->queue_idler = ecore_idler_add(_icon_to_image, wd);

        if (wd->anim_timer)
          wd->anim_timer = NULL;

        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_anim(Widget_Data * wd)
{
   Evas_Coord w;

   if (wd->x == wd->move_x)
     {
        _imageslider_update_pos(wd, wd->move_x, wd->y, wd->w);
        return;
     }

   wd->ani_lock = EINA_TRUE;

   w = wd->move_x;
   gettimeofday(&wd->tv, NULL);

   if (!wd->anim_timer)
     wd->anim_timer = ecore_timer_add(ANI_TIME, _timer_cb, wd);
}

// Update Image Slider Items.
static void
_imageslider_update(Widget_Data * wd)
{
   int i;

   Eina_List *l[BLOCK_MAX];

   Elm_Imageslider_Item *it;

   Evas_Object *eo;

   if (!wd) return;

   if (!wd->cur) return;

   l[BLOCK_LEFT] = eina_list_prev(wd->cur);
   l[BLOCK_CENTER] = wd->cur;
   l[BLOCK_RIGHT] = eina_list_next(wd->cur);

   for (i = 0; i < BLOCK_MAX; i++)
     {
        eo = elm_object_part_content_get((const Evas_Object*)wd->ly[i], "swl.photo");
        if (!l[i])
          {
             elm_object_part_content_set(wd->ly[i], "swl.photo", NULL);
          }
        else
          {
             it = eina_list_data_get(l[i]);
             if (!it)
               return;

             if (!eo)
               {
                  eo = elm_image_add(wd->obj);
                  //elm_image_prescale_set(eo, wd->w);
                  elm_image_file_set(eo, it->photo_file, NULL);
                  elm_object_part_content_set(wd->ly[i], "swl.photo", eo);
               }
          }
     }
   _anim(wd);
}

EAPI Evas_Object *
elm_imageslider_add(Evas_Object *parent)
{
   int i;

   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "imageslider");
   elm_widget_type_set(obj, "imageslider");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->clip = evas_object_rectangle_add(e);
   elm_widget_sub_object_add(obj, wd->clip);
   elm_widget_resize_object_set(obj, wd->clip);

   for (i = 0; i < BLOCK_MAX; i++)
     {
        wd->ly[i] = elm_layout_add(obj);
        elm_layout_theme_set(wd->ly[i], "imageslider", "base", "default");
        elm_widget_sub_object_add(obj, wd->ly[i]);
        evas_object_smart_member_add(wd->ly[i], obj);

        evas_object_event_callback_add(wd->ly[i], EVAS_CALLBACK_MOUSE_DOWN, _ev_imageslider_down_cb, wd);
        evas_object_event_callback_add(wd->ly[i], EVAS_CALLBACK_MOUSE_UP, _ev_imageslider_up_cb, wd);
        evas_object_event_callback_add(wd->ly[i], EVAS_CALLBACK_MOUSE_MOVE, _ev_imageslider_move_cb, wd);
        evas_object_clip_set(wd->ly[i], wd->clip);
        evas_object_show(wd->ly[i]);
     }

   wd->obj = obj;
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _imageslider_resize, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _imageslider_move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _imageslider_show, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _imageslider_hide, obj);

   _sizing_eval(obj);
   return obj;
}

EAPI Elm_Imageslider_Item *
elm_imageslider_item_append(Evas_Object *obj, const char *photo_file, Elm_Imageslider_Cb func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;

   Elm_Imageslider_Item *it;

   if (!obj || !(wd = elm_widget_data_get(obj)))
     return NULL;

   it = (Elm_Imageslider_Item *) calloc(1, sizeof(Elm_Imageslider_Item));
   if (!it)
      return NULL;
   it->photo_file = eina_stringshare_add(photo_file);
   it->func = func;
   it->data = data;
   it->obj = obj;
   wd->its = eina_list_append(wd->its, it);

   if (!wd->cur)
      wd->cur = wd->its;
   _imageslider_update(wd);
   return it;
}

EAPI Elm_Imageslider_Item *
elm_imageslider_item_append_relative(Evas_Object *obj, const char *photo_file, Elm_Imageslider_Cb func, unsigned int index, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;

   Elm_Imageslider_Item *it;

   DBG("New elm_imageslider_item_append_relative()\n");

   if (!obj || !(wd = elm_widget_data_get(obj)))
     return NULL;

   it = (Elm_Imageslider_Item *) calloc(1, sizeof(Elm_Imageslider_Item));
   if (!it)
      return NULL;

   it->obj = obj;
   it->photo_file = eina_stringshare_add(photo_file);
   it->func = func;
   it->data = data;

   wd->its = eina_list_append_relative(wd->its, it, eina_list_nth(wd->its,
                                                                  index - 2));

   if (!wd->cur)
      wd->cur = wd->its;
   _imageslider_update(wd);
   return it;
}

EAPI Elm_Imageslider_Item *
elm_imageslider_item_prepend(Evas_Object *obj, const char *photo_file, Elm_Imageslider_Cb func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;

   Elm_Imageslider_Item *it;

   if (!obj || !(wd = elm_widget_data_get(obj)))
     return NULL;

   it = (Elm_Imageslider_Item *) calloc(1, sizeof(Elm_Imageslider_Item));
   it->photo_file = eina_stringshare_add(photo_file);
   it->func = func;
   it->data = data;
   it->obj = obj;
   wd->its = eina_list_prepend(wd->its, it);
   if (!wd->cur)
      wd->cur = wd->its;
   _imageslider_update(wd);
   return it;
}

EAPI void
elm_imageslider_item_del(Elm_Imageslider_Item * it)
{
   Widget_Data *wd;

   Elm_Imageslider_Item *_it;

   Eina_List *l;

   if (!it || !(wd = elm_widget_data_get(it->obj)))
     return;

   EINA_LIST_FOREACH(wd->its, l, _it)
     {
        if (_it == it)
          {
             if (l == wd->cur)
                wd->cur = eina_list_prev(wd->cur);
             wd->its = eina_list_remove(wd->its, it);
             if (!wd->cur)
                wd->cur = wd->its;
             break;
          }
     }
   _imageslider_update(wd);
}

EAPI Elm_Imageslider_Item *
elm_imageslider_selected_item_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;

   if (!obj || (!(wd = elm_widget_data_get(obj))))
     return NULL;

   if (!wd->cur)
      return NULL;

   return eina_list_data_get(wd->cur);
}

EAPI Eina_Bool
elm_imageslider_item_selected_get(Elm_Imageslider_Item * it)
{
   Widget_Data *wd;

   if (!it || !it->obj || (!(wd = elm_widget_data_get(it->obj))))
     return EINA_FALSE;

   if (!wd->cur)
      return EINA_FALSE;

   if (eina_list_data_get(wd->cur) == it)
      return EINA_TRUE;
   else
      return EINA_FALSE;
}

EAPI void
elm_imageslider_item_selected_set(Elm_Imageslider_Item * it)
{
   int i;

   Widget_Data *wd;

   Elm_Imageslider_Item *_it;

   Eina_List *l;

   Evas_Object *eo;

   if (!it || !it->obj || (!(wd = elm_widget_data_get(it->obj))))
     return;

   EINA_LIST_FOREACH(wd->its, l, _it)
     {
        if (_it == it)
          wd->cur = l;
     }

   for (i = 0; i < BLOCK_MAX; i++)
     {
       eo = elm_object_part_content_get(wd->ly[i], "swl.photo");
       if (eo)
         elm_object_part_content_set(wd->ly[i], "swl.photo", NULL);
     }
   _imageslider_update(wd);
}

EAPI const char *
elm_imageslider_item_photo_file_get(Elm_Imageslider_Item * it)
{
   if (!it) return NULL;

   return it->photo_file;
}

EAPI void
elm_imageslider_item_photo_file_set(Elm_Imageslider_Item *it, const char *photo_file)
{
   if (!it) return;
   ELM_CHECK_WIDTYPE(it->obj, widtype);

   if (photo_file)
     {
        eina_stringshare_replace(&(it->photo_file), photo_file);
        elm_imageslider_item_update(it);
     }
}

EAPI Elm_Imageslider_Item *
elm_imageslider_item_prev(Elm_Imageslider_Item * it)
{
   Widget_Data *wd;

   Elm_Imageslider_Item *_it;

   Eina_List *l;

   if (!it || (!(wd = elm_widget_data_get(it->obj))))
     return NULL;

   EINA_LIST_FOREACH(wd->its, l, _it)
     {
        if (_it == it)
          {
             l = eina_list_prev(l);
             if (!l)
               break;
             return eina_list_data_get(l);
          }
     }
   return NULL;
}

EAPI Elm_Imageslider_Item *
elm_imageslider_item_next(Elm_Imageslider_Item * it)
{
   Widget_Data *wd;

   Elm_Imageslider_Item *_it;

   Eina_List *l;

   if (!it || (!(wd = elm_widget_data_get(it->obj))))
     return NULL;

   EINA_LIST_FOREACH(wd->its, l, _it)
     {
        if (_it == it)
          {
             l = eina_list_next(l);
             if (!l)
               break;
             return eina_list_data_get(l);
          }
     }
   return NULL;
}

EAPI void
elm_imageslider_prev(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   if (!obj || (!(wd = elm_widget_data_get(obj))))
     return;
   if (wd->ani_lock)
     return;
   _imageslider_obj_move(wd, -1);
}

EAPI void
elm_imageslider_next(Evas_Object * obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   if (!obj || (!(wd = elm_widget_data_get(obj))))
     return;
   if (wd->ani_lock)
     return;
   _imageslider_obj_move(wd, 1);
}

EAPI void
elm_imageslider_item_update(Elm_Imageslider_Item *it)
{
   Widget_Data *wd;

   if (!it || (!(wd = elm_widget_data_get(it->obj)))) return;
   ELM_CHECK_WIDTYPE(it->obj, widtype);

   if (wd->ani_lock) return;
   if (it == eina_list_data_get(eina_list_prev(wd->cur)))
     elm_object_part_content_set(wd->ly[BLOCK_LEFT], "swl.photo", NULL);
   else if (it == eina_list_data_get(wd->cur))
     elm_object_part_content_set(wd->ly[BLOCK_CENTER], "swl.photo", NULL);
   else if (it == eina_list_data_get(eina_list_next(wd->cur)))
     elm_object_part_content_set(wd->ly[BLOCK_RIGHT], "swl.photo", NULL);
   _imageslider_update(wd);
}