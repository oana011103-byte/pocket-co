#pragma once
#include "lvgl.h"
#include "pocket-co-state.h"

bool pocket_co_fonts_valid(void);
void pocket_co_ui_create(void);
void pocket_co_ui_render(const pocket_co_state_t *state, uint32_t now_ms);
void pocket_co_ui_battery(int percent);
void pocket_co_ui_input_error(void);
