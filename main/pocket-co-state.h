#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define POCKET_CO_THINK_MS 1100u
#define POCKET_CO_DIM_MS 60000u

typedef enum {
    POCKET_CO_IDLE,
    POCKET_CO_THINKING,
    POCKET_CO_ANSWER,
} pocket_co_phase_t;

typedef struct {
    pocket_co_phase_t phase;
    size_t answer;
    size_t previous;
    uint32_t started_ms;
    uint32_t activity_ms;
    bool dimmed;
} pocket_co_state_t;

void pocket_co_state_init(pocket_co_state_t *state, uint32_t now_ms);
bool pocket_co_press(pocket_co_state_t *state, uint32_t now_ms, uint32_t random, size_t count);
void pocket_co_return_home(pocket_co_state_t *state, uint32_t now_ms);
bool pocket_co_tick(pocket_co_state_t *state, uint32_t now_ms);
