#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include "main.h"
#include "svg.h"


void draw_line(FILE *fp, float x1, float y1, float x2, float y2, std::string color=std::string("black"))
{
    x1 *= PX_PER_GRID; y1 *= PX_PER_GRID;
    x2 *= PX_PER_GRID; y2 *= PX_PER_GRID;
    y1 *= -1;
    y2 *= -1;
    fprintf(fp, "<line stroke='%s' stroke-width='1px' x1='%f' x2='%f' y1='%f' y2='%f' />\n", color.c_str(), x1, x2, y1, y2);
}

void draw_rect(FILE *fp, int x, int y, int w, int h, std::string fill=std::string(""), float fill_opacity=0, int stroke_width=5)
{
    x *= PX_PER_GRID; y *= PX_PER_GRID;
    w *= PX_PER_GRID; h *= PX_PER_GRID;
    y *= -1;
    y -= h;
    fprintf(fp, "<rect fill='%s' fill-opacity='%f' stroke='black' stroke-width='%dpx' x='%d' y='%d' width='%d' height='%d' />\n", fill.c_str(), fill_opacity, stroke_width, x, y, w, h);
}

void draw_text(FILE *fp, std::string text, float x, float y, int size=10, std::string color=std::string("black"), int rotate=0)
{
    x *= PX_PER_GRID;
    y *= PX_PER_GRID;
    y *= -1;
    fprintf(fp, "<text dominant-baseline='central' transform='rotate(%d %f,%f)' fill='%s' text-anchor='middle' x='%f' y='%f' font-size='%d'>%s</text>\n", rotate, x, y, color.c_str(), x, y, size, text.c_str());
}

void draw_via(FILE *fp, int x, int y)
{
    draw_line(fp, x, y, x+1, y+1);
    draw_line(fp, x+1, y, x, y+1);
}

void write_svg(std::string filename, rows_t& rows, channels_t& channels)
{

    //
    // Figure out the bounding box
    //

    point_t extents = bounding_box(rows, &channels);

    printf("SVG Extents: %d %d (%d)\n", extents.x, extents.y, extents.x*extents.y);

    //
    // Calculate viewport
    //

    int width = 2000;
    int height = 2000;

    int view_w = (extents.x + 2*GRID_BORDER) * PX_PER_GRID;
    int view_h = (extents.y + 2*GRID_BORDER) * PX_PER_GRID;
    int view_x = -GRID_BORDER * PX_PER_GRID;
    int view_y = -1*view_h + GRID_BORDER * PX_PER_GRID;

    filename.append(".svg");

    FILE *fp = fopen(filename.c_str(), "w");

    fprintf(fp, "<?xml version='1.0' encoding='utf-8' ?>\n");
    fprintf(fp, "<svg baseProfile='full' width='%dpx' height='%dpx' version='1.1' viewBox='%d %d %d %d' preserveAspectRatio='xMinYMin meet' xmlns='http://www.w3.org/2000/svg' xmlns:ev='http://www.w3.org/2001/xml-events' xmlns:xlink='http://www.w3.org/1999/xlink'>\n", width, height, view_x, view_y, view_w, view_h);
    fprintf(fp, "<defs />\n");

    //draw_rect(fp, 0, 0, biggest_x, biggest_y);

    //
    // Draw a grid
    //

    for (int i=-GRID_BORDER; i<extents.x+GRID_BORDER; i++) {
        draw_line(fp, i, -GRID_BORDER, i, extents.y+GRID_BORDER, std::string("gray"));
    }
    for (int i=-GRID_BORDER; i<extents.y+GRID_BORDER; i++) {
        draw_line(fp, -GRID_BORDER, i, extents.x+GRID_BORDER, i, std::string("gray"));
    }


    //
    // Draw all the cells
    //

    for (auto &row : rows) {
        for (auto &cell : row) {

            if (cell->feed_through) {
                draw_rect(fp, cell->position.x, cell->position.y, 3, 6, std::string("white"), 0.75);
                draw_text(fp, std::string("FT").append(std::to_string(cell->number+1)), cell->position.x + 1.5, cell->position.y + 3, 12, std::string("black"), 270);
            } else {
                draw_rect(fp, cell->position.x, cell->position.y, 6, 6, std::string("white"), 0.75);
                draw_text(fp, std::string("C").append(std::to_string(cell->number+1)), cell->position.x + 3, cell->position.y + 3, 12);
            }

            for (auto &term : cell->terms) {
                draw_text(fp, std::to_string(term.number+1), term.position.x + 0.5, term.position.y + 0.5, 6);
            }
        }
    }

    //
    // Draw all the routes
    //

    for (auto &channel : channels) {
        for (auto &src_term : channel.terms) {

            term_t *dst_term = src_term->dest_term;

            if (src_term->dest_cell == nullptr) continue;
            if (src_term->track_num == UNROUTED) continue;

            point_t p1, p2;
            int x1, y1, x2, y2;

            // vertical routes

            if (src_term->track_num == VERTICAL) {
                p1 = src_term->position;
                p2 = dst_term->position;
                x1 = std::min(p1.x, p2.x);
                y1 = std::min(p1.y, p2.y);
                x2 = std::max(p1.x, p2.x) + TRACK_WIDTH;
                y2 = std::max(p1.y, p2.y) + TRACK_WIDTH;
                draw_rect(fp, x1, y1, x2-x1, y2-y1, std::string("purple"), 0.5, 0);
                continue;
            }

            p1 = p2 = src_term->position;
            p2.y = src_term->track_y;
            x1 = std::min(p1.x, p2.x);
            y1 = std::min(p1.y, p2.y);
            x2 = std::max(p1.x, p2.x) + TRACK_WIDTH;
            y2 = std::max(p1.y, p2.y) + TRACK_WIDTH;
            draw_rect(fp, x1, y1, x2-x1, y2-y1, std::string("purple"), 0.5, 0);

            // horizontal routes

            p1 = src_term->position;
            p2 = dst_term->position;

            p1.y = src_term->track_y;

            x1 = std::min(p1.x, p2.x);
            x2 = std::max(p1.x, p2.x)+1;
            y1 = p1.y;
            y2 = p1.y + 1;
            draw_rect(fp, x1, y1, x2-x1, y2-y1, std::string("blue"), 0.5, 0);
            draw_via(fp, p1.x, p1.y);
            draw_via(fp, p2.x, p1.y);

        }
    }

    //
    // Draw all the net labels
    //

    for (auto &channel : channels) {
        for (auto &src_term : channel.terms) {

            term_t *dst_term = src_term->dest_term;

            if (src_term->dest_cell == nullptr) continue;
            if (src_term->track_num == UNROUTED) continue;

            point_t p1, p2;
            int x1, y1, x2, y2;
            float x, y;

            p1 = src_term->position;
            p2 = dst_term->position;

            if (src_term->track_num == VERTICAL) {
                y1 = std::min(p1.y, p2.y);
                y2 = std::max(p1.y, p2.y)+1;
                x = p1.x + 0.5;
                y = (y2-y1)/2.0+y1;
                draw_text(fp, std::to_string(src_term->label), x, y, 10, std::string("yellow"), 270);
                continue;
            }

            p1.y = src_term->track_y;

            x1 = std::min(p1.x, p2.x);
            x2 = std::max(p1.x, p2.x)+1;
            y1 = p1.y;
            x = (x2-x1)/2.0+x1;
            y = y1 + 0.5;
            draw_text(fp, std::to_string(src_term->label), x, y, 10, std::string("yellow"));

        }
    }

    fprintf(fp, "</svg>\n");
    fclose(fp);
}

void write_placement_svg(std::string filename, rows_t& rows)
{
    // calculate extents

    point_t extents = bounding_box(rows);

    // calculate viewport

    int width = 1000;
    int height = 1000;

    int view_w = (extents.x + 2*GRID_BORDER) * PX_PER_GRID;
    int view_h = (extents.y + 2*GRID_BORDER) * PX_PER_GRID;
    int view_x = -GRID_BORDER * PX_PER_GRID;
    int view_y = -1*view_h + GRID_BORDER * PX_PER_GRID;

    filename.append(".svg");

    FILE *fp = fopen(filename.c_str(), "w");

    fprintf(fp, "<?xml version='1.0' encoding='utf-8' ?>\n");
    fprintf(fp, "<svg baseProfile='full' width='%dpx' height='%dpx' version='1.1' viewBox='%d %d %d %d' preserveAspectRatio='xMinYMin meet' xmlns='http://www.w3.org/2000/svg' xmlns:ev='http://www.w3.org/2001/xml-events' xmlns:xlink='http://www.w3.org/1999/xlink'>\n", width, height, view_x, view_y, view_w, view_h);
    fprintf(fp, "<defs />\n");

    // draw a grid

    for (int i=-GRID_BORDER; i<extents.x+GRID_BORDER; i++) {
        draw_line(fp, i, -GRID_BORDER, i, extents.y+GRID_BORDER, std::string("gray"));
    }
    for (int i=-GRID_BORDER; i<extents.y+GRID_BORDER; i++) {
        draw_line(fp, -GRID_BORDER, i, extents.x+GRID_BORDER, i, std::string("gray"));
    }

    // draw all the cells
    for (auto &row : rows) {
        for (auto &cell : row) {
            if (cell == nullptr) continue;

            if (cell->feed_through) {
                draw_rect(fp, cell->position.x, cell->position.y, 3, 6, std::string("lightgray"), 0.75);
                draw_text(fp, std::string("FT").append(std::to_string(cell->number+1)), cell->position.x + 1.5, cell->position.y + 3, 12, std::string("black"), 270);
            } else {
                if (cell->num_connections == 0)
                    draw_rect(fp, cell->position.x, cell->position.y, 6, 6, std::string("gray"), 0.75);
                else
                    draw_rect(fp, cell->position.x, cell->position.y, 6, 6, std::string("white"), 0.75);
                draw_text(fp, std::string("C").append(std::to_string(cell->number+1)), cell->position.x + 3, cell->position.y + 3, 12);
            }

        }
    }

    // draw all the connections
    for (auto &row : rows) {
        for (auto &cell : row) {
            if (cell == nullptr) continue;
            for (auto &term : cell->terms) {
                if (term.dest_term == nullptr) continue;
                draw_line(fp, term.position.x+0.5, term.position.y+0.5, term.dest_term->position.x+0.5, term.dest_term->position.y+0.5, std::string("red"));
            }
        }
    }

    fprintf(fp, "</svg>\n");
    fclose(fp);
}
