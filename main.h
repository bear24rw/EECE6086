#ifndef __MAIN_H__
#define __MAIN_H__

#include <unordered_set>
#include <vector>
#include "cell.h"
#include "term.h"

#define TRACK_WIDTH   1
#define TRACK_SPACING 1
#define CELL_SPACING  1
#define CELL_HEIGHT   6
#define CELL_WIDTH    6


typedef std::vector<cell_t*> row_t;
typedef std::vector<row_t> rows_t;


typedef std::unordered_set<term_t*> track_t;

typedef struct {
    std::vector<term_t*> terms;
    std::vector<track_t> tracks;
} channel_t;

typedef std::vector<channel_t> channels_t;

void calculate_x_values(rows_t& rows);
void calculate_y_values(rows_t& rows, channels_t& channels);
void calculate_term_positions(rows_t& rows);
point_t get_term_position(term_t& term);

#endif
