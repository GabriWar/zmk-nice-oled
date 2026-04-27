#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/split/bluetooth/peripheral.h>
#include <zmk/usb.h>

#include "animation.h"
#include "battery.h"
#include "output.h"
#include "screen_peripheral.h"

#include <fonts.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

/**
 * Peripheral animation — drawn DIRECTLY to canvas buffer (no child lv_obj —
 * they break in-buffer rotate_canvas on this side). Frame array selected at
 * compile time by Kconfig. Frame index advances on lv_timer; redraw happens
 * via the timer callback so all widgets re-render together.
 *
 * Selector order matches original animation.c:
 *   HEAD → CAT → SPACEMAN → POKEMON → (default) crystal
 * SMART_BATTERY uses the crystal array but renders battery-aware in draw_canvas.
 */
#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_HEAD)
LV_IMAGE_DECLARE(head_00); LV_IMAGE_DECLARE(head_01); LV_IMAGE_DECLARE(head_02);
LV_IMAGE_DECLARE(head_03); LV_IMAGE_DECLARE(head_04); LV_IMAGE_DECLARE(head_05);
LV_IMAGE_DECLARE(head_06); LV_IMAGE_DECLARE(head_07); LV_IMAGE_DECLARE(head_08);
LV_IMAGE_DECLARE(head_09); LV_IMAGE_DECLARE(head_10); LV_IMAGE_DECLARE(head_11);
LV_IMAGE_DECLARE(head_12); LV_IMAGE_DECLARE(head_13); LV_IMAGE_DECLARE(head_14);
LV_IMAGE_DECLARE(head_15);
static const lv_image_dsc_t *const periph_anim_frames[] = {
    &head_00, &head_01, &head_02, &head_03, &head_04, &head_05,
    &head_06, &head_07, &head_08, &head_09, &head_10, &head_11,
    &head_12, &head_13, &head_14, &head_15,
};
#elif IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_CAT)
LV_IMAGE_DECLARE(cat_0); LV_IMAGE_DECLARE(cat_1); LV_IMAGE_DECLARE(cat_2);
LV_IMAGE_DECLARE(cat_3); LV_IMAGE_DECLARE(cat_4); LV_IMAGE_DECLARE(cat_5);
LV_IMAGE_DECLARE(cat_6); LV_IMAGE_DECLARE(cat_7);
static const lv_image_dsc_t *const periph_anim_frames[] = {
    &cat_0, &cat_1, &cat_2, &cat_3, &cat_4, &cat_5, &cat_6, &cat_7,
};
#elif IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_SPACEMAN)
LV_IMAGE_DECLARE(spaceman_00); LV_IMAGE_DECLARE(spaceman_01); LV_IMAGE_DECLARE(spaceman_02);
LV_IMAGE_DECLARE(spaceman_03); LV_IMAGE_DECLARE(spaceman_04); LV_IMAGE_DECLARE(spaceman_05);
LV_IMAGE_DECLARE(spaceman_06); LV_IMAGE_DECLARE(spaceman_07); LV_IMAGE_DECLARE(spaceman_08);
LV_IMAGE_DECLARE(spaceman_09); LV_IMAGE_DECLARE(spaceman_10); LV_IMAGE_DECLARE(spaceman_11);
LV_IMAGE_DECLARE(spaceman_12); LV_IMAGE_DECLARE(spaceman_13); LV_IMAGE_DECLARE(spaceman_14);
LV_IMAGE_DECLARE(spaceman_15); LV_IMAGE_DECLARE(spaceman_16); LV_IMAGE_DECLARE(spaceman_17);
LV_IMAGE_DECLARE(spaceman_18); LV_IMAGE_DECLARE(spaceman_19);
static const lv_image_dsc_t *const periph_anim_frames[] = {
    &spaceman_00, &spaceman_01, &spaceman_02, &spaceman_03, &spaceman_04,
    &spaceman_05, &spaceman_06, &spaceman_07, &spaceman_08, &spaceman_09,
    &spaceman_10, &spaceman_11, &spaceman_12, &spaceman_13, &spaceman_14,
    &spaceman_15, &spaceman_16, &spaceman_17, &spaceman_18, &spaceman_19,
};
#elif IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_GABRIWAR)
LV_IMAGE_DECLARE(gabriwar);
static const lv_image_dsc_t *const periph_anim_frames[] = { &gabriwar };
#elif IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_POKEMON)
LV_IMAGE_DECLARE(pokemon_00); LV_IMAGE_DECLARE(pokemon_01); LV_IMAGE_DECLARE(pokemon_02);
LV_IMAGE_DECLARE(pokemon_03); LV_IMAGE_DECLARE(pokemon_04); LV_IMAGE_DECLARE(pokemon_05);
LV_IMAGE_DECLARE(pokemon_06); LV_IMAGE_DECLARE(pokemon_07); LV_IMAGE_DECLARE(pokemon_08);
LV_IMAGE_DECLARE(pokemon_09); LV_IMAGE_DECLARE(pokemon_10); LV_IMAGE_DECLARE(pokemon_11);
LV_IMAGE_DECLARE(pokemon_12); LV_IMAGE_DECLARE(pokemon_13); LV_IMAGE_DECLARE(pokemon_14);
LV_IMAGE_DECLARE(pokemon_15); LV_IMAGE_DECLARE(pokemon_16); LV_IMAGE_DECLARE(pokemon_17);
LV_IMAGE_DECLARE(pokemon_18); LV_IMAGE_DECLARE(pokemon_19); LV_IMAGE_DECLARE(pokemon_20);
LV_IMAGE_DECLARE(pokemon_21); LV_IMAGE_DECLARE(pokemon_22); LV_IMAGE_DECLARE(pokemon_23);
LV_IMAGE_DECLARE(pokemon_24); LV_IMAGE_DECLARE(pokemon_25); LV_IMAGE_DECLARE(pokemon_26);
LV_IMAGE_DECLARE(pokemon_27); LV_IMAGE_DECLARE(pokemon_28); LV_IMAGE_DECLARE(pokemon_29);
LV_IMAGE_DECLARE(pokemon_30); LV_IMAGE_DECLARE(pokemon_31); LV_IMAGE_DECLARE(pokemon_32);
LV_IMAGE_DECLARE(pokemon_33); LV_IMAGE_DECLARE(pokemon_34); LV_IMAGE_DECLARE(pokemon_35);
LV_IMAGE_DECLARE(pokemon_36); LV_IMAGE_DECLARE(pokemon_37); LV_IMAGE_DECLARE(pokemon_38);
LV_IMAGE_DECLARE(pokemon_39); LV_IMAGE_DECLARE(pokemon_40); LV_IMAGE_DECLARE(pokemon_41);
LV_IMAGE_DECLARE(pokemon_42); LV_IMAGE_DECLARE(pokemon_43); LV_IMAGE_DECLARE(pokemon_44);
LV_IMAGE_DECLARE(pokemon_45); LV_IMAGE_DECLARE(pokemon_46); LV_IMAGE_DECLARE(pokemon_47);
static const lv_image_dsc_t *const periph_anim_frames[] = {
    &pokemon_00, &pokemon_01, &pokemon_02, &pokemon_03, &pokemon_04, &pokemon_05,
    &pokemon_06, &pokemon_07, &pokemon_08, &pokemon_09, &pokemon_10, &pokemon_11,
    &pokemon_12, &pokemon_13, &pokemon_14, &pokemon_15, &pokemon_16, &pokemon_17,
    &pokemon_18, &pokemon_19, &pokemon_20, &pokemon_21, &pokemon_22, &pokemon_23,
    &pokemon_24, &pokemon_25, &pokemon_26, &pokemon_27, &pokemon_28, &pokemon_29,
    &pokemon_30, &pokemon_31, &pokemon_32, &pokemon_33, &pokemon_34, &pokemon_35,
    &pokemon_36, &pokemon_37, &pokemon_38, &pokemon_39, &pokemon_40, &pokemon_41,
    &pokemon_42, &pokemon_43, &pokemon_44, &pokemon_45, &pokemon_46, &pokemon_47,
};
#else /* default: crystal (also used by SMART_BATTERY) */
LV_IMAGE_DECLARE(crystal_01); LV_IMAGE_DECLARE(crystal_02);
LV_IMAGE_DECLARE(crystal_03); LV_IMAGE_DECLARE(crystal_04);
LV_IMAGE_DECLARE(crystal_05); LV_IMAGE_DECLARE(crystal_06);
LV_IMAGE_DECLARE(crystal_07); LV_IMAGE_DECLARE(crystal_08);
LV_IMAGE_DECLARE(crystal_09); LV_IMAGE_DECLARE(crystal_10);
LV_IMAGE_DECLARE(crystal_11); LV_IMAGE_DECLARE(crystal_12);
LV_IMAGE_DECLARE(crystal_13); LV_IMAGE_DECLARE(crystal_14);
LV_IMAGE_DECLARE(crystal_15); LV_IMAGE_DECLARE(crystal_16);
static const lv_image_dsc_t *const periph_anim_frames[] = {
    &crystal_01, &crystal_02, &crystal_03, &crystal_04,
    &crystal_05, &crystal_06, &crystal_07, &crystal_08,
    &crystal_09, &crystal_10, &crystal_11, &crystal_12,
    &crystal_13, &crystal_14, &crystal_15, &crystal_16,
};
#endif
#define PERIPH_ANIM_FRAME_COUNT (sizeof(periph_anim_frames) / sizeof(periph_anim_frames[0]))

static uint8_t periph_anim_frame_idx = 0;

/**
 * sleep status
 **/

#if IS_ENABLED(CONFIG_NICE_OLED_SHOW_SLEEP_ART_ON_IDLE) ||                                         \
    IS_ENABLED(CONFIG_NICE_OLED_SHOW_SLEEP_ART_ON_SLEEP)
#include "sleep_status.h"
static struct zmk_widget_sleep_status sleep_status_widget;
#endif

/**
 * luna
 **/

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_WPM)
#include "luna.h"
static struct zmk_widget_luna luna_widget;
#endif

/**
 * Draw canvas
 **/

static void draw_canvas(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    LOG_INF("PERIPH_DRAW: enter");
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);
    LOG_INF("PERIPH_DRAW: canvas=%p", (void *)canvas);

    LOG_INF("PERIPH_DRAW: bg");
    draw_background(canvas);
    LOG_INF("PERIPH_DRAW: output");
    draw_output_status(canvas, state);
    LOG_INF("PERIPH_DRAW: battery");
    draw_battery_status(canvas, state);


    LOG_INF("PERIPH_DRAW: rotate");
    rotate_canvas(canvas, cbuf);

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL)
    /* Crystal AFTER rotate — drawn in post-rotation/display coords so it
     * stays upright. Frame selection:
     *   SMART_BATTERY + charging → cycle 16 frames (animated)
     *   SMART_BATTERY + battery  → static frame mapped to charge level
     *   else (regular anim)      → cycle 16 frames always */
#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_SMART_BATTERY)
    uint8_t frame = state->charging
        ? periph_anim_frame_idx
        : (uint8_t)((state->battery * (PERIPH_ANIM_FRAME_COUNT - 1)) / 100);
#else
    uint8_t frame = periph_anim_frame_idx;
#endif
    if (frame >= PERIPH_ANIM_FRAME_COUNT) frame = PERIPH_ANIM_FRAME_COUNT - 1;
    LOG_INF("PERIPH_DRAW: crystal frame %u (charging=%d batt=%u)",
            frame, state->charging, state->battery);
    {
        lv_draw_image_dsc_t img_dsc;
        lv_draw_image_dsc_init(&img_dsc);
        canvas_draw_img(canvas,
                        CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_CUSTOM_X,
                        CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_CUSTOM_Y,
                        periph_anim_frames[frame], &img_dsc);
    }
#endif /* CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL */
    LOG_INF("PERIPH_DRAW: done");
}

/* Advance crystal frame + force a redraw on every registered widget. */
static void crystal_anim_timer_cb(lv_timer_t *timer) {
    (void)timer;
    periph_anim_frame_idx = (periph_anim_frame_idx + 1) % PERIPH_ANIM_FRAME_COUNT;
    struct zmk_widget_screen *w;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
        draw_canvas(w->obj, w->cbuf, &w->state);
    }
}

/**
 * Battery status
 **/

static void set_battery_status(struct zmk_widget_screen *widget,
                               struct battery_status_state state) {

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

    widget->state.battery = state.level;

    draw_canvas(widget->obj, widget->cbuf, &widget->state);

    // draw_animation(widget->obj, widget);

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_SMART_BATTERY)
    if (widget->state.charging) {
        // mostrar
        // lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);
        animation_smart_battery_on(widget->obj);

    } else {
        // quitar
        // lv_obj_add_flag(widget->art, LV_OBJ_FLAG_HIDDEN);
        animation_smart_battery_off(widget->obj);
    }
#endif
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    return (struct battery_status_state){
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state);

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

/**
 * Peripheral status
 **/

static struct peripheral_status_state get_state(const zmk_event_t *_eh) {
    return (struct peripheral_status_state){.connected = zmk_split_bt_peripheral_is_connected()};
}

static void set_connection_status(struct zmk_widget_screen *widget,
                                  struct peripheral_status_state state) {
    widget->state.connected = state.connected;

    draw_canvas(widget->obj, widget->cbuf, &widget->state);
}

static void output_status_update_cb(struct peripheral_status_state state) {
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_connection_status(widget, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_status, struct peripheral_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_peripheral_status, zmk_split_peripheral_status_changed);

/**
 * Initialization
 **/

int zmk_widget_screen_init(struct zmk_widget_screen *widget, lv_obj_t *parent) {
    LOG_INF("PERIPH_INIT: enter");
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, CANVAS_HEIGHT, CANVAS_WIDTH);
    /* Match central: zero pad/border so canvas aligns to true (0,0) */
    lv_obj_set_style_pad_all(widget->obj, 0, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    LOG_INF("PERIPH_INIT: widget set_size + zero pad/border done");

    lv_obj_t *canvas = lv_canvas_create(widget->obj);
    lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, 0, 0);
    /* 160x160 RGB565 — matches central screen.c. lv_draw_* in v9 doesn't render
     * to indexed I1 (LV_COLOR_FORMAT_NATIVE under LV_COLOR_DEPTH=1) → blank canvas. */
    lv_canvas_set_buffer(canvas, widget->cbuf, CANVAS_HEIGHT, CANVAS_HEIGHT, LV_COLOR_FORMAT_RGB565);
    LOG_INF("PERIPH_INIT: set_buffer done");

    sys_slist_append(&widgets, &widget->node);
    LOG_INF("PERIPH_INIT: list append done");

    /* Force initial render so screen isn't blank before first event fires */
    draw_canvas(widget->obj, widget->cbuf, &widget->state);
    LOG_INF("PERIPH_INIT: initial draw_canvas done");

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL)
    /* Crystal anim driver — fires every ANIMATION_PERIPHERAL_MS / frames.
     * In SMART_BATTERY mode the static-frame branch ignores the index, so
     * timer ticks while on battery just produce identical redraws — harmless. */
    static bool crystal_timer_started = false;
    if (!crystal_timer_started) {
        uint32_t period = CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_MS / PERIPH_ANIM_FRAME_COUNT;
        if (period < 30) period = 30; /* don't spam display thread */
        lv_timer_create(crystal_anim_timer_cb, period, NULL);
        crystal_timer_started = true;
        LOG_INF("PERIPH_INIT: crystal anim timer @ %u ms", period);
    }
#endif

    /* DEBUG: skip draw_animation — empty animimg overlay sibling on canvas may
     * disturb in-buffer rotate_canvas. Test pure-canvas render w/ rotation. */
    widget_battery_status_init();
    widget_peripheral_status_init();

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_WPM)
    zmk_widget_luna_init(&luna_widget, canvas);
    lv_obj_align(zmk_widget_luna_obj(&luna_widget), LV_ALIGN_TOP_LEFT, CONFIG_NICE_OLED_WIDGET_LUNA_CUSTOM_X, CONFIG_NICE_OLED_WIDGET_LUNA_CUSTOM_Y);
#endif

#if IS_ENABLED(CONFIG_NICE_OLED_SHOW_SLEEP_ART_ON_IDLE) ||                                         \
    IS_ENABLED(CONFIG_NICE_OLED_SHOW_SLEEP_ART_ON_SLEEP)
    zmk_widget_sleep_status_init(&sleep_status_widget, canvas);
    lv_obj_align(zmk_widget_sleep_status_obj(&sleep_status_widget), LV_ALIGN_TOP_LEFT, CONFIG_NICE_OLED_WIDGET_SLEEP_STATUS_CUSTOM_X, CONFIG_NICE_OLED_WIDGET_SLEEP_STATUS_CUSTOM_Y);
#endif

    return 0;
}

lv_obj_t *zmk_widget_screen_obj(struct zmk_widget_screen *widget) { return widget->obj; }
