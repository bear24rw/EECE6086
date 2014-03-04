#ifndef __MAIN_H__
#define __MAIN_H__

#include <vector>

#define TRACK_WIDTH   1
#define TRACK_SPACING 1
#define CELL_SPACING  1
#define CELL_HEIGHT   6
#define CELL_WIDTH    6

typedef struct cell_t cell_t;

typedef struct {
    // pointer to cell that this term belongs too
    cell_t *cell;

    cell_t *dest_cell;
    int dest_term;

    // absolute world position of terminal
    int x, y;

    // this gets set to true in add_feed_throughs to indicate the term is
    // in the proper channel (area between two rows) probably needs a
    // better name
    bool connected;

    // 3 possible values:
    //  - track # within the channel (0-n)
    //  - UNROUTED means this term doesn't need any routing
    //  - VERTICAL means doesn't need a track since the connection is purely vertical
    int track;
} term_t;

#define UNROUTED -1
#define VERTICAL -2

struct cell_t {
    int number;
    int x, y;
    bool flip_x;
    bool flip_y;
    bool feed_through;
    int row, col; // the row and col of the cell in the placement grid, before feed throughs are added
    term_t term[4];
};

typedef struct {
    int x, y;
} point_t;

// a vector of rows where each row is a vector of pointers to cells
typedef std::vector<std::vector<cell_t*>> rows_t;


typedef struct {
    std::vector<term_t*> terms;
    std::vector<std::vector<term_t*>> tracks;
} channel_t;

typedef std::vector<channel_t> channels_t;

point_t get_term_position(cell_t *cell, int term);
bool term_on_top(cell_t *cell, int term);
bool term_on_left(cell_t *cell, int term);
int wirelength(cell_t cell_a, int term_a, cell_t cell_b, int term_b);
void calculate_x_values(rows_t& rows);
void calculate_y_values(rows_t& rows, channels_t& channels);

#endif
