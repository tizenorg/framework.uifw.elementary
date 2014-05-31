//Compile with:
//gcc -g box_example_03.c -o box_example_03 `pkg-config --cflags --libs elementary`

#include <Elementary.h>
typedef struct
{
   Evas_Object *btn[100];
   Evas_Object *radio[100];
} Item_Data;


typedef struct
{
   int count;
   int select;
   Item_Data *item;
   Evas_Object *box;
   Evas_Object *radio_box;
   Evas_Object *radio_group;
   Evas_Object *weight_w_sl;
   Evas_Object *weight_h_sl;
   Evas_Object *align_w_sl;
   Evas_Object *align_h_sl;
} Test_Data;

static void
_select_cb(void *data, Evas_Object *obj, void *ev)
{
   Test_Data *tdata = data;
   double ww, wh, aw, ah;
   tdata->select = elm_radio_state_value_get(obj);

   evas_object_size_hint_weight_get(tdata->item->btn[tdata->select], &ww, &wh);
   elm_slider_value_set(tdata->weight_w_sl, ww);
   elm_slider_value_set(tdata->weight_h_sl, wh);

   evas_object_size_hint_align_get(tdata->item->btn[tdata->select], &aw, &ah);
   elm_slider_value_set(tdata->align_w_sl, aw);
   elm_slider_value_set(tdata->align_h_sl, ah);

   printf("%d button selected\n",tdata->select);
}
static void
_add_cb(void *data, Evas_Object *obj, void *ev)
{
   Evas_Object *btn, *radio;
   Test_Data *tdata = data;
   char buf[100];

   if (tdata->count < 100)
     {
        printf("%d box added\n",tdata->count+1);
        snprintf(buf, sizeof(buf), "Btn %d",tdata->count);
        btn = elm_button_add(tdata->box);
        elm_object_text_set(btn, buf);
        evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(tdata->box, btn);
        evas_object_show(btn);

        radio = elm_radio_add(tdata->radio_box);
        elm_object_text_set(radio, buf);
        elm_radio_state_value_set(radio, tdata->count);

        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(radio, "changed", _select_cb, tdata);
        elm_box_pack_end(tdata->radio_box, radio);
        evas_object_show(radio);

        tdata->item->btn[tdata->count] = btn;
        tdata->item->radio[tdata->count] = radio;
        if (tdata->count == 0)
          {
             tdata->radio_group = radio;
             _select_cb(tdata, radio, NULL);
          }
        else
          elm_radio_group_add(radio, tdata->radio_group);
        tdata->count++;

     }
   else printf("box is full\n");
}

static void
_unpack_cb(void *data, Evas_Object *obj, void *ev)
{
   Test_Data *tdata = data;
   if (tdata->count > 0)
     {
        printf("%d is deleted\n",tdata->count);
        tdata->count--;
        elm_box_unpack(tdata->box, tdata->item->btn[tdata->count]);
        elm_box_unpack(tdata->radio_box, tdata->item->radio[tdata->count]);
        evas_object_move(tdata->item->btn[tdata->count],0,0);
        evas_object_move(tdata->item->radio[tdata->count],0,0);
        evas_object_color_set(tdata->item->radio[tdata->count], 128, 64, 0, 128);
        evas_object_color_set(tdata->item->radio[tdata->count], 128, 64, 0, 128);
        evas_object_del(tdata->item->btn[tdata->count]);
        evas_object_del(tdata->item->radio[tdata->count]);
     }
}

static void
_cw_cb(void *data, Evas_Object *obj, void *event_info)
{
   Test_Data *tdata = data;
   double val = elm_slider_value_get(obj);
   double val2 = elm_slider_value_get(tdata->weight_h_sl);

   evas_object_size_hint_weight_set(tdata->item->btn[tdata->select], val, val2);
}

static void
_ch_cb(void *data, Evas_Object *obj, void *event_info)
{
   Test_Data *tdata = data;
   double val = elm_slider_value_get(obj);
   double val2 = elm_slider_value_get(tdata->weight_w_sl);

   evas_object_size_hint_weight_set(tdata->item->btn[tdata->select], val2, val);
}

static void
_caw_cb(void *data, Evas_Object *obj, void *event_info)
{
   Test_Data *tdata = data;
   double val = elm_slider_value_get(obj);
   double val2 = elm_slider_value_get(tdata->align_h_sl);

   evas_object_size_hint_align_set(tdata->item->btn[tdata->select], val, val2);
}

static void
_cah_cb(void *data, Evas_Object *obj, void *event_info)
{
   Test_Data *tdata = data;
   double val = elm_slider_value_get(obj);
   double val2 = elm_slider_value_get(tdata->align_w_sl);

   evas_object_size_hint_align_set(tdata->item->btn[tdata->select], val, val2);
}

static void
_vertical_cb(void *data, Evas_Object *obj, void *event_info)
{
   Test_Data *tdata = data;
   elm_box_horizontal_set(tdata->box, EINA_FALSE);
}

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   Evas_Object *win, *bg, *bigbox, *bx, *bx2, *bx3, *bx4, *bx5, *bx6, *bx7, *bt, *sc, *lb, *sl, *sl2, *cb;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   Test_Data *data = calloc(1, sizeof(Test_Data));
   data->count = 0;
   data->select = 0;
   data->item = calloc(1, sizeof(Item_Data));

   win = elm_win_add(NULL, "box-test", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Test");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bigbox = elm_box_add(win);
   evas_object_size_hint_weight_set(bigbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(bigbox, EINA_FALSE);
   elm_win_resize_object_add(win, bigbox);
   evas_object_show(bigbox);

   bx = elm_box_add(bigbox);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.2);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_pack_end(bigbox, bx);
   evas_object_show(bx);

   /* add & del */
   bx3 = elm_box_add(bigbox);
   evas_object_size_hint_weight_set(bx3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx3, EINA_FALSE);
   elm_box_pack_end(bx, bx3);
   evas_object_show(bx3);

   bt = elm_button_add(bx3);
   elm_object_text_set(bt, "Add");
   evas_object_smart_callback_add(bt, "clicked", _add_cb, data);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx3, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx3);
   elm_object_text_set(bt, "Del");
   evas_object_smart_callback_add(bt, "clicked", _unpack_cb, data);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx3, bt);
   evas_object_show(bt);

   /* radio scroll */
   sc = elm_scroller_add(bx);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_ON);
   elm_box_pack_end(bx, sc);
   evas_object_show(sc);

   bx4 = elm_box_add(sc);
   evas_object_size_hint_weight_set(bx4, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx4, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx4, EINA_FALSE);
   elm_object_content_set(sc, bx4);
   evas_object_show(bx4);
   data->radio_box = bx4;

   /* weight setting */
   bx5 = elm_box_add(bx);
   evas_object_size_hint_weight_set(bx5, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx5, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx5, EINA_FALSE);
   elm_box_pack_end(bx, bx5);
   evas_object_show(bx5);

   lb = elm_label_add(bx5);
   elm_object_text_set(lb, "weight");
   elm_box_pack_end(bx5, lb);
   evas_object_show(lb);

   sl = elm_slider_add(bx5);
   elm_slider_unit_format_set(sl, "%0.1lf");
   elm_slider_min_max_set(sl, 0, 10);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(sl, "changed", _cw_cb, data);
   elm_box_pack_end(bx5, sl);
   evas_object_show(sl);
   data->weight_w_sl = sl;

   sl = elm_slider_add(bx5);
   elm_slider_unit_format_set(sl, "%0.1lf");
   elm_slider_min_max_set(sl, 0, 10);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(sl, "changed", _ch_cb, data);
   elm_box_pack_end(bx5, sl);
   evas_object_show(sl);
   data->weight_h_sl = sl;

   /* align setting */
   bx6 = elm_box_add(bx);
   evas_object_size_hint_weight_set(bx6, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx6, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx6, EINA_FALSE);
   elm_box_pack_end(bx, bx6);
   evas_object_show(bx6);

   lb = elm_label_add(bx6);
   elm_object_text_set(lb, "align");
   elm_box_pack_end(bx6, lb);
   evas_object_show(lb);

   sl = elm_slider_add(bx6);
   elm_slider_unit_format_set(sl, "%0.1lf");
   elm_slider_min_max_set(sl, -1, 1);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(sl, "changed", _caw_cb, data);
   elm_box_pack_end(bx6, sl);
   evas_object_show(sl);
   data->align_w_sl = sl;

   sl = elm_slider_add(bx6);
   elm_slider_unit_format_set(sl, "%0.1lf");
   elm_slider_min_max_set(sl, -1, 1);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(sl, "changed", _cah_cb, data);
   elm_box_pack_end(bx6, sl);
   evas_object_show(sl);
   data->align_h_sl = sl;

   bx7 = elm_box_add(bx);
   evas_object_size_hint_weight_set(bx7, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx7, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx7, EINA_FALSE);
   elm_box_pack_end(bx, bx7);
   evas_object_show(bx7);


   cb = elm_check_add(bx7);
   elm_object_text_set(cb, "vertical");
   evas_object_smart_callback_add(cb, "changed", _vertical_cb, data);
   elm_box_pack_end(bx7,cb);
   evas_object_show(cb);

   bx2 = elm_box_add(bigbox);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.8);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bigbox, bx2);
   evas_object_show(bx2);
   data->box = bx2;

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
