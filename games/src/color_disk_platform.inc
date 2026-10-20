/* Only ordinary game files on the mounted visible FAT partition. No raw I/O. */
#ifndef A918_GAME_DISK_ROOT
#define A918_GAME_DISK_ROOT "/tmp/a918-game-disk"
#endif
#ifndef GAME_DIR
#define GAME_DIR A918_GAME_DISK_ROOT "/GAMES/TAMAGOTCHI/ID"
#endif
static const char*color_test_path;
static int game_storage_ready=1;
static const char*disk_file(unsigned which){
 static const char*real[]={GAME_DIR "/rom.bin",GAME_DIR "/state.sav",GAME_DIR "/state.bak",GAME_DIR "/new.sav",GAME_DIR "/state.tmp",GAME_DIR};
 static const char*test[]={"work/game-external/disk/GAMES/TAMAGOTCHI/ID/rom.bin","work/game-external/disk/GAMES/TAMAGOTCHI/ID/state.sav","work/game-external/disk/GAMES/TAMAGOTCHI/ID/state.bak","work/game-external/disk/GAMES/TAMAGOTCHI/ID/new.sav","work/game-external/disk/GAMES/TAMAGOTCHI/ID/state.tmp","work/game-external/disk/GAMES/TAMAGOTCHI/ID"};
 return color_test_path?(which==0?color_test_path:test[which]):real[which];
}
uint32_t color_now_ms(void){
#ifdef A918_HOST
 return 0;
#else
 long tv[2];if(SC(78,tv,0,0)<0)return 0;return (uint32_t)tv[0]*1000+(uint32_t)tv[1]/1000;
#endif
}
int color_storage_available(void){return game_storage_ready;}
void *color_alloc(unsigned n){long p=syscall6(192,0,n,3,0x22,-1,0);return (unsigned long)p>=(unsigned long)-4095?0:(void*)p;}
void color_free(void*p,unsigned n){SC(91,p,n,0);}
int color_read(int source,void*p,unsigned n,unsigned off){
 if(!game_storage_ready)return -1;
 const char*path=disk_file(source);
 int fd=SC(5,path,0100000,0);if(fd==-2)return 0;if(fd<0)return -1;
 if(off&&SC(19,fd,off,0)!=(long)off){SC(6,fd,0,0);return -1;}
 unsigned done=0;while(done<n){long r=SC(3,fd,(char*)p+done,n-done);if(r==-4)continue;if(r<0){SC(6,fd,0,0);return -1;}if(!r)break;done+=(unsigned)r;}
 SC(6,fd,0,0);return done;
}
static void save_pulse(int saving){outer_pulse(saving?2:1);if(heartbeat>=0){char b=saving?2:1;SC(4,heartbeat,&b,1);}}
static int sync_game_dir(void){int fd=SC(5,disk_file(5),040000|0100000,0);if(fd<0)return 0;int ok=SC(118,fd,0,0)>=0;SC(6,fd,0,0);return ok;}
static int disk_space(unsigned needed){uint32_t st[16];if(SC(99,disk_file(5),st,0)<0||!st[1])return 0;return (uint64_t)st[1]*st[4]>=(uint64_t)needed+65536;}
static int write_all(int fd,const void*p,unsigned n){unsigned done=0;while(done<n){unsigned chunk=n-done;if(chunk>16384)chunk=16384;save_pulse(1);long r=SC(4,fd,(const char*)p+done,chunk);if(r==-4)continue;if(r<=0)return 0;done+=(unsigned)r;}return 1;}
int color_store(const void*meta,unsigned n,const void*rom,unsigned size){
 if(!game_storage_ready)return 0;
 const uint8_t*bits=(const uint8_t*)meta+32;unsigned bytes=n;for(unsigned i=0;i<size/8192;i++)if(bits[i/8]&(1u<<(i%8)))bytes+=8192;
 if(!disk_space(bytes))return 0;
 int fd=SC(5,disk_file(4),1|64|512|0100000,0600);if(fd<0)return 0;
 save_pulse(1);int ok=write_all(fd,meta,n);
 for(unsigned i=0;ok&&i<size/8192;i++)if(bits[i/8]&(1u<<(i%8)))ok=write_all(fd,(const uint8_t*)rom+i*8192,8192);
 if(SC(118,fd,0,0)<0)ok=0;if(SC(6,fd,0,0)<0)ok=0;
 if(ok&&SC(38,disk_file(4),disk_file(1),0)<0)ok=0;
 if(ok)ok=sync_game_dir();
 if(!ok)SC(10,disk_file(4),0,0);save_pulse(0);return ok;
}
int color_archive_snapshot(void){
 if(!game_storage_ready)return 0;save_pulse(1);
 long r=SC(38,disk_file(1),disk_file(2),0);int ok=r==-2?1:r<0?0:sync_game_dir();save_pulse(0);return ok;
}
