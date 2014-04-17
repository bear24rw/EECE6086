#ifndef __SHARED_H__
#define __SHARED_H__

typedef struct {
    char **cubes;
    int cols;
    int rows;
    int alloc_cols;
    int alloc_rows;
} matrix_t;

extern char is_tautology;

char whole_row_of(matrix_t *matrix, char value);
int max(int a, int b);
void free_matrix(matrix_t *matrix);
void print_matrix(matrix_t *matrix);
void replace_under_with_dash(char *cube, int num_cols);
matrix_t *co_factor(matrix_t *matrix, int column, char pc);

#endif
