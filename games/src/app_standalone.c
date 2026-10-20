/* R13: Tetris and disk-loaded color iD. No P1 or Sony-menu hooks. */
#include "app.h"
#include "color.h"
static uint16_t framebuffer[240*320];
static Blocks game;
static int page,home_selection,list_selection,return_requested,menu_selection,confirm_selection,menu_error;
static int rendered_color_status=-1;
static uint32_t seed_counter;
static int choice_only,games_requested;
#include "draw.inc"
static void list(int y,const char*const*items,int count,int selected){for(int i=0;i<count;i++){rectangle(12,y+i*43,216,33,i==selected?0x3a6f:0x1106);text(24,y+12+i*43,items[i],0xffff,1);}}
static void show_color(void){page=PAGE_COLOR;rendered_color_status=-1;}
void app_init(uint32_t seed){seed_counter=seed?seed:918;blocks_new(&game,seed_counter);page=PAGE_HOME;home_selection=list_selection=return_requested=choice_only=games_requested=0;rendered_color_status=-1;}
void app_enter_boot(void){page=PAGE_HOME;home_selection=0;return_requested=0;}
void app_boot_choice_only(void){choice_only=1;}
void app_enter_games(void){
#ifdef A918_APP_COLOR
 color_open();show_color();
#else
 blocks_new(&game,++seed_counter);page=PAGE_BLOCKS;
#endif
}
int app_games_requested(void){return games_requested;}
void app_enter_extension(void){return_requested=1;}
void app_enter_direction_extension(void){return_requested=1;}
int app_return_requested(void){return return_requested;}
uint16_t*app_pixels(void){return framebuffer;}
int app_page(void){return page;}
int app_selection(void){return page==PAGE_GAMES?list_selection:home_selection;}
int app_game_over(void){return game.over;}
uint32_t app_score(void){return game.score;}
void app_key(int key){
 if(key<K_LEFT||key>K_CENTER)return;
 if(page==PAGE_HOME){if(key==K_UP||key==K_LEFT)home_selection=0;else if(key==K_DOWN||key==K_RIGHT)home_selection=1;else if(key==K_CENTER){if(home_selection==0)return_requested=1;else if(choice_only){games_requested=return_requested=1;}else{page=PAGE_GAMES;list_selection=0;}}return;}
 if(page==PAGE_GAMES){if(key==K_UP||key==K_LEFT){if(list_selection)list_selection--;}else if(key==K_DOWN||key==K_RIGHT){if(list_selection<2)list_selection++;}else if(key==K_CENTER){if(list_selection==2)return_requested=1;else if(list_selection==1){color_open();show_color();}else{blocks_new(&game,++seed_counter);page=PAGE_BLOCKS;}}return;}
 if(page==PAGE_BLOCKS){if(key==K_CENTER){return_requested=1;}else if(key==K_LEFT)blocks_move(&game,-1,0);else if(key==K_RIGHT)blocks_move(&game,1,0);else if(key==K_UP)blocks_rotate(&game);else if(key==K_DOWN)blocks_move(&game,0,1);return;}
 if(page==PAGE_COLOR){if(key==K_DOWN){color_buttons_clear();menu_selection=menu_error=0;page=PAGE_COLOR_MENU;}else if(key==K_LEFT)color_button(0,1);else if(key==K_CENTER)color_button(1,1);else if(key==K_RIGHT)color_button(2,1);return;}
 if(page==PAGE_COLOR_MENU){
  if(key==K_UP||key==K_LEFT){if(menu_selection)menu_selection--;}
  else if(key==K_DOWN||key==K_RIGHT){if(menu_selection<3)menu_selection++;}
  else if(key==K_CENTER){if(menu_selection==0)show_color();else if(menu_selection==1){if(color_save()){color_close();return_requested=1;}else menu_error=1;}else{page=PAGE_COLOR_CONFIRM;confirm_selection=menu_error=0;}}return;
 }
 if(page==PAGE_COLOR_CONFIRM){if(key==K_UP||key==K_LEFT)confirm_selection=0;else if(key==K_DOWN||key==K_RIGHT)confirm_selection=1;else if(key==K_CENTER){if(!confirm_selection)page=PAGE_COLOR_MENU;else if(menu_selection==2?color_new_game():color_restore_backup())show_color();else{page=PAGE_COLOR_MENU;menu_error=1;}}}
}
void app_key_release(int key){if(page==PAGE_COLOR){if(key==K_LEFT)color_button(0,0);else if(key==K_CENTER)color_button(1,0);else if(key==K_RIGHT)color_button(2,0);}}
int app_shutdown(void){if(page==PAGE_COLOR||page==PAGE_COLOR_MENU||page==PAGE_COLOR_CONFIRM){int ok=color_save();color_close();return ok;}return 1;}
void app_tick(uint32_t ms){if(page==PAGE_BLOCKS)blocks_tick(&game,ms);else if(page==PAGE_COLOR)color_tick(ms);}
void app_prompt(const char*a,const char*b){rectangle(0,0,240,320,0x0843);text(12,80,a,0xffff,1);text(12,110,b,0xffff,1);}
void app_render(void){
 if(page!=PAGE_COLOR)rendered_color_status=-1;
 int status=color_status();int repaint=page!=PAGE_COLOR||rendered_color_status!=status;
 if(repaint){rectangle(0,0,240,320,0x0843);rectangle(0,0,240,27,0x19a8);}
 if(page==PAGE_HOME){const char*items[]={"WALKMAN","GAMES"};text(12,9,"SELECT",0xffff,1);list(100,items,2,home_selection);text(12,267,"DIRECTIONS SELECT  CENTER OPEN",0xbdf7,1);text(12,291,"AUTO WALKMAN IN 8 SECONDS",0x07ff,1);return;}
 if(page==PAGE_GAMES){const char*items[]={"TETRIS","TAMAGOTCHI ID","RETURN TO WALKMAN"};text(12,9,"GAMES",0xffff,1);list(65,items,3,list_selection);text(12,257,"DIRECTIONS SELECT",0xbdf7,1);text(12,280,"CENTER OPEN",0x07ff,1);return;}
 if(page==PAGE_COLOR_MENU||page==PAGE_COLOR_CONFIRM){
  text(12,9,"TAMAGOTCHI ID MENU",0xffff,1);
  if(page==PAGE_COLOR_MENU){const char*items[]={"CONTINUE","SAVE AND EXIT","NEW GAME","RESTORE BACKUP"};list(50,items,4,menu_selection);text(12,244,menu_error?"ACTION FAILED - SAVE KEPT":"GAME PAUSED IN THIS MENU",menu_error?0xfd20:0xbdf7,1);}
  else{const char*items[]={"CANCEL","YES - CONFIRM"};text(12,52,menu_selection==2?"START A NEW PET":"RESTORE PREVIOUS SAVE",0xffff,1);text(12,81,menu_selection==2?"CURRENT SAVE WILL BE BACKED UP":"CURRENT SESSION WILL BE REPLACED",0xbdf7,1);text(12,105,menu_selection==2?"SETUP FILE MUST BE ON DISK":"BACKUP FILE WILL BE KEPT",0xbdf7,1);list(151,items,2,confirm_selection);}
  text(12,276,"UP DOWN SELECT",0xbdf7,1);text(12,296,"CENTER CONFIRM",0x07ff,1);return;
 }
 if(page==PAGE_COLOR){
  if(repaint)text(12,9,"TAMAGOTCHI ID",0xffff,1);
  if(status!=COLOR_OK&&status!=COLOR_SAVE_FAILED){const char*errors[]={"","ROM NOT FOUND ON DISK","ROM INVALID OR UNSUPPORTED","NOT ENOUGH MEMORY","SAVE INVALID - NOT OVERWRITTEN","","CORE STOPPED - NO SAVE","GAME DISK UNAVAILABLE"};text(12,64,errors[status],0xfd20,1);text(12,100,"ADD FILES TO WALKMAN",0xffff,1);text(12,122,"APPS  TAMAGOTCHI-ID",0xbdf7,1);text(12,145,"ROM BIN AND NEW SAV",0xbdf7,1);text(12,190,"DOWN - OPEN MENU",0x07ff,1);return;}
  const uint16_t*p=color_pixels();int visible=color_visible();rectangle(55,79,130,130,0x4a69);for(int y=0;y<128;y++)for(int x=0;x<128;x++)framebuffer[(80+y)*240+56+x]=visible?p[y*128+x]:0;
  if(repaint){text(12,230,"128 X 128 - ORIGINAL SIZE",0xbdf7,1);text(12,250,"LEFT A  CENTER B  RIGHT C",0xbdf7,1);text(12,292,"DOWN - OPEN MENU",0x07ff,1);}rendered_color_status=status;
  rectangle(12,268,228,11,0x0843);text(12,270,status==COLOR_SAVE_FAILED?"SAVE FAILED - OPEN MENU":color_input_pending()?"PROCESSING INPUT - PLEASE WAIT":"ROM AND SAVES ON GAME DISK",status==COLOR_SAVE_FAILED||color_input_pending()?0xfd20:0xbdf7,1);return;
 }
 if(page==PAGE_BLOCKS){
#include "tetris_draw.inc"
 }
}
