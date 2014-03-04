#include "cell.h"
#include "term.h"

cell_t::cell_t() {
    number = 0;
    flip_x = false;
    flip_y = false;
    feed_through = false;
    terms.resize(4);
    for (int t=0; t<4; t++) {
        terms[t].cell = this;
        terms[t].number = t;
    }
}

cell_t::cell_t(bool ft) {
    number = 0;
    flip_x = false;
    flip_y = false;
    feed_through = true;
    terms.resize(2);
    for (int t=0; t<2; t++) {
        terms[t].cell = this;
        terms[t].number = t;
    }
}
