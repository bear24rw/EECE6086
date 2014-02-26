#ifndef __MAIN_H__
#define __MAIN_H__

typedef enum {
    ROT_0   = 0,
    ROT_90  = 90,
    ROT_180 = 180,
    ROT_270 = 270
} rot_t;

typedef struct {
    int cell_a, cell_b;
    int term_a, term_b;
} net_t;

typedef struct {
    int x, y;
    rot_t rot;
    int flipped;
} cell_t;

typedef struct {
    int x, y;
} point_t;

point_t get_term_position(cell_t cell, int term);
int wirelength(cell_t cell_a, int term_a, cell_t cell_b, int term_b);

#endif
