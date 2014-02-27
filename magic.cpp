#include <stdio.h>
#include "magic.h"
#include "main.h"

void write_magic(cell_t *cells, int num_cells)
{

    // transform matrices
    // xy flip [-1, 0, x, 0, -1, y]
    // x  flip [ 1, 0, x, 0, -1, y]
    // y  flip [-1, 0, x, 0,  1, y]
    // no flip [ 1, 0, x, 0,  1, y]


    printf("magic\n");
    printf("tech scmos\n");

    for (int i=0; i<num_cells; i++)
    {
        printf("use cell cell_%d\n", i);

        if (cells[i].flip_x && cells[i].flip_y) {
            printf("transform -1  0 %d 0 -1 %d\n", cells[i].x + 6, cells[i].y + 6);
        } else if (cells[i].flip_x) {
            printf("transform  1  0 %d 0 -1 %d\n", cells[i].x + 0, cells[i].y + 6);
        } else if (cells[i].flip_y) {
            printf("transform -1  0 %d 0  1 %d\n", cells[i].x + 6, cells[i].y + 0);
        } else {
            printf("transform  1  0 %d 0  1 %d\n", cells[i].x + 0, cells[i].y + 0);
        }

        printf("box 0 0 6 6\n");
    }

}
