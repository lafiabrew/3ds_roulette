#include "pk_platform.h"
#include "roulette.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    char sd_root[512];
    roul_state_t st;
    char status_msg[128];
} roul_app_t;

static void app_init(roul_app_t *app, const char *sd_root) {
    memset(app, 0, sizeof(*app));
    snprintf(app->sd_root, sizeof(app->sd_root), "%s",
             sd_root != NULL ? sd_root : pk_platform_default_sd_root());
    roul_init(&app->st, 1000u);
    roul_place_bet(&app->st, ROUL_BET_STRAIGHT, 17, 25u);
    roul_place_bet(&app->st, ROUL_BET_RED, 0, 50u);
    roul_settle_outcome(&app->st, 17);
    snprintf(app->status_msg, sizeof(app->status_msg),
             "MONTE CARLO 3DS | A:BET STRAIGHT Y:BET RED X:SPIN WHEEL");
    pk_log_event(app->sd_root, "roulette", "INFO", app->status_msg);
}

static void app_step(roul_app_t *app, const pk_input_t *in) {
    if (in->down & PK_BTN_DLEFT && app->st.selected_number > 0) app->st.selected_number--;
    if (in->down & PK_BTN_DRIGHT && app->st.selected_number < 36) app->st.selected_number++;
    if (in->down & PK_BTN_DUP) app->st.selected_number = (app->st.selected_number + 3) % 37;
    if (in->down & PK_BTN_DDOWN) app->st.selected_number = (app->st.selected_number + 34) % 37;

    if (in->touch_just_pressed && in->touch_y >= 24 && in->touch_y < 144) {
        int col = (in->touch_x - 32) / 22;
        int row = (in->touch_y - 24) / 36;
        if (col >= 0 && col < 12 && row >= 0 && row < 3) {
            app->st.selected_number = col * 3 + (3 - row);
            roul_place_bet(&app->st, ROUL_BET_STRAIGHT, (uint8_t)app->st.selected_number, app->st.chip_value);
        }
    }

    if (in->down & PK_BTN_A) {
        if (roul_place_bet(&app->st, ROUL_BET_STRAIGHT, (uint8_t)app->st.selected_number, app->st.chip_value)) {
            snprintf(app->status_msg, sizeof(app->status_msg),
                     "BET $%u STRAIGHT ON #%d (35:1)",
                     (unsigned)app->st.chip_value, app->st.selected_number);
        }
    }
    if (in->down & PK_BTN_Y) {
        roul_place_bet(&app->st, ROUL_BET_RED, 0, app->st.chip_value);
        snprintf(app->status_msg, sizeof(app->status_msg), "BET $%u ON RED (1:1)", (unsigned)app->st.chip_value);
    }
    if (in->down & PK_BTN_L) {
        roul_place_bet(&app->st, ROUL_BET_BLACK, 0, app->st.chip_value);
        snprintf(app->status_msg, sizeof(app->status_msg), "BET $%u ON BLACK (1:1)", (unsigned)app->st.chip_value);
    }
    if (in->down & PK_BTN_B) {
        roul_clear_bets(&app->st);
        snprintf(app->status_msg, sizeof(app->status_msg), "CLEARED BETS ON TABLE");
    }
    if (in->down & PK_BTN_X) {
        uint8_t p = roul_spin_random(&app->st);
        int c = roul_pocket_color(p);
        snprintf(app->status_msg, sizeof(app->status_msg),
                 "WHEEL LANDED ON %u (%s) | PAYOUT: $%u | BANK: $%u",
                 (unsigned)p, c == 0 ? "GREEN" : (c == 1 ? "RED" : "BLACK"),
                 (unsigned)app->st.last_payout, (unsigned)app->st.bankroll);
        pk_log_event(app->sd_root, "roulette", "INFO", app->status_msg);
    }
}

static void app_render(const roul_app_t *app) {
    pk_render_begin_frame();
    pk_render_begin_target(PK_TARGET_TOP_LEFT, PK_RGBA8(10, 38, 22, 255));
    pk_draw_rect(0, 0, 400, 26, PK_RGBA8(18, 68, 38, 255));
    pk_draw_text(8, 7, 0.48f, PK_RGBA8(255, 220, 90, 255),
                 "ROULETTE 3DS - EUROPEAN 37-POCKET SIMULATOR");

    char line[140];
    snprintf(line, sizeof(line), "BANKROLL: $%u  |  CHIP: $%u  |  ACTIVE BETS: %d  |  SPINS: %u",
             (unsigned)app->st.bankroll, (unsigned)app->st.chip_value,
             app->st.bet_count, (unsigned)app->st.total_spins);
    pk_draw_text(10, 34, 0.45f, PK_RGBA8(230, 250, 235, 255), line);

    /* Wheel Outcome & History Marquee */
    int lcol = roul_pocket_color(app->st.last_outcome);
    uint32_t badge_col = (lcol == 0) ? PK_RGBA8(40, 185, 85, 255)
                       : (lcol == 1) ? PK_RGBA8(205, 42, 48, 255)
                                     : PK_RGBA8(28, 32, 42, 255);
    pk_draw_rect(20, 58, 120, 96, badge_col);
    pk_draw_rect_outline(20, 58, 120, 96, 2.0f, PK_RGBA8(245, 205, 80, 255));
    snprintf(line, sizeof(line), "LAST: #%u", (unsigned)app->st.last_outcome);
    pk_draw_text(34, 86, 0.55f, PK_RGBA8(255, 255, 255, 255), line);
    snprintf(line, sizeof(line), "WON: $%u", (unsigned)app->st.last_payout);
    pk_draw_text(34, 114, 0.48f, PK_RGBA8(255, 235, 110, 255), line);

    pk_draw_rect(154, 58, 232, 96, PK_RGBA8(16, 52, 30, 255));
    pk_draw_rect_outline(154, 58, 232, 96, 1.0f, PK_RGBA8(85, 185, 115, 255));
    pk_draw_text(164, 68, 0.45f, PK_RGBA8(255, 225, 110, 255), "PAYOUT TABLE & ODDS:");
    pk_draw_text(164, 88, 0.43f, PK_RGBA8(220, 245, 225, 255), "STRAIGHT UP (#0..36): 35 TO 1");
    pk_draw_text(164, 106, 0.43f, PK_RGBA8(220, 245, 225, 255), "DOZENS (1-12/13-24/25-36): 2 TO 1");
    pk_draw_text(164, 124, 0.43f, PK_RGBA8(220, 245, 225, 255), "RED / BLACK / EVEN / ODD: 1 TO 1");

    pk_draw_rect(0, 216, 400, 24, PK_RGBA8(16, 56, 32, 255));
    pk_draw_text(8, 222, 0.44f, PK_RGBA8(120, 255, 175, 255), app->status_msg);

    /* Bottom Touch Screen: 0 + 3x12 European Roulette Felt Grid */
    pk_render_begin_target(PK_TARGET_BOTTOM, PK_RGBA8(10, 34, 20, 255));
    pk_draw_rect(6, 24, 24, 106, PK_RGBA8(35, 165, 75, 255));
    pk_draw_text(12, 70, 0.48f, PK_RGBA8(255, 255, 255, 255), "0");

    for (int col = 0; col < 12; col++) {
        for (int row = 0; row < 3; row++) {
            int num = col * 3 + (3 - row);
            float bx = (float)(32 + col * 23);
            float by = (float)(24 + row * 36);
            int pc = roul_pocket_color((uint8_t)num);
            uint32_t bg = (pc == 1) ? PK_RGBA8(195, 38, 46, 255) : PK_RGBA8(26, 30, 40, 255);
            pk_draw_rect(bx, by, 21.0f, 34.0f, bg);
            if (num == app->st.selected_number) {
                pk_draw_rect_outline(bx, by, 21.0f, 34.0f, 2.0f, PK_RGBA8(255, 235, 80, 255));
            }
            char nstr[8];
            snprintf(nstr, sizeof(nstr), "%d", num);
            pk_draw_text(bx + 3.0f, by + 12.0f, 0.40f, PK_RGBA8(255, 255, 255, 255), nstr);
        }
    }
    pk_draw_rect(32, 138, 136, 30, PK_RGBA8(195, 38, 46, 255));
    pk_draw_text(64, 148, 0.46f, PK_RGBA8(255, 255, 255, 255), "Y: BET RED");
    pk_draw_rect(172, 138, 136, 30, PK_RGBA8(26, 30, 40, 255));
    pk_draw_text(196, 148, 0.46f, PK_RGBA8(255, 255, 255, 255), "L: BET BLACK");

    pk_draw_text(8, 218, 0.42f, PK_RGBA8(215, 245, 225, 255),
                 "TAP/A:BET # | X:SPIN WHEEL | B:CLEAR BETS");
    pk_render_end_frame();
}

int main(int argc, char **argv) {
    const char *sd_root = NULL;
    bool one_shot = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--sd-root") == 0 && i + 1 < argc) sd_root = argv[++i];
        else if (strcmp(argv[i], "--once") == 0) one_shot = true;
    }
    if (!pk_platform_init()) return 1;
    roul_app_t app;
    app_init(&app, sd_root);
    while (pk_platform_main_loop()) {
        pk_input_t in;
        pk_platform_poll_input(&in);
        if (in.down & PK_BTN_START) break;
        app_step(&app, &in);
        app_render(&app);
        if (one_shot) break;
    }
    pk_platform_shutdown();
    return 0;
}
