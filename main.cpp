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
#include "svg.h"

int num_nets;
int num_cells;


int wirelength(cell_t *cell_a, int term_a, cell_t *cell_b, int term_b)
{
    //
    // Returns the manhatten distance between two terminals of two given cells
    //

    point_t term_a_pos = cell_a->terms[term_a].position();
    point_t term_b_pos = cell_b->terms[term_a].position();

    int x_len = abs(term_a_pos.x - term_b_pos.x);
    int y_len = abs(term_a_pos.y - term_b_pos.y);

    return x_len + y_len;
}


int main(int argc, char *argv[])
{

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
    }

    //
    // Read all the nets
    //

    for (int net, cell_a, term_a, cell_b, term_b; fp >> net >> cell_a >> term_a >> cell_b >> term_b;) {
        // subtract 1 from all the numbers to 0 index them
        cell_a--; cell_b--;
        term_a--; term_b--;
        cells[cell_a].terms[term_a].dest_cell = &cells[cell_b];
        cells[cell_a].terms[term_a].dest_term = &cells[cell_b].terms[term_b];
        cells[cell_b].terms[term_b].dest_cell = &cells[cell_a];
        cells[cell_b].terms[term_b].dest_term = &cells[cell_a].terms[term_a];
        cells[cell_a].terms[term_a].label = net;
        cells[cell_b].terms[term_b].label = net;
    }

    printf("Placing cells\n");
    rows_t rows = place(cells);

    printf("Calculating cell X positions\n");
    calculate_x_values(rows);

    printf("Routing cells\n");
    channels_t channels = route(rows);

    printf("Calculating cell Y positions\n");
    calculate_y_values(rows, channels);

    printf("Writing magic file\n");
    write_magic(filename, rows, channels);

    printf("Writing svg file\n");
    write_svg(filename, rows, channels);

    printf("Done.\n");
}

void calculate_x_values(rows_t& rows)
{
    for (auto &row : rows) {

        int current_x = 0;

        for (auto &cell : row) {

            cell->position.x = current_x;

            // feed through cells are 3 wide
            // normal cells are 6 wide
            // every cell as a seperation of 1
            if (cell->feed_through)
                current_x += 4;
            else
                current_x += 7;

        }
    }
}

void calculate_y_values(rows_t& rows, channels_t& channels)
{
    int current_y = 0;

    for (unsigned int i=0; i<channels.size()-1; i++) {
        current_y += channels[i].tracks.size() * (TRACK_WIDTH + TRACK_SPACING);
        for (auto &cell : rows[i]) {
            cell->position.y = current_y;
        }
        current_y += CELL_HEIGHT + CELL_SPACING;
    }
}
