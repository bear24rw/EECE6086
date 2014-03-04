#include <stdio.h>
#include <vector>
#include "route.h"
#include "main.h"

channels_t route(rows_t& rows)
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


    // list of terminals in each channel
    channels_t channels(rows.size()+1);

    // go through the terminals of each cell in each row and add the terminals
    // to the correct channels
    for (unsigned int row_idx=0; row_idx<rows.size(); row_idx++) {
        for (auto &cell : rows[row_idx]) {
            for (auto &term : cell->terms) {
                if (term.on_top()) {
                    channels[row_idx + 1].terms.push_back(&term);
                } else {
                    channels[row_idx].terms.push_back(&term);
                }
            }
        }
    }


    // go through each channel and figure out what track to put each terminal in
    for (auto &channel : channels) {
        printf("channel --------------\n");
        for (auto &term : channel.terms) {
            if (term->dest_cell == nullptr) continue;
            if (term->track >= 0) continue;

            int x1_a = std::min(term->position().x, term->dest_term->position().x);
            int x1_b = std::max(term->position().x, term->dest_term->position().x);

            // if the two terminals are directly above each
            // other there is no need for a track
            if (x1_a == x1_b) {
                term->track = VERTICAL;
                term->dest_term->track = VERTICAL;
                continue;
            }

            // loop through each existing tracks and see if there is a spot for this net

            /*
                        1A--------------1B
                                 2A-----------2B

                        1A--------------1B
                 2A-----------2B

                        1A--------------1B
                            2A----2B

                        1A--------------1B
                    2A---------------------------2B
            */

            int track_num = 0;
            bool fits = false;
            for (auto &track : channel.tracks) {

                // assume we are able to fit this net on this track unless we determine otherwise
                fits = true;

                // TODO: if both terms are on the bottom we want to iterate backwards over the tracks
                // check if this new net intersect with any of the existing nets
                for (auto &existing_term : track) {
                    int x2_a = std::min(existing_term->position().x, existing_term->dest_term->position().x);
                    int x2_b = std::max(existing_term->position().x, existing_term->dest_term->position().x);
                    if (x2_a > x1_a && x2_a < x1_b) { fits = false; }
                    if (x2_b > x1_a && x2_b < x1_b) { fits = false; }
                    if (x2_a > x1_a && x2_b < x1_b) { fits = false; }
                    if (x2_a < x1_a && x2_b > x1_b) { fits = false; }
                }

                // if fits is still true it means we didn't hit an existing net, we can add it to this existing track and stop searching
                if (fits) {
                    term->track = track_num;
                    term->dest_term->track = track_num;
                    track.push_back(term);
                    track.push_back(term->dest_term);
                    break;
                }

                track_num++;
            }

            // if we weren't able to fit this term onto an existing track then create a new one for it
            if (!fits) {
                term->track = channel.tracks.size();
                term->dest_term->track = channel.tracks.size();

                std::vector<term_t*> track;
                track.push_back(term);
                track.push_back(term->dest_term);
                channel.tracks.push_back(track);
            }
        }
    }

    return channels;
}


