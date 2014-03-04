#ifndef __TERM_H__
#define __TERM_H__

#include "point.h"
#include "cell.h"
#include "term.h"

typedef struct cell_t cell_t;
typedef struct point_t point_t;

struct term_t {
    // pointer to cell that this term belongs too
    cell_t *cell;

    int number;

    cell_t *dest_cell;
    term_t *dest_term;

    // this gets set to true in add_feed_throughs to indicate the term is
    // in the proper channel (area between two rows) probably needs a
    // better name
    bool connected;

    // 3 possible values:
    //  - track # within the channel (0-n)
    //  - UNROUTED means this term doesn't need any routing
    //  - VERTICAL means doesn't need a track since the connection is purely vertical
    #define UNROUTED -1
    #define VERTICAL -2
    int track;

    term_t();
    bool on_top(void);
    bool on_left(void);
    point_t position(void);

};

#endif
