#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include "magic.h"
#include "main.h"

void write_magic(std::string filename, rows_t& rows)
{
    // transform matrices
    // xy flip [-1, 0, x, 0, -1, y]
    // x  flip [ 1, 0, x, 0, -1, y]
    // y  flip [-1, 0, x, 0,  1, y]
    // no flip [ 1, 0, x, 0,  1, y]

    filename.append(".mag");

    FILE *fp = fopen(filename.c_str(), "w");

    fprintf(fp, "magic\n");
    fprintf(fp, "tech scmos\n");

    //
    // Place all the cells
    //

    for (auto &row : rows) {
        for (auto &cell : row) {

            if (cell->feed_through) {
                fprintf(fp, "use feed_through feed_though_%d\n", cell->number);
            } else {
                fprintf(fp, "use cell cell_%d\n", cell->number);
            }

            if (cell->flip_x && cell->flip_y) {
                fprintf(fp, "transform -1  0 %d 0 -1 %d\n", cell->x + 6, cell->y + 6);
            } else if (cell->flip_x) {
                fprintf(fp, "transform  1  0 %d 0 -1 %d\n", cell->x + 0, cell->y + 6);
            } else if (cell->flip_y) {
                fprintf(fp, "transform -1  0 %d 0  1 %d\n", cell->x + 6, cell->y + 0);
            } else {
                fprintf(fp, "transform  1  0 %d 0  1 %d\n", cell->x + 0, cell->y + 0);
            }

            if (cell->feed_through) {
                fprintf(fp, "box 0 0 3 6\n");
            } else {
                fprintf(fp, "box 0 0 6 6\n");
            }
        }
    }

    //
    // Place all the horizontal routes
    //

    fprintf(fp, "<< metal1 >>\n");
    for (auto &row : rows) {
        for (auto &cell : row) {
            int num_terms = cell->feed_through ? 2 : 4;
            for (int term=0; term<num_terms; term++) {
                if (cell->term[term].dest_cell == nullptr) continue;
                point_t p1 = get_term_position(cell, term);
                point_t p2 = get_term_position(cell->term[term].dest_cell, cell->term[term].dest_term);
                if (term_on_top(cell, term)) {
                    p1.y += cell->term[term].track * 2;
                } else {
                    p1.y -= cell->term[term].track * 2;
                }
                p2.y = p1.y;
                int x1 = std::min(p1.x, p2.x);
                int x2 = std::max(p1.x, p2.x);
                int y1 = p1.y;
                int y2 = p1.y + 1;
                fprintf(fp, "rect %d %d %d %d\n", x1, y1, x2, y2);
            }
        }
    }

    fprintf(fp, "<< end >>\n");
    fclose(fp);
}
