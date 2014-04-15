#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

char done = 0;

typedef struct {
    char **cubes;
    int cols;
    int rows;
    int alloc_cols;
    int alloc_rows;
} matrix_t;

double mem_usage(void) {
    int tSize = 0, resident = 0, share = 0;

    FILE *fp = fopen("/proc/self/statm", "r");
    fscanf(fp, "%d %d %d", &tSize, &resident, &share);
    fclose(fp);

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    double rss = resident * page_size_kb;
    double shared_mem = share * page_size_kb;

    //printf("Memory used: %f kB\n", rss-shared_mem);

    return rss-shared_mem;
}

void *mem_log() {
    FILE *fp = fopen("/tmp/mem_log.txt", "w");
    struct timeval tv;

    gettimeofday(&tv, NULL);
    unsigned long start = 1000000*tv.tv_sec+tv.tv_usec;

    double last_mem = 0;
    do {
        double mem = mem_usage();
        if (mem != last_mem) {
            gettimeofday(&tv, NULL);
            unsigned long microseconds = 1000000*tv.tv_sec+tv.tv_usec;
            fprintf(fp, "%ld %f\n", microseconds-start, mem);
            last_mem = mem;
        }
    } while (!done);

    fclose(fp);

    return NULL;
}

void free_matrix(matrix_t *matrix) {
    for (int i=0; i < matrix->alloc_rows; i++)
        free(matrix->cubes[i]);
    free(matrix->cubes);
    free(matrix);
}

void replace_under_with_dash(char *cube, int num_cols)
{
    for (int i=0; i<num_cols; i++)
        if (cube[i] == '_') cube[i] = '-';
}

inline char all_dash(matrix_t *matrix)
{
    char has_all_dash = 0;

    for (int i=0; i < matrix->rows; i++) {

        has_all_dash = 1;
        for (int j=0; j < matrix->cols; j++) {
            if (matrix->cubes[i][j] == '1') has_all_dash = 0;
            if (matrix->cubes[i][j] == '0') has_all_dash = 0;
        }
        if (has_all_dash) break;
    }

    return has_all_dash;
}

int find_most_binate(matrix_t *matrix)
{
    int comp_form = 0;
    int true_form = 0;
    int min_column = 0;
    int min_difference = INT_MAX;

    for (int j=0; j<matrix->cols; j++) {

        true_form = 0;
        comp_form = 0;

        // total up the number of 1s and 0s in this column
        for (int i=0; i<matrix->rows; i++) {
            if (matrix->cubes[i][j] == '1') true_form++;
            if (matrix->cubes[i][j] == '0') comp_form++;
        }

        if (true_form == 0 || comp_form == 0)
            continue;

        int difference = abs(true_form - comp_form);
        if (difference < min_difference) {
            min_difference = difference;
            min_column = j;
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

matrix_t *unate_reduction(matrix_t *matrix)
{
    matrix_t *temp_matrix = (matrix_t *)malloc(sizeof(matrix_t));

    temp_matrix->rows = matrix->rows;
    temp_matrix->cols = matrix->cols;
    temp_matrix->alloc_rows = matrix->rows;
    temp_matrix->cubes = (char **)malloc(matrix->rows*sizeof(char*));

    for (int i=0; i<matrix->rows; i++) {
        temp_matrix->cubes[i] = (char *)malloc(matrix->cols);
    }

    //
    // Find the unate columns
    //

    char unate_columns[matrix->cols];

    int comp_form = 0;
    int true_form = 0;

    for (int j=0; j<matrix->cols; j++) {

        true_form = 0;
        comp_form = 0;

        // total up the number of 1s and 0s in this column
        for (int i=0; i<matrix->rows; i++) {
            if (matrix->cubes[i][j] == '1') true_form++;
            if (matrix->cubes[i][j] == '0') comp_form++;
        }

        if ((true_form == 0 && comp_form >= 0) ||
            (comp_form == 0 && true_form >= 0)) {
            unate_columns[j] = 1;
        } else {
            unate_columns[j] = 0;
        }
    }

    //
    // Figure out what rows to keep
    //

    char keep_rows[matrix->rows];

    for (int i=0; i<matrix->rows; i++) {

        // assume all unate columns have dash unless proven otherwise
        char all_dash = 1;

        // check that all unate columns in this row are '-'
        for (int j=0; j<matrix->cols; j++) {
            if (!unate_columns[j]) continue;
            if (matrix->cubes[i][j] != '-')
                all_dash = 0;
        }

        if (all_dash)
            keep_rows[i] = 1;
        else
            keep_rows[i] = 0;
    }

    //
    //
    //

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

    temp_matrix->rows = row;
    temp_matrix->cols = col;

    return temp_matrix;
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

    // check to see if cube consists of all dashes, if so then we have a
    // tautology
    if (all_dash(matrix)) return 1;

    if (matrix->rows == 1 && !all_dash(matrix))
        return 0;

    matrix_t *reduced_matrix = unate_reduction(matrix);

    // pick most binate variable to check for tautology
    int binate_var = find_most_binate(reduced_matrix);

    matrix_t *C0 = co_factor(reduced_matrix, binate_var, '0');
    if (!check_tautology(C0)){
        free_matrix(reduced_matrix);
        free_matrix(C0);
        return 0;
    }

    matrix_t *C1 = co_factor(reduced_matrix, binate_var, '1');
    if (!check_tautology(C1)){
        free_matrix(reduced_matrix);
        free_matrix(C1);
        return 0;
    }

    free_matrix(C0);
    free_matrix(C1);
    free_matrix(reduced_matrix);

    return 1;
}

int main(int argc, char *argv[])
{
    //
    // Get number of variables and cubes
    //

    if (argc < 2) {
        printf("Usage: ./main inputfile\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Could not open file!\n");
        return 1;
    }

    pthread_t log_thread;
    pthread_create(&log_thread, NULL, &mem_log, NULL);

    matrix_t *matrix = (matrix_t *)malloc(sizeof(matrix_t));

    fscanf(fp, "%d", &matrix->cols);
    fscanf(fp, "%d", &matrix->rows);

    printf("Found %d variables and %d cubes\n", matrix->cols, matrix->rows);

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

    done = 1;

    pthread_join(log_thread, NULL);

    return 0;
}
