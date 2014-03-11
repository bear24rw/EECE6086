#include <stdlib.h>
#include "term.h"
#include "cell.h"

term_t::term_t()
{
    dest_cell = nullptr;
    dest_term = nullptr;
    in_correct_channel = false;
    track_num = UNROUTED;
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

int term_t::distance(point_t& p)
{
    return abs(position.x-p.x)+abs(position.y-p.y);
}
