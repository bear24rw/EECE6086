#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "main.h"
#include "magic.h"
#include "test.h"

int num_nets;
int num_cells;

net_t *nets;
cell_t *cells;

point_t get_term_position(cell_t cell, int term)
{
    //
    // Returns the absolute position of a specified terminal for a given cell
    //

    // coordinates of terminals are -1 from what is given in
    // the pdf since we want to work with 0 based indexing
    point_t offsets[]    = {{1,5}, {4,5}, {1,0}, {4,0}};
    point_t offsets_x[]  = {{1,0}, {4,0}, {1,5}, {4,5}};
    point_t offsets_y[]  = {{4,5}, {1,5}, {4,0}, {1,0}};
    point_t offsets_xy[] = {{4,0}, {1,0}, {4,5}, {1,5}};

    point_t position;
    position.x = cell.x + offsets[term].x;
    position.y = cell.y + offsets[term].y;

    if (cell.flip_x) {
        position.x = cell.x + offsets_x[term].x;
        position.y = cell.y + offsets_x[term].y;
    }

    if (cell.flip_y) {
        position.x = cell.x + offsets_y[term].x;
        position.y = cell.y + offsets_y[term].y;
    }

    if (cell.flip_x && cell.flip_y) {
        position.x = cell.x + offsets_xy[term].x;
        position.y = cell.y + offsets_xy[term].y;
    }

    return position;
}

int wirelength(cell_t cell_a, int term_a, cell_t cell_b, int term_b)
{
    //
    // Returns the manhatten distance between two terminals of two given cells
    //

    point_t term_a_pos = get_term_position(cell_a, term_a);
    point_t term_b_pos = get_term_position(cell_b, term_b);

    int x_len = abs(term_a_pos.x - term_b_pos.x);
    int y_len = abs(term_a_pos.y - term_b_pos.y);

    return x_len + y_len;
}


int main(int argc, char *argv[])
{
    #ifdef TEST
    test();
    return 0;
    #endif

    //
    // Build connection list from file
    //

    if (argc < 2) {
        printf("Usage: ./main inputfile\n");
        return EINVAL;
    }

    FILE *fp = fopen(argv[1], "r");

    if(fp == NULL) {
        printf("Cannot open file.\n");
        return ENOENT;
    }

    fscanf(fp, "%d", &num_cells);
    fscanf(fp, "%d", &num_nets);

    printf("Found %d cells and %d nets\n", num_cells, num_nets);

    nets = malloc(num_nets * sizeof(net_t));

    memset(nets, 0, num_nets * sizeof(net_t));

    while (1) {
        int net, cell_a, term_a, cell_b, term_b;
        if (fscanf(fp, "%d%d%d%d%d", &net, &cell_a, &term_a, &cell_b, &term_b) == EOF) break;
        nets[net].cell_a = cell_a;
        nets[net].cell_b = cell_b;
        nets[net].term_a = term_a;
        nets[net].term_b = term_b;
    }

    //
    // Generate all cells
    //

    cells = malloc(num_cells * sizeof(cell_t));

    int grid_w = ceil(sqrt(num_cells));
    int grid_h = ceil(sqrt(num_cells));

    for (int i=0; i<num_cells; i++) {
        cells[i].x = 7 * (i % grid_w);
        cells[i].y = 7 * ((i / grid_w) % grid_w);
        cells[i].flip_x = 0;
        cells[i].flip_y = 0;
    }

    write_magic(cells, num_cells);
}
