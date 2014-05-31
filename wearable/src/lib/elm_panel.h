/**
 * @defgroup Panel Panel
 * @ingroup Elementary
 *
 * @image html panel_inheritance_tree.png
 * @image latex panel_inheritance_tree.eps
 *
 * @image html img/widget/panel/preview-00.png
 * @image latex img/widget/panel/preview-00.eps
 *
 * @brief A panel is an animated side-panel that contains a
 * sub-object. It can be expanded or contracted by clicking the
 * button on its edge.
 *
 * Orientations are as follows:
 * @li @c ELM_PANEL_ORIENT_TOP
 * @li @c ELM_PANEL_ORIENT_LEFT
 * @li @c ELM_PANEL_ORIENT_RIGHT
 * @li @c ELM_PANEL_ORIENT_BOTTOM
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for panel objects (@since 1.8).
 *
 * Default content parts of the panel widget that you can use are:
 * @li @c "default" - A content of the panel
 *
 * This widget emits the following signals, besides the ones sent from
 * @li @c "scroll" - the content has been scrolled (moved)
 *
 * @ref tutorial_panel shows one way to use this widget.
 * @{
 */
typedef enum
{
   ELM_PANEL_ORIENT_TOP, /**< Panel (dis)appears from the top */
   ELM_PANEL_ORIENT_BOTTOM, /**< Panel (dis)appears from the bottom */
   ELM_PANEL_ORIENT_LEFT, /**< Panel (dis)appears from the left */
   ELM_PANEL_ORIENT_RIGHT, /**< Panel (dis)appears from the right */
} Elm_Panel_Orient;

/**
 * @typedef Elm_Panel_Scroll_Info
 *
 * When panel content is scrolled, if the panel object is scrollable,
 * This information will be passed by the @c event_info argument in the
 * @c "scroll" smart callback function.
 */
typedef struct _Elm_Panel_Scroll_Info
{
   double rel_x;   /** <content scrolled position (0.0 ~ 1.0) in the panel */
   double rel_y;   /** <content scrolled position (0.0 ~ 1.0) in the panel */

} Elm_Panel_Scroll_Info;

/**
 * @brief Adds a panel object
 *
 * @param parent The parent object
 *
 * @return The panel object, or NULL on failure
 *
 * @ingroup Panel
 */
EAPI Evas_Object                 *elm_panel_add(Evas_Object *parent);

/**
 * @brief Sets the orientation of the panel
 *
 * @param obj The panel object
 * @param orient The panel orientation. Can be one of the following:
 * @li ELM_PANEL_ORIENT_TOP
 * @li ELM_PANEL_ORIENT_LEFT
 * @li ELM_PANEL_ORIENT_RIGHT
 *
 * Sets from where the panel will (dis)appear.
 *
 * @ingroup Panel
 */
EAPI void                         elm_panel_orient_set(Evas_Object *obj, Elm_Panel_Orient orient);

/**
 * @brief Get the orientation of the panel.
 *
 * @param obj The panel object
 * @return The Elm_Panel_Orient, or ELM_PANEL_ORIENT_LEFT on failure.
 *
 * @ingroup Panel
 */
EAPI Elm_Panel_Orient             elm_panel_orient_get(const Evas_Object *obj);

/**
 * @brief Set the state of the panel.
 *
 * @param obj The panel object
 * @param hidden If true, the panel will run the animation to disappear.
 *
 * @ingroup Panel
 */
EAPI void                         elm_panel_hidden_set(Evas_Object *obj, Eina_Bool hidden);

/**
 * @brief Get the state of the panel.
 *
 * @param obj The panel object
 * @return EINA_TRUE if it is hidden state
 *
 * @ingroup Panel
 */
EAPI Eina_Bool                    elm_panel_hidden_get(const Evas_Object *obj);

/**
 * @brief Toggle the hidden state of the panel from code
 *
 * @param obj The panel object
 *
 * @ingroup Panel
 */
EAPI void                         elm_panel_toggle(Evas_Object *obj);

/**
 * Enable or disable scrolling in panel
 *
 * Normally the panel is not scrollable unless you enable it with this call.
 *
 * @param obj The panel object
 * @param scrollable EINA_TRUE if it is to be scrollable, EINA_FALSE otherwise
 *
 * @ingroup Panel
 */
EAPI void                         elm_panel_scrollable_set(Evas_Object *obj, Eina_Bool scrollable);

/**
 * Get the scrollable state of the panel
 *
 * Normally the panel is not scrollable. This gets the scrollable state
 * of the panel.
 *
 * @param obj The panel object
 * @return The scrollable state
 *
 * @see elm_panel_scrollable_set()
 *
 * @ingroup Panel
 */
EAPI Eina_Bool          elm_panel_scrollable_get(const Evas_Object *obj);

/**
 * @}
 */
