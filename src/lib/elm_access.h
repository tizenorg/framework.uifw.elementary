/**
 * @defgroup Access Access
 * @ingroup Elementary
 *
 * TODO: description
 *
 */

#define ELM_ACCESS_TYPE          0    /* when reading out widget or item
                                       * this is read first */
#define ELM_ACCESS_INFO          1    /* next read is info - this is
                                       * normally label */
#define ELM_ACCESS_STATE         2    /* if there is a state (eg checkbox)
                                       * then read state out */
#define ELM_ACCESS_CONTEXT_INFO  3    /* to give contextual information */

typedef char *(*Elm_Access_Content_Cb)(void *data, Evas_Object *obj);
typedef void (*Elm_Access_Activate_Cb)(void *data, Evas_Object *part_obj, Elm_Object_Item *item);

/**
 * @brief Register evas object as an accessible object.
 * @since 1.8
 *
 * @param parent Accessibility parent object. this should be one of widget.
 * @param target Evas object to register as an accessible object.
 *
 * @ingroup Access
 */
EAPI void elm_access_object_register(Evas_Object *parent, Evas_Object *target);

/**
 * @brief Unregister accessible object.
 * @since 1.8
 *
 * @param obj Accessible object.
 *
 * @ingroup Access
 */
EAPI void elm_access_object_unregister(Evas_Object *target);

/**
 * @brief Set text to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 * @param text The text information that will be read
 *
 * @see elm_access_content_cb_set
 * @ingroup Access
 */
EAPI void elm_access_text_set(Evas_Object *obj, int type, const char *text);

/**
 * @brief Set text to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 *
 * @see elm_access_content_cb_set
 * @ingroup Access
 */
EAPI char * elm_access_text_get(Evas_Object *obj, int type);

/**
 * @brief Set content callback to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 * @param func The function to be called when the content is read
 * @param data The data pointer to be passed to @p func
 *
 * The type would be one of ELM_ACCESS_TYPE, ELM_ACCESS_INFO,
 * ELM_ACCESS_STATE, ELM_ACCESS_CONTEXT_INFO.
 *
 * In the case of button widget, the content of ELM_ACCESS_TYPE would be
 * "button". The label of button such as "ok", "cancel" is for ELM_ACCESS_INFO.
 * If the button is disabled, content of ELM_ACCESS_STATE would be "disabled".
 * And if there is contextual information, ELM_ACCESS_CONTEXT_INFO is used.
 *
 * @ingroup Access
 */
EAPI void elm_access_content_cb_set(Evas_Object *obj, int type, Elm_Access_Content_Cb func, const void *data);

/**
 * @brief Set activate callback to activate highlight object.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param func The function to be called when the activate gesture is detected
 * @param data The data pointer to be passed to @p func
 *
 * @ingroup Access
 */
EAPI void elm_access_activate_cb_set(Evas_Object *obj, Elm_Access_Activate_Cb func, void *data);

//TODO: remvoe below - use elm_access_text_set(); or elm_access_cb_set();
EINA_DEPRECATED EAPI void elm_access_external_info_set(Evas_Object *obj, const char *text);
EINA_DEPRECATED EAPI char * elm_access_external_info_get(const Evas_Object *obj);
