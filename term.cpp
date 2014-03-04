#include "term.h"
#include "cell.h"

term_t::term_t() {
    dest_cell = nullptr;
    connected = false;
    track = UNROUTED;
}

bool term_t::on_top(void)
{
    // Returns true if the terminal is currently on the top edge of the cell

    if (cell->feed_through) {
        if (number == 0)
            return !cell->flip_x;
        else
            return cell->flip_x;
    }

    if (number == 0 || number == 1)
        return !cell->flip_x;
    else
        return cell->flip_x;
}

bool term_t::on_left(void)
{
    // Returns true if the terminal is currently on the left side of the cell

    if (cell->feed_through) {
        return true;
    }

    if (number == 0 || number == 2)
        return !cell->flip_y;
    else
        return cell->flip_y;
}

point_t term_t::position(void)
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

    point_t term_position = offsets[number];

    if (cell->flip_x)                 { term_position = offsets_x[number];  }
    if (cell->flip_y)                 { term_position = offsets_y[number];  }
    if (cell->flip_x && cell->flip_y) { term_position = offsets_xy[number]; }

    return cell->position + term_position;
}
