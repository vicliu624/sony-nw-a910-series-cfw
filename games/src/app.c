#include "app.h"
#include "menu_assets.h"
#ifdef A918_COLOR
#include "color.h"
#define LAST_GAME_ITEM 3
#else
#define LAST_GAME_ITEM 2
#endif
#ifdef A918_P1
#include "p1.h"
#endif
static uint16_t framebuffer[240*320];
static Blocks game;
static int page,home_selection,list_selection,info_selection;
static uint32_t seed_counter;
static int extension_mode,return_requested;
static int direction_only;
static int boot_mode;
#ifdef A918_COLOR_MENU
static int pet_menu_selection,pet_confirm_selection,pet_menu_error;
#endif
#ifdef A918_COLOR_PERF
static int rendered_color_status=-1;
#endif
static const char *labels[11]={"SHUFFLE","TV","SEARCH","PHOTOS","MUSIC","VIDEOS","SETTINGS","RECORDING","NOW PLAYING","GAME","FM RADIO"};
static const unsigned char glyph[36][7]={
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,1,14},
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},{7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},{17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},{17,17,10,4,4,4,4},{31,1,2,4,8,16,31}
};
static const uint16_t palette[8]={0,0x07ff,0xffe0,0xf81f,0x07e0,0xf800,0x001f,0xfd20};
static void rectangle(int x,int y,int w,int h,uint16_t color){for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)if(xx>=0&&xx<240&&yy>=0&&yy<320)framebuffer[yy*240+xx]=color;}
static void text(int x,int y,const char*s,uint16_t color,int scale){for(;*s;s++,x+=6*scale){int index=*s>='0'&&*s<='9'?*s-'0':*s>='A'&&*s<='Z'?10+*s-'A':-1;if(index>=0)for(int row=0;row<7;row++)for(int col=0;col<5;col++)if(glyph[index][row]&(1<<(4-col)))rectangle(x+col*scale,y+row*scale,scale,scale,color);else{}else if(*s=='-')rectangle(x,y+3*scale,5*scale,scale,color);else if(*s==':'){rectangle(x+2*scale,y+2*scale,scale,scale,color);rectangle(x+2*scale,y+5*scale,scale,scale,color);}else if(*s=='>'){rectangle(x,y+2*scale,scale,scale,color);rectangle(x+scale,y+3*scale,scale,scale,color);rectangle(x,y+4*scale,scale,scale,color);}}}
static void number(int x,int y,uint32_t value){char out[12];int i=0;do{out[i++]=(char)('0'+value%10);value/=10;}while(value&&i<11);char reversed[12];for(int j=0;j<i;j++)reversed[j]=out[i-1-j];reversed[i]=0;text(x,y,reversed,0xffff,1);}
static void block(int x,int y,int type,int size){rectangle(x,y,size-1,size-1,palette[type]);rectangle(x+1,y+1,size-3,1,0xffff);}
static void extension_home(void){
 for(int i=0;i<240*320;i++)framebuffer[i]=extension_wallpaper[i];
 rectangle(0,0,240,28,0x21a8);text(12,9,"HOME",0xffff,1);text(176,9,"PAGE 2",0xbdf7,1);
 for(int item=9;item<=10;item++){
  int x=(item-9)*80;
  if(home_selection==item){rectangle(x+2,29,76,2,0x07ff);rectangle(x+2,99,76,2,0x07ff);rectangle(x+2,29,2,72,0x07ff);rectangle(x+76,29,2,72,0x07ff);}
  {
   const unsigned short *icon=item==9?game_icon:fm_icon;
   const unsigned char *alpha=item==9?game_icon_alpha:fm_icon_alpha;
   for(int y=0;y<64;y++)for(int xx=0;xx<72;xx++){
    int a=alpha[y*72+xx];if(!a)continue;
    unsigned fg=icon[y*72+xx],bg=framebuffer[(y+33)*240+x+4+xx];
    unsigned r=(((fg>>11)&31)*a+((bg>>11)&31)*(255-a)+127)/255;
    unsigned g=(((fg>>5)&63)*a+((bg>>5)&63)*(255-a)+127)/255;
    unsigned b=((fg&31)*a+(bg&31)*(255-a)+127)/255;
    framebuffer[(y+33)*240+x+4+xx]=(uint16_t)((r<<11)|(g<<5)|b);
   }
  }
 }
 text(27,110,"GAME",0xffff,1);text(92,110,"FM RADIO",0xffff,1);text(91,125,"PENDING",0xfd20,1);
 text(12,250,home_selection==9?"GAME":"FM RADIO - UNVERIFIED",0xffff,1);
 text(12,275,"CENTER OPEN",0xbdf7,1);text(12,293,direction_only?"BACK OR EDGE - SONY HOME":"BACK OR OPTION - SONY HOME",0xffff,1);
}
void app_prompt(const char*first,const char*second){rectangle(0,0,240,320,0x0843);text(12,30,"A918 GAME INPUT",0xffff,1);text(12,90,first,0x07ff,1);text(12,115,second,0xffff,1);text(12,280,"TIMEOUT RETURNS TO SONY",0xbdf7,1);}
void app_init(uint32_t seed){seed_counter=seed?seed:918;blocks_new(&game,seed_counter);page=PAGE_HOME;home_selection=4;list_selection=0;extension_mode=0;return_requested=0;direction_only=0;boot_mode=0;}
void app_enter_boot(void){boot_mode=1;extension_mode=0;return_requested=0;page=PAGE_HOME;home_selection=0;list_selection=0;}
void app_enter_extension(void){extension_mode=1;return_requested=0;page=PAGE_HOME;home_selection=9;}
void app_enter_direction_extension(void){app_enter_extension();direction_only=1;}
int app_return_requested(void){return return_requested;}
uint16_t*app_pixels(void){return framebuffer;}
int app_page(void){return page;}
int app_selection(void){return page==PAGE_GAMES?list_selection:home_selection;}
int app_game_over(void){return game.over;}
uint32_t app_score(void){return game.score;}
void app_key(int key){
 if(boot_mode){
  if(key<K_LEFT||key>K_CENTER)return;
#ifdef A918_COLOR_MENU
  if(page==PAGE_COLOR_MENU){
   if(key==K_UP||key==K_LEFT){if(pet_menu_selection>0)pet_menu_selection--;}
   else if(key==K_DOWN||key==K_RIGHT){if(pet_menu_selection<3)pet_menu_selection++;}
   else if(key==K_CENTER){
    if(pet_menu_selection==0){page=PAGE_COLOR;rendered_color_status=-1;}
    else if(pet_menu_selection==1){if(color_save()){color_close();page=PAGE_GAMES;list_selection=2;}else pet_menu_error=1;}
    else{page=PAGE_COLOR_CONFIRM;pet_confirm_selection=0;pet_menu_error=0;}
   }return;
  }
  if(page==PAGE_COLOR_CONFIRM){
   if(key==K_UP||key==K_LEFT)pet_confirm_selection=0;
   else if(key==K_DOWN||key==K_RIGHT)pet_confirm_selection=1;
   else if(key==K_CENTER){
    if(!pet_confirm_selection)page=PAGE_COLOR_MENU;
    else{int ok=pet_menu_selection==2?color_new_game():color_restore_backup();if(ok){page=PAGE_COLOR;rendered_color_status=-1;}else{page=PAGE_COLOR_MENU;pet_menu_error=1;}}
   }return;
  }
#endif
  if(page==PAGE_HOME){
   if(key==K_UP||key==K_LEFT)home_selection=0;
   else if(key==K_DOWN||key==K_RIGHT)home_selection=1;
   else if(key==K_CENTER){if(home_selection==0)return_requested=1;else{page=PAGE_GAMES;list_selection=0;}}
   return;
  }
  if(page==PAGE_GAMES){
#ifdef A918_P1
   if(key==K_UP||key==K_LEFT){if(list_selection>0)list_selection--;}
   else if(key==K_DOWN||key==K_RIGHT){if(list_selection<LAST_GAME_ITEM)list_selection++;}
   else if(key==K_CENTER){
    if(list_selection==LAST_GAME_ITEM)return_requested=1;
#ifdef A918_COLOR
    else if(list_selection==2){color_open();page=PAGE_COLOR;
#ifdef A918_COLOR_PERF
     rendered_color_status=-1;
#endif
    }
#endif
    else if(list_selection==1){p1_open();page=PAGE_P1;}
    else{blocks_new(&game,++seed_counter);page=PAGE_BLOCKS;}
   }
#else
   if(key==K_UP||key==K_LEFT)list_selection=0;
   else if(key==K_DOWN||key==K_RIGHT)list_selection=1;
   else if(key==K_CENTER){if(list_selection==1)return_requested=1;else{blocks_new(&game,++seed_counter);page=PAGE_BLOCKS;}}
#endif
   return;
  }
#ifdef A918_P1
 #ifdef A918_COLOR
  if(page==PAGE_COLOR){
#ifdef A918_COLOR_MENU
   if(key==K_DOWN){color_buttons_clear();pet_menu_selection=0;pet_menu_error=0;page=PAGE_COLOR_MENU;return;}
#else
   if(key==K_DOWN){if(color_save()){color_close();page=PAGE_GAMES;list_selection=2;}return;}
#endif
   if(key==K_LEFT)color_button(0,1);else if(key==K_CENTER)color_button(1,1);else if(key==K_RIGHT)color_button(2,1);
   return;
  }
 #endif
  if(page==PAGE_P1){
   if(key==K_DOWN){if(p1_status()==3||p1_save()){page=PAGE_GAMES;list_selection=1;}return;}
   if(key==K_LEFT)p1_button(0,1);else if(key==K_CENTER)p1_button(1,1);else if(key==K_RIGHT)p1_button(2,1);
   return;
  }
#endif
 }
 if(key==K_OPTION)return;
 if(page==PAGE_HOME){
  if(extension_mode){
   if(direction_only&&key==K_OPTION)return;
   if(key==K_BACK||key==K_OPTION){return_requested=1;return;}
   if(key==K_LEFT||key==K_UP){if(home_selection==9)return_requested=1;else home_selection=9;return;}
   if(key==K_RIGHT||key==K_DOWN){if(home_selection==10)return_requested=1;else home_selection=10;return;}
  }
  if(key==K_BACK){home_selection=4;return;}
  if(home_selection<9){if(key==K_LEFT)home_selection=home_selection?home_selection-1:10;else if(key==K_RIGHT)home_selection++;else if(key==K_DOWN)home_selection=home_selection<6?home_selection+3:9;else if(key==K_UP)home_selection=home_selection>=3?home_selection-3:9;}
  else{if(key==K_LEFT||key==K_UP)home_selection=home_selection==10?9:8;else if(key==K_RIGHT)home_selection=home_selection==9?10:0;else if(key==K_DOWN)home_selection=0;}
  if(key==K_CENTER){if(home_selection==9){page=PAGE_GAMES;list_selection=0;}else{info_selection=home_selection;page=PAGE_INFO;}}
 }else if(page==PAGE_GAMES){
  if(key==K_UP||key==K_DOWN)list_selection=1-list_selection;
  else if(key==K_BACK||(key==K_CENTER&&list_selection==1)){page=PAGE_HOME;home_selection=9;}
  else if(key==K_CENTER){blocks_new(&game,++seed_counter);page=PAGE_BLOCKS;}
 }else if(page==PAGE_BLOCKS){
  if(key==K_BACK)return;
  if(key==K_CENTER){page=PAGE_GAMES;list_selection=0;return;}
  if(key==K_LEFT)blocks_move(&game,-1,0);else if(key==K_RIGHT)blocks_move(&game,1,0);else if(key==K_UP)blocks_rotate(&game);else if(key==K_DOWN)blocks_move(&game,0,1);
 }else if(key==K_BACK||key==K_CENTER)page=PAGE_HOME;
}
void app_key_release(int key){
#ifdef A918_COLOR
 if(page==PAGE_COLOR){if(key==K_LEFT)color_button(0,0);else if(key==K_CENTER)color_button(1,0);else if(key==K_RIGHT)color_button(2,0);}
#endif
#ifdef A918_P1
 if(page==PAGE_P1){if(key==K_LEFT)p1_button(0,0);else if(key==K_CENTER)p1_button(1,0);else if(key==K_RIGHT)p1_button(2,0);}
#else
 (void)key;
#endif
}
int app_shutdown(void){
#ifdef A918_COLOR_MENU
 if(page==PAGE_COLOR_MENU||page==PAGE_COLOR_CONFIRM){int ok=color_save();color_close();return ok;}
#endif
#ifdef A918_COLOR
 if(page==PAGE_COLOR){int ok=color_save();color_close();return ok;}
#endif
#ifdef A918_P1
 return page!=PAGE_P1||p1_save();
#else
 return 1;
#endif
}
void app_tick(uint32_t ms){if(page==PAGE_BLOCKS)blocks_tick(&game,ms);
#ifdef A918_COLOR
 else if(page==PAGE_COLOR)color_tick(ms);
#endif
#ifdef A918_P1
 else if(page==PAGE_P1)p1_tick(ms);
#endif
}
void app_render(void){
#ifdef A918_COLOR_PERF
 if(page!=PAGE_COLOR)rendered_color_status=-1;
 if(page!=PAGE_COLOR||rendered_color_status!=color_status()){
#endif
 rectangle(0,0,240,320,0x0843);rectangle(0,0,240,27,0x19a8);
#ifdef A918_COLOR_PERF
 }
#endif
#ifdef A918_COLOR
#ifdef A918_COLOR_MENU
 if(page==PAGE_COLOR_MENU||page==PAGE_COLOR_CONFIRM){
  text(12,9,"TAMAGOTCHI ID MENU",0xffff,1);
  if(page==PAGE_COLOR_MENU){
   const char*items[4]={"CONTINUE","SAVE AND EXIT","NEW GAME","RESTORE BACKUP"};
   for(int i=0;i<4;i++){rectangle(12,50+i*43,216,33,i==pet_menu_selection?0x3a6f:0x1106);text(24,62+i*43,items[i],0xffff,1);}
   text(12,244,pet_menu_error?"ACTION FAILED - TRY AGAIN":"GAME PAUSED IN THIS MENU",pet_menu_error?0xfd20:0xbdf7,1);
   text(12,269,"UP DOWN SELECT",0xbdf7,1);text(12,291,"CENTER CONFIRM",0x07ff,1);
  }else{
   text(12,52,pet_menu_selection==2?"START A NEW PET":"RESTORE PREVIOUS SAVE",0xffff,1);
   text(12,81,pet_menu_selection==2?"CURRENT SAVE WILL BE BACKED UP":"CURRENT SESSION WILL BE REPLACED",0xbdf7,1);
   text(12,105,pet_menu_selection==2?"RETURNS TO FIRST TIME SETUP":"BACKUP FILE WILL BE KEPT",0xbdf7,1);
   const char*items[2]={"CANCEL","YES - CONFIRM"};
   for(int i=0;i<2;i++){rectangle(12,151+i*47,216,34,i==pet_confirm_selection?0x3a6f:0x1106);text(24,164+i*47,items[i],0xffff,1);}
   text(12,276,"UP DOWN SELECT",0xbdf7,1);text(12,296,"CENTER CONFIRM",0x07ff,1);
  }return;
 }
#endif
 if(page==PAGE_COLOR){
  int s=color_status();
#ifdef A918_COLOR_PERF
  int repaint=rendered_color_status!=s;
#else
  int repaint=1;
#endif
  if(repaint)text(12,9,"TAMAGOTCHI ID",0xffff,1);
  if(s!=COLOR_OK&&s!=COLOR_SAVE_FAILED){
   const char*errors[]={"","COLOR ID ROM NOT FOUND","COLOR ID ROM INVALID","NOT ENOUGH MEMORY","SAVE INVALID - NOT OVERWRITTEN","","CORE STOPPED - NO SAVE","STORAGE UNAVAILABLE"};
   text(12,76,errors[s],0xfd20,1);text(12,106,
#ifdef A918_COLOR_MENU
   "DOWN - OPEN MENU",
#else
   "DOWN - RETURN TO GAMES",
#endif
   0xffff,1);return;
  }
  /* Pixel-for-pixel, x=56..183, y=80..207. No scaling. */
  const uint16_t*p=color_pixels();int visible=color_visible();rectangle(55,79,130,130,0x4a69);
  for(int y=0;y<128;y++)for(int x=0;x<128;x++)framebuffer[(80+y)*240+56+x]=visible?p[y*128+x]:0;
  if(repaint){text(12,230,"128 X 128 - ORIGINAL SIZE",0xbdf7,1);
  text(12,250,"LEFT A  CENTER B  RIGHT C",0xbdf7,1);
  text(12,270,s==COLOR_SAVE_FAILED?"SAVE FAILED - DOWN RETRY":"SILENT - EXPERIMENTAL SPEED",s==COLOR_SAVE_FAILED?0xfd20:0xbdf7,1);
  text(12,292,
#ifdef A918_COLOR_MENU
   "DOWN - OPEN MENU",
#else
   "DOWN - SAVE AND RETURN",
#endif
   0x07ff,1);}
#ifdef A918_COLOR_PERF
  rendered_color_status=s;
#endif
#ifdef A918_COLOR_MENU
  rectangle(12,268,228,11,0x0843);
  text(12,270,s==COLOR_SAVE_FAILED?"SAVE FAILED - OPEN MENU":color_input_pending()?"PROCESSING INPUT - PLEASE WAIT":"SILENT - EXPERIMENTAL SPEED",s==COLOR_SAVE_FAILED||color_input_pending()?0xfd20:0xbdf7,1);
#endif
  return;
 }
#endif
#ifdef A918_P1
 if(page==PAGE_P1){
  text(12,9,"TAMAGOTCHI P1",0xffff,1);
  if(p1_status()==1||p1_status()==2||p1_status()==6){
   text(12,80,p1_status()==6?"P1 STORAGE UNAVAILABLE":p1_status()==1?"P1 ROM NOT FOUND":"P1 ROM INVALID",0xfd20,1);
   text(12,110,"DOWN - RETURN TO GAMES",0xffff,1);return;
  }
  const uint8_t *pixels=p1_pixels(),*icons=p1_icons();
  static const char*names[8]={"FOOD","LIGHT","PLAY","MED","WC","STATUS","TRAIN","CALL"};
  for(int i=0;i<8;i++){int x=(i%4)*59+4,y=i<4?63:222;rectangle(x,y,55,22,icons[i]?0x4b4c:0x1106);text(x+3,y+7,names[i],icons[i]?0xffff:0x6b8d,1);}
  rectangle(6,93,228,116,0x94e9);
  for(int y=0;y<16;y++)for(int x=0;x<32;x++)rectangle(8+x*7,95+y*7,7,7,pixels[y*32+x]?0x1903:0xbe33);
  text(12,265,"LEFT A  CENTER B  RIGHT C",0xbdf7,1);
  text(12,282,p1_status()==4?"SAVE FAILED - DOWN RETRY":p1_status()==3?"CORE ERROR - DOWN TO GAMES":p1_status()==5?"SAVE INVALID - NEW SESSION":"SILENT - TIME FREEZES ON EXIT",p1_status()?0xfd20:0xbdf7,1);
  text(12,300,"DOWN - SAVE AND RETURN",0x07ff,1);return;
 }
 if(boot_mode&&page==PAGE_GAMES){
  text(12,9,"GAMES",0xffff,1);
#ifdef A918_COLOR
  const char*items[4]={"TETRIS","TAMAGOTCHI P1","TAMAGOTCHI ID","RETURN TO WALKMAN"};
#else
  const char*items[3]={"TETRIS","TAMAGOTCHI P1","RETURN TO WALKMAN"};
#endif
  for(int i=0;i<=LAST_GAME_ITEM;i++){rectangle(12,55+i*45,216,34,i==list_selection?0x3a6f:0x1106);text(24,68+i*45,items[i],0xffff,1);}
  text(12,252,"DIRECTIONS SELECT",0xbdf7,1);text(12,273,"CENTER OPEN",0xbdf7,1);return;
 }
#endif
 if(boot_mode&&(page==PAGE_HOME||page==PAGE_GAMES)){
  text(12,9,page==PAGE_HOME?"WALKMAN":"GAMES",0xffff,1);
  text(24,48,page==PAGE_HOME?"SELECT":"GAMES",0xffff,2);
  const char *a=page==PAGE_HOME?"WALKMAN":"TETRIS",*b=page==PAGE_HOME?"GAMES":"RETURN TO WALKMAN";
  int selected=page==PAGE_HOME?home_selection:list_selection;
  rectangle(12,100,216,42,selected==0?0x3a6f:0x1106);
  rectangle(12,158,216,42,selected==1?0x3a6f:0x1106);
  text(25,116,a,0xffff,1);text(25,174,b,0xffff,1);
  text(12,242,"DIRECTIONS SELECT",0xbdf7,1);text(12,262,"CENTER OPEN",0xbdf7,1);
  text(12,289,page==PAGE_HOME?"AUTO WALKMAN IN 8 SECONDS":"CENTER IN GAME ENDS ROUND",0x07ff,1);
  return;
 }
 if(page==PAGE_HOME){
  if(home_selection>=9){extension_home();return;}
  text(12,9,"HOME",0xffff,1);text(176,9,home_selection<9?"PAGE 1":"PAGE 2",0xbdf7,1);
  int first=home_selection<9?0:9,last=home_selection<9?9:11;
  for(int i=first;i<last;i++){int slot=i-first,x=(slot%3)*80+4,y=(slot/3)*72+34;rectangle(x,y,72,62,i==home_selection?0x3a6f:0x1106);text(x+7,y+14,labels[i],0xffff,1);if(i==9)text(x+7,y+34,"1 GAME",0x07ff,1);if(i==10)text(x+7,y+34,"UNVERIFIED",0xfd20,1);}
  text(12,265,labels[home_selection],0xffff,1);text(12,286,"DIRECTIONS MOVE  CENTER OPEN",0xbdf7,1);text(12,302,"DIRECTION AT EDGE - PAGE",0xbdf7,1);
 }else if(page==PAGE_GAMES){
  text(12,9,"GAME",0xffff,1);text(16,43,"TETRIS",0x07ff,2);const char*items[2]={"TETRIS","BACK TO HOME"};
  for(int i=0;i<2;i++){rectangle(10,88+i*40,220,31,i==list_selection?0x3a6f:0x1106);text(20,100+i*40,items[i],0xffff,1);}
  text(12,235,"LEFT RIGHT - MOVE",0xbdf7,1);text(12,250,"UP ROTATE  DOWN SOFT DROP",0xbdf7,1);text(12,265,"CENTER - END ROUND",0xbdf7,1);text(12,286,"EACH START IS A NEW ROUND",0x07ff,1);
 }else if(page==PAGE_BLOCKS){
  text(12,9,"TETRIS",0xffff,1);rectangle(10,42,102,202,0xffff);rectangle(11,43,100,200,0x0000);
  for(int y=0;y<BH;y++)for(int x=0;x<BW;x++)if(game.board[y][x])block(11+x*10,43+y*10,game.board[y][x],10);
  for(int y=0;y<4;y++)for(int x=0;x<4;x++)if(blocks_cell(game.piece,game.rotation,x,y)&&game.y+y>=0)block(11+(game.x+x)*10,43+(game.y+y)*10,game.piece+1,10);
  text(128,44,"NEXT",0xbdf7,1);for(int y=0;y<4;y++)for(int x=0;x<4;x++)if(blocks_cell(game.next,0,x,y))block(130+x*12,65+y*12,game.next+1,12);
  text(128,125,"SCORE",0xbdf7,1);number(128,140,game.score);text(128,165,"LINES",0xbdf7,1);number(128,180,game.lines);text(128,205,"LEVEL",0xbdf7,1);number(128,220,game.lines/10+1);
  if(game.over){rectangle(16,125,208,40,0x19a8);text(25,138,"GAME OVER",0xffff,2);}
  text(12,263,"LEFT RIGHT MOVE  UP ROTATE",0xbdf7,1);text(12,280,"DOWN - SOFT DROP",0xbdf7,1);text(12,300,"CENTER - END ROUND",0x07ff,1);
 }else{
  text(12,9,labels[info_selection],0xffff,1);
  if(info_selection==10){text(12,65,"FM HARDWARE UNVERIFIED",0xfd20,1);text(12,90,"RECEPTION NOT VERIFIED",0xffff,1);text(12,115,"NOT MARKED AS ENABLED",0xffff,1);}
  else{text(12,65,"ORIGINAL SONY FUNCTION",0xffff,1);text(12,90,"LOCAL PREVIEW ONLY",0xfd20,1);text(12,115,"NO DEVICE ACTION TAKEN",0xffff,1);}
  text(12,275,"BACK RETURN TO HOME",0xbdf7,1);
 }
}
