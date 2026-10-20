/* A918 TamaLIB host adapter. Silent; game time freezes outside this program. */
#include "p1.h"
#include "../vendor/tamalib/tamalib.h"
#include <stddef.h>
static u12_t program[8192];
static uint8_t matrix[16][32],icons[8],raw[12289],saved[2048];
static uint32_t virtual_us,target_us;
static int status,loaded;
extern unsigned cpu_save_bytes(unsigned char*,unsigned);
extern int cpu_load_bytes(const unsigned char*,unsigned);
static void *allocate(u32_t size){(void)size;return 0;}
static void release(void*ptr){(void)ptr;}
static void halt(void){status=3;}
static bool_t log_enabled(log_level_t level){return level==LOG_ERROR;}
static void log_message(log_level_t level,char*message,...){(void)message;if(level==LOG_ERROR)status=3;}
static void until(timestamp_t target){if((int32_t)(target-virtual_us)>0)virtual_us=target;}
static timestamp_t timestamp(void){return virtual_us;}
static void update(void){}
static void pixel(u8_t x,u8_t y,bool_t value){if(x<32&&y<16)matrix[y][x]=!!value;else status=3;}
static void icon(u8_t index,bool_t value){if(index<8)icons[index]=!!value;else status=3;}
static void freq(u32_t value){(void)value;}
static void sound(bool_t value){(void)value;}
static int handler(void){return 0;}
static hal_t hal={allocate,release,halt,log_enabled,log_message,until,timestamp,update,pixel,icon,freq,sound,handler};
static uint32_t crc32(const uint8_t*data,unsigned size){uint32_t c=~0u;for(unsigned i=0;i<size;i++){c^=data[i];for(int b=0;b<8;b++)c=(c>>1)^((0u-(c&1))&0xedb88320u);}return ~c;}
static void put(uint8_t*p,uint32_t value){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(value>>(8*i));}
static uint32_t get(const uint8_t*p){uint32_t v=0;for(unsigned i=0;i<4;i++)v|=(uint32_t)p[i]<<(8*i);return v;}
int p1_load_rom(const uint8_t*data,unsigned size){
 loaded=0;status=2;
 if(size!=12288||crc32(data,size)!=0x5c864cb1u)return 0;
 for(unsigned i=0;i<8192;i++)program[i]=0xfff;
 for(unsigned i=0;i<6144;i++){if(data[i*2]>15)return 0;program[i]=(u12_t)(data[i*2]*256+data[i*2+1]);}
 for(unsigned y=0;y<16;y++)for(unsigned x=0;x<32;x++)matrix[y][x]=0;
 for(unsigned i=0;i<8;i++)icons[i]=0;
 virtual_us=target_us=0;status=0;tamalib_register_hal(&hal);
 if(tamalib_init(program,0,1000000)){status=3;return 0;}
 tamalib_set_speed(1);tamalib_set_exec_mode(EXEC_MODE_RUN);loaded=1;return 1;
}
int p1_open(void){
 if(!p1_storage_available()){loaded=0;status=6;return 0;}
 int size=p1_read_file("rom",raw,sizeof(raw));if(size<=0){loaded=0;status=1;return 0;}
 if(!p1_load_rom(raw,(unsigned)size))return 0;
 size=p1_read_file("save",saved,sizeof(saved));
 if(size>0&&!p1_import(saved,(unsigned)size))status=5;
 else if(size<0){loaded=0;status=6;return 0;}
 return 1;
}
void p1_tick(uint32_t ms){
 if(!loaded||status==3)return;if(ms>100)ms=100;
 target_us+=ms*1000;unsigned budget=4096;
 while((int32_t)(target_us-virtual_us)>0&&budget--&&status!=3)tamalib_step();
 /* A bounded slice keeps exit and USB handling responsive even at OSC3. */
}
void p1_button(unsigned button,int pressed){if(loaded&&button<3)tamalib_set_button((button_t)button,pressed?BTN_STATE_PRESSED:BTN_STATE_RELEASED);}
unsigned p1_export(uint8_t*out,unsigned capacity){
 if(!loaded||status==3||capacity<28)return 0;
 unsigned n=cpu_save_bytes(out+24,capacity-28);if(!n)return 0;
 put(out,0x31503941);put(out+4,1);put(out+8,0x5c864cb1);put(out+12,virtual_us);put(out+16,target_us);put(out+20,n);
 put(out+24+n,crc32(out,24+n));return 28+n;
}
int p1_import(const uint8_t*data,unsigned size){
 if(!loaded||size<28||size>2048||get(data)!=0x31503941||get(data+4)!=1||get(data+8)!=0x5c864cb1||get(data+20)!=size-28||get(data+size-4)!=crc32(data,size-4))return 0;
 if((int32_t)(get(data+16)-get(data+12))>1000000)return 0;
 if(!cpu_load_bytes(data+24,size-28))return 0;
 virtual_us=get(data+12);target_us=get(data+16);status=0;tamalib_refresh_hw();return 1;
}
int p1_save(void){
 if(!loaded)return 1;
 for(unsigned i=0;i<3;i++)p1_button(i,0);
 unsigned n=p1_export(saved,sizeof(saved));if(!n||!p1_write_save(saved,n)){status=4;return 0;}return 1;
}
const uint8_t*p1_pixels(void){return &matrix[0][0];}
const uint8_t*p1_icons(void){return icons;}
int p1_status(void){return status;}
