#include "app.h"
static int same(const void*a,const void*b,unsigned size){const uint8_t*x=a,*y=b;for(unsigned i=0;i<size;i++)if(x[i]!=y[i])return 0;return 1;}
#define CHECK(x) do{if(!(x))return __LINE__;}while(0)
int blocks_selftest(void){
 Blocks g,h;uint8_t save[512];
 for(int p=0;p<7;p++)for(int r=0;r<4;r++){int count=0;for(int y=0;y<4;y++)for(int x=0;x<4;x++)count+=blocks_cell(p,r,x,y);CHECK(count==4);}
 blocks_new(&g,918);CHECK(blocks_valid(&g));
 for(int i=0;i<30;i++)blocks_move(&g,-1,0);int edge=g.x;CHECK(!blocks_move(&g,-1,0));CHECK(g.x==edge);CHECK(blocks_valid(&g));
 g.paused=1;h=g;blocks_tick(&g,900);blocks_move(&g,1,0);blocks_rotate(&g);blocks_drop(&g);CHECK(same(&g,&h,sizeof(g)));
 g.paused=0;int n=blocks_save(&g,save,sizeof(save));CHECK(n==267);CHECK(blocks_load(&h,save,n));CHECK(same(&g,&h,sizeof(g)));
 Blocks old=h;save[23]^=1;CHECK(!blocks_load(&h,save,n));CHECK(same(&old,&h,sizeof(h)));CHECK(!blocks_load(&h,save,n-1));
 blocks_new(&g,33);for(int y=0;y<BH;y++)for(int x=0;x<BW;x++)g.board[y][x]=0;
 for(int y=18;y<20;y++)for(int x=0;x<10;x++)if(x!=5&&x!=6)g.board[y][x]=3;
 g.piece=1;g.rotation=0;g.x=4;g.y=18;g.score=0;CHECK(blocks_valid(&g));blocks_drop(&g);CHECK(g.lines==2);CHECK(g.score==300);
 for(int x=0;x<BW;x++)CHECK(g.board[19][x]==0);
 for(unsigned seed=1;seed<40;seed++){blocks_new(&g,seed);for(int i=0;i<150;i++){
   if(g.over)blocks_new(&g,seed+(unsigned)i+1);blocks_rotate(&g);for(int j=0;j<(i%6);j++)blocks_move(&g,i%2?-1:1,0);blocks_drop(&g);CHECK(blocks_valid(&g));
   n=blocks_save(&g,save,sizeof(save));CHECK(n==267);CHECK(blocks_load(&h,save,n));CHECK(same(&g,&h,sizeof(g)));
 }}
 return 0;
}
int app_selftest(void){
 int r=blocks_selftest();if(r)return r;
 app_init(918);CHECK(app_page()==PAGE_HOME);CHECK(app_selection()==4);
 app_key(K_DOWN);app_key(K_DOWN);CHECK(app_selection()==9);app_key(K_RIGHT);CHECK(app_selection()==10);app_key(K_CENTER);CHECK(app_page()==PAGE_INFO);
 app_key(K_BACK);CHECK(app_page()==PAGE_HOME);app_key(K_LEFT);CHECK(app_selection()==9);app_key(K_CENTER);CHECK(app_page()==PAGE_GAMES);
 CHECK(app_selection()==0);
 for(int round=0;round<100;round++){
  app_key(K_CENTER);CHECK(app_page()==PAGE_BLOCKS);CHECK(app_score()==0);CHECK(!app_game_over());
  app_key(K_DOWN);CHECK(app_score()>0);uint32_t before_score=app_score();
  app_key(K_OPTION);app_key(K_BACK);CHECK(app_page()==PAGE_BLOCKS);CHECK(app_score()==before_score);
  app_key(K_CENTER);CHECK(app_page()==PAGE_GAMES);CHECK(app_selection()==0);
 }
 app_key(K_DOWN);app_key(K_CENTER);CHECK(app_page()==PAGE_HOME);CHECK(app_selection()==9);
 /* Native extension must never route to any of the local Sony placeholders. */
 app_enter_extension();CHECK(app_page()==PAGE_HOME);CHECK(app_selection()==9);CHECK(!app_return_requested());
 app_key(K_RIGHT);CHECK(app_selection()==10);app_key(K_CENTER);CHECK(app_page()==PAGE_INFO);
 app_key(K_BACK);CHECK(app_selection()==10);app_key(K_LEFT);CHECK(app_selection()==9);
 app_key(K_CENTER);CHECK(app_page()==PAGE_GAMES);app_key(K_BACK);CHECK(app_selection()==9);CHECK(!app_return_requested());
 app_key(K_BACK);CHECK(app_return_requested());
 for(int key=K_LEFT;key<=K_OPTION;key++){
  if(key==K_CENTER)continue;app_enter_extension();app_key(key);
  CHECK(app_selection()==9||app_selection()==10);
 }
 app_enter_extension();app_key(K_RIGHT);app_key(K_RIGHT);CHECK(app_return_requested());
 app_enter_direction_extension();app_key(K_OPTION);CHECK(!app_return_requested());CHECK(app_selection()==9);
 app_key(K_RIGHT);CHECK(app_selection()==10);app_key(K_DOWN);CHECK(app_return_requested());
 app_render();return 0;
}
