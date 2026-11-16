#include "pocket-co-state.h"
#include <stdint.h>

void pocket_co_state_init(pocket_co_state_t *state, uint32_t now_ms)
{
    *state = (pocket_co_state_t){
        .phase = POCKET_CO_IDLE,
        .previous = SIZE_MAX,
        .activity_ms = now_ms,
    };
}

bool pocket_co_press(pocket_co_state_t *state, uint32_t now_ms, uint32_t random, size_t count)
{
    if (count == 0 || state->phase == POCKET_CO_THINKING) return false;
    state->activity_ms = now_ms;
    if (state->dimmed) {
        state->dimmed = false;
        return true; // The first press restores the answer instead of replacing it.
    }
    size_t next;
    if (state->previous < count && count > 1) {
        next = random % (count - 1);
        if (next >= state->previous) ++next;
    } else {
        next = random % count;
    }
    state->answer = next;
    state->started_ms = now_ms;
    state->phase = POCKET_CO_THINKING;
    return true;
}

void pocket_co_return_home(pocket_co_state_t *state, uint32_t now_ms)
{
    // Keep the last revealed answer so returning home cannot bypass repeat prevention.
    state->phase = POCKET_CO_IDLE;
    state->started_ms = now_ms;
    state->activity_ms = now_ms;
    state->dimmed = false;
}

bool pocket_co_tick(pocket_co_state_t *state, uint32_t now_ms)
{
    bool changed = false;
    if (state->phase == POCKET_CO_THINKING &&
        (uint32_t)(now_ms - state->started_ms) >= POCKET_CO_THINK_MS) {
        state->phase = POCKET_CO_ANSWER;
        state->previous = state->answer;
        state->activity_ms = now_ms;
        changed = true;
    }
    if (!state->dimmed && state->phase != POCKET_CO_THINKING &&
        (uint32_t)(now_ms - state->activity_ms) >= POCKET_CO_DIM_MS) {
        state->dimmed = true;
        changed = true;
    }
    return changed;
}
