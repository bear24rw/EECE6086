#include <queue>
#include <climits>
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
        cells[i].col = i % grid_w;
    }

    printf("----------------------------------\n");
    for (auto &row : rows) {
        for (auto &cell : row) {
            if (cell == nullptr)
                printf("null   ");
            else
                printf("%3d   ", cell->number);
        }
        printf("\n");
    }
    printf("----------------------------------\n");

    std::priority_queue<cell_t*, std::vector<cell_t*>, force_compare_t> sorted_cells;

    #ifdef DEBUGGING
    printf("--------------------------------------------\n");
    printf("             INITIAL PLACEMENT              \n");
    printf("--------------------------------------------\n");
    printf(" c1 | c2  |  (x1, y1)  |  (x2, y2)  | force \n");
    printf("--------------------------------------------\n");
    #endif
    for (auto &cell : cells) {
        for (auto &term : cell.terms) {
            // dont continue if this term is no connected
            if (term.dest_cell == nullptr) continue;
            cell.total_conn += 1;
            cell.sum_x += term.dest_cell->col;
            cell.sum_y += term.dest_cell->row;
            cell.force += wirelen(cell, *term.dest_cell);
            #ifdef DEBUGGING
            printf("%3d | %3d | (%3d, %3d) | (%3d, %3d) | %3d\n",
                    cell.number, term.dest_cell->number,
                    cell.col, cell.row,
                    term.dest_cell->col, term.dest_cell->row,
                    cell.force);
            #endif
        }
        sorted_cells.push(&cell);
    }

    // sort in descending order
    // TODO: figure out a different way to do this, sorting messes up all the pointers
    //std::sort(cells.begin(), cells.end(), compare_force);

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
        printf(" %2d | %2d | %2d | %d\n", cells[i].number, cells[i].total_conn, cells[i].force, i);
    }
    #endif

    int iteration_count = 0;
    int iteration_limit = 100;

    int abort_count = 0;
    int abort_limit = 10;

    point_t target_pos;
    point_t seed_pos;

    bool end_ripple = false;
    int target_point = VACANT;

    printf("[place] ====== ITERATION %d ======\n", iteration_count);

    // TODO: make iteration limit: if min_total_force >
    // next_iteration_total_force: min_total_force = next_total_iteration_force
    while (iteration_count < iteration_limit) {

        // if we run out of seed cells we need to stop
        if (sorted_cells.empty())
            break;

        cell_t *seed_cell = sorted_cells.top();
        sorted_cells.pop();

        seed_pos.x = seed_cell->col;
        seed_pos.y = seed_cell->row;

        // mark seed position as vacant
        rows[seed_pos.y][seed_pos.x] = nullptr;

        end_ripple = false;

        while (!end_ripple) {

            // compute the zero force location
            if (seed_cell->total_conn > 0) {
                target_pos.x = round(seed_cell->sum_x / seed_cell->total_conn);
                target_pos.y = round(seed_cell->sum_y / seed_cell->total_conn);
            } else {
                target_pos = seed_pos;
            }

            #ifdef DEBUGGING
            printf("[place] zero force location for %d: %d %d\n", seed_cell->number, target_pos.x, target_pos.y);
            #endif

            // figure out the status of the target point
            if (rows[target_pos.y][target_pos.x] == nullptr) {
                target_point = VACANT;
            } else if (rows[target_pos.y][target_pos.x]->locked) {
                target_point = LOCKED;
            } else if (target_pos == seed_pos) {
                target_point = SAME_LOC;
            } else if (rows[target_pos.y][target_pos.x] != nullptr) {
                target_point = OCCUPIED;
            } else {
                printf("[place] Could not figure out status of target point!\n");
                target_point = -1;
            }

            #ifdef DEBUGGING
            switch(target_point) {
                case VACANT   : printf("[place] target_point : VACANT\n")   ; break ;
                case SAME_LOC : printf("[place] target_point : SAME_LOC\n") ; break ;
                case LOCKED   : printf("[place] target_point : LOCKED\n")   ; break ;
                case OCCUPIED : printf("[place] target_point : OCCUPIED by cell %d\n", rows[target_pos.y][target_pos.x]->number) ; break ;
            }
            #endif

            switch (target_point) {
                case VACANT:
                {
                    rows[target_pos.y][target_pos.x] = seed_cell;
                    seed_cell->row = target_pos.y;
                    seed_cell->col = target_pos.x;

                    seed_cell->locked = true;
                    end_ripple = true;
                    abort_count = 0;

                    break;
                }

                case SAME_LOC:
                {
                    rows[seed_pos.y][seed_pos.x] = seed_cell;
                    end_ripple = true;
                    abort_count = 0;

                    break;
                }

                case LOCKED:
                {
                    // find the closest vacant position to the target point

                    point_t best_pos(0,0);
                    int best_dist = INT_MAX;

                    for (unsigned int y=0; y<rows.size(); y++) {
                        for (unsigned int x=0; x<rows[y].size(); x++) {

                            // if this position is not vacant keep looking
                            if (rows[y][x] != nullptr) continue;

                            int dist = abs(target_pos.y - y) + abs(target_pos.x - x);

                            if (dist < best_dist) {
                                best_dist = dist;
                                best_pos.x = x;
                                best_pos.y = y;
                            }

                        }
                    }

                    rows[best_pos.y][best_pos.x] = seed_cell;
                    seed_cell->row = best_pos.y;
                    seed_cell->col = best_pos.x;

                    end_ripple = true;

                    abort_count++;

                    if (abort_count > abort_limit) {

                        for (auto &cell : cells) {
                            cell.locked = false;
                        }

                        iteration_count++;
                        printf("[place] ====== ITERATION %d ======\n", iteration_count);
                    }

                    break;
                }

                case OCCUPIED:
                {
                    cell_t *new_seed_cell = rows[target_pos.y][target_pos.x];

                    rows[target_pos.y][target_pos.x] = seed_cell;
                    seed_cell->row = target_pos.y;
                    seed_cell->col = target_pos.x;

                    seed_cell->locked = true;

                    seed_cell = new_seed_cell;
                    seed_pos.x = seed_cell->col;
                    seed_pos.y = seed_cell->row;

                    end_ripple = false;
                    abort_count = 0;

                    break;
                }

                default:
                {
                    printf("none of cases\n");
                    break;
                }
            }

        }
    }

    // recalculate the current row and col of each cell
    printf("[place] updating cell rows\n");
    update_cell_rows(rows);

    printf("[place] trying flips\n");
    try_flips(rows);

    printf("[place] adding feed throughs\n");
    add_feed_throughs(rows);

    return rows;
}

int wirelen(cell_t& a, cell_t& b)
{
    int dx = abs(a.row - b.row);
    int dy = abs(a.col - b.col);

    return dx + dy;
}

void update_cell_rows(rows_t& rows)
{
    // after placement we need to update the row and column of each cell
    // since we need that information when adding feed through cells in
    // the next stage. also arrange the cells in a grid so that we
    // can calculate the terminal positons which we need to figure
    // out if we need to flip a cell or not.

    int current_row = 0;
    int current_col = 0;

    for (auto &row : rows) {
        for (auto &cell : row) {
            cell->row = current_row;
            cell->col = current_col;
            cell->x = cell->col * 6;
            cell->y = cell->row * 6;
            current_col++;
        }
        current_row++;
    }

    calculate_term_positions(rows);
}

void try_flips(rows_t& rows)
{
    for (auto &row : rows) {
        for (auto &cell : row) {

            int force = 0;
            int min_force = INT_MAX;
            bool do_x = false;
            bool do_y = false;

            bool flips[4][2] = { {false, false},
                                 {false, true},
                                 {true, false},
                                 {true, true} };

            printf("[flip] trying cell %d\n", cell->number);

            for (int f=0; f<4; f++) {

                force = 0;
                cell->flip_x = flips[f][0];
                cell->flip_y = flips[f][1];

                calculate_term_positions(rows);

                for (auto &term : cell->terms) {
                    if (term.dest_term == nullptr) continue;
                    force += term.distance(term.dest_term->position);
                }

                printf("[flip] force for flip %d %d = %d\n", cell->flip_x, cell->flip_y, force);

                if (force < min_force) {
                    min_force = force;
                    do_x = cell->flip_x;
                    do_y = cell->flip_y;
                }

            }

            cell->flip_x = do_x;
            cell->flip_y = do_y;

        }
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
