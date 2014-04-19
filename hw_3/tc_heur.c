#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "shared.h"
#include "tc_heur.h"
#include "mem_log.h"

int find_most_binate(matrix_t *matrix, char *more_ones_than_zeros)
{
    int comp_form = 0;
    int true_form = 0;

    int min_column = 0;
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

        if (true_form == 0 && comp_form == 0)
            continue;

        // special case means its not a tautology
        if (true_form == matrix->rows || comp_form == matrix->rows)
            return -1;

        int difference = abs(true_form - comp_form);
        if ((difference < min_difference) ||
            (difference == min_difference &&
             (comp_form > min_num_comps || true_form > min_num_trues))) {
            min_difference = difference;
            min_column = j;
            min_num_comps = comp_form;
            min_num_trues = true_form;
            *more_ones_than_zeros = true_form > comp_form;
        }
    }

    return min_column;
}

matrix_t *unate_reduction(matrix_t *matrix)
{

    //
    // Find the unate columns
    //

    //printf("reducing:\n");
    //print_matrix(matrix);

    char *unate_columns = malloc(matrix->cols);
    int num_unate_cols = 0;

    for (int j = 0; j < matrix->cols; j++) {

        // assume this column is unate
        unate_columns[j] = 1;

        char all_dashes = 1;
        char has_one = 0;
        char has_zero = 0;

        for (int i = 0; i < matrix->rows; i++) {

            if (matrix->cubes[i][j] != '-') all_dashes = 0;

            if (matrix->cubes[i][j] == '1') has_one = 1;
            if (matrix->cubes[i][j] == '0') has_zero = 1;

            // if there are different values in this column exit
            if (has_one && has_zero) {
                unate_columns[j] = 0;
                break;
            }
        }

        // if whole column is dashes we dont mark it as unate
        if (all_dashes) unate_columns[j] = 1;

        if (unate_columns[j]) {
            num_unate_cols++;
        }
    }

    // if we didn't find any unate columns we are done
    if (num_unate_cols == 0) {
        free(unate_columns);
        return matrix;
    }

    //
    // Figure out what rows to keep
    //

    char *keep_rows = malloc(matrix->rows);
    int num_rows = 0;

    for (int i = 0; i < matrix->rows; i++) {

        // assume all unate columns have dash unless proven otherwise
        char all_dash = 1;

        // assume we are not going to keep this run
        keep_rows[i] = 0;

        // check that all unate columns in this row are '-'
        for (int j = 0; j < matrix->cols; j++) {
            if (!unate_columns[j]) continue;
            if (matrix->cubes[i][j] != '-') {
                all_dash = 0;
                break;
            }
        }

        if (all_dash) {
            keep_rows[i] = 1;
            num_rows++;
        }
    }

    if (num_rows == 0) {
        free(unate_columns);
        free(keep_rows);
        return matrix;
    }

    //
    // Move the submatrix in the bottom right to the top left of its own matrix
    //

    matrix_t *temp_matrix = alloc_matrix(num_rows, matrix->cols - num_unate_cols);

    int row = 0;
    int col = 0;
    for (int i = 0; i < matrix->rows; i++) {
        if (!keep_rows[i]) continue;
        col = 0;
        for (int j = 0; j < matrix->cols; j++) {
            if (unate_columns[j]) continue;
            temp_matrix->cubes[row][col] = matrix->cubes[i][j];
            col++;
        }
        row++;
    }

    /*
    printf("Reduced %d %d (ptr: %p) to %d %d (ptr: %p)\n",
            matrix->rows, matrix->cols, matrix,
            temp_matrix->rows, temp_matrix->cols, temp_matrix);

    printf("new matrix:\n");
    print_matrix(temp_matrix);
    */

    free(keep_rows);
    free(unate_columns);

    return temp_matrix;
}

// http://cc.ee.ntu.edu.tw/~jhjiang/instruction/courses/fall10-lsv/lec03-2_2p.pdf
int check_tautology(matrix_t *matrix, int depth)
{
    recursion_depth = depth;

    // check to see if we have run out of cubes
    if (matrix->rows == 0) return 0;

    char all_dashes = whole_row_of(matrix, '-');

    if (matrix->rows == 1 && !all_dashes) return 0;

    if (all_dashes) return 1;

    // undate_reduction may or may not return a new matrix we need to keep
    // track of the original one so we can tell if we need to free it or not.
    // if unate reduction returns a new matrix we need to free it before we
    // leave.
    matrix_t *oldmat = matrix;
    matrix = unate_reduction(matrix);

    // check for special case
    if (whole_row_of(matrix, '-')) {
        if (oldmat != matrix) free_matrix(matrix);
        return 1;
    }

    // pick most binate variable to check for tautology
    char more_ones_than_zeros = 0;
    int binate_var = find_most_binate(matrix, &more_ones_than_zeros);

    if (binate_var == -1) {
        if (oldmat != matrix) free_matrix(matrix);
        return 0;
    }

    matrix_t *C0 = co_factor(matrix, binate_var, more_ones_than_zeros ? '1' : '0');
    if (!check_tautology(C0, depth+1)) {
        recursion_depth = depth;
        if (oldmat != matrix) free_matrix(matrix);
        free_matrix(C0);
        return 0;
    }

    matrix_t *C1 = co_factor(matrix, binate_var, more_ones_than_zeros ? '0' : '1');
    if (!check_tautology(C1, depth+1)) {
        recursion_depth = depth;
        if (oldmat != matrix) free_matrix(matrix);
        free_matrix(C0);
        free_matrix(C1);
        return 0;
    }

    free_matrix(C0);
    free_matrix(C1);
    if (oldmat != matrix) free_matrix(matrix);
    recursion_depth = depth;

    return 1;
}

void *heur(void *filename)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    FILE *fp = fopen((const char *)filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file!\n");
        pthread_cond_signal(&done_signal);
        return NULL;
    }

    int rows = 0;
    int cols = 0;

    fscanf(fp, "%d", &cols);
    fscanf(fp, "%d", &rows);

    matrix_t *matrix = alloc_matrix(rows, cols);

    for (int i = 0; i < matrix->rows; i++) {
        fscanf(fp, "%s", matrix->cubes[i]);
        replace_under_with_dash(matrix->cubes[i], matrix->cols);
    }

    fclose(fp);

    is_tautology = check_tautology(matrix, 0);

    free_matrix(matrix);

    fprintf(stderr, "Heur found it\n");
    pthread_cond_signal(&done_signal);

    return NULL;
}
