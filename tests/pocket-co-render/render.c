#include "pocket-co-ui.h"
#include "pocket-co-content.h"
#include "bsp_display_rounding.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LV_FONT_DECLARE(pocket_co_font_22);
static uint16_t s_pixels[240 * 320];
static uint16_t s_draw[240 * 20];

static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels)
{
    const int width = lv_area_get_width(area);
    for (int y = area->y1; y <= area->y2; ++y) {
        for (int x = area->x1; x <= area->x2; ++x) {
            if (bsp_display_pixel_outside_rounded_rect(x, y, 240, 320, 30))
                ((uint16_t *)pixels)[(y - area->y1) * width + x - area->x1] = 0;
        }
    }
    for (int y = area->y1; y <= area->y2; ++y)
        memcpy(s_pixels + y * 240 + area->x1,
            (uint16_t *)pixels + (y - area->y1) * width, (size_t)width * 2);
    lv_display_flush_ready(display);
}

static void capture(const char *directory, const char *name)
{
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(NULL);
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s.ppm", directory, name);
    FILE *file = fopen(path, "wb");
    assert(file);
    fprintf(file, "P6\n240 320\n255\n");
    for (int i = 0; i < 240 * 320; ++i) {
        unsigned v = s_pixels[i];
        unsigned char rgb[3] = {
            (unsigned char)(((v >> 11) & 31) * 255 / 31),
            (unsigned char)(((v >> 5) & 63) * 255 / 63),
            (unsigned char)((v & 31) * 255 / 31),
        };
        fwrite(rgb, 1, 3, file);
    }
    fclose(file);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    lv_init();
    lv_display_t *display = lv_display_create(240, 320);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, s_draw, NULL, sizeof(s_draw), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush);
    assert(pocket_co_fonts_valid());
    pocket_co_ui_create();
    pocket_co_ui_battery(100);
    pocket_co_state_t state;
    pocket_co_state_init(&state, 0);
    pocket_co_ui_render(&state, 0);
    capture(argv[1], "home");
    assert(pocket_co_press(&state, 0, 0, pocket_co_answer_count));
    pocket_co_ui_render(&state, 550);
    capture(argv[1], "thinking");
    assert(pocket_co_tick(&state, POCKET_CO_THINK_MS));
    pocket_co_ui_render(&state, POCKET_CO_THINK_MS + 320);
    capture(argv[1], "answer");
    int tallest = 0;
    size_t tallest_index = 0;
    int widest = 0;
    for (size_t i = 0; i < pocket_co_answer_count; ++i) {
        lv_point_t size;
        lv_text_get_size(&size, pocket_co_answers[i], &pocket_co_font_22, 0, 5, 204, LV_TEXT_FLAG_NONE);
        if (size.y > tallest || (size.y == tallest && size.x > widest)) {
            tallest = size.y; tallest_index = i; widest = size.x;
        }
        // Each authored line must fit: avoid a punctuation-only wrapped line.
        const char *line = pocket_co_answers[i];
        while (*line) {
            const char *end = strchr(line, '\n');
            size_t length = end ? (size_t)(end - line) : strlen(line);
            char buffer[128];
            assert(length < sizeof(buffer));
            memcpy(buffer, line, length);
            buffer[length] = 0;
            lv_point_t line_size;
            lv_text_get_size(&line_size, buffer, &pocket_co_font_22, 0, 5, 204, LV_TEXT_FLAG_EXPAND);
            if (line_size.x > 204) {
                fprintf(stderr, "Answer %zu has an overlong authored line: %s\n", i, buffer);
                return 1;
            }
            if (!end) break;
            line = end + 1;
        }
        if (size.y > 82) {
            fprintf(stderr, "Answer %zu overflows: %d px: %s\n", i, (int)size.y, pocket_co_answers[i]);
            return 1;
        }
        state.answer = i;
        pocket_co_ui_render(&state, 1200);
        lv_refr_now(NULL);
    }
    state.answer = tallest_index;
    pocket_co_ui_render(&state, 1200);
    capture(argv[1], "longest-answer");
    // Exercise the actual frame tables through idle, draw and completion.
    pocket_co_state_init(&state, 0);
    for (uint32_t time = 0; time < 8400; time += 80) {
        if (time == 4000) assert(pocket_co_press(&state, time, 0, pocket_co_answer_count));
        if (time == 7200) pocket_co_return_home(&state, time);
        pocket_co_tick(&state, time);
        pocket_co_ui_render(&state, time);
        char name[64];
        snprintf(name, sizeof(name), "motion-%03u", (unsigned)(time / 80));
        capture(argv[1], name);
    }
    pocket_co_return_home(&state, 8400);
    pocket_co_ui_render(&state, 8400);
    capture(argv[1], "returned-home");
    puts("LVGL host render: PASS (linked font coverage and all 110 answer heights)");
    printf("Tallest answer: %zu, %d/82 pixels\n", tallest_index, tallest);
    return 0;
}
