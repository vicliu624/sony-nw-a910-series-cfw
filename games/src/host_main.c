#include <stdio.h>
#include <stdlib.h>
#include "app.h"
int main(int argc,char**argv){
 int result=app_selftest();if(result){fprintf(stderr,"FAIL at line %d\n",result);return 1;}
 puts("PASS: shapes, walls, pause, two-line clear, scoring, 5850 randomized locks,");
 puts("save round-trip, corrupt/truncated save rejection, 11-item pagination, FM unavailable,");
 puts("Instant Game: 100 fresh rounds, center ends round, return to Home; no app pause/save API.");
 if(argc>1){app_init(918);app_enter_direction_extension();app_key(K_CENTER);app_key(K_CENTER);app_render();FILE*f=fopen(argv[1],"wb");if(!f)return 2;fprintf(f,"P6\n240 320\n255\n");uint16_t*p=app_pixels();for(int i=0;i<240*320;i++){unsigned v=p[i];unsigned char rgb[3]={((v>>11)&31)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);}
 return 0;
}
