#include <vector>
#include <math.h>
#include "place.h"
#include "main.h"

rows_t place(std::vector<cell_t>& cells)
{
    int grid_w = ceil(sqrt(cells.size()));
    int grid_h = ceil(sqrt(cells.size()));

    rows_t rows(grid_h);


    // just arrange the cells into a square for now
    for (unsigned int i=0; i<cells.size(); i++) {
        rows[i/grid_w].push_back(&cells[i]);
    }

    // recalculate the current row and col of each cell
    update_cell_rows(rows);

    add_feed_throughs(rows);

    return rows;
}

void update_cell_rows(rows_t& rows)
{
    int current_row = 0;
    int current_col = 0;

    for (auto &row : rows) {
        for (auto &cell : row) {
            cell->row = current_row;
            cell->col = current_col;
            current_col++;
        }
        current_row++;
    }
}

void add_feed_throughs(rows_t& rows)
{
    for (auto &row : rows) {

        unsigned int row_idx = 0;

        // since we are modifying the row vector in the loop we can't use an iterator
        while (row_idx < row.size()) {

            cell_t *src_cell = row[row_idx];

            for (auto &term : src_cell->terms) {

                term_t *src_term = &term;

                // dont process terminals that have already been checked
                if (src_term->in_correct_channel) continue;

                // get the cell and terminal of the destination
                cell_t *dst_cell = src_term->dest_cell;
                term_t *dst_term = src_term->dest_term;

                if (dst_cell == nullptr) continue;

                /*

                   Terminals are in the same row and both facing same direction there is no need for feed through

                   /----S--\    /--D----\        /-------\    /-------\
                   |       |    |       |        |       |    |       |
                   |       |    |       |   OR   |       |    |       |
                   |       |    |       |        |       |    |       |
                   \-------/    \-------/        \----S--/    \--D----/

               */

                if (src_cell->row == dst_cell->row && src_term->on_top() == dst_term->on_top()) {
                    src_term->in_correct_channel = true;
                    dst_term->in_correct_channel = true;
                    continue;
                }

                /*

                   Destination is in the row above and they are facing each other

                   /-------\
                   |       |
                   |       |
                   |       |
                   \----D--/

                   /----S--\
                   |       |
                   |       |
                   |       |
                   \-------/

               */

                if (src_cell->row + 1 == dst_cell->row && src_term->on_top() && !dst_term->on_top()) {
                    src_term->in_correct_channel = true;
                    dst_term->in_correct_channel = true;
                    continue;
                }

                /*

                   Source is on bottom and the destination is somewhere above it
                   We need a feeder cell to get to the source to the top of the row

                                                    /-------\
                                                    |       |
                                                    |       |
                                                    |       |
                                                    \----D--/
                                             OR
                   /----D--\    /-------\           /-------\
                   |       |    |       |           |       |
                   |       |    |       |           |       |
                   |       |    |       |           |       |
                   \-------/    \--S----/           \----S--/



               */

                if (src_cell->row == dst_cell->row && !src_term->on_top() && dst_term->on_top()) {

                    cell_t *feed = new cell_t(/*feed_through=*/true);
                    feed->row = src_cell->row;

                    // figure out if we should add the feeder to the left or right side of the cell
                    if (src_term->on_left()) {
                        row.insert(row.begin()+row_idx, feed);
                    } else {
                        row.insert(row.begin()+row_idx+1, feed);
                    }

                    // remap the source to go through bottom of the feed cell
                    src_term->dest_cell = feed;
                    src_term->dest_term = &feed->terms[1];

                    // remap the bottom of feed cell to connect to source
                    feed->terms[1].dest_cell = src_cell;
                    feed->terms[1].dest_term = src_term;

                    // remap the top of the feed cell to connect to original dest
                    feed->terms[0].dest_cell = dst_cell;
                    feed->terms[0].dest_term = dst_term;

                    // remap the original destination to connect to top of feed cell
                    dst_term->dest_cell = feed;
                    dst_term->dest_term = &feed->terms[0];

                    // the source and bottom of feeder terms are now in_correct_channel
                    feed->terms[1].in_correct_channel = true;
                    src_term->in_correct_channel = true;

                    feed->terms[0].label = src_term->label;
                    feed->terms[1].label = src_term->label;

                    continue;
                }


                /*
                   Source is on the top of this row and the destination is in some row above it
                   we need to add a feeder cell to get it up into at least the next row

                    /----D--\
                    |       |
                    |       |
                    |       |
                    \-------/

                    /----S--\
                    |       |
                    |       |
                    |       |
                    \-------/
                */
            }

            row_idx++;
        }
    }
}
