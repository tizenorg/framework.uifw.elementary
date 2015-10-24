/**
 * @addtogroup Label
 *
 * @{
 */

/**
 * Slide mode of a label widget
 */
typedef enum
{
   ELM_LABEL_SLIDE_MODE_NONE = 0, /**< no slide effect */
   ELM_LABEL_SLIDE_MODE_AUTO, /**< slide only if the label area is bigger than the text width length */
   ELM_LABEL_SLIDE_MODE_ALWAYS /**< slide always */
} Elm_Label_Slide_Mode;

// TIZEN_ONLY(20150914): Add anchor tag feature and smart callbacks.
typedef struct _Elm_Label_Anchor_Info Elm_Label_Anchor_Info;
struct _Elm_Label_Anchor_Info
{
   char                 *name;
};
//

/**
 * @}
 */
