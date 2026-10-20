#ifndef A918_P1_H
#define A918_P1_H
#include <stdint.h>
/* Platform returns bytes, zero for missing, negative for I/O failure. */
int p1_read_file(const char *name,void *buffer,unsigned capacity);
int p1_write_save(const void *buffer,unsigned size);
int p1_storage_available(void);
int p1_open(void);
int p1_load_rom(const uint8_t *data,unsigned size);
void p1_tick(uint32_t ms);
void p1_button(unsigned button,int pressed);
int p1_save(void);
unsigned p1_export(uint8_t *out,unsigned capacity);
int p1_import(const uint8_t *data,unsigned size);
const uint8_t *p1_pixels(void);
const uint8_t *p1_icons(void);
int p1_status(void); /* 0 running, 1 missing ROM, 2 invalid ROM, 3 core fault, 4 save error, 5 save rejected, 6 no persistent mount */
#endif
