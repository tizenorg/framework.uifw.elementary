/**
 * @defgroup GLView GLView
 * @ingroup elm_widget_group
 *
 * @image html glview_inheritance_tree.png
 * @image latex glview_inheritance_tree.eps
 *
 * A GLView widget allows for simple GL rendering in elementary environment.
 * GLView hides all the complicated evas_gl details so that the user only
 * has to deal with registering a few callback functions for rendering
 * to a surface using OpenGL APIs.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref GLView:
 * - @c "focused" - when glview has received focus.
 * - @c "unfocused" - when glview has lost focus.
 * - @c "language,changed" - the program's language changed
 *
 * @section ELM_GLVIEW_RELATED_FEATURES Related Features
 * This API is related with the following feature:\n
 * - http://tizen.org/feature/opengles.version.1_1
 * - http://tizen.org/feature/opengles.version.2_0
 * - http://tizen.org/feature/opengles.version.3_0
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/getting-started/native-application/understanding-tizen-programming/application-filtering"><b>Feature List</b>.</a>
 *
 */

/**
 * @ingroup GLView
 */

#include "elm_glview_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_glview_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_glview_legacy.h"
#endif
/**
 * @}
 */
