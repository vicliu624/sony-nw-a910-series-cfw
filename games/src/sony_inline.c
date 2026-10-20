/* Cooperative Home overlay. No fork/wait/kill/SIGSTOP/input grab/new FB mode.
 * All display pointers/layout come from Sony's existing graphics context.
 * Binary addresses must be validated against the pinned original application.
 */
#include "app.h"
#include <stdint.h>
int raise(int signal){(void)signal;return -1;}
static long sc(long n,long a,long b,long c){
 register long r0 __asm__("r0")=a,r1 __asm__("r1")=b,r2 __asm__("r2")=c,r7 __asm__("r7")=n;
 __asm__ volatile("svc 0":"+r"(r0):"r"(r1),"r"(r2),"r"(r7):"memory","cc");return r0;
}
static uint32_t now_ms(void){int32_t tv[2];if(sc(78,(long)tv,0,0)<0)return 0;return (uint32_t)tv[0]*1000u+(uint32_t)tv[1]/1000u;}
static unsigned char *owner;
static int initialized,active,previous=4,original_selection;
static uint32_t tick;
static uint16_t snapshots[2][240*320];
static uint16_t *saved_buffers[2];
static int graphics_ready(void){
 volatile uint16_t *geometry=(volatile uint16_t*)0x005d1538;
 uint16_t **buffers=(uint16_t**)0x005d9a38;
 uint16_t *draw=*(uint16_t* volatile*)0x005d1534;
 return geometry[0]==240&&geometry[1]==320&&geometry[2]==480&&buffers[0]&&buffers[1]&&buffers[0]!=buffers[1]&&(draw==buffers[0]||draw==buffers[1]);
}
static void snapshot(void){uint16_t **b=(uint16_t**)0x005d9a38;for(int j=0;j<2;j++){saved_buffers[j]=b[j];for(int i=0;i<240*320;i++)snapshots[j][i]=b[j][i];}}
static void initialize(uint32_t time){
 app_init(time);
 initialized=1;
}
static void leave(void){
 active=0;
 /* Restore only still-owned buffers; do not overwrite a reconfigured display. */
 uint16_t **b=(uint16_t**)0x005d9a38;
 for(int j=0;j<2;j++)if(b[j]==saved_buffers[j])for(int i=0;i<240*320;i++)b[j][i]=snapshots[j][i];
}
int sony_direction_edge(int selection,int key){
 if(selection<0||selection>8)return 0;
 return (key==1&&selection<3)||(key==2&&selection>=6)||
        (key==3&&selection%3==0)||(key==4&&selection%3==2);
}
static int translate(int key){static const int map[8]={0,K_UP,K_DOWN,K_LEFT,K_RIGHT,K_CENTER,K_BACK,K_OPTION};return key>0&&key<8?map[key]:0;}
static uint32_t *get_input(void){
 void *root=*(void* volatile*)0x0056383c;if(!root)return 0;
 uint32_t *v=*(uint32_t**)root;return ((uint32_t*(*)(void*))v[8])(root);
}
static int on_home(void){
 void *root=*(void* volatile*)0x0056383c;if(!root)return 0;
 uint32_t *v=*(uint32_t**)root;unsigned char *view=((unsigned char*(*)(void*))v[20])(root);
 return view&&*(uint32_t*)(view+0x960)==11;
}
int sony_inline_update(unsigned char *page){
 uint32_t *input=get_input();if(!input)return 0;
 int selection=*(uint16_t*)(page+0x2f6);
 if(owner!=page){if(active)leave();owner=page;previous=selection;}
 int key=(int)input[6],state=(int)input[7],pending=((unsigned char*)input)[32];
 /* Preserve Sony-owned OPTION/BACK events, including long press/release. */
 if(pending&&(key==7||key==6)){
  if(active){
   leave();*(uint16_t*)(page+0x2f6)=(uint16_t)original_selection;previous=original_selection;
  }
  return 0;
 }
 uint32_t time=now_ms();
 if(!active){
  if(!(pending&&state==4&&sony_direction_edge(previous,key))){previous=selection;return 0;}
  if(!graphics_ready())return 0;
  original_selection=previous;snapshot();if(!initialized)initialize(time);
  app_enter_direction_extension();if(key==1||key==3)app_key(K_RIGHT);
  active=1;tick=time;
 }else{
  unsigned dt=time-tick;tick=time;app_tick(dt>100?100:dt);
  if(pending&&(state==4||(state==7&&app_page()==PAGE_BLOCKS&&(key==2||key==3||key==4)))){
   app_key(translate(key));
  }
  if(app_return_requested()){leave();*(uint16_t*)(page+0x2f6)=(uint16_t)original_selection;previous=original_selection;}
 }
 /* Consume only while the extension owns the Home event. */
 ((unsigned char*)input)[32]=0;input[6]=input[7]=0;
 if(active){app_render();((void(*)(void))0x001bc4ac)();}
 return 1;
}
void sony_inline_present(void){
 if(!active)return;
 if(!on_home()||!graphics_ready()){leave();return;}
 uint16_t *dest=*(uint16_t* volatile*)0x005d1534,*src=app_pixels();
 for(int i=0;i<240*320;i++)dest[i]=src[i];
}
