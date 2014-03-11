#include "cell.h"
#include "term.h"

// keep a count of how many feed through cells
// we've constructed so we can uniquely label them
int feed_count = 0;

// default constructor creates a non feed_through cell
cell_t::cell_t() : cell_t(false) { }

cell_t::cell_t(bool ft)
{
    force = 0;
    locked = false;
    flip_x = false;
    flip_y = false;
    feed_through = ft;

    // feed through cells have 2 terminals, normal cells have 4
    if (ft) {
        terms.resize(2);
        number = feed_count++;
    } else {
        terms.resize(4);
        number = 0;
    }

    // set each of our terminals to point back to us
    for (unsigned int t=0; t<terms.size(); t++) {
        terms[t].cell = this;
        terms[t].number = t;
    }
}
