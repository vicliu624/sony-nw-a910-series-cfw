#include "p1.h"
#include "app.h"
#include <stdio.h>
#include <string.h>
static const char *rom_path;
static unsigned save_size;
static uint8_t saved[2048];
int p1_test_failure;
int p1_storage_available(void){return p1_test_failure!=3;}
int p1_read_file(const char*name,void*buffer,unsigned capacity){
 if(strcmp(name,"rom")){if(save_size>capacity)return -1;memcpy(buffer,saved,save_size);return (int)save_size;}
 FILE*f=fopen(rom_path,"rb");if(!f)return 0;unsigned n=(unsigned)fread(buffer,1,capacity,f);int extra=fgetc(f);fclose(f);return extra==EOF?(int)n:-1;
}
int p1_write_save(const void*buffer,unsigned size){if(p1_test_failure||size>sizeof(saved))return 0;memcpy(saved,buffer,size);save_size=size;return 1;}
extern int boot_selftest(void),p1_selftest(void);
int main(int argc,char**argv){
 if(argc<2)return 64;rom_path=argv[1];int rc=boot_selftest();if(!rc)rc=p1_selftest();
 if(rc){fprintf(stderr,"P1 test failed at line %d\n",rc);return 1;}
 puts("P1 real ROM: boot, buttons, deterministic snapshot replay, corrupt/truncated rejection, failed-save retry, Games navigation PASS");
 if(argc>2){
  save_size=0;
  app_init(918);app_enter_boot();app_key(K_DOWN);app_key(K_CENTER);app_key(K_DOWN);app_key(K_CENTER);
  for(int i=0;i<50;i++)app_tick(100);
  if(argc>3){
   for(const char*s=argv[3];*s;s++){
    if(*s>='A'&&*s<='C'){unsigned button=(unsigned)(*s-'A');p1_button(button,1);p1_tick(100);p1_button(button,0);for(int i=0;i<10;i++)p1_tick(100);}
    else if(*s=='w'){for(int i=0;i<600;i++)p1_tick(100);}
   }
  }
  app_render();
  if(argc>3){unsigned h=2166136261u;for(unsigned i=0;i<512;i++)h=(h^p1_pixels()[i])*16777619u;printf("P1 lifecycle sequence=%s LCD_FNV=%08x status=%d\n",argv[3],h,p1_status());}
  FILE*f=fopen(argv[2],"wb");if(!f)return 2;fprintf(f,"P6\n240 320\n255\n");
  uint16_t*p=app_pixels();for(unsigned i=0;i<240*320;i++){unsigned v=p[i];uint8_t rgb[3]={((v>>11)&31)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);
 }
 return 0;
}
