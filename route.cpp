#include <vector>
#include "route.h"
#include "main.h"
void route(rows_t& rows)
{
    /*
       ============== CHANNEL 5
       [] [] [] [] [] ROW 4
       ============== CHANNEL 4
       [] [] [] [] [] ROW 3
       ============== CHANNEL 3
       [] [] [] [] [] ROW 2
       ============== CHANNEL 2
       [] [] [] [] [] ROW 1
       ============== CHANNEL 1
       [] [] [] [] [] ROW 0
       ============== CHANNEL 0
    */

    /*
       ...
       -------------- CHANNEL 2 TRACK 0
       -------------- CHANNEL 2 TRACK 1
       -------------- CHANNEL 2 TRACK 2
       [] [] [] [] [] ROW 1
       -------------- CHANNEL 1 TRACK 0
       -------------- CHANNEL 1 TRACK 1
       -------------- CHANNEL 1 TRACK 2
       [] [] [] [] [] ROW 0
       -------------- CHANNEL 0 TRACK 0
       -------------- CHANNEL 0 TRACK 1
       -------------- CHANNEL 0 TRACK 2
    */


    // a vector of channels where each channel is a vector of pointers to terminals
    std::vector<std::vector<term_t*>> channel_terms(rows.size()+1);

    // go through the terminals of each cell in each row and add a pointer to
    // that terminal to the correct channel
    printf("Assigning terminals to channels\n");
    for (unsigned int row_idx=0; row_idx<rows.size(); row_idx++) {
        for (auto &cell : rows[row_idx]) {
            int num_terms = cell->feed_through ? 2 : 4;
            for (int term=0; term<num_terms; term++) {
                if (term_on_top(cell, term)) {
                    channel_terms[row_idx + 1].push_back(&cell->term[term]);
                } else {
                    channel_terms[row_idx].push_back(&cell->term[term]);
                }
            }
        }
    }


    printf("Assigning tracks to terminals\n");
    for (auto &channel : channel_terms) {
        std::vector<std::vector<term_t*>> tracks;
        int track = 0;
        for (auto &term : channel) {
            if (term->track != -1) continue;
            term->track = track;
            term->dest_cell->term[term->dest_term].track = track;
            track++;
        }
    }

}

