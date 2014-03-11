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

    int num_nets;
    int num_cells;

    fp >> num_cells;
    fp >> num_nets;

    printf("Found %d cells and %d nets\n", num_cells, num_nets);

    //
    // Generate all cells
    //

    std::vector<cell_t> cells(num_cells);

    for (int i=0; i<num_cells; i++) {
        cells[i].number = i;
        cells[i].vacant = false;
        cells[i].locked = false;
        cells[i].occupied = false;
        cells[i].placed = false;
        cells[i].force = 0;
    }

    //
    // Read all the nets
    //

    for (int net, cell_a, term_a, cell_b, term_b; fp >> net >> cell_a >> term_a >> cell_b >> term_b;) {
        // subtract 1 from all the numbers to 0 index them
        cell_a--; cell_b--;
        term_a--; term_b--;
        // create symetric connection for each net
        cells[cell_a].terms[term_a].dest_cell = &cells[cell_b];
        cells[cell_b].terms[term_b].dest_cell = &cells[cell_a];
        cells[cell_a].terms[term_a].dest_term = &cells[cell_b].terms[term_b];
        cells[cell_b].terms[term_b].dest_term = &cells[cell_a].terms[term_a];
        cells[cell_a].terms[term_a].label = net;
        cells[cell_b].terms[term_b].label = net;
    }

    printf("Placing cells\n");
    rows_t rows = place(cells);

    printf("Calculating cell X positions\n");
    calculate_x_values(rows);

    calculate_term_positions(rows);

    printf("Routing cells\n");
    channels_t channels = route(rows);

    printf("Calculating cell Y positions\n");
    calculate_y_values(rows, channels);

    calculate_term_positions(rows);

    calculate_track_positions(channels);

    printf("Writing magic file\n");
    write_magic(filename, rows, channels);

    printf("Writing svg file\n");
    write_svg(filename, rows, channels);

    printf("Done.\n");
}

void calculate_x_values(rows_t& rows)
{
    // update the absolute x position of each cell in each row

    for (auto &row : rows) {

        int current_x = 0;

        for (auto &cell : row) {

            cell->position.x = current_x;

            // feed through cells are 3 wide, normal cells are 6 wide
            // every cell has a seperation of 1
            if (cell->feed_through)
                current_x += 4;
            else
                current_x += 7;

        }
    }
}

void calculate_y_values(rows_t& rows, channels_t& channels)
{
    // update the absolute y position of each cell in each row based
    // on the height of the channel between the rows

    int current_y = 0;

    for (unsigned int i=0; i<channels.size()-1; i++) {
        current_y += channels[i].tracks.size() * (TRACK_WIDTH + TRACK_SPACING);
        for (auto &cell : rows[i]) {
            cell->position.y = current_y;
        }
        current_y += CELL_HEIGHT + CELL_SPACING;
    }
}

void calculate_term_positions(rows_t& rows)
{
    // calculates the absolute xy position of each term within each cell
    for (auto &row : rows) {
        for (auto &cell : row) {
            for (auto &term : cell->terms) {
                term.position = get_term_position(term);
            }
        }
    }
}

point_t get_term_position(term_t& term)
{
    //
    // Returns the absolute world position of this terminal
    //

    // coordinates of terminals are -1 from what is given in
    // the pdf since we want to work with 0 based indexing
    point_t offsets[]    = {{1,5}, {4,5}, {1,0}, {4,0}};
    point_t offsets_x[]  = {{1,0}, {4,0}, {1,5}, {4,5}};
    point_t offsets_y[]  = {{4,5}, {1,5}, {4,0}, {1,0}};
    point_t offsets_xy[] = {{4,0}, {1,0}, {4,5}, {1,5}};
    point_t offsets_ft[] = {{1,5}, {1,0}};

    point_t term_position = offsets[term.number];

    if (term.cell->feed_through)
        return term.cell->position + offsets_ft[term.number];

    if (term.cell->flip_x)                      { term_position = offsets_x[term.number];  }
    if (term.cell->flip_y)                      { term_position = offsets_y[term.number];  }
    if (term.cell->flip_x && term.cell->flip_y) { term_position = offsets_xy[term.number]; }

    return term.cell->position + term_position;
}

void calculate_track_positions(channels_t& channels)
{
    for (auto &channel : channels) {
        for (auto &term : channel.terms) {

            int y = term->position.y;

            if (term->on_top()) {
                y += 1 + CELL_SPACING + term->track_num * (TRACK_WIDTH + TRACK_SPACING);
            } else {
                y -= 1 + CELL_SPACING + (channel.tracks.size()-1 - term->track_num) * (TRACK_WIDTH + TRACK_SPACING);
            }

            term->track_y = y;
        }
    }
}
