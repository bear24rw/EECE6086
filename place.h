#ifndef __PLACE_H__
#define __PLACE_H__

#include <vector>
#include "main.h"

rows_t place(std::vector<cell_t>& cells);
void update_cell_rows(rows_t& rows);
void add_feed_throughs(rows_t& rows);

#endif
