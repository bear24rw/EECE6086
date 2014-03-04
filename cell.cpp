#include "cell.h"
#include "term.h"

cell_t::cell_t() {
    cell_t(false);
}

cell_t::cell_t(bool ft) {
    number = 0;
    flip_x = false;
    flip_y = false;
    feed_through = ft;
    ft ? terms.resize(2) : terms.resize(4);
    for (unsigned int t=0; t<terms.size(); t++) {
        terms[t].cell = this;
        terms[t].number = t;
    }
}
