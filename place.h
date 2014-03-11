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
int wirelen(int x1, int y1, int x2, int y2);
void swap_xy(int col1, int row1, int col2, int row2);
void update_cell_rows(rows_t& rows);
void add_feed_throughs(rows_t& rows);

#endif
