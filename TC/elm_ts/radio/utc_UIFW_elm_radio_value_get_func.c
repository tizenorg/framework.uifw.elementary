#include <tet_api.h>
#include <Elementary.h>

// Definitions
// For checking the result of the positive test case.
#define TET_CHECK_PASS(x1, y...) \
{ \
	Evas_Object *err = y; \
	if (err == (x1)) \
		{ \
			tet_printf("[TET_CHECK_PASS]:: %s[%d] : Test has failed..", __FILE__,__LINE__); \
			tet_result(TET_FAIL); \
			return; \
		} \
}

// For checking the result of the negative test case.
#define TET_CHECK_FAIL(x1, y...) \
{ \
	Evas_Object *err = y; \
	if (err != (x1)) \
		{ \
			tet_printf("[TET_CHECK_FAIL]:: %s[%d] : Test has failed..", __FILE__,__LINE__); \
			tet_result(TET_FAIL); \
			return; \
		} \
}


Evas_Object *main_win;

static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_UIFW_elm_radio_value_get_func_01(void);
static void utc_UIFW_elm_radio_value_get_func_02(void);

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

struct tet_testlist tet_testlist[] = {
	{ utc_UIFW_elm_radio_value_get_func_01, POSITIVE_TC_IDX },
	{ utc_UIFW_elm_radio_value_get_func_02, NEGATIVE_TC_IDX },
	{ NULL, 0 }
};

static void startup(void)
{
	tet_infoline("[[ TET_MSG ]]:: ============ Startup ============ ");
	elm_init(0, NULL);
	main_win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
	evas_object_show(main_win);
}

static void cleanup(void)
{
	if ( NULL != main_win ) {
		evas_object_del(main_win);
	       	main_win = NULL;
	}
	elm_shutdown();
	tet_infoline("[[ TET_MSG ]]:: ============ Cleanup ============ ");
}

/**
 * @brief Positive test case of elm_radio_value_get()
 */
static void utc_UIFW_elm_radio_value_get_func_01(void)
{
   Evas_Object *radio = NULL;
   Evas_Object *rdg = NULL;
   int value;

   radio = elm_radio_add(main_win);
   rdg = radio;
   elm_radio_group_add(radio, rdg);
   elm_radio_state_value_set(radio, 0);
   elm_radio_value_set(rdg, 0);
   value = elm_radio_value_get(radio);
   if(value != 0)
   {
      tet_infoline("elm_radio_value_get() failed in positive test case");
      tet_result(TET_FAIL);
      return;
   }
   evas_object_show(radio);
   evas_object_del(radio);
   radio = NULL;
   tet_result(TET_PASS);
}

/**
 * @brief Negative test case of ug_init elm_radio_value_get()
 */
static void utc_UIFW_elm_radio_value_get_func_02(void)
{
   Evas_Object *radio = NULL;
   Evas_Object *rdg = NULL;
   int value;

   radio = elm_radio_add(main_win);
   rdg = radio;
   elm_radio_group_add(radio, rdg);
   elm_radio_state_value_set(radio, 0);
   elm_radio_value_set(radio, 0);
   value = elm_radio_value_get(NULL);
   evas_object_del(radio);
   radio = NULL;
   tet_result(TET_PASS);
}
