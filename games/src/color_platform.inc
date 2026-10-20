/* Included in boot_arm.c; all calls use the Sony OABI bridge. */
static const char*color_test_path;
uint32_t color_now_ms(void){long tv[2];if(SC(78,tv,0,0)<0)return 0;return (uint32_t)tv[0]*1000+(uint32_t)tv[1]/1000;}
int color_storage_available(void){return p1_storage_ready;}
void *color_alloc(unsigned n){long p=syscall6(192,0,n,3,0x22,-1,0);return (unsigned long)p>=0xfffff001u?0:(void*)p;}
void color_free(void*p,unsigned n){SC(91,p,n,0);}
int color_read(int snapshot,void*p,unsigned n,unsigned off){
 const char*path=snapshot?"/tmp/a918-p1-state/a918-games/color-id.sav":"/usr/local/bin/a918-color-id.bin";
 if(color_test_path)path=snapshot?"work/game-color/test-save.sav":color_test_path;
 if(snapshot==2)path=color_test_path?"work/game-color/test-save.bak":"/tmp/a918-p1-state/a918-games/color-id.bak";
 if(snapshot==3)path=color_test_path?"work/game-color-menu/new-game.sav":"/usr/local/bin/a918-color-id-new.sav";
 int fd=SC(5,path,0,0);if(fd==-2)return 0;if(fd<0)return -1;
 if(off&&SC(19,fd,off,0)!=(long)off){SC(6,fd,0,0);return -1;}
 unsigned done=0;while(done<n){long r=SC(3,fd,(char*)p+done,n-done);if(r==-4)continue;if(r<0){SC(6,fd,0,0);return -1;}if(!r)break;done+=r;}
 SC(6,fd,0,0);return done;
}
int color_archive_snapshot(void){
 if(!color_storage_available())return 0;
 const char*save=color_test_path?"work/game-color/test-save.sav":"/tmp/a918-p1-state/a918-games/color-id.sav";
 const char*backup=color_test_path?"work/game-color/test-save.bak":"/tmp/a918-p1-state/a918-games/color-id.bak";
 long r=SC(38,save,backup,0);if(r==-2)return 1;if(r<0)return 0;
 int d=SC(5,color_test_path?"work/game-color":"/tmp/a918-p1-state/a918-games",65536,0);if(d<0)return 0;int ok=SC(118,d,0,0)>=0;SC(6,d,0,0);return ok;
}
static void color_save_heartbeat(int saving){if(heartbeat>=0){char b=saving?2:1;SC(4,heartbeat,&b,1);}}
static int color_write_all(int fd,const void*p,unsigned n){unsigned done=0;while(done<n){unsigned chunk=n-done;if(chunk>16384)chunk=16384;color_save_heartbeat(1);long r=SC(4,fd,(const char*)p+done,chunk);if(r==-4)continue;if(r<=0)return 0;done+=r;}return 1;}
int color_store(const void*meta,unsigned n,const void*rom,unsigned size){
 if(!color_storage_available())return 0;
 if(!color_test_path)SC(39,"/tmp/a918-p1-state/a918-games",0700,0);
 const char*temp="/tmp/a918-p1-state/a918-games/color-id.tmp";
 const char*save="/tmp/a918-p1-state/a918-games/color-id.sav";
 if(color_test_path){temp="work/game-color/test-save.tmp";save="work/game-color/test-save.sav";}
 int fd=SC(5,temp,1|64|512,0600);if(fd<0)return 0;
 color_save_heartbeat(1);int ok=color_write_all(fd,meta,n);
 const unsigned char*bits=(const unsigned char*)meta+32;
 for(unsigned i=0;ok&&i<size/8192;i++)if(bits[i/8]&(1u<<(i%8)))ok=color_write_all(fd,(const char*)rom+i*8192,8192);
 if(SC(118,fd,0,0)<0)ok=0;if(SC(6,fd,0,0)<0)ok=0;
 if(ok&&SC(38,temp,save,0)<0)ok=0;
 if(ok){int d=SC(5,color_test_path?"work/game-color":"/tmp/a918-p1-state/a918-games",65536,0);if(d<0)ok=0;else{if(SC(118,d,0,0)<0)ok=0;SC(6,d,0,0);}}
 if(!ok)SC(10,temp,0,0);color_save_heartbeat(0);return ok;
}
