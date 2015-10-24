#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

const char* Atspi_Name[] = {
    "invalid",
    "accelerator label",
    "alert",
    "animation",
    "arrow",
    "calendar",
    "canvas",
    "check box",
    "check menu item",
    "color chooser",
    "column header",
    "combo box",
    "dateeditor",
    "desktop icon",
    "desktop frame",
    "dial",
    "dialog",
    "directory pane",
    "drawing area",
    "file chooser",
    "filler",
    "focus traversable",
    "font chooser",
    "frame",
    "glass pane",
    "html container",
    "icon",
    "image",
    "internal frame",
    "label",
    "layered pane",
    "list",
    "list item",
    "menu",
    "menu bar",
    "menu item",
    "option pane",
    "page tab",
    "page tab list",
    "panel",
    "password text",
    "popup menu",
    "progress bar",
    "push button",
    "radio button",
    "radio menu item",
    "root pane",
    "row header",
    "scroll bar",
    "scroll pane",
    "separator",
    "slider",
    "spin button",
    "split pane",
    "status bar",
    "table",
    "table cell",
    "table column header",
    "table row header",
    "tearoff menu item",
    "terminal",
    "text",
    "toggle button",
    "tool bar",
    "tool tip",
    "tree",
    "tree table",
    "unknown",
    "viewport",
    "window",
    "extended",
    "header",
    "footer",
    "paragraph",
    "ruler",
    "application",
    "autocomplete",
    "editbar",
    "embedded",
    "entry",
    "chart",
    "caption",
    "document frame",
    "heading",
    "page",
    "section",
    "redundant object",
    "form",
    "link",
    "input method window",
    "table row",
    "tree item",
    "document spreadsheet",
    "document presentation",
    "document text",
    "document web",
    "document email",
    "comment",
    "list box",
    "grouping",
    "image map",
    "notification",
    "info bar",
    "last defined"
};


EOLIAN static int
_elm_interface_atspi_accessible_index_in_parent_get(Eo *obj, void *pd EINA_UNUSED)
{
   Eina_List *l, *children = NULL;
   Eo *chld, *parent = NULL;
   int ret = 0;

   eo_do(obj, parent = elm_interface_atspi_accessible_parent_get());
   if (!parent) return -1;

   eo_do(parent, children = elm_interface_atspi_accessible_children_get());
   if (!children) return -1;

   EINA_LIST_FOREACH(children, l, chld)
     {
       if (obj == chld)
         break;
       ret++;
     }
   if (ret == (int)eina_list_count(children))
     {
        ERR("Object %s not present in its AT-SPI parents (%s) children list! This should never happen.", eo_class_name_get(eo_class_get(obj)), eo_class_name_get(eo_class_get(parent)));
        ret = -1;
     }
   eina_list_free(children);
   return ret;
}

EOLIAN static Eo *
_elm_interface_atspi_accessible_parent_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Eo *parent = NULL;

   /* By default using Eo_Base object hierarchy */
   eo_do(obj, parent = eo_parent_get());
   if (!parent) return NULL;

   return eo_isa(parent, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN) ? parent : NULL;
}

EOLIAN static void
_elm_interface_atspi_accessible_parent_set(Eo *obj, void *priv EINA_UNUSED, Eo *new_parent EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_parent_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

EOLIAN Eina_List*
_elm_interface_atspi_accessible_attributes_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_attributes_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return NULL;
}

EOLIAN static Elm_Atspi_Role
_elm_interface_atspi_accessible_role_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return ELM_ATSPI_ROLE_UNKNOWN;
}

EOLIAN static void
_elm_interface_atspi_accessible_role_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Elm_Atspi_Role role EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_role_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

EOLIAN const char *
_elm_interface_atspi_accessible_role_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Elm_Atspi_Role role;

   eo_do(obj, role = elm_interface_atspi_accessible_role_get());

   return role > ELM_ATSPI_ROLE_LAST_DEFINED ? "" : Atspi_Name[role];
}

EOLIAN char *
_elm_interface_atspi_accessible_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_name_get\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return NULL;
}

EOLIAN static void
_elm_interface_atspi_accessible_name_set(Eo *obj, void *pd EINA_UNUSED, char *val EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_name_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

const char * _elm_interface_atspi_accessible_description_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_description_get\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return NULL;
}

EOLIAN static void
_elm_interface_atspi_accessible_description_set(Eo *obj, void *pd EINA_UNUSED, const char *val EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_description_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

EOLIAN static const char *
_elm_interface_atspi_accessible_localized_role_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   const char *ret = NULL;
   eo_do(obj, ret = elm_interface_atspi_accessible_role_name_get());
#ifdef ENABLE_NLS
   ret = gettext(ret);
#endif
   return ret;
}

EOLIAN static Eina_List *
_elm_interface_atspi_accessible_children_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Eina_List *children = NULL;
   Eina_Iterator *iter = NULL;
   Eo *chld;

   // By default use Eo_Base object hierarchy
   eo_do(obj, iter = eo_children_iterator_new());
   if (!iter) return NULL;

   EINA_ITERATOR_FOREACH(iter, chld)
     {
        if (eo_isa(chld, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
          children = eina_list_append(children, chld);
     }
   eina_iterator_free(iter);

   return children;
}

EOLIAN static Elm_Atspi_State_Set
_elm_interface_atspi_accessible_state_set_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Elm_Atspi_State_Set ret = 0;
   return ret;
}

//TIZEN_ONLY(20150709) : atspi relations api
EOLIAN Elm_Atspi_Relation_Set
_elm_interface_atspi_accessible_relation_set_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Elm_Atspi_Relation_Set ret = 0;
   WRN("The %s object does not implement the \"accessible_relation_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return ret;
}
//////////////////////////

EAPI void elm_atspi_attributes_list_free(Eina_List *list)
{
   Elm_Atspi_Attribute *attr;
   EINA_LIST_FREE(list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}

//TIZEN_ONLY(20150709) : atspi relations api
EAPI void elm_atspi_relation_free(Elm_Atspi_Relation *relation)
{
   eina_list_free(relation->objects);
   free(relation);
}

EAPI Elm_Atspi_Relation *elm_atspi_relation_clone(const Elm_Atspi_Relation *relation)
{
   Elm_Atspi_Relation *ret = calloc(sizeof(Elm_Atspi_Relation), 1);
   if (!ret) return NULL;

   ret->type = relation->type;
   ret->objects = eina_list_clone(relation->objects);
   return ret;
}

static Eina_Bool
_on_rel_obj_del(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Atspi_Relation_Set *set = data;
   Elm_Atspi_Relation *rel;
   Eina_List *l, *l2, *p, *p2;
   Eo *rel_obj;

   EINA_LIST_FOREACH_SAFE(*set, l, l2, rel)
     {
        EINA_LIST_FOREACH_SAFE(rel->objects, p, p2, rel_obj)
          {
             if (rel_obj == obj)
               rel->objects = eina_list_remove_list(rel->objects, p);
          }
        if (!rel->objects)
          {
             *set = eina_list_remove_list(*set, l);
             free(rel);
          }
     }
   return EINA_TRUE;
}

EAPI Eina_Bool elm_atspi_relation_set_relation_append(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type, const Eo *rel_obj)
{
   Elm_Atspi_Relation *rel;
   Eina_List *l;

   if (!eo_isa(rel_obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
     return EINA_FALSE;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             if (!eina_list_data_find(rel->objects, rel_obj))
               {
                  rel->objects = eina_list_append(rel->objects, rel_obj);
                  eo_do(rel_obj, eo_event_callback_add(EO_BASE_EVENT_DEL, _on_rel_obj_del, set));
               }
             return EINA_TRUE;
          }
     }

   rel = calloc(sizeof(Elm_Atspi_Relation), 1);
   if (!rel) return EINA_FALSE;

   rel->type = type;
   rel->objects = eina_list_append(rel->objects, rel_obj);
   *set = eina_list_append(*set, rel);

   eo_do(rel_obj, eo_event_callback_add(EO_BASE_EVENT_DEL, _on_rel_obj_del, set));
   return EINA_TRUE;
}

EAPI void elm_atspi_relation_set_relation_remove(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type, const Eo *rel_obj)
{
   Eina_List *l;
   Elm_Atspi_Relation *rel;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             if (eina_list_data_find(rel->objects, rel_obj))
               {
                  eo_do(rel_obj, eo_event_callback_del(EO_BASE_EVENT_DEL, _on_rel_obj_del, set));
                  rel->objects = eina_list_remove(rel->objects, rel_obj);
               }
             if (!rel->objects)
               {
                  *set = eina_list_remove(*set, rel);
                  elm_atspi_relation_free(rel);
               }
             return;
          }
     }
}

EAPI void elm_atspi_relation_set_relation_type_remove(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type)
{
   Eina_List *l;
   Elm_Atspi_Relation *rel;
   Eo *obj;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             EINA_LIST_FOREACH(rel->objects, l, obj)
                eo_do(obj, eo_event_callback_del(EO_BASE_EVENT_DEL, _on_rel_obj_del, set));
             *set = eina_list_remove(*set, rel);
             elm_atspi_relation_free(rel);
             return;
          }
     }
}

EAPI void elm_atspi_relation_set_free(Elm_Atspi_Relation_Set *set)
{
   Elm_Atspi_Relation *rel;
   Eina_List *l;
   Eo *obj;

   EINA_LIST_FREE(*set, rel)
     {
        EINA_LIST_FOREACH(rel->objects, l, obj)
           eo_do(obj, eo_event_callback_del(EO_BASE_EVENT_DEL, _on_rel_obj_del, set));
        elm_atspi_relation_free(rel);
     }
}

EAPI Elm_Atspi_Relation_Set elm_atspi_relation_set_clone(const Elm_Atspi_Relation_Set *set)
{
   Elm_Atspi_Relation_Set ret = NULL;
   Eina_List *l;
   Elm_Atspi_Relation *rel;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        Elm_Atspi_Relation *cpy = elm_atspi_relation_clone(rel);
        ret = eina_list_append(ret, cpy);
     }

   return ret;
}
/////

#include "elm_interface_atspi_accessible.eo.c"
