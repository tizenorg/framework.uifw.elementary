/*
 * TODO:
 *  - child elements focusing support
 *  - user defined icon/label cb
 *  - show/hide/add buttons ???
 *  - show/hide hidden files
 *  - double click to choose a file
 *  - multi-selection
 *  - make variable/function names that are sensible
 *  - Filter support
 */

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EIO
# include <Eio.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   EINA_REFCOUNT;

   Evas_Object *edje;
   Evas_Object *filename_entry;
   Evas_Object *path_entry;
   Evas_Object *files_list;
   Evas_Object *files_grid;
   Evas_Object *up_button;
   Evas_Object *home_button;

   Evas_Object *ok_button;
   Evas_Object *cancel_button;

   const char  *path;
   const char  *selection;
   Ecore_Idler *sel_idler;

   const char  *path_separator;

#ifdef HAVE_EIO
   Eio_File    *current;
#endif

   Elm_Fileselector_Mode mode;

   Eina_Bool    only_folder : 1;
   Eina_Bool    expand : 1;
};

struct sel_data
{
   Evas_Object *fs;
   const char  *path;
};

typedef struct _Widget_Request Widget_Request;
struct _Widget_Request
{
   Widget_Data *wd;
   Elm_Object_Item *parent;

   Evas_Object *obj;
   const char *path;
   Eina_Bool first : 1;
};

typedef enum {
  ELM_DIRECTORY = 0,
  ELM_FILE_IMAGE = 1,
  ELM_FILE_UNKNOW = 2,
  ELM_FILE_LAST
} Elm_Fileselector_Type;

static Elm_Genlist_Item_Class *list_itc[ELM_FILE_LAST];
static Elm_Gengrid_Item_Class *grid_itc[ELM_FILE_LAST];

static const char *widtype = NULL;

static const char SIG_DIRECTORY_OPEN[] = "directory,open";
static const char SIG_DONE[] = "done";
static const char SIG_SELECTED[] = "selected";
static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_DIRECTORY_OPEN, "s"},
   {SIG_DONE, "s"},
   {SIG_SELECTED, "s"},
   {NULL, NULL}
};

static void _populate(Evas_Object      *obj,
                      const char       *path,
                      Elm_Object_Item  *parent);
static void _do_anchors(Evas_Object *obj,
                        const char  *path);

/***  ELEMENTARY WIDGET  ***/
static void
_widget_data_free(Widget_Data *wd)
{
   if (wd->path) eina_stringshare_del(wd->path);
   if (wd->selection) eina_stringshare_del(wd->selection);
   if (wd->sel_idler)
     {
        void *sd;

        sd = ecore_idler_del(wd->sel_idler);
        free(sd);
     }
   free(wd);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   int i;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   for (i = 0; i < ELM_FILE_LAST; ++i)
     {
        elm_genlist_item_class_free(list_itc[i]);
        elm_gengrid_item_class_free(grid_itc[i]);
     }

#ifdef HAVE_EIO
   if (wd->current)
     eio_file_cancel(wd->current);
#endif

   wd->files_list = NULL;
   wd->files_grid = NULL;

   EINA_REFCOUNT_UNREF(wd)
     _widget_data_free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->edje, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_mirrored_set(wd->cancel_button, rtl);
   elm_widget_mirrored_set(wd->ok_button, rtl);
   elm_widget_mirrored_set(wd->files_list, rtl);
   elm_widget_mirrored_set(wd->up_button, rtl);
   elm_widget_mirrored_set(wd->home_button, rtl);
   edje_object_mirrored_set(wd->edje, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *style = elm_widget_style_get(obj);
   const char *data;
   char buf[1024];

   if (!wd) return;
   _elm_widget_mirrored_reload(obj);

   _elm_theme_object_set(obj, wd->edje, "fileselector", "base", style);

   if (elm_object_disabled_get(obj))
     edje_object_signal_emit(wd->edje, "elm,state,disabled", "elm");

   data = edje_object_data_get(wd->edje, "path_separator");
   if (data)
     wd->path_separator = data;
   else
     wd->path_separator = "/";

   if (!style) style = "default";
   snprintf(buf, sizeof(buf), "fileselector/%s", style);

#define SWALLOW(part_name, object_ptn)                                \
  if (object_ptn)                                                     \
    {                                                                 \
       elm_widget_style_set(object_ptn, buf);                         \
       if (edje_object_part_swallow(wd->edje, part_name, object_ptn)) \
         evas_object_show(object_ptn);                                \
       else                                                           \
         evas_object_hide(object_ptn);                                \
    }
   SWALLOW("elm.swallow.up", wd->up_button);
   SWALLOW("elm.swallow.home", wd->home_button);

   if (wd->mode == ELM_FILESELECTOR_LIST)
     {
        if (edje_object_part_swallow(wd->edje, "elm.swallow.files",
                                     wd->files_list))
          {
             evas_object_show(wd->files_list);
             evas_object_hide(wd->files_grid);
          }
        else
          evas_object_hide(wd->files_list);
     }
   else
     {
        if (edje_object_part_swallow(wd->edje, "elm.swallow.files",
                                     wd->files_grid))
          {
             evas_object_show(wd->files_grid);
             evas_object_hide(wd->files_list);
          }
        else
          evas_object_hide(wd->files_grid);
     }

   SWALLOW("elm.swallow.filename", wd->filename_entry);
   SWALLOW("elm.swallow.path", wd->path_entry);

   snprintf(buf, sizeof(buf), "fileselector/actions/%s", style);
   SWALLOW("elm.swallow.cancel", wd->cancel_button);
   SWALLOW("elm.swallow.ok", wd->ok_button);
#undef SWALLOW

   edje_object_message_signal_process(wd->edje);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   edje_object_scale_set
     (wd->edje, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

/***  GENLIST "MODEL"  ***/
static char *
_itc_text_get(void              *data,
               Evas_Object *obj   __UNUSED__,
               const char *source __UNUSED__)
{
   return elm_entry_utf8_to_markup(ecore_file_file_get(data)); /* NOTE this will be
                                                                * free() by the
                                                                * caller */
}

static Evas_Object *
_itc_icon_folder_get(void        *data __UNUSED__,
                     Evas_Object *obj,
                     const char  *source)
{
   Evas_Object *ic;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "folder");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL,
                                    1, 1);
   return ic;
}

static Evas_Object *
_itc_icon_image_get(void        *data,
                    Evas_Object *obj,
                    const char  *source)
{
   const char *filename = data;
   Evas_Object *ic;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "image");
   elm_icon_thumb_set(ic, filename, NULL);

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL,
                                    1, 1);
   return ic;
}

static Evas_Object *
_itc_icon_file_get(void        *data __UNUSED__,
                   Evas_Object *obj,
                   const char  *source)
{
   Evas_Object *ic;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "file");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL,
                                    1, 1);
   return ic;
}

static Eina_Bool
_itc_state_get(void *data         __UNUSED__,
               Evas_Object *obj   __UNUSED__,
               const char *source __UNUSED__)
{
   return EINA_FALSE;
}

static void
_itc_del(void            *data,
         Evas_Object *obj __UNUSED__)
{
   eina_stringshare_del(data);
}

static void
_expand_done(void            *data,
             Evas_Object *obj __UNUSED__,
             void            *event_info)
{
   Elm_Object_Item *it = event_info;
   const char *path = elm_object_item_data_get(it);
   _populate(data, path, it);
}

static void
_contract_done(void *data       __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void            *event_info)
{
   Elm_Object_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static void
_expand_req(void *data       __UNUSED__,
            Evas_Object *obj __UNUSED__,
            void            *event_info)
{
   Elm_Object_Item *it = event_info;
   elm_genlist_item_expanded_set(it, EINA_TRUE);
}

static void
_contract_req(void *data       __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void            *event_info)
{
   Elm_Object_Item *it = event_info;
   elm_genlist_item_expanded_set(it, EINA_FALSE);
}

/***  PRIVATES  ***/
static Eina_Bool
_sel_do(void *data)
{
   struct sel_data *sd;
   const char *path;
   Widget_Data *wd;
   const char *p;

   sd = data;
   wd = elm_widget_data_get(sd->fs);
   path = sd->path;

   if ((!wd->only_folder) && ecore_file_is_dir(path))
     {
        if (wd->expand && wd->mode == ELM_FILESELECTOR_LIST)
          {
             _do_anchors(sd->fs, path);
             elm_object_text_set(wd->filename_entry, "");
          }
        else
          {
             /* keep a ref to path 'couse it will be destroyed by _populate */
             p = eina_stringshare_add(path);
             _populate(sd->fs, p, NULL);
             eina_stringshare_del(p);
          }
        goto end;
     }
   else /* navigating through folders only or file is not a dir. */
     {
        char *s;
        
        if (wd->expand && wd->mode == ELM_FILESELECTOR_LIST)
          _do_anchors(sd->fs, path);
        else if (wd->only_folder)
          {
             /* keep a ref to path 'couse it will be destroyed by _populate */
             p = eina_stringshare_add(path);
             _populate(sd->fs, p, NULL);
             eina_stringshare_del(p);
          }
        s = elm_entry_utf8_to_markup(ecore_file_file_get(path));
        if (s)
          {
             elm_object_text_set(wd->filename_entry, s);
             free(s);
          }
        else
          elm_object_text_set(wd->filename_entry, "");
     }

   evas_object_smart_callback_call(sd->fs, SIG_SELECTED, (void *)path);

end:
   wd->sel_idler = NULL;
   free(sd);
   return ECORE_CALLBACK_CANCEL;
}

static void
_sel(void            *data,
     Evas_Object *obj __UNUSED__,
     void            *event_info)
{
   struct sel_data *sd;
   Widget_Data *wd;
   void *old_sd;
   char *dir;
   //This event_info could be a list or gengrid item
   Elm_Object_Item *it = event_info;

   wd = elm_widget_data_get(data);
   if (!wd) return;

   sd = malloc(sizeof(*sd));
   sd->fs = data;
   sd->path = elm_object_item_data_get(it);

   if (!sd->path)
     {
        eina_stringshare_replace(&wd->path, "");
        goto end;
     }

   dir = wd->only_folder ? strdup(sd->path) : ecore_file_dir_get(sd->path);
   if (dir)
     {
        eina_stringshare_replace(&wd->path, dir);
        free(dir);
     }
   else
     {
        eina_stringshare_replace(&wd->path, "");
     }

end:
   if (wd->sel_idler)
     {
        old_sd = ecore_idler_del(wd->sel_idler);
        free(old_sd);
     }
   wd->sel_idler = ecore_idler_add(_sel_do, sd);
}

static void
_up(void            *data,
    Evas_Object *obj __UNUSED__,
    void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   char *parent;

   Widget_Data *wd = elm_widget_data_get(fs);
   if (!wd) return;
   parent = ecore_file_dir_get(wd->path);
   _populate(fs, parent, NULL);
   free(parent);
}

static void
_home(void            *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   _populate(fs, getenv("HOME"), NULL);
}

static void
_ok(void            *data,
    Evas_Object *obj __UNUSED__,
    void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   evas_object_smart_callback_call(fs, SIG_DONE,
                                   (void *)elm_fileselector_selected_get(fs));
}

static void
_canc(void            *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   evas_object_smart_callback_call(fs, SIG_DONE, NULL);
}

static void
_anchor_clicked(void            *data,
                Evas_Object *obj __UNUSED__,
                void            *event_info)
{
   Evas_Object *fs = data;
   Widget_Data *wd = elm_widget_data_get(fs);
   Elm_Entry_Anchor_Info *info = event_info;
   const char *p;
   if (!wd) return;
   // keep a ref to path 'couse it will be destroyed by _populate
   p = eina_stringshare_add(info->name);
   _populate(fs, p, NULL);
   evas_object_smart_callback_call(data, SIG_SELECTED, (void *)p);
   eina_stringshare_del(p);
}

static void
_do_anchors(Evas_Object *obj,
            const char  *path)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char **tok, buf[PATH_MAX * 3], *s;
   int i, j;
   
   if (!wd) return;
   s = elm_entry_utf8_to_markup(path);
   if (!s) return;
   buf[0] = '\0';
   tok = eina_str_split(s, "/", 0);
   free(s);
   eina_strlcat(buf, "<a href=/>root</a>", sizeof(buf));
   for (i = 0; tok[i]; i++)
     {
        if ((!tok[i]) || (!tok[i][0])) continue;
        eina_strlcat(buf, wd->path_separator, sizeof(buf));
        eina_strlcat(buf, "<a href=", sizeof(buf));
        for (j = 0; j <= i; j++)
          {
             if (strlen(tok[j]) < 1) continue;
             eina_strlcat(buf, "/", sizeof(buf));
             eina_strlcat(buf, tok[j], sizeof(buf));
          }
        eina_strlcat(buf, ">", sizeof(buf));
        eina_strlcat(buf, tok[i], sizeof(buf));
        eina_strlcat(buf, "</a>", sizeof(buf));
     }
   free(tok[0]);
   free(tok);

   elm_object_text_set(wd->path_entry, buf);
}

#ifdef HAVE_EIO
static Eina_Bool
_filter_cb(void *data __UNUSED__, Eio_File *handler, const Eina_File_Direct_Info *info)
{
   const char *filename;

   if (info->path[info->name_start] == '.')
     return EINA_FALSE;

   filename = eina_stringshare_add(info->path);
   eio_file_associate_direct_add(handler, "filename", filename, EINA_FREE_CB(eina_stringshare_del));

   if (info->type == EINA_FILE_DIR)
     {
        eio_file_associate_direct_add(handler, "type/grid", grid_itc[ELM_DIRECTORY], NULL);
        eio_file_associate_direct_add(handler, "type/list", list_itc[ELM_DIRECTORY], NULL);
     }
   else
     {
        if (evas_object_image_extension_can_load_get(info->path + info->name_start))
          {
             eio_file_associate_direct_add(handler, "type/grid", grid_itc[ELM_FILE_IMAGE], NULL);
             eio_file_associate_direct_add(handler, "type/list", list_itc[ELM_FILE_IMAGE], NULL);
          }
        else
          {
             eio_file_associate_direct_add(handler, "type/grid", grid_itc[ELM_FILE_UNKNOW], NULL);
             eio_file_associate_direct_add(handler, "type/list", list_itc[ELM_FILE_UNKNOW], NULL);
          }
     }

   return EINA_TRUE;
}

static int
_file_grid_cmp(const void *a, const void *b)
{
   const Elm_Object_Item *ga = a;
   const Elm_Object_Item *gb = b;
   const Elm_Gengrid_Item_Class *ca = elm_gengrid_item_item_class_get(ga);
   const Elm_Gengrid_Item_Class *cb = elm_gengrid_item_item_class_get(gb);

   if (ca == grid_itc[ELM_DIRECTORY])
     {
        if (cb != grid_itc[ELM_DIRECTORY])
          return -1;
     }
   else if (cb == grid_itc[ELM_DIRECTORY])
     {
        return 1;
     }

   return strcoll(elm_object_item_data_get(ga), elm_object_item_data_get(gb));
}

static int
_file_list_cmp(const void *a, const void *b)
{
   const Elm_Object_Item *la = a;
   const Elm_Object_Item *lb = b;
   const Elm_Genlist_Item_Class *ca = elm_genlist_item_item_class_get(la);
   const Elm_Genlist_Item_Class *cb = elm_genlist_item_item_class_get(lb);

   if (ca == list_itc[ELM_DIRECTORY])
     {
        if (cb != list_itc[ELM_DIRECTORY])
          return -1;
     }
   else if (cb == list_itc[ELM_DIRECTORY])
     {
        return 1;
     }

   return strcoll(elm_object_item_data_get(la), elm_object_item_data_get(lb));
}

static void
_signal_first(Widget_Request *wr)
{
   if (!wr->first) return ;
   evas_object_smart_callback_call(wr->obj, SIG_DIRECTORY_OPEN, (void *)wr->path);
   if (!wr->parent)
     {
        elm_genlist_clear(wr->wd->files_list);
        elm_gengrid_clear(wr->wd->files_grid);
        eina_stringshare_replace(&wr->wd->path, wr->path);
        _do_anchors(wr->obj, wr->path);
     }

   if (wr->wd->filename_entry) elm_object_text_set(wr->wd->filename_entry, "");

   wr->first = EINA_FALSE;
}

static void
_main_cb(void *data, Eio_File *handler, const Eina_File_Direct_Info *info __UNUSED__)
{
   Widget_Request *wr = data;

   if (eio_file_check(handler))
     return ;
   if (!wr->wd->files_list || !wr->wd->files_grid || wr->wd->current != handler)
     {
        eio_file_cancel(handler);
        return ;
     }

   _signal_first(wr);

   if (wr->wd->mode == ELM_FILESELECTOR_LIST)
     {
        Eina_Bool is_dir = (eio_file_associate_find(handler, "type/list") == list_itc[ELM_DIRECTORY]);

        elm_genlist_item_sorted_insert(wr->wd->files_list, eio_file_associate_find(handler, "type/list"),
                                       eina_stringshare_ref(eio_file_associate_find(handler, "filename")),
                                       wr->parent, wr->wd->expand && is_dir ? ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                                       _file_list_cmp, NULL, NULL);
     }
   else if (wr->wd->mode == ELM_FILESELECTOR_GRID)
     elm_gengrid_item_sorted_insert(wr->wd->files_grid, eio_file_associate_find(handler, "type/grid"),
                                           eina_stringshare_ref(eio_file_associate_find(handler, "filename")),
                                           _file_grid_cmp, NULL, NULL);
}

static void
_widget_request_cleanup(Widget_Request *wr)
{
   EINA_REFCOUNT_UNREF(wr->wd)
     _widget_data_free(wr->wd);

   eina_stringshare_del(wr->path);
   free(wr);
}

static void
_done_cb(void *data, Eio_File *handler __UNUSED__)
{
   Widget_Request *wr = data;

   _signal_first(wr);

   wr->wd->current = NULL;
   _widget_request_cleanup(wr);
}

static void
_error_cb(void *data, Eio_File *handler, int error __UNUSED__)
{
   Widget_Request *wr = data;

   if (wr->wd->current == handler)
     wr->wd->current = NULL;
   _widget_request_cleanup(wr);
}

#endif

static void
_populate(Evas_Object      *obj,
          const char       *path,
          Elm_Object_Item  *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
#ifdef HAVE_EIO
   Widget_Request *wr;
#else
   Eina_File_Direct_Info *file;
   Eina_Iterator *it;
   const char *real;
   Eina_List *files = NULL, *dirs = NULL;
#endif

   if (!wd) return;
#ifndef HAVE_EIO
   if (!ecore_file_is_dir(path)) return ;
   it = eina_file_stat_ls(path);
   if (!it) return ;
   evas_object_smart_callback_call(obj, SIG_DIRECTORY_OPEN, (void *)path);
   if (!parent)
     {
        elm_genlist_clear(wd->files_list);
        elm_gengrid_clear(wd->files_grid);
        eina_stringshare_replace(&wd->path, path);
        _do_anchors(obj, path);
     }

   if (wd->filename_entry) elm_object_text_set(wd->filename_entry, "");
   EINA_ITERATOR_FOREACH(it, file)
     {
        const char *filename;

        if (file->path[file->name_start] == '.')
          continue ;

        filename = eina_stringshare_add(file->path);
        if (file->type == EINA_FILE_DIR)
          dirs = eina_list_append(dirs, filename);
        else if (!wd->only_folder)
          files = eina_list_append(files, filename);
     }
   eina_iterator_free(it);

   files = eina_list_sort(files, eina_list_count(files),
                          EINA_COMPARE_CB(strcoll));
   dirs = eina_list_sort(dirs, eina_list_count(dirs), EINA_COMPARE_CB(strcoll));
   EINA_LIST_FREE(dirs, real)
     {
        if (wd->mode == ELM_FILESELECTOR_LIST)
          elm_genlist_item_append(wd->files_list, list_itc[ELM_DIRECTORY],
                                  real, /* item data */
                                  parent,
                                  wd->expand ? ELM_GENLIST_ITEM_TREE :
                                  ELM_GENLIST_ITEM_NONE,
                                  NULL, NULL);
        else if (wd->mode == ELM_FILESELECTOR_GRID)
          elm_gengrid_item_append(wd->files_grid, grid_itc[ELM_DIRECTORY],
                                  real, /* item data */
                                  NULL, NULL);
     }

   EINA_LIST_FREE(files, real)
     {
        Elm_Fileselector_Type type = evas_object_image_extension_can_load_fast_get(real) ?
          ELM_FILE_IMAGE : ELM_FILE_UNKNOW;

        if (wd->mode == ELM_FILESELECTOR_LIST)
          elm_genlist_item_append(wd->files_list, list_itc[type],
                                  real, /* item data */
                                  parent, ELM_GENLIST_ITEM_NONE,
                                  NULL, NULL);
        else if (wd->mode == ELM_FILESELECTOR_GRID)
          elm_gengrid_item_append(wd->files_grid, grid_itc[type],
                                  real, /* item data */
                                  NULL, NULL);
     }
#else
   if (wd->expand && wd->current) return ;
   if (wd->current)
     eio_file_cancel(wd->current);
   wr = malloc(sizeof (Widget_Request));
   if (!wr) return ;
   wr->wd = wd;
   EINA_REFCOUNT_REF(wr->wd);
   wr->parent = parent; /* FIXME: should we refcount the parent ? */
   wr->obj = obj;
   wr->path = eina_stringshare_add(path);
   wr->first = EINA_TRUE;

   wd->current = eio_file_stat_ls(path,
                                  _filter_cb,
                                  _main_cb,
                                  _done_cb,
                                  _error_cb,
                                  wr);
#endif
}

/***  API  ***/

EAPI Evas_Object *
elm_fileselector_add(Evas_Object *parent)
{
   Evas *e;
   Evas_Object *obj, *ic, *bt, *li, *en, *grid;
   Widget_Data *wd;
   unsigned int i;
   int s;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   EINA_REFCOUNT_INIT(wd);

   ELM_SET_WIDTYPE(widtype, "fileselector");
   elm_widget_type_set(obj, "fileselector");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->expand = !!_elm_config->fileselector_expand_enable;

   wd->edje = edje_object_add(e);
   _elm_theme_object_set(obj, wd->edje, "fileselector", "base", "default");
   elm_widget_resize_object_set(obj, wd->edje);

   // up btn
   ic = elm_icon_add(parent);
   elm_icon_standard_set(ic, "arrow_up");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(parent);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_part_content_set(bt, "icon", ic);
   elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Up"));
   evas_object_size_hint_align_set(bt, 0.0, 0.0);

   evas_object_smart_callback_add(bt, "clicked", _up, obj);

   elm_widget_sub_object_add(obj, bt);
   wd->up_button = bt;

   // home btn
   ic = elm_icon_add(parent);
   elm_icon_standard_set(ic, "home");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(parent);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_part_content_set(bt, "icon", ic);
   elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Home"));
   evas_object_size_hint_align_set(bt, 0.0, 0.0);

   evas_object_smart_callback_add(bt, "clicked", _home, obj);

   elm_widget_sub_object_add(obj, bt);
   wd->home_button = bt;

   for (i = 0; i < ELM_FILE_LAST; ++i)
     {
        list_itc[i] = elm_genlist_item_class_new();
        grid_itc[i] = elm_gengrid_item_class_new();

        list_itc[i]->item_style = "default";
        list_itc[i]->func.text_get = grid_itc[i]->func.text_get = _itc_text_get;
        list_itc[i]->func.state_get = grid_itc[i]->func.state_get = _itc_state_get;
        list_itc[i]->func.del = grid_itc[i]->func.del = _itc_del;
     }

   list_itc[ELM_DIRECTORY]->func.content_get =
     grid_itc[ELM_DIRECTORY]->func.content_get = _itc_icon_folder_get;
   list_itc[ELM_FILE_IMAGE]->func.content_get =
     grid_itc[ELM_FILE_IMAGE]->func.content_get = _itc_icon_image_get;
   list_itc[ELM_FILE_UNKNOW]->func.content_get =
     grid_itc[ELM_FILE_UNKNOW]->func.content_get = _itc_icon_file_get;

   li = elm_genlist_add(parent);
   elm_widget_mirrored_automatic_set(li, EINA_FALSE);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(li, 100, 100);

   grid = elm_gengrid_add(parent);
   elm_widget_mirrored_automatic_set(grid, EINA_FALSE);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   // XXX: will fail for dynamic finger size changing
   s = _elm_config->finger_size * 2;
   elm_gengrid_item_size_set(grid, s, s);
   elm_gengrid_align_set(grid, 0.0, 0.0);

   evas_object_smart_callback_add(li, "selected", _sel, obj);
   evas_object_smart_callback_add(li, "expand,request", _expand_req, obj);
   evas_object_smart_callback_add(li, "contract,request", _contract_req, obj);
   evas_object_smart_callback_add(li, "expanded", _expand_done, obj);
   evas_object_smart_callback_add(li, "contracted", _contract_done, obj);

   evas_object_smart_callback_add(grid, "selected", _sel, obj);

   elm_widget_sub_object_add(obj, li);
   elm_widget_sub_object_add(obj, grid);
   wd->files_list = li;
   wd->files_grid = grid;

   // path entry
   en = elm_entry_add(parent);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_widget_mirrored_automatic_set(en, EINA_FALSE);
   elm_entry_editable_set(en, EINA_FALSE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

   evas_object_smart_callback_add(en, "anchor,clicked", _anchor_clicked, obj);

   elm_widget_sub_object_add(obj, en);
   wd->path_entry = en;

   // filename entry
   en = elm_entry_add(parent);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_widget_mirrored_automatic_set(en, EINA_FALSE);
   elm_entry_editable_set(en, EINA_TRUE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_widget_sub_object_add(obj, en);
   wd->filename_entry = en;

   elm_fileselector_buttons_ok_cancel_set(obj, EINA_TRUE);
   elm_fileselector_is_save_set(obj, EINA_FALSE);

   _theme_hook(obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

EAPI void
elm_fileselector_is_save_set(Evas_Object *obj,
                             Eina_Bool    is_save)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_object_disabled_set(wd->filename_entry, !is_save);

   if (is_save)
     edje_object_signal_emit(wd->edje, "elm,state,save,on", "elm");
   else
     edje_object_signal_emit(wd->edje, "elm,state,save,off", "elm");
}

EAPI Eina_Bool
elm_fileselector_is_save_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return !elm_object_disabled_get(wd->filename_entry);
}

EAPI void
elm_fileselector_folder_only_set(Evas_Object *obj,
                                 Eina_Bool    only)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->only_folder == only) return;
   wd->only_folder = !!only;
   if (wd->path) _populate(obj, wd->path, NULL);
}

EAPI Eina_Bool
elm_fileselector_folder_only_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->only_folder;
}

EAPI void
elm_fileselector_buttons_ok_cancel_set(Evas_Object *obj,
                                       Eina_Bool    visible)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *bt;
   if (!wd) return;

   if (visible)
     {
        // cancel btn
        bt = elm_button_add(obj);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Cancel"));

        evas_object_smart_callback_add(bt, "clicked", _canc, obj);

        elm_widget_sub_object_add(obj, bt);
        wd->cancel_button = bt;

        // ok btn
        bt = elm_button_add(obj);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_object_domain_translatable_text_set(bt, PACKAGE, N_("OK"));

        evas_object_smart_callback_add(bt, "clicked", _ok, obj);

        elm_widget_sub_object_add(obj, bt);
        wd->ok_button = bt;

        _theme_hook(obj);
     }
   else
     {
        evas_object_del(wd->cancel_button);
        wd->cancel_button = NULL;
        evas_object_del(wd->ok_button);
        wd->ok_button = NULL;
     }
}

EAPI Eina_Bool
elm_fileselector_buttons_ok_cancel_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->ok_button ? EINA_TRUE : EINA_FALSE;
}

EAPI void
elm_fileselector_expandable_set(Evas_Object *obj,
                                Eina_Bool    expand)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->expand = !!expand;

   if (wd->path) _populate(obj, wd->path, NULL);
}

EAPI Eina_Bool
elm_fileselector_expandable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->expand;
}

EAPI void
elm_fileselector_path_set(Evas_Object *obj,
                          const char  *_path)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   char *path;
   path = ecore_file_realpath(_path);
   _populate(obj, path, NULL);
   free(path);
}

EAPI const char *
elm_fileselector_path_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->path;
}

EAPI void
elm_fileselector_mode_set(Evas_Object          *obj,
                          Elm_Fileselector_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (mode == wd->mode) return;

   if (mode == ELM_FILESELECTOR_LIST)
     {
        if (edje_object_part_swallow(wd->edje, "elm.swallow.files",
                                     wd->files_list))
          {
             evas_object_show(wd->files_list);
             evas_object_hide(wd->files_grid);
          }
        else
          evas_object_hide(wd->files_list);
     }
   else
     {
        if (edje_object_part_swallow(wd->edje, "elm.swallow.files",
                                     wd->files_grid))
          {
             evas_object_show(wd->files_grid);
             evas_object_hide(wd->files_list);
          }
        else
          evas_object_hide(wd->files_grid);
     }

   wd->mode = mode;

   _populate(obj, wd->path, NULL);
}

EAPI Elm_Fileselector_Mode
elm_fileselector_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_FILESELECTOR_LAST;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_FILESELECTOR_LAST;

   return wd->mode;
}

EAPI const char *
elm_fileselector_selected_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   if (!wd->path) return NULL;
   
   if (wd->filename_entry)
     {
        const char *name;
        char buf[PATH_MAX];
        char *dir, *s;

        dir = wd->only_folder ? ecore_file_dir_get(wd->path) : strdup(wd->path);
        name = elm_object_text_get(wd->filename_entry);
        if (name)
          {
             s = elm_entry_markup_to_utf8(name);
             if (s)
               {
                  snprintf(buf, sizeof(buf), "%s/%s", dir, s);
                  free(s);
               }
             else
               snprintf(buf, sizeof(buf), "%s", dir);
          }
        else
          {
             snprintf(buf, sizeof(buf), "%s", dir);
          }
        if (wd->only_folder && !ecore_file_is_dir(buf))
          eina_stringshare_replace(&wd->selection, ecore_file_dir_get(buf));
        else
          eina_stringshare_replace(&wd->selection, buf);
        if (dir) free(dir);
        return wd->selection;
     }

   if (wd->mode == ELM_FILESELECTOR_LIST)
     {
        Elm_Object_Item *gl_it = elm_genlist_selected_item_get(wd->files_list);
        if (gl_it) return elm_object_item_data_get(gl_it);
     }
   else
     {
        Elm_Object_Item *gg_it = elm_gengrid_selected_item_get(wd->files_grid);
        if (gg_it) return elm_object_item_data_get(gg_it);
     }

   return wd->path;
}

EAPI Eina_Bool
elm_fileselector_selected_set(Evas_Object *obj,
                              const char  *_path)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   Eina_Bool ret = EINA_TRUE;
   char *path;
   path = ecore_file_realpath(_path);

   if (ecore_file_is_dir(path))
     _populate(obj, path, NULL);
   else
     {
        if (!ecore_file_exists(path))
          {
             ret = EINA_FALSE;
             goto clean_up;
          }

        _populate(obj, ecore_file_dir_get(path), NULL);
        if (wd->filename_entry)
          {
             char *s;
             
             s = elm_entry_utf8_to_markup(ecore_file_file_get(path));
             if (s)
               {
                  elm_object_text_set(wd->filename_entry, s);
                  free(s);
               }
             else
               elm_object_text_set(wd->filename_entry, "");
             eina_stringshare_replace(&wd->selection, path);
          }
     }

clean_up:
   free(path);
   return ret;
}

