#ifndef __CELL_H__
#define __CELL_H__

#include <vector>
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
    bool vacant;
    bool locked;
    bool occupied;
    bool placed;
    int force;
    int total_conn;
    int x, y;
    int sum_x, sum_y;
    int row, col; // the row and col of the cell in the placement grid, before feed throughs are added
    std::vector<term_t> terms;
    cell_t();
    cell_t(bool);
};

#endif
