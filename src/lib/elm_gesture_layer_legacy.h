/**
 * @brief Call this function to construct a new gesture-layer object.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] parent The gesture layer's parent widget.
 *
 * @return A new gesture layer object.
 *
 * @remark This does not activate the gesture layer. You have to
 * call elm_gesture_layer_attach() in order to 'activate' gesture-layer.
 *
 * @ingroup Elm_Gesture_Layer
 */
EAPI Evas_Object *elm_gesture_layer_add(Evas_Object *parent);

/**
 * @internal
 *
 * This function adds a callback called during Tap + Long Tap sequence.
 *
 * @param state state for the callback to add.
 * @param cb callback pointer
 * @param data user data for the callback.
 *
 * The callbacks will be called as followed:
 * - start cbs on single tap start
 * - move cbs on long press move
 * - end cbs on long press end
 * - abort cbs whenever in the sequence. The event info will be NULL, because it
 *   can be triggered from multiple events (timer expired, abort single/long taps).
 *
 * You can remove the callbacks by using elm_gesture_layer_tap_longpress_cb_del.
 *
 * @see elm_gesture_layer_tap_longpress_cb_del
 * @since 1.8
 * @ingroup Elm_Gesture_Layer
 */
EAPI void elm_gesture_layer_tap_longpress_cb_add(Evas_Object *obj, Elm_Gesture_State state, Elm_Gesture_Event_Cb cb, void *data);

/**
 * @internal
 *
 * This function removes a callback called during Tap + Long Tap sequence.
 *
 * @param state state for the callback to add.
 * @param cb callback pointer
 * @param data user data for the callback.
 *
 * The internal data used for the sequence will be freed ONLY when all the
 * callbacks added via elm_gesture_layer_tap_longpress_cb_add are removed by
 * this function.
 *
 * @see elm_gesture_layer_tap_longpress_cb_add
 * @since 1.8
 * @ingroup Elm_Gesture_Layer
 */
EAPI void elm_gesture_layer_tap_longpress_cb_del(Evas_Object *obj, Elm_Gesture_State state, Elm_Gesture_Event_Cb cb, void *data);

#include "elm_gesture_layer.eo.legacy.h"
