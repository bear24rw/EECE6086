#ifndef __MAIN_H__
#define __MAIN_H__

#include <vector>
#include "cell.h"
#include "term.h"

#define TRACK_WIDTH   1
#define TRACK_SPACING 1
#define CELL_SPACING  1
#define CELL_HEIGHT   6
#define CELL_WIDTH    6


// a vector of rows where each row is a vector of pointers to cells
typedef std::vector<std::vector<cell_t*>> rows_t;


typedef struct {
    std::vector<term_t*> terms;
    std::vector<std::vector<term_t*>> tracks;
} channel_t;

typedef std::vector<channel_t> channels_t;

int wirelength(cell_t cell_a, int term_a, cell_t cell_b, int term_b);
void calculate_x_values(rows_t& rows);
void calculate_y_values(rows_t& rows, channels_t& channels);

#endif
