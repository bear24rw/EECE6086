#include <stdio.h>
#include <vector>
#include <set>
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

            unsigned int track_num = -1;

            std::set<int> open_tracks;

            // assume all tracks are intially open
            for (track_num=0; track_num<channel.tracks.size(); track_num++) {
                open_tracks.insert(track_num);
            }


            // if there is not enough horizontal space between this terminal
            // and an existing one the track becomes invalid. also, every track
            // between the terminal itself and the track its going to is
            // invalid.  this image illustrates the problem we are solving (net
            // 4 should not be allowed to cross net 2):
            // http://i.imgur.com/zPGh1AP.png
            track_num = -1;
            for (auto &track : channel.tracks) {
                track_num++;
                for (auto &existing_term : track) {

                    // if there is enough horizontal spacing between these terms skip it
                    if (abs(term->position().x - existing_term->position().x) > TRACK_SPACING)
                        continue;

                    // if the existing term is on top of its cell that means
                    // there is a trace spanning up from the term to the
                    // current track. all the tracks in this area are invalid.
                    if (existing_term->on_top()) {
                        for (unsigned int t=0; t<=track_num; t++) {
                            open_tracks.erase(t);
                        }
                    }

                    // if the existing term is on bottom of its cell that means
                    // there is a trace spanning down from the term to the
                    // current track. all the tracks in this area are invalid.
                    if (!existing_term->on_top()) {
                        for (unsigned int t=track_num; t<=channel.tracks.size(); t++) {
                            open_tracks.erase(t);
                        }
                    }
                }
            }


            /*
                check to make sure that we dont directly overlap an existing net in each track
                there are 4 possible overlap conditions. if any of them occur the track is invalid


                        1A--------------1B
                                 2A-----------2B


                        1A--------------1B
                 2A-----------2B


                        1A--------------1B
                            2A----2B


                        1A--------------1B
                    2A---------------------------2B
            */

            track_num = -1;
            for (auto &track : channel.tracks) {
                track_num++;
                for (auto &existing_term : track) {
                    bool fits = true;
                    int x2_a = std::min(existing_term->position().x, existing_term->dest_term->position().x);
                    int x2_b = std::max(existing_term->position().x, existing_term->dest_term->position().x);
                    if (x2_a >= x1_a-TRACK_SPACING && x2_a <= x1_b+TRACK_SPACING) { fits = false; }
                    if (x2_b >= x1_a-TRACK_SPACING && x2_b <= x1_b+TRACK_SPACING) { fits = false; }
                    if (x2_a >= x1_a-TRACK_SPACING && x2_b <= x1_b+TRACK_SPACING) { fits = false; }
                    if (x2_a <= x1_a-TRACK_SPACING && x2_b >= x1_b+TRACK_SPACING) { fits = false; }
                    if (!fits){
                        open_tracks.erase(track_num);
                    }
                }
            }

            // if there are no open tracks left we have to make a new one
            if (open_tracks.empty()) {
                term->track = channel.tracks.size();
                term->dest_term->track = channel.tracks.size();
                std::vector<term_t*> track;
                track.push_back(term);
                track.push_back(term->dest_term);
                channel.tracks.push_back(track);
                continue;
            }

            // if both terms are on the bottom of the cell we actually want to find the
            // highest track in order to pull the net up closer to the bottom of the cell
            bool find_highest = !term->on_top() && !term->dest_term->on_top();

            if (find_highest)
                track_num = *open_tracks.rbegin();
            else
                track_num = *open_tracks.begin();

            term->track = track_num;
            term->dest_term->track = track_num;
            channel.tracks[track_num].push_back(term);
            channel.tracks[track_num].push_back(term->dest_term);

        }
    }

    return channels;
}


