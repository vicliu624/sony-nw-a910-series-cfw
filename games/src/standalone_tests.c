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
 app_init(918);app_enter_games();
 if(color_status()!=COLOR_OK||app_page()!=PAGE_COLOR)return 10;
 for(int i=0;i<80;i++)app_tick(24);
 app_render();
 const int keys[]={K_LEFT,K_CENTER,K_RIGHT};
 for(int i=0;i<3;i++){app_key(keys[i]);app_key_release(keys[i]);for(int j=0;j<30;j++)app_tick(24);}
 app_key(K_DOWN);if(app_page()!=PAGE_COLOR_MENU)return 11;
 app_key(K_CENTER);if(app_page()!=PAGE_COLOR)return 12; /* Continue */
 app_key(K_DOWN);app_key(K_DOWN);app_key(K_DOWN);app_key(K_CENTER);
 if(app_page()!=PAGE_COLOR_CONFIRM)return 13;
 app_key(K_DOWN);app_key(K_CENTER); /* Confirm New Game. */
 if(app_page()!=PAGE_COLOR||color_status()!=COLOR_OK)return 14;
 app_key(K_DOWN);app_key(K_DOWN);app_key(K_DOWN);app_key(K_DOWN);app_key(K_CENTER);
 if(app_page()!=PAGE_COLOR_CONFIRM)return 15;
 app_key(K_DOWN);app_key(K_CENTER); /* Confirm Restore Backup. */
 if(app_page()!=PAGE_COLOR||color_status()!=COLOR_OK)return 16;
 app_key(K_DOWN);app_key(K_DOWN);app_key(K_CENTER); /* Save and Exit. */
 if(!app_return_requested()||!app_shutdown())return 17;
 app_init(918);app_enter_games();
 if(color_status()!=COLOR_OK||app_page()!=PAGE_COLOR)return 18;
 return app_shutdown()?0:19;
}
#else
int standalone_disk_test(void){return boot_selftest();}
#endif
