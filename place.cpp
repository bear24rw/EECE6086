#include "place.h"
#include "main.h"
#define DEBUGGING

rows_t place(std::vector<cell_t>& cells)
{
    int grid_w = ceil(sqrt(cells.size()));
    int grid_h = ceil(sqrt(cells.size()));

    rows_t rows(grid_h);

    // just arrange the cells into a square for now
    for (unsigned int i=0; i<cells.size(); i++) {
        rows[i/grid_w].push_back(&cells[i]);
        cells[i].row = i/grid_w;
        cells[i].col = i % grid_h;
    }

    #ifdef DEBUGGING
    printf("--------------------------------------------\n");
    printf("             INITIAL PLACEMENT              \n");
    printf("--------------------------------------------\n");
    printf(" c1 | c2  |  (x1, y1)  |  (x2, y2)  | force \n");
    printf("--------------------------------------------\n");
    #endif
    for (auto &cell_1 : cells) {
        for (auto &cell_2 : cells) {
            for (auto &term : cell_2.terms) {
                // dont continue if these cells are not connected
                if (term.dest_cell != &cell_1) continue;
                cell_1.total_conn += 1;
                cell_1.sum_x += cell_2.col;
                cell_1.sum_y += cell_2.row;
                cell_1.force += wirelen(cell_1, cell_2);
                #ifdef DEBUGGING
                printf("%3d | %3d | (%3d, %3d) | (%3d, %3d) | %3d\n", cell_1.number+1, cell_2.number+1,
                                                                      cell_1.col, cell_1.row,
                                                                      cell_2.col, cell_2.row,
                                                                      cell_1.force);
                #endif
            }
        }
    }

    // sort in descending order
    std::sort(cells.begin(), cells.end(), compare_force);

    #ifdef DEBUGGING
    printf("\n");
    #endif

    #ifdef DEBUGGING
    printf("--------------------------------------------\n");
    printf("               FORCE ON CELL                \n");
    printf("--------------------------------------------\n");
    printf(" C# | TC | F  | CNT                          \n");
    printf("--------------------------------------------\n");
    for (unsigned int i=0; i<cells.size(); i++) {
        printf(" %2d | %2d | %2d | %d\n", cells[i].number+1, cells[i].total_conn, cells[i].force, i);
    }
    #endif

    int iteration_count = 0;
    int iteration_limit = 3;

    int abort_count = 0;
    int abort_limit = 3;

    int idx = 0;

    int s_row  = 0;
    int s_cell = 0;

    bool end_ripple = false;
    int target_point = VACANT;

    int inc_x  = 0;
    int dest_x, dest_y = 0;
    int temp_x, temp_y = 0;

    int grid_x = cells.size();
    int grid_y = cells.size();

    // TODO: make iteration limit: if min_total_force >
    // next_iteration_total_force: min_total_force = next_total_iteration_force
    while (iteration_count < iteration_limit) {

        // all the cells are in descending order (maximum force first)...let's
        // make that the seed first s_row  = seed row s_cell = seed cell
        if (s_cell < cells.size())
            rows[s_row][s_cell] = &cells[s_cell];
        else
            break;

        // if we have grabbed a cell outside the width of the grid, set s_cell
        // to 0, and increment the row
        if (s_cell > grid_x && s_row < grid_y) {
            s_cell = 0;
            s_row++;
            continue;
        }

        // since in the occupied case we keep end_ripple = false, it's quite
        // possible that we may have already placed that cell, so we want to
        // make sure to not place it again so go ahead and grab another cell
        if (rows[s_row][s_cell]->placed) {
            s_cell++;
            continue;
        }

        // since we're going to move this cell anyways we're going to go ahead
        // and just mark the position of that cell as vacant
        rows[s_row][s_cell]->vacant = true;
        end_ripple = false;

        while (end_ripple == false) {
            printf("inner\n");

            // zero-force target = sum of distance among all of the cells
            // connected divided by how many connections that specific cell
            // has. check for 0 connections so we do not get a divide by zero
            // error
            if (rows[s_row][s_cell]->total_conn != 0) {
                dest_x = round(rows[s_row][s_cell]->sum_x / rows[s_row][s_cell]->total_conn);
                dest_y = round(rows[s_row][s_cell]->sum_y / rows[s_row][s_cell]->total_conn);
                printf("--------------------------------------------  \n");
                printf("cell %02d has zero-force location at: (%d, %d)\n", \
                        rows[s_row][s_cell]->number+1, dest_x, dest_y);
                printf("--------------------------------------------  \n");
            }
            // TODO: if cell has no connections i may just move it to the next
            // vacant area since it doesn't matter where it's located on the
            // grid
            else {
                printf("--------------------------------------------  \n");
                printf("cell %02d has no connections\n", rows[s_row][s_cell]->number+1);
                        //rows[s_row][s_cell]->dest_x, rows[s_row][s_cell]->dest_y);
                printf("--------------------------------------------  \n");
                s_cell++;
                break;
            }
            // is the zero-force location already occupied and locked for that
            // cell? if so set target point to LOCKED and handle that case,
            // otherwise the cell is occupied, but not locked so handle that
            // case. if we exit the for loop without setting the target
            // location, then we know that the target location is vacant and
            // that case is handled.
            for (unsigned int i = 0; i < cells.size(); i++) {
                if ((rows[s_row][s_cell] != &cells[i]               ) &&
                    dest_x == cells[i].col && dest_y == cells[i].row) {
                    if (!cells[i].placed && cells[i].locked && !rows[s_row][s_cell]->locked) {
                        target_point = LOCKED;
                        break;
                    }
                    else if (!cells[i].placed && !rows[s_row][s_cell]->locked) {
                        idx = i;
                        target_point = OCCUPIED;
                        break;
                    }
                    // TODO: remove this...
                    /*
                    else {
                        s_cell++;
                        break;
                    }
                    */
                }
                // check if zero-force location of cell is in the same place it
                // is already located in
                else if (rows[s_row][s_cell]  == &(cells[i]              ) &&
                         dest_x == cells[i].col && dest_y == cells[i].row) {

                         target_point = SAME_LOC;
                         break;
                }
            }

            if (target_point == LOCKED || target_point == OCCUPIED ||
                target_point == SAME_LOC) {}
            else
                target_point = VACANT;

            switch (target_point) {
                case VACANT:
                    rows[s_row][s_cell]->placed = true;

                    printf("in vacant\n");
                    printf("cell[%d] before being moved: (%d, %d)\n", \
                            rows[s_row][s_cell]->number+1, rows[s_row][s_cell]->col, rows[s_row][s_cell]->row);

                    rows[s_row][s_cell]->col = dest_x;
                    rows[s_row][s_cell]->row = dest_y;

                    printf("cell[%d] is being moved to: (%d, %d)\n", \
                            rows[s_row][s_cell]->number+1, rows[s_row][s_cell]->col, rows[s_row][s_cell]->row);

                    rows[s_row][s_cell]->locked = true;
                    s_cell++;
                    end_ripple = true;
                    abort_count = 0;
                    break;

                case SAME_LOC:
                    //rows[s_row][s_cell]->vacant = false;
                    //rows[s_row][s_cell]->placed = true;
                    //rows[s_row][s_cell]->locked = true;

                    printf("cell[%d] is placed in same location: (%d, %d)\n", \
                            rows[s_row][s_cell]->number+1, rows[s_row][s_cell]->col, rows[s_row][s_cell]->row);
                    s_cell++;
                    end_ripple = true;
                    abort_count = 0;
                    break;

                case LOCKED:
                    rows[s_row][s_cell]->placed = true;
                    temp_x = dest_x;
                    temp_y = dest_y;

                    printf("in locked\n");
                    printf("cell[%d] before being moved: (%d, %d)\n", \
                            rows[s_row][s_cell]->number+1, rows[s_row][s_cell]->col, rows[s_row][s_cell]->row);

                    inc_x = 1;
                    for (unsigned int i = 0; i < cells.size(); i++) {
                        // so if we increment the x value, we want to stay
                        // inside the grid so check for that, but if we never
                        // reach a vacant area for the whole time we have gone
                        // along the x, then let's try setting x back to 0 and
                        // incrememnt the y value, and try the same thing.

                        if (inc_x && temp_x++ > grid_w) {
                            inc_x = 0;
                            printf("inc x\n");
                            temp_x = dest_x;
                            break;
                        }
                        else if (!inc_x && temp_y++ > grid_h) {
                            inc_x = 1;
                            printf("inc y\n");
                            temp_y = dest_y;
                            break;
                        }

                        if (rows[s_row][s_cell] != &(cells[i]               ) &&
                            dest_x != cells[i].col && dest_y != cells[i].row) {
                            printf("find xy\n");
                            printf("x = %d | y = %d\n", dest_x, dest_y);

                            rows[s_row][s_cell]->col = temp_x;
                            rows[s_row][s_cell]->row = temp_y;

                            printf("x = %d | y = %d\n", dest_x, dest_y);
                            break;
                        }
                        else
                            continue;
                    }
                    //if (temp_x == 0 && temp_y == 0) {
                    //else if (rows[s_row][s_cell]->dest_x == 0 && rows[s_row][s_cell]->dest_y == grid_h) {
                    //else if (rows[s_row][s_cell]->dest_x == grid_w && rows[s_row][s_cell]->dest_y == 0) {
                    //else if (rows[s_row][s_cell]->dest_x == grid_w && rows[s_row][s_cell]->dest_y == grid_h) {
                    //if (rows[s_row][s_cell]->dest_x == 0) {

                    printf("cell[%d] is being moved to: (%d, %d)\n", \
                            rows[s_row][s_cell]->number+1, rows[s_row][s_cell]->col, rows[s_row][s_cell]->row);

                    s_cell++;
                    end_ripple = true;
                    abort_count += 1;
                    if (abort_count > abort_limit) {
                        for (unsigned int i=0; i<cells.size(); i++) {
                            //if (cells[i].locked) cells[i].locked = false;
                            cells[i].locked = false;
                        } iteration_count += 1;
                    }
                    //target_point = VACANT;
                    break;

                case OCCUPIED:

                    printf("in occ\n");
                    printf("cell[%d] before being moved: (%d, %d)\n", \
                            rows[s_row][s_cell]->number+1, rows[s_row][s_cell]->col, rows[s_row][s_cell]->row);

                    // go ahead and set the current location of the cell to the
                    // occupied cell and lock it
                    rows[s_row][s_cell]->col = dest_x;
                    rows[s_row][s_cell]->row = dest_y;
                    rows[s_row][s_cell]->locked = true;

                    printf("cell[%d] is being moved to: (%d, %d)\n", \
                            rows[s_row][s_cell]->number+1, rows[s_row][s_cell]->col, rows[s_row][s_cell]->row);
                    rows[s_row][s_cell]->placed = true;

                    // since we just placed the previous cell in this location,
                    // we now must find a new place for this cell
                    s_cell = idx;
                    rows[s_row][s_cell] = &cells[s_cell];
                    printf("cell[%d] was at: (%d, %d) and now needs to be moved\n",
                            rows[s_row][s_cell]->number+1, rows[s_row][s_cell]->col, rows[s_row][s_cell]->row);

                    //target_point = VACANT;
                    target_point = 0;
                    end_ripple = false;
                    abort_count = 0;
                    break;

                default:
                    printf("none of cases\n");
                    break;
            }

        }
    }

    // recalculate the current row and col of each cell
    update_cell_rows(rows);

    add_feed_throughs(rows);

    return rows;
}

int wirelen(cell_t& a, cell_t& b)
{
    int dx = abs(a.row - b.row);
    int dy = abs(a.col - b.col);

    return dx + dy;
}

bool compare_force(cell_t c1, cell_t c2)
{
    return (c1.force > c2.force);
}

void update_cell_rows(rows_t& rows)
{
    // after placement we need to update the row and column of each cell
    // since we need that information when adding feed through cells in
    // the next stage

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
    // go through the terminals of each row and figure out if we need to add a
    // feedthrough since we are modifying the row vectors we can't use iterators

    unsigned int row_idx = 0;

    while (row_idx < rows.size()) {

        row_t row = rows[row_idx];

        unsigned int cell_idx = 0;

        // when we add a feedthrough to the current row we need to rescan the
        // row because we might need to add another feedthrough for the
        // feedthrough we just added
        bool rescan_row = false;

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

                   /-------\        /-------\
                   |       |        |       |
                   |       |        |       |
                   |       |        |       |
                   \----D--/        \----S--/
                               OR
                   /----S--\        /----D--\
                   |       |        |       |
                   |       |        |       |
                   |       |        |       |
                   \-------/        \-------/

               */

                if ((src_cell->row + 1 == dst_cell->row && src_term->on_top() && !dst_term->on_top()) ||
                    (dst_cell->row + 1 == src_cell->row && dst_term->on_top() && !src_term->on_top())) {
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

                if ((!src_term->on_top() && dst_term->on_top() && dst_cell->row == src_cell->row) ||
                    (!src_term->on_top() && dst_cell->row > src_cell->row)) {

                    cell_t *feed = new cell_t(true);
                    feed->row = src_cell->row;

                    auto position = rows[row_idx].begin() + cell_idx;

                    if (!src_term->on_left())
                        position++;

                    if (position > rows[row_idx].end())
                        position = rows[row_idx].end();

                    rows[row_idx].insert(position, feed);

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

                    // the source and bottom of feeder terms are now
                    // in_correct_channel we can't say the destination is in
                    // correct channels since it might be more than 1 row above
                    // us
                    feed->terms[1].in_correct_channel = true;
                    src_term->in_correct_channel = true;

                    feed->terms[0].label = src_term->label;
                    feed->terms[1].label = src_term->label;

                    // since we just added a cell to this row we need to go back and do it again
                    rescan_row = true;
                    break;

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

                if (src_term->on_top() && dst_cell->row > src_cell->row) {

                    cell_t *feed = new cell_t(true);
                    feed->row = src_cell->row + 1;

                    auto position = rows[row_idx+1].begin() + cell_idx;

                    if (!src_term->on_left())
                        position++;

                    if (position > rows[row_idx+1].end())
                        position = rows[row_idx+1].end();

                    rows[row_idx+1].insert(position, feed);

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

            }

            if (rescan_row) break;

            cell_idx++;
        }

        if (!rescan_row) row_idx++;
    }
}
