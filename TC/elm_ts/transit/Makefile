CC ?= gcc

TARGETS = utc_UIFW_elm_transit_add_func \
	  utc_UIFW_elm_transit_del_func \
  	  utc_UIFW_elm_transit_effect_add_func \
  	  utc_UIFW_elm_transit_effect_del_func \
  	  utc_UIFW_elm_transit_object_add_func \
  	  utc_UIFW_elm_transit_object_remove_func \
  	  utc_UIFW_elm_transit_objects_get_func \
  	  utc_UIFW_elm_transit_objects_final_state_keep_set_func \
 	  utc_UIFW_elm_transit_objects_final_state_keep_get_func \
 	  utc_UIFW_elm_transit_event_enabled_set_func \
 	  utc_UIFW_elm_transit_event_enabled_get_func \
 	  utc_UIFW_elm_transit_del_cb_set_func \
  	  utc_UIFW_elm_transit_auto_reverse_set_func \
	  utc_UIFW_elm_transit_auto_reverse_get_func \
	  utc_UIFW_elm_transit_repeat_times_set_func \
	  utc_UIFW_elm_transit_repeat_times_get_func \
	  utc_UIFW_elm_transit_tween_mode_set_func \
	  utc_UIFW_elm_transit_tween_mode_get_func \
	  utc_UIFW_elm_transit_duration_set_func \
	  utc_UIFW_elm_transit_duration_get_func \
	  utc_UIFW_elm_transit_go_func \
	  utc_UIFW_elm_transit_paused_set_func \
	  utc_UIFW_elm_transit_paused_get_func \
	  utc_UIFW_elm_transit_progress_value_get_func \
	  utc_UIFW_elm_transit_chain_transit_add_func \
	  utc_UIFW_elm_transit_chain_transits_get_func \
	  utc_UIFW_elm_transit_effect_blend_add_func \
	  utc_UIFW_elm_transit_effect_resizing_add_func \
	  utc_UIFW_elm_transit_effect_translation_add_func \
	  utc_UIFW_elm_transit_effect_zoom_add_func \
	  utc_UIFW_elm_transit_effect_flip_add_func \
	  utc_UIFW_elm_transit_effect_resizable_flip_add_func \
	  utc_UIFW_elm_transit_effect_wipe_add_func \
	  utc_UIFW_elm_transit_effect_color_add_func \
	  utc_UIFW_elm_transit_effect_fade_add_func \
	  utc_UIFW_elm_transit_effect_rotation_add_func \
	  utc_UIFW_elm_transit_effect_image_animation_add_func

PKGS = elementary evas eina

LDFLAGS = `pkg-config --libs $(PKGS)`
LDFLAGS += $(TET_ROOT)/lib/tet3/tcm_s.o
LDFLAGS += -L$(TET_ROOT)/lib/tet3 -ltcm_s
LDFLAGS += -L$(TET_ROOT)/lib/tet3 -lapi_s

CFLAGS = -I. `pkg-config --cflags $(PKGS)`
CFLAGS += -I$(TET_ROOT)/inc/tet3
CFLAGS += -Wall

all: $(TARGETS)

$(TARGETS): %: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)
	rm -f tet_captured
	rm -f *~
