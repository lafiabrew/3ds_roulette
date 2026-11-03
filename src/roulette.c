#include "roulette.h"

#include <string.h>

/* Standard European Roulette red numbers:
   1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36 */
int roul_pocket_color(uint8_t pocket) {
    if (pocket == 0) return 0; /* Green */
    if (pocket > 36) return -1;
    static const uint8_t k_reds[18] = {
        1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36
    };
    for (int i = 0; i < 18; i++) {
        if (k_reds[i] == pocket) return 1; /* Red */
    }
    return 2; /* Black */
}

void roul_init(roul_state_t *st, uint32_t initial_bankroll) {
    if (st == NULL) return;
    memset(st, 0, sizeof(*st));
    st->bankroll = initial_bankroll > 0 ? initial_bankroll : 1000u;
    st->chip_value = 25u;
    st->selected_number = 17;
    st->rng_state = 0x7773D500u;
}

bool roul_place_bet(roul_state_t *st, roul_bet_type_t type, uint8_t number, uint32_t wager) {
    if (st == NULL || wager == 0 || wager > st->bankroll || st->bet_count >= ROUL_MAX_BETS) {
        return false;
    }
    if ((unsigned)type > (unsigned)ROUL_BET_DOZEN_3) return false;
    if (type == ROUL_BET_STRAIGHT && number > 36u) return false;

    st->bankroll -= wager;
    roul_bet_t b;
    b.type = type;
    b.number = number;
    b.wager = wager;
    st->bets[st->bet_count++] = b;
    return true;
}

void roul_clear_bets(roul_state_t *st) {
    if (st == NULL) return;
    for (int i = 0; i < st->bet_count; i++) {
        st->bankroll += st->bets[i].wager;
    }
    st->bet_count = 0;
}

uint32_t roul_settle_outcome(roul_state_t *st, uint8_t p) {
    if (st == NULL || p > 36u) return 0;
    uint32_t total_return = 0;
    int col = roul_pocket_color(p);

    for (int i = 0; i < st->bet_count; i++) {
        const roul_bet_t *b = &st->bets[i];
        uint32_t mult = 0; /* total return multiplier (e.g. 36x = 35:1 profit + 1x original stake) */
        switch (b->type) {
        case ROUL_BET_STRAIGHT:
            if (p == b->number) mult = 36u;
            break;
        case ROUL_BET_RED:
            if (col == 1) mult = 2u;
            break;
        case ROUL_BET_BLACK:
            if (col == 2) mult = 2u;
            break;
        case ROUL_BET_EVEN:
            if (p != 0 && (p % 2u) == 0u) mult = 2u;
            break;
        case ROUL_BET_ODD:
            if (p != 0 && (p % 2u) == 1u) mult = 2u;
            break;
        case ROUL_BET_LOW_1_18:
            if (p >= 1u && p <= 18u) mult = 2u;
            break;
        case ROUL_BET_HI_19_36:
            if (p >= 19u && p <= 36u) mult = 2u;
            break;
        case ROUL_BET_DOZEN_1:
            if (p >= 1u && p <= 12u) mult = 3u;
            break;
        case ROUL_BET_DOZEN_2:
            if (p >= 13u && p <= 24u) mult = 3u;
            break;
        case ROUL_BET_DOZEN_3:
            if (p >= 25u && p <= 36u) mult = 3u;
            break;
        }
        total_return += b->wager * mult;
    }

    st->bankroll += total_return;
    st->last_outcome = p;
    st->last_payout = total_return;
    st->bet_count = 0;
    st->total_spins++;

    /* Prepend to spin history */
    for (int i = ROUL_HIST_LEN - 1; i > 0; i--) {
        st->history[i] = st->history[i - 1];
    }
    st->history[0] = p;
    if (st->hist_count < ROUL_HIST_LEN) st->hist_count++;
    return total_return;
}

uint8_t roul_spin_random(roul_state_t *st) {
    if (st == NULL) return 0;
    st->rng_state = st->rng_state * 1664525u + 1013904223u;
    uint8_t pocket = (uint8_t)((st->rng_state >> 16) % 37u);
    roul_settle_outcome(st, pocket);
    return pocket;
}
