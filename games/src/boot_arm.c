/* Independent boot chooser. Sony has not been launched; no hooks or process stop.
 * QFB sequence follows stock Gemini 0x1bc6e0 / 0x1bca08. Hardware validation pending.
 */
#include "app.h"
#include <stddef.h>
extern long a918_oabi_call(long n,long a,long b,long c,long d,long e,long f);
static long syscall6(long n,long a,long b,long c,long d,long e,long f){return a918_oabi_call(n,a,b,c,d,e,f);}
#define SC(n,a,b,c) syscall6(n,(long)(a),(long)(b),(long)(c),0,0,0)
static int heartbeat=-1;
#ifdef A918_STANDALONE
static int outer_heartbeat=-1;
static void outer_pulse(char b){if(outer_heartbeat>=0)SC(4,outer_heartbeat,&b,1);}
#else
static void outer_pulse(char b){(void)b;}
#endif
#if defined(A918_LAUNCHER) || (defined(A918_STANDALONE) && !defined(A918_COLOR))
static int game_storage_ready;
#endif
#ifdef A918_LAUNCHER
#include "launcher_platform.inc"
#endif
#ifdef A918_P1
#include "p1_platform.inc"
#endif
#ifdef A918_COLOR
#ifdef A918_EXTERNAL
#include "color_disk_platform.inc"
#else
#include "color_platform.inc"
#endif
#endif
static int eq(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
static void logline(const char*s){int n=0;while(s[n])n++;SC(4,2,s,n);}
static uint32_t now_ms(void){long tv[2];if(SC(78,tv,0,0)<0)return 0;return(uint32_t)tv[0]*1000+(uint32_t)tv[1]/1000;}
static void sleep_ms(void){outer_pulse(1);if(heartbeat>=0){char b=1;SC(4,heartbeat,&b,1);}long ts[2]={0,16000000};SC(162,ts,0,0);}
#ifdef A918_COLOR_PERF
static void color_yield(void){outer_pulse(1);if(heartbeat>=0){char b=1;SC(4,heartbeat,&b,1);}long ts[2]={0,1000000};SC(162,ts,0,0);}
#endif
static volatile int stopped;
static void stop(int sig,void*info,void*ctx){(void)sig;(void)info;(void)ctx;stopped=1;}
__attribute__((naked))static void sigreturn_stub(void){__asm__ volatile("svc 0x9000ad");}
static void install_signals(void){
 struct{void(*fn)(int,void*,void*);unsigned flags;void(*restorer)(void);unsigned mask[2];}a={stop,0x04000004,sigreturn_stub,{0,0}};
 syscall6(174,2,(long)&a,0,8,0,0);syscall6(174,15,(long)&a,0,8,0,0);
}
#ifdef A918_LAUNCHER
#include "launcher_watchdog.inc"
#endif
struct event32{int32_t sec,usec;uint16_t type,code;int32_t value;};
static int keycode(unsigned raw){switch(raw){case 2:return K_LEFT;case 1:return K_RIGHT;case 3:return K_UP;case 4:return K_DOWN;case 0:return K_CENTER;default:return 0;}}
static int present(int fb,uint16_t*mapped,unsigned page){
 uint16_t*src=app_pixels(),*dest=mapped+page*0x20000;
 for(int i=0;i<240*320;i++)dest[i]=src[i];
 /* plane visible, RGB565, full opacity, physical page, no extra overlay */
 unsigned args[6]={1,0x800,255,page,0,0};
 return SC(54,fb,0x40044e01,args)>=0;
}
extern int boot_selftest(void);
static int interactive_option(const char*s){if(eq(s,"--boot")||eq(s,"--boot-no-storage"))return 1;
#ifdef A918_EXTERNAL
 if(eq(s,"--choose")||eq(s,"--games")||eq(s,"--games-no-storage"))return 1;
#endif
 return 0;}
static int run(int argc,char**argv){
#ifdef A918_STANDALONE
 if(argc==2&&eq(argv[1],"--app-heartbeat-test")){outer_pulse(1);outer_pulse(2);outer_pulse(1);return 0;}
 if(argc==2&&eq(argv[1],"--app-test")){extern int standalone_disk_test(void);return standalone_disk_test();}
#endif
#ifdef A918_LAUNCHER
 if(argc==2&&eq(argv[1],"--run-app"))return launcher_run_app();
 if(argc==3&&eq(argv[1],"--stage"))return launcher_stage(argv[2]);
 if(argc==2&&eq(argv[1],"--catalog-test"))return launcher_catalog_test();
#endif
#ifdef A918_EXTERNAL
 if(argc==2&&eq(argv[1],"--usb-present")){int fd=SC(5,"/dev/usbconn",0,0),state=-1;if(fd<0)return 0;int ok=SC(54,fd,0x80046701,&state)>=0;SC(6,fd,0,0);return ok&&state==0?1:0;}
 if(argc==2&&(eq(argv[1],"--boot-no-storage")||eq(argv[1],"--games-no-storage")||eq(argv[1],"--choose")))game_storage_ready=0;
#endif
#if defined(A918_COLOR_MENU) && !defined(A918_STANDALONE)
 if(argc==3&&eq(argv[1],"--color-menu-test")){extern int color_menu_selftest(void);color_test_path=argv[2];return color_menu_selftest();}
#endif
#ifdef A918_COLOR
#ifndef A918_EXTERNAL
 if(argc==3&&eq(argv[1],"--color-selftest")){extern int color_selftest(void);color_test_path=argv[2];return color_selftest()?1:0;}
#endif
 #ifndef A918_STANDALONE
 if(argc==3&&eq(argv[1],"--color-rom-test")){extern int color_real_rom_selftest(void);color_test_path=argv[2];return color_real_rom_selftest();}
 #endif
#endif
#ifdef A918_P1
 if(argc==3&&eq(argv[1],"--pet-selftest")){extern int p1_selftest(void);p1_test_mode=1;p1_test_rom_path=argv[2];return p1_selftest()?1:0;}
 if(argc==2&&eq(argv[1],"--boot-no-storage"))p1_storage_ready=0;
#endif
 if(argc==2&&eq(argv[1],"--selftest"))return boot_selftest()?1:0;
 if(argc!=2||!interactive_option(argv[1]))return 64;
 install_signals();
 int fb=(int)SC(5,"/dev/fb/0",2,0),input=-1,usb=-1,result=77,grabbed=0;
 uint16_t*mapped=0;
 if(fb<0){logline("boot: no framebuffer; continue to Walkman\n");return result;}
 /* Original QFB timing control, not a read-only query (kernel qfb_ioctl).
  * The {0,33} setup matches stock Gemini. No driver or kernel code is changed. */
 unsigned query[2]={0,33};
 if(SC(54,fb,0x80044e04,query)<0){result=78;goto cleanup;}
 long address=syscall6(192,0,0x80000,3,1,fb,0);
 if((unsigned long)address>=0xfffff001u){result=78;goto cleanup;}
 mapped=(uint16_t*)address;
 input=(int)SC(5,"/dev/input/event0",2048,0);if(input<0){result=79;goto cleanup;}
 if(SC(54,input,0x40044590,1)<0){result=80;goto cleanup;}grabbed=1;
 usb=(int)SC(5,"/dev/usbconn",0,0);int state=-1;
 /* Preserve Sony USB servicing: immediately start Walkman on cable presence
  * or unreadable status. Stock AscUsbConn getter uses this read-only request. */
 if(usb<0||SC(54,usb,0x80046701,&state)<0||state!=0){result=0;goto cleanup;}
 app_init(now_ms());app_enter_boot();
#ifdef A918_EXTERNAL
 if(eq(argv[1],"--choose"))app_boot_choice_only();
 if(eq(argv[1],"--games")||eq(argv[1],"--games-no-storage"))app_enter_games();
#endif
 unsigned setup[6]={1,0x800,255,0,0,0};
 if(SC(54,fb,0x40044e02,setup)<0){result=78;goto cleanup;}
 uint32_t started=now_ms(),last=started,last_input=started,repeat_at=0,usb_at=started;
 unsigned held=0,page=0;int repeat=0,used=0,count=0;struct event32 event;
#ifdef A918_EXTERNAL
 if(eq(argv[1],"--games")||eq(argv[1],"--games-no-storage"))used=1;
#endif
#ifdef A918_COLOR_PERF
 uint32_t drawn_at=started;int drawn_page=-1;
#endif
 result=0;
 while(!stopped&&!app_return_requested()){
  uint32_t now=now_ms(),elapsed=now-last;last=now;if(elapsed>100)elapsed=100;
  for(int limit=0;limit<64;limit++){
   int n=(int)SC(3,input,(char*)&event+count,16-count);
   if(n==-11||n==-4)break;if(n<=0){result=79;goto cleanup;}
   count+=n;if(count<16)continue;count=0;
   if(event.type!=1)continue;int key=keycode(event.code);if(!key)continue;
   /* Original icx1087_key.ko emits value 0 for a new press and 1 for
    * release (input_event calls at .text+0x254 / +0x220). */
   if(event.value==0){
    if(held&(1u<<key))continue;
    held|=1u<<key;used=1;last_input=now;int oldpage=app_page();app_key(key);
    repeat=(app_page()==PAGE_BLOCKS&&oldpage==PAGE_BLOCKS&&(key==K_LEFT||key==K_RIGHT||key==K_DOWN))?key:0;
    repeat_at=now+180;
   }else if(event.value==1){held&=~(1u<<key);app_key_release(key);if(repeat==key)repeat=0;}
  }
  if(!used&&(uint32_t)(now-started)>=8000)break;
  if((uint32_t)(now-last_input)>=120000&&app_page()!=PAGE_BLOCKS&&app_page()!=PAGE_P1&&app_page()!=PAGE_COLOR)break;
  if(repeat&&(int32_t)(now-repeat_at)>=0){app_key(repeat);repeat_at=now+65;}
  if((uint32_t)(now-usb_at)>=100){usb_at=now;if(SC(54,usb,0x80046701,&state)<0||state!=0)break;}
  app_tick(elapsed);
#ifdef A918_COLOR_PERF
  /* Input/USB/ticking continue between frames. Page changes draw immediately. */
  uint32_t draw_now=now_ms();int current=app_page();
  if(current!=PAGE_COLOR||current!=drawn_page||(uint32_t)(draw_now-drawn_at)>=50){
   app_render();if(!present(fb,mapped,page)){result=78;break;}page^=1;drawn_at=draw_now;drawn_page=current;
  }
  if(current==PAGE_COLOR)color_yield();else sleep_ms();
#else
  app_render();if(!present(fb,mapped,page)){result=78;break;}page^=1;sleep_ms();
#endif
 }
 /* Drain release of CENTER before Sony opens the input device. Bounded. */
 started=now_ms();while(held&&!stopped&&(uint32_t)(now_ms()-started)<700){
  int n=(int)SC(3,input,(char*)&event+count,16-count);
  if(n>0){count+=n;if(count==16){count=0;int key=keycode(event.code);if(event.type==1&&event.value==1&&key)held&=~(1u<<key);}}
  sleep_ms();
 }
cleanup:
 if(!app_shutdown())logline("boot: pet save failed before return\n");
 if(grabbed)SC(54,input,0x40044590,0);
 if(input>=0)SC(6,input,0,0);if(usb>=0)SC(6,usb,0,0);
 if(mapped)SC(91,mapped,0x80000,0);SC(6,fb,0,0);
 logline("boot: chooser finished; continue to Walkman\n");
#ifdef A918_EXTERNAL
 #ifdef A918_LAUNCHER
 if(result==0&&app_launch_code())return app_launch_code();
#endif
 if(result==0&&app_games_requested())return 90;
#endif
 return result;
}
int raise(int sig){(void)sig;return -1;}
static int supervised(int argc,char**argv){
 int test=0;
 if(argc==2){
  if(eq(argv[1],"--supervisor-test-normal"))test=1;
  else if(eq(argv[1],"--supervisor-test-crash"))test=2;
  else if(eq(argv[1],"--supervisor-test-stall"))test=3;
  else if(eq(argv[1],"--supervisor-test-signal"))test=4;
  else if(eq(argv[1],"--supervisor-test-save-grace"))test=5;
 }
 if(!test&&(argc!=2||!interactive_option(argv[1])))return run(argc,argv);
 int pipefd[2];if(SC(42,pipefd,0,0)<0)return 83;
 SC(55,pipefd[0],4,2048);SC(55,pipefd[1],4,2048);install_signals();
 int child=(int)SC(2,0,0,0);
 if(child<0){SC(6,pipefd[0],0,0);SC(6,pipefd[1],0,0);return 83;}
 if(!child){
  SC(6,pipefd[0],0,0);heartbeat=pipefd[1];int rc;
  if(test==1)rc=0;
  else if(test==2){SC(37,SC(20,0,0,0),9,0);rc=99;}
  else if(test==3){long ts[2]={10,0};SC(162,ts,0,0);rc=99;}
  else if(test==4){SC(37,SC(20,0,0,0),15,0);rc=stopped?0:99;}
  else if(test==5){char b=2;SC(4,heartbeat,&b,1);long ts[2]={0,350000000};SC(162,ts,0,0);b=1;SC(4,heartbeat,&b,1);rc=0;}
  else rc=run(argc,argv);
  SC(1,rc,0,0);for(;;){}
 }
#ifdef A918_STANDALONE
 outer_heartbeat=-1; /* Only the worker may keep the independent launcher alive. */
#endif
 SC(6,pipefd[1],0,0);uint32_t last=now_ms(),allowance=5000;int status=0,result=84;
 for(;;){
  int waited=(int)SC(114,child,&status,1);
  if(waited==child){result=(status&127)?85:(status>>8)&255;break;}
  if(waited<0&&waited!=-4){result=86;break;}
  char b[128];int n=(int)SC(3,pipefd[0],b,sizeof(b));if(n>0){last=now_ms();for(int i=0;i<n;i++)allowance=b[i]==2?30000:5000;}
  if(stopped||(uint32_t)(now_ms()-last)>(test?(test==5&&allowance==30000?1000u:250u):allowance)){
   SC(37,child,9,0);while(SC(114,child,&status,0)==-4){}result=84;break;
  }
  sleep_ms();
 }
 SC(6,pipefd[0],0,0);
 if(test)return result==((test==1||test==4||test==5)?0:test==2?85:84)?0:1;
 return result;
}
__attribute__((noreturn))void boot_start(unsigned long*stack){
#ifdef A918_STANDALONE
 char**env=(char**)&stack[stack[0]+2];const char*prefix="A918_APP_HEARTBEAT_FD=";
 for(;*env;env++){const char*p=*env,*q=prefix;while(*q&&*p==*q){p++;q++;}if(!*q){unsigned n=0;int valid=1;for(;*p;p++){if(*p<'0'||*p>'9'||n>1000){valid=0;break;}n=n*10+*p-'0';}if(valid&&n>=3&&n<=1024)outer_heartbeat=n;}}
#endif
 int result=supervised((int)stack[0],(char**)&stack[1]);SC(1,result,0,0);for(;;){}}
__attribute__((naked,noreturn))void _start(void){__asm__ volatile("mov r0,sp\nb boot_start");}
