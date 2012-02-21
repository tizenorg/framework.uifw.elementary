/**
 * @addtogroup Actionslider Actionslider
 *
 * @image html img/widget/actionslider/preview-00.png
 * @image latex img/widget/actionslider/preview-00.eps
 *
 * An actionslider is a switcher for 2 or 3 labels with customizable magnet
 * properties. The user drags and releases the indicator, to choose a label.
 *
 * Labels occupy the following positions.
 * a. Left
 * b. Right
 * c. Center
 *
 * Positions can be enabled or disabled.
 *
 * Magnets can be set on the above positions.
 *
 * When the indicator is released, it will move to its nearest "enabled and magnetized" position.
 *
 * @note By default all positions are set as enabled.
 *
 * Signals that you can add callbacks for are:
 *
 * "selected" - when user selects an enabled position (the label is passed as
 *              event info)".
 * "pos_changed" - when the indicator reaches any of the positions("left",
 *                 "right" or "center").
 *
 * Default text parts of the actionslider widget that you can use for are:
 * @li "indicator" - A indicator label of the actionslider
 * @li "left" - A left label of the actionslider
 * @li "right" - A right label of the actionslider
 * @li "center" - A center label of the actionslider
 *
 * Supported elm_object common APIs.
 * @li elm_object_part_text_set
 * @li elm_object_part_text_get
 * 
 * See an example of actionslider usage @ref actionslider_example_page "here"
 * @{
 */
typedef enum
{
   ELM_ACTIONSLIDER_NONE = 0,
   ELM_ACTIONSLIDER_LEFT = 1 << 0,
   ELM_ACTIONSLIDER_CENTER = 1 << 1,
   ELM_ACTIONSLIDER_RIGHT = 1 << 2,
   ELM_ACTIONSLIDER_ALL = (1 << 3) - 1
} Elm_Actionslider_Pos;

/**
 * Add a new actionslider to the parent.
 *
 * @param parent The parent object
 * @return The new actionslider object or NULL if it cannot be created
 */
EAPI Evas_Object                *elm_actionslider_add(Evas_Object *parent);

/**
 * Get actionslider selected label.
 *
 * @param obj The actionslider object
 * @return The selected label
 */
EAPI const char                 *elm_actionslider_selected_label_get(const Evas_Object *obj);

/**
 * Set actionslider indicator position.
 *
 * @param obj The actionslider object.
 * @param pos The position of the indicator.
 */
EAPI void                        elm_actionslider_indicator_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider indicator position.
 *
 * @param obj The actionslider object.
 * @return The position of the indicator.
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_indicator_pos_get(const Evas_Object *obj);

/**
 * Set actionslider magnet position. To make multiple positions magnets @c or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT)
 *
 * @param obj The actionslider object.
 * @param pos Bit mask indicating the magnet positions.
 */
EAPI void                        elm_actionslider_magnet_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider magnet position.
 *
 * @param obj The actionslider object.
 * @return The positions with magnet property.
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_magnet_pos_get(const Evas_Object *obj);

/**
 * Set actionslider enabled position. To set multiple positions as enabled @c or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT).
 *
 * @note All the positions are enabled by default.
 *
 * @param obj The actionslider object.
 * @param pos Bit mask indicating the enabled positions.
 */
EAPI void                        elm_actionslider_enabled_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider enabled position.
 *
 * @param obj The actionslider object.
 * @return The enabled positions.
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_enabled_pos_get(const Evas_Object *obj);

/**
 * Hold actionslider object movement.
 *
 * @param[in] obj The actionslider object
 * @param[in] flag Actionslider hold/release
 * (EINA_TURE = hold/EIN_FALSE = release)
 *
 * @ingroup Actionslider
 */
EAPI void   elm_actionslider_hold(Evas_Object *obj, Eina_Bool flag);


/*
 *  Deprecated APIs
 */

typedef enum
  {
     ELM_ACTIONSLIDER_LABEL_LEFT,
     ELM_ACTIONSLIDER_LABEL_RIGHT,
     ELM_ACTIONSLIDER_LABEL_CENTER,
     ELM_ACTIONSLIDER_LABEL_BUTTON
  } Elm_Actionslider_Label_Pos;

/**
 * @}
 */