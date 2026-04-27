#include "widgets/screen.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <fonts.h>

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_STATUS)
static struct zmk_widget_screen screen_widget;
#endif

lv_obj_t *zmk_display_status_screen() {
    LOG_INF("STATUS_SCREEN: enter");
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);
    LOG_INF("STATUS_SCREEN: lv_obj_create done");

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_STATUS)
    LOG_INF("STATUS_SCREEN: calling zmk_widget_screen_init");
    zmk_widget_screen_init(&screen_widget, screen);
    LOG_INF("STATUS_SCREEN: screen_init returned");
    lv_obj_align(zmk_widget_screen_obj(&screen_widget), LV_ALIGN_TOP_LEFT, 0, 0);
    LOG_INF("STATUS_SCREEN: align done");
#endif

    LOG_INF("STATUS_SCREEN: returning");
    return screen;
}
