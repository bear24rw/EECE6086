#ifndef __MAIN_H__
#define __MAIN_H__

#include <unordered_set>
#include <vector>
#include "cell.h"
#include "term.h"

//#define DEBUG

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

extern int num_nets;
extern int num_cells;

extern std::string filename;

extern clock_t start_time;
extern clock_t place_time;
extern clock_t route_time;
extern clock_t end_time;

void calculate_x_values(rows_t& rows);
void calculate_y_values(rows_t& rows, channels_t& channels);
void calculate_term_positions(rows_t& rows);
void calculate_term_positions(cell_t* cell);
void calculate_track_positions(channels_t& channels);
int total_wire_length(channels_t& channels);
point_t get_term_position(term_t& term);
point_t bounding_box(rows_t& rows, channels_t* channels = nullptr);

#ifdef DEBUG
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) do{}while(0)
#endif

#endif
