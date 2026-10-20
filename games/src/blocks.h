#ifndef A918_BLOCKS_H
#define A918_BLOCKS_H
#include <stdint.h>
#define BW 10
#define BH 20
typedef struct {
    uint8_t board[BH][BW], bag[7];
    int32_t piece, next, rotation, x, y, bag_pos;
    uint32_t rng, score, lines, elapsed;
    int32_t paused, over;
} Blocks;
void blocks_new(Blocks *g,uint32_t seed);
int blocks_move(Blocks *g,int dx,int dy);
int blocks_rotate(Blocks *g);
void blocks_tick(Blocks *g,uint32_t ms);
void blocks_drop(Blocks *g);
int blocks_cell(int piece,int rotation,int x,int y);
int blocks_valid(const Blocks *g);
int blocks_save(const Blocks *g,uint8_t *out,int capacity);
int blocks_load(Blocks *g,const uint8_t *data,int length);
int blocks_selftest(void);
#endif
