/**
 * @addtogroup Colorselector
 *
 * @{
 */

typedef struct _Elm_Color_RGBA Elm_Color_RGBA;
struct _Elm_Color_RGBA
{
   unsigned int r;
   unsigned int g;
   unsigned int b;
   unsigned int a;
};


typedef struct _Elm_Custom_Palette Elm_Custom_Palette;
struct _Elm_Custom_Palette
{
   const char *palette_name;
   Eina_List  *color_list;
};

/**
 * @enum Elm_Colorselector_Mode
 *
 * Different modes supported by Colorselector
 *
 * @see elm_colorselector_mode_set()
 * @see elm_colorselector_mode_get()
 */
typedef enum
{
   ELM_COLORSELECTOR_PALETTE = 0, /**< only color palette is displayed */
   ELM_COLORSELECTOR_COMPONENTS, /**< only color selector is displayed */
   ELM_COLORSELECTOR_BOTH, /**< Both Palette and selector is displayed, default */
   ELM_COLORSELECTOR_PICKER, /**< only color picker is displayed */
   ELM_COLORSELECTOR_PLANE, /**< @DEPRECATED: This mode is not supported. If you use this, nothing will be shown */
   ELM_COLORSELECTOR_PALETTE_PLANE, /**< @DEPRECATED: This mode is not supported. If you use this, it will be shown same with ELM_COLORSELECTOR_PALETTE mode */
   ELM_COLORSELECTOR_ALL /**< all possible color selector is displayed */
} Elm_Colorselector_Mode;

/**
 * @brief Get Palette item's color.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] it The color palette item.
 * @param[out] r integer pointer for r-value of color
 * @param[out] g integer pointer for g-value of color
 * @param[out] b integer pointer for b-value of color
 * @param[out] a integer pointer for a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_item_color_get(const Elm_Object_Item *it, int *r, int *g, int *b, int *a);

/**
 * @brief Set palette item's color.
 *
 * @if MOBILE @since_tizen 2.3
 * @elseif WEARABLE @since_tizen 2.3.1
 * @endif
 *
 * @param[in] it The color palette item.
 * @param[in] r r-value of color
 * @param[in] g g-value of color
 * @param[in] b b-value of color
 * @param[in] a a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_item_color_set(Elm_Object_Item *it, int r, int g, int b, int a);

/**
 * @internal
 *
 * @brief Get the selected state of color palette item.
 *
 * @param[in] it The Colorpalette item
 * @return @c EINA_TRUE if the item is selected, @c EINA_FALSE otherwise.
 *
 * @since 1.9
 * @ingroup Colorselector
 */
EAPI Eina_Bool elm_colorselector_palette_item_selected_get(const Elm_Object_Item *it);

EAPI void elm_colorselector_palette_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * @}
 */
