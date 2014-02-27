#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <fstream>
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

    std::ifstream fp(argv[1]);

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
    }

    //
    // Read all the nets
    //

    for (int net, cell_a, term_a, cell_b, term_b;
            fp >> net >> cell_a >> term_a >> cell_b >> term_b;) {
        // subtract 1 from the cell numbers to 0 index them
        cell_a--; cell_b--;
        cells[cell_a].term[term_a].dest_cell = &(cells[cell_b]);
        cells[cell_a].term[term_a].dest_term = term_b;
        printf("Cell A: %d Cell B: %d Set dest_cell %p to %p\n", cell_a, cell_b, cells[cell_a].term[term_a].dest_cell, &cells[cell_b]);
    }

    rows_t rows = place(cells);

    write_magic(rows);

    printf("Done.\n");
}
