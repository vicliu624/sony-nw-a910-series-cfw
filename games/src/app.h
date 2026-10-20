#ifndef A918_APP_H
#define A918_APP_H
#include "blocks.h"
enum {K_LEFT=1,K_RIGHT,K_UP,K_DOWN,K_CENTER,K_BACK,K_OPTION};
enum {PAGE_HOME=0,PAGE_GAMES,PAGE_BLOCKS,PAGE_INFO,PAGE_P1,PAGE_COLOR,PAGE_COLOR_MENU,PAGE_COLOR_CONFIRM};
void app_init(uint32_t seed);
/* Companion page 2 only: requests a return instead of exposing placeholder Sony items. */
void app_enter_extension(void);
void app_enter_direction_extension(void);
void app_enter_boot(void);
void app_boot_choice_only(void);
void app_enter_games(void);
int app_games_requested(void);
int app_launch_code(void);
int app_return_requested(void);
void app_key(int key);
void app_key_release(int key);
int app_shutdown(void);
void app_tick(uint32_t ms);
void app_render(void);
uint16_t *app_pixels(void);
int app_page(void);
int app_selection(void);
int app_game_over(void);
uint32_t app_score(void);
int app_selftest(void);
void app_prompt(const char *first,const char *second);
#endif
