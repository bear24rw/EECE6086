#include <stdlib.h>
#include <string>
#include <queue>
#include <climits>
#include "place.h"
#include "main.h"
#include "svg.h"
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

    update_cell_positions(rows);

    write_placement_svg(std::string("placement_0_start"), cells);

    force_directed(cells, rows);

    // recalculate the current row and col of each cell
    update_cell_positions(rows);

    write_placement_svg(std::string("placement_1_force"), cells);

    try_flips(rows);

    write_placement_svg(std::string("placement_2_flip"), cells);

    add_feed_throughs(rows);

    return rows;
}

void force_directed(std::vector<cell_t>& cells, rows_t& rows)
{
    // sorted cells are the cells with connections that we are going to actually try to place
    // extra cells are the cells with no connections that we will fill in the empty spots with after
    std::priority_queue<cell_t*, std::vector<cell_t*>, force_compare_t> sorted_cells;
    std::priority_queue<cell_t*, std::vector<cell_t*>, force_compare_t> extra_cells;

    // calculate the force of each cell and figure out which queue to put it in
    for (auto &cell : cells) {
        for (auto &term : cell.terms) {
            // dont continue if this term is not connected
            if (term.dest_cell == nullptr) continue;
            cell.force += wirelen(cell, *term.dest_cell);
        }

        // if there is a force on this cell it is connected to something
        // if there is no force then it is unconnected so we remove it from
        // the placement grid for now
        if (cell.force > 0) {
            sorted_cells.push(&cell);
        } else {
            rows[cell.row][cell.col] = nullptr;
            extra_cells.push(&cell);
        }
    }

    int iteration_count = 0;
    int iteration_limit = 100;

    int abort_count = 0;
    int abort_limit = 10;

    point_t target_pos;
    point_t seed_pos;

    bool end_ripple = false;
    int target_point = VACANT;

    printf("[force] ====== ITERATION %d ======\n", iteration_count);

    // TODO: make iteration limit: if min_total_force >
    // next_iteration_total_force: min_total_force = next_total_iteration_force
    while (iteration_count < iteration_limit) {

        // if we run out of seed cells we need to stop
        if (sorted_cells.empty())
            break;

        cell_t *seed_cell = sorted_cells.top();
        sorted_cells.pop();

        printf("[force] new seed cell: %d (location %d %d)\n", seed_cell->number, seed_cell->col, seed_cell->row);

        seed_pos.x = seed_cell->col;
        seed_pos.y = seed_cell->row;

        // mark seed position as vacant
        rows[seed_pos.y][seed_pos.x] = nullptr;

        end_ripple = false;

        while (!end_ripple) {

            // compute the zero force location
            target_pos = calculate_target_point(seed_cell);

            // the rows grid is not a complete square so its possible
            // that we calculate a position that isn't valid
            if (target_pos.x >= (signed)rows[target_pos.y].size())
                target_pos.x = rows[target_pos.y].size()-1;

            #ifdef DEBUGGING
            printf("[force] zero force location for %d: %d %d\n", seed_cell->number, target_pos.x, target_pos.y);
            #endif

            // figure out the status of the target point
            if (target_pos == seed_pos && rows[target_pos.y][target_pos.x] == nullptr) {
                target_point = SAME_LOC;
            } else if (rows[target_pos.y][target_pos.x] == nullptr) {
                target_point = VACANT;
            } else if (rows[target_pos.y][target_pos.x]->locked) {
                target_point = LOCKED;
            } else if (rows[target_pos.y][target_pos.x] != nullptr) {
                target_point = OCCUPIED;
            } else {
                printf("[force] Could not figure out status of target point!\n");
                target_point = -1;
            }

            #ifdef DEBUGGING
            switch(target_point) {
                case VACANT   : printf("[force] target_point : VACANT\n")   ; break ;
                case SAME_LOC : printf("[force] target_point : SAME_LOC\n") ; break ;
                case LOCKED   : printf("[force] target_point : LOCKED by cell %d\n", rows[target_pos.y][target_pos.x]->number)   ; break ;
                case OCCUPIED : printf("[force] target_point : OCCUPIED by cell %d\n", rows[target_pos.y][target_pos.x]->number) ; break ;
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
                    // find the closest vacant or unlocked position to the target point

                    point_t best_pos(0,0);
                    int best_dist = INT_MAX;
                    bool was_vacant = false;

                    for (unsigned int y=0; y<rows.size(); y++) {
                        for (unsigned int x=0; x<rows[y].size(); x++) {

                            // check if this position is vacant or unlocked
                            if (rows[y][x] == nullptr || (rows[y][x] != nullptr && !rows[y][x]->locked)) {

                                int dist = abs(target_pos.y - y) + abs(target_pos.x - x);

                                if (dist < best_dist) {
                                    best_dist = dist;
                                    best_pos.x = x;
                                    best_pos.y = y;
                                    was_vacant = (rows[y][x] == nullptr);
                                }
                            }

                        }
                    }

                    if (was_vacant) {
                        printf("[force] moving %d to vacant spot %d %d instead\n", seed_cell->number, best_pos.x, best_pos.y);

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
                            printf("[force] ====== ITERATION %d ======\n", iteration_count);
                        }
                    } else {
                        printf("[force] moving %d to unlocked spot %d %d instead\n", seed_cell->number, best_pos.x, best_pos.y);

                        cell_t *new_seed_cell = rows[best_pos.y][best_pos.x];

                        rows[best_pos.y][best_pos.x] = seed_cell;
                        seed_cell->row = best_pos.y;
                        seed_cell->col = best_pos.x;

                        seed_cell->locked = true;

                        seed_cell = new_seed_cell;
                        seed_pos.x = seed_cell->col;
                        seed_pos.y = seed_cell->row;

                        end_ripple = false;
                        abort_count = 0;
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

    // go through the grid and put the extra cells in the empty spots
    for (unsigned int y=0; y<rows.size(); y++) {
        for (unsigned int x=0; x<rows[y].size(); x++) {
            if (rows[y][x] == nullptr) {
                rows[y][x] = extra_cells.top();
                extra_cells.pop();
            }
        }
    }

}

point_t calculate_target_point(cell_t* cell)
{
    // the target point is the average location of
    // all the cells this cell is connected to

    point_t average(0,0);
    int connections = 0;

    for (auto &term : cell->terms) {
        // dont continue if this term is no connected
        if (term.dest_cell == nullptr) continue;
        // dont consider ourselves
        if (term.dest_cell == term.cell) continue;
        average.x += term.dest_cell->col;
        average.y += term.dest_cell->row;
        connections++;
    }

    if (connections > 0) {
        average.x = round((float)average.x / (float)connections);
        average.y = round((float)average.y / (float)connections);
    } else {
        average.x = cell->col;
        average.y = cell->row;
    }

    return average;
}

int wirelen(cell_t& a, cell_t& b)
{
    int dx = abs(a.row - b.row);
    int dy = abs(a.col - b.col);

    return dx + dy;
}

void print_rows(rows_t& rows) {
    for (auto &row : rows) {
        printf(">");
        for (auto &cell : row) {
            if (cell == nullptr)
                printf("nil ");
            else
                printf("%3d ", cell->number);
        }
        printf("\n");
    }
}


void update_cell_positions(rows_t& rows)
{
    // after placement we need to update the row and column of each cell
    // since we need that information when adding feed through cells in
    // the next stage. also arrange the cells in a grid so that we
    // can calculate the terminal positons which we need to figure
    // out if we need to flip a cell or not.

    int current_row = 0;
    int current_col = 0;

    for (auto &row : rows) {
        current_col = 0;
        for (auto &cell : row) {
            cell->row = current_row;
            cell->col = current_col;
            cell->position.x = cell->col * 6;
            cell->position.y = cell->row * 6;
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
            bool old_x = cell->flip_x;
            bool old_y = cell->flip_y;
            bool new_x = false;
            bool new_y = false;

            bool flips[4][2] = { {false, false},
                                 {false, true},
                                 {true, false},
                                 {true, true} };

            for (int f=0; f<4; f++) {

                force = 0;
                cell->flip_x = flips[f][0];
                cell->flip_y = flips[f][1];

                calculate_term_positions(rows);

                for (auto &term : cell->terms) {
                    if (term.dest_term == nullptr) continue;
                    force += term.distance(term.dest_term->position);
                }

                if (force < min_force) {
                    min_force = force;
                    new_x = cell->flip_x;
                    new_y = cell->flip_y;
                }

            }

            cell->flip_x = new_x;
            cell->flip_y = new_y;

            if (old_x != new_y || old_y != new_y) {
                printf("[flip] flipped cell %d\n", cell->number);
            }

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
