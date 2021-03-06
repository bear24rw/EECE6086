#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "shared.h"
#include "mem_log.h"

char is_tautology = 0;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void free_matrix(matrix_t *matrix)
{
    heap_size -= matrix->cols * matrix->alloc_rows;
    heap_size -= matrix->alloc_rows * sizeof(char*);
    heap_size -= sizeof(matrix_t);

    for (int i = 0; i < matrix->alloc_rows; i++) {
        free(matrix->cubes[i]);
    }
    free(matrix->cubes);
    free(matrix);
}

matrix_t *alloc_matrix(int rows, int cols)
{
    heap_size += sizeof(matrix_t);
    heap_size += rows * sizeof(char*);
    heap_size += rows * cols;

    matrix_t *m = (matrix_t *)malloc(sizeof(matrix_t));
    m->rows = rows;
    m->cols = cols;
    m->alloc_rows = rows;
    m->cubes = (char **)malloc(rows * sizeof(char *));
    for (int i = 0; i < rows; i++) {
        m->cubes[i] = (char *)malloc(cols);
    }
    return m;
}


void print_matrix(matrix_t *matrix)
{
    for (int y = 0; y < matrix->rows; y++) {
        for (int x = 0; x < matrix->cols; x++) {
            printf("%c", matrix->cubes[y][x]);
        }
        printf("\n");
    }
}

void replace_under_with_dash(char *cube, int num_cols)
{
    for (int i = 0; i < num_cols; i++)
        if (cube[i] == '_') cube[i] = '-';
}

char whole_row_of(matrix_t *matrix, char value)
{
    char has_whole_row = 0;

    for (int i = 0; i < matrix->rows; i++) {

        has_whole_row = 1;
        for (int j = 0; j < matrix->cols; j++) {
            if (matrix->cubes[i][j] != value) has_whole_row = 0;
        }
        if (has_whole_row) break;
    }

    return has_whole_row;
}

matrix_t *co_factor(matrix_t *matrix, int column, char pc)
{
    matrix_t *temp_matrix = alloc_matrix(matrix->rows, matrix->cols);

    int row = 0;

    for (int i = 0; i < matrix->rows; i++) {

        if (matrix->cubes[i][column] != pc && matrix->cubes[i][column] != '-')
            continue;

        memcpy(temp_matrix->cubes[row], matrix->cubes[i], matrix->cols);

        temp_matrix->cubes[row][column] = '-';

        row++;
    }

    temp_matrix->rows = row;

    return temp_matrix;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

