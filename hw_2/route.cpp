#include <stdio.h>
#include <vector>
#include <algorithm>
#include <set>
#include <climits>
#include <stdlib.h>
#include <string.h>
#include "route.h"
#include "main.h"

void assign_terms_to_channels(channels_t& channels, rows_t& rows);
void assign_terms_to_tracks(channel_t& channel);
void find_vertical_nets(channel_t& channel);
void expand(channel_t& channel);
bool fix_overlaps(channel_t& channel, rows_t& rows);
bool pull_up_down(channel_t& channel, bool up);
bool pull_in(channel_t& channel);
void remove_empty_tracks(channel_t& channel);
void reset_term_track_numbers(channel_t& channel);
std::vector<int> get_open_tracks(channel_t& channel, term_t* term);

channels_t route(rows_t& rows)
{
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

    assign_terms_to_channels(channels, rows);

    for (auto &channel : channels) {
        assign_terms_to_tracks(channel);
        expand(channel);
        remove_empty_tracks(channel);
    }

    bool not_fixed = true;
    while (not_fixed) {
        for (auto &channel : channels) {
            not_fixed = fix_overlaps(channel, rows);
            expand(channel);
            remove_empty_tracks(channel);
        }
    }

    for (auto &channel : channels) {
        find_vertical_nets(channel);

        while (pull_up_down(channel, false)) {}
        remove_empty_tracks(channel);

        pull_in(channel);
        remove_empty_tracks(channel);

        while (pull_up_down(channel, true)) {}
        remove_empty_tracks(channel);

        pull_in(channel);
        remove_empty_tracks(channel);
    }

    return channels;
}

void assign_terms_to_channels(channels_t& channels, rows_t& rows)
{
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

}

void assign_terms_to_tracks(channel_t& channel)
{
    // give each terminal in the channel its own track

    unsigned int track_num = 0;

    for (auto &term : channel.terms) {
        if (term->dest_cell == nullptr) continue;
        if (term->track_num >= 0) continue;

        term->track_num            = track_num;
        term->dest_term->track_num = track_num;

        track_t track;
        track.insert(term);
        track.insert(term->dest_term);
        channel.tracks.push_back(track);

        track_num++;
    }
}

void find_vertical_nets(channel_t& channel)
{
    for (auto &term : channel.terms) {

        if (term->dest_cell == nullptr) continue;

        if (term->track_num == VERTICAL || term->dest_term->track_num == VERTICAL)
            continue;

        int x1_a = std::min(term->position.x, term->dest_term->position.x);
        int x1_b = std::max(term->position.x, term->dest_term->position.x);

        // if the two terminals are directly above each other there is no
        // need for them to be on a track. remove them from the track they
        // were on and mark it as a vertical track. also mark destination
        // term as unrouted so we don't draw it twice
        if (x1_a == x1_b) {
            channel.tracks[term->track_num].erase(term);
            channel.tracks[term->track_num].erase(term->dest_term);
            term->track_num = VERTICAL;
            term->dest_term->track_num = UNROUTED;
        }
    }
}

void expand(channel_t& channel)
{
    // keep track of which nets we have already fixed so we don't touch them again
    std::vector<term_t*> moved;

    for (auto &term : channel.terms) {
        for (auto &existing_term : channel.terms) {

            // don't consider our ourself or our destination
            if (existing_term == term->dest_term) continue;
            if (existing_term == term) continue;
            if (term->track_num == VERTICAL) continue;
            if (term->track_num == UNROUTED) continue;
            if (existing_term->track_num == VERTICAL) continue;
            if (existing_term->track_num == UNROUTED) continue;

            if (std::count(moved.begin(), moved.end(), term) > 0) continue;

            // if there is enough horizontal spacing between these terms skip it
            if (abs(term->position.x - existing_term->position.x) > TRACK_SPACING)
                continue;

            if (term->on_top() && term->track_num < existing_term->track_num && existing_term->on_bot())
                continue;

            if (term->on_bot() && term->track_num > existing_term->track_num && existing_term->on_top())
                continue;

            // erase the terms from their original track
            channel.tracks[term->track_num].erase(term);
            channel.tracks[term->track_num].erase(term->dest_term);

            // make a new one for them
            track_t track;
            track.insert(term);
            track.insert(term->dest_term);

            // if the existing term is on top of its cell that means there is a
            // trace spanning up from the term to its track so place above it
            if (existing_term->on_top()) {
                channel.tracks.insert(channel.tracks.begin() + existing_term->track_num+1, track);
            } else {
                channel.tracks.insert(channel.tracks.begin() + existing_term->track_num+0, track);
            }

            reset_term_track_numbers(channel);

            moved.push_back(term);
            moved.push_back(existing_term);
        }

    }
}

bool fix_overlaps(channel_t& channel, rows_t& rows)
{
    bool moved = false;

    for (auto &term : channel.terms) {

        if (term->dest_cell == nullptr) continue;

        if (term->track_num == VERTICAL || term->dest_term->track_num == VERTICAL)
            continue;

        std::vector<int> open_tracks = get_open_tracks(channel, term);

        // if there are open tracks that means there isn't an overlap
        if (!open_tracks.empty()) continue;

        dprintf("[overlap] net %d has an overlap\n", term->label);

        // we need to find which cell is more to the right since
        // we fix the issue by moving it more to the right
        cell_t *find_cell;
        if (term->dest_term->position.x > term->position.x) {
            find_cell = term->dest_cell;
        } else {
            find_cell = term->cell;
        }

        // loop through the row that the cell is on and start moving
        // each cell to the right after we find the cell we are looking for
        // add 2 because the nets could be directly on top of each other
        // or side by side. its easier to just add 2 to handle both cases
        // then try to figure out if we need to add 1 or 2
        bool move = false;
        for (auto &cell : rows[find_cell->row]) {
            if (cell == find_cell) move = true;
            if (move) cell->position.x += 2;
            calculate_term_positions(cell);
        }

        moved = true;
    }

    return moved;
}


bool pull_up_down(channel_t& channel, bool up)
{
    bool moved = false;

    // attempt to pull each net down as low as possible
    for (auto &term : channel.terms) {

        if (term->dest_cell == nullptr) continue;

        if (term->track_num == VERTICAL || term->dest_term->track_num == VERTICAL)
            continue;

        std::vector<int> open_tracks = get_open_tracks(channel, term);

        // if there are no open tracks we have to just leave it
        if (open_tracks.empty())
            continue;

        int track_num = -1;

        if (up)
            track_num = open_tracks.front();
        else
            track_num = open_tracks.back();

        // if its the same track we are already on just leave it
        if (track_num == term->track_num)
            continue;

        // remove them from the original track
        channel.tracks[term->track_num].erase(term);
        channel.tracks[term->track_num].erase(term->dest_term);

        // add them to the new one
        term->track_num = track_num;
        term->dest_term->track_num = track_num;
        channel.tracks[track_num].insert(term);
        channel.tracks[track_num].insert(term->dest_term);

        moved = true;
    }

    return moved;
}

bool pull_in(channel_t& channel)
{
    bool moved = false;

    // attempt to pull each net down as low as possible
    for (auto &term : channel.terms) {

        if (term->dest_cell == nullptr) continue;

        if (term->track_num == VERTICAL || term->dest_term->track_num == VERTICAL)
            continue;

        // if both terminals are not on the same side we can't pull the net in
        if (term->on_bot() != term->dest_term->on_bot())
            continue;

        std::vector<int> open_tracks = get_open_tracks(channel, term);

        // if there are no open tracks we have to just leave it
        if (open_tracks.empty())
            continue;

        // if both terms are on the bottom of the cell we actually want to find the
        // highest track in order to pull the net up closer to the bottom of the cell
        bool find_highest = term->on_bot() && term->dest_term->on_bot();

        int track_num = -1;

        if (find_highest)
            track_num = open_tracks.back();
        else
            track_num = open_tracks.front();

        // if its the same track we are already on just leave it
        if (track_num == term->track_num)
            continue;

        // remove them from the original track
        channel.tracks[term->track_num].erase(term);
        channel.tracks[term->track_num].erase(term->dest_term);

        // add them to the new one
        term->track_num = track_num;
        term->dest_term->track_num = track_num;
        channel.tracks[track_num].insert(term);
        channel.tracks[track_num].insert(term->dest_term);

        moved = true;
    }

    return moved;
}

std::vector<int> get_open_tracks(channel_t& channel, term_t* term)
{
    // return a list of open tracks that this terminals net can fit

    int *open_tracks = new int[channel.tracks.size()];

    // assume all tracks are intially open
    memset(open_tracks, 1, channel.tracks.size() * sizeof(int));

    // if we are the only net on this track then we really want to try to
    // move it to another track that already has nets on it so remove its
    // current track from the list of possible tracks it can end up
    /*
    if (channel.tracks[term->track_num].size() == 2) {
        open_tracks[term->track_num] = 0;
    }
    */

    // if there is not enough horizontal space between this terminal
    // and an existing one the track becomes invalid. also, every track
    // between the terminal itself and the track its going to is
    // invalid.  this image illustrates the problem we are solving (net
    // 4 should not be allowed to cross net 2):
    // http://i.imgur.com/zPGh1AP.png
    int track_num = -1;
    for (auto &track : channel.tracks) {

        track_num++;

        // don't consider the track we are already on
        if (term->track_num == track_num) continue;

        for (auto &existing_term : track) {

            // don't consider our destination
            if (term->dest_term == existing_term) continue;

            // if there is enough horizontal spacing between these terms skip it
            if ((abs(term->position.x - existing_term->position.x) > TRACK_SPACING) &&
                (abs(term->dest_term->position.x - existing_term->position.x) > TRACK_SPACING))
                continue;

            // if the existing term is on top of its cell that means
            // there is a trace spanning up from the term to the
            // current track. all the tracks in this area are invalid.
            if (existing_term->on_top()) {
                for (int t=0; t<=track_num; t++) {
                    open_tracks[t] = 0;
                }
            }

            // if the existing term is on bottom of its cell that means
            // there is a trace spanning down from the term to the
            // current track. all the tracks in this area are invalid.
            if (existing_term->on_bot()) {
                for (unsigned int t=track_num; t<channel.tracks.size(); t++) {
                    open_tracks[t] = 0;
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

        // don't consider the track we are already on
        if (term->track_num == track_num) continue;

        for (auto &existing_term : track) {

            // don't consider our destination
            if (term->dest_term == existing_term) continue;

            bool fits = true;
            int x1_a = std::min(term->position.x, term->dest_term->position.x);
            int x1_b = std::max(term->position.x, term->dest_term->position.x);
            int x2_a = std::min(existing_term->position.x, existing_term->dest_term->position.x);
            int x2_b = std::max(existing_term->position.x, existing_term->dest_term->position.x);
            if (x2_a >= x1_a-TRACK_SPACING && x2_a <= x1_b+TRACK_SPACING) { fits = false; }
            if (x2_b >= x1_a-TRACK_SPACING && x2_b <= x1_b+TRACK_SPACING) { fits = false; }
            if (x2_a >= x1_a-TRACK_SPACING && x2_b <= x1_b+TRACK_SPACING) { fits = false; }
            if (x2_a <= x1_a-TRACK_SPACING && x2_b >= x1_b+TRACK_SPACING) { fits = false; }
            if (!fits){
                open_tracks[track_num] = 0;
            }
        }
    }

    std::vector<int> open;
    for (unsigned int t=0; t<channel.tracks.size(); t++) {
        if (open_tracks[t]) open.push_back(t);
    }

    delete [] open_tracks;

    return open;
}

void remove_empty_tracks(channel_t& channel)
{
    // if there are terms in the track we are free to remove it

    for (unsigned int i=0; i<channel.tracks.size(); i++) {
        if (channel.tracks[i].empty()) {
            channel.tracks.erase(channel.tracks.begin()+i);
            i--;
        }
    }

    reset_term_track_numbers(channel);

}

void reset_term_track_numbers(channel_t& channel)
{
    // if tracks get removed then all the terminals in tracks above it will
    // have the wrong track number. this function just goes through each track
    // and resets the track number of each terminal in each track

    int track_num = 0;

    for (auto &track : channel.tracks) {
        for (auto &term : track) {
            term->track_num = track_num;
        }
        track_num++;
    }
}

