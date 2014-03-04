#include "cell.h"
#include "term.h"

cell_t::cell_t() {
    flip_x = false;
    flip_y = false;
    feed_through = false;
    terms = new term_t[4];
    for (int t=0; t<4; t++) {
        terms[t].cell = this;
        terms[t].number = t;
    }
}
