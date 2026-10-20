/* Included only in ARM boot target; wrapper owns a private mount of mtdblock/3.
 * ROM is bundled read-only alongside the executable, never loaded from USB.
 */
static int p1_same(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
int p1_test_failure;
static int p1_test_mode;
static const char *p1_test_rom_path;
static unsigned p1_test_saved_size;
static unsigned char p1_test_saved[2048];
static int p1_storage_ready=1;
int p1_storage_available(void){return p1_test_mode?p1_test_failure!=3:p1_storage_ready;}
int p1_read_file(const char*name,void*buffer,unsigned capacity){
 if(p1_test_mode&&!p1_same(name,"rom")){if(p1_test_saved_size>capacity)return -1;for(unsigned i=0;i<p1_test_saved_size;i++)((unsigned char*)buffer)[i]=p1_test_saved[i];return (int)p1_test_saved_size;}
 const char*path=p1_same(name,"rom")?"/usr/local/bin/a918-p1.bin":"/tmp/a918-p1-state/a918-games/p1.sav";
 if(p1_test_mode)path=p1_test_rom_path;
 int fd=(int)SC(5,path,0,0);if(fd==-2)return 0;if(fd<0)return -1;
 unsigned total=0;while(total<capacity){int n=(int)SC(3,fd,(char*)buffer+total,capacity-total);if(n==-4)continue;if(n<0){SC(6,fd,0,0);return -1;}if(!n)break;total+=(unsigned)n;}
 char tail;int extra=(int)SC(3,fd,&tail,1);SC(6,fd,0,0);return extra==0?(int)total:-1;
}
int p1_write_save(const void*buffer,unsigned size){
 if(p1_test_mode){if(p1_test_failure||size>sizeof(p1_test_saved))return 0;for(unsigned i=0;i<size;i++)p1_test_saved[i]=((const unsigned char*)buffer)[i];p1_test_saved_size=size;return 1;}
 if(!p1_storage_ready)return 0;
 if(SC(39,"/tmp/a918-p1-state/a918-games",0700,0)<0){/* EEXIST is fine; open verifies availability. */}
 int fd=(int)SC(5,"/tmp/a918-p1-state/a918-games/p1.tmp",1|64|512,0600);if(fd<0)return 0;
 unsigned total=0;int ok=1;while(total<size){int n=(int)SC(4,fd,(const char*)buffer+total,size-total);if(n==-4)continue;if(n<=0){ok=0;break;}total+=(unsigned)n;}
 if(SC(118,fd,0,0)<0)ok=0;SC(6,fd,0,0);
 if(ok&&SC(38,"/tmp/a918-p1-state/a918-games/p1.tmp","/tmp/a918-p1-state/a918-games/p1.sav",0)<0)ok=0;
 if(ok){int dir=(int)SC(5,"/tmp/a918-p1-state/a918-games",65536,0);if(dir<0)ok=0;else{if(SC(118,dir,0,0)<0)ok=0;SC(6,dir,0,0);}}
 if(!ok)SC(10,"/tmp/a918-p1-state/a918-games/p1.tmp",0,0);return ok;
}
