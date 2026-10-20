#include "app.h"
static uint16_t framebuffer[240*320];static int page,selection,count,done,request,launchcode;
extern int launcher_scan(void);extern const char*launcher_name(int);
#include "draw.inc"
void app_init(uint32_t seed){(void)seed;page=PAGE_HOME;selection=count=done=request=launchcode=0;}
void app_enter_boot(void){page=PAGE_HOME;}
void app_boot_choice_only(void){}
void app_enter_games(void){page=PAGE_GAMES;selection=0;count=launcher_scan();}
void app_enter_extension(void){}void app_enter_direction_extension(void){}
int app_games_requested(void){return request;}int app_launch_code(void){return launchcode;}
int app_return_requested(void){return done;}uint16_t*app_pixels(void){return framebuffer;}int app_page(void){return page;}int app_selection(void){return selection;}int app_game_over(void){return 0;}uint32_t app_score(void){return 0;}
void app_key(int key){if(page==PAGE_HOME){if(key==K_UP||key==K_LEFT)selection=0;else if(key==K_DOWN||key==K_RIGHT)selection=1;else if(key==K_CENTER){request=selection;done=1;}return;}
 if(key==K_UP||key==K_LEFT){if(selection)selection--;}else if(key==K_DOWN||key==K_RIGHT){if(selection<count+1)selection++;}else if(key==K_CENTER){if(selection==count){count=launcher_scan();selection=0;}else if(selection==count+1)done=1;else{launchcode=100+selection;done=1;}}}
void app_key_release(int key){(void)key;}int app_shutdown(void){return 1;}void app_tick(uint32_t ms){(void)ms;}
void app_prompt(const char*a,const char*b){rectangle(0,0,240,320,0x0843);text(12,80,a,0xffff,1);text(12,110,b,0xffff,1);}
void app_render(void){rectangle(0,0,240,320,0x0843);rectangle(0,0,240,27,0x19a8);text(12,9,page==PAGE_HOME?"SELECT":"APPLICATIONS",0xffff,1);
 if(page==PAGE_HOME){text(30,110,selection==0?"> WALKMAN":"WALKMAN",0xffff,2);text(30,165,selection==1?"> APPS":"APPS",0xffff,2);text(12,285,"AUTO WALKMAN IN 8 SECONDS",0x07ff,1);return;}
 int first=selection/5*5;for(int i=first;i<count+2&&i<first+5;i++){int y=45+(i-first)*40;rectangle(10,y,220,32,i==selection?0x3a6f:0x1106);text(17,y+12,i<count?launcher_name(i):i==count?"RESCAN":"RETURN TO WALKMAN",0xffff,1);}if(!count)text(12,267,"NO APPS - COPY TO APPS FOLDER",0xbdf7,1);text(12,295,"DIRECTIONS SELECT  CENTER OPEN",0x07ff,1);}
int boot_selftest(void){app_init(1);app_enter_boot();app_key(K_DOWN);app_key(K_CENTER);return !app_games_requested();}
