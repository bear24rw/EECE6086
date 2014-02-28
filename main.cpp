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
#include "magic.h"
#include "test.h"

int num_nets;
int num_cells;

point_t get_term_position(cell_t cell, int term)
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
    position.x = cell.x + offsets[term].x;
    position.y = cell.y + offsets[term].y;

    if (cell.flip_x) {
        position.x = cell.x + offsets_x[term].x;
        position.y = cell.y + offsets_x[term].y;
    }

    if (cell.flip_y) {
        position.x = cell.x + offsets_y[term].x;
        position.y = cell.y + offsets_y[term].y;
    }

    if (cell.flip_x && cell.flip_y) {
        position.x = cell.x + offsets_xy[term].x;
        position.y = cell.y + offsets_xy[term].y;
    }

    return position;
}

int term_on_top(cell_t cell, int term)
{
    //
    // Returns true if the specified terminal is currently on the top edge of the cell
    //

    if (term == 0 || term == 1)
        return !cell.flip_y;
    else
        return cell.flip_y;
}

int wirelength(cell_t cell_a, int term_a, cell_t cell_b, int term_b)
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
        cells[i].term[0].dest_cell = nullptr;
        cells[i].term[1].dest_cell = nullptr;
        cells[i].term[2].dest_cell = nullptr;
        cells[i].term[3].dest_cell = nullptr;
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

    rows_t rows = place(cells);

    calculate_x_values(rows);
    calculate_y_values(rows);

    write_magic(filename, rows);

    printf("Done.\n");
}

void calculate_x_values(rows_t& rows)
{
    for (auto &row : rows) {

        int current_x = 0;

        for (auto &cell : row) {

            // if this cell is a feed through we want to put it right up
            // against the previous cell
            if (cell->feed_through)
                current_x = std::max(current_x-1, 0);

            cell->x = current_x;

            // feed through cells are 3 wide with no border
            // normal cells are 6 wide with border of 1
            if (cell->feed_through)
                current_x += 3;
            else
                current_x += 7;
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
