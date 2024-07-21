/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"
#include "net.h"
#include "mqtt.h"
//#include "./lvgl/src/drivers/display/sunxifb/sunxifb.h"
#include "./lvgl/src/drivers/evdev/lv_evdev.h"
/*********************
 *      DEFINES
 *********************/
#define _X11_   1
#define _SDL_  2
#define _FB_    3
#define DISP_DEV  _FB_

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t * hal_init(int32_t w, int32_t h);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
bool terminated = false;
void create_main_screen(lv_obj_t* parent);
void update_main_screen();
void init_font();
int main(int argc, char **argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the display, and the input devices*/
  hal_init(800, 480);

#if 0
  /*Open a demo or an example*/
  if (argc == 0) {
    lv_demo_widgets();
    //  lv_example_chart_1();
  } else {
    if (!lv_demos_create(&argv[1], argc - 1)) {
      lv_demos_show_help();
      goto demo_end;
    }
  }

#endif
//lv_example_chart_1();
  my_mqtt_init();
  init_font();
  init_screen();
  //create_main_screen(lv_scr_act());
  /*To hide the memory and performance indicators in the corners
   *disable `LV_USE_MEM_MONITOR` and `LV_USE_PERF_MONITOR` in `lv_conf.h`*/

  //request_url("192.168.1.5","5000","/stock/price/000997,000561");
  while(!terminated) {

	  update_screen();
	  //printf("update\n");
	  //update_main_screen();
      /* Periodically call the lv_task handler.
       * It could be done in a timer interrupt or an OS task too.*/
      lv_timer_handler();
      usleep(10* 1000);
  }

demo_end:
my_mqtt_del();
  lv_deinit();
  return 0;
}


#if !LV_X11_DIRECT_EXIT
static void on_close_cb(lv_event_t * e)
{
    //...

    terminated = true;
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t * hal_init(int32_t w, int32_t h)
{
  lv_group_set_default(lv_group_create());
#if DISP_DEV==_X11_
  lv_display_t * disp = lv_x11_window_create("lvgl",w, h);


  lv_x11_inputs_create(disp,NULL);
  LV_IMG_DECLARE(mouse_cursor_icon);
  lv_x11_inputs_create(disp, &mouse_cursor_icon);

#if !LV_X11_DIRECT_EXIT
    /* set optional window close callback to enable application cleanup and exit */
  lv_x11_window_set_close_cb(disp, on_close_cb, disp);
#endif
  lv_display_set_default(disp);
#elif DISP_DEV==_FB_
  lv_display_t *disp = lv_linux_fbdev_create();
  lv_linux_fbdev_set_file(disp, "/dev/fb0");

  lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event2");
  lv_indev_set_display(touch, disp);
#if 0
    
    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);                /*Basic initialization*/
    indev_drv.type =LV_INDEV_TYPE_POINTER;        /*See below.*/
    indev_drv.read_cb = evdev_read;               /*See below.*/
    /*Register the driver in LVGL and save the created input device object*/
    lv_indev_t * evdev_indev = lv_indev_drv_register(&indev_drv);
    #endif
#else
  lv_display_t * disp = lv_sdl_window_create(w, h);

  lv_indev_t * mouse = lv_sdl_mouse_create();
  lv_indev_set_group(mouse, lv_group_get_default());
  lv_indev_set_display(mouse, disp);
  lv_display_set_default(disp);

  LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t * cursor_obj;
  cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
  lv_image_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
  lv_indev_set_cursor(mouse, cursor_obj);             /*Connect the image  object to the driver*/

  lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
  lv_indev_set_display(mousewheel, disp);

  lv_indev_t * keyboard = lv_sdl_keyboard_create();
  lv_indev_set_display(keyboard, disp);
  lv_indev_set_group(keyboard, lv_group_get_default());
#endif
  return disp;
}
