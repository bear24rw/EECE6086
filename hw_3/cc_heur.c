#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include "shared.h"
#include "cc_heur.h"
#include "mem_log.h"

#define POS_UNATE 1
#define NEG_UNATE 2
#define BINATE 3

int find_unate_column(matrix_t *matrix)
{
    // returns the first column that is all ones or all zeros

    int unate_column = -1;

    for (int j = 0; j < matrix->cols; j++) {

        char has_dash = 0;
        char has_one = 0;
        char has_zero = 0;

        for (int i = 0; i < matrix->rows; i++) {

            if (matrix->cubes[i][j] == '-') has_dash = 1;
            if (matrix->cubes[i][j] == '1') has_one = 1;
            if (matrix->cubes[i][j] == '0') has_zero = 1;

            if (has_dash) break;

            if (has_one && has_zero) {
                break;
            }
        }

        if (has_dash) continue;
        if (has_one && has_zero) continue;

        unate_column = j;
        break;
    }

    return unate_column;
}

void and(matrix_t *matrix, int col, char value)
{
    for (int y = 0; y < matrix->rows; y++) {
        if (matrix->cubes[y][col] == '-') matrix->cubes[y][col] = value;
    }
}

matrix_t *concat(matrix_t *a, matrix_t *b)
{
    matrix_t *m = alloc_matrix(a->rows + b->rows, a->cols);

    int row = 0;

    for (int i = 0; i < a->rows; i++) {
        memcpy(m->cubes[row], a->cubes[i], a->cols);
        row++;
    }

    for (int i = 0; i < b->rows; i++) {
        memcpy(m->cubes[row], b->cubes[i], b->cols);
        row++;
    }

    return m;
}

int find_binate_column(matrix_t *matrix)
{
    int comp_form = 0;
    int true_form = 0;

    int min_column = -1;
    int min_difference = INT_MAX;
    int min_num_comps = 0;
    int min_num_trues = 0;

    for (int j = 0; j < matrix->cols; j++) {

        true_form = 0;
        comp_form = 0;

        // total up the number of 1s and 0s in this column
        for (int i = 0; i < matrix->rows; i++) {
            if (matrix->cubes[i][j] == '1') true_form++;
            if (matrix->cubes[i][j] == '0') comp_form++;
        }

        if (true_form == 0 || comp_form == 0) continue;

        int difference = abs(true_form - comp_form);
        if ((difference < min_difference) ||
            (difference == min_difference &&
             (comp_form > min_num_comps || true_form > min_num_trues))) {
            min_difference = difference;
            min_column = j;
            min_num_comps = comp_form;
            min_num_trues = true_form;
        }
    }

    return min_column;
}

matrix_t *check_complement(matrix_t *matrix, int depth)
{
    recursion_depth = depth;

    if (whole_row_of(matrix, '-')) {
        matrix_t *m = alloc_matrix(0, matrix->cols);
        free_matrix(matrix);
        return m;
    }

    if (matrix->rows == 0) {
        matrix_t *m = alloc_matrix(1, matrix->cols);
        memset(m->cubes[0], '-', matrix->cols);
        free_matrix(matrix);
        return m;
    }

    if (matrix->rows == 1) {

        int num_of_none_dashes = 0;
        for (int i = 0; i < matrix->cols; i++) {
            if (matrix->cubes[0][i] == '0') num_of_none_dashes++;
            if (matrix->cubes[0][i] == '1') num_of_none_dashes++;
        }

        matrix_t *m = alloc_matrix(num_of_none_dashes, matrix->cols);

        int row = 0;
        for (int i = 0; i < matrix->cols; i++) {
            if (matrix->cubes[0][i] == '-') continue;

            memset(m->cubes[row], '-', matrix->cols);

            if (matrix->cubes[0][i] == '0') m->cubes[row][i] = '1';
            if (matrix->cubes[0][i] == '1') m->cubes[row][i] = '0';

            row++;
        }

        free_matrix(matrix);
        return m;
    }

    int col_type = BINATE;
    int column = find_binate_column(matrix);

    if (column == -1) {
        column = find_unate_column(matrix);
        if (matrix->cubes[0][column] == '1') col_type = POS_UNATE;
        if (matrix->cubes[0][column] == '0') col_type = NEG_UNATE;
    }

    matrix_t *pos = check_complement(co_factor(matrix, column, '1'), depth+1); recursion_depth = depth;
    matrix_t *neg = check_complement(co_factor(matrix, column, '0'), depth+1); recursion_depth = depth;

    // positive unate
    if (col_type == POS_UNATE) and(neg, column, '0');
    if (col_type == NEG_UNATE) and(pos, column, '1');
    if (col_type == BINATE) {
        and(pos, column, '1');
        and(neg, column, '0');
    }

    matrix_t *m = concat(pos, neg);

    free_matrix(pos);
    free_matrix(neg);
    free_matrix(matrix);

    return m;
}

void *heur(void *filename)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    FILE *fp = fopen((const char *)filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file!\n");
        pthread_cond_signal(&done_signal);
        return 0;
    }


    int cols, rows;

    fscanf(fp, "%d", &cols);
    fscanf(fp, "%d", &rows);

    matrix_t *matrix = alloc_matrix(rows, cols);

    for (int i = 0; i < matrix->rows; i++) {
        fscanf(fp, "%s", matrix->cubes[i]);
        replace_under_with_dash(matrix->cubes[i], matrix->cols);
    }

    fclose(fp);

    matrix_t *complements = check_complement(matrix, 0);

    pthread_mutex_lock(&print_mutex);

    fprintf(stderr, "Heur is printing complements\n");

    print_matrix(complements);

    free_matrix(matrix);

    pthread_cond_signal(&done_signal);

    return NULL;
}
