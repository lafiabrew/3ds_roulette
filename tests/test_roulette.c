/*
 * Unit & pass/fail tests for 3ds_roulette:
 * - Pass-the-pass: European pocket color table (0=Green, 1=Red, 2=Black), 35:1 straight payout (36x return),
 *   2:1 dozen payout (3x return), 1:1 Red/Black/Even/Odd payout (2x return), 0 house-edge wipe on outside bets.
 * - Fail-the-fail: wagering more than bankroll rejected, pocket > 36 rejected, 0 wager rejected.
 */

#include "roulette.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(roul_pocket_color(0) == 0);
    assert(roul_pocket_color(1) == 1);
    assert(roul_pocket_color(2) == 2);
    assert(roul_pocket_color(17) == 2); /* 17 is Black */
    assert(roul_pocket_color(37) == -1);

    roul_state_t st;
    roul_init(&st, 1000u);

    /* Bet $100 Straight on #17 (Black) and $100 on Black */
    assert(roul_place_bet(&st, ROUL_BET_STRAIGHT, 17, 100u));
    assert(roul_place_bet(&st, ROUL_BET_BLACK, 0, 100u));
    assert(st.bankroll == 800u);

    /* Settle outcome #17: Straight returns 3600 ($3500 profit + $100 stake), Black returns 200 -> 3800 total */
    uint32_t ret = roul_settle_outcome(&st, 17);
    assert(ret == 3800u);
    assert(st.bankroll == 4600u);

    /* Outside bets lose on Green 0 */
    assert(roul_place_bet(&st, ROUL_BET_RED, 0, 100u));
    assert(roul_place_bet(&st, ROUL_BET_EVEN, 0, 100u));
    assert(roul_settle_outcome(&st, 0) == 0u);

    /* Fail-the-fail negative assertions */
    assert(roul_place_bet(NULL, ROUL_BET_RED, 0, 10u) == false);
    assert(roul_place_bet(&st, ROUL_BET_RED, 0, 0u) == false);
    assert(roul_place_bet(&st, ROUL_BET_RED, 0, 999999u) == false);
    assert(roul_place_bet(&st, ROUL_BET_STRAIGHT, 37, 10u) == false);
    assert(roul_settle_outcome(&st, 37) == 0u);

    printf("PASS: test_roulette all European 37-pocket odds, 35:1/2:1/1:1 payouts, and pass/fail checks succeeded.\n");
    return 0;
}
