/* A910 1.03-specific. Addresses are checked by the packaging script.
 * Uses Sony's libc fork wrapper (pthread atfork handling), then ONLY raw
 * syscalls in the child. No malloc, stdio, dynamic loading or shell in child.
 */
typedef unsigned int u32;
static long sc3(long n,long a,long b,long c){
 register long r0 __asm__("r0")=a,r1 __asm__("r1")=b,r2 __asm__("r2")=c,r7 __asm__("r7")=n;
 __asm__ volatile("svc 0":"+r"(r0):"r"(r1),"r"(r2),"r"(r7):"memory","cc");return r0;
}
/* Pure policy is also tested in the host harness. Sony virtual key 7 is
 * OPTION; 4 is its short activation state, as used by 0x16fd00/0x76b88.
 * Raw /dev/input codes are independent and remain calibrated on the device.
 */
int sony_extension_event(unsigned event_pending,unsigned key,unsigned state){
 return event_pending&&key==7&&state==4;
}
int sony_home_hook(unsigned char *page){
 void *root=*(void * volatile *)0x0056383c;
 if(!root)return 0;
 u32 *vt=*(u32**)root;
 u32 *input=((u32*(*)(void*))vt[8])(root);
 if(!input||!sony_extension_event(((unsigned char*)input)[32],input[6],input[7]))return 0;
 /* A missing companion never changes the stock page. */
 static const char program[]="/usr/local/bin/a918-games";
 if(sc3(33,(long)program,1,0)<0)return 0;
 int pid=((int(*)(void))0x0000baf0)();
 if(pid<0)return 0;
 if(!pid){
  const char *args[]={program,"--sony-menu",0};
  sc3(11,(long)program,(long)args,0);
  sc3(1,127,0,0);for(;;){}
 }
 /* wait4(pid,status,0,NULL); EINTR must not leave the child unowned. */
 int status;long rc;
 do{register long r0 __asm__("r0")=pid,r1 __asm__("r1")=(long)&status,r2 __asm__("r2")=0,r3 __asm__("r3")=0,r7 __asm__("r7")=114;
  __asm__ volatile("svc 0":"+r"(r0):"r"(r1),"r"(r2),"r"(r3),"r"(r7):"memory","cc");rc=r0;
 }while(rc==-4);
 /* 0x658 is a PageIn flag, NOT a key-pending flag. Leave it untouched. */
 (void)page;((unsigned char*)input)[32]=0;input[6]=0;input[7]=0;
 return 1;
}
