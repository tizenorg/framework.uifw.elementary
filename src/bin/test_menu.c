#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static Evas_Object *menu;

static void
_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   elm_menu_move(data, ev->canvas.x, ev->canvas.y);
   evas_object_show(data);
}

static void
_populate_4(Elm_Object_Item *menu_it)
{
   Elm_Object_Item *menu_it2;

   elm_menu_item_add(menu, menu_it, "go-bottom", "menu 2", NULL, NULL);
   elm_menu_item_add(menu, menu_it, "go-first", "menu 3", NULL, NULL);
   elm_menu_item_separator_add(menu, menu_it);
   menu_it2 = elm_menu_item_add(menu, menu_it, "go-last", "Disabled item", NULL,
                                NULL);
   elm_object_item_disabled_set(menu_it2, EINA_TRUE);
   menu_it2 = elm_menu_item_add(menu, menu_it, "go-next", "Disabled item", NULL,
                                NULL);
   elm_object_item_disabled_set(menu_it2, EINA_TRUE);
   menu_it2 = elm_menu_item_add(menu, menu_it, "go-up", "Disabled item", NULL,
                                NULL);
   elm_object_item_disabled_set(menu_it2, EINA_TRUE);
}

static void
_populate_3(Elm_Object_Item *menu_it)
{
   Elm_Object_Item *menu_it2;

   elm_menu_item_add(menu, menu_it, "media-eject", "menu 2", NULL, NULL);
   elm_menu_item_add(menu, menu_it, "media-playback-start", "menu 3", NULL,
                     NULL);
   elm_menu_item_separator_add(menu, menu_it);
   menu_it2 = elm_menu_item_add(menu, menu_it, "media-playback-stop",
                                "Disabled item", NULL, NULL);
   elm_object_item_disabled_set(menu_it2, EINA_TRUE);
}

static void
_populate_2(Elm_Object_Item *menu_it)
{
   Elm_Object_Item *menu_it2, *menu_it3;

   elm_menu_item_add(menu, menu_it, "system-reboot", "menu 2", NULL, NULL);
   menu_it2 = elm_menu_item_add(menu, menu_it, "system-shutdown", "menu 3",
                                NULL, NULL);
   _populate_3(menu_it2);

   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);

   menu_it2 = elm_menu_item_add(menu, menu_it, "system-lock-screen", "menu 2",
                                NULL, NULL);
   elm_menu_item_separator_add(menu, menu_it);

   menu_it3 = elm_menu_item_add(menu, menu_it, "system-run", "Disabled item",
                                NULL, NULL);
   elm_object_item_disabled_set(menu_it3, EINA_TRUE);

   _populate_4(menu_it2);
}

static void
_populate_1(Evas_Object *obj, Elm_Object_Item *menu_it)
{
   Elm_Object_Item *menu_it2, *menu_it3;
   Evas_Object *radio;

   radio = elm_radio_add(obj);
   elm_radio_state_value_set(radio, 0);
   elm_radio_value_set(radio, 0);
   elm_object_text_set(radio, "radio in menu");
   menu_it2 = elm_menu_item_add(menu, menu_it, "object-rotate-left", "menu 1",
                                NULL, NULL);
   menu_it3 = elm_menu_item_add(menu, menu_it, NULL, NULL, NULL, NULL);
   elm_object_item_content_set(menu_it3, radio);

   _populate_2(menu_it2);
}

void
test_menu(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *rect;
   Elm_Object_Item *menu_it;

   win = elm_win_util_standard_add("menu", "Menu");
   elm_win_autodel_set(win, EINA_TRUE);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_win_resize_object_add(win, rect);
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   menu = elm_menu_add(win);
   elm_menu_item_add(menu, NULL, NULL, "first item", NULL, NULL);

   menu_it = elm_menu_item_add(menu, NULL, "mail-reply-all", "second item",
                               NULL, NULL);
   _populate_1(win, menu_it);

   elm_menu_item_add(menu, menu_it, "window-new", "sub menu", NULL, NULL);

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _show, menu);

   evas_object_resize(win, 350, 200);
   evas_object_show(win);
}

#endif
