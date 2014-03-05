#include <stdio.h>
#include <vector>
#include <climits>
#include <stdlib.h>
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
        for (auto &term : channel.terms) {
            if (term->dest_cell == nullptr) continue;
            if (term->track >= 0) continue;

            int x1_a = std::min(term->position().x, term->dest_term->position().x);
            int x1_b = std::max(term->position().x, term->dest_term->position().x);

            // if the two terminals are directly above each
            // other there is no need for a track. mark the
            // destination term as unrouted so we don't draw it twice
            if (x1_a == x1_b) {
                term->track = VERTICAL;
                term->dest_term->track = UNROUTED;
                continue;
            }

            int lowest_allowed_track = -1;
            int highest_allowed_track = INT_MAX;

            int track_num = -1;

            // figure out the highest and lowest track we are allowed to place this net.
            // if two terminals are on either side of the channel and they don't have
            // greater than TRACK_SPACING horizontal distance between them then we
            // can't allow them to cross. this image illustrates the problem we
            // are solving (net 4 should not be allowed to cross net 2):
            // http://i.imgur.com/zPGh1AP.png
            for (auto &track : channel.tracks) {
                track_num++;
                for (auto &existing_term : track) {

                    // if there is enough horizontal spacing between these terms skip it
                    if (abs(term->position().x - existing_term->position().x) > TRACK_SPACING) continue;

                    // if we are above the existing terminal than that terminal defines our new lowest
                    if (term->cell->row > existing_term->cell->row) {
                        if (track_num > lowest_allowed_track)
                            lowest_allowed_track = track_num;
                    } else {
                        if (track_num < highest_allowed_track)
                            highest_allowed_track = track_num;
                    }
                }
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

            // if both terms are on the bottom of the cell we actually want to find the
            // highest track in order to pull the net up closer to the bottom of the cell
            bool find_highest = !term->on_top() && !term->dest_term->on_top();
            int highest_track = -1;

            track_num = -1;
            bool fits = false;
            for (auto &track : channel.tracks) {

                track_num++;

                if (track_num < lowest_allowed_track) continue;
                if (track_num > highest_allowed_track) continue;

                // assume we are able to fit this net on this track unless we determine otherwise
                fits = true;

                // TODO: if both terms are on the bottom we want to iterate backwards over the tracks
                // check if this new net intersect with any of the existing nets
                for (auto &existing_term : track) {
                    int x2_a = std::min(existing_term->position().x, existing_term->dest_term->position().x);
                    int x2_b = std::max(existing_term->position().x, existing_term->dest_term->position().x);
                    if (x2_a >= x1_a-TRACK_SP && x2_a <= x1_b+TRACK_SPACING) { fits = false; }
                    if (x2_b >= x1_a-TRACK_SP && x2_b <= x1_b+TRACK_SPACING) { fits = false; }
                    if (x2_a >= x1_a-TRACK_SP && x2_b <= x1_b+TRACK_SPACING) { fits = false; }
                    if (x2_a <= x1_a-TRACK_SP && x2_b >= x1_b+TRACK_SPACING) { fits = false; }
                }

                // if we are looking for the highest track and this track fits then it is the new highest, but continue looking
                if (find_highest && fits) {
                    highest_track = track_num;
                    continue;
                }

                // if fits is still true it means we didn't hit an existing net, we can add it to this existing track and stop searching
                if (fits) {
                    term->track = track_num;
                    term->dest_term->track = track_num;
                    track.push_back(term);
                    track.push_back(term->dest_term);
                    break;
                }

            }

            // if we're looking for the highest track and we found a valid one add the term to it
            if (find_highest && highest_track != -1) {
                term->track = highest_track;
                term->dest_term->track = highest_track;
                channel.tracks[highest_track].push_back(term);
                channel.tracks[highest_track].push_back(term->dest_term);
                continue;
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


