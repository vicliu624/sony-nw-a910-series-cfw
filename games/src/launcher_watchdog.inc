/* Independently supervises the disk app, including its own supervisor. */
static int launcher_run_app(void){
 int fds[2];if(SC(42,fds,0,0)<0)return 83;
 SC(55,fds[0],4,2048);SC(55,fds[1],4,2048);install_signals();
 int child=SC(2,0,0,0);if(child<0){SC(6,fds[0],0,0);SC(6,fds[1],0,0);return 83;}
 if(!child){
  SC(6,fds[0],0,0);if(SC(57,0,0,0)<0){SC(1,126,0,0);for(;;){}}char env[48]="A918_APP_HEARTBEAT_FD=";int i=0;while(env[i])i++;char digits[12];int n=0;unsigned fd=fds[1];do{digits[n++]='0'+fd%10;fd/=10;}while(fd);while(n)env[i++]=digits[--n];env[i]=0;
  char*argv[]={"app","--games",0};char*environment[]={env,0};
  SC(11,APP_RAM "/program",argv,environment);SC(1,127,0,0);for(;;){}
 }
 SC(57,child,child,0);SC(6,fds[1],0,0);unsigned last=now_ms(),allowance=30000;int status=0,rc=84;
 for(;;){int r=SC(114,child,&status,1);if(r==child){rc=(status&127)?85:(status>>8)&255;break;}if(r<0&&r!=-4){rc=86;break;}
  char pulse[64];int n=SC(3,fds[0],pulse,sizeof(pulse));if(n>0){last=now_ms();for(int i=0;i<n;i++)allowance=pulse[i]==2?30000:5000;}
  if(stopped||(unsigned)(now_ms()-last)>allowance){SC(37,-child,9,0);SC(37,child,9,0);while(SC(114,child,&status,0)==-4){}rc=84;break;}
  sleep_ms();
 }
 SC(37,-child,9,0);SC(6,fds[0],0,0);return rc;
}
