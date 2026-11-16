#include "pocket-co-state.h"
#include "pocket-co-content.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    pocket_co_state_t s;
    pocket_co_state_init(&s, 0);
    assert(s.phase == POCKET_CO_IDLE);
    assert(!pocket_co_press(&s, 0, 0, 0));
    assert(pocket_co_answer_count == 110);
    for (size_t i = 0; i < pocket_co_answer_count; ++i) {
        assert(strlen(pocket_co_answers[i]) > 0);
        for (size_t j = i + 1; j < pocket_co_answer_count; ++j)
            assert(strcmp(pocket_co_answers[i], pocket_co_answers[j]) != 0);
    }
    size_t previous = SIZE_MAX;
    unsigned reached[110] = {0};
    uint32_t now = 0;
    for (uint32_t i = 0; i < 20000; ++i) {
        assert(pocket_co_press(&s, now, i * 2654435761u, pocket_co_answer_count));
        assert(s.phase == POCKET_CO_THINKING && s.answer < pocket_co_answer_count);
        size_t chosen = s.answer;
        assert(!pocket_co_press(&s, now + 200, 50, pocket_co_answer_count));
        assert(s.answer == chosen);
        assert(!pocket_co_tick(&s, now + POCKET_CO_THINK_MS - 1));
        assert(pocket_co_tick(&s, now + POCKET_CO_THINK_MS));
        assert(s.phase == POCKET_CO_ANSWER && s.answer != previous);
        previous = s.answer;
        reached[s.answer]++;
        now += 2000;
    }
    for (size_t i = 0; i < pocket_co_answer_count; ++i) assert(reached[i] > 0);
    now = s.activity_ms + POCKET_CO_DIM_MS;
    assert(pocket_co_tick(&s, now) && s.dimmed);
    previous = s.answer;
    assert(pocket_co_press(&s, now + 1, 0, pocket_co_answer_count));
    assert(!s.dimmed && s.phase == POCKET_CO_ANSWER && s.answer == previous);
    assert(pocket_co_press(&s, now + 2, 0, pocket_co_answer_count));
    assert(s.phase == POCKET_CO_THINKING);
    pocket_co_state_init(&s, UINT32_MAX - 500);
    assert(pocket_co_press(&s, UINT32_MAX - 500, UINT32_MAX, 1));
    assert(pocket_co_tick(&s, 600));
    assert(s.phase == POCKET_CO_ANSWER && s.answer == 0);
    assert(pocket_co_press(&s, 601, 0, 1));
    assert(pocket_co_tick(&s, 1701));
    assert(s.answer == 0);
    // Returning from an answer preserves repeat prevention across home visits.
    previous = s.previous;
    pocket_co_return_home(&s, 2000);
    assert(s.phase == POCKET_CO_IDLE && !s.dimmed && s.previous == previous);
    assert(s.activity_ms == 2000);
    assert(!pocket_co_tick(&s, 2000 + POCKET_CO_THINK_MS));
    assert(pocket_co_press(&s, 3200, 0, pocket_co_answer_count));
    assert(s.answer != previous);
    // A return during thinking cancels the pending answer and its timer.
    pocket_co_return_home(&s, 3300);
    assert(!pocket_co_tick(&s, 5000));
    assert(s.phase == POCKET_CO_IDLE && s.previous == previous);
    assert(pocket_co_tick(&s, 3300 + POCKET_CO_DIM_MS));
    assert(s.dimmed);
    pocket_co_return_home(&s, 3301 + POCKET_CO_DIM_MS);
    assert(s.phase == POCKET_CO_IDLE && !s.dimmed);
    assert(!pocket_co_tick(&s, 3301 + POCKET_CO_DIM_MS));
    // Returning again on the cover refreshes the idle timer without drawing.
    pocket_co_return_home(&s, 70000);
    assert(s.activity_ms == 70000 && s.phase == POCKET_CO_IDLE);
    puts("Pocket co state: PASS (20,000 draws, repeat/busy/dim/home/cancel/clock-wrap checks)");
}
