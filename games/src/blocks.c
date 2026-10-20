#include "blocks.h"
static const uint16_t shapes[7][4]={
 {0x00f0,0x4444,0x0f00,0x2222}, {0x0066,0x0066,0x0066,0x0066},
 {0x0072,0x0262,0x0270,0x0232}, {0x0036,0x0462,0x0360,0x0231},
 {0x0063,0x0264,0x0630,0x0132}, {0x0071,0x0226,0x0470,0x0322},
 {0x0074,0x0622,0x0170,0x0223}
};
int blocks_cell(int p,int r,int x,int y){return p>=0&&p<7&&x>=0&&x<4&&y>=0&&y<4&&((shapes[p][r&3]>>(y*4+x))&1);}
static uint32_t random32(Blocks*g){uint32_t x=g->rng;x^=x<<13;x^=x>>17;x^=x<<5;return g->rng=x;}
static int take(Blocks*g){
 if(g->bag_pos>=7){for(int i=0;i<7;i++)g->bag[i]=(uint8_t)i;for(int i=6;i>0;i--){int j=(int)(random32(g)%(uint32_t)(i+1));uint8_t t=g->bag[i];g->bag[i]=g->bag[j];g->bag[j]=t;}g->bag_pos=0;}
 return g->bag[g->bag_pos++];
}
static int fits(const Blocks*g,int x,int y,int rotation){
 for(int a=0;a<4;a++)for(int b=0;b<4;b++)if(blocks_cell(g->piece,rotation,b,a)){
  int xx=x+b,yy=y+a;if(xx<0||xx>=BW||yy>=BH)return 0;if(yy>=0&&g->board[yy][xx])return 0;
 }return 1;
}
static void spawn(Blocks*g){g->piece=g->next;g->next=take(g);g->rotation=0;g->x=3;g->y=-1;g->elapsed=0;if(!fits(g,g->x,g->y,0))g->over=1;}
void blocks_new(Blocks*g,uint32_t seed){
 uint8_t*p=(uint8_t*)g;for(unsigned i=0;i<sizeof(*g);i++)p[i]=0;
 g->rng=seed?seed:0x9180123u;g->bag_pos=7;g->next=take(g);spawn(g);
}
static void lock_piece(Blocks*g){
 int above=0;for(int a=0;a<4;a++)for(int b=0;b<4;b++)if(blocks_cell(g->piece,g->rotation,b,a)){
  int x=g->x+b,y=g->y+a;if(y<0)above=1;else if(y<BH&&x>=0&&x<BW)g->board[y][x]=(uint8_t)(g->piece+1);
 }if(above){g->over=1;return;}
 int cleared=0;for(int y=BH-1;y>=0;y--){int full=1;for(int x=0;x<BW;x++)if(!g->board[y][x])full=0;
  if(full){cleared++;for(int row=y;row>0;row--)for(int x=0;x<BW;x++)g->board[row][x]=g->board[row-1][x];for(int x=0;x<BW;x++)g->board[0][x]=0;y++;}
 }
 static const unsigned points[5]={0,100,300,500,800};g->score+=points[cleared]*(g->lines/10+1);g->lines+=(unsigned)cleared;spawn(g);
}
int blocks_move(Blocks*g,int dx,int dy){
 if(g->paused||g->over||dx < -1||dx>1||dy<0||dy>1)return 0;
 if(fits(g,g->x+dx,g->y+dy,g->rotation)){g->x+=dx;g->y+=dy;if(dy)g->score++;return 1;}
 if(dy)lock_piece(g);return 0;
}
int blocks_rotate(Blocks*g){
 if(g->paused||g->over)return 0;int r=(g->rotation+1)&3;static const int kick[7][2]={{0,0},{-1,0},{1,0},{-2,0},{2,0},{0,-1},{0,-2}};
 for(int i=0;i<7;i++)if(fits(g,g->x+kick[i][0],g->y+kick[i][1],r)){g->x+=kick[i][0];g->y+=kick[i][1];g->rotation=r;return 1;}return 0;
}
void blocks_drop(Blocks*g){if(g->paused||g->over)return;while(fits(g,g->x,g->y+1,g->rotation)){g->y++;g->score+=2;}lock_piece(g);}
void blocks_tick(Blocks*g,uint32_t ms){
 if(g->paused||g->over)return;if(ms>1000)ms=1000;g->elapsed+=ms;unsigned level=g->lines/10,delay=level>=12?100:700-level*50;
 while(g->elapsed>=delay&&!g->over){g->elapsed-=delay;if(fits(g,g->x,g->y+1,g->rotation))g->y++;else{lock_piece(g);break;}}
}
int blocks_valid(const Blocks*g){
 if(g->piece<0||g->piece>6||g->next<0||g->next>6||g->rotation<0||g->rotation>3||g->x < -3||g->x>9||g->y < -3||g->y>19||g->bag_pos<0||g->bag_pos>7||!g->rng||g->paused<0||g->paused>1||g->over<0||g->over>1||g->elapsed>1000||g->score>1000000000u||g->lines>1000000u)return 0;
 unsigned seen=0;for(int i=0;i<7;i++){if(g->bag[i]>6||(seen&(1u<<g->bag[i])))return 0;seen|=1u<<g->bag[i];}
 for(int y=0;y<BH;y++)for(int x=0;x<BW;x++)if(g->board[y][x]>7)return 0;
 return g->over||fits(g,g->x,g->y,g->rotation);
}
static uint32_t checksum(const uint8_t*b,int n){uint32_t c=0xffffffff;for(int i=0;i<n;i++){c^=b[i];for(int j=0;j<8;j++)c=(c>>1)^((0u-(c&1u))&0xedb88320u);}return ~c;}
static void put(uint8_t*b,int*o,uint32_t v){for(int i=0;i<4;i++)b[(*o)++]=(uint8_t)(v>>(8*i));}
static uint32_t get(const uint8_t*b,int*o){uint32_t v=0;for(int i=0;i<4;i++)v|=(uint32_t)b[(*o)++]<<(8*i);return v;}
#define SAVE_SIZE 267
int blocks_save(const Blocks*g,uint8_t*b,int capacity){
 if(capacity<SAVE_SIZE||!blocks_valid(g))return 0;int o=0;put(b,&o,0x42383139);put(b,&o,1);
 for(int y=0;y<BH;y++)for(int x=0;x<BW;x++)b[o++]=g->board[y][x];for(int i=0;i<7;i++)b[o++]=g->bag[i];
 put(b,&o,g->piece);put(b,&o,g->next);put(b,&o,g->rotation);put(b,&o,g->x);put(b,&o,g->y);put(b,&o,g->bag_pos);
 put(b,&o,g->rng);put(b,&o,g->score);put(b,&o,g->lines);put(b,&o,g->elapsed);put(b,&o,g->paused);put(b,&o,g->over);
 put(b,&o,checksum(b,o));return o;
}
int blocks_load(Blocks*g,const uint8_t*b,int length){
 if(length!=SAVE_SIZE)return 0;int tail=length-4;uint32_t expected=get(b,&tail);if(expected!=checksum(b,length-4))return 0;
 int o=0;if(get(b,&o)!=0x42383139||get(b,&o)!=1)return 0;Blocks v={0};
 for(int y=0;y<BH;y++)for(int x=0;x<BW;x++)v.board[y][x]=b[o++];for(int i=0;i<7;i++)v.bag[i]=b[o++];
 v.piece=(int32_t)get(b,&o);v.next=(int32_t)get(b,&o);v.rotation=(int32_t)get(b,&o);v.x=(int32_t)get(b,&o);v.y=(int32_t)get(b,&o);v.bag_pos=(int32_t)get(b,&o);
 v.rng=get(b,&o);v.score=get(b,&o);v.lines=get(b,&o);v.elapsed=get(b,&o);v.paused=(int32_t)get(b,&o);v.over=(int32_t)get(b,&o);
 if(!blocks_valid(&v))return 0;*g=v;return 1;
}
