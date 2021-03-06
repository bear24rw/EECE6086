#ifndef __PLACE_H__
#define __PLACE_H__

#include <vector>
#include <math.h>
#include <algorithm>
#include "main.h"

#define VACANT   1
#define SAME_LOC 2
#define LOCKED   3
#define OCCUPIED 4

rows_t place(std::vector<cell_t>& cells);
point_t calculate_grid_size(void);
void force_directed(std::vector<cell_t>& cells, rows_t& rows);
point_t calculate_target_point(cell_t* cell);
int calculate_force(cell_t* cell);
int wirelen(cell_t& a, cell_t& b);
void print_rows(rows_t& rows);
void update_cell_positions(rows_t& rows);
void try_flips(rows_t& rows);
void add_feed_throughs(rows_t& rows);
void move_feed_throughs(rows_t& rows);
void even_up_row_lengths(rows_t& rows);
bool pull_cells_together(rows_t& rows);

typedef struct {
    bool operator()(cell_t *a, cell_t *b)
    {
    return (a->force < b->force);
    }
} force_compare_t;

#endif
