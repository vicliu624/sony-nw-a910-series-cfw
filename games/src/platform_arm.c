/* Freestanding ARMv5 Linux adapter: no dependency on the device's old libc.
 * R6 Home hook calls --sony-menu. Only its supervised worker may draw/grab.
 * Hardware compatibility and repeated Sony round trips still need testing.
 */
#include "app.h"
#include <linux/fb.h>
#include <stddef.h>
static long call6(long n,long a,long b,long c,long d,long e,long f){
 register long r0 __asm__("r0")=a,r1 __asm__("r1")=b,r2 __asm__("r2")=c,r3 __asm__("r3")=d,r4 __asm__("r4")=e,r5 __asm__("r5")=f,r7 __asm__("r7")=n;
 __asm__ volatile("svc 0":"+r"(r0):"r"(r1),"r"(r2),"r"(r3),"r"(r4),"r"(r5),"r"(r7):"memory","cc");return r0;
}
#define SC(n,a,b,c) call6(n,(long)(a),(long)(b),(long)(c),0,0,0)
static int open_file(const char*p,int flags,int mode){return(int)SC(5,p,flags,mode);}
static int close_file(int fd){return(int)SC(6,fd,0,0);}
static int read_file(int fd,void*p,int n){return(int)SC(3,fd,p,n);}
static int write_file(int fd,const void*p,int n){return(int)SC(4,fd,p,n);}
static int ioctl_file(int fd,unsigned request,void*p){return(int)SC(54,fd,request,p);}
static unsigned length(const char*s){unsigned n=0;while(s[n])n++;return n;}
static int equal(const char*a,const char*b){while(*a&&*a==*b)a++,b++;return *a==*b;}
static void print(const char*s){write_file(1,s,(int)length(s));}
static void decimal(unsigned v){char b[12],r[12];int n=0;do{b[n++]=(char)('0'+v%10);v/=10;}while(v);for(int i=0;i<n;i++)r[i]=b[n-1-i];write_file(1,r,n);}
static uint32_t millis(void){long tv[2];if(SC(78,tv,0,0)<0)return 0;return(uint32_t)tv[0]*1000u+(uint32_t)tv[1]/1000u;}
static int heartbeat_fd=-1;
static uint32_t heartbeat_last;
static void delay(unsigned ms){
 if(heartbeat_fd>=0&&(uint32_t)(millis()-heartbeat_last)>=250){char byte=1;write_file(heartbeat_fd,&byte,1);heartbeat_last=millis();}
 long ts[2]={(long)(ms/1000),(long)(ms%1000)*1000000};SC(162,ts,0,0);
}
static volatile int interrupted;
static void stop_handler(int sig,void *info,void *context){(void)sig;(void)info;(void)context;interrupted=1;}
__attribute__((naked))static void sigreturn_stub(void){__asm__ volatile("mov r7,#173\nsvc 0");}
static void signals(void){
 /* ARM chooses a legacy frame unless SA_SIGINFO is set. rt_sigreturn must
  * be paired with an RT frame, even when installed via rt_sigaction. */
 struct{void(*handler)(int,void*,void*);unsigned flags;void(*restorer)(void);unsigned mask[2];}action={stop_handler,0x04000004,sigreturn_stub,{0,0}};
 call6(174,2,(long)&action,0,8,0,0);call6(174,15,(long)&action,0,8,0,0);
 action.handler=(void(*)(int,void*,void*))1;action.flags=0;action.restorer=0;call6(174,13,(long)&action,0,8,0,0);
}
static uint16_t *hardware;
static unsigned stride,map_size;
static uint16_t backup[240*320];
static void flip(void){uint16_t*p=app_pixels();for(int y=0;y<320;y++)for(int x=0;x<240;x++)hardware[y*stride+x]=p[y*240+x];}
static void restore(void){for(int y=0;y<320;y++)for(int x=0;x<240;x++)hardware[y*stride+x]=backup[y*240+x];}
static int setup_framebuffer(int fd,int map){
 struct fb_fix_screeninfo fixed;struct fb_var_screeninfo variable;
 if(ioctl_file(fd,FBIOGET_FSCREENINFO,&fixed)<0||ioctl_file(fd,FBIOGET_VSCREENINFO,&variable)<0){print("Framebuffer ioctl failed; refusing guessed layout.\n");return 0;}
 print("FB: ");decimal(variable.xres);print("x");decimal(variable.yres);print(" bpp=");decimal(variable.bits_per_pixel);print(" stride=");decimal(fixed.line_length);print("\n");
 if(variable.xres!=240||variable.yres!=320||variable.bits_per_pixel!=16||variable.xoffset||variable.yoffset||variable.red.offset!=11||variable.green.offset!=5||variable.blue.offset!=0||variable.red.length!=5||variable.green.length!=6||variable.blue.length!=5||fixed.line_length<480||fixed.line_length>4096||fixed.line_length%2){print("Unsupported framebuffer format.\n");return 0;}
 stride=fixed.line_length/2;map_size=fixed.line_length*320;
 if(fixed.smem_len<map_size){print("Framebuffer too short.\n");return 0;}
 if(!map)return 1;
 long result=call6(192,0,map_size,3,1,fd,0);
 if((unsigned long)result>=0xfffff001u){print("Framebuffer mapping failed.\n");return 0;}
 hardware=(uint16_t*)result;
 for(int y=0;y<320;y++)for(int x=0;x<240;x++)backup[y*240+x]=hardware[y*stride+x];
 return 1;
}
struct input_event32{int32_t sec,usec;uint16_t type,code;int32_t value;};
static unsigned char event_bytes[16];static int event_count;
static unsigned down_mask;
static int read_event(int fd,struct input_event32*ev){
 int n=read_file(fd,event_bytes+event_count,16-event_count);if(n<=0)return 0;event_count+=n;if(event_count<16)return 0;
 unsigned char*p=(unsigned char*)ev;for(int i=0;i<16;i++)p[i]=event_bytes[i];event_count=0;return 1;
}
static int atomic_write(const char*temp,const char*path,const void*data,int size){
 int fd=open_file(temp,1|64|512,0600);if(fd<0)return 0;
 int offset=0,ok=1;while(offset<size){int n=write_file(fd,(const unsigned char*)data+offset,size-offset);if(n<=0){ok=0;break;}offset+=n;}
 if(SC(118,fd,0,0)<0)ok=0;close_file(fd);
 if(ok&&SC(38,temp,path,0)<0)ok=0;
 if(ok){int dir=open_file("/var/a918-games",0|65536,0);if(dir<0)ok=0;else{if(SC(118,dir,0,0)<0)ok=0;close_file(dir);}}
 if(!ok)SC(10,temp,0,0);return ok;
}
static uint16_t codes[8],pressed_value;
static int load_keys(void){
 unsigned char b[20];int fd=open_file("/var/a918-games/keys.cfg",0,0);if(fd<0)return 0;int n=read_file(fd,b,20);close_file(fd);
 if(n!=20||b[0]!='K'||b[1]!='E'||b[2]!='Y'||b[3]!='1')return 0;
 pressed_value=(uint16_t)(b[18]|b[19]<<8);if(pressed_value>1)return 0;
 for(int i=1;i<=7;i++){codes[i]=(uint16_t)(b[2+i*2]|b[3+i*2]<<8);if(codes[i]>255)return 0;for(int j=1;j<i;j++)if(codes[i]==codes[j])return 0;}return 1;
}
static int calibrate(int fd){
 static const char*names[8]={"","PRESS LEFT","PRESS RIGHT","PRESS UP","PRESS DOWN","PRESS CENTER","PRESS BACK","PRESS OPTION"};
 struct input_event32 ev;
 for(int key=1;key<=7;key++){
  while(read_event(fd,&ev)){}app_prompt(names[key],"PRESS AND RELEASE ONCE");flip();
  uint32_t start=millis();int found=0;
  while(!interrupted&&(uint32_t)(millis()-start)<20000){
   if(read_event(fd,&ev)&&ev.type==1&&ev.code<64&&(ev.value==0||ev.value==1)){
    int duplicate=0;for(int i=1;i<key;i++)if(codes[i]==ev.code)duplicate=1;if(duplicate)continue;
    codes[key]=ev.code;if(key==1)pressed_value=(uint16_t)ev.value;else if(ev.value!=pressed_value)continue;
    found=1;break;
   }delay(8);
  }if(!found)return 0;
  int released=0;start=millis();while(!interrupted&&(uint32_t)(millis()-start)<10000){if(read_event(fd,&ev)&&ev.type==1&&ev.code==codes[key]&&ev.value!=pressed_value){released=1;break;}delay(8);}if(!released)return 0;delay(150);
 }
 unsigned char b[20]={'K','E','Y','1'};for(int i=1;i<=7;i++){b[2+i*2]=(unsigned char)codes[i];b[3+i*2]=(unsigned char)(codes[i]>>8);}b[18]=(unsigned char)pressed_value;b[19]=0;
 return atomic_write("/var/a918-games/keys.tmp","/var/a918-games/keys.cfg",b,20);
}
static void drain_releases(int fd){
 /* Do not hand Sony an orphan BACK/OPTION release from our exit gesture. */
 uint32_t start=millis();struct input_event32 ev;
 while(down_mask&&(uint32_t)(millis()-start)<(interrupted?150u:700u)){
  while(read_event(fd,&ev))if(ev.type==1)for(int key=1;key<=7;key++)if(codes[key]==ev.code){
   if(ev.value==pressed_value)down_mask|=1u<<key;else if(ev.value==1-pressed_value)down_mask&=~(1u<<key);
  }
  delay(8);
 }
}
static void load_game(void){int fd=open_file("/var/a918-games/game.sav",0,0);if(fd>=0){int n=read_file(fd,app_save_buffer(),512);close_file(fd);if(n>0&&!app_import(n))print("Rejected invalid saved game.\n");}}
static unsigned char last_saved[512];static int last_saved_size;
static void save_game(void){
 int n=app_export();if(!n)return;int different=n!=last_saved_size;
 for(int i=0;i<n&&!different;i++)if(last_saved[i]!=app_save_buffer()[i])different=1;
 if(!different)return;
 if(!atomic_write("/var/a918-games/game.tmp","/var/a918-games/game.sav",app_save_buffer(),n)){print("Save failed.\n");return;}
 for(int i=0;i<n;i++)last_saved[i]=app_save_buffer()[i];last_saved_size=n;
}
int arm_main(int argc,char**argv);
#include "sony_supervisor.inc"
int arm_main(int argc,char**argv){
 if(argc>1&&equal(argv[1],"--selftest")){int result=app_selftest();if(result){print("FAIL line ");decimal((unsigned)result);print("\n");return 1;}print("ARM selftest PASS\n");return 0;}
 if(argc>1&&equal(argv[1],"--handoff-selftest"))return handoff_selftest();
 if(argc>1&&equal(argv[1],"--sony-menu"))return sony_supervise(0);
 int extension=argc>1&&equal(argv[1],"--run-extension");
 int probe=argc>1&&equal(argv[1],"--probe");
 if(!probe&&!extension&&!(argc>1&&equal(argv[1],"--run-games"))){print("Usage: a918-games --selftest | --handoff-selftest | --probe | --sony-menu\n");return 0;}
 if(!probe&&heartbeat_fd<0){print("Refusing unsupervised display/input takeover.\n");return 82;}
 int fb=open_file("/dev/fb/0",probe?0:2,0);if(fb<0){print("Cannot open framebuffer.\n");return 77;}
 if(!setup_framebuffer(fb,!probe)){close_file(fb);return 78;}
 if(probe){close_file(fb);print("Probe is read-only. Input event mapping still requires calibration.\n");return 0;}
 signals();int fd=open_file("/dev/input/event0",0|2048,0);if(fd<0){restore();SC(91,hardware,map_size,0);close_file(fb);return 79;}
 /* Fail closed if exclusive input ownership cannot be established. */
 if(ioctl_file(fd,0x40044590,(void*)1)<0){print("Cannot grab input; returning without running game.\n");restore();close_file(fd);SC(91,hardware,map_size,0);close_file(fb);return 80;}
 SC(39,"/var/a918-games",0700,0);app_init(millis());int result=0;
 if(!load_keys()&&!calibrate(fd)){print("Input calibration cancelled or failed.\n");result=81;goto cleanup;}
 load_game();if(extension)app_enter_extension();else{app_key(K_OPTION);app_key(K_CENTER);}app_render();flip();
 uint32_t last=millis(),last_input=last,last_save=last,next_repeat=0;int held=0;
 while(!interrupted){
  uint32_t now=millis(),elapsed=now-last;last=now;if(elapsed>100)elapsed=100;
  struct input_event32 event;while(read_event(fd,&event))if(event.type==1){
   for(int key=1;key<=7;key++)if(codes[key]==event.code){
    if(event.value==pressed_value){down_mask|=1u<<key;last_input=now;int previous_page=app_page();app_key(key);if(key==K_BACK||(key==K_CENTER&&app_paused()))save_game();if(previous_page!=app_page())held=0;else if(app_page()==PAGE_BLOCKS&&(key==K_LEFT||key==K_RIGHT||key==K_DOWN)){held=key;next_repeat=now+180;}}
    else if(event.value==1-pressed_value){down_mask&=~(1u<<key);if(held==key)held=0;}
   }
  }
  if(held&&(int32_t)(now-next_repeat)>=0){app_key(held);next_repeat=now+65;}
  if(extension?app_return_requested():app_page()==PAGE_HOME)break;
  app_tick(elapsed);app_render();flip();
  if(now-last_save>30000){save_game();last_save=now;}
  if(now-last_input>120000){print("Idle timeout: saving and returning.\n");break;}
  delay(30);
 }
 save_game();
cleanup:
 drain_releases(fd);ioctl_file(fd,0x40044590,0);close_file(fd);restore();SC(91,hardware,map_size,0);close_file(fb);return result;
}
int raise(int signal){(void)signal;return -1;}
__attribute__((noreturn))void arm_start(unsigned long*stack){int result=arm_main((int)stack[0],(char**)&stack[1]);SC(1,result,0,0);for(;;){}}
__attribute__((naked,noreturn))void _start(void){__asm__ volatile("mov r0,sp\nb arm_start");}
