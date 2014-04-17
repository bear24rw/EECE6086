#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    char **cubes;
    int cols;
    int rows;
    int alloc_cols;
    int alloc_rows;
} matrix_t;

void print_matrix(matrix_t *matrix) {
    for (int y=0; y<matrix->rows; y++) {
        for (int x=0; x<matrix->cols; x++) {
            printf("%c", matrix->cubes[y][x]);
        }
        printf("\n");
    }
}

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

int find_unate_column(matrix_t *matrix)
{
    // returns the first column that is all ones or all zeros

    int unate_column = -1;

    for (int j=0; j<matrix->cols; j++) {

        char has_dash = 0;
        char has_one = 0;
        char has_zero = 0;

        for (int i=0; i<matrix->rows; i++) {

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
    for (int y=0; y<matrix->rows; y++) {
        if (matrix->cubes[y][col] == '-')
            matrix->cubes[y][col] = value;
    }
}

matrix_t *concat(matrix_t *a, matrix_t *b)
{
    matrix_t *m = (matrix_t *)malloc(sizeof(matrix_t));

    m->cols = a->cols;
    m->rows = a->rows + b->rows;
    m->alloc_rows = m->rows;
    m->cubes = (char **)malloc(m->rows*sizeof(char*));

    int row = 0;

    for (int i=0; i<a->rows; i++) {
        m->cubes[row] = (char *)malloc(a->cols);
        memcpy(m->cubes[row], a->cubes[i], a->cols);
        row++;
    }

    for (int i=0; i<b->rows; i++) {
        m->cubes[row] = (char *)malloc(b->cols);
        memcpy(m->cubes[row], b->cubes[i], b->cols);
        row++;
    }

    return m;
}

int find_binate_column(matrix_t *matrix)
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

        int difference = abs(true_form - comp_form);
        if ((difference < min_difference) ||
            (difference == min_difference && (comp_form > min_num_comps || true_form > min_num_trues))) {
            min_difference = difference;
            min_column = j;
            min_num_comps = comp_form;
            min_num_trues = true_form;
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

    printf("in cofactor rows: %d col: %d pos/neg: %c\n", matrix->rows, column, pc);
    printf("cofactoring matrix:\n");
    print_matrix(matrix);
    for (int i=0; i<matrix->rows; i++) {

        if (matrix->cubes[i][column] != pc && matrix->cubes[i][column] != '-') continue;

        memcpy(temp_matrix->cubes[row], matrix->cubes[i], matrix->cols);

        temp_matrix->cubes[row][column] = '-';

        row++;
    }

    printf("new cofactor rows: %d\n", row);
    temp_matrix->rows = row;

    return temp_matrix;
}

matrix_t *check_complement(matrix_t *matrix)
{
    matrix_t *return_matrix  = (matrix_t*)malloc(sizeof(matrix_t));

    if (whole_row_of(matrix, '-') || matrix->rows == 0) {
        return_matrix->rows = 0;
        return_matrix->cols = 0;
        return_matrix->alloc_rows = 0;
        return return_matrix;
    }

    if (matrix->rows == 1) {

        int num_of_none_dashes = 0;
        for (int i=0; i<matrix->cols; i++) {
            if (matrix->cubes[0][i] == '0') num_of_none_dashes++;
            if (matrix->cubes[0][i] == '1') num_of_none_dashes++;
        }

        return_matrix->cols = matrix->cols;
        return_matrix->rows = num_of_none_dashes;
        return_matrix->alloc_rows = num_of_none_dashes;
        return_matrix->cubes = (char **)malloc(sizeof(char*));

        int row = 0;
        for (int i=0; i<matrix->cols; i++) {
            if (matrix->cubes[0][i] == '-') continue;

            return_matrix->cubes[row] = (char *)malloc(matrix->cols);
            memset(return_matrix->cubes[row], '-', matrix->cols);

            if (matrix->cubes[0][i] == '0') return_matrix->cubes[row][i] = '1';
            if (matrix->cubes[0][i] == '1') return_matrix->cubes[row][i] = '0';

            row++;
        }

        return return_matrix;
    }

    int binate_col = find_binate_column(matrix);

    printf("calling co_factor with matrix with rows: %d\n", matrix->rows);
    matrix_t *pos = check_complement(co_factor(matrix, binate_col, '1'));
    matrix_t *neg = check_complement(co_factor(matrix, binate_col, '0'));

    and(pos, binate_col, '1');
    and(neg, binate_col, '0');

    return_matrix = concat(pos, neg);

    //free_matrix(pos);
    //free_matrix(neg);

    return return_matrix;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: ./main inputfile\n");
        return 1;
    }

    FILE *fp = fopen((const char *)argv[1], "r");
    if (fp == NULL) {
        printf("Could not open file!\n");
        return 0;
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

    matrix_t *complements = check_complement(matrix);

    print_matrix(complements);

    //free_matrix(matrix);
    //free_matrix(complements);
}
