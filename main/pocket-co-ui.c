#include "pocket-co-ui.h"
#include "pocket-co-content.h"
#include "pocket-co-animation.h"

LV_FONT_DECLARE(pocket_co_font_16);
LV_FONT_DECLARE(pocket_co_font_22);


#define CO_BG 0x211C18
#define CO_PAPER 0xFFF4E7
#define CO_MUTED 0xC4B6A8
#define CO_ORANGE 0xF1752D

static lv_obj_t *s_root;
static lv_obj_t *s_battery;
static lv_obj_t *s_mascot;
static lv_obj_t *s_title;
static lv_obj_t *s_body;
static lv_obj_t *s_hint;
static lv_obj_t *s_dots[3];
static const lv_image_dsc_t *s_frame;
static int s_phase = -1;
static size_t s_answer = SIZE_MAX;

static bool text_has_glyphs(const lv_font_t *font, const char *text)
{
    const unsigned char *p = (const unsigned char *)text;
    while (*p) {
        uint32_t cp = *p++;
        if (cp >= 0xF0) {
            cp &= 7;
            for (int i = 0; i < 3; ++i) cp = (cp << 6) | (*p++ & 63);
        } else if (cp >= 0xE0) {
            cp &= 15;
            for (int i = 0; i < 2; ++i) cp = (cp << 6) | (*p++ & 63);
        } else if (cp >= 0xC0) {
            cp = ((cp & 31) << 6) | (*p++ & 63);
        }
        if (cp == '\n' || cp == '\r') continue;
        lv_font_glyph_dsc_t glyph = {0};
        if (!lv_font_get_glyph_dsc(font, &glyph, cp, 0) || glyph.is_placeholder) return false;
    }
    return true;
}

bool pocket_co_fonts_valid(void)
{
    const char *const labels[] = {
        CO_TITLE, CO_IDLE_TITLE, CO_IDLE_TEXT, CO_IDLE_HINT,
        CO_THINK_TITLE, CO_THINK_TEXT, CO_THINK_HINT,
        CO_ANSWER_HINT, CO_ERROR_TITLE, CO_ERROR_TEXT,
    };
    for (size_t i = 0; i < sizeof(labels) / sizeof(labels[0]); ++i) {
        if (!text_has_glyphs(&pocket_co_font_16, labels[i]) ||
            !text_has_glyphs(&pocket_co_font_22, labels[i])) return false;
    }
    for (size_t i = 0; i < pocket_co_answer_count; ++i) {
        if (!text_has_glyphs(&pocket_co_font_22, pocket_co_answers[i])) return false;
    }
    return true;
}

static lv_obj_t *label(int x, int y, int w, int h, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *obj = lv_label_create(s_root);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(obj, 5, 0);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
    lv_label_set_text(obj, "");
    return obj;
}

void pocket_co_ui_create(void)
{
    s_root = lv_obj_create(NULL);
    lv_obj_remove_style_all(s_root);
    lv_obj_set_size(s_root, 240, 320);
    lv_obj_remove_flag(s_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_root, lv_color_hex(CO_BG), 0);
    lv_obj_set_style_bg_opa(s_root, LV_OPA_COVER, 0);

    lv_obj_t *name = label(27, 25, 136, 24, &pocket_co_font_16, CO_PAPER);
    lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(name, CO_TITLE);
    s_battery = label(168, 26, 48, 20, &lv_font_montserrat_14, CO_MUTED);
    lv_obj_set_style_text_align(s_battery, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(s_battery, "--");

    s_mascot = lv_image_create(s_root);
    lv_obj_set_pos(s_mascot, 56, 60);
    s_frame = NULL;
    s_phase = -1;
    s_answer = SIZE_MAX;
    s_title = label(18, 188, 204, 34, &pocket_co_font_22, CO_PAPER);
    s_body = label(18, 228, 204, 52, &pocket_co_font_16, CO_MUTED);
    s_hint = label(18, 284, 204, 26, &pocket_co_font_16, CO_ORANGE);
    for (int i = 0; i < 3; ++i) {
        s_dots[i] = lv_obj_create(s_root);
        lv_obj_remove_style_all(s_dots[i]);
        lv_obj_set_size(s_dots[i], 6, 6);
        lv_obj_set_pos(s_dots[i], 101 + i * 16, 251);
        lv_obj_set_style_radius(s_dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(s_dots[i], lv_color_hex(CO_ORANGE), 0);
    }
    lv_screen_load(s_root);
}

void pocket_co_ui_render(const pocket_co_state_t *state, uint32_t now_ms)
{
    const bool thinking = state->phase == POCKET_CO_THINKING;
    const bool answer = state->phase == POCKET_CO_ANSWER;
    const lv_image_dsc_t *frame;
    if (thinking) {
        uint32_t index = (now_ms - state->started_ms) / POCKET_CO_FRAME_MS;
        if (index >= POCKET_CO_THINKING_FRAME_COUNT) index = POCKET_CO_THINKING_FRAME_COUNT - 1;
        frame = pocket_co_thinking_frames[index];
    } else if (answer) {
        uint32_t elapsed = now_ms - state->started_ms - POCKET_CO_THINK_MS;
        if (elapsed < POCKET_CO_COMPLETE_FRAME_COUNT * POCKET_CO_FRAME_MS) {
            frame = pocket_co_complete_frames[elapsed / POCKET_CO_FRAME_MS];
        } else {
            uint32_t idle = elapsed - POCKET_CO_COMPLETE_FRAME_COUNT * POCKET_CO_FRAME_MS;
            frame = pocket_co_idle_frames[(idle / POCKET_CO_FRAME_MS) % POCKET_CO_IDLE_FRAME_COUNT];
        }
    } else {
        frame = pocket_co_idle_frames[(now_ms / POCKET_CO_FRAME_MS) % POCKET_CO_IDLE_FRAME_COUNT];
    }
    if (s_frame != frame) {
        lv_image_set_src(s_mascot, frame);
        s_frame = frame;
    }
    // Keep text allocation and layout out of the animation-only updates.
    if (s_phase != (int)state->phase || (answer && s_answer != state->answer)) {
        s_phase = (int)state->phase;
        s_answer = state->answer;
        if (answer) {
            lv_obj_add_flag(s_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(s_body, 18, 190);
            lv_obj_set_size(s_body, 204, 82);
            lv_obj_set_style_text_font(s_body, &pocket_co_font_22, 0);
            lv_obj_set_style_text_color(s_body, lv_color_hex(CO_PAPER), 0);
            lv_label_set_text(s_body, pocket_co_answers[state->answer]);
            lv_label_set_text(s_hint, CO_ANSWER_HINT);
        } else {
            lv_obj_remove_flag(s_title, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(s_title, thinking ? CO_THINK_TITLE : CO_IDLE_TITLE);
            lv_obj_set_pos(s_body, 18, thinking ? 229 : 228);
            lv_obj_set_size(s_body, 204, 52);
            lv_obj_set_style_text_font(s_body, &pocket_co_font_16, 0);
            lv_obj_set_style_text_color(s_body, lv_color_hex(CO_MUTED), 0);
            lv_label_set_text(s_body, thinking ? CO_THINK_TEXT : CO_IDLE_TEXT);
            lv_label_set_text(s_hint, thinking ? CO_THINK_HINT : CO_IDLE_HINT);
        }
    }
    for (int i = 0; i < 3; ++i) {
        if (thinking) {
            lv_obj_remove_flag(s_dots[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_opa(s_dots[i],
                (int)(((now_ms - state->started_ms) / 180) % 3) == i ? LV_OPA_COVER : LV_OPA_30, 0);
        } else {
            lv_obj_add_flag(s_dots[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void pocket_co_ui_battery(int percent)
{
    if (percent >= 0 && percent <= 100) lv_label_set_text_fmt(s_battery, "%d%%", percent);
    else lv_label_set_text(s_battery, "--");
}

void pocket_co_ui_input_error(void)
{
    lv_label_set_text(s_title, CO_ERROR_TITLE);
    lv_label_set_text(s_body, CO_ERROR_TEXT);
    lv_label_set_text(s_hint, "");
}
