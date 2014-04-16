#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include "main.h"

typedef struct {
    char **cubes;
    int cols;
    int rows;
    int alloc_cols;
    int alloc_rows;
} matrix_t;

void free_matrix(matrix_t *matrix) {
    for (int i=0; i < matrix->alloc_rows; i++)
        free(matrix->cubes[i]);
    free(matrix->cubes);
    free(matrix);
    matrix = NULL;
}

static void replace_under_with_dash(char *cube, int num_cols)
{
    for (int i=0; i<num_cols; i++)
        if (cube[i] == '_') cube[i] = '-';
}

char whole_row_of(matrix_t *matrix, char value)
{
    char has_whole_row = 0;

    for (int i=0; i < matrix->rows; i++) {

        has_whole_row = 1;
        for (int j=0; j < matrix->cols; j++) {
            if (matrix->cubes[i][j] != value) has_whole_row = 0;
        }
        if (has_whole_row) break;
    }

    return has_whole_row;
}

int find_most_binate(matrix_t *matrix, char *more_ones_than_zeros)
{
    int comp_form = 0;
    int true_form = 0;

    int min_column = 0;
    int min_difference = INT_MAX;
    int min_num_comps = 0;
    int min_num_trues = 0;

    for (int j=0; j<matrix->cols; j++) {

        true_form = 0;
        comp_form = 0;

        // total up the number of 1s and 0s in this column
        for (int i=0; i<matrix->rows; i++) {
            if (matrix->cubes[i][j] == '1') true_form++;
            if (matrix->cubes[i][j] == '0') comp_form++;
        }

        if (true_form == 0 && comp_form == 0)
            continue;

        // if num 1s and num 0s are same there is nothing better
        /*
        if (matrix->rows & 1) {
            if (true_form > 0 && abs(true_form - comp_form) <= 1) return j;
            if (comp_form > 0 && abs(true_form - comp_form) <= 1) return j;
        } else {
            if (true_form > 0 && true_form == comp_form) return j;
            if (comp_form > 0 && true_form == comp_form) return j;
        }
        */

        // special case means its not a tautology
        if (true_form == matrix->rows || comp_form == matrix->rows)
            return -1;

        int difference = abs(true_form - comp_form);
        if ((difference < min_difference) ||
            (difference == min_difference && (comp_form > min_num_comps || true_form > min_num_trues))) {
            min_difference = difference;
            min_column = j;
            min_num_comps = comp_form;
            min_num_trues = true_form;
            *more_ones_than_zeros = true_form > comp_form;
        }
    }

    return min_column;
}

matrix_t *co_factor(matrix_t *matrix, int column, char pc)
{
    matrix_t *temp_matrix = (matrix_t *)malloc(sizeof(matrix_t));

    temp_matrix->rows = matrix->rows;
    temp_matrix->cols = matrix->cols;
    temp_matrix->alloc_rows = matrix->rows;
    temp_matrix->cubes = (char **)malloc(matrix->rows*sizeof(char*));

    for (int i=0; i<matrix->rows; i++) {
        temp_matrix->cubes[i] = (char *)malloc(matrix->cols);
    }

    int row = 0;

    for (int i=0; i<matrix->rows; i++) {

        if (matrix->cubes[i][column] != pc && matrix->cubes[i][column] != '-') continue;

        memcpy(temp_matrix->cubes[row], matrix->cubes[i], matrix->cols);

        temp_matrix->cubes[row][column] = '-';

        row++;
    }

    temp_matrix->rows = row;

    return temp_matrix;
}

void unate_reduction(matrix_t *matrix)
{

    //
    // Find the unate columns
    //

    char *unate_columns = malloc(matrix->cols);
    int num_unate_cols = 0;

    for (int j=0; j<matrix->cols; j++) {

        // assume this column is unate
        unate_columns[j] = 1;

        char all_dashes = 1;
        char has_one = 0;
        char has_zero = 0;

        for (int i=0; i<matrix->rows; i++) {

            if (matrix->cubes[i][j] != '-')
                all_dashes = 0;

            if (matrix->cubes[i][j] == '1') has_one = 1;
            if (matrix->cubes[i][j] == '0') has_zero = 1;

            // if there are different values in this column exit
            if (has_one && has_zero) {
                unate_columns[j] = 0;
                break;
            }
        }

        // if whole column is dashes we dont mark it as unate
        if (all_dashes) unate_columns[j] = 0;

        if (unate_columns[j])  {
            num_unate_cols++;
        }
    }

    // if we didn't find any unate columns we are done
    if (num_unate_cols == 0) {
        free(unate_columns);
        return;
    }

    //
    // Figure out what rows to keep
    //

    char *keep_rows = malloc(matrix->rows);
    int num_rows = 0;

    for (int i=0; i<matrix->rows; i++) {

        // assume all unate columns have dash unless proven otherwise
        char all_dash = 1;

        // assume we are not going to keep this run
        keep_rows[i] = 0;

        // check that all unate columns in this row are '-'
        for (int j=0; j<matrix->cols; j++) {
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
        return;
    }

    //
    // Move the submatrix in the bottom right to the top left of its own matrix
    //

    matrix_t *temp_matrix = (matrix_t *)malloc(sizeof(matrix_t));

    temp_matrix->rows = num_rows;
    temp_matrix->cols = matrix->cols - num_unate_cols;
    temp_matrix->alloc_rows = num_rows;
    temp_matrix->cubes = (char **)malloc(num_rows*sizeof(char*));

    for (int i=0; i<num_rows; i++) {
        temp_matrix->cubes[i] = (char *)malloc(matrix->cols - num_unate_cols);
    }


    int row = 0;
    int col = 0;
    for (int i=0; i<matrix->rows; i++) {
        if (!keep_rows[i]) continue;
        col = 0;
        for (int j=0; j<matrix->cols; j++) {
            if (unate_columns[j]) continue;
            temp_matrix->cubes[row][col] = matrix->cubes[i][j];
            col++;
        }
        row++;
    }


    free(keep_rows);
    free(unate_columns);

    matrix = temp_matrix;

    /*
    printf(">>> reduced matrix to this\n");
    for (int i=0; i<matrix->rows; i++) {
        for (int j=0; j<matrix->cols; j++) {
            printf("%c", matrix->cubes[i][j]);
        }
        printf("\n");
    }
    */

    return;
}

//http://cc.ee.ntu.edu.tw/~jhjiang/instruction/courses/fall10-lsv/lec03-2_2p.pdf
int check_tautology(matrix_t *matrix)
{
    /*
     positive cofactor (x = 1):
     [..1..] => remove from minterm from cube
     [..0..] => replace minterm with don't care (-)
     [..-..] => leave cube alone

     negative cofactor (x = 0):
     [..0..] => remove from minterm from cube
     [..1..] => replace minterm with don't care (-)
     [..-..] => leave cube alone
    */

    // check to see if we have run out of cubes
    if (matrix->rows == 0) return 0;

    if (matrix->rows == 1 && !whole_row_of(matrix, '-'))
        return 0;

    unate_reduction(matrix);

    // check for special case
    if (whole_row_of(matrix, '-')) {
        return 1;
    }

    // pick most binate variable to check for tautology
    char more_ones_than_zeros = 0;
    int binate_var = find_most_binate(matrix, &more_ones_than_zeros);

    if (binate_var == -1) return 0;

    matrix_t *C0 = co_factor(matrix, binate_var, more_ones_than_zeros ? '0' : '1');
    if (!check_tautology(C0)){
        free_matrix(C0);
        return 0;
    }

    matrix_t *C1 = co_factor(matrix, binate_var, more_ones_than_zeros ? '1' : '0');
    if (!check_tautology(C1)){
        free_matrix(C1);
        return 0;
    }

    free_matrix(C0);
    free_matrix(C1);

    return 1;
}

void *heur(void *filename)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

    FILE *fp = fopen((const char *)filename, "r");
    if (fp == NULL) {
        printf("Could not open file!\n");
        pthread_cond_signal(&done_signal);
        return NULL;
    }

    matrix_t *matrix = (matrix_t *)malloc(sizeof(matrix_t));

    fscanf(fp, "%d", &matrix->cols);
    fscanf(fp, "%d", &matrix->rows);

    matrix->cubes = (char **)malloc(matrix->rows*sizeof(char*));

    matrix->alloc_rows = matrix->rows;

    for (int i=0; i<matrix->rows; i++) {
        matrix->cubes[i] = (char *)malloc(matrix->cols);
    }

    for (int i=0; i<matrix->rows; i++) {
        fscanf(fp, "%s", matrix->cubes[i]);
        replace_under_with_dash(matrix->cubes[i], matrix->cols);
    }

    fclose(fp);

    if (check_tautology(matrix)) {
        printf("Function is a tautololgy\n");
    } else {
        printf("Function is NOT a tautololgy\n");
    }

    free_matrix(matrix);

    printf("Heur found it\n");
    pthread_cond_signal(&done_signal);

    return NULL;
}
