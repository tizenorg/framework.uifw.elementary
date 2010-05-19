#include <Elementary.h>
#include "elm_priv.h"
#include <Ecore.h>

/**
 * @addtogroup Colorpicker Colorpicker
 *
 * By using colorpicker, you can select a color.
 * Colorpicker made a color using HSV/HSB mode.
 */

#define BASE_STEP 360
#define HUE_STEP 360
#define SAT_STEP 127
#define LIG_STEP 255

typedef struct _Colorpicker_Data Colorpicker_Data;
struct _Colorpicker_Data
{
	Evas_Object *parent;
	Evas_Object *colorbar;
	Evas_Object *bar;
	Evas_Object *lbt;
	Evas_Object *rbt;
	Evas_Object *bg_rect;
	Evas_Object *arrow;
	Evas_Object *touch_area;

	int colorpicker_num;
	int button_state;
};

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data
{
	Evas_Object *e;
	Evas_Object *lay;
	Colorpicker_Data *cp[3];
	Evas_Coord _x, _y, _w, _h;
	int r, g, b;
	int er, eg, eb;
	int sr, sg, sb;
	double h, s, l;

	Ecore_Timer *lp_timer;
	Ecore_Timer *mv_timer;
};

typedef enum{
	BUTTON_RELEASED,
	L_BUTTON_PRESSED,
	R_BUTTON_PRESSED
} Button_State;

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void _colorpicker_object_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _colorpicker_object_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _colorpicker_object_show(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _colorpicker_object_hide(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _rgb_to_hsl(void *data);
static void _hsl_to_rgb(void *data);
static void _color_with_saturation(void *data);
static void _draw_rects(void *data, double x);
static void _arrow_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _colorbar_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static int _mv_timer(void *data);
static int _long_press_timer(void *data);
static void _left_button_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _right_button_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _left_button_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _right_button_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _add_colorbar(Evas_Object* obj);

static void
_del_hook(Evas_Object *obj)
{
	Widget_Data *wd = elm_widget_data_get(obj);
	int i;
	if (!wd) return;

	for( i=0 ; i<3 ; i++)
	{
		evas_object_del(wd->cp[i]->colorbar);
		wd->cp[i]->colorbar = NULL;
		evas_object_del(wd->cp[i]->bar);
		wd->cp[i]->bar = NULL;
		evas_object_del(wd->cp[i]->lbt);
		wd->cp[i]->lbt = NULL;
		evas_object_del(wd->cp[i]->rbt);
		wd->cp[i]->rbt = NULL;
		if(i==1){
			evas_object_del(wd->cp[i]->bg_rect);
			wd->cp[i]->bg_rect = NULL;
		}
		evas_object_del(wd->cp[i]->arrow);
		wd->cp[i]->arrow = NULL;
		evas_object_del(wd->cp[i]->touch_area);
		wd->cp[i]->touch_area = NULL;
	}

	if (wd->lay){
		evas_object_smart_member_del(wd->lay);
		evas_object_del(wd->lay);
		wd->lay = NULL;
	}
	free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
	Widget_Data *wd = elm_widget_data_get(obj);
	int i;
	if (!wd){
		return;
	}

	_elm_theme_object_set(obj, wd->lay, "colorpicker", "bg", elm_widget_style_get(obj));

	for( i=0 ; i<3 ; i++)
	{
		evas_object_del(wd->cp[i]->colorbar);
		wd->cp[i]->colorbar = NULL;
		evas_object_del(wd->cp[i]->bar);
		wd->cp[i]->bar = NULL;
		evas_object_del(wd->cp[i]->lbt);
		wd->cp[i]->lbt = NULL;
		evas_object_del(wd->cp[i]->rbt);
		wd->cp[i]->rbt = NULL;
		if(i==1){
			evas_object_del(wd->cp[i]->bg_rect);
			wd->cp[i]->bg_rect = NULL;
		}
		evas_object_del(wd->cp[i]->arrow);
		wd->cp[i]->arrow = NULL;
		evas_object_del(wd->cp[i]->touch_area);
		wd->cp[i]->touch_area = NULL;
	}

	_add_colorbar(obj);
//	_sizing_eval(obj);
    // FIXME : add more codes
    //

}


static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
	Widget_Data *wd = elm_widget_data_get(obj);
	//Evas_Object *sub = event_info;
	if (!wd)
	       	return;
}

static void
_sizing_eval(Evas_Object *obj)
{
	Widget_Data *wd = elm_widget_data_get(obj);

	if (!wd)
	       	return;

	DBG("[%s]\n", __func__);

	_colorpicker_object_move(obj, NULL, obj, NULL);
	_colorpicker_object_resize(obj, NULL, obj, NULL);
}


static void _colorpicker_object_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("%s", __func__);

	Widget_Data *wd;
	Evas_Coord x, y;

	if(!data)
		return;

	wd = elm_widget_data_get((Evas_Object *)data);

	if(!wd)
	       	return;

	evas_object_geometry_get(wd->lay, &x, &y, NULL, NULL);

	wd->_x = x;
	wd->_y = y;

	evas_object_move(wd->lay, x, y);
}

static void _colorpicker_object_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("%s", __func__);

	Widget_Data *wd;
	Evas_Coord w, h;
	Evas_Coord arrow_w, arrow_h;

	int i;

	if(!data)
		return;

	wd = elm_widget_data_get((Evas_Object *)data);

	if(!wd)	return;
	if(!wd->lay) return;

	evas_object_geometry_get(wd->lay, NULL, NULL, &w, &h);
	wd->_w = w;
	wd->_h = h;

	edje_object_part_geometry_get(wd->cp[0]->colorbar, "arrow_bg", NULL, NULL, NULL,&arrow_h);
	edje_object_part_geometry_get(wd->cp[0]->colorbar, "arrow", NULL, NULL, &arrow_w, NULL);

	for (i=0 ;  i<3 ; i++)
		evas_object_size_hint_min_set(wd->cp[i]->arrow, arrow_w, arrow_h);

	evas_object_resize(wd->lay, w, h);

	elm_colorpicker_color_set((Evas_Object*)data, wd->r, wd->g , wd->b);
}


static void _colorpicker_object_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("%s", __func__);

	Widget_Data *wd = NULL;

	if(data == NULL)
	       	return;

	wd = elm_widget_data_get((Evas_Object *)data);

	if(wd == NULL)
	       	return;

	if (wd->lay) {
		evas_object_show(wd->lay);
	}
}

static void _colorpicker_object_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("%s", __func__);

	Widget_Data *wd = NULL;

	if(data == NULL)
	       	return;

	wd = elm_widget_data_get((Evas_Object *)data);

	if(wd == NULL)
		return;

	if (wd->lay) {
		evas_object_hide(wd->lay);
	}
}

static void _rgb_to_hsl(void *data)
{
	Widget_Data *wd = (Widget_Data*) data;

	double r, g, b;
	double v, m, vm;
	double r2, g2, b2;

	r = wd->r;
	g = wd->g;
	b = wd->b;

	r /= 255.0;
	g /= 255.0;
	b /= 255.0;

	v = (r > g) ? r : g ;
	v = (v > b) ? v : b ;

	m = (r < g) ? r : g ;
	m = (m < b) ? m : b ;

	wd->h = 0.0;
	wd->s = 0.0;
	wd->l = 0.0;


	wd->l = (m + v) / 2.0;

	if (wd->l <= 0.0)
		return;

	vm = v - m;
	wd->s = vm;

	if (wd->s > 0.0)
		wd->s /= (wd->l <= 0.5) ? (v + m ) : (2.0 - v - m) ;

	else
		return;

	r2 = (v - r) / vm;
	g2 = (v - g) / vm;
	b2 = (v - b) / vm;

	if (r == v)
		wd->h = (g == m ? 5.0 + b2 : 1.0 - g2);

	else if (g == v)
		wd->h = (b == m ? 1.0 + r2 : 3.0 - b2);

	else
		wd->h = (r == m ? 3.0 + g2 : 5.0 - r2);

	wd->h *= 60.0 ;
}


static void _hsl_to_rgb(void *data)
{
	Widget_Data *wd = (Widget_Data*) data;

	double r, g, b;
	double _h, _s, _l ;
	int i;
	double sv, vsf, f, p, q, t, v;

	_h = wd->h;
	_s = wd->s;
	_l = wd->l;

	if(_s == 0.0){
		r = g = b = _l;
	}

	else {
		if ( _h == 360.0 )
			_h = 0.0;
		_h /= 60.0;

		v = (_l <= 0.5)? ( _l * (1.0 + _s)) : ( _l +  _s - (_l * _s));
		p = _l + _l - v;

		if (v != 0)
			sv = (v - p) / v;
		else
			sv = 0;

		i = (int)_h;
		f = _h - i;

		vsf = v * sv * f;

		t = p + vsf; //mid1
		q = v - vsf; //mid2

		switch(i) {
			case 0: r = v; g = t; b = p; break;
			case 1: r = q; g = v; b = p; break;
			case 2: r = p; g = v; b = t; break;
			case 3: r = p; g = q; b = v; break;
			case 4: r = t; g = p; b = v; break;
			case 5: r = v; g = p; b = q; break;
		}
	}

	i = (int)(r * 255.0);
	f = (r * 255.0) - i;
	wd->r = ( f <= 0.5 )? i : (i + 1) ;

	i = (int)(g * 255.0);
	f = (g * 255.0) - i;
	wd->g = ( f <= 0.5 )? i : (i + 1) ;

	i = (int)(b * 255.0);
	f = (b * 255.0) - i;
	wd->b = ( f <= 0.5 )? i : (i + 1) ;

}

static void _color_with_saturation(void *data)
{
	Widget_Data *wd = (Widget_Data*) data;

	if( wd->er > 127 )
		wd->sr = (int) ( (double)127 + ((double)wd->er - (double)127) * wd->s );
	else
		wd->sr = (int) ( (double)127 - ((double)127 - (double)wd->er) * wd->s );

	if( wd->eg > 127 )
		wd->sg = (int) ( (double)127 + ((double)wd->eg - (double)127) * wd->s );
	else
		wd->sg = (int) ( (double)127 - ((double)127 - (double)wd->eg) * wd->s );

	if( wd->eb > 127 )
		wd->sb = (int) ( (double)127 + ((double)wd->eb - (double)127) * wd->s );
	else
		wd->sb = (int) ( (double)127 - ((double)127 - (double)wd->eb) * wd->s );
}

static void _draw_rects(void *data, double x)
{
	Colorpicker_Data *cp = (Colorpicker_Data*)data;
	Widget_Data *wd = elm_widget_data_get(cp->parent);

	double one_six = (double)1/(double)6;

	switch(cp->colorpicker_num)
	{
		case 0 :
			wd->h =  360.0 * x ;

			if( x < one_six ){
				wd->er = 255;
				wd->eg = (int)((double)255 * x * (double)6 );
				wd->eb = 0;
			}
			else if( x < (double)2 * one_six ){
				wd->er = 255 - (int)((double)255 * ( x - one_six ) * (double)6 );
				wd->eg = 255;
				wd->eb = 0;
			}
			else if( x < (double)3 * one_six ){
				wd->er = 0;
				wd->eg = 255;
				wd->eb = (int)((double)255 * ( x - (double)2 * one_six ) * (double)6 );
			}
			else if( x < (double)4 * one_six ){
				wd->er = 0;
				wd->eg = 255 - (int)((double)255 * ( x - (double)3 * one_six ) * (double)6);
				wd->eb = 255;
			}
			else if( x < (double)5 * one_six ){
				wd->er = (int)((double)255 * ( x - (double)4 * one_six ) * (double)6 );
				wd->eg = 0;
				wd->eb = 255;
			}
			else {
				wd->er = 255;
				wd->eg = 0;
				wd->eb = 255 - (int)((double)255 * ( x - (double)5 * one_six ) * (double)6 );
			}

			evas_object_color_set(wd->cp[0]->arrow, wd->er, wd->eg, wd->eb, 255);
			evas_object_color_set(wd->cp[1]->bg_rect, wd->er, wd->eg, wd->eb, 255);

			_color_with_saturation(wd);

			evas_object_color_set(wd->cp[1]->arrow, wd->sr, wd->sg, wd->sb, 255);

			break;

		case 1:
			wd->s = (double)1 - x;

			_color_with_saturation(wd);

			evas_object_color_set(wd->cp[1]->arrow, wd->sr, wd->sg, wd->sb, 255);

			break;

		case 2:
			wd->l = x ;

			evas_object_color_set(wd->cp[2]->arrow, (int)( (double)255 * x ), (int)( (double)255 * x ), (int)( (double)255 * x ), 255);

			break;

		default:
			break;

	}

	_hsl_to_rgb(wd);

}

static void _arrow_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Colorpicker_Data *cp = (Colorpicker_Data*)data;
	double x, y;
	edje_object_part_drag_value_get(obj, "arrow", &x, &y);

	_draw_rects(data, x);

	evas_object_smart_callback_call(cp->parent, "clicked", NULL);
}

static void _colorbar_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Colorpicker_Data *cp = (Colorpicker_Data*)data;
	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Coord x, y, w, h;
	double arrow_x, arrow_y;

	evas_object_geometry_get(cp->bar, &x, &y, &w, &h);

	edje_object_part_drag_value_get(cp->colorbar, "arrow", &arrow_x, &arrow_y);

	arrow_x = ((double)ev->output.x - (double)x)  / (double)w;

	if (arrow_x > 1)
		arrow_x = 1;

	if (arrow_x < 0)
		arrow_x = 0;

	edje_object_part_drag_value_set(cp->colorbar, "arrow", arrow_x, arrow_y);

	_draw_rects(data, arrow_x);

	evas_object_smart_callback_call(cp->parent, "clicked", NULL);
	evas_event_feed_mouse_cancel(e, NULL, NULL);
	evas_event_feed_mouse_down(e, 1, EVAS_BUTTON_NONE, NULL, NULL);
}


static int _mv_timer(void *data)
{
	Colorpicker_Data *cp = (Colorpicker_Data *)data;

	double x, y;

	edje_object_part_drag_value_get(cp->colorbar, "arrow", &x, &y);

	if (cp->button_state == L_BUTTON_PRESSED ) {
		x -= (double)1/(double)BASE_STEP;

		if(x < 0)
			x = 0;
		edje_object_part_drag_value_set(cp->colorbar, "arrow", x, y);

		_draw_rects(data, x);

		evas_object_smart_callback_call(cp->parent, "clicked", NULL);

		return 1;
	}

	else if (cp->button_state == R_BUTTON_PRESSED ) {
		x += (double)1/(double)BASE_STEP;

		if(x > 1)
			x = 1;
		edje_object_part_drag_value_set(cp->colorbar, "arrow", x, y);

		_draw_rects(data, x);

		evas_object_smart_callback_call(cp->parent, "clicked", NULL);

		return 1;
	}

	else
		return 0;
}

static int _long_press_timer(void *data)
{
	Colorpicker_Data *cp = (Colorpicker_Data *)data;
	Widget_Data *wd = elm_widget_data_get(cp->parent);

	if(wd->lp_timer){
		ecore_timer_del(wd->lp_timer);
		wd->lp_timer = NULL;
	}

	wd->mv_timer = ecore_timer_add(0.01, _mv_timer, cp);

	return EXIT_SUCCESS;
}

static void _left_button_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Colorpicker_Data *cp = (Colorpicker_Data*)data;
	Widget_Data *wd = elm_widget_data_get(cp->parent);
	double x, y;

	edje_object_signal_emit(cp->lbt, "MOUSE_DOWN", "LEFT_ARROW");

	edje_object_part_drag_value_get(cp->colorbar, "arrow", &x, &y);

	if (cp->colorpicker_num == 0)
		x -= (double)1/(double)HUE_STEP;
	else if (cp->colorpicker_num == 1)
		x -= (double)1/(double)SAT_STEP;
	else if (cp->colorpicker_num == 2)
		x -= (double)1/(double)LIG_STEP;

	if(x < 0)
		x = 0;
	edje_object_part_drag_value_set(cp->colorbar, "arrow", x, y);

	_draw_rects(data, x);

	evas_object_smart_callback_call(cp->parent, "clicked", NULL);
	cp->button_state = L_BUTTON_PRESSED;

	if(wd->lp_timer){
		ecore_timer_del(wd->lp_timer);
		wd->lp_timer = NULL;
	}
	wd->lp_timer = ecore_timer_add(1.0, _long_press_timer, cp);
}

static void _right_button_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Colorpicker_Data *cp = (Colorpicker_Data*)data;
	Widget_Data *wd = elm_widget_data_get(cp->parent);
	double x, y;

	edje_object_signal_emit(cp->rbt, "MOUSE_DOWN", "RIGHT_ARROW");

	edje_object_part_drag_value_get(cp->colorbar, "arrow", &x, &y);

	if (cp->colorpicker_num == 0)
		x += (double)1/(double)HUE_STEP;
	else if (cp->colorpicker_num == 1)
		x += (double)1/(double)SAT_STEP;
	else if (cp->colorpicker_num == 2)
		x += (double)1/(double)LIG_STEP;

	if(x > 1)
		x = 1;
	edje_object_part_drag_value_set(cp->colorbar, "arrow", x, y);

	_draw_rects(data, x);

	evas_object_smart_callback_call(cp->parent, "clicked", NULL);
	cp->button_state = R_BUTTON_PRESSED;
	if(wd->lp_timer){
		ecore_timer_del(wd->lp_timer);
		wd->lp_timer = NULL;
	}
	wd->lp_timer = ecore_timer_add(1.0, _long_press_timer, cp);
}

static void _left_button_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Colorpicker_Data *cp = (Colorpicker_Data*)data;
	Widget_Data *wd = elm_widget_data_get(cp->parent);
	
	if(wd->lp_timer){
		ecore_timer_del(wd->lp_timer);
		wd->lp_timer = NULL;
	}

	cp->button_state = BUTTON_RELEASED;
	edje_object_signal_emit(cp->lbt, "MOUSE_UP", "LEFT_ARROW");
}

static void _right_button_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Colorpicker_Data *cp = (Colorpicker_Data*)data;
	Widget_Data *wd = elm_widget_data_get(cp->parent);
	
	if(wd->lp_timer){
		ecore_timer_del(wd->lp_timer);
		wd->lp_timer = NULL;
	}

	cp->button_state = BUTTON_RELEASED;
	edje_object_signal_emit(cp->rbt, "MOUSE_UP", "RIGHT_ARROW");
}

static void _add_colorbar(Evas_Object* obj)
{
	int i;
	Widget_Data *wd ;
	Evas *e;
	char colorbar_name[128];

	wd = elm_widget_data_get(obj);
	if (!wd) return ;

	e = wd->e;

	for( i=0 ; i<3 ; i++ ){
		wd->cp[i] = ELM_NEW(Colorpicker_Data);

		wd->cp[i]->parent = obj;
		wd->cp[i]->colorpicker_num = i;

		/* load colorbar area */
		wd->cp[i]->colorbar =  edje_object_add(e);
		_elm_theme_object_set(obj, wd->cp[i]->colorbar, "colorpicker", "base", "default");
		snprintf(colorbar_name, sizeof(colorbar_name), "colorbar_%d", i);
		edje_object_signal_callback_add(wd->cp[i]->colorbar, "drag", "*", _arrow_cb, wd->cp[i]);
		edje_object_part_swallow(wd->lay, colorbar_name, wd->cp[i]->colorbar);
		evas_object_show(wd->cp[i]->colorbar);

		/* load colorbar image */
		wd->cp[i]->bar = edje_object_add(e);
		_elm_theme_object_set(obj, wd->cp[i]->bar, "colorpicker", "image", colorbar_name);
		evas_object_show(wd->cp[i]->bar);
		edje_object_part_swallow(wd->cp[i]->colorbar, "bar", wd->cp[i]->bar);

		/* provide expanded touch area */
		wd->cp[i]->touch_area = evas_object_rectangle_add(e);
		evas_object_color_set(wd->cp[i]->touch_area, 0, 0, 0, 0);
		edje_object_part_swallow(wd->cp[i]->colorbar, "arrow_bg", wd->cp[i]->touch_area);
		evas_object_event_callback_add(wd->cp[i]->touch_area, EVAS_CALLBACK_MOUSE_DOWN, _colorbar_cb, wd->cp[i]);
		evas_object_show(wd->cp[i]->touch_area);

		/* load background rectangle of the colorbar. used for changing color of the opacity bar */
		if(i==1){
			wd->cp[i]->bg_rect = evas_object_rectangle_add(e);
			evas_object_color_set(wd->cp[i]->bg_rect, wd->er, wd->eg, wd->eb, 255);
			evas_object_resize(wd->cp[i]->bg_rect, 10, 10);
			evas_object_show(wd->cp[i]->bg_rect);
			edje_object_part_swallow(wd->cp[i]->colorbar, "bar_bg", wd->cp[i]->bg_rect);
		}
		/* load arrow image, pointing the colorbar */
		wd->cp[i]->arrow = edje_object_add(e);
		_elm_theme_object_set(obj, wd->cp[i]->arrow, "colorpicker", "image", "updown");
		evas_object_show(wd->cp[i]->arrow);
		edje_object_part_swallow(wd->cp[i]->colorbar, "arrow", wd->cp[i]->arrow);

		if(i==2)
			evas_object_color_set(wd->cp[i]->arrow, 0, 0, 0, 255);
		else
			evas_object_color_set(wd->cp[i]->arrow, wd->er, wd->eg, wd->eb, 255);

		/* load left button */
		wd->cp[i]->lbt = edje_object_add(e);
		_elm_theme_object_set(obj, wd->cp[i]->lbt, "colorpicker", "button", "left");
		evas_object_event_callback_add(wd->cp[i]->lbt, EVAS_CALLBACK_MOUSE_DOWN, _left_button_down_cb, wd->cp[i]);
		evas_object_event_callback_add(wd->cp[i]->lbt, EVAS_CALLBACK_MOUSE_UP, _left_button_up_cb, wd->cp[i]);
		evas_object_show(wd->cp[i]->lbt);
		edje_object_part_swallow(wd->cp[i]->colorbar, "l_button", wd->cp[i]->lbt);

		/* load right button */
		wd->cp[i]->rbt = edje_object_add(e);
		_elm_theme_object_set(obj, wd->cp[i]->rbt, "colorpicker", "button", "right");
		evas_object_event_callback_add(wd->cp[i]->rbt, EVAS_CALLBACK_MOUSE_DOWN, _right_button_down_cb, wd->cp[i]);
		evas_object_event_callback_add(wd->cp[i]->rbt, EVAS_CALLBACK_MOUSE_UP, _right_button_up_cb, wd->cp[i]);
		evas_object_show( wd->cp[i]->rbt);
		edje_object_part_swallow(wd->cp[i]->colorbar, "r_button", wd->cp[i]->rbt);

	}
}

/**
 * Add a new colorpicker to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Colorpicker
 */
EAPI Evas_Object *elm_colorpicker_add(Evas_Object *parent)
{
	Evas_Object *obj = NULL;
	Widget_Data *wd = NULL;
	Evas *e;
	wd = ELM_NEW(Widget_Data);

	e = evas_object_evas_get(parent);
	if (e == NULL) return NULL;
	obj = elm_widget_add(e);
	elm_widget_type_set(obj, "colorpicker");
	elm_widget_sub_object_add(parent, obj);
	elm_widget_data_set(obj, wd);
	wd->e = e;
	elm_widget_del_hook_set(obj, _del_hook);
	elm_widget_theme_hook_set(obj, _theme_hook);

	/* load background edj */
	wd->lay = edje_object_add(e);
	_elm_theme_object_set(obj, wd->lay, "colorpicker", "bg", "default");

	evas_object_size_hint_weight_set(wd->lay, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if(wd->lay == NULL) {
		printf("Cannot load bg edj\n");
		return NULL;
	}

	elm_widget_resize_object_set(obj, wd->lay);

	wd->er = 255;
	wd->eg = 0;
	wd->eb = 0;
	wd->h = 0.0;
	wd->s = 1.0;
	wd->l = 0.0;

	_hsl_to_rgb(wd);

	_add_colorbar(obj);

//	evas_object_smart_member_add(wd->lay, obj);
	evas_object_event_callback_add(wd->lay, EVAS_CALLBACK_RESIZE, _colorpicker_object_resize, obj);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _colorpicker_object_move, obj);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _colorpicker_object_show, obj);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _colorpicker_object_hide, obj);

	// FIXME
	// evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
	// _sizing_eval(obj);

	return obj;
}


/**
 * Set a color for the colorpicker
 *
 * @param obj	Colorpicker object
 * @param r	r-value of color
 * @param g	g-value of color
 * @param b	b-value of color
 *
 * @ingroup Colorpicker
 */
EAPI void elm_colorpicker_color_set(Evas_Object *obj, unsigned int r, unsigned int g , unsigned int b)
{
	Widget_Data *wd = elm_widget_data_get(obj);
	double x, y;

	wd->r = r;
	wd->g = g;
	wd->b = b;

	_rgb_to_hsl(wd);

	edje_object_part_drag_value_get(wd->cp[0]->colorbar, "arrow", &x, &y);
	x = wd->h / 360.0;
	edje_object_part_drag_value_set(wd->cp[0]->colorbar, "arrow", x, y);
	_draw_rects(wd->cp[0], x);

	edje_object_part_drag_value_get(wd->cp[1]->colorbar, "arrow", &x, &y);
	x = 1.0 - wd->s;
	edje_object_part_drag_value_set(wd->cp[1]->colorbar, "arrow", x, y);
	_draw_rects(wd->cp[1], x);

	edje_object_part_drag_value_get(wd->cp[2]->colorbar, "arrow", &x, &y);
	x = wd->l ;
	edje_object_part_drag_value_set(wd->cp[2]->colorbar, "arrow", x, y);
	_draw_rects(wd->cp[2], x);
}


/**
 * Get a color from the colorpicker
 *
 * @param obj	Colorpicker object
 * @param r	integer pointer for r-value of color
 * @param g	integer pointer for g-value of color
 * @param b	integer pointer for b-value of color
 *
 * @ingroup Colorpicker
 */
EAPI void elm_colorpicker_color_get(Evas_Object *obj, unsigned int *r, unsigned int *g , unsigned int *b)
{
	Widget_Data *wd = elm_widget_data_get(obj);

	*r = wd->r;
	*g = wd->g;
	*b = wd->b;
}

