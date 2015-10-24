/**
 * @brief Set a text of an object
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary object
 * @param[in] part The text part name to set (NULL for the default part)
 * @param[in] text The new text of the part
 *
 * @remark Elementary objects may have many text parts (e.g. Action Slider)
 *
 * @ingroup General
 */
EAPI void                         elm_object_part_text_set(Evas_Object *obj, const char *part, const char *text);

/**
 * @def elm_object_text_set
 * @brief Convenient macro for elm_object_part_text_set
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_part_text_set
 * @ingroup General
 */
#define elm_object_text_set(obj, text) elm_object_part_text_set((obj), NULL, (text))

/**
 * @brief Get a text of an object
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary object
 * @param[in] part The text part name to get (NULL for the default part)
 * @return text of the part or NULL for any error
 *
 * @remark Elementary objects may have many text parts (e.g. Action Slider)
 *
 * @ingroup General
 */
EAPI const char                  *elm_object_part_text_get(const Evas_Object *obj, const char *part);

/**
 * @def elm_object_text_get
 * @brief Convenient macro for elm_object_part_text_get
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_part_text_get
 * @ingroup General
 */
#define elm_object_text_get(obj) elm_object_part_text_get((obj), NULL)

/**
 * @brief Set the text for an object's part, marking it as translatable.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark The string to set as @p text must be the original one. Do not pass the
 * return of @c gettext() here. Elementary will translate the string
 * internally and set it on the object using elm_object_part_text_set(),
 * also storing the original string so that it can be automatically
 * translated when the language is changed with elm_language_set().
 *
 * @remark The @p domain will be stored along to find the translation in the
 * correct catalog. It can be NULL, in which case it will use whatever
 * domain was set by the application with @c textdomain(). This is useful
 * in case you are building a library on top of Elementary that will have
 * its own translatable strings, that should not be mixed with those of
 * programs using the library.
 *
 * @param[in] obj The object containing the text part
 * @param[in] part The name of the part to set
 * @param[in] domain The translation domain to use
 * @param[in] text The original, non-translated text to set
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI void      elm_object_domain_translatable_part_text_set(Evas_Object *obj, const char *part, const char *domain, const char *text);

/**
 * @def elm_object_domain_translatable_text_set
 * @brief Convenient macro for elm_object_domain_translatable_part_text_set.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_domain_translatable_part_text_set
 * @ingroup General
 */
#define elm_object_domain_translatable_text_set(obj, domain, text) elm_object_domain_translatable_part_text_set((obj), NULL, (domain), (text))

/**
 * @def  elm_object_translatable_text_set
 * @brief Convenient macro for  elm_object_domain_translatable_part_text_set
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_domain_translatable_part_text_set
 * @ingroup General
 */
#define elm_object_translatable_text_set(obj, text)                elm_object_domain_translatable_part_text_set((obj), NULL, NULL, (text))

/**
 * @def  elm_object_translatable_part_text_set
 * @brief Convenient macro for  elm_object_domain_translatable_part_text_set
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_domain_translatable_part_text_set
 * @ingroup General
 */
#define elm_object_translatable_part_text_set(obj, part, text)     elm_object_domain_translatable_part_text_set((obj), (part), NULL, (text))

/**
 * @brief Get the original string set as translatable for an object
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark When setting translated strings, the function elm_object_part_text_get()
 * will return the translation returned by @c gettext(). To get the
 * original string use this function.
 *
 * @param[in] obj The object
 * @param[in] part The name of the part that was set
 *
 * @return The original, untranslated string
 *
 * @see elm_object_translatable_part_text_set()
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI const char *elm_object_translatable_part_text_get(const Evas_Object *obj, const char *part);

/**
 * @def elm_object_translatable_text_get
 * @brief Convenient macro for elm_object_translatable_part_text_get
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_translatable_part_text_get
 * @ingroup General
 */
#define elm_object_translatable_text_get(obj) elm_object_translatable_part_text_get((obj), NULL)

/**
 * @brief Mark the part text to be translatable or not.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark Once you mark the part text to be translatable, the text will be translated
 * internally regardless of elm_object_part_text_set() and
 * elm_object_domain_translatable_part_text_set(). In other case, if you set the
 * Elementary policy that all text will be translatable in default, you can set
 * the part text to not be translated by calling this API.
 *
 * @param[in] obj The object containing the text part
 * @param[in] part The part name of the translatable text
 * @param[in] domain The translation domain to use
 * @param[in] translatable @c EINA_TRUE, the part text will be translated
 *        internally. @c EINA_FALSE, otherwise.
 *
 * @see elm_object_domain_translatable_part_text_set()
 * @see elm_object_part_text_set()
 * @see elm_policy()
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI void elm_object_domain_part_text_translatable_set(Evas_Object *obj, const char *part, const char *domain, Eina_Bool translatable);

/**
 * @def elm_object_part_text_translatable_set
 * @brief Convenient macro for elm_object_domain_part_text_translatable_set
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_domain_part_text_translatable_set
 * @ingroup General
 */
#define elm_object_part_text_translatable_set(obj, part, translatable) elm_object_domain_part_text_translatable_set((obj), (part), NULL, (translatable))

/**
 * @def elm_object_domain_text_translatable_set
 * @brief Convenient macro for elm_object_domain_part_text_translatable_set
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_domain_part_text_translatable_set
 * @ingroup General
 */
#define elm_object_domain_text_translatable_set(obj, domain, translatable) elm_object_domain_part_text_translatable_set((obj), NULL, (domain), (translatable))

/**
 * @brief Set the content on part of a given container widget
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary container widget
 * @param[in] part The container's part name to set (some might accept
 *        @c NULL for the default part)
 * @param[in] content The new content for that part
 *
 * @remark All widgets deriving from the @ref elm-container-class may hold
 * child objects as content at given parts. This sets new content to
 * a given part. If any object was already set as a content object in
 * the same part, the previous object will be deleted automatically
 * with this call. If the @p content is NULL, this call will just delete the
 * previous object. If the If you wish to preserve it, issue
 * elm_object_part_content_unset() on it first.
 *
 * @see elm_object_part_content_get()
 *
 * @ingroup General
 */
EAPI void                         elm_object_part_content_set(Evas_Object *obj, const char *part, Evas_Object *content);

/**
 * @def elm_object_content_set
 * @brief Convenient macro for elm_object_part_content_set
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_part_content_set
 * @ingroup General
 */
#define elm_object_content_set(obj, content) elm_object_part_content_set((obj), NULL, (content))

/**
 * @brief Get the content on a part of a given container widget
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary container widget
 * @param[in] part The container's part name to get (some might accept
 *        @c NULL for the default part)
 * @return content of the object at the given part or @c NULL, on
 *         errors
 *
 * @see elm_object_part_content_set() for more details
 *
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_part_content_get(const Evas_Object *obj, const char *part);

/**
 * @def elm_object_content_get
 * @brief Convenient macro for elm_object_part_content_get
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_part_content_get
 * @ingroup General
 */
#define elm_object_content_get(obj) elm_object_part_content_get((obj), NULL)

/**
 * @brief Unset the content on a part of a given container widget
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary container widget
 * @param[in] part The container's part name to unset (some might accept
 *        @c NULL for the default part)
 * @return content of the object at the given part or @c NULL, on
 *         errors
 *
 * @see elm_object_part_content_set() for more details
 *
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_part_content_unset(Evas_Object *obj, const char *part);

/**
 * @def elm_object_content_unset
 * @brief Convenient macro for elm_object_part_content_unset
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @see elm_object_part_content_unset
 * @ingroup General
 */
#define elm_object_content_unset(obj) elm_object_part_content_unset((obj), NULL)

/**
 * @brief Set the text to read out when in accessibility mode
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The object which is to be described
 * @param[in] txt The text that describes the widget to people with poor or no vision
 *
 * @ingroup General
 */
EAPI void                         elm_object_access_info_set(Evas_Object *obj, const char *txt);

/**
 * @brief Get a named object from the children
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The parent object whose children to look at
 * @param[in] name The name of the child to find
 * @param[in] recurse Set to the maximum number of levels to recurse (0 == none, 1 is only look at 1 level of children etc.)
 * @return The found object of that name, or NULL if none is found
 *
 * @remark This function searches the children (or recursively children of
 * children and so on) of the given @p obj object looking for a child with
 * the name of @p name. If the child is found the object is returned, or
 * NULL is returned. You can set the name of an object with
 * evas_object_name_set(). If the name is not unique within the child
 * objects (or the tree is @p recurse is greater than 0) then it is
 * undefined as to which child of that name is returned, so ensure the name
 * is unique amongst children. If recurse is set to -1 it will recurse
 * without limit.
 *
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_name_find(const Evas_Object *obj, const char *name, int recurse);

/**
 * @defgroup Styles Styles
 * @ingroup elm_infra_group
 *
 * Widgets can have different styles of look. These generic API's
 * set styles of widgets, if they support them (and if the theme(s)
 * do).
 *
 */

/**
 * @brief Set the style to used by a given widget
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary widget to style
 * @param[in] style The name of the style to use on it
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @remark This sets the style (by name) that will define the appearance of a
 * widget. Styles vary from widget to widget and may also be defined
 * by other themes by means of extensions and overlays.
 *
 * @see elm_theme_extension_add()
 * @see elm_theme_extension_del()
 * @see elm_theme_overlay_add()
 * @see elm_theme_overlay_del()
 *
 * @ingroup Styles
 */
EAPI Eina_Bool    elm_object_style_set(Evas_Object *obj, const char *style);

/**
 * @brief Get the style used by the widget
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark This gets the style being used for that widget. Note that the string
 * pointer is only valid as long as the object is valid and the style doesn't
 * change.
 *
 * @param[in] obj The Elementary widget to query for its style
 * @return The style name used
 *
 * @see elm_object_style_set()
 *
 * @ingroup Styles
 */
EAPI const char  *elm_object_style_get(const Evas_Object *obj);

/**
 * @brief Set the disabled state of an Elementary object.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary object to operate on
 * @param[in] disabled The state to put in in: @c EINA_TRUE for
 *        disabled, @c EINA_FALSE for enabled
 *
 * @remark Elementary objects can be @b disabled, in which state they won't
 * receive input and, in general, will be themed differently from
 * their normal state, usually greyed out. Useful for contexts
 * where you don't want your users to interact with some of the
 * parts of you interface.
 *
 * @remark This sets the state for the widget, either disabling it or
 * enabling it back.
 *
 * @ingroup General
 */
EAPI void         elm_object_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * @brief Get the disabled state of an Elementary object.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary object to operate on
 * @return @c EINA_TRUE, if the widget is disabled, @c EINA_FALSE
 *            if it's enabled (or on errors)
 *
 * @remark This gets the state of the widget, which might be enabled or disabled.
 *
 * @ingroup General
 */
EAPI Eina_Bool    elm_object_disabled_get(const Evas_Object *obj);

/**
 * @defgroup WidgetNavigation Widget Tree Navigation
 * @ingroup elm_infra_group
 *
 * These functions provide checks for if a Evas_Object is an Elementary widget,
 * the possibility of getting a widget's parent, top level parent and getting a
 * string representation of a widget's type.
 */

/**
 * @brief Check if the given Evas Object is an Elementary widget.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj the object to query.
 * @return @c EINA_TRUE if it is an elementary widget variant,
 *         @c EINA_FALSE otherwise
 * @ingroup WidgetNavigation
 */
EAPI Eina_Bool    elm_object_widget_check(const Evas_Object *obj);

/**
 * @brief Get the first parent of the given object that is an Elementary
 * widget.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj the Elementary object to query parent from.
 * @return the parent object that is an Elementary widget, or @c
 *         NULL, if it was not found.
 *
 * @remark Use this to query for an object's parent widget.
 *
 * @remark Most of Elementary users wouldn't be mixing non-Elementary
 * smart objects in the objects tree of an application, as this is
 * an advanced usage of Elementary with Evas. So, except for the
 * application's window, which is the root of that tree, all other
 * objects would have valid Elementary widget parents.
 *
 * @ingroup WidgetNavigation
 */
EAPI Evas_Object *elm_object_parent_widget_get(const Evas_Object *obj);

/**
 * @brief Get the top level parent of an Elementary widget.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The object to query.
 * @return The top level Elementary widget, or @c NULL if parent cannot be
 * found.
 * @ingroup WidgetNavigation
 */
EAPI Evas_Object *elm_object_top_widget_get(const Evas_Object *obj);

/**
 * @brief Get the string that represents this Elementary widget.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark Elementary is weird and exposes itself as a single
 *       Evas_Object_Smart_Class of type "elm_widget", so
 *       evas_object_type_get() always return that, making debug and
 *       language bindings hard. This function tries to mitigate this
 *       problem, but the solution is to change Elementary to use
 *       proper inheritance.
 *
 * @param[in] obj the object to query.
 * @return Elementary widget name, or @c NULL if not a valid widget.
 * @ingroup WidgetNavigation
 */
EAPI const char  *elm_object_widget_type_get(const Evas_Object *obj);

/**
 * @brief Send a signal to the widget edje object.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark This function sends a signal to the edje object of the obj. An
 * edje program can respond to a signal by specifying matching
 * 'signal' and 'source' fields.
 *
 * @param[in] obj The object
 * @param[in] emission The signal's name.
 * @param[in] source The signal's source.
 * @ingroup General
 */
EAPI void         elm_object_signal_emit(Evas_Object *obj, const char *emission, const char *source);

/**
 * @brief Add a callback for a signal emitted by widget edje object.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark This function connects a callback function to a signal emitted by the
 * edje object of the obj.
 * Globs can occur in either the emission or source name.
 *
 * @param[in] obj The object
 * @param[in] emission The signal's name.
 * @param[in] source The signal's source.
 * @param[in] func The callback function to be executed when the signal is
 * emitted.
 * @param[in] data A pointer to data to pass to the callback function.
 * @ingroup General
 */
EAPI void         elm_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Remove a signal-triggered callback from a widget edje object.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The object handle
 * @param[in] emission The signal's name.
 * @param[in] source The signal's source.
 * @param[in] func The callback function to be executed when the signal is
 * emitted.
 * @return The data pointer of the signal callback or @c NULL, on
 * errors.
 *
 * @remark This function removes the @b last callback, previously attached to
 * a signal emitted by an underlying Edje object of @a obj, whose
 * parameters @a emission, @a source and @c func match exactly with
 * those passed to a previous call to
 * elm_object_signal_callback_add(). The data pointer that was passed
 * to this call will be returned.
 *
 * @ingroup General
 */
EAPI void        *elm_object_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);

/**
 * @brief Add a callback for input events (key up, key down, mouse wheel)
 * on a given Elementary widget
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The widget to add an event callback on
 * @param[in] func The callback function to be executed when the event
 * happens
 * @param[in] data Data to pass in to @p func
 *
 * @remark Every widget in an Elementary interface set to receive focus,
 * with elm_object_focus_allow_set(), will propagate @b all of its
 * key up, key down and mouse wheel input events up to its parent
 * object, and so on. All of the focusable ones in this chain which
 * had an event callback set, with this call, will be able to treat
 * those events. There are two ways of making the propagation of
 * these event upwards in the tree of widgets to @b cease:
 * - Just return @c EINA_TRUE on @p func. @c EINA_FALSE will mean
 *   the event was @b not processed, so the propagation will go on.
 * - The @p event_info pointer passed to @p func will contain the
 *   event's structure and, if you OR its @c event_flags inner
 *   value to @c EVAS_EVENT_FLAG_ON_HOLD, you're telling Elementary
 *   one has already handled it, thus killing the event's
 *   propagation, too.
 *
 * @remark Your event callback will be issued on those events taking
 * place only if no other child widget of @p obj has consumed the
 * event already.
 *
 * @remark Not to be confused with @c
 * evas_object_event_callback_add(), which will add event callbacks
 * per type on general Evas objects (no event propagation
 * infrastructure taken in account).
 *
 * @remark Not to be confused with @c
 * elm_object_signal_callback_add(), which will add callbacks to @b
 * signals coming from a widget's theme, not input events.
 *
 * @remark Not to be confused with @c
 * edje_object_signal_callback_add(), which does the same as
 * elm_object_signal_callback_add(), but directly on an Edje
 * object.
 *
 * @remark Not to be confused with @c
 * evas_object_smart_callback_add(), which adds callbacks to smart
 * objects' <b>smart events</b>, and not input events.
 *
 * @see elm_object_event_callback_del()
 *
 * @ingroup General
 */
EAPI void         elm_object_event_callback_add(Evas_Object *obj, Elm_Event_Cb func, const void *data);

/**
 * @brief Remove an event callback from a widget.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark This function removes a callback, previously attached to event emission
 * by the @p obj.
 * The parameters func and data must match exactly those passed to
 * a previous call to elm_object_event_callback_add(). The data pointer that
 * was passed to this call will be returned.
 *
 * @param[in] obj The object
 * @param[in] func The callback function to be executed when the event is
 * emitted.
 * @param[in] data Data to pass in to the callback function.
 * @return The data pointer
 * @ingroup General
 */
EAPI void        *elm_object_event_callback_del(Evas_Object *obj, Elm_Event_Cb func, const void *data);

/**
 * @brief Disable the orientation mode of a given widget.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @remark Orientation Mode is used for widgets to change it's styles or to send signals
 * whenever it's window degree is changed. If the orientation mode is enabled
 * and the widget has different looks and styles for the window degree(0, 90,
 * 180, 270), it will apply a style that is readied for the current degree,
 * otherwise, it will send signals to it's own edje to change it's states if
 * the style doesn't be readied.
 *
 * @param[in] obj The Elementary object to operate on orientation mode.
 * @param[in] disabled The state to put in in: @c EINA_TRUE for disabled,
 *        @c EINA_FALSE for enabled.
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI void        elm_object_orientation_mode_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * @brief Get the orientation mode of a given widget.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] obj The Elementary widget to query for its orientation mode.
 * @return @c EINA_TRUE, if the orientation mode is disabled, @c EINA_FALSE
 *            if the orientation mode is enabled (or on errors)
 * @see elm_object_orientation_mode_disabled_set()
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI Eina_Bool   elm_object_orientation_mode_disabled_get(const Evas_Object *obj);

/**
 * @internal
 *
 * Set the accessibility highlight of a given widget.
 *
 * This function sets accessiblity highlight frame of given object
 *
 * @param obj The Elementary widget to set highlight frame.
 * @param visible Set to visible state to frame: @c EINA_TRUE for make frame visible,
          @c EINA_FALSE to hide frame.
 *
 * @ingroup General
 */
EAPI void elm_object_accessibility_highlight_set(Evas_Object *obj, Eina_Bool visible);

// TIZEN_ONLY(20150705): Genlist item align feature
/**
 * @brief Sets the scroll item align enable.
 * @remarks Tizen only feature.
 *
 *
 * @remarks If the object's scroll item align is enabled, the nearest
 *          item to the end of the scroll will be located according to
 *          its vertical align.
 *
 * @remarks Currently implemented only Genlist.
 *
 * @since_tizen 2.3.1
 *
 * @param[in] obj The object handle
 * @param[in] enabled The scroll item align state
 *
 * @see elm_object_scroll_item_align_enabled_get()
 */
EAPI void elm_object_scroll_item_align_enabled_set(Evas_Object *obj, Eina_Bool scroll_item_align_enable);

/**
 * @brief Sets the scroll item align enable.
 * @remarks Tizen only feature.
 *
 *
 * @remarks If @c EINA_TRUE, the nearest item to the end of the scroll will be
 *          aligned according to its vertical align. If @c EINA_FALSE, the scroll
 *          item align is disabled.
 *
 * @remarks Currently implemented only Genlist.
 *
 * @since_tizen 2.3.1
 *
 * @param[in] obj The object handle
 * @return The scroll item align state.
 *
 * @see elm_object_scroll_item_align_enabled_set()
 */
EAPI Eina_Bool elm_object_scroll_item_align_enabled_get(const Evas_Object *obj);

/**
 * @brief Sets the scroll item vertical align.
 * @remarks Tizen only feature.
 *
 *
 * @remarks This API sets the scroll item vertical align. The scroll item
 *          vertical align is referred when the object's scroll item align
 *          is enabled. When the widget is scrolled, the nearest item to the
 *          end of the scroll will be located according to the given alignment.
 *
 * @remarks Currently implemented only Genlist.
 *
 * @since_tizen 2.3.1
 *
 * @param[in] obj The object handle
 * @param[in] vertical align The position name.
 *
 * @see elm_object_scroll_item_valign_get()
 */
EAPI void elm_object_scroll_item_valign_set(Evas_Object *obj, char *scroll_item_valign);

/**
 * @brief Sets the scroll item vertical align.
 * @remarks Tizen only feature.
 *
 *
 * @remarks This API gets the scroll item vertical align. The scroll item
 *          vertical align is referred when the object's scroll item align
 *          is enabled. When the widget is scrolled, the nearest item to the
 *          end of the scroll will be located according to the given alignment.
 *
 * @remarks Currently implemented only Genlist.
 *
 * @since_tizen 2.3.1
 *
 * @param[in] obj The object handle
 * @return The scroll item vertical align name.
 *
 * @see elm_object_scroll_item_valign_set()
 */
EAPI const char* elm_object_scroll_item_valign_get(const Evas_Object *obj);
//
