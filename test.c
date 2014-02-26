#include <stdio.h>
#include "test.h"
#include "main.h"
#include "magic.h"

int test_num = 0;

void expect_pos(point_t pos, int term, int x, int y) {
    if (pos.x != x) printf("[test: %d] Expected term %d x position to be %d got %d\n", test_num, term, x, pos.x);
    if (pos.y != y) printf("[test: %d] Expected term %d y position to be %d got %d\n", test_num, term, y, pos.y);
}

void expect_len(int term_a, int term_b, int actual, int expected) {
    if (actual != expected) printf("[test: %d] Expected term %d to term %d to be length %d got %d\n", test_num, term_a, term_b, expected, actual);
}

void test()
{
    point_t pos;

    cell_t cell_a;
    cell_a.x = 0;
    cell_a.y = 0;

    // ROTATION TESTS

    cell_a.flipped = 0;

    test_num = 1;
    cell_a.rot = ROT_0;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 1, 5);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 4, 5);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 1, 0);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 4, 0);

    test_num = 2;
    cell_a.rot = ROT_90;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 0, 1);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 0, 4);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 5, 1);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 5, 4);

    test_num = 3;
    cell_a.rot = ROT_180;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 4, 0);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 1, 0);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 4, 5);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 1, 5);

    test_num = 4;
    cell_a.rot = ROT_270;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 5, 4);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 5, 1);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 0, 4);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 0, 1);

    // FLIP + ROTATION TESTS

    cell_a.flipped = 1;

    test_num = 5;
    cell_a.rot = ROT_0;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 4, 5);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 1, 5);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 4, 0);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 1, 0);

    test_num = 6;
    cell_a.rot = ROT_90;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 4, 0);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 4, 5);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 1, 0);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 1, 5);

    test_num = 7;
    cell_a.rot = ROT_180;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 1, 0);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 4, 0);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 1, 5);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 4, 5);

    test_num = 8;
    cell_a.rot = ROT_270;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 4, 5);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 1, 5);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 4, 0);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 1, 0);

    cell_a.flipped = 0;

    // WIRELENGTH TESTS

    int len = -1;

    cell_t cell_b;
    cell_b.rot = ROT_0;
    cell_b.flipped = 0;

    cell_a.rot = ROT_0;
    cell_a.flipped = 0;

    test_num = 8;
    cell_a.x = 0; cell_a.y = 0;
    cell_b.x = 6; cell_b.y = 0;
    len = wirelength(cell_a, 0, cell_b, 0); expect_len(0, 0, len, 6);
    len = wirelength(cell_a, 0, cell_b, 1); expect_len(0, 1, len, 9);
    len = wirelength(cell_a, 0, cell_b, 2); expect_len(0, 2, len, 11);
    len = wirelength(cell_a, 0, cell_b, 3); expect_len(0, 3, len, 14);

    len = wirelength(cell_a, 1, cell_b, 0); expect_len(1, 0, len, 3);
    len = wirelength(cell_a, 1, cell_b, 1); expect_len(1, 1, len, 6);
    len = wirelength(cell_a, 1, cell_b, 2); expect_len(1, 2, len, 8);
    len = wirelength(cell_a, 1, cell_b, 3); expect_len(1, 3, len, 11);

    len = wirelength(cell_a, 2, cell_b, 0); expect_len(2, 0, len, 11);
    len = wirelength(cell_a, 2, cell_b, 1); expect_len(2, 1, len, 14);
    len = wirelength(cell_a, 2, cell_b, 2); expect_len(2, 2, len, 6);
    len = wirelength(cell_a, 2, cell_b, 3); expect_len(2, 3, len, 9);

    len = wirelength(cell_a, 3, cell_b, 0); expect_len(3, 0, len, 8);
    len = wirelength(cell_a, 3, cell_b, 1); expect_len(3, 1, len, 11);
    len = wirelength(cell_a, 3, cell_b, 2); expect_len(3, 2, len, 3);
    len = wirelength(cell_a, 3, cell_b, 3); expect_len(3, 3, len, 6);

    // MAGIC OUTPUT

    //    1 2     2 4     4 3     3 1
    //    3 4     1 3     2 1     4 2

    //    2 1     4 2     3 4     1 3
    //    4 3     3 1     1 2     2 4

    cell_t cells[8];

    cells[0].x = 0*7; cells[0].y = 7; cells[0].rot = ROT_0;   cells[0].flipped = 0;
    cells[1].x = 1*7; cells[1].y = 7; cells[1].rot = ROT_90;  cells[1].flipped = 0;
    cells[2].x = 2*7; cells[2].y = 7; cells[2].rot = ROT_180; cells[2].flipped = 0;
    cells[3].x = 3*7; cells[3].y = 7; cells[3].rot = ROT_270; cells[3].flipped = 0;

    cells[4].x = 0*7; cells[4].y = 0; cells[4].rot = ROT_0;   cells[4].flipped = 1;
    cells[5].x = 1*7; cells[5].y = 0; cells[5].rot = ROT_90;  cells[5].flipped = 1;
    cells[6].x = 2*7; cells[6].y = 0; cells[6].rot = ROT_180; cells[6].flipped = 1;
    cells[7].x = 3*7; cells[7].y = 0; cells[7].rot = ROT_270; cells[7].flipped = 1;

    write_magic(cells, 8);

    printf("Tests done.\n");
}
