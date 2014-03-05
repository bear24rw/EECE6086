#include "cell.h"
#include "term.h"

int feed_count = 0;

cell_t::cell_t() : cell_t(false) { }

cell_t::cell_t(bool ft) {
    flip_x = false;
    flip_y = false;
    feed_through = ft;
    if (ft) {
        terms.resize(2);
        number = feed_count++;
    } else {
        terms.resize(4);
        number = 0;
    }
    for (unsigned int t=0; t<terms.size(); t++) {
        terms[t].cell = this;
        terms[t].number = t;
    }
}
