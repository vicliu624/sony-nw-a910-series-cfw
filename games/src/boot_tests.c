#include "app.h"
#define CHECK(x) do{if(!(x))return __LINE__;}while(0)
int boot_selftest(void){
 int rc=blocks_selftest();if(rc)return rc;
 app_init(918);app_enter_boot();CHECK(app_selection()==0);
 app_key(K_OPTION);app_key(K_BACK);CHECK(!app_return_requested());CHECK(app_selection()==0);
 app_key(K_CENTER);CHECK(app_return_requested());
 app_init(918);app_enter_boot();app_key(K_DOWN);app_key(K_CENTER);
 CHECK(app_page()==PAGE_GAMES);CHECK(app_selection()==0);
 for(int i=0;i<200;i++){
  app_key(K_CENTER);CHECK(app_page()==PAGE_BLOCKS);CHECK(app_score()==0);CHECK(!app_game_over());
  app_key(K_DOWN);CHECK(app_score()>0);uint32_t score=app_score();
  app_key(K_BACK);app_key(K_OPTION);CHECK(app_page()==PAGE_BLOCKS);CHECK(app_score()==score);
  app_tick(800);app_render();app_key(K_CENTER);CHECK(app_page()==PAGE_GAMES);CHECK(!app_return_requested());
 }
 app_key(K_DOWN);
#ifdef A918_P1
 app_key(K_DOWN);
#endif
 #ifdef A918_COLOR
 app_key(K_DOWN);
#endif
 app_key(K_CENTER);CHECK(app_return_requested());
 return 0;
}
