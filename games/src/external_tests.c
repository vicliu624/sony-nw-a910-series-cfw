#include "app.h"
#include "color.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define DIR "work/game-external/disk/GAMES/TAMAGOTCHI/ID/"
extern void external_failure(int),external_storage(int);
extern unsigned external_writes(void);
extern int boot_selftest(void);
#define CHECK(x) do{if(!(x)){fprintf(stderr,"external test line %d\n",__LINE__);return 1;}}while(0)
static unsigned filehash(const char*path){FILE*f=fopen(path,"rb");if(!f)return 0;unsigned h=2166136261u;int c;while((c=fgetc(f))!=EOF)h=(h^(unsigned char)c)*16777619u;fclose(f);return h;}
static void clean(void){remove(DIR "state.sav");remove(DIR "state.tmp");remove(DIR "state.bak");}
static void shot(const char*name){char path[160];snprintf(path,sizeof(path),"work/game-external/%s.ppm",name);app_render();FILE*f=fopen(path,"wb");fprintf(f,"P6\n240 320\n255\n");uint16_t*p=app_pixels();for(int i=0;i<240*320;i++){unsigned v=p[i];unsigned char c[3]={((v>>11)&31)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};fwrite(c,1,3,f);}fclose(f);}
static void open_id(void){app_init(918);app_enter_boot();app_key(K_DOWN);app_key(K_CENTER);app_key(K_DOWN);app_key(K_CENTER);}
int main(void){
 clean();CHECK(boot_selftest()==0);
 external_storage(0);unsigned w=external_writes();CHECK(!color_open()&&color_status()==COLOR_NO_STORAGE);CHECK(w==external_writes());color_close();external_storage(1);
 CHECK(rename(DIR "rom.bin",DIR "rom.off")==0);CHECK(!color_open()&&color_status()==COLOR_NO_ROM);CHECK(rename(DIR "rom.off",DIR "rom.bin")==0);color_close();
 FILE*f=fopen(DIR "rom.bin","r+b");CHECK(f);int original=fgetc(f);rewind(f);fputc(original^1,f);fclose(f);CHECK(!color_open()&&color_status()==COLOR_BAD_ROM);color_close();f=fopen(DIR "rom.bin","r+b");CHECK(f);fputc(original,f);fclose(f);
 external_failure(5);CHECK(!color_open()&&color_status()==COLOR_NO_RAM);color_close();external_failure(0);
 open_id();CHECK(app_page()==PAGE_COLOR&&color_status()==COLOR_OK);for(int i=0;i<500;i++)app_tick(16);CHECK(color_status()==COLOR_OK);shot("color-id");
 uint16_t image[16384];memcpy(image,color_pixels(),sizeof(image));CHECK(color_save());unsigned old=filehash(DIR "state.sav");CHECK(old);
 for(int fail=1;fail<=6;fail++){if(fail==5)continue;external_failure(fail);CHECK(!color_save());CHECK(filehash(DIR "state.sav")==old);external_failure(0);CHECK(color_save());}
 CHECK(rename(DIR "new.sav",DIR "new.off")==0);CHECK(!color_new_game());CHECK(filehash(DIR "state.sav")==old);CHECK(rename(DIR "new.off",DIR "new.sav")==0);
 app_key(K_DOWN);CHECK(app_page()==PAGE_COLOR_MENU);shot("game-menu");app_tick(1000);CHECK(!memcmp(image,color_pixels(),sizeof(image)));app_key(K_CENTER);CHECK(app_page()==PAGE_COLOR);
 app_key(K_DOWN);app_key(K_DOWN);app_key(K_DOWN);app_key(K_CENTER);CHECK(app_page()==PAGE_COLOR_CONFIRM);shot("reset-confirm");app_key(K_CENTER);CHECK(app_page()==PAGE_COLOR_MENU);CHECK(filehash(DIR "state.sav")==old);
 app_key(K_CENTER);app_key(K_DOWN);app_key(K_CENTER);CHECK(app_page()==PAGE_COLOR&&color_status()==COLOR_OK);CHECK(filehash(DIR "state.bak")==old);shot("new-game");
 app_key(K_DOWN);app_key(K_DOWN);app_key(K_DOWN);app_key(K_DOWN);app_key(K_CENTER);app_key(K_DOWN);app_key(K_CENTER);CHECK(app_page()==PAGE_COLOR&&color_status()==COLOR_OK);CHECK(!memcmp(image,color_pixels(),sizeof(image)));
 app_key(K_DOWN);app_key(K_DOWN);app_key(K_CENTER);CHECK(app_page()==PAGE_GAMES&&app_selection()==1);shot("games");app_key(K_CENTER);CHECK(!memcmp(image,color_pixels(),sizeof(image)));CHECK(app_shutdown());
 /* Truncated/corrupted saves reject without replacement; user files stay visible. */
 f=fopen(DIR "state.sav","r+b");CHECK(f);fseek(f,100,SEEK_SET);int c=fgetc(f);fseek(f,100,SEEK_SET);fputc(c^1,f);fclose(f);unsigned bad=filehash(DIR "state.sav");CHECK(!color_open()&&color_status()==COLOR_BAD_SAVE);CHECK(color_save());CHECK(filehash(DIR "state.sav")==bad);color_close();clean();
 puts("External disk: missing/corrupt ROM, no storage/RAM, readonly/full disk/write/fsync/rename failures, old save preserved, missing reset seed, menu cancel, reset backup/restore, exit/reenter, corrupt save rejection PASS");
 return 0;
}
