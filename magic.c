#include <stdio.h>
#include "magic.h"
#include "main.h"

void write_magic(cell_t *cells, int num_cells)
{
    // transform [ _,  _, x,  _,  _, y]

    // 0         [ 1,  0,  0,  1]
    // 90        [ 0,  1, -1,  0]
    // 180       [-1,  0,  0, -1]
    // 270       [ 0, -1,  1,  0]

    // 0 + Y     [-1,  0,  0,  1]
    // 90 + Y    [ 0,  1,  1,  0]
    // 180 + Y   [ 1,  0,  0, -1]
    // 270 + Y   [ 0, -1, -1,  0]

    /*
                0       90      180     270
                ---     ---     ---     ---
    No Flip     1 2     2 4     4 3     3 1
                3 4     1 3     2 1     4 2

    Y Flip      2 1     4 2     3 4     1 3
                4 3     3 1     1 2     2 4

    X Flip      3 4     1 3     2 1     4 2
                1 2     2 4     4 3     3 1

    XY Flip     4 3     3 1     1 2     2 4
                2 1     4 2     3 4     1 3
    */

    printf("magic\n");
    printf("tech scmos\n");

    for (int i=0; i<num_cells; i++)
    {
        printf("use cell cell_%d\n", i);

        if (cells[i].flipped) {
            switch (cells[i].rot) {
                case ROT_0:   printf("transform -1  0 %d  0  1 %d\n", cells[i].x + 6, cells[i].y + 0); break;
                case ROT_90:  printf("transform  0  1 %d  1  0 %d\n", cells[i].x + 0, cells[i].y + 0); break;
                case ROT_180: printf("transform  1  0 %d  0 -1 %d\n", cells[i].x + 0, cells[i].y + 6); break;
                case ROT_270: printf("transform  0 -1 %d -1  0 %d\n", cells[i].x + 6, cells[i].y + 6); break;
            }
        } else {
            switch (cells[i].rot) {
                case ROT_0:   printf("transform  1  0 %d  0  1 %d\n", cells[i].x + 0, cells[i].y + 0); break;
                case ROT_90:  printf("transform  0 -1 %d  1  0 %d\n", cells[i].x + 6, cells[i].y + 0); break;
                case ROT_180: printf("transform -1  0 %d  0 -1 %d\n", cells[i].x + 6, cells[i].y + 6); break;
                case ROT_270: printf("transform  0  1 %d -1  0 %d\n", cells[i].x + 0, cells[i].y + 6); break;
            }
        }

        printf("box 0 0 6 6\n");
    }

}
