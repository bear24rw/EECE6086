#ifndef __CELL_H__
#define __CELL_H__

#include "point.h"
#include "term.h"

typedef struct term_t term_t;
typedef struct point_t point_t;

struct cell_t {
    int number;
    point_t position;
    bool flip_x;
    bool flip_y;
    bool feed_through;
    int row, col; // the row and col of the cell in the placement grid, before feed throughs are added
    term_t *terms;
    cell_t();
};

#endif
