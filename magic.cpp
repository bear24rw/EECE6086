#include <stdio.h>
#include <vector>
#include <string>
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

    for (auto &row : rows) {
        for (auto &cell : row) {

            fprintf(fp, "use cell cell_%d\n", cell->number);

            if (cell->flip_x && cell->flip_y) {
                fprintf(fp, "transform -1  0 %d 0 -1 %d\n", cell->x + 6, cell->y + 6);
            } else if (cell->flip_x) {
                fprintf(fp, "transform  1  0 %d 0 -1 %d\n", cell->x + 0, cell->y + 6);
            } else if (cell->flip_y) {
                fprintf(fp, "transform -1  0 %d 0  1 %d\n", cell->x + 6, cell->y + 0);
            } else {
                fprintf(fp, "transform  1  0 %d 0  1 %d\n", cell->x + 0, cell->y + 0);
            }

            fprintf(fp, "box 0 0 6 6\n");
        }
    }

    fclose(fp);
}
