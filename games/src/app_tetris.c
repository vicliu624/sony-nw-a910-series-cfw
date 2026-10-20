#include "app.h"
static uint16_t framebuffer[240*320];static Blocks game;static int done;
#include "draw.inc"
void app_init(uint32_t seed){blocks_new(&game,seed?seed:918);done=0;}
void app_enter_boot(void){}void app_enter_games(void){}void app_boot_choice_only(void){}void app_enter_extension(void){}void app_enter_direction_extension(void){}
int app_games_requested(void){return 0;}int app_return_requested(void){return done;}uint16_t*app_pixels(void){return framebuffer;}int app_page(void){return PAGE_BLOCKS;}int app_selection(void){return 0;}int app_game_over(void){return game.over;}uint32_t app_score(void){return game.score;}
void app_key(int k){if(k==K_CENTER)done=1;else if(k==K_LEFT)blocks_move(&game,-1,0);else if(k==K_RIGHT)blocks_move(&game,1,0);else if(k==K_UP)blocks_rotate(&game);else if(k==K_DOWN)blocks_move(&game,0,1);}
void app_key_release(int k){(void)k;}int app_shutdown(void){return 1;}void app_tick(uint32_t ms){blocks_tick(&game,ms);}
void app_prompt(const char*a,const char*b){rectangle(0,0,240,320,0x0843);text(12,80,a,0xffff,1);text(12,110,b,0xffff,1);}
void app_render(void){rectangle(0,0,240,320,0x0843);rectangle(0,0,240,27,0x19a8);
#include "tetris_draw.inc"
}
