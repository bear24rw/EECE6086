#include <stdio.h>
#include <vector>
#include "magic.h"
#include "main.h"

void write_magic(rows_t& rows)
{

    // transform matrices
    // xy flip [-1, 0, x, 0, -1, y]
    // x  flip [ 1, 0, x, 0, -1, y]
    // y  flip [-1, 0, x, 0,  1, y]
    // no flip [ 1, 0, x, 0,  1, y]


    printf("magic\n");
    printf("tech scmos\n");

    for (auto &row : rows) {
        for (auto &cell : row) {

            printf("use cell cell_%d\n", cell->number);

            if (cell->flip_x && cell->flip_y) {
                printf("transform -1  0 %d 0 -1 %d\n", cell->x + 6, cell->y + 6);
            } else if (cell->flip_x) {
                printf("transform  1  0 %d 0 -1 %d\n", cell->x + 0, cell->y + 6);
            } else if (cell->flip_y) {
                printf("transform -1  0 %d 0  1 %d\n", cell->x + 6, cell->y + 0);
            } else {
                printf("transform  1  0 %d 0  1 %d\n", cell->x + 0, cell->y + 0);
            }

            printf("box 0 0 6 6\n");
        }
        printf("finished row\n");
    }

    printf("finished cells\n");

}
