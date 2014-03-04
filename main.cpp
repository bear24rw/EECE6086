#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <string>
#include "main.h"
#include "place.h"
#include "route.h"
#include "magic.h"
#include "test.h"

int num_nets;
int num_cells;

point_t get_term_position(cell_t *cell, int term)
{
    //
    // Returns the absolute position of a specified terminal for a given cell
    //

    // coordinates of terminals are -1 from what is given in
    // the pdf since we want to work with 0 based indexing
    point_t offsets[]    = {{1,5}, {4,5}, {1,0}, {4,0}};
    point_t offsets_x[]  = {{1,0}, {4,0}, {1,5}, {4,5}};
    point_t offsets_y[]  = {{4,5}, {1,5}, {4,0}, {1,0}};
    point_t offsets_xy[] = {{4,0}, {1,0}, {4,5}, {1,5}};

    point_t position;
    position.x = cell->x + offsets[term].x;
    position.y = cell->y + offsets[term].y;

    if (cell->flip_x) {
        position.x = cell->x + offsets_x[term].x;
        position.y = cell->y + offsets_x[term].y;
    }

    if (cell->flip_y) {
        position.x = cell->x + offsets_y[term].x;
        position.y = cell->y + offsets_y[term].y;
    }

    if (cell->flip_x && cell->flip_y) {
        position.x = cell->x + offsets_xy[term].x;
        position.y = cell->y + offsets_xy[term].y;
    }

    return position;
}

bool term_on_top(cell_t *cell, int term)
{
    //
    // Returns true if the specified terminal is currently on the top edge of the cell
    //

    if (cell->feed_through) {
        if (term == 0)
            return !cell->flip_x;
        else
            return cell->flip_x;
    }

    if (term == 0 || term == 1)
        return !cell->flip_x;
    else
        return cell->flip_x;
}

bool term_on_left(cell_t *cell, int term)
{
    //
    // Returns true if the specified terminal is currently on the left side of the cell
    //

    if (cell->feed_through) {
        return true;
    }

    if (term == 0 || term == 2)
        return !cell->flip_y;
    else
        return cell->flip_y;
}

int wirelength(cell_t *cell_a, int term_a, cell_t *cell_b, int term_b)
{
    //
    // Returns the manhatten distance between two terminals of two given cells
    //

    point_t term_a_pos = get_term_position(cell_a, term_a);
    point_t term_b_pos = get_term_position(cell_b, term_b);

    int x_len = abs(term_a_pos.x - term_b_pos.x);
    int y_len = abs(term_a_pos.y - term_b_pos.y);

    return x_len + y_len;
}


int main(int argc, char *argv[])
{
    #ifdef TEST
    test();
    return 0;
    #endif

    //
    // Get number of cells and nets
    //

    if (argc < 2) {
        printf("Usage: ./main inputfile\n");
        return EINVAL;
    }

    std::string filename = argv[1];

    std::ifstream fp(filename);

    if(fp.fail()) {
        printf("Cannot open file.\n");
        return ENOENT;
    }

    fp >> num_cells;
    fp >> num_nets;

    printf("Found %d cells and %d nets\n", num_cells, num_nets);

    //
    // Generate all cells
    //

    std::vector<cell_t> cells(num_cells);

    for (int i=0; i<num_cells; i++) {
        cells[i].number = i;
        cells[i].flip_x = false;
        cells[i].flip_y = false;
        cells[i].feed_through = false;
        for (int t=0; t<4; t++) {
            cells[i].term[t].dest_cell = nullptr;
            cells[i].term[t].connected = false;
            cells[i].term[t].track = -1;
        }
    }

    //
    // Read all the nets
    //

    for (int net, cell_a, term_a, cell_b, term_b; fp >> net >> cell_a >> term_a >> cell_b >> term_b;) {
        // subtract 1 from all the numbers to 0 index them
        cell_a--; cell_b--;
        term_a--; term_b--;
        cells[cell_a].term[term_a].dest_cell = &cells[cell_b];
        cells[cell_a].term[term_a].dest_term = term_b;
        cells[cell_b].term[term_b].dest_cell = &cells[cell_a];
        cells[cell_b].term[term_b].dest_term = term_a;
    }

    printf("Placing cells\n");
    rows_t rows = place(cells);

    printf("Calculating cell X positions\n");
    calculate_x_values(rows);

    printf("Routing cells\n");
    route(rows);

    printf("Calculating cell Y positions\n");
    calculate_y_values(rows);

    printf("Writing magic file\n");
    write_magic(filename, rows);

    printf("Done.\n");
}

void calculate_x_values(rows_t& rows)
{
    for (auto &row : rows) {

        int current_x = 0;
        bool last_was_feed_through = false;

        for (auto &cell : row) {

            // if this cell is a feed through we want to put it right up
            // against the previous cell which usually has a 1 unit boarder on
            // the right. but, if the last cell we placed was a feed through we
            // don't need to account for any border if the first cell is a
            // feed_through we don't want it to go negative.
            if (cell->feed_through && !last_was_feed_through)
                current_x = std::max(current_x-1, 0);

            cell->x = current_x;

            // feed through cells are 3 wide with no border
            // normal cells are 6 wide with border of 1
            if (cell->feed_through)
                current_x += 3;
            else
                current_x += 7;

            last_was_feed_through = cell->feed_through;
        }
    }
}

void calculate_y_values(rows_t& rows /*, channels*/)
{
    int current_y = 0;

    for (auto &row : rows) {

        int channel_width = 1; // TODO: use real channel width

        for (auto &cell : row) {
            cell->y = current_y;
        }

        current_y += 7 + channel_width;
    }
}
