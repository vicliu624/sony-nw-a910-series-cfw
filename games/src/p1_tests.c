#include "app.h"
#include "p1.h"
#define CHECK(x) do{if(!(x))return __LINE__;}while(0)
static int same(const uint8_t*a,const uint8_t*b,unsigned n){for(unsigned i=0;i<n;i++)if(a[i]!=b[i])return 0;return 1;}
extern int p1_test_failure;
static uint8_t state_a[2048],state_b[2048],state_c[2048];
static uint8_t test_rom[12289];
static uint32_t lcd_hash(void){uint32_t h=2166136261u;for(unsigned i=0;i<512;i++)h=(h^p1_pixels()[i])*16777619u;return h;}
static void press(unsigned button){p1_button(button,1);p1_tick(100);p1_button(button,0);for(unsigned i=0;i<10;i++)p1_tick(100);}
int p1_selftest(void){
 p1_test_failure=3;CHECK(!p1_open());CHECK(p1_status()==6);p1_test_failure=0;
 CHECK(p1_open());CHECK(p1_status()==0);
 for(unsigned i=0;i<50;i++)p1_tick(100);
 unsigned lit=0;for(unsigned i=0;i<512;i++)lit+=p1_pixels()[i];CHECK(lit>0);CHECK(p1_status()==0);
 unsigned n=p1_export(state_a,sizeof(state_a));CHECK(n>100&&n<2048);
 for(unsigned i=0;i<10;i++)p1_tick(100);
 CHECK(p1_export(state_b,sizeof(state_b))==n);
 CHECK(p1_import(state_a,n));
 for(unsigned i=0;i<10;i++)p1_tick(100);
 CHECK(p1_export(state_c,sizeof(state_c))==n);CHECK(same(state_b,state_c,n));
 CHECK(!p1_import(state_a,n-1));state_a[32]^=1;CHECK(!p1_import(state_a,n));state_a[32]^=1;
 CHECK(p1_import(state_a,n));
 for(unsigned button=0;button<3;button++){p1_button(button,1);p1_tick(40);p1_button(button,0);p1_tick(40);}
 CHECK(p1_status()==0);CHECK(p1_save());
 CHECK(p1_export(state_a,sizeof(state_a))==n);CHECK(p1_open());CHECK(p1_export(state_b,sizeof(state_b))==n);CHECK(same(state_a,state_b,n));
 /* Full Games navigation: P1 is a real running page, DOWN saves and exits. */
 app_init(918);app_enter_boot();app_key(K_DOWN);app_key(K_CENTER);app_key(K_DOWN);app_key(K_CENTER);
 CHECK(app_page()==PAGE_P1);CHECK(p1_status()==0);
 app_key(K_LEFT);app_tick(40);app_key_release(K_LEFT);app_render();CHECK(app_page()==PAGE_P1);
 p1_test_failure=1;app_key(K_DOWN);CHECK(app_page()==PAGE_P1);CHECK(p1_status()==4);
 p1_test_failure=0;app_key(K_DOWN);CHECK(app_page()==PAGE_GAMES);CHECK(app_selection()==1);
 app_key(K_DOWN);
#ifdef A918_COLOR
 app_key(K_DOWN);
#endif
 app_key(K_CENTER);CHECK(app_return_requested());
 /* Real ROM lifecycle, verified images: set 02:01, hatch, then feed a meal.
  * No clock acceleration: advance virtual game time in 100ms slices. */
 int bytes=p1_read_file("rom",test_rom,sizeof(test_rom));CHECK(bytes==12288);CHECK(p1_load_rom(test_rom,(unsigned)bytes));
 for(unsigned i=0;i<50;i++)p1_tick(100);
 const char *setup="BAABCB";for(const char*s=setup;*s;s++)press((unsigned)(*s-'A'));
 for(unsigned i=0;i<3600;i++)p1_tick(100);
 CHECK(p1_status()==0);CHECK(lcd_hash()==0x9f120531u);
 press(0);press(1);press(1);CHECK(p1_status()==0);CHECK(lcd_hash()==0x998f8721u);
 CHECK(p1_save());CHECK(p1_export(state_a,sizeof(state_a))==n);
 CHECK(p1_open());CHECK(p1_export(state_b,sizeof(state_b))==n);CHECK(same(state_a,state_b,n));
 return 0;
}
