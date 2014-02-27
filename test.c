#include <stdio.h>
#include "test.h"
#include "main.h"
#include "magic.h"

int test_num = 0;

void expect_len(int term_a, int term_b, int actual, int expected) {
    if (actual != expected) printf("[test: %d] Expected term %d to term %d to be length %d got %d\n", test_num, term_a, term_b, expected, actual);
}

void test()
{

    // WIRELENGTH TESTS

    int len = -1;

    cell_t cell_a;
    cell_t cell_b;

    cell_a.flip_x = 0; cell_a.flip_y = 0;
    cell_b.flip_x = 0; cell_b.flip_y = 0;

    cell_a.x = 0; cell_a.y = 0;
    cell_b.x = 6; cell_b.y = 0;

    test_num = 1;
    len = wirelength(cell_a, 0, cell_b, 0); expect_len(0, 0, len, 6);
    len = wirelength(cell_a, 0, cell_b, 1); expect_len(0, 1, len, 9);
    len = wirelength(cell_a, 0, cell_b, 2); expect_len(0, 2, len, 11);
    len = wirelength(cell_a, 0, cell_b, 3); expect_len(0, 3, len, 14);

    test_num = 2;
    len = wirelength(cell_a, 1, cell_b, 0); expect_len(1, 0, len, 3);
    len = wirelength(cell_a, 1, cell_b, 1); expect_len(1, 1, len, 6);
    len = wirelength(cell_a, 1, cell_b, 2); expect_len(1, 2, len, 8);
    len = wirelength(cell_a, 1, cell_b, 3); expect_len(1, 3, len, 11);

    test_num = 3;
    len = wirelength(cell_a, 2, cell_b, 0); expect_len(2, 0, len, 11);
    len = wirelength(cell_a, 2, cell_b, 1); expect_len(2, 1, len, 14);
    len = wirelength(cell_a, 2, cell_b, 2); expect_len(2, 2, len, 6);
    len = wirelength(cell_a, 2, cell_b, 3); expect_len(2, 3, len, 9);

    test_num = 4;
    len = wirelength(cell_a, 3, cell_b, 0); expect_len(3, 0, len, 8);
    len = wirelength(cell_a, 3, cell_b, 1); expect_len(3, 1, len, 11);
    len = wirelength(cell_a, 3, cell_b, 2); expect_len(3, 2, len, 3);
    len = wirelength(cell_a, 3, cell_b, 3); expect_len(3, 3, len, 6);

    // MAGIC OUTPUT

    //    1 2     2 1     3 4     4 3
    //    3 4     4 3     1 2     2 1

    cell_t cells[4];

    cells[0].x = 0*7; cells[0].y = 0; cells[0].flip_x = 0; cells[0].flip_y = 0;
    cells[1].x = 1*7; cells[1].y = 0; cells[1].flip_x = 0; cells[1].flip_y = 1;
    cells[2].x = 2*7; cells[2].y = 0; cells[2].flip_x = 1; cells[2].flip_y = 0;
    cells[3].x = 3*7; cells[3].y = 0; cells[3].flip_x = 1; cells[3].flip_y = 1;

    write_magic(cells, 4);

    printf("Tests done.\n");
}
