#ifndef ROULETTE_H
#define ROULETTE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ROUL_MAX_BETS 16
#define ROUL_HIST_LEN 12

typedef enum {
    ROUL_BET_STRAIGHT = 0, /* 35:1 on exact number 0..36 */
    ROUL_BET_RED      = 1, /* 1:1 */
    ROUL_BET_BLACK    = 2, /* 1:1 */
    ROUL_BET_EVEN     = 3, /* 1:1 (1..36 even) */
    ROUL_BET_ODD      = 4, /* 1:1 (1..36 odd) */
    ROUL_BET_LOW_1_18 = 5, /* 1:1 */
    ROUL_BET_HI_19_36 = 6, /* 1:1 */
    ROUL_BET_DOZEN_1  = 7, /* 2:1 (1..12) */
    ROUL_BET_DOZEN_2  = 8, /* 2:1 (13..24) */
    ROUL_BET_DOZEN_3  = 9  /* 2:1 (25..36) */
} roul_bet_type_t;

typedef struct {
    roul_bet_type_t type;
    uint8_t number; /* 0..36 for ROUL_BET_STRAIGHT */
    uint32_t wager;
} roul_bet_t;

typedef struct {
    uint32_t bankroll;
    uint32_t chip_value;
    int selected_number;
    roul_bet_t bets[ROUL_MAX_BETS];
    int bet_count;
    uint8_t history[ROUL_HIST_LEN];
    int hist_count;
    uint8_t last_outcome;
    uint32_t last_payout;
    uint32_t rng_state;
    uint32_t total_spins;
} roul_state_t;

void roul_init(roul_state_t *st, uint32_t initial_bankroll);
/* Returns 0=Green(0), 1=Red, 2=Black according to standard European Roulette wheel */
int roul_pocket_color(uint8_t pocket);
bool roul_place_bet(roul_state_t *st, roul_bet_type_t type, uint8_t number, uint32_t wager);
void roul_clear_bets(roul_state_t *st);
/* Settle all active bets against `outcome_pocket` (0..36) and return total chips returned (stake + winnings) */
uint32_t roul_settle_outcome(roul_state_t *st, uint8_t outcome_pocket);
uint8_t roul_spin_random(roul_state_t *st);

#endif /* ROULETTE_H */
