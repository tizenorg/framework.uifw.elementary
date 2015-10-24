/**
 * Simple Elementary's <b>diskselector widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g diskselector_example_01.c -o diskselector_example_01 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bx, *ds;
   unsigned int i;
   static const char *lbl[] =
     {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
     };

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("diskselector", "Diskselector Example");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   /* default */
   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_diskselector_item_append(ds, lbl[i], NULL, NULL, NULL);
   evas_object_show(ds);

   /* set round mode and define side label length */
   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_diskselector_item_append(ds, lbl[i], NULL, NULL, NULL);
   elm_diskselector_round_enabled_set(ds, EINA_TRUE);
   elm_diskselector_side_text_max_length_set(ds, 2);
   evas_object_show(ds);

   /* display more than 3 items */
   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_diskselector_item_append(ds, lbl[i], NULL, NULL, NULL);
   elm_diskselector_display_item_num_set(ds, 5);
   printf("Number of Items in DiskSelector : %d\n", elm_diskselector_display_item_num_get(ds));
   evas_object_show(ds);

   /* set bounce and scroller policy */
   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_diskselector_item_append(ds, lbl[i], NULL, NULL, NULL);
   elm_scroller_bounce_set(ds, EINA_TRUE, EINA_TRUE);
   elm_scroller_policy_set(ds, ELM_SCROLLER_POLICY_AUTO,
                           ELM_SCROLLER_POLICY_ON);
   evas_object_show(ds);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
