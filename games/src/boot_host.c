#include <stdio.h>
#include "app.h"
extern int boot_selftest(void);
int main(int argc,char**argv){
 int rc=boot_selftest();if(rc){fprintf(stderr,"boot test failed line %d\n",rc);return 1;}
 puts("Boot selection, Walkman return, 200 fresh five-key rounds PASS");
 app_init(918);app_enter_boot();app_render();
 if(argc>1){FILE*f=fopen(argv[1],"wb");if(!f)return 2;fprintf(f,"P6\n240 320\n255\n");
  uint16_t*p=app_pixels();for(int i=0;i<240*320;i++){unsigned v=p[i];unsigned char b[3]={((v>>11)&31)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};fwrite(b,1,3,f);}fclose(f);}
 return 0;
}
