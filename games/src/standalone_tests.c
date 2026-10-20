#include "app.h"
int boot_selftest(void){
#ifdef A918_APP_COLOR
 return 0;
#else
 for(int i=0;i<200;i++){app_init(i+1);app_enter_games();if(app_page()!=PAGE_BLOCKS)return 1;app_key(K_DOWN);app_tick(800);app_render();app_key(K_CENTER);if(!app_return_requested())return 2;}return 0;
#endif
}

#ifdef A918_APP_COLOR
#include "color.h"
int standalone_disk_test(void){
 if(!color_open()||color_status()!=COLOR_OK)return 10;
 for(int i=0;i<80;i++)color_tick(24);
 color_button(1,1);color_button(1,0);for(int i=0;i<30;i++)color_tick(24);
 if(!color_save())return 11;
 if(!color_new_game())return 12;
 if(!color_restore_backup())return 13;
 color_close();if(!color_open()||color_status()!=COLOR_OK)return 14;color_close();return 0;
}
#else
int standalone_disk_test(void){return boot_selftest();}
#endif
