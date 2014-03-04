#ifndef __MAIN_H__
#define __MAIN_H__

#include <vector>

typedef struct cell_t cell_t;

typedef struct {
    cell_t *dest_cell;
    int dest_term;

    // this gets set to true in add_feed_throughs to indicate the term is
    // in the proper channel (area between two rows) probably needs a
    // better name
    bool connected;

    // track within the channel
    int track;
} term_t;

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

point_t get_term_position(cell_t cell, int term);
bool term_on_top(cell_t *cell, int term);
bool term_on_left(cell_t *cell, int term);
int wirelength(cell_t cell_a, int term_a, cell_t cell_b, int term_b);
void calculate_x_values(rows_t& rows);
void calculate_y_values(rows_t& rows);

#endif
