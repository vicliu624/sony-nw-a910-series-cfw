#include "color.h"
#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int color_test_failure;
extern void color_host_clear(void),color_host_corrupt(void);
#define CHECK(x) do{if(!(x)){fprintf(stderr,"color test line %d\n",__LINE__);return 1;}}while(0)
int main(int argc,char**argv){
 extern int color_selftest(void);color_host_clear();CHECK(color_selftest()==0);color_host_clear();
 color_host_clear();color_test_failure=3;CHECK(!color_open()&&color_status()==COLOR_NO_STORAGE);color_close();
 color_test_failure=2;CHECK(!color_open()&&color_status()==COLOR_NO_RAM);color_close();
 color_test_failure=4;CHECK(!color_open()&&color_status()==COLOR_NO_ROM);CHECK(color_save());color_close();
 color_test_failure=0;CHECK(color_open());color_button(0,1);color_tick(1);color_button(0,0);CHECK(color_status()==COLOR_OK);CHECK(color_save());color_close();CHECK(color_open());CHECK(color_status()==COLOR_OK);
 color_test_failure=1;CHECK(!color_save()&&color_status()==COLOR_SAVE_FAILED);color_test_failure=0;CHECK(color_save());color_close();
 color_host_corrupt();CHECK(!color_open()&&color_status()==COLOR_BAD_SAVE);CHECK(color_save());color_close();color_host_clear();
 /* Reach new list item, leave safely when ROM is missing, then choose Sony. */
 color_test_failure=4;app_init(918);app_enter_boot();app_key(K_DOWN);app_key(K_CENTER);app_key(K_DOWN);app_key(K_DOWN);app_key(K_CENTER);CHECK(app_page()==PAGE_COLOR);CHECK(color_status()==COLOR_NO_ROM);app_key(K_DOWN);
#ifdef A918_COLOR_MENU
 CHECK(app_page()==PAGE_COLOR_MENU);app_key(K_DOWN);app_key(K_CENTER);
#endif
 CHECK(app_page()==PAGE_GAMES&&app_selection()==2);app_key(K_DOWN);app_key(K_CENTER);CHECK(app_return_requested());
 color_host_clear();puts("Color synthetic ROM loading, snapshot roundtrip, corruption rejection, save retry, missing ROM, allocation/storage failure, Games routing PASS");
 if(argc>1){app_init(918);app_enter_boot();app_key(K_DOWN);app_key(K_CENTER);app_render();FILE*f=fopen(argv[1],"wb");if(!f)return 2;fprintf(f,"P6\n240 320\n255\n");uint16_t*p=app_pixels();for(int i=0;i<240*320;i++){unsigned v=p[i];unsigned char rgb[3]={((v>>11)&31)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);}
 return 0;
}
