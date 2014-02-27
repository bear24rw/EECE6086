#ifndef __MAIN_H__
#define __MAIN_H__

#include <vector>

typedef struct {
    int cell_a, cell_b;
    int term_a, term_b;
} net_t;

typedef struct {
    int number;
    int x, y;
    bool flip_x;
    bool flip_y;
    bool feed_through;
} cell_t;

typedef struct {
    int x, y;
} point_t;

// a vector of rows where each row is a vector of pointers to cells
typedef std::vector<std::vector<cell_t*>> rows_t;

point_t get_term_position(cell_t cell, int term);
int wirelength(cell_t cell_a, int term_a, cell_t cell_b, int term_b);

#endif
