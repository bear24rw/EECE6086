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

                // we're about to handle this net so just mark it as done
                src_term->in_correct_channel = true;
                dst_term->in_correct_channel = true;

                // terminals are in the same row and both facing same direction there is no need for feed through
                if (src_cell->row == dst_cell->row && src_term->on_top() == dst_term->on_top())
                    continue;

                // same row but the source is on top and the dst is on the bottom
                if (src_cell->row == dst_cell-> row && src_term->on_top() && !dst_term->on_top()) {
                    cell_t *feed = new cell_t(/*feed_through=*/true);
                    // figure out if we should add the feeder to the left or right side of the cell
                    if (src_term->on_left()) {
                        row.insert(row.begin()+row_idx, feed);
                    } else {
                        row.insert(row.begin()+row_idx+1, feed);
                    }
                    // remap the source to go through top of the feed cell
                    src_term->dest_cell = feed;
                    src_term->dest_term = &feed->terms[0];
                    // remap the top of feed cell to connect to source
                    feed->terms[0].dest_cell = src_cell;
                    feed->terms[0].dest_term = src_term;
                    // remap the bottom of the feed cell to connect to original dest
                    feed->terms[1].dest_cell = dst_cell;
                    feed->terms[1].dest_term = dst_term;
                    // remap the original destination to connect to bottom of feed cell
                    dst_term->dest_cell = feed;
                    dst_term->dest_term = &feed->terms[1];
                    // both terminals of the feeder are now in_correct_channel
                    feed->terms[0].in_correct_channel = true;
                    feed->terms[1].in_correct_channel = true;

                    feed->terms[0].label = src_term->label;
                    feed->terms[1].label = src_term->label;

                    // since we just added a cell we need to skip
                    row_idx++;

                    continue;
                }

                // TODO: if src_term is on the top and the term it's connected to is not on the bottom of the next row up
                // then we need to insert a feeder on the next row
            }

            row_idx++;
        }
    }
}
