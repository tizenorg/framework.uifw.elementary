/*
 *
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef ELEMENTARY_H
#define ELEMENTARY_H

/**
 * @file Elementary.h
 * @brief Elementary's API
 *
 * Elementary API.
 */

#define ELM_UNIX
#undef ELM_WIN32
#undef ELM_WINCE
#define ELM_EDBUS
#define ELM_EFREET
#define ELM_ETHUMB
#define ELM_IMF
#define ELM_ALLOCA_H
#define ELM_LIBINTL_H

/* Standard headers for standard system calls etc. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <dlfcn.h>
#include <math.h>
#include <fnmatch.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>

#ifdef ELM_UNIX
# include <locale.h>
# ifdef ELM_LIBINTL_H
#  include <libintl.h>
# endif
# include <signal.h>
# include <grp.h>
# include <glob.h>
#endif

#ifdef ELM_ALLOCA_H
# include <alloca.h>
#endif

#if defined (ELM_WIN32) || defined (ELM_WINCE)
# include <malloc.h>
# ifndef alloca
#  define alloca _alloca
# endif
#endif


/* EFL headers */
#include <Eina.h>
#include <Eet.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>

#ifdef ELM_EDBUS
# include <E_DBus.h>
# include <E_Hal.h>
#endif

#ifdef ELM_EFREET
# include <Efreet.h>
# include <Efreet_Mime.h>
# include <Efreet_Trash.h>
#endif

#ifdef ELM_ETHUMB
# include <Ethumb_Client.h>
#endif

#ifdef ELM_IMF
# include <Ecore_IMF.h>
#endif


#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef ELEMENTARY_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */


/* allow usage from c++ */
#ifdef __cplusplus
extern "C" {
#endif

/* handy macros */
#define ELM_RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && (((x) + (w)) > (xx)) && (((y) + (h)) > (yy)))
#define ELM_PI 3.14159265358979323846

/**************************************************************************/
   EAPI extern int ELM_ECORE_EVENT_ETHUMB_CONNECT;

   /* Objects */
   typedef enum _Elm_Win_Type
     {
	ELM_WIN_BASIC,
	ELM_WIN_DIALOG_BASIC,
	ELM_WIN_DESKTOP,
	ELM_WIN_DOCK,
	ELM_WIN_TOOLBAR,
	ELM_WIN_MENU,
	ELM_WIN_UTILITY,
	ELM_WIN_SPLASH
     } Elm_Win_Type;

   typedef enum _Elm_Win_Keyboard_Mode
     {
	ELM_WIN_KEYBOARD_UNKNOWN,
	  ELM_WIN_KEYBOARD_OFF,
	  ELM_WIN_KEYBOARD_ON,
	  ELM_WIN_KEYBOARD_ALPHA,
	  ELM_WIN_KEYBOARD_NUMERIC,
	  ELM_WIN_KEYBOARD_PIN,
	  ELM_WIN_KEYBOARD_PHONE_NUMBER,
	  ELM_WIN_KEYBOARD_HEX,
	  ELM_WIN_KEYBOARD_TERMINAL,
	  ELM_WIN_KEYBOARD_PASSWORD,
	  ELM_WIN_KEYBOARD_IP,
	  ELM_WIN_KEYBOARD_HOST,
	  ELM_WIN_KEYBOARD_FILE,
	  ELM_WIN_KEYBOARD_URL,
	  ELM_WIN_KEYBOARD_KEYPAD,
	  ELM_WIN_KEYBOARD_J2ME
     } Elm_Win_Keyboard_Mode;

  /**
   * Emitted when policy value changed.
   */
  EAPI extern int ELM_EVENT_POLICY_CHANGED;
  typedef struct _Elm_Event_Policy_Changed
    {
       unsigned int policy;
       int new_value;
       int old_value;
    } Elm_Event_Policy_Changed;

  /**
   * Policy identifiers.
   *
   * @see elm_policy_set()
   */
  typedef enum _Elm_Policy
    {
      ELM_POLICY_QUIT, /**< when should quit application automatically.
			* @see Elm_Policy_Quit.
			*/
      ELM_POLICY_LAST
    } Elm_Policy;

  typedef enum _Elm_Policy_Quit
    {
      ELM_POLICY_QUIT_NONE = 0, /**< never quit application automatically */
      ELM_POLICY_QUIT_LAST_WINDOW_CLOSED /**< quit when last window is closed */
    } Elm_Policy_Quit;

  typedef enum _Elm_Thumb_Animation_Setting
    {
       ELM_THUMB_ANIMATION_START = 0, /* Play animation once */
       ELM_THUMB_ANIMATION_LOOP,      /* Keep playing animation until stop is requested */
       ELM_THUMB_ANIMATION_STOP,
       ELM_THUMB_ANIMATION_LAST
    } Elm_Thumb_Animation_Setting;
   
   typedef struct _Elm_Theme Elm_Theme;
   
   typedef enum _Elm_Clock_Digedit
     {
        ELM_CLOCK_NONE         = 0,
        ELM_CLOCK_HOUR_DECIMAL = 1 << 0,
        ELM_CLOCK_HOUR_UNIT    = 1 << 1,
        ELM_CLOCK_MIN_DECIMAL  = 1 << 2,
        ELM_CLOCK_MIN_UNIT     = 1 << 3,
        ELM_CLOCK_SEC_DECIMAL  = 1 << 4,
        ELM_CLOCK_SEC_UNIT     = 1 << 5,
        ELM_CLOCK_ALL	       = (1 << 6) - 1
     } Elm_Clock_Digedit;
   
#ifndef ELM_LIB_QUICKLAUNCH
#define ELM_MAIN() int main(int argc, char **argv) {elm_init(argc, argv); return elm_main(argc, argv);}
#else
#define ELM_MAIN() int main(int argc, char **argv) {return elm_quicklaunch_fallback(argc, argv);}
#endif

/**************************************************************************/
   /* General calls */
   EAPI void         elm_init(int argc, char **argv);
   EAPI void         elm_shutdown(void);
   EAPI void         elm_run(void);
   EAPI void         elm_exit(void);

   EAPI void         elm_quicklaunch_init(int argc, char **argv);
   EAPI void         elm_quicklaunch_sub_init(int argc, char **argv);
   EAPI void         elm_quicklaunch_sub_shutdown(void);
   EAPI void         elm_quicklaunch_shutdown(void);
   EAPI void         elm_quicklaunch_seed(void);
   EAPI Eina_Bool    elm_quicklaunch_prepare(int argc, char **argv);
   EAPI Eina_Bool    elm_quicklaunch_fork(int argc, char **argv, char *cwd, void (postfork_func) (void *data), void *postfork_data);
   EAPI void         elm_quicklaunch_cleanup(void);
   EAPI int          elm_quicklaunch_fallback(int argc, char **argv);
   EAPI char        *elm_quicklaunch_exe_path_get(const char *exe);

   EAPI void         elm_need_efreet(void);
   EAPI void         elm_need_e_dbus(void);
   EAPI void	     elm_need_ethumb(void);

   EAPI Eina_Bool    elm_policy_set(unsigned int policy, int value);
   EAPI int          elm_policy_get(unsigned int policy);
   
   EAPI void         elm_all_flush(void);
       
   EAPI void         elm_object_scale_set(Evas_Object *obj, double scale);
   EAPI double       elm_object_scale_get(const Evas_Object *obj);
   EAPI void         elm_object_style_set(Evas_Object *obj, const char *style);
   EAPI const char  *elm_object_style_get(const Evas_Object *obj);
   EAPI void         elm_object_disabled_set(Evas_Object *obj, Eina_Bool disabled);
   EAPI Eina_Bool    elm_object_disabled_get(const Evas_Object *obj);

   EAPI Eina_Bool    elm_object_widget_check(const Evas_Object *obj);
   EAPI Evas_Object *elm_object_parent_widget_get(const Evas_Object *obj);
   EAPI const char  *elm_object_widget_type_get(const Evas_Object *obj);

   EAPI double       elm_scale_get(void);
   EAPI void         elm_scale_set(double scale);
   EAPI void         elm_scale_all_set(double scale);
   EAPI Evas_Coord   elm_finger_size_get(void);
   EAPI void         elm_finger_size_set(Evas_Coord size);
   EAPI void         elm_finger_size_all_set(Evas_Coord size);
       
   EAPI void         elm_object_focus(Evas_Object *obj);
   EAPI void         elm_object_unfocus(Evas_Object *obj);
   EAPI void         elm_object_focus_allow_set(Evas_Object *obj, Eina_Bool enable);
   EAPI Eina_Bool    elm_object_focus_allow_get(const Evas_Object *obj);

   EAPI void         elm_object_scroll_hold_push(Evas_Object *obj);
   EAPI void         elm_object_scroll_hold_pop(Evas_Object *obj);
   EAPI void         elm_object_scroll_freeze_push(Evas_Object *obj);
   EAPI void         elm_object_scroll_freeze_pop(Evas_Object *obj);

   EAPI void         elm_coords_finger_size_adjust(int times_w, Evas_Coord *w, int times_h, Evas_Coord *h);

   EAPI Elm_Theme   *elm_theme_new(void);
  EAPI void         elm_theme_free(Elm_Theme *th);
   EAPI void         elm_theme_overlay_add(Elm_Theme *th, const char *item);
   EAPI void         elm_theme_overlay_del(Elm_Theme *th, const char *item);
   EAPI void         elm_theme_extension_add(Elm_Theme *th, const char *item);
   EAPI void         elm_theme_extension_del(Elm_Theme *th, const char *item);
   EAPI void         elm_theme_set(Elm_Theme *th, const char *theme);
   EAPI const char  *elm_theme_get(Elm_Theme *th);
   EAPI void         elm_theme_flush(Elm_Theme *th);
   EAPI void         elm_theme_full_flush(void);
   
   EAPI void         elm_theme_all_set(const char *theme);

   EAPI void         elm_object_theme_set(Evas_Object *obj, Elm_Theme *th);
   EAPI Elm_Theme   *elm_object_theme_get(Evas_Object *obj);
   
   EAPI Evas_Object *elm_win_add(Evas_Object *parent, const char *name, Elm_Win_Type type);
   EAPI void         elm_win_resize_object_add(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_win_resize_object_del(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_win_title_set(Evas_Object *obj, const char *title);
   EAPI void         elm_win_autodel_set(Evas_Object *obj, Eina_Bool autodel);
   EAPI void         elm_win_activate(Evas_Object *obj);
   EAPI void         elm_win_lower(Evas_Object *obj);
   EAPI void         elm_win_raise(Evas_Object *obj);
   EAPI void         elm_win_borderless_set(Evas_Object *obj, Eina_Bool borderless);
   EAPI Eina_Bool    elm_win_borderless_get(const Evas_Object *obj);
   EAPI void         elm_win_shaped_set(Evas_Object *obj, Eina_Bool shaped);
   EAPI Eina_Bool    elm_win_shaped_get(const Evas_Object *obj);
   EAPI void         elm_win_alpha_set(Evas_Object *obj, Eina_Bool alpha);
   EAPI Eina_Bool    elm_win_transparent_get(const Evas_Object *obj);
   EAPI void         elm_win_transparent_set(Evas_Object *obj, Eina_Bool transparent);
   EAPI Eina_Bool    elm_win_alpha_get(const Evas_Object *obj);
   EAPI void         elm_win_override_set(Evas_Object *obj, Eina_Bool override);
   EAPI Eina_Bool    elm_win_override_get(const Evas_Object *obj);
   EAPI void         elm_win_fullscreen_set(Evas_Object *obj, Eina_Bool fullscreen);
   EAPI Eina_Bool    elm_win_fullscreen_get(const Evas_Object *obj);
   EAPI void         elm_win_maximized_set(Evas_Object *obj, Eina_Bool maximized);
   EAPI Eina_Bool    elm_win_maximized_get(const Evas_Object *obj);
   EAPI void         elm_win_iconified_set(Evas_Object *obj, Eina_Bool iconified);
   EAPI Eina_Bool    elm_win_iconified_get(const Evas_Object *obj);
   EAPI void         elm_win_layer_set(Evas_Object *obj, int layer);
   EAPI int          elm_win_layer_get(const Evas_Object *obj);
   EAPI void         elm_win_rotation_set(Evas_Object *obj, int rotation);
   EAPI void         elm_win_rotation_with_resize_set(Evas_Object *obj, int rotation);
   EAPI int          elm_win_rotation_get(const Evas_Object *obj);
   EAPI void         elm_win_sticky_set(Evas_Object *obj, Eina_Bool sticky);
   EAPI Eina_Bool    elm_win_sticky_get(const Evas_Object *obj);
   EAPI void         elm_win_conformant_set(Evas_Object *obj, Eina_Bool conformant);
   EAPI Eina_Bool    elm_win_conformant_get(const Evas_Object *obj);
   EAPI void         elm_win_quickpanel_set(Evas_Object *obj, Eina_Bool quickpanel);
   EAPI Eina_Bool    elm_win_quickpanel_get(const Evas_Object *obj);
   EAPI void         elm_win_quickpanel_priority_major_set(Evas_Object *obj, int priority);
   EAPI int          elm_win_quickpanel_priority_major_get(const Evas_Object *obj);
   EAPI void         elm_win_quickpanel_priority_minor_set(Evas_Object *obj, int priority);
   EAPI int          elm_win_quickpanel_priority_minor_get(const Evas_Object *obj);
   EAPI void         elm_win_quickpanel_zone_set(Evas_Object *obj, int zone);
   EAPI void         elm_win_indicator_state_set(Evas_Object *obj, int show_state);
   EAPI int          elm_win_indicator_state_get(Evas_Object *obj);

   /*...
    * ecore_x_icccm_hints_set -> accepts_focus (add to ecore_evas)
    * ecore_x_icccm_hints_set -> window_group (add to ecore_evas)
    * ecore_x_icccm_size_pos_hints_set -> request_pos (add to ecore_evas)
    * ecore_x_icccm_client_leader_set -> l (add to ecore_evas)
    * ecore_x_icccm_window_role_set -> role (add to ecore_evas)
    * ecore_x_icccm_transient_for_set -> forwin (add to ecore_evas)
    * ecore_x_netwm_window_type_set -> type (add to ecore_evas)
    *
    * (add to ecore_x) set netwm argb icon! (add to ecore_evas)
    * (blank mouse, private mouse obj, defaultmouse)
    *
    */
   EAPI void         elm_win_keyboard_mode_set(Evas_Object *obj, Elm_Win_Keyboard_Mode mode);
   EAPI void         elm_win_keyboard_win_set(Evas_Object *obj, Eina_Bool is_keyboard);
   
   EAPI void         elm_win_screen_position_get(const Evas_Object *obj, int *x, int *y);
       
   EAPI Evas_Object *elm_win_inwin_add(Evas_Object *obj);
   EAPI void         elm_win_inwin_activate(Evas_Object *obj);
   EAPI void         elm_win_inwin_content_set(Evas_Object *obj, Evas_Object *content);
   /* available styles:
    * default
    * minimal
    * minimal_vertical
    */

   /* X specific calls - won't work on non-x engines (return 0) */
   EAPI Ecore_X_Window elm_win_xwindow_get(const Evas_Object *obj);
   /* smart callbacks called:
    * "delete,request" - the user requested to delete the window
    * "focus,in" - window got focus
    * "focus,out" - window lost focus
    * "moved" - window that holds the canvas was moved
    */

   EAPI Evas_Object *elm_bg_add(Evas_Object *parent);
   EAPI void         elm_bg_file_set(Evas_Object *obj, const char *file, const char *group);
   /* smart callbacks called:
    */

   EAPI Evas_Object *elm_icon_add(Evas_Object *parent);
   EAPI Eina_Bool    elm_icon_file_set(Evas_Object *obj, const char *file, const char *group);
   EAPI Eina_Bool    elm_icon_standard_set(Evas_Object *obj, const char *name);
   EAPI void         elm_icon_smooth_set(Evas_Object *obj, Eina_Bool smooth);
   EAPI void         elm_icon_no_scale_set(Evas_Object *obj, Eina_Bool no_scale);
   EAPI void         elm_icon_scale_set(Evas_Object *obj, Eina_Bool scale_up, Eina_Bool scale_down);
   EAPI void         elm_icon_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside);
   EAPI void         elm_icon_prescale_set(Evas_Object *obj, int size);
   /* smart callbacks called:
    * "clicked" - the user clicked the icon
    */

   typedef enum _Elm_Image_Orient
     {
	ELM_IMAGE_ORIENT_NONE,
	ELM_IMAGE_ROTATE_90_CW,
	ELM_IMAGE_ROTATE_180_CW,
	ELM_IMAGE_ROTATE_90_CCW,
	ELM_IMAGE_FLIP_HORIZONTAL,
	ELM_IMAGE_FLIP_VERTICAL,
	ELM_IMAGE_FLIP_TRANSPOSE,
	ELM_IMAGE_FLIP_TRANSVERSE
     } Elm_Image_Orient;

   EAPI Evas_Object *elm_image_add(Evas_Object *parent);
   EAPI Eina_Bool    elm_image_file_set(Evas_Object *obj, const char *file, const char *group);
   EAPI void         elm_image_smooth_set(Evas_Object *obj, Eina_Bool smooth);
   EAPI void         elm_image_object_size_get(const Evas_Object *obj, int *w, int *h);
   EAPI void         elm_image_no_scale_set(Evas_Object *obj, Eina_Bool no_scale);
   EAPI void         elm_image_scale_set(Evas_Object *obj, Eina_Bool scale_up, Eina_Bool scale_down);
   EAPI void         elm_image_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside);
   EAPI void         elm_image_prescale_set(Evas_Object *obj, int size);
   EAPI void         elm_image_orient_set(Evas_Object *obj, Elm_Image_Orient orient);
   /* smart callbacks called:
    * "clicked" - the user clicked the image
    */

   EAPI Evas_Object *elm_box_add(Evas_Object *parent);
   EAPI void         elm_box_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);
   EAPI void         elm_box_homogenous_set(Evas_Object *obj, Eina_Bool homogenous);
   EAPI void         elm_box_pack_start(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_box_pack_end(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_box_pack_before(Evas_Object *obj, Evas_Object *subobj, Evas_Object *before);
   EAPI void         elm_box_pack_after(Evas_Object *obj, Evas_Object *subobj, Evas_Object *after);
   EAPI void         elm_box_clear(Evas_Object *obj);
   EAPI void         elm_box_unpack(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_box_unpack_all(Evas_Object *obj);
   /* smart callbacks called:
    */
	typedef enum {
	   UIControlStateDefault,
	   UIControlStateHighlighted,
	   UIControlStateDisabled,
	   UIControlStateFocused,
	   UIControlStateReserved
	}UIControlState;

   EAPI Evas_Object *elm_button_add(Evas_Object *parent);
   EAPI void         elm_button_label_set(Evas_Object *obj, const char *label);
   EAPI void         elm_button_label_set_for_state(Evas_Object *obj, const char *label, UIControlState state);
   EAPI const char  *elm_button_label_get(const Evas_Object *obj);
   EAPI const char  *elm_button_label_get_for_state(const Evas_Object *obj, UIControlState state);
   EAPI void         elm_button_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_button_icon_get(const Evas_Object *obj);
   EAPI void         elm_button_autorepeat_set(Evas_Object *obj, Eina_Bool on);
   EAPI void         elm_button_autorepeat_initial_timeout_set(Evas_Object *obj, double t);
   EAPI void         elm_button_autorepeat_gap_timeout_set(Evas_Object *obj, double t);

   EAPI Evas_Object *elm_fileselector_button_add(Evas_Object *parent);
   EAPI void         elm_fileselector_button_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_fileselector_button_label_get(const Evas_Object *obj);
   EAPI void         elm_fileselector_button_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_fileselector_button_icon_get(const Evas_Object *obj);
   EAPI void         elm_fileselector_button_window_title_set(Evas_Object *obj, const char *title);
   EAPI const char  *elm_fileselector_button_window_title_get(Evas_Object *obj);
   EAPI void         elm_fileselector_button_window_size_set(Evas_Object *obj, Evas_Coord width, Evas_Coord height);
   EAPI void         elm_fileselector_button_window_size_get(Evas_Object *obj, Evas_Coord *width, Evas_Coord *height);
   EAPI void         elm_fileselector_button_path_set(Evas_Object *obj, const char *path);
   EAPI const char  *elm_fileselector_button_path_get(Evas_Object *obj);
   EAPI void         elm_fileselector_button_expandable_set(Evas_Object *obj, Eina_Bool value);
   EAPI Eina_Bool    elm_fileselector_button_expandable_get(Evas_Object *obj);
   EAPI void         elm_fileselector_button_folder_only_set(Evas_Object *obj, Eina_Bool value);
   EAPI Eina_Bool    elm_fileselector_button_folder_only_get(Evas_Object *obj);
   EAPI void         elm_fileselector_button_is_save_set(Evas_Object *obj, Eina_Bool value);
   EAPI Eina_Bool    elm_fileselector_button_is_save_get(Evas_Object *obj);
   EAPI void         elm_fileselector_button_inwin_mode_set(Evas_Object *obj, Eina_Bool value);
   EAPI Eina_Bool    elm_fileselector_button_inwin_mode_get(Evas_Object *obj);
   EAPI void         elm_fileselector_button_selected_set(Evas_Object *obj, const char *path);
   EAPI const char  *elm_fileselector_button_selected_get(const Evas_Object *obj);

   /* available styles:
    * default
    * anchor
    * hoversel_vertical
    * hoversel_vertical_entry
    */
   /* smart callbacks called:
    * "clicked" - the user clicked the button
    * "repeated" - the user pressed the button without releasing it
    * "unpressed" - when the button is unpressed (released)
    */

   typedef enum _Elm_Scroller_Policy
     {
	ELM_SCROLLER_POLICY_AUTO = 0,
	ELM_SCROLLER_POLICY_ON,
	ELM_SCROLLER_POLICY_OFF,
	ELM_SCROLLER_POLICY_LAST
     }
   Elm_Scroller_Policy;

   EAPI Evas_Object *elm_scroller_add(Evas_Object *parent);
   EAPI void         elm_scroller_content_set(Evas_Object *obj, Evas_Object *child);
   EAPI void         elm_scroller_custom_widget_base_theme_set(Evas_Object *obj, const char *widget, const char *base);
   EAPI void         elm_scroller_content_min_limit(Evas_Object *obj, Eina_Bool w, Eina_Bool h);
   EAPI void         elm_scroller_region_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
   EAPI void         elm_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);
   EAPI void         elm_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);
   EAPI void         elm_scroller_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
   EAPI void         elm_scroller_child_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
   EAPI void         elm_scroller_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
   EAPI void         elm_scroller_page_relative_set(Evas_Object *obj, double h_pagerel, double v_pagerel);
   EAPI void         elm_scroller_page_size_set(Evas_Object *obj, Evas_Coord h_pagesize, Evas_Coord v_pagesize);
   EAPI void         elm_scroller_region_bring_in(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
   EAPI void         elm_scroller_page_move_set(Evas_Object *obj, Eina_Bool set);
   /* smart callbacks called:
    * "edge,left"
    * "edge,right"
    * "edge,top"
    * "edge,bottom"
    * "scroll"
    * "scroll,anim,start"
    * "scroll,anim,stop"
    * "scroll,drag,start"
    * "scroll,drag,stop"
    */

   EAPI Evas_Object *elm_label_add(Evas_Object *parent);
   EAPI void         elm_label_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_label_label_get(const Evas_Object *obj);
   EAPI void         elm_label_line_wrap_set(Evas_Object *obj, Eina_Bool wrap);
   EAPI Eina_Bool    elm_label_line_wrap_get(const Evas_Object *obj);
   EAPI void         elm_label_wrap_width_set(Evas_Object *obj, Evas_Coord w);
   EAPI Evas_Coord   elm_label_wrap_width_get(const Evas_Object *obj);
   EAPI void         elm_label_fontsize_set(Evas_Object *obj, const int fontsize);
   EAPI void         elm_label_text_color_set(Evas_Object *obj, unsigned int r, unsigned int g, unsigned int b, unsigned int a);
   EAPI void         elm_label_text_align_set(Evas_Object *obj, char *alignmode);
   EAPI void         elm_label_background_color_set(Evas_Object *obj, unsigned int r, unsigned int g, unsigned int b, unsigned int a);
   EAPI void         elm_label_ellipsis_set(Evas_Object *obj, Eina_Bool ellipsis);
   /* available styles:
    * default
    * marker
    */
   /* smart callbacks called:
    */

   EAPI Evas_Object *elm_toggle_add(Evas_Object *parent);
   EAPI void         elm_toggle_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_toggle_label_get(const Evas_Object *obj);
   EAPI void         elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_toggle_icon_get(const Evas_Object *obj);
   EAPI void         elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel);
   EAPI void         elm_toggle_states_labels_get(const Evas_Object *obj, const char **onlabel, const char **offlabel);
   EAPI void         elm_toggle_state_set(Evas_Object *obj, Eina_Bool state);
   EAPI Eina_Bool    elm_toggle_state_get(const Evas_Object *obj);
   EAPI void         elm_toggle_state_pointer_set(Evas_Object *obj, Eina_Bool *statep);
   /* smart callbacks called:
    * "changed" - the user toggled the state
    */

   EAPI Evas_Object *elm_frame_add(Evas_Object *parent);
   EAPI void         elm_frame_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_frame_label_get(const Evas_Object *obj);
   EAPI void         elm_frame_content_set(Evas_Object *obj, Evas_Object *content);
   /* available styles:
    * default
    * pad_small
    * pad_medium
    * pad_large
    * pad_huge
    * outdent_top
    * outdent_bottom
    */
   /* smart callbacks called:
    */

   EAPI Evas_Object *elm_table_add(Evas_Object *parent);
   EAPI void         elm_table_homogenous_set(Evas_Object *obj, Eina_Bool homogenous);
   EAPI void	     elm_table_padding_set(Evas_Object *obj, Evas_Coord horizontal, Evas_Coord vertical);
   EAPI void         elm_table_pack(Evas_Object *obj, Evas_Object *subobj, int x, int y, int w, int h);
   EAPI void         elm_table_unpack(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_table_clear(Evas_Object *obj, Eina_Bool clear);
       
   typedef struct _Elm_Grid_Cell_Class Elm_Grid_Cell_Class;
   typedef struct _Elm_Grid_Cell_Class_Func Elm_Grid_Cell_Class_Func;
   typedef struct _Elm_Grid_Cell Elm_Grid_Cell;
   typedef char        *(*GridCellLabelGetFunc) (const void *data, Evas_Object *obj, const char *part);
   typedef Evas_Object *(*GridCellIconGetFunc)  (const void *data, Evas_Object *obj, const char *part);
   typedef Eina_Bool    (*GridCellStateGetFunc) (const void *data, Evas_Object *obj, const char *part);
   typedef void         (*GridCellDelFunc)      (const void *data, Evas_Object *obj);

   struct _Elm_Grid_Cell_Class
     {
	const char *item_style;
	struct _Elm_Grid_Cell_Class_Func
          {
	     GridCellLabelGetFunc label_get;
	     GridCellIconGetFunc icon_get;
	     GridCellStateGetFunc state_get;
	     GridCellDelFunc del;
	  } func;
     };

   EAPI Evas_Object *elm_scrolled_grid_add(Evas_Object *parent);
   EAPI void	     elm_scrolled_grid_cell_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
   EAPI void	     elm_scrolled_grid_cell_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
   EAPI void         elm_scrolled_grid_align_set(Evas_Object *obj, double align_x, double align_y);
   EAPI void         elm_scrolled_grid_align_get(const Evas_Object *obj, double *align_x, double *align_y);
   EAPI void	     elm_scrolled_grid_cell_del(Elm_Grid_Cell *cell);
   EAPI void	     elm_scrolled_grid_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);
   EAPI Eina_Bool    elm_scrolled_grid_always_select_mode_get(const Evas_Object *obj);
   EAPI void	     elm_scrolled_grid_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select);
   EAPI Eina_Bool    elm_scrolled_grid_no_select_mode_get(const Evas_Object *obj);
   EAPI void	     elm_scrolled_grid_multi_select_set(Evas_Object *obj, Eina_Bool multi);
   EAPI Eina_Bool    elm_scrolled_grid_multi_select_get(const Evas_Object *obj);
   EAPI void	     elm_scrolled_grid_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
   EAPI void	     elm_scrolled_grid_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);
   EAPI void	     elm_scrolled_grid_horizontal_set(Evas_Object *obj, Eina_Bool setting);
   EAPI void	     elm_scrolled_grid_clear(Evas_Object *obj);
   EAPI void	    *elm_scrolled_grid_cell_data_get(Elm_Grid_Cell *cell);
   EAPI void	     elm_scrolled_grid_cell_pos_get(const Elm_Grid_Cell *cell, unsigned int *x, unsigned int *y);
   EAPI Eina_Bool    elm_scrolled_grid_cell_selected_get(const Elm_Grid_Cell *cell);
   EAPI const Evas_Object *elm_scrolled_grid_cell_object_get(Elm_Grid_Cell *cell);
   EAPI const Eina_List *elm_scrolled_grid_selected_cells_get(const Evas_Object *obj);
   EAPI Elm_Grid_Cell *elm_scrolled_grid_cell_add(Evas_Object *obj, const Elm_Grid_Cell_Class *gcc,  const void *data, Evas_Smart_Cb func, const void *func_data);
   /* smart callbacks called:
    *
    * selected - User has selected a cell.
    * unselected - User has unselected a cell.
    * clicked - User has double-clicked a cell.
    * realized - An evas object for a cell was built.
    * scroll - the content has been scrolled (moved).
    * "scroll,drag,start" - dragging the contents around has started.
    * "scroll,drat,stop" - dragging the contents around has stopped.
    * drag - grid is being dragged.
    * "drag,start,up" - grid has been dragged (not scrolled) up.
    * "drag,start,down" - grid has been dragged (not scrolled) down.
    * "drag,start,left" - grid has been dragged (not scrolled) left.
    * "drag,start,rigth" - grid has been dragged (nto scrolled) right.
    * "drag,stop" - grid is not being dragged.
    */

   EAPI Evas_Object *elm_clock_add(Evas_Object *parent);
   EAPI void         elm_clock_time_set(Evas_Object *obj, int hrs, int min, int sec);
   EAPI void         elm_clock_time_get(const Evas_Object *obj, int *hrs, int *min, int *sec);
   EAPI void         elm_clock_edit_set(Evas_Object *obj, Eina_Bool edit);
   EAPI Eina_Bool    elm_clock_edit_get(const Evas_Object *obj);
   EAPI void	     elm_clock_digit_edit_set(Evas_Object *obj, unsigned int digedit);
   EAPI unsigned int elm_clock_digit_edit_get(const Evas_Object *obj);
   EAPI void         elm_clock_show_am_pm_set(Evas_Object *obj, Eina_Bool am_pm);
   EAPI Eina_Bool    elm_clock_show_am_pm_get(const Evas_Object *obj);
   EAPI void         elm_clock_show_seconds_set(Evas_Object *obj, Eina_Bool seconds);
   EAPI Eina_Bool    elm_clock_show_seconds_get(const Evas_Object *obj);
   /* smart callbacks called:
    * "changed" - the user changed the time
    */

   EAPI Evas_Object *elm_layout_add(Evas_Object *parent);
   EAPI Eina_Bool    elm_layout_file_set(Evas_Object *obj, const char *file, const char *group);
   EAPI Eina_Bool    elm_layout_theme_set(Evas_Object *obj, const char *clas, const char *group, const char *style);
   EAPI void         elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);
   EAPI Evas_Object *elm_layout_edje_get(const Evas_Object *obj);
   EAPI void         elm_layout_sizing_eval(Evas_Object *obj);
   /* smart callbacks called:
    */

   typedef enum _Elm_Notify_Orient
     {
	 ELM_NOTIFY_ORIENT_TOP,
	 ELM_NOTIFY_ORIENT_CENTER,
	 ELM_NOTIFY_ORIENT_BOTTOM,
	 ELM_NOTIFY_ORIENT_LEFT,
	 ELM_NOTIFY_ORIENT_RIGHT,
	 ELM_NOTIFY_ORIENT_TOP_LEFT,
	 ELM_NOTIFY_ORIENT_TOP_RIGHT,
	 ELM_NOTIFY_ORIENT_BOTTOM_LEFT,
	 ELM_NOTIFY_ORIENT_BOTTOM_RIGHT
     } Elm_Notify_Orient;
   EAPI Evas_Object *elm_notify_add(Evas_Object *parent);
   EAPI void         elm_notify_content_set(Evas_Object *obj, Evas_Object *content);
   EAPI void	     elm_notify_parent_set(Evas_Object *obj, Evas_Object *parent);
   EAPI void	     elm_notify_orient_set(Evas_Object *obj, Elm_Notify_Orient orient);
   EAPI void         elm_notify_timeout_set(Evas_Object *obj, int timeout);
   EAPI void	     elm_notify_timer_init(Evas_Object *obj);
   EAPI void	     elm_notify_repeat_events_set(Evas_Object *obj, Eina_Bool repeat);
   /* smart callbacks called:
    */
   
   typedef enum _Elm_Hover_Axis
     {
	ELM_HOVER_AXIS_NONE,
	  ELM_HOVER_AXIS_HORIZONTAL,
	  ELM_HOVER_AXIS_VERTICAL,
	  ELM_HOVER_AXIS_BOTH
     } Elm_Hover_Axis;
   EAPI Evas_Object *elm_hover_add(Evas_Object *parent);
   EAPI void         elm_hover_target_set(Evas_Object *obj, Evas_Object *target);
   EAPI Evas_Object *elm_hover_target_get(Evas_Object *obj);
   EAPI void         elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent);
   EAPI Evas_Object *elm_hover_parent_get(Evas_Object *obj);
   EAPI void         elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);
   EAPI const char  *elm_hover_best_content_location_get(const Evas_Object *obj, Elm_Hover_Axis pref_axis);
   /* available styles:
    * default
    * popout
    * menu
    * hoversel_vertical
    */
   /* smart callbacks called:
    * "clicked" - the user clicked the empty space in the hover to dismiss
    */

   typedef struct _Elm_Entry_Anchor_Info Elm_Entry_Anchor_Info;
   struct _Elm_Entry_Anchor_Info
     {
	const char *name;
	int button;
	Evas_Coord x, y, w, h;
     };
   typedef enum _Elm_Icon_Type
     {
	ELM_ICON_NONE,
	  ELM_ICON_FILE,
	  ELM_ICON_STANDARD
     } Elm_Icon_Type;
   typedef struct _Elm_Hoversel_Item Elm_Hoversel_Item;
   EAPI Evas_Object *elm_entry_add(Evas_Object *parent);
   EAPI void         elm_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);
   EAPI Eina_Bool    elm_entry_single_line_get(const Evas_Object *obj);
   EAPI void         elm_entry_password_set(Evas_Object *obj, Eina_Bool password);
   EAPI Eina_Bool    elm_entry_password_get(const Evas_Object *obj);
   EAPI void         elm_entry_entry_set(Evas_Object *obj, const char *entry);
   EAPI const char  *elm_entry_entry_get(const Evas_Object *obj);
   EAPI const char  *elm_entry_selection_get(const Evas_Object *obj);
   EAPI void         elm_entry_entry_insert(Evas_Object *obj, const char *entry);
   EAPI void         elm_entry_line_wrap_set(Evas_Object *obj, Eina_Bool wrap);
   EAPI void         elm_entry_line_char_wrap_set(Evas_Object *obj, Eina_Bool wrap);
   EAPI void         elm_entry_editable_set(Evas_Object *obj, Eina_Bool editable);
   EAPI Eina_Bool    elm_entry_editable_get(const Evas_Object *obj);
   EAPI void         elm_entry_select_none(Evas_Object *obj);
   EAPI void         elm_entry_select_all(Evas_Object *obj);
   EAPI Eina_Bool    elm_entry_cursor_next(Evas_Object *obj);
   EAPI Eina_Bool    elm_entry_cursor_prev(Evas_Object *obj);
   EAPI Eina_Bool    elm_entry_cursor_up(Evas_Object *obj);
   EAPI Eina_Bool    elm_entry_cursor_down(Evas_Object *obj);
   EAPI void         elm_entry_cursor_begin_set(Evas_Object *obj);
   EAPI void         elm_entry_cursor_end_set(Evas_Object *obj);
   EAPI void         elm_entry_cursor_line_begin_set(Evas_Object *obj);
   EAPI void         elm_entry_cursor_line_end_set(Evas_Object *obj);
   EAPI void         elm_entry_cursor_selection_begin(Evas_Object *obj);
   EAPI void         elm_entry_cursor_selection_end(Evas_Object *obj);
   EAPI Eina_Bool    elm_entry_cursor_is_format_get(const Evas_Object *obj);
   EAPI Eina_Bool    elm_entry_cursor_is_visible_format_get(const Evas_Object *obj);
   EAPI const char  *elm_entry_cursor_content_get(const Evas_Object *obj);
   EAPI void         elm_entry_selection_cut(Evas_Object *obj);
   EAPI void         elm_entry_selection_copy(Evas_Object *obj);
   EAPI void         elm_entry_selection_paste(Evas_Object *obj);
   EAPI void         elm_entry_context_menu_clear(Evas_Object *obj);
   EAPI void         elm_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);
   EAPI void         elm_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled);
   EAPI Eina_Bool    elm_entry_context_menu_disabled_get(const Evas_Object *obj);
   EAPI void         elm_entry_item_provider_append(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data);
   EAPI void         elm_entry_item_provider_prepend(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data);
   EAPI void         elm_entry_item_provider_remove(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data);
   EAPI char        *elm_entry_markup_to_utf8(const char *s);
   EAPI char        *elm_entry_utf8_to_markup(const char *s);

   EAPI void	     elm_entry_autocapitalization_set(Evas_Object *obj, Eina_Bool on);
   EAPI void	     elm_entry_autoenable_returnkey_set(Evas_Object *obj, Eina_Bool on);
#ifdef ELM_IMF
   EAPI const Ecore_IMF_Context *elm_entry_imf_context_get(Evas_Object *obj);
#else
   EAPI void        *elm_entry_imf_context_get(Evas_Object *obj);
#endif

   /* smart callbacks called:
    * "changed" - the text content changed
    * "selection,start" - the user started selecting text
    * "selection,changed" - the user modified the selection size/location
    * "selection,cleared" - the user cleared the selection
    * "selection,paste" - the user requested a paste of text
    * "selection,copy" - the user copied the text
    * "selection,cut" - the user cut the text
    * "cursor,changed" - the cursor changed position
    * "anchor,clicked" - achor called was clicked | event_info = Elm_Entry_Anchor_Info
    * "activated" - when the enter key is pressed (useful for single line)
    * "press" - when finger/mouse is pressed down
    * "clicked" - when finger/mouse is pressed and released (without a drag etc.)
    * "clicked,double" - when finger/mouse is double-pressed
    * "longpressed" - the entry has been longpressed
    * "focused" - the entry has received keyboard focus
    * "unfocused" - keyboard focus is gone
    */

   /* composite widgets - these basically put together basic widgets above
    * in convenient packages that do more than basic stuff */
   typedef enum _Elm_Text_Format
     {
	ELM_TEXT_FORMAT_PLAIN_UTF8,
	ELM_TEXT_FORMAT_MARKUP_UTF8
     } Elm_Text_Format;
   EAPI Evas_Object *elm_notepad_add(Evas_Object *parent);
   EAPI void         elm_notepad_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format);
   EAPI void         elm_notepad_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
   /* smart callbacks called:
    */

   typedef struct _Elm_Entry_Anchorview_Info Elm_Entry_Anchorview_Info;
   struct _Elm_Entry_Anchorview_Info
     {
	const char *name;
	int button;
	Evas_Object *hover;
	struct
          {
             Evas_Coord x, y, w, h;
	  } anchor, hover_parent;
	Eina_Bool hover_left : 1;
	Eina_Bool hover_right : 1;
	Eina_Bool hover_top : 1;
	Eina_Bool hover_bottom : 1;
     };
   EAPI Evas_Object *elm_anchorview_add(Evas_Object *parent);
   EAPI void         elm_anchorview_text_set(Evas_Object *obj, const char *text);
   EAPI const char  *elm_anchorview_text_get(const Evas_Object *obj);

   EAPI void         elm_anchorview_hover_parent_set(Evas_Object *obj, Evas_Object *parent);
   EAPI void         elm_anchorview_hover_style_set(Evas_Object *obj, const char *style);
   EAPI void         elm_anchorview_hover_end(Evas_Object *obj);
   EAPI void         elm_anchorview_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
   EAPI void         elm_anchorview_item_provider_append(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorview, const char *item), void *data);
   EAPI void         elm_anchorview_item_provider_prepend(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorview, const char *item), void *data);
   EAPI void         elm_anchorview_item_provider_remove(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorview, const char *item), void *data);
   /* smart callbacks called:
    * "anchor,clicked" - achor called was clicked | event_info = Elm_Entry_Anchorview_Info
    */

   typedef struct _Elm_Entry_Anchorblock_Info Elm_Entry_Anchorblock_Info;
   struct _Elm_Entry_Anchorblock_Info
     {
	const char *name;
	int button;
	Evas_Object *hover;
	struct
          {
	     Evas_Coord x, y, w, h;
	  } anchor, hover_parent;
	Eina_Bool hover_left : 1;
	Eina_Bool hover_right : 1;
	Eina_Bool hover_top : 1;
	Eina_Bool hover_bottom : 1;
     };
   EAPI Evas_Object *elm_anchorblock_add(Evas_Object *parent);
   EAPI void         elm_anchorblock_text_set(Evas_Object *obj, const char *text);
   EAPI const char  *elm_anchorblock_text_get(const Evas_Object *obj);
   EAPI void         elm_anchorblock_hover_parent_set(Evas_Object *obj, Evas_Object *parent);
   EAPI void         elm_anchorblock_hover_style_set(Evas_Object *obj, const char *style);
   EAPI void         elm_anchorblock_hover_end(Evas_Object *obj);
   EAPI void         elm_anchorblock_item_provider_append(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorblock, const char *item), void *data);
   EAPI void         elm_anchorblock_item_provider_prepend(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorblock, const char *item), void *data);
   EAPI void         elm_anchorblock_item_provider_remove(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *anchorblock, const char *item), void *data);
   /* smart callbacks called:
    * "anchor,clicked" - achor called was clicked | event_info = Elm_Entry_Anchorblock_Info
    */

   EAPI Evas_Object *elm_bubble_add(Evas_Object *parent);
   EAPI void         elm_bubble_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_bubble_label_get(const Evas_Object *obj);
   EAPI void         elm_bubble_info_set(Evas_Object *obj, const char *info);
   EAPI const char  *elm_bubble_info_get(const Evas_Object *obj);

   EAPI void         elm_bubble_content_set(Evas_Object *obj, Evas_Object *content);
   EAPI void         elm_bubble_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_bubble_icon_get(const Evas_Object *obj);
   EAPI void         elm_bubble_corner_set(Evas_Object *obj, const char *corner);
   /* smart callbacks called:
    */

   EAPI Evas_Object *elm_photo_add(Evas_Object *parent);
   EAPI Eina_Bool    elm_photo_file_set(Evas_Object *obj, const char *file);
   EAPI void         elm_photo_size_set(Evas_Object *obj, int size);
   /* smart callbacks called:
    * "clicked" - the user clicked the icon
    */

   EAPI Evas_Object *elm_thumb_add(Evas_Object *parent);
   EAPI void	     elm_thumb_file_set(Evas_Object *obj, const char *file, const char *key);
   EAPI void	     elm_thumb_file_get(const Evas_Object *obj, const char **file, const char **key);
   EAPI void	     elm_thumb_keep_aspect_set(Evas_Object *obj, Eina_Bool setting);
   EAPI Eina_Bool    elm_thumb_keep_aspect_get(const Evas_Object *obj);
   EAPI void	     elm_thumb_align_set(Evas_Object *obj, float x_align, float y_align);
   EAPI void	     elm_thumb_align_get(const Evas_Object *obj, float *x, float *y);
   EAPI void	     elm_thumb_animate_set(Evas_Object *obj, Elm_Thumb_Animation_Setting s);
   EAPI Elm_Thumb_Animation_Setting elm_thumb_animate_get(const Evas_Object *obj);
   EAPI Eina_Bool    elm_thumb_ethumb_client_connected(void);
#ifdef ELM_ETHUMB
   EAPI Ethumb_Client *elm_thumb_ethumb_client_get(void);
#else
   EAPI void	    *elm_thumb_ethumb_client_get(void);
#endif
   /* smart callbacks called:
    * "clicked" - the user clicked the thumb
    * "clicked,double" - the user double clicked the thumb
    * "press" - the user pressed the thumb
    * "generate,start" - the thumbnail generation started
    * "generate,stop" - the thumbnail generation stopped
    * "generate,error" - the thumbnail generation failed
    * "load,error" - the thumbnail image loading failed
    */

   EAPI Evas_Object *elm_hoversel_add(Evas_Object *parent);
   EAPI void         elm_hoversel_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);
   EAPI Eina_Bool    elm_hoversel_horizontal_get(const Evas_Object *obj);
   EAPI void         elm_hoversel_hover_parent_set(Evas_Object *obj, Evas_Object *parent);
   EAPI void         elm_hoversel_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_hoversel_label_get(const Evas_Object *obj);
   EAPI void         elm_hoversel_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_hoversel_icon_get(const Evas_Object *obj);
   EAPI void         elm_hoversel_hover_begin(Evas_Object *obj);
   EAPI void         elm_hoversel_hover_end(Evas_Object *obj);
   EAPI Eina_Bool    elm_hoversel_expanded_get(Evas_Object *obj);
   EAPI void         elm_hoversel_clear(Evas_Object *obj);
   EAPI const Eina_List * elm_hoversel_items_get(const Evas_Object *obj);
   EAPI Elm_Hoversel_Item *elm_hoversel_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);
   EAPI void         elm_hoversel_item_del(Elm_Hoversel_Item *item);
   EAPI void         elm_hoversel_item_del_cb_set(Elm_Hoversel_Item *it, Evas_Smart_Cb func);
   EAPI void        *elm_hoversel_item_data_get(Elm_Hoversel_Item *it);
   EAPI const char  *elm_hoversel_item_label_get(Elm_Hoversel_Item *it);
   EAPI void         elm_hoversel_item_icon_set(Elm_Hoversel_Item *it, const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type);
   EAPI void         elm_hoversel_item_icon_get(Elm_Hoversel_Item *it, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type);
   /* smart callbacks called:
    * "clicked" - the user clicked the hoversel button and popped up the sel
    * "selected" - an item in the hoversel list is selected
    * "dismissed" - the hover is dismissed
    */

   typedef struct _Elm_Toolbar_Item Elm_Toolbar_Item;
   EAPI Evas_Object      *elm_toolbar_add(Evas_Object *parent);
   EAPI void              elm_toolbar_icon_size_set(Evas_Object *obj, int icon_size);
   EAPI int               elm_toolbar_icon_size_get(const Evas_Object *obj);
   EAPI Elm_Toolbar_Item *elm_toolbar_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, Evas_Smart_Cb func, const void *data);
   EAPI Evas_Object      *elm_toolbar_item_icon_get(Elm_Toolbar_Item *item);
   EAPI const char       *elm_toolbar_item_label_get(const Elm_Toolbar_Item *item);
   EAPI Eina_Bool         elm_toolbar_item_selected_get(const Elm_Toolbar_Item *item);
   EAPI void              elm_toolbar_item_label_set(Elm_Toolbar_Item *item, const char *label);
   EAPI void              elm_toolbar_item_del(Elm_Toolbar_Item *item);
   EAPI void              elm_toolbar_item_del_cb_set(Elm_Toolbar_Item *it, Evas_Smart_Cb func);
   EAPI void              elm_toolbar_item_select(Elm_Toolbar_Item *item);
   EAPI Eina_Bool         elm_toolbar_item_disabled_get(Elm_Toolbar_Item *item);
   EAPI void              elm_toolbar_item_disabled_set(Elm_Toolbar_Item *item, Eina_Bool disabled);
   EAPI void              elm_toolbar_item_separator_set(Elm_Toolbar_Item *item, Eina_Bool separator);
   EAPI Eina_Bool         elm_toolbar_item_separator_get(Elm_Toolbar_Item *item);
   EAPI Eina_List        *elm_toolbar_item_get_all(Evas_Object *obj);
   EAPI Elm_Toolbar_Item *elm_toolbar_item_get_first(Evas_Object *obj);
   EAPI Elm_Toolbar_Item *elm_toolbar_item_get_last(Evas_Object *obj);
   EAPI Elm_Toolbar_Item *elm_toolbar_item_get_next(Evas_Object *obj);
   EAPI Elm_Toolbar_Item *elm_toolbar_item_select_next(Evas_Object *obj);
   EAPI Elm_Toolbar_Item *elm_toolbar_item_select_first(Evas_Object *obj);
   EAPI Elm_Toolbar_Item *elm_toolbar_item_select_last(Evas_Object *obj);
   EAPI Elm_Toolbar_Item *elm_toolbar_item_find_by_label(Evas_Object *obj, const char *label);
   EAPI void              elm_toolbar_scrollable_set(Evas_Object *obj, Eina_Bool scrollable);
   EAPI Eina_Bool         elm_toolbar_scrollable_get(Evas_Object *obj);
   EAPI void	          elm_toolbar_homogenous_set(Evas_Object *obj, Eina_Bool homogenous);
   EAPI Eina_Bool         elm_toolbar_homogenous_get(Evas_Object *obj);
   EAPI void	          elm_toolbar_menu_parent_set(Evas_Object *obj, Evas_Object *parent);
   EAPI Evas_Object      *elm_toolbar_menu_parent_get(Evas_Object *obj);
   EAPI void              elm_toolbar_item_unselect_all(Evas_Object *obj);
   EAPI void              elm_toolbar_align_set(Evas_Object *obj, double align);
   EAPI double            elm_toolbar_align_get(const Evas_Object *obj);
   EAPI void	          elm_toolbar_item_menu_set(Elm_Toolbar_Item *item, Eina_Bool menu);
   EAPI Evas_Object      *elm_toolbar_item_menu_get(Elm_Toolbar_Item *item);
   /* smart callbacks called:
    * "clicked" - when the user clicks on a toolbar item and becomes selected
    */

   typedef struct _Menu_Item Elm_Menu_Item;
   EAPI Evas_Object     *elm_menu_add(Evas_Object *parent);
   EAPI void		 elm_menu_parent_set(Evas_Object *obj, Evas_Object *parent);
   EAPI void		 elm_menu_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
   EAPI Evas_Object     *elm_menu_object_get(const Elm_Menu_Item *it);
   EAPI Elm_Menu_Item   *elm_menu_item_add(Evas_Object *obj, Elm_Menu_Item *parent, Evas_Object *icon, const char *label, Evas_Smart_Cb func, const void *data);
   EAPI void		 elm_menu_item_label_set(Elm_Menu_Item *item, const char *label);
   EAPI const char      *elm_menu_item_label_get(Elm_Menu_Item *item);
   EAPI void		 elm_menu_item_icon_set(Elm_Menu_Item *item, Evas_Object *icon);
   EAPI Evas_Object     *elm_menu_item_icon_get(Elm_Menu_Item *item);
   EAPI void		 elm_menu_item_disabled_set(Elm_Menu_Item *item, Eina_Bool disabled);
   EAPI Eina_Bool        elm_menu_item_disabled_get(Elm_Menu_Item *item);
   EAPI Elm_Menu_Item   *elm_menu_item_separator_add(Evas_Object *obj, Elm_Menu_Item *parent);
   EAPI Eina_Bool        elm_menu_item_is_separator(Elm_Menu_Item *item);
   EAPI void		 elm_menu_item_del(Elm_Menu_Item *item);
   EAPI void		 elm_menu_item_del_cb_set(Elm_Menu_Item *it, Evas_Smart_Cb func);
   EAPI void		*elm_menu_item_data_get(const Elm_Menu_Item *it);
   EAPI void		 elm_menu_item_data_set(Elm_Menu_Item *item, const void *data);
   EAPI const Eina_List *elm_menu_item_subitems_get(Elm_Menu_Item *item);
   /* smart callbacks called:
    * "clicked" - the user clicked the empty space in the menu to dismiss. event_info is NULL.
    */

   typedef enum _Elm_List_Mode
     {
	ELM_LIST_COMPRESS = 0,
	ELM_LIST_SCROLL,
	ELM_LIST_LIMIT,
	ELM_LIST_LAST
     } Elm_List_Mode;
   typedef struct _Elm_List_Item Elm_List_Item;
   EAPI Evas_Object     *elm_list_add(Evas_Object *parent);
   EAPI Elm_List_Item   *elm_list_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data);
   EAPI Elm_List_Item   *elm_list_item_prepend(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data);
   EAPI Elm_List_Item   *elm_list_item_insert_before(Evas_Object *obj, Elm_List_Item *before, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data);
   EAPI Elm_List_Item   *elm_list_item_insert_after(Evas_Object *obj, Elm_List_Item *after, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data);
   EAPI Elm_List_Item   *elm_list_item_sorted_insert(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func);
   EAPI void             elm_list_clear(Evas_Object *obj);
   EAPI void             elm_list_go(Evas_Object *obj);
   EAPI void             elm_list_multi_select_set(Evas_Object *obj, Eina_Bool multi);
   EAPI Eina_Bool	 elm_list_multi_select_get(const Evas_Object *obj);
   EAPI void             elm_list_horizontal_mode_set(Evas_Object *obj, Elm_List_Mode mode);
   EAPI Elm_List_Mode	 elm_list_horizontal_mode_get(const Evas_Object *obj);
   EAPI void             elm_list_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);
   EAPI Eina_Bool	 elm_list_always_select_mode_get(const Evas_Object *obj);
   EAPI const Eina_List *elm_list_items_get(const Evas_Object *obj);
   EAPI Elm_List_Item   *elm_list_selected_item_get(const Evas_Object *obj);
   EAPI const Eina_List *elm_list_selected_items_get(const Evas_Object *obj);
   EAPI void             elm_list_item_selected_set(Elm_List_Item *item, Eina_Bool selected);
   EAPI Eina_Bool        elm_list_item_selected_get(Elm_List_Item *item);
   EAPI void             elm_list_item_show(Elm_List_Item *item);
   EAPI void             elm_list_item_del(Elm_List_Item *item);
   EAPI void             elm_list_item_del_cb_set(Elm_List_Item *item, Evas_Smart_Cb func);
   EAPI void            *elm_list_item_data_get(const Elm_List_Item *item);
   EAPI Evas_Object     *elm_list_item_icon_get(const Elm_List_Item *item);
   EAPI void             elm_list_item_icon_set(Elm_List_Item *item, Evas_Object *icon);
   EAPI Evas_Object     *elm_list_item_end_get(const Elm_List_Item *item);
   EAPI void             elm_list_item_end_set(Elm_List_Item *item, Evas_Object *end);
   EAPI Evas_Object     *elm_list_item_base_get(const Elm_List_Item *item);
   EAPI const char      *elm_list_item_label_get(const Elm_List_Item *item);
   EAPI void             elm_list_item_label_set(Elm_List_Item *item, const char *text);
   EAPI Elm_List_Item   *elm_list_item_prev(const Elm_List_Item *it);
   EAPI Elm_List_Item   *elm_list_item_next(const Elm_List_Item *it);
   EAPI void             elm_list_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
   EAPI void             elm_list_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);
   EAPI void             elm_list_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);
   /* smart callbacks called:
    * "clicked" - when the user double-clicked an item
    * "selected" - when the user selected an item
    * "unselected" - when the user selected an item
    * "longpressed" - an item in the hoversel list is long-pressed
    */

   // FIXME: incomplete - carousel. don't use this until this comment is removed
   typedef struct _Elm_Carousel_Item Elm_Carousel_Item;
   EAPI Evas_Object       *elm_carousel_add(Evas_Object *parent);
   EAPI Elm_Carousel_Item *elm_carousel_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, Evas_Smart_Cb func, const void *data);
   EAPI void               elm_carousel_item_del(Elm_Carousel_Item *item);
   EAPI void               elm_carousel_item_select(Elm_Carousel_Item *item);
   /* smart callbacks called:
    * "clicked" - when the user clicks on a carousel item and becomes selected
    */

   EAPI Evas_Object *elm_slider_add(Evas_Object *parent);
   EAPI void         elm_slider_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_slider_label_get(const Evas_Object *obj);
   EAPI void         elm_slider_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_slider_icon_get(const Evas_Object *obj);
   EAPI void         elm_slider_span_size_set(Evas_Object *obj, Evas_Coord size);
   EAPI Evas_Coord   elm_slider_span_size_get(const Evas_Object *obj);
   EAPI void         elm_slider_unit_format_set(Evas_Object *obj, const char *format);
   EAPI const char  *elm_slider_unit_format_get(const Evas_Object *obj);
   EAPI void         elm_slider_indicator_format_set(Evas_Object *obj, const char *indicator);
   EAPI const char  *elm_slider_indicator_format_get(const Evas_Object *obj);
   EAPI void         elm_slider_indicator_format_function_set(Evas_Object *obj, const char *(*func)(double val));
   EAPI void         elm_slider_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);
   EAPI Eina_Bool    elm_slider_horizontal_get(const Evas_Object *obj);
   EAPI void         elm_slider_min_max_set(Evas_Object *obj, double min, double max);
   EAPI void         elm_slider_min_max_get(const Evas_Object *obj, double *min, double *max);
   EAPI void         elm_slider_value_set(Evas_Object *obj, double val);
   EAPI double       elm_slider_value_get(const Evas_Object *obj);
   EAPI void         elm_slider_inverted_set(Evas_Object *obj, Eina_Bool inverted);
   EAPI Eina_Bool    elm_slider_inverted_get(const Evas_Object *obj);

   // added for supporting aqua feature
   EAPI Eina_Bool       elm_slider_right_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *	elm_slider_right_icon_get(Evas_Object *obj);
   EAPI void		elm_slider_indicator_show_set(Evas_Object *obj, Eina_Bool show);
   EAPI void		elm_slider_value_animated_set(Evas_Object *obj, double val);

   typedef enum _Elm_Genlist_Item_Flags
     {
	ELM_GENLIST_ITEM_NONE = 0,
	ELM_GENLIST_ITEM_SUBITEMS = (1 << 0)
     } Elm_Genlist_Item_Flags;
   typedef struct _Elm_Genlist_Item_Class Elm_Genlist_Item_Class;
   typedef struct _Elm_Genlist_Item       Elm_Genlist_Item;
   typedef struct _Elm_Genlist_Item_Class_Func Elm_Genlist_Item_Class_Func;
   typedef char        *(*GenlistItemLabelGetFunc) (const void *data, Evas_Object *obj, const char *part);
   typedef Evas_Object *(*GenlistItemIconGetFunc)  (const void *data, Evas_Object *obj, const char *part);
   typedef Eina_Bool    (*GenlistItemStateGetFunc) (const void *data, Evas_Object *obj, const char *part);
   typedef void         (*GenlistItemDelFunc)      (const void *data, Evas_Object *obj);

   struct _Elm_Genlist_Item_Class
     {
	const char *item_style;
	struct _Elm_Genlist_Item_Class_Func
          {
	     GenlistItemLabelGetFunc label_get;
	     GenlistItemIconGetFunc icon_get;
	     GenlistItemStateGetFunc state_get;
	     GenlistItemDelFunc del;
	  } func;
     };

   EAPI Evas_Object      *elm_genlist_add(Evas_Object *parent);
   EAPI void              elm_genlist_clear(Evas_Object *obj);
   EAPI void              elm_genlist_multi_select_set(Evas_Object *obj, Eina_Bool multi);
   EAPI Eina_Bool         elm_genlist_multi_select_get(const Evas_Object *obj);
   EAPI void              elm_genlist_horizontal_mode_set(Evas_Object *obj, Elm_List_Mode mode);
   EAPI Elm_List_Mode     elm_genlist_horizontal_mode_get(const Evas_Object *obj);
   EAPI void              elm_genlist_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);
   EAPI Eina_Bool         elm_genlist_always_select_mode_get(const Evas_Object *obj);
   EAPI void              elm_genlist_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select);
   EAPI Eina_Bool         elm_genlist_no_select_mode_get(const Evas_Object *obj);
   EAPI void              elm_genlist_compress_mode_set(Evas_Object *obj, Eina_Bool compress);
   EAPI Eina_Bool         elm_genlist_compress_mode_get(const Evas_Object *obj);
   EAPI void              elm_genlist_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
   EAPI void              elm_genlist_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);
   EAPI void              elm_genlist_homogeneous_set(Evas_Object *obj, Eina_Bool homogeneous);
   EAPI Eina_Bool         elm_genlist_homogeneous_get(const Evas_Object *obj);
   EAPI void              elm_genlist_block_count_set(Evas_Object *obj, int n);
   EAPI int               elm_genlist_block_count_get(const Evas_Object *obj);

   /* operations to add items */
   EAPI Elm_Genlist_Item *elm_genlist_item_append(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Genlist_Item *parent, Elm_Genlist_Item_Flags flags, Evas_Smart_Cb func, const void *func_data);
   EAPI Elm_Genlist_Item *elm_genlist_item_prepend(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Genlist_Item *parent, Elm_Genlist_Item_Flags flags, Evas_Smart_Cb func, const void *func_data);
   EAPI Elm_Genlist_Item *elm_genlist_item_insert_before(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Genlist_Item *before, Elm_Genlist_Item_Flags flags, Evas_Smart_Cb func, const void *func_data);
   EAPI Elm_Genlist_Item *elm_genlist_item_insert_after(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Genlist_Item *after, Elm_Genlist_Item_Flags flags, Evas_Smart_Cb func, const void *func_data);
   /* operations to retrieve existing items */
   EAPI Elm_Genlist_Item *elm_genlist_selected_item_get(const Evas_Object *obj);
   EAPI const Eina_List  *elm_genlist_selected_items_get(const Evas_Object *obj);
   EAPI Eina_List        *elm_genlist_realized_items_get(const Evas_Object *obj);
   EAPI Elm_Genlist_Item *elm_genlist_at_xy_item_get(const Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *posret);
   EAPI Elm_Genlist_Item *elm_genlist_first_item_get(const Evas_Object *obj);
   EAPI Elm_Genlist_Item *elm_genlist_last_item_get(const Evas_Object *obj);
   /* available item styles:
    * default
    * default_style - The text part is a textblock
    * double_label
    * icon_top_text_bottom
    */

   /* Genlist Item operation */
   EAPI Elm_Genlist_Item *elm_genlist_item_next_get(const Elm_Genlist_Item *item);
   EAPI Elm_Genlist_Item *elm_genlist_item_prev_get(const Elm_Genlist_Item *item);
   EAPI Evas_Object      *elm_genlist_item_genlist_get(const Elm_Genlist_Item *item);
   EAPI Elm_Genlist_Item *elm_genlist_item_parent_get(const Elm_Genlist_Item *it);
   EAPI void              elm_genlist_item_subitems_clear(Elm_Genlist_Item *item);
   EAPI void              elm_genlist_item_selected_set(Elm_Genlist_Item *item, Eina_Bool selected);
   EAPI Eina_Bool         elm_genlist_item_selected_get(const Elm_Genlist_Item *item);
   EAPI void              elm_genlist_item_expanded_set(Elm_Genlist_Item *item, Eina_Bool expanded);
   EAPI Eina_Bool         elm_genlist_item_expanded_get(const Elm_Genlist_Item *item);
   EAPI void              elm_genlist_item_disabled_set(Elm_Genlist_Item *item, Eina_Bool disabled);
   EAPI Eina_Bool         elm_genlist_item_disabled_get(const Elm_Genlist_Item *item);
   EAPI void              elm_genlist_item_display_only_set(Elm_Genlist_Item *it, Eina_Bool display_only);
   EAPI Eina_Bool         elm_genlist_item_display_only_get(const Elm_Genlist_Item *it);
   EAPI void              elm_genlist_item_show(Elm_Genlist_Item *item);
   EAPI void              elm_genlist_item_bring_in(Elm_Genlist_Item *item);
   EAPI void              elm_genlist_item_top_show(Elm_Genlist_Item *item);
   EAPI void              elm_genlist_item_top_bring_in(Elm_Genlist_Item *item);
   EAPI void		  elm_genlist_item_middle_show(Elm_Genlist_Item *it);
   EAPI void		  elm_genlist_item_middle_bring_in(Elm_Genlist_Item *it);
   EAPI void              elm_genlist_item_del(Elm_Genlist_Item *item);
   EAPI const void       *elm_genlist_item_data_get(const Elm_Genlist_Item *item);
   EAPI void              elm_genlist_item_data_set(Elm_Genlist_Item *it, const void *data);
   EAPI const Evas_Object *elm_genlist_item_object_get(const Elm_Genlist_Item *it);
   EAPI void              elm_genlist_item_update(Elm_Genlist_Item *item);

   EAPI Evas_Object *elm_check_add(Evas_Object *parent);
   EAPI void         elm_check_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_check_label_get(const Evas_Object *obj);
   EAPI void         elm_check_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_check_icon_get(const Evas_Object *obj);
   EAPI void         elm_check_state_set(Evas_Object *obj, Eina_Bool state);
   EAPI Eina_Bool    elm_check_state_get(const Evas_Object *obj);
   EAPI void         elm_check_state_pointer_set(Evas_Object *obj, Eina_Bool *statep);

   EAPI Evas_Object *elm_radio_add(Evas_Object *parent);
   EAPI void         elm_radio_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_radio_label_get(const Evas_Object *obj);
   EAPI void         elm_radio_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_radio_icon_get(const Evas_Object *obj);
   EAPI void         elm_radio_group_add(Evas_Object *obj, Evas_Object *group);
   EAPI void         elm_radio_state_value_set(Evas_Object *obj, int value);
   EAPI void         elm_radio_value_set(Evas_Object *obj, int value);
   EAPI int          elm_radio_value_get(const Evas_Object *obj);
   EAPI void         elm_radio_value_pointer_set(Evas_Object *obj, int *valuep);
   typedef struct _Elm_Segment_Item Elm_Segment_Item;
   EAPI Evas_Object *elm_segment_control_add(Evas_Object *parent);
   EAPI Elm_Segment_Item * elm_segment_control_add_segment(Evas_Object *obj, Evas_Object *icon, const char *label, Eina_Bool animate);
   EAPI void		 elm_segment_control_insert_segment_at(Evas_Object *obj, Evas_Object *icon, const char *label, unsigned int index, Eina_Bool animate);
   EAPI Evas_Object *elm_segment_control_get_segment_at(Evas_Object *obj, unsigned int index);
   EAPI void		 elm_segment_control_delete_segment(Evas_Object *obj, Elm_Segment_Item *item, Eina_Bool animate);
   EAPI void		 elm_segment_control_delete_segment_at(Evas_Object *obj,  unsigned int index, Eina_Bool animate);
   EAPI const char  *elm_segment_control_get_segment_label_at(Evas_Object *obj, unsigned int index);
   EAPI Evas_Object *elm_segment_control_get_segment_icon_at(Evas_Object *obj, unsigned int index);
   EAPI Elm_Segment_Item *elm_segment_control_selected_segment_get(const Evas_Object *obj, int *value);
   EAPI int         elm_segment_control_get_segment_count(Evas_Object *obj);

   EAPI Evas_Object *elm_page_control_add(Evas_Object *parent);
   EAPI void		elm_page_control_page_count_set(Evas_Object *obj, unsigned int page_count);
   EAPI	void 		elm_page_control_page_id_set(Evas_Object *obj, unsigned int page_id);
   EAPI unsigned int elm_page_control_page_id_get(Evas_Object *obj);
   EAPI Evas_Object *elm_pager_add(Evas_Object *parent);
   EAPI void         elm_pager_content_push(Evas_Object *obj, Evas_Object *content);
   EAPI void         elm_pager_content_pop(Evas_Object *obj);
   EAPI void         elm_pager_content_promote(Evas_Object *obj, Evas_Object *content);
   EAPI Evas_Object *elm_pager_content_bottom_get(const Evas_Object *obj);
   EAPI Evas_Object *elm_pager_content_top_get(const Evas_Object *obj);
   EAPI void         elm_pager_animation_set(Evas_Object *obj, Eina_Bool animation);
  /* available item styles:
    * default
    * fade
    * fade_translucide
    */

   typedef struct _Elm_Slideshow_Item_Class Elm_Slideshow_Item_Class;
   typedef struct _Elm_Slideshow_Item_Class_Func Elm_Slideshow_Item_Class_Func;
   typedef struct _Elm_Slideshow_Item       Elm_Slideshow_Item;
   typedef Evas_Object *(*SlideshowItemGetFunc) (void *data, Evas_Object *obj);
   typedef void         (*SlideshowItemDelFunc) (void *data, Evas_Object *obj);

   struct _Elm_Slideshow_Item_Class
     {
	struct _Elm_Slideshow_Item_Class_Func {
	     SlideshowItemGetFunc get;
	     SlideshowItemDelFunc del;
	} func;
     };

   EAPI Evas_Object *	elm_slideshow_add(Evas_Object *parent);
   EAPI Elm_Slideshow_Item* elm_slideshow_item_add(Evas_Object *obj, const Elm_Slideshow_Item_Class *itc, const void *data);
   EAPI void		elm_slideshow_show(Elm_Slideshow_Item *item);
   EAPI void		elm_slideshow_next(Evas_Object *obj);
   EAPI void		elm_slideshow_previous(Evas_Object *obj);
   EAPI const Eina_List *elm_slideshow_transitions_get(const Evas_Object *obj);
   EAPI void	        elm_slideshow_transition_set(Evas_Object *obj, const char *);
   EAPI const char     *elm_slideshow_transition_get(const Evas_Object *obj);
   EAPI void            elm_slideshow_timeout_set(Evas_Object *obj ,int timeout);
   EAPI int		elm_slideshow_timeout_get(const Evas_Object *obj);
   EAPI void		elm_slideshow_loop_set(Evas_Object *obj, Eina_Bool loop);
   EAPI Eina_Bool	elm_slideshow_loop_get(const Evas_Object *obj);
   EAPI void		elm_slideshow_clear(Evas_Object *obj);
   EAPI const Eina_List *elm_slideshow_items_get(const Evas_Object *obj);
   EAPI void		elm_slideshow_item_del(Elm_Slideshow_Item *item);
   EAPI void *		elm_slideshow_item_data_get(Elm_Slideshow_Item *item);
   EAPI Elm_Slideshow_Item* elm_slideshow_item_current_get(const Evas_Object *obj);
   EAPI Evas_Object*	elm_slideshow_item_object_get(Elm_Slideshow_Item* item);


   EAPI Evas_Object *elm_fileselector_add(Evas_Object *parent);
   EAPI void         elm_fileselector_is_save_set(Evas_Object *obj, Eina_Bool is_save);
   EAPI Eina_Bool    elm_fileselector_is_save_get(const Evas_Object *obj);
   EAPI void         elm_fileselector_folder_only_set(Evas_Object *obj, Eina_Bool only);
   EAPI Eina_Bool    elm_fileselector_folder_only_get(const Evas_Object *obj);
   EAPI void         elm_fileselector_buttons_ok_cancel_set(Evas_Object *obj, Eina_Bool buttons);
   EAPI Eina_Bool    elm_fileselector_buttons_ok_cancel_get(const Evas_Object *obj);
   EAPI Eina_Bool    elm_fileselector_expandable_get(const Evas_Object *obj);
   EAPI void         elm_fileselector_expandable_set(Evas_Object *obj, Eina_Bool expand);
   EAPI void         elm_fileselector_path_set(Evas_Object *obj, const char *path);
   EAPI const char  *elm_fileselector_path_get(const Evas_Object *obj);
   EAPI const char  *elm_fileselector_selected_get(const Evas_Object *obj);
   EAPI Eina_Bool    elm_fileselector_selected_set(Evas_Object *obj, const char *path);

   /* smart callbacks called:
    * "selected" - the user click on a file
    * "directory,open" - the list is populate with a new content. event_info is a directory.
    * "done" - the user click on the ok or cancel buttons
    */

   EAPI Evas_Object *elm_progressbar_add(Evas_Object *parent);
   EAPI void         elm_progressbar_pulse_set(Evas_Object *obj, Eina_Bool pulse);
   EAPI Eina_Bool    elm_progressbar_pulse_get(const Evas_Object *obj);
   EAPI void         elm_progressbar_pulse(Evas_Object *obj, Eina_Bool state);
   EAPI void         elm_progressbar_value_set(Evas_Object *obj, double val);
   EAPI double       elm_progressbar_value_get(const Evas_Object *obj);
   EAPI void         elm_progressbar_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_progressbar_label_get(const Evas_Object *obj);
   EAPI void         elm_progressbar_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_progressbar_icon_get(const Evas_Object *obj);
   EAPI void         elm_progressbar_span_size_set(Evas_Object *obj, Evas_Coord size);
   EAPI Evas_Coord   elm_progressbar_span_size_get(const Evas_Object *obj);
   EAPI void         elm_progressbar_unit_format_set(Evas_Object *obj, const char *format);
   EAPI const char  *elm_progressbar_unit_format_get(const Evas_Object *obj);
   EAPI void         elm_progressbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);
   EAPI Eina_Bool    elm_progressbar_horizontal_get(const Evas_Object *obj);
   EAPI void         elm_progressbar_inverted_set(Evas_Object *obj, Eina_Bool inverted);
   EAPI Eina_Bool    elm_progressbar_inverted_get(const Evas_Object *obj);
   /* smart callbacks called:
    */
   /* available item styles:
    * default
    * wheel (simple style, no text, no progression, only pulse is available)
    */

   EAPI Evas_Object *elm_separator_add(Evas_Object *parent);
   EAPI void         elm_separator_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);
   EAPI Eina_Bool    elm_separator_horizontal_get(const Evas_Object *obj);
   /* smart callbacks called:
    */

   EAPI Evas_Object *elm_spinner_add(Evas_Object *parent);
   EAPI void	     elm_spinner_label_format_set(Evas_Object *obj, const char *fmt);
   EAPI const char  *elm_spinner_label_format_get(const Evas_Object *obj);
   EAPI void         elm_spinner_min_max_set(Evas_Object *obj, double min, double max);
   EAPI void         elm_spinner_min_max_get(const Evas_Object *obj, double *min, double *max);
   EAPI void	     elm_spinner_step_set(Evas_Object *obj, double step);
   EAPI double	     elm_spinner_step_get(const Evas_Object *obj);
   EAPI void	     elm_spinner_value_set(Evas_Object *obj, double val);
   EAPI double	     elm_spinner_value_get(const Evas_Object *obj);
   EAPI void         elm_spinner_wrap_set(Evas_Object *obj, Eina_Bool wrap);
   EAPI Eina_Bool    elm_spinner_wrap_get(const Evas_Object *obj);
   EAPI void	     elm_spinner_editable_set(Evas_Object *obj, Eina_Bool editable);
   EAPI Eina_Bool    elm_spinner_editable_get(const Evas_Object *obj);
   EAPI void	     elm_spinner_special_value_add(Evas_Object *obj, double value, const char *label);
   /* smart callbacks called:
    * "changed" - when the spinner value changes
    * "delay,changed" - when the spinner value changed, but a small time after a change (use this if you only want to respond to a change once the spinner is held still for a short while).
    */
   /* available item styles:
    * default
    * vertical (two up/down buttons at the right side and text left aligned)
    */

   EAPI Evas_Object *elm_index_add(Evas_Object *parent);
   EAPI void         elm_index_active_set(Evas_Object *obj, Eina_Bool active);
   EAPI void         elm_index_item_level_set(Evas_Object *obj, int level);
   EAPI int          elm_index_item_level_get(const Evas_Object *obj);
   EAPI const void  *elm_index_item_selected_get(const Evas_Object *obj, int level);
   EAPI void         elm_index_item_append(Evas_Object *obj, const char *letter, const void *item);
   EAPI void         elm_index_item_prepend(Evas_Object *obj, const char *letter, const void *item);
   EAPI void         elm_index_item_append_relative(Evas_Object *obj, const char *letter, const void *item, const void *relative);
   EAPI void         elm_index_item_prepend_relative(Evas_Object *obj, const char *letter, const void *item, const void *relative);
   EAPI void         elm_index_item_del(Evas_Object *obj, const void *item);
   EAPI void         elm_index_item_clear(Evas_Object *obj);
   EAPI void         elm_index_item_go(Evas_Object *obj, int level);
   /* smart callbacks called:
    * "changed" - when the selected index item changes
    * "delay,changed" - when the selected index item changes, but after some small idle period
    * "selected" - when the user releases heir finger and selects an item
    */

   typedef enum _Elm_Photocam_Zoom_Mode
     {
	ELM_PHOTOCAM_ZOOM_MODE_MANUAL = 0,
	ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT,
	ELM_PHOTOCAM_ZOOM_MODE_AUTO_FILL,
	ELM_PHOTOCAM_ZOOM_MODE_LAST
     } Elm_Photocam_Zoom_Mode;
   EAPI Evas_Object *elm_photocam_add(Evas_Object *parent);
   EAPI int          elm_photocam_file_set(Evas_Object *obj, const char *file);
   EAPI const char * elm_photocam_file_get(const Evas_Object *obj);
   EAPI void         elm_photocam_zoom_set(Evas_Object *obj, double zoom);
   EAPI double       elm_photocam_zoom_get(const Evas_Object *obj);
   EAPI void         elm_photocam_zoom_mode_set(Evas_Object *obj, Elm_Photocam_Zoom_Mode mode);
   EAPI Elm_Photocam_Zoom_Mode elm_photocam_zoom_mode_get(const Evas_Object *obj);
   EAPI void         elm_photocam_image_size_get(const Evas_Object *obj, int *w, int *h);
   EAPI void         elm_photocam_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);
   EAPI void         elm_photocam_image_region_show(Evas_Object *obj, int x, int y, int w, int h);
   EAPI void         elm_photocam_image_region_bring_in(Evas_Object *obj, int x, int y, int w, int h);
   EAPI void         elm_photocam_paused_set(Evas_Object *obj, Eina_Bool paused);
   EAPI Eina_Bool    elm_photocam_paused_get(const Evas_Object *obj);
   EAPI Evas_Object *elm_photocam_internal_image_get(const Evas_Object *obj);
   
   /* smart callbacks called:
    * "clicked" - when image clicked
    * "press" - when mouse/finger held down initially on image
    * "longpressed" - when mouse/finger held for long time on image
    * "clicked,double" - when mouse/finger double-clicked
    * "load" - when photo load begins
    * "loaded" - when photo load done
    * "load,details" - when detailed image load begins
    * "loaded,details" - when detailed image load done
    * "zoom,start" - when zooming started
    * "zoom,stop" - when zooming stopped
    * "zoom,change" - when auto zoom mode changed zoom level
    * "scroll - the content has been scrolled (moved)
    * "scroll,anim,start" - scrolling animation has started
    * "scroll,anim,stop" - scrolling animation has stopped
    * "scroll,drag,start" - dragging the contents around has started
    * "scroll,drag,stop" - dragging the contents around has stopped
    */

   typedef enum _Elm_Map_Zoom_Mode
     {
	ELM_MAP_ZOOM_MODE_MANUAL,
	ELM_MAP_ZOOM_MODE_AUTO_FIT,
	ELM_MAP_ZOOM_MODE_AUTO_FILL,
	ELM_MAP_ZOOM_MODE_LAST
     } Elm_Map_Zoom_Mode;

   typedef enum _Elm_Map_Sources
     {
	ELM_MAP_SOURCE_MAPNIK,
	ELM_MAP_SOURCE_OSMARENDER,
	ELM_MAP_SOURCE_CYCLEMAP,
	ELM_MAP_SOURCE_MAPLINT,
	ELM_MAP_SOURCE_CUSTOM_1,
	ELM_MAP_SOURCE_CUSTOM_2,
	ELM_MAP_SOURCE_CUSTOM_3,
	ELM_MAP_SOURCE_CUSTOM_4,
	ELM_MAP_SOURCE_CUSTOM_5,
	ELM_MAP_SOURCE_CUSTOM_6,
	ELM_MAP_SOURCE_CUSTOM_7,
	ELM_MAP_SOURCE_LAST
     } Elm_Map_Sources;

   typedef struct _Elm_Map_Marker Elm_Map_Marker;
   typedef struct _Elm_Map_Marker_Class Elm_Map_Marker_Class;
   typedef struct _Elm_Map_Group_Class Elm_Map_Group_Class;
   typedef Evas_Object *(*ElmMapMarkerGetFunc)      (Evas_Object *obj, Elm_Map_Marker *marker, void *data);
   typedef void         (*ElmMapMarkerDelFunc)      (Evas_Object *obj, Elm_Map_Marker *marker, void *data, Evas_Object *o);
   typedef Evas_Object *(*ElmMapMarkerIconGetFunc)  (Evas_Object *obj, Elm_Map_Marker *marker, void *data);
   typedef Evas_Object *(*ElmMapGroupIconGetFunc)  (Evas_Object *obj, void *data);
   typedef char *(*ElmMapSourceURLFunc) (int x, int y, int zoom);

   EAPI Evas_Object *elm_map_add(Evas_Object *parent);
   EAPI void         elm_map_zoom_set(Evas_Object *obj, int zoom);
   EAPI double       elm_map_zoom_get(const Evas_Object *obj);
   EAPI void         elm_map_zoom_mode_set(Evas_Object *obj, Elm_Map_Zoom_Mode mode);
   EAPI Elm_Map_Zoom_Mode elm_map_zoom_mode_get(const Evas_Object *obj);
   EAPI void	     elm_map_geo_region_get(const Evas_Object *obj, double *lon, double *lat);
   EAPI void         elm_map_geo_region_bring_in(Evas_Object *obj, double lon, double lat);
   EAPI void         elm_map_geo_region_show(Evas_Object *obj, double lon, double lat);
   EAPI void         elm_map_paused_set(Evas_Object *obj, Eina_Bool paused);
   EAPI Eina_Bool    elm_map_paused_get(const Evas_Object *obj);
   EAPI void	     elm_map_paused_markers_set(Evas_Object *obj, Eina_Bool paused);
   EAPI Eina_Bool    elm_map_paused_markers_get(const Evas_Object *obj);
   EAPI void	     elm_map_utils_convert_coord_into_geo(int x, int y, int size, double *lon, double *lat);
   EAPI void         elm_map_utils_convert_geo_into_coord(double lon, double lat, int size, int *x, int *y);
   EAPI Elm_Map_Marker *elm_map_marker_add(Evas_Object *obj, double lon, double lat, Elm_Map_Marker_Class *clas, Elm_Map_Group_Class *clas_group, void *data);
   EAPI void	     elm_map_max_marker_per_group_set(Evas_Object *obj, int max);
   EAPI void	     elm_map_marker_remove(Elm_Map_Marker *marker);
   EAPI void	     elm_map_marker_bring_in(Elm_Map_Marker *marker);
   EAPI void	     elm_map_marker_show(Elm_Map_Marker *marker);
   EAPI void	     elm_map_markers_list_show(Eina_List *markers);
   EAPI Evas_Object *elm_map_marker_object_get(Elm_Map_Marker *marker);
   EAPI void	     elm_map_marker_update(Elm_Map_Marker *marker);
   EAPI void	     elm_map_bubbles_close(Evas_Object *obj);

   EAPI Elm_Map_Group_Class * elm_map_group_class_new(Evas_Object *obj);
   EAPI void         elm_map_group_class_style_set(Elm_Map_Group_Class *clas, const char *style);
   EAPI void         elm_map_group_class_icon_cb_set(Elm_Map_Group_Class *clas, ElmMapGroupIconGetFunc icon_get);
   EAPI void	     elm_map_group_class_data_set(Elm_Map_Group_Class *clas, void *data);
   EAPI void         elm_map_group_class_zoom_displayed_set(Elm_Map_Group_Class *clas, int zoom);
   EAPI void	     elm_map_group_class_zoom_grouped_set(Elm_Map_Group_Class *clas, int zoom);
   EAPI void         elm_map_group_class_hide_set(Evas_Object *obj, Elm_Map_Group_Class *clas, Eina_Bool hide);

   EAPI Elm_Map_Marker_Class * elm_map_marker_class_new(Evas_Object *obj);
   EAPI void         elm_map_marker_class_style_set(Elm_Map_Marker_Class *clas, const char *style);
   EAPI void         elm_map_marker_class_icon_cb_set(Elm_Map_Marker_Class *clas, ElmMapMarkerIconGetFunc icon_get);
   EAPI void         elm_map_marker_class_get_cb_set(Elm_Map_Marker_Class *clas, ElmMapMarkerGetFunc get);
   EAPI void         elm_map_marker_class_del_cb_set(Elm_Map_Marker_Class *clas, ElmMapMarkerDelFunc del);

   EAPI void         elm_map_source_set(Evas_Object *obj, Elm_Map_Sources source);
   EAPI Elm_Map_Sources elm_map_source_get(const Evas_Object *obj);
   EAPI void         elm_map_source_custom_api_set(Elm_Map_Sources source, const char *label, 
				int zoom_min, int zoom_max, ElmMapSourceURLFunc url_cb);
   EAPI int          elm_map_source_zoom_min_get(Elm_Map_Sources source);
   EAPI int	     elm_map_source_zoom_max_get(Elm_Map_Sources source);
   EAPI const char * elm_map_source_name_get(Elm_Map_Sources source);
   /* smart callbacks called:
    * "clicked" - when image clicked
    * "press" - when mouse/finger held down initially on image
    * "longpressed" - when mouse/finger held for long time on image
    * "clicked,double" - when mouse/finger double-clicked
    * "load,details" - when detailed image load begins
    * "loaded,details" - when detailed image load done
    * "zoom,start" - when zooming started
    * "zoom,stop" - when zooming stopped
    * "zoom,change" - when auto zoom mode changed zoom level
    * "scroll - the content has been scrolled (moved)
    * "scroll,anim,start" - scrolling animation has started
    * "scroll,anim,stop" - scrolling animation has stopped
    * "scroll,drag,start" - dragging the contents around has started
    * "scroll,drag,stop" - dragging the contents around has stopped
    */

   typedef enum _Elm_Panel_Orient
     {
        ELM_PANEL_ORIENT_TOP,
          ELM_PANEL_ORIENT_BOTTOM,
          ELM_PANEL_ORIENT_LEFT,
          ELM_PANEL_ORIENT_RIGHT,
     } Elm_Panel_Orient;
   EAPI Evas_Object          *elm_panel_add(Evas_Object *parent);
   EAPI void                  elm_panel_orient_set(Evas_Object *obj, Elm_Panel_Orient orient);
   EAPI Elm_Panel_Orient      elm_panel_orient_get(Evas_Object *obj);
   EAPI void                  elm_panel_content_set(Evas_Object *obj, Evas_Object *content);
   EAPI void                  elm_panel_hidden_set(Evas_Object *obj, Eina_Bool hidden);
   EAPI Eina_Bool             elm_panel_hidden_get(Evas_Object *obj);
   EAPI void                  elm_panel_toggle(Evas_Object *obj);

   typedef enum _Elm_Flip_Mode
     {
        ELM_FLIP_ROTATE_Y_CENTER_AXIS,
          ELM_FLIP_ROTATE_X_CENTER_AXIS,
          ELM_FLIP_ROTATE_XZ_CENTER_AXIS,
          ELM_FLIP_ROTATE_YZ_CENTER_AXIS
     } Elm_Flip_Mode;
   
   EAPI Evas_Object *elm_flip_add(Evas_Object *parent);
   EAPI void         elm_flip_content_front_set(Evas_Object *obj, Evas_Object *content);
   EAPI void         elm_flip_content_back_set(Evas_Object *obj, Evas_Object *content);
   EAPI Evas_Object *elm_flip_content_front_get(const Evas_Object *obj);
   EAPI Evas_Object *elm_flip_content_back_get(const Evas_Object *obj);
   EAPI Eina_Bool    elm_flip_front_get(const Evas_Object *obj);
   EAPI void         elm_flip_perspective_set(Evas_Object *obj, Evas_Coord foc, Evas_Coord x, Evas_Coord y);
   EAPI void         elm_flip_go(Evas_Object *obj, Elm_Flip_Mode mode);
   /* smart callbacks called:
    * "animate,done" - when a flip animation is finished
    */
   
   EAPI Evas_Object *elm_scrolled_entry_add(Evas_Object *parent);
   EAPI void         elm_scrolled_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);
   EAPI Eina_Bool    elm_scrolled_entry_single_line_get(const Evas_Object *obj);
   EAPI void         elm_scrolled_entry_password_set(Evas_Object *obj, Eina_Bool password);
   EAPI Eina_Bool    elm_scrolled_entry_password_get(const Evas_Object *obj);
   EAPI void         elm_scrolled_entry_entry_set(Evas_Object *obj, const char *entry);
   EAPI const char  *elm_scrolled_entry_entry_get(const Evas_Object *obj);
   EAPI const char  *elm_scrolled_entry_selection_get(const Evas_Object *obj);
   EAPI void         elm_scrolled_entry_entry_insert(Evas_Object *obj, const char *entry);
   EAPI void         elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Eina_Bool wrap);
   EAPI void         elm_scrolled_entry_line_char_wrap_set(Evas_Object *obj, Eina_Bool wrap);
   EAPI void         elm_scrolled_entry_editable_set(Evas_Object *obj, Eina_Bool editable);
   EAPI Eina_Bool    elm_scrolled_entry_editable_get(const Evas_Object *obj);
   EAPI void         elm_scrolled_entry_select_none(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_select_all(Evas_Object *obj);
   EAPI Eina_Bool    elm_scrolled_entry_cursor_next(Evas_Object *obj);
   EAPI Eina_Bool    elm_scrolled_entry_cursor_prev(Evas_Object *obj);
   EAPI Eina_Bool    elm_scrolled_entry_cursor_up(Evas_Object *obj);
   EAPI Eina_Bool    elm_scrolled_entry_cursor_down(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_cursor_begin_set(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_cursor_end_set(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_cursor_line_begin_set(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_cursor_line_end_set(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_cursor_selection_begin(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_cursor_selection_end(Evas_Object *obj);
   EAPI Eina_Bool    elm_scrolled_entry_cursor_is_format_get(const Evas_Object *obj);
   EAPI Eina_Bool    elm_scrolled_entry_cursor_is_visible_format_get(const Evas_Object *obj);
   EAPI const char  *elm_scrolled_entry_cursor_content_get(const Evas_Object *obj);
   EAPI void         elm_scrolled_entry_selection_cut(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_selection_copy(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_selection_paste(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_context_menu_clear(Evas_Object *obj);
   EAPI void         elm_scrolled_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);
   EAPI void         elm_scrolled_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled);
   EAPI Eina_Bool    elm_scrolled_entry_context_menu_disabled_get(const Evas_Object *obj);
   EAPI void         elm_scrolled_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v);
   EAPI void         elm_scrolled_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

   EAPI Evas_Object *elm_conformant_add(Evas_Object *parent);
   EAPI void         elm_conformant_content_set(Evas_Object *obj, Evas_Object *content);

   EAPI Evas_Object *elm_mapbuf_add(Evas_Object *parent);
   EAPI void         elm_mapbuf_content_set(Evas_Object *obj, Evas_Object *content);
   EAPI void         elm_mapbuf_enabled_set(Evas_Object *obj, Eina_Bool enabled);
   EAPI Eina_Bool    elm_mapbuf_enabled_get(const Evas_Object *obj);
   EAPI void         elm_mapbuf_smooth_set(Evas_Object *obj, Eina_Bool smooth);
   EAPI Eina_Bool    elm_mapbuf_smooth_get(const Evas_Object *obj);
   EAPI void         elm_mapbuf_alpha_set(Evas_Object *obj, Eina_Bool alpha);
   EAPI Eina_Bool    elm_mapbuf_alpha_get(const Evas_Object *obj);

   typedef struct _Picker_Item Elm_Picker_Item;
   EAPI Evas_Object     *elm_picker_add(Evas_Object *parent);
   EAPI void            elm_picker_next(Evas_Object *obj);
   EAPI void            elm_picker_prev(Evas_Object *obj);
   EAPI Elm_Picker_Item *elm_picker_item_append(Evas_Object *obj, const char *label, void (*func)(void *data, Evas_Object *obj, void *event_info), void *data);
   EAPI Elm_Picker_Item *elm_picker_item_prepend(Evas_Object *obj, const char *label, void (*func)(void *data, Evas_Object *obj, void *event_info), void *data);
   EAPI const Eina_List *elm_picker_items_get(Evas_Object *obj);
   EAPI Elm_Picker_Item *elm_picker_first_item_get(Evas_Object *obj);
   EAPI Elm_Picker_Item *elm_picker_last_item_get(Evas_Object *obj);
   EAPI Elm_Picker_Item *elm_picker_selected_item_get(Evas_Object *obj);
   EAPI void             elm_picker_item_selected_set(Elm_Picker_Item *item);
   EAPI void             elm_picker_item_del(Elm_Picker_Item *item);
   EAPI const char      *elm_picker_item_label_get(Elm_Picker_Item *item);
   EAPI void             elm_picker_item_label_set(Elm_Picker_Item *item, const char *label);
   EAPI Elm_Picker_Item *elm_picker_item_prev(Elm_Picker_Item *item);
   EAPI Elm_Picker_Item *elm_picker_item_next(Elm_Picker_Item *item);
   /* smart callback called:
    * "changed" - when picker selected item is changed
    * "overflowed" - when picker item is changed to first item from last item
    * "underflowed" - when picker item is changed to last item from first item.
    */
   /* available styles:
    * default
    * no_button
    * timepicker/ampm
    */

   typedef struct _Picker2_Item Elm_Picker2_Item;
   EAPI Evas_Object     *elm_picker2_add(Evas_Object *parent);
   EAPI void		elm_picker2_row_height_set(Evas_Object *obj, unsigned int row_height);
   EAPI void            elm_picker2_next(Evas_Object *obj);
   EAPI void            elm_picker2_prev(Evas_Object *obj);
   EAPI Elm_Picker2_Item *elm_picker2_item_append(Evas_Object *obj, const char *label, void (*func)(void *data, Evas_Object *obj, void *event_info), void *data);
   EAPI Elm_Picker2_Item *elm_picker2_item_prepend(Evas_Object *obj, const char *label, void (*func)(void *data, Evas_Object *obj, void *event_info), void *data);
   EAPI const Eina_List *elm_picker2_items_get(Evas_Object *obj);
   EAPI Elm_Picker2_Item *elm_picker2_first_item_get(Evas_Object *obj);
   EAPI Elm_Picker2_Item *elm_picker2_last_item_get(Evas_Object *obj);
   EAPI Elm_Picker2_Item *elm_picker2_selected_item_get(Evas_Object *obj);
   EAPI void             elm_picker2_item_selected_set(Elm_Picker2_Item *item);
   EAPI void             elm_picker2_item_del(Elm_Picker2_Item *item);
   EAPI const char      *elm_picker2_item_label_get(Elm_Picker2_Item *item);
   EAPI void             elm_picker2_item_label_set(Elm_Picker2_Item *item, const char *label);
   EAPI Elm_Picker2_Item *elm_picker2_item_prev(Elm_Picker2_Item *item);
   EAPI Elm_Picker2_Item *elm_picker2_item_next(Elm_Picker2_Item *item);
   EAPI void             *elm_picker2_item_data_get(Elm_Picker2_Item *item);
   EAPI void              elm_picker2_item_data_set(Elm_Picker2_Item *item, void *data);
   EAPI Eina_Bool         elm_picker2_item_disabled_get(Elm_Picker2_Item *item);
   EAPI void              elm_picker2_item_disabled_set(Elm_Picker2_Item *item, Eina_Bool disabled);
    /* smart callback called:
    * "selected" - when picker selected item is changed
    * "overflowed" - when picker item is changed to first item from last item
    * "underflowed" - when picker item is changed to last item from first item.
    */
   /* available styles:
    * default
    */

   EAPI Evas_Object *elm_timepicker_add(Evas_Object *parent);
   EAPI void         elm_timepicker_time_set(Evas_Object *obj, int hrs, int min, int sec);
   EAPI void         elm_timepicker_time_get(Evas_Object *obj, int *hrs, int *min, int *sec);
   EAPI void         elm_timepicker_show_am_pm_set(Evas_Object *obj, Eina_Bool am_pm);
   EAPI void         elm_timepicker_show_seconds_set(Evas_Object *obj, Eina_Bool seconds);
   /* smart callback called:
    * "changed" - when timepicker selected item is changed
    */
   /* available styles:
    * default
    */

   EAPI Evas_Object *elm_datepicker_add(Evas_Object *parent);
   EAPI void         elm_datepicker_date_set(Evas_Object *obj, int year, int month, int day);
   EAPI void         elm_datepicker_date_get(Evas_Object *obj, int *year, int *month, int *day);
   EAPI void         elm_datepicker_date_min_set(Evas_Object *obj, int year, int month, int day);  // Not implemented
   EAPI void         elm_datepicker_date_min_get(Evas_Object *obj, int *year, int *month, int *day);
   EAPI void         elm_datepicker_date_max_set(Evas_Object *obj, int year, int month, int day);  // Not implemented
   EAPI void         elm_datepicker_date_max_get(Evas_Object *obj, int *year, int *month, int *day);
   EAPI void         elm_datepicker_date_format_set(Evas_Object *obj, const char *fmt);
   EAPI const char  *elm_datepicker_date_format_get(Evas_Object *obj);
   /* smart callback called:
    * "changed" - when datepicker selected item is changed
    */
   /* available styles:
    * default
    */

   EAPI Evas_Object *elm_datepicker2_add(Evas_Object *parent);
   EAPI void         elm_datepicker2_date_set(Evas_Object *obj, int year, int month, int day);
   EAPI void         elm_datepicker2_date_get(Evas_Object *obj, int *year, int *month, int *day);
   EAPI void         elm_datepicker2_date_min_set(Evas_Object *obj, int year, int month, int day);  // Not implemented
   EAPI void         elm_datepicker2_date_min_get(Evas_Object *obj, int *year, int *month, int *day);
   EAPI void         elm_datepicker2_date_max_set(Evas_Object *obj, int year, int month, int day);  // Not implemented
   EAPI void         elm_datepicker2_date_max_get(Evas_Object *obj, int *year, int *month, int *day);
   EAPI void         elm_datepicker2_date_format_set(Evas_Object *obj, const char *fmt);
   EAPI const char  *elm_datepicker2_date_format_get(Evas_Object *obj);
   /* smart callback called:
    * "changed" - when datepicker selected item is changed
    */
   /* available styles:
    * default
    */
    
    /* titlebar */
   EAPI Evas_Object *elm_titlebar_add(Evas_Object *parent);
   EAPI void         elm_titlebar_label_set(Evas_Object *obj, const char *label);
   EAPI const char  *elm_titlebar_label_get(Evas_Object *obj);
   EAPI void         elm_titlebar_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_titlebar_icon_get(Evas_Object *obj);
   EAPI void         elm_titlebar_end_set(Evas_Object *obj, Evas_Object *end);
   EAPI Evas_Object *elm_titlebar_end_get(Evas_Object *obj);    
	/* popup */
	typedef enum _Elm_Popup_Response
	{
		ELM_POPUP_RESPONSE_NONE = -1,
		ELM_POPUP_RESPONSE_TIMEOUT = -2,
		ELM_POPUP_RESPONSE_OK = -3,
		ELM_POPUP_RESPONSE_CANCEL = -4,	
		ELM_POPUP_RESPONSE_CLOSE = -5

	}Elm_Popup_Response;

	typedef enum _Elm_Popup_Mode
	{
		ELM_POPUP_TYPE_NONE = 0,
		ELM_POPUP_TYPE_ALERT = (1 << 0)
		
	}Elm_Popup_Mode;
	typedef enum _Elm_Popup_Orient
	{
		 ELM_POPUP_ORIENT_TOP,
		 ELM_POPUP_ORIENT_CENTER,
		 ELM_POPUP_ORIENT_BOTTOM,
		 ELM_POPUP_ORIENT_LEFT,
		 ELM_POPUP_ORIENT_RIGHT,
		 ELM_POPUP_ORIENT_TOP_LEFT,
		 ELM_POPUP_ORIENT_TOP_RIGHT,
		 ELM_POPUP_ORIENT_BOTTOM_LEFT,
		 ELM_POPUP_ORIENT_BOTTOM_RIGHT
	}Elm_Popup_Orient;

	 /* smart callbacks called:
    * "response" - when ever popup is closed, this signal is sent with appropriate response id.".
    */

	EAPI Evas_Object *elm_popup_add(Evas_Object *parent);
	EAPI void elm_popup_desc_set(Evas_Object *obj, const char *text);
	EAPI const char* elm_popup_desc_get(Evas_Object *obj);
	EAPI void elm_popup_title_set(Evas_Object *obj, const char *text);
	EAPI const char* elm_popup_title_get(Evas_Object *obj);
	EAPI void elm_popup_content_set(Evas_Object *obj, Evas_Object *content);
	EAPI Evas_Object* elm_popup_content_get(Evas_Object *obj);
	EAPI void elm_popup_buttons_add(Evas_Object *obj,int no_of_buttons, char *first_button_text,  ...);
	Evas_Object* elm_popup_add_with_buttons(Evas_Object *parent, char *title, char *desc_text,int no_of_buttons, char *first_button_text, ... );
	EAPI void elm_popup_timeout_set(Evas_Object *popup, int timeout);
	EAPI void elm_popup_set_mode(Evas_Object *popup, Elm_Popup_Mode mode);
	EAPI void elm_popup_response(Evas_Object *popup, int  response_id);
	EAPI void elm_popup_orient_set(Evas_Object *popup, Elm_Popup_Orient orient);
   /* Contextual Popup */
   typedef struct _Ctx_Popup_Item Elm_Ctxpopup_Item;

   EAPI Evas_Object* elm_ctxpopup_add(Evas_Object* parent);
   EAPI void elm_ctxpopup_horizontal_set(Evas_Object* obj, Eina_Bool horizontal);
   EAPI Eina_Bool elm_ctxpopup_horizontal_get(Evas_Object* obj);
   EAPI void elm_ctxpopup_item_icon_set(Elm_Ctxpopup_Item* item, Evas_Object* icon);
   EAPI void elm_ctxpopup_item_label_set(Elm_Ctxpopup_Item* item, const char* label);
   EAPI Elm_Ctxpopup_Item* elm_ctxpopup_icon_add( Evas_Object* obj, Evas_Object* icon, void (*func ) (void* data, Evas_Object* obj, void* event_info ),	const void* data ); 
   EAPI Elm_Ctxpopup_Item* elm_ctxpopup_label_add( Evas_Object* obj, const char* label,	void (*func ) (void* data, Evas_Object* obj, void* event_info ), const void* data );
   EAPI void elm_ctxpopup_item_del(Elm_Ctxpopup_Item* item);
   EAPI void elm_ctxpopup_item_disabled_set(Elm_Ctxpopup_Item* item, Eina_Bool disabled);
   EAPI void elm_ctxpopup_clear(Evas_Object* obj);
   EAPI const char* elm_ctxpopup_item_label_get( Elm_Ctxpopup_Item* item );	
   EAPI Evas_Object* elm_ctxpopup_item_icon_get( Elm_Ctxpopup_Item* item ); 
   EAPI void elm_ctxpopup_scroller_disabled_set(Evas_Object* obj, Eina_Bool disabled );



/* Tab */
   typedef enum _Elm_Tab_Move_Type
     {
	ELM_TAB_MOVE_LEFT = 0,
	ELM_TAB_MOVE_RIGHT = 1
     } Elm_Tab_Move_Type;

   typedef enum _Elm_Tab_Mode_Type
     {
	ELM_TAB_PORTRAIT_2 = 2,
	ELM_TAB_PORTRAIT_3 = 3,
	ELM_TAB_PORTRAIT_4 = 4,
	ELM_TAB_LANDSCAPE_2 = 12,
	ELM_TAB_LANDSCAPE_3 = 13,
	ELM_TAB_LANDSCAPE_4 = 14,
	ELM_TAB_LANDSCAPE_5 = 15,
     } Elm_Tab_Mode_Type;

   typedef struct _Elm_Tab_Item Elm_Tab_Item;

   EAPI Evas_Object *elm_tab_add(Evas_Object *parent);
   EAPI void elm_tab_set(Evas_Object *obj, int mode);
   EAPI Elm_Tab_Item *elm_tab_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data);
   EAPI void elm_tab_item_del(Elm_Tab_Item *item);
   EAPI void elm_tab_item_select(Elm_Tab_Item *item);
   EAPI Evas_Object *elm_tab_item_icon_get(Elm_Tab_Item *item);
   EAPI const char *elm_tab_item_label_get(Elm_Tab_Item *item);
   EAPI void elm_tab_item_label_set(Elm_Tab_Item *item, const char *label);
   EAPI void elm_tab_item_badge_set(Elm_Tab_Item *item, const int badge);   
   EAPI Elm_Tab_Item *elm_tab_selected_item_get(Evas_Object *obj);   
   EAPI Elm_Tab_Item *elm_tab_first_item_get(Evas_Object *obj);   
   EAPI Elm_Tab_Item *elm_tab_last_item_get(Evas_Object *obj);   
   EAPI Eina_List *elm_tab_items_get(Evas_Object *obj);   
   EAPI Elm_Tab_Item *elm_tab_item_prev(Elm_Tab_Item *item);   
   EAPI Elm_Tab_Item *elm_tab_item_next(Elm_Tab_Item *item);   
   EAPI void elm_tab_move(Evas_Object *obj, int direction);
   EAPI void elm_tab_edit_disable_set(Evas_Object *obj, Eina_Bool disable);
   EAPI Eina_Bool elm_tab_edit_disable_get(Evas_Object *obj);


   /* animator */
   typedef struct _animator Elm_Animator;

   typedef enum { ELM_ANIMATOR_CURVE_LINEAR, ELM_ANIMATOR_CURVE_IN_OUT, ELM_ANIMATOR_CURVE_IN, ELM_ANIMATOR_CURVE_OUT } Elm_Animator_Curve_Style; 

   EAPI Elm_Animator* elm_animator_add( Evas_Object* parent );
   EAPI void elm_animator_del( Elm_Animator* animator );
   EAPI void elm_animator_duration_set( Elm_Animator* animator, const double duration );
   EAPI void elm_animator_operation_callback_set( Elm_Animator* animator, void (*op)(void*, Elm_Animator*, const double), void* data ); 
   EAPI void elm_animator_completion_callback_set( Elm_Animator* animator, void (*op)(void*), void* data );
   EAPI void elm_animator_operation_set( Elm_Animator* animator, void (*op)(void*, Elm_Animator*, const double), void* data ) __attribute__((deprecated));
   EAPI void elm_animator_completion_set( Elm_Animator* animator, void (*op)(void*), void* data ) __attribute__((deprecated));
   EAPI void elm_animator_stop( Elm_Animator* animator );
   EAPI void elm_animator_repeat_set( Elm_Animator* animator, const unsigned int repeat_cnt );
   EAPI void elm_animator_animate( Elm_Animator* animator );
   EAPI void elm_animator_curve_style_set( Elm_Animator* animator, Elm_Animator_Curve_Style cs );
   EAPI void elm_animator_auto_reverse_set( Elm_Animator* animator, Eina_Bool reverse );
   EAPI Eina_Bool elm_animator_auto_reverse_get( Elm_Animator* animator );
   EAPI Eina_Bool elm_animator_operating_get( Elm_Animator* animator );
   EAPI unsigned int elm_animator_repeat_get( Elm_Animator* animator );

   

  /*tansit*/
   typedef struct _transit Elm_Transit;
   typedef struct _effect Elm_Effect;

   EAPI Elm_Transit* elm_transit_add( Evas_Object* parent );
   EAPI void elm_transit_del( Elm_Transit* transit );
   EAPI void elm_transit_run( Elm_Transit* transit, const double duration );
   EAPI void elm_transit_stop( Elm_Transit* transit );

   //Please Use elm_transit_completion_callback_set
   EAPI void elm_transit_completion_set( Elm_Transit* transit, void (*op)(void*), void* data ) __attribute__((deprecated));
   EAPI void elm_transit_completion_callback_set( Elm_Transit* transit, void (*op)(void*,Elm_Transit*), void* data );
   EAPI void elm_transit_repeat_set( Elm_Transit* transit, const unsigned int repeat );
   EAPI void elm_transit_auto_reverse_set( Elm_Transit* transit, Eina_Bool reverse );
   EAPI void elm_transit_curve_style_set( Elm_Transit* transit, Elm_Animator_Curve_Style cs );
   EAPI void elm_transit_event_block_disabled_set( Elm_Transit* transit, Eina_Bool disable );
   EAPI Eina_Bool elm_transit_fx_insert( Elm_Transit* transit, Elm_Effect* effect );
   EAPI void elm_transit_fx_clear( Elm_Transit* transit );
   EAPI Eina_Bool elm_transit_event_block_disabled_get( Elm_Transit* transit );

   /*Translation FX*/  
   EAPI Elm_Effect* elm_fx_translation_add( Evas_Object* obj, const Evas_Coord from_x, const Evas_Coord from_y, const Evas_Coord to_x,  const Evas_Coord to_y );

   /*Transfer FX*/  
   //Please Use Translation FX
   EAPI Elm_Effect* elm_fx_transfer_add( Evas_Object* obj, const Evas_Coord from_x, const Evas_Coord from_y, const Evas_Coord to_x,  const Evas_Coord to_y ) __attribute__((deprecated));

   /*Resizing FX*/  
   EAPI Elm_Effect* elm_fx_resizing_add( Evas_Object* obj, const Evas_Coord from_w, const Evas_Coord from_h, const Evas_Coord to_w,  const Evas_Coord to_h );

   /*Zoom Fx*/  
   EAPI Elm_Effect* elm_fx_zoom_add( Evas_Object* obj, float from_rate, float to_rate );

   /*Flip Fx*/
   typedef enum { ELM_FX_FLIP_AXIS_X, ELM_FX_FLIP_AXIS_Y } Elm_Fx_Flip_Axis;
   EAPI Elm_Effect* elm_fx_flip_add( Evas_Object* front, Evas_Object* back, const Elm_Fx_Flip_Axis axis, const Eina_Bool cw );

   /*Blend Fx*/
   EAPI Elm_Effect* elm_fx_blend_add( Evas_Object* before, Evas_Object* after );

   /*Fade Fx*/
   EAPI Elm_Effect* elm_fx_fade_add( Evas_Object* before, Evas_Object* after );
    
   /*Rotation Fx*/
   EAPI Elm_Effect* elm_fx_rotation_add( Evas_Object* obj, const float from_degree, const float to_degree, const Eina_Bool cw );

   /*Color Fx*/
   EAPI Elm_Effect* elm_fx_color_add( Evas_Object* obj, const unsigned int from_r, const unsigned int from_g, const unsigned int from_b, const unsigned int from_a,  const unsigned int to_r ,const unsigned int to_g, const unsigned int to_b, const unsigned int to_a );

   /*Wipe Fx*/
   typedef enum { ELM_FX_WIPE_DIR_LEFT, ELM_FX_WIPE_DIR_RIGHT, ELM_FX_WIPE_DIR_UP, ELM_FX_WIPE_DIR_DOWN } Elm_Fx_Wipe_Dir;
   typedef enum { ELM_FX_WIPE_TYPE_HIDE, ELM_FX_WIPE_TYPE_SHOW } Elm_Fx_Wipe_Type;
   EAPI Elm_Effect* elm_fx_wipe_add( Evas_Object* obj, Elm_Fx_Wipe_Type type,  Elm_Fx_Wipe_Dir dir );

   /*ResizableFlip Fx*/
   EAPI Elm_Effect* elm_fx_resizable_flip_add( Evas_Object* front, Evas_Object* back, const Elm_Fx_Flip_Axis axis, const Eina_Bool cw );
   
   /*Transform Fx*/
   typedef struct _elm_fx_matrix {
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
   } Elm_Fx_Matrix;
/*
   EAPI Elm_Effect* elm_fx_transform_add( Evas_Object* obj, Elm_Fx_Matrix* from, Elm_Fx_Matrix* to ); 
   EAPI void elm_fx_transform_identity_set( Elm_Fx_Matrix* m );
   EAPI void elm_fx_transform_translate( Elm_Fx_Matrix* m, const float pos_x, const float pos_y, const float pos_z );
   EAPI void elm_fx_transform_scale( Elm_Fx_Matrix* m, const float scale_x, const float scale_y, const float scale_z );
   EAPI void elm_fx_transform_rotate( Elm_Fx_Matrix* m, const float rad_x, const float rad_y, const float rad_z );
   EAPI void elm_fx_transform_multiply( Elm_Fx_Matrix* m, Elm_Fx_Matrix* m1, Elm_Fx_Matrix* m2 );
   */

   /* NavigationBar */
   EAPI Evas_Object *elm_navigationbar_add(Evas_Object *parent);
   EAPI void elm_navigationbar_push(Evas_Object *obj, const char *title, Evas_Object *left_btn, Evas_Object *right_btn, Evas_Object *content, Eina_Bool animation);
   EAPI void elm_navigationbar_pop(Evas_Object *obj, Eina_Bool animation);
   EAPI void elm_navigationbar_title_set(Evas_Object *obj, Evas_Object *content, const char *title);
   EAPI const char *elm_navigationbar_title_get(Evas_Object *obj, Evas_Object *content);
   EAPI void elm_navigationbar_back_button_set(Evas_Object *obj, Evas_Object *content, Evas_Object *button, Eina_Bool animation);
   EAPI Evas_Object *elm_navigationbar_back_button_get(Evas_Object *obj, Evas_Object *content);
   EAPI void elm_navigationbar_left_button_set(Evas_Object *obj, Evas_Object *content, Evas_Object *button, Eina_Bool animation);
   EAPI Evas_Object *elm_navigationbar_left_button_get(Evas_Object *obj, Evas_Object *content);
   EAPI void elm_navigationbar_right_button_set(Evas_Object *obj, Evas_Object *content, Evas_Object *button, Eina_Bool animation);
   EAPI Evas_Object *elm_navigationbar_right_button_get(Evas_Object *obj, Evas_Object *content);
   EAPI Evas_Object *elm_navigationbar_content_top_get(Evas_Object *obj);
   EAPI void elm_navigationbar_hidden_set(Evas_Object *obj, Eina_Bool hidden, Eina_Bool animation);
   

   /* Tab Bar */
   #define TAB_BAR_SYSTEM_ICON_CONTACT "elm/icon/tabbar_contact/default" 
   #define TAB_BAR_SYSTEM_ICON_GROUP "elm/icon/tabbar_group/default" 
   #define TAB_BAR_SYSTEM_ICON_SONG "elm/icon/tabbar_song/default" 
   #define TAB_BAR_SYSTEM_ICON_FAVORITES "elm/icon/tabbar_favorites/default" 
   #define TAB_BAR_SYSTEM_ICON_SEARCH "elm/icon/tabbar_search/default" 
   #define TAB_BAR_SYSTEM_ICON_MESSAGE "elm/icon/tabbar_message/default" 
   #define TAB_BAR_SYSTEM_ICON_SETTING "elm/icon/tabbar_setting/default" 

   EAPI Evas_Object *elm_tabbar_add(Evas_Object *parent);
   EAPI Elm_Tab_Item *elm_tabbar_item_add(Evas_Object *obj, const char *icon_path, const char *label, Evas_Object *view);
   EAPI Evas_Object *elm_tabbar_item_icon_get(Elm_Tab_Item *it);
   EAPI const char *elm_tabbar_item_label_get(Elm_Tab_Item *it);
   EAPI void elm_tabbar_item_label_set(Elm_Tab_Item *it, const char *label);
   EAPI void elm_tabbar_item_select(Elm_Tab_Item *it);
   EAPI void elm_tabbar_edit_start(Evas_Object *obj);
   EAPI void elm_tabbar_item_visible_set(Elm_Tab_Item *it, Eina_Bool bar);

  /* smart callbacks called:
   * "clicked" - when item clicked
    */

  /* diskcontroller */
  typedef struct _Elm_Diskcontroller_Item Elm_Diskcontroller_Item;

   EAPI Evas_Object *elm_diskcontroller_add(Evas_Object *parent);
   EAPI void elm_diskcontroller_round_set(Evas_Object *obj, Eina_Bool round);
   EAPI Elm_Diskcontroller_Item *elm_diskcontroller_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data);
   EAPI void elm_diskcontroller_item_del(Elm_Diskcontroller_Item *it);
   EAPI const char *elm_diskcontroller_item_label_get(Elm_Diskcontroller_Item *it);
   EAPI void elm_diskcontroller_item_label_set(Elm_Diskcontroller_Item *it, const char *label);
   EAPI void elm_diskcontroller_item_focus_set(Elm_Diskcontroller_Item *it);
   /* smart callbacks called:
    * "stop" - when scroller stop
    */

   /* SearchBar */
   EAPI Evas_Object *elm_searchbar_add(Evas_Object *parent);
   EAPI void elm_searchbar_text_set(Evas_Object *obj, const char *entry);
   EAPI const char *elm_searchbar_text_get(Evas_Object *obj);
   EAPI Evas_Object *elm_searchbar_entry_get(Evas_Object *obj);
   EAPI void elm_searchbar_cancel_button_animation_set(Evas_Object *obj, Eina_Bool cancel_btn_ani_flag);

   /* actionsheet */
   typedef enum _Elm_Actionsheet_Button_Type {
      ELM_ACTIONSHEET_BT_NORMAL = 0,
      ELM_ACTIONSHEET_BT_CANCEL,
      ELM_ACTIONSHEET_BT_DESTRUCTIVE
   } Elm_Actionsheet_Button_Type;

   EAPI Evas_Object *elm_actionsheet_add(Evas_Object *parent);
   EAPI void elm_actionsheet_title_set(Evas_Object *obj, const char *title);
   EAPI void elm_actionsheet_button_add(Evas_Object *obj, const Elm_Actionsheet_Button_Type type, const char *title, void (*button_clicked_cb) (void *data, Evas_Object *obj, void *event_info), void *event_info);
   EAPI int elm_actionsheet_button_title_get(Evas_Object *obj, const int index, char **title);

   /* Toolbar 2 */
   typedef struct _Elm_Toolbar2_Item Elm_Toolbar2_Item;
   EAPI Evas_Object *elm_toolbar2_add(Evas_Object *parent);
   EAPI void elm_toolbar2_icon_size_set(Evas_Object *obj, int icon_size);
   EAPI int elm_toolbar2_icon_size_get(Evas_Object *obj);
   EAPI Elm_Toolbar2_Item *elm_toolbar2_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data);
   EAPI Evas_Object *elm_toolbar2_item_icon_get(Elm_Toolbar2_Item *item);
   EAPI void elm_toolbar2_item_del(Elm_Toolbar2_Item *it);
   EAPI void elm_toolbar2_item_del_cb_set(Elm_Toolbar2_Item *it, void (*func)(void *data, Evas_Object *obj, void *event_info));
   EAPI void elm_toolbar2_scrollable_set(Evas_Object *obj, Eina_Bool scrollable);
   EAPI void elm_toolbar2_homogenous_set(Evas_Object *obj, Eina_Bool homogenous);
   EAPI void elm_toolbar2_align_set(Evas_Object *obj, double align);

  /* colorpicker */
   EAPI Evas_Object *elm_colorpicker_add(Evas_Object *parent);
   EAPI void elm_colorpicker_color_set(Evas_Object *obj, unsigned int r, unsigned int g , unsigned int b);
   EAPI void elm_colorpicker_color_get(Evas_Object *obj, unsigned int *r, unsigned int *g , unsigned int *b);
    /* smart callbacks called:
    * "clicked" - when bar or button is clicked
    */


 /* colorpalette */
    typedef struct _Colorpalette_Color Elm_Colorpalette_Color; 

    struct _Colorpalette_Color {
     unsigned int r, g, b;
    };

    EAPI Evas_Object *elm_colorpalette_add(Evas_Object *parent);
    EAPI void elm_colorpalette_color_set(Evas_Object *obj, int color_num, Elm_Colorpalette_Color *color);
    EAPI void elm_colorpalette_row_column_set(Evas_Object *obj, int row, int col);
    /* smart callbacks called:
    * "clicked" - when image clicked
    */

   /* editfield */
   EAPI Evas_Object *elm_editfield_add(Evas_Object *parent);
   EAPI void elm_editfield_label_set(Evas_Object *obj, const char *label);
   EAPI const char* elm_editfield_label_get(Evas_Object *obj);
   EAPI void elm_editfield_guide_text_set(Evas_Object *obj, const char *text);
   EAPI const char* elm_editfield_guide_text_get(Evas_Object *obj);
   EAPI Evas_Object *elm_editfield_entry_get(Evas_Object *obj);
   EAPI Evas_Object *elm_editfield_clear_button_show(Evas_Object *obj, Eina_Bool show);
   EAPI Eina_Bool elm_editfield_right_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_editfield_right_icon_get(Evas_Object *obj);
   EAPI Eina_Bool elm_editfield_left_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI Evas_Object *elm_editfield_lett_icon_get(Evas_Object *obj);
   EAPI void elm_editfield_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);

 /* softkey */
   typedef enum {
		ELM_SK_LEFT_BTN,
		ELM_SK_RIGHT_BTN,
   } Elm_Softkey_Type;

   /* softkey */
   typedef struct _Elm_Softkey_Item Elm_Softkey_Item;
   EAPI Evas_Object *elm_softkey_add(Evas_Object *parent);
   EAPI Elm_Softkey_Item *elm_softkey_button_add(Evas_Object *obj, Elm_Softkey_Type type, Evas_Object *icon, const char *label, void (*func)(void *data, Evas_Object *obj, void *event_info), const void *data);
   EAPI void         elm_softkey_button_del(Evas_Object *obj, Elm_Softkey_Type type);
   EAPI void         elm_softkey_button_show(Evas_Object *obj, Elm_Softkey_Type type);
   EAPI void         elm_softkey_button_hide(Evas_Object *obj, Elm_Softkey_Type type);

   EAPI Elm_Softkey_Item *elm_softkey_panel_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func)(void *data, Evas_Object *obj, void *event_info), const void *data);
   EAPI int          elm_softkey_panel_del(Evas_Object *obj);
   EAPI int          elm_softkey_panel_close(Evas_Object *obj);
   EAPI int          elm_softkey_panel_open(Evas_Object *obj);

   EAPI void         elm_softkey_item_icon_set(Elm_Softkey_Item *item, Evas_Object *icon);
   EAPI Evas_Object *elm_softkey_item_icon_get(Elm_Softkey_Item * item);
   EAPI void         elm_softkey_item_label_set(Elm_Softkey_Item *item, const char *label);
   EAPI const char  *elm_softkey_item_label_get(Elm_Softkey_Item * item);
   EAPI Eina_Bool    elm_softkey_item_disabled_get(Elm_Softkey_Item *item);
   EAPI void         elm_softkey_item_disabled_set(Elm_Softkey_Item *item, Eina_Bool disabled);
   EAPI void         elm_softkey_item_callback_set(Elm_Softkey_Item* item, void (*func)(void *data, Evas_Object *obj, void *event_info), const void *data );
   /* smart callback called: 
    * "clicked" - the user clicked the button, event_info: Softkey item
    * "press"   - the user press the button, event_info: Softkey item
    * "panel,show" - when the panel is shown
	* "panel,hide" - when the panel is hidden
    */
       
   /* gridbox */
   EAPI Evas_Object *elm_gridbox_add(Evas_Object *parent);
   EAPI void elm_gridbox_padding_set(Evas_Object *obj, Evas_Coord horizontal, Evas_Coord vertical);
   EAPI void elm_gridbox_item_size_set(Evas_Object *obj, Evas_Coord h_itemsize, Evas_Coord v_itemsize);
   EAPI void elm_gridbox_pack(Evas_Object *obj, Evas_Object *subobj);
   EAPI Eina_Bool elm_gridbox_unpack(Evas_Object *obj, Evas_Object *subobj);
   EAPI Eina_List *elm_gridbox_children_get(Evas_Object *obj);
   EAPI void elm_gridbox_homogenous_padding_set(Evas_Object *obj, Eina_Bool homogenous);


#ifdef __cplusplus
}
#endif

#endif
