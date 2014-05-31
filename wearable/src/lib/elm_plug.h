/**
 * @defgroup Plug Plug
 * @ingroup Elementary
 *
 * @image html plug_inheritance_tree.png
 * @image latex plug_inheritance_tree.eps
 *
 * An object that allows one to show an image which other process created.
 * It can be used anywhere like any other elementary widget.
 *
 */

/**
 * @addtogroup Plug
 * @{
 */

/**
 * @typedef Elm_Plug_Message
 *
 * Structure holding the message
 * which elm plug received from ecore evas
 *
 */
struct _Elm_Plug_Message
{
   int msg_domain;
   int msg_id;
   void *data;
   int size;
};

typedef struct _Elm_Plug_Message Elm_Plug_Message;

/**
 * Add a new plug image to the parent.
 *
 * @param parent The parent object
 * @return The new plug image object or NULL if it cannot be created
 *
 * @ingroup Plug
 */
EAPI Evas_Object    *elm_plug_add(Evas_Object *parent);

/**
 * Connect a plug widget to service provided by socket image.
 *
 * @param obj The Evas_Object where the new image object will live.
 * @param svcname The service name to connect to set up by the socket.
 * @param svcnum The service number to connect to (set up by socket).
 * @param svcsys Boolean to set if the service is a system one or not (set up by socket).
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @ingroup Plug
 */
EAPI Eina_Bool       elm_plug_connect(Evas_Object *obj, const char *svcname, int svcnum, Eina_Bool svcsys);

/**
 * Get the basic Evas_Image object from this object (widget).
 *
 * @param obj The image object to get the inlined image from
 * @return The inlined image object, or NULL if none exists
 *
 * This function allows one to get the underlying @c Evas_Object of type
 * Image from this elementary widget. It can be useful to do things like get
 * the pixel data, save the image to a file, etc.
 *
 * @note Be careful to not manipulate it, as it is under control of
 * elementary.
 *
 * @ingroup Plug
 */
EAPI Evas_Object    *elm_plug_image_object_get(const Evas_Object *obj);

/**
 * Send message to plug widget's socket
 *
 * @param obj The plug object to send message
 * @param msg_domain The domain of message
 * @param msg_id The id of message
 * @param data The data of message
 * @param size The size of message data
 *
 * @warning Support for this depends on the underlying windowing system.
 * @since 1.8.0
 *
 */
EAPI Eina_Bool       elm_plug_msg_send(Evas_Object *obj, int msg_domain, int msg_id, void *data, int size); 

/**
 * @}
 */
