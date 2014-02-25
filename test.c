#include <stdio.h>
#include "test.h"
#include "main.h"

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
    cell_a.rot = ROT_0;
    cell_a.flip = FLIP_NONE;

    test_num = 1;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 1, 5);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 4, 5);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 1, 0);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 4, 0);

    // FLIP TESTS

    test_num = 2;
    cell_a.flip = FLIP_Y;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 4, 5);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 1, 5);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 4, 0);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 1, 0);

    test_num = 3;
    cell_a.flip = FLIP_X;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 1, 0);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 4, 0);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 1, 5);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 4, 5);

    test_num = 4;
    cell_a.flip = FLIP_XY;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 4, 0);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 1, 0);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 4, 5);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 1, 5);

    cell_a.flip = FLIP_NONE;

    // ROTATION TESTS

    test_num = 5;
    cell_a.rot = ROT_90;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 0, 1);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 0, 4);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 5, 1);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 5, 4);

    test_num = 6;
    cell_a.rot = ROT_180;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 4, 0);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 1, 0);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 4, 5);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 1, 5);

    test_num = 7;
    cell_a.rot = ROT_270;
    pos = get_term_position(cell_a, 0); expect_pos(pos, 0, 5, 4);
    pos = get_term_position(cell_a, 1); expect_pos(pos, 1, 5, 1);
    pos = get_term_position(cell_a, 2); expect_pos(pos, 2, 0, 4);
    pos = get_term_position(cell_a, 3); expect_pos(pos, 3, 0, 1);

    // TODO: FLIP + ROTATION TESTS

    // WIRELENGTH TESTS

    int len = -1;

    cell_t cell_b;
    cell_b.rot = ROT_0;
    cell_b.flip = FLIP_NONE;

    cell_a.rot = ROT_0;
    cell_a.flip = FLIP_NONE;

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

    printf("Tests done.\n");
}
