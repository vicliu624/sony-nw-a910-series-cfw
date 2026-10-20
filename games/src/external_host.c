/* Exercise the SAME disk-platform code on local fixture files, no mounts. */
#define _GNU_SOURCE
#define A918_GAME_DISK_ROOT "work/game-external/disk"
#include "color.h"
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int heartbeat=-1;
static int test_failure;
static unsigned file_writes;
static long neg(long r){return r<0?-errno:r;}
static int allowed(const char*p){return !strncmp(p,A918_GAME_DISK_ROOT "/GAMES/TAMAGOTCHI/ID/",sizeof(A918_GAME_DISK_ROOT "/GAMES/TAMAGOTCHI/ID/")-1);}
static long syscall6(long n,long a,long b,long c,long d,long e,long f){(void)d;(void)e;(void)f;
 if(n==192){if(test_failure==5)return -12;void*p=mmap(0,b,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);return p==MAP_FAILED?-errno:(long)p;}
 if(n==91)return neg(munmap((void*)a,b));
 if(n==5){const char*p=(const char*)a;int flags=(int)b;
  if(flags&(1|2|64|512)){if(!allowed(p))abort();file_writes++;if(test_failure==1)return -EROFS;}
  int host=flags&~(040000|0100000);if(flags&040000)host|=O_DIRECTORY;if(flags&0100000)host|=O_NOFOLLOW;return neg(open(p,host,c));}
 if(n==3)return neg(read(a,(void*)b,c));
 if(n==4){if(test_failure==2)return -ENOSPC;return neg(write(a,(const void*)b,c));}
 if(n==6)return neg(close(a));
 if(n==19)return neg(lseek(a,b,c));
 if(n==118){if(test_failure==3)return -EIO;return neg(fsync(a));}
 if(n==38){if(!allowed((char*)a)||!allowed((char*)b))abort();if(test_failure==4)return -EIO;return neg(rename((char*)a,(char*)b));}
 if(n==10){if(!allowed((char*)a))abort();return neg(unlink((char*)a));}
 if(n==99){struct statvfs s;if(statvfs((char*)a,&s))return -errno;uint32_t*p=(void*)b;memset(p,0,64);p[1]=s.f_frsize;p[4]=test_failure==6?0:(s.f_bavail>0xffffffffu?0xffffffffu:(uint32_t)s.f_bavail);return 0;}
 fprintf(stderr,"unexpected disk syscall %ld\n",n);abort();
}
#define SC(n,a,b,c) syscall6(n,(long)(a),(long)(b),(long)(c),0,0,0)
#include "color_disk_platform.inc"
void external_failure(int which){test_failure=which;}
void external_storage(int ready){game_storage_ready=ready;}
unsigned external_writes(void){return file_writes;}
