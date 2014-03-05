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
    unsigned int row_idx = 0;

    while (row_idx < rows.size()) {

        row_t row = rows[row_idx];

        unsigned int cell_idx = 0;

        bool rescan_row = false;

        // since we are modifying the row vector in the loop we can't use an iterator
        while (cell_idx < row.size()) {

            cell_t *src_cell = row[cell_idx];

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

                cell_t *feed = new cell_t(true);

                if (src_term->on_top()) {

                    feed->row = src_cell->row + 1;

                    // figure out if we should add the feeder to the left or
                    // right side of the cell and add it to the row above

                    // TODO: maybe try to figure out the closest x position


                    auto position = rows[row_idx+1].begin() + cell_idx;

                    if (position > rows[row_idx+1].end())
                        position = rows[row_idx+1].end() - 1;
                    else
                        position = rows[row_idx+1].begin() + cell_idx;

                    if (src_term->on_left()) {
                        rows[row_idx+1].insert(position, feed);
                    } else {
                        rows[row_idx+1].insert(position+0, feed);
                    }

                } else {

                    feed->row = src_cell->row;

                    // figure out if we should add the feeder to the left or right side of the cell
                    if (src_term->on_left()) {
                        rows[row_idx].insert(rows[row_idx].begin()+cell_idx, feed);
                    } else {
                        rows[row_idx].insert(rows[row_idx].begin()+cell_idx+1, feed);
                    }

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

                // since we just added a cell to this row we need to go back and do it again
                rescan_row = true;
                break;
            }

            if (rescan_row) break;

            cell_idx++;
        }

        if (!rescan_row) row_idx++;
    }
}
