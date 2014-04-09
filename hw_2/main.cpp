#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <string>
#include <ctime>
#include "main.h"
#include "place.h"
#include "route.h"
#include "magic.h"
#include "svg.h"
#include "report.h"

int num_nets;
int num_cells;

std::string filename;

clock_t start_time;
clock_t place_time;
clock_t route_time;
clock_t end_time;

int main(int argc, char *argv[])
{
    start_time = std::clock();

    //
    // Get number of cells and nets
    //

    if (argc < 2) {
        dprintf("Usage: ./main inputfile\n");
        return EINVAL;
    }

    filename = argv[1];

    std::ifstream fp(filename);

    if(fp.fail()) {
        dprintf("Cannot open file.\n");
        return ENOENT;
    }

    fp >> num_cells;
    fp >> num_nets;

    dprintf("Found %d cells and %d nets\n", num_cells, num_nets);

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
        // create symetric connection for each net
        cells[cell_a].terms[term_a].dest_cell = &cells[cell_b];
        cells[cell_b].terms[term_b].dest_cell = &cells[cell_a];
        cells[cell_a].terms[term_a].dest_term = &cells[cell_b].terms[term_b];
        cells[cell_b].terms[term_b].dest_term = &cells[cell_a].terms[term_a];
        cells[cell_a].terms[term_a].label = net;
        cells[cell_b].terms[term_b].label = net;
        cells[cell_a].num_connections++;
        cells[cell_b].num_connections++;
    }

    //
    // Place
    //

    rows_t rows = place(cells);

    calculate_x_values(rows);

    calculate_term_positions(rows);

    place_time = std::clock();

    //
    // Route
    //

    channels_t channels = route(rows);

    calculate_y_values(rows, channels);

    calculate_term_positions(rows);

    calculate_track_positions(channels);

    route_time = std::clock();

    //
    // Output files and reports
    //

    write_magic(rows, channels);

    write_svg(rows, channels);

    end_time = std::clock();

    write_report(rows, channels);
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
void calculate_term_positions(cell_t* cell)
{
    // calculates the absolute xy position of each term within each cell
    for (auto &term : cell->terms) {
        term.position = get_term_position(term);
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

int total_wire_length(channels_t& channels)
{
    int length = 0;

    for (auto &channel : channels) {
        for (auto &term : channel.terms) {
            if (term->dest_cell == nullptr) continue;

            if (term->track_num == VERTICAL) {
                length += abs(term->position.y - term->dest_term->position.y);
                continue;
            }

            if (term->on_top() == term->dest_term->on_top()) {
                length += abs(term->position.x - term->dest_term->position.x)+1;
                length += abs(term->position.y - term->track_y) * 2;
                continue;
            }

            length += abs(term->position.x - term->dest_term->position.x) + 1;
            length += abs(term->position.y - term->dest_term->position.y);
        }
    }

    length = length / 2;

    return length;
}

point_t bounding_box(rows_t& rows, channels_t* channels)
{
    point_t biggest(0,0);

    // Y dimension

    biggest.y = rows.size() * 6;

    if (channels != nullptr) {
        for (auto &channel : *channels) {
            biggest.y += channel.tracks.size() * 2 + 1;
        }
        // outer 2 channels don't have the +1
        biggest.y -= 2;
    }

    // X dimension

    for (auto &row : rows) {

        int current_x = 0;

        for (auto &cell : row) {
            if (cell == nullptr) {
                current_x += 6;
            } else {
                current_x = cell->position.x;
                current_x += cell->feed_through ? 3 : 6;
            }
        }

        if (current_x > biggest.x) biggest.x = current_x;
    }

    return biggest;
}
