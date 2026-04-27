#include "util.h"
#include <ctype.h>
#include <string.h>
#include <zephyr/kernel.h>

void to_uppercase(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = toupper(str[i]);
  }
}

void rotate_canvas(lv_obj_t *canvas, lv_color_t cbuf[]) {
  /* For an I1 canvas, the buffer holds a packed 1bpp bitmap (no palette).
   * lv_image_dsc_t with cf=I1 expects palette bytes (2 colors x 4 BGRA = 8 bytes)
   * prepended to the bitmap data. Build a temp image with that layout so
   * lv_draw_image can render it back rotated.
   */
  enum {
    BITMAP_BYTES  = (CANVAS_HEIGHT * CANVAS_HEIGHT) / 8,
    PALETTE_BYTES = 8,
  };
  static uint8_t cbuf_tmp[PALETTE_BYTES + BITMAP_BYTES];

  /* Palette must match the canvas's palette setup in screen*.c:
   *   idx 0 = white (BGRA 0xFF 0xFF 0xFF 0xFF)
   *   idx 1 = black (BGRA 0x00 0x00 0x00 0xFF)
   * Inversion is handled at color-write time via LVGL_BACKGROUND/FOREGROUND;
   * the bitmap bits already encode which palette index each pixel uses.
   */
  cbuf_tmp[0] = 0xff; cbuf_tmp[1] = 0xff; cbuf_tmp[2] = 0xff; cbuf_tmp[3] = 0xff;
  cbuf_tmp[4] = 0x00; cbuf_tmp[5] = 0x00; cbuf_tmp[6] = 0x00; cbuf_tmp[7] = 0xff;

  memcpy(&cbuf_tmp[PALETTE_BYTES], cbuf, BITMAP_BYTES);

  lv_image_dsc_t img;
  memset(&img, 0, sizeof(img));
  img.data = cbuf_tmp;
  img.data_size = sizeof(cbuf_tmp);
  img.header.cf = LV_COLOR_FORMAT_I1;
  img.header.w = CANVAS_HEIGHT;
  img.header.h = CANVAS_HEIGHT;

  lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

  lv_layer_t layer;
  lv_canvas_init_layer(canvas, &layer);

  lv_draw_image_dsc_t img_dsc;
  lv_draw_image_dsc_init(&img_dsc);
  img_dsc.src = &img;
  img_dsc.rotation = 900;
  img_dsc.scale_x = LV_SCALE_NONE;
  img_dsc.scale_y = LV_SCALE_NONE;
  img_dsc.pivot.x = CANVAS_HEIGHT / 2;
  img_dsc.pivot.y = CANVAS_HEIGHT / 2;
  img_dsc.antialias = 0;

  lv_area_t coords = {0, 0, CANVAS_HEIGHT - 1, CANVAS_HEIGHT - 1};
  lv_draw_image(&layer, &img_dsc, &coords);

  lv_canvas_finish_layer(canvas, &layer);
}

void draw_background(lv_obj_t *canvas) {
  lv_draw_rect_dsc_t rect_black_dsc;
  init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);

  canvas_draw_rect(canvas, 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT,
                   &rect_black_dsc);
}

void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color,
                    const lv_font_t *font, lv_text_align_t align) {
  lv_draw_label_dsc_init(label_dsc);
  label_dsc->color = color;
  label_dsc->font = font;
  label_dsc->align = align;
}

void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color) {
  lv_draw_rect_dsc_init(rect_dsc);
  rect_dsc->bg_color = bg_color;
}

void init_line_dsc(lv_draw_line_dsc_t *line_dsc, lv_color_t color,
                   uint8_t width) {
  lv_draw_line_dsc_init(line_dsc);
  line_dsc->color = color;
  line_dsc->width = width;
}

void canvas_draw_text(lv_obj_t *canvas, int32_t x, int32_t y, int32_t max_w,
                      lv_draw_label_dsc_t *dsc, const char *text) {
  lv_layer_t layer;
  lv_canvas_init_layer(canvas, &layer);
  dsc->text = text;
  lv_area_t coords = {x, y, x + max_w - 1, y + 1024};
  lv_draw_label(&layer, dsc, &coords);
  lv_canvas_finish_layer(canvas, &layer);
}

void canvas_draw_img(lv_obj_t *canvas, int32_t x, int32_t y,
                     const lv_image_dsc_t *src,
                     lv_draw_image_dsc_t *dsc) {
  lv_layer_t layer;
  lv_canvas_init_layer(canvas, &layer);
  dsc->src = src;
  lv_area_t coords = {x, y, x + src->header.w - 1, y + src->header.h - 1};
  lv_draw_image(&layer, dsc, &coords);
  lv_canvas_finish_layer(canvas, &layer);
}

void canvas_draw_rect(lv_obj_t *canvas, int32_t x, int32_t y, int32_t w,
                      int32_t h, lv_draw_rect_dsc_t *dsc) {
  lv_layer_t layer;
  lv_canvas_init_layer(canvas, &layer);
  lv_area_t coords = {x, y, x + w - 1, y + h - 1};
  lv_draw_rect(&layer, dsc, &coords);
  lv_canvas_finish_layer(canvas, &layer);
}

void canvas_draw_line(lv_obj_t *canvas, lv_point_t *points, uint32_t n,
                      lv_draw_line_dsc_t *dsc) {
  if (n < 2) return;
  lv_layer_t layer;
  lv_canvas_init_layer(canvas, &layer);
  for (uint32_t i = 0; i < n - 1; i++) {
    dsc->p1 = lv_point_to_precise(&points[i]);
    dsc->p2 = lv_point_to_precise(&points[i + 1]);
    lv_draw_line(&layer, dsc);
  }
  lv_canvas_finish_layer(canvas, &layer);
}
