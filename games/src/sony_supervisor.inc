/* Included by the freestanding ARM platform. Syscall numbers are ARM EABI.
 * The helper is a DIRECT child of Gemini, never located by process-name scan.
 * Rescue owns the final framebuffer restoration and SIGCONT. Its lifetime
 * pipe closes even if the supervisor is killed; workers cannot retain it.
 */
static int wait_child(int pid,int *status,int options){return(int)call6(114,pid,(long)status,options,0,0,0);}
static int signal_pid(int pid,int sig){return(int)SC(37,pid,sig,0);}
static void child_exit(int code){SC(1,code,0,0);for(;;){}}
static int new_pipe(int fds[2]){return(int)SC(42,fds,0,0);}
static int direct_sony_parent(int pid){
 char path[48]="/proc/",digits[12],target[256];int n=0,v=pid;
 do{digits[n++]=(char)('0'+v%10);v/=10;}while(v);int at=6;
 while(n)path[at++]=digits[--n];const char *tail="/exe";while(*tail)path[at++]=*tail++;path[at]=0;
 n=(int)SC(85,path,target,255);if(n<=0)return 0;target[n]=0;
 const char *base=target;for(int i=0;i<n;i++)if(target[i]=='/')base=target+i+1;
 return equal(base,"gemini-ICX1101")||equal(base,"gemini");
}
static void rescue_parent(int sony,int life,int hb0,int hb1,int gate0,int gate1){
 close_file(hb0);close_file(hb1);close_file(gate0);close_file(gate1);
 int worker=0,complete=0,token=0,at=0;
 for(;;){int n=read_file(life,(char*)&token+at,4-at);if(n==-4)continue;if(n<=0)break;at+=n;
  if(at==4){at=0;if(token>0)worker=token;else if(token==0)complete=1;}}
 if(worker>0&&!complete){
  signal_pid(worker,9);
  for(int i=0;i<100&&signal_pid(worker,0)==0;i++)delay(10);
 }
 if(hardware)restore();signal_pid(sony,18);close_file(life);child_exit(0);
}
static void terminate_worker(int worker){
 signal_pid(worker,15);int status;
 for(int i=0;i<100;i++){int r=wait_child(worker,&status,1);if(r==worker||r==-10)return;delay(10);}
 signal_pid(worker,9);while(wait_child(worker,&status,0)==-4){}
}
/* test modes 1 normal, 2 worker killed, 3 worker stalled, 4 supervisor killed,
 * 5 graceful SIGTERM delivery AND signal-return ABI.
 * The selftest uses only its own parent/children and never opens hardware. */
static int sony_supervise(int test_mode){
 int sony=(int)SC(64,0,0,0);if(sony<=1||(!test_mode&&!direct_sony_parent(sony)))return 82;
 signals();int fb=-1,usb=-1,usb_initial=0;
 if(!test_mode){
  SC(39,"/var/a918-games",0700,0);int log=open_file("/var/a918-games/handoff.log",1|64|512,0600);
  if(log>=0){SC(63,log,1,0);SC(63,log,2,0);if(log>2)close_file(log);}
  print("A918 Game handoff begin.\n");
  /* Proven read-only request: Gemini AscUsbConn_getStatus 0x1d0f24,
   * icx1087_usbconn.ko usbconn_ioctl 0x6fc..0x788 copies status to user.
   * Requests 0x40046702/03 change USB mode and are NEVER used here. */
  usb=open_file("/dev/usbconn",0,0);
  if(usb<0||ioctl_file(usb,0x80046701,&usb_initial)<0||usb_initial<0||usb_initial>3){if(usb>=0)close_file(usb);print("Cannot monitor USB safely; returning.\n");return 88;}
  fb=open_file("/dev/fb/0",2,0);if(fb<0){close_file(usb);return 77;}
  if(!setup_framebuffer(fb,1)){close_file(usb);close_file(fb);return 78;}
 }
 int life[2]={-1,-1},hb[2]={-1,-1},gate[2]={-1,-1},result=83,status=0,rescue=-1,worker=-1;
 if(new_pipe(life)<0||new_pipe(hb)<0||new_pipe(gate)<0)goto close_all;
 SC(55,hb[0],4,2048);SC(55,hb[1],4,2048);
 rescue=(int)SC(2,0,0,0);
 if(!rescue){close_file(life[1]);rescue_parent(sony,life[0],hb[0],hb[1],gate[0],gate[1]);}
 if(rescue<0)goto close_all;
 close_file(life[0]);life[0]=-1;
 /* Rescue is already alive before the player can be stopped. */
 if(signal_pid(sony,19)<0)goto release;
 worker=(int)SC(2,0,0,0);
 if(!worker){
  close_file(life[1]);close_file(hb[0]);close_file(gate[1]);
  char go=0;if(read_file(gate[0],&go,1)!=1||go!=1)child_exit(83);close_file(gate[0]);
  if(test_mode){
   /* A shared, padded fake framebuffer survives child crashes exactly like
    * an mmap of the real display. Rescue must restore pixels, not padding. */
   if(hardware)for(int y=0;y<320;y++)for(int x=0;x<240;x++)hardware[y*stride+x]=0xbeef;
   if(test_mode!=6)close_file(hb[1]);if(test_mode==2){signal_pid((int)SC(20,0,0,0),9);child_exit(85);}
   if(test_mode==5){signal_pid((int)SC(20,0,0,0),15);delay(10);child_exit(interrupted?0:87);}
   if(test_mode==6){heartbeat_fd=hb[1];while(!interrupted)delay(10);child_exit(0);}
   if(test_mode==3||test_mode==4)for(;;)delay(100);
   delay(25);child_exit(0);
  }
  heartbeat_fd=hb[1];heartbeat_last=0;
  char *args[]={"/usr/local/bin/a918-games","--run-extension",0};
  child_exit(arm_main(2,args));
 }
 close_file(gate[0]);gate[0]=-1;close_file(hb[1]);hb[1]=-1;
 if(worker<0)goto release;
 if(write_file(life[1],&worker,4)!=4){terminate_worker(worker);goto release;}
 {char go=1;if(write_file(gate[1],&go,1)!=1){terminate_worker(worker);goto release;}}
 close_file(gate[1]);gate[1]=-1;
 if(test_mode==4)signal_pid((int)SC(20,0,0,0),9);
 {uint32_t last=millis(),usb_last=last,started=last;unsigned timeout=test_mode?150:5000;
  for(;;){
   int r=wait_child(worker,&status,1);
   if(r==worker){result=(status&127)?85:(status>>8)&255;break;}
   if(r<0&&r!=-4){result=85;break;}
   char bytes[64];if(read_file(hb[0],bytes,64)>0)last=millis();
   if((test_mode==6&&(uint32_t)(millis()-started)>50)||(!test_mode&&(uint32_t)(millis()-usb_last)>100)){
    int current=usb_initial;usb_last=millis();
    if(test_mode==6||ioctl_file(usb,0x80046701,&current)<0||current!=usb_initial){
     print("USB state changed: returning to Sony.\n");terminate_worker(worker);result=89;break;
    }
   }
   if(interrupted||(uint32_t)(millis()-last)>timeout){terminate_worker(worker);result=84;break;}
   if(wait_child(rescue,&status,1)==rescue){rescue=-1;terminate_worker(worker);result=86;break;}
   delay(10);
  }
 }
release:
 /* Only signal completion after the game is reaped and cannot draw again. */
 if(worker>0){int done=0;write_file(life[1],&done,4);}
 close_file(life[1]);life[1]=-1;
 if(rescue>0){int r;do{r=wait_child(rescue,&status,0);}while(r==-4);if(r!=rescue||(status&127)){if(hardware)restore();signal_pid(sony,18);}}
 else{if(hardware)restore();signal_pid(sony,18);}
close_all:
 for(int i=0;i<2;i++){if(life[i]>=0)close_file(life[i]);if(hb[i]>=0)close_file(hb[i]);if(gate[i]>=0)close_file(gate[i]);}
 if(fb>=0){SC(91,hardware,map_size,0);close_file(fb);hardware=0;}
 if(usb>=0)close_file(usb);
 if(!test_mode){print("A918 Game handoff returned, code ");decimal((unsigned)result);print("\n");}
 return result;
}
static int handoff_selftest(void){
 stride=248;map_size=stride*320*2;
 long memory=call6(192,0,map_size,3,0x21,-1,0);if((unsigned long)memory>=0xfffff001u)return 1;
 hardware=(uint16_t*)memory;
 for(int mode=1;mode<=6;mode++){
  for(int y=0;y<320;y++)for(unsigned x=0;x<stride;x++)hardware[y*stride+x]=x<240?0x1234:0xa55a;
  for(int i=0;i<240*320;i++)backup[i]=0x1234;
  int pid=(int)SC(2,0,0,0),status=0;if(pid<0)return 1;
  if(!pid)child_exit(sony_supervise(mode));
  int r;do{r=wait_child(pid,&status,0);}while(r==-4);
  int expected=(mode==1||mode==5)?0:mode==2?85:mode==6?89:84;
  if(r!=pid||(mode==4?(status&127)!=9:((status&127)||(status>>8)!=expected))){print("Handoff failure mode ");decimal(mode);print("\n");return 1;}
  for(int y=0;y<320;y++)for(unsigned x=0;x<stride;x++)if(hardware[y*stride+x]!=(x<240?0x1234:0xa55a)){print("Framebuffer restoration failed.\n");return 1;}
 }
 SC(91,hardware,map_size,0);hardware=0;
 print("Handoff PASS: normal exit, worker crash, heartbeat timeout, supervisor SIGKILL rescue, graceful SIGTERM, USB transition; parent resumed; shared framebuffer restored with row padding intact.\n");return 0;
}
