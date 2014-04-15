#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <getopt.h>
#include <limits.h>
//#include "main.h"

int num_vars;
int num_cubes;

//clock_t start_time;
//clock_t place_time;
//clock_t route_time;
//clock_t end_time;

void replace_under_with_dash(char *vector)
{
    for (int i=0; i<num_vars; i++)
        if (vector[i] == '_') vector[i] = '-';
}

inline char all_dash(char *vector)
{
    for (int i=0; i<num_vars; i++) {
        if (vector[i] == '1') return 0;
        if (vector[i] == '0') return 0;
    }

    return 1;
}

int find_most_binate(char **vector)
{
    int comp_form = 0;
    int true_form = 0;
    int min_column = 0;
    int min_difference = INT_MAX;

    for (int j=0; j<num_vars; j++) {

        true_form = 0;
        comp_form = 0;

        // total up the number of 1s and 0s in this column
        for (int i=0; i<num_cubes && vector[i] != NULL; i++) {
            if (vector[i][j] == '1') true_form++;
            if (vector[i][j] == '0') comp_form++;
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

char **co_factor(char **vector, int column, char pc)
{
    char **temp_vector = (char **)malloc(num_cubes*sizeof(char*));

    for (int i=0; i<num_cubes; i++) {
        temp_vector[i] = (char *)malloc(num_vars);
    }

    int row = 0;

    for (int i=0; i<num_cubes && vector[i] != NULL; i++) {

        if (vector[i][column] != pc && vector[i][column] != '-') continue;

        memcpy(temp_vector[row], vector[i], num_vars*sizeof(char));

        temp_vector[row][column] = '-';

        row++;
    }

    //printf("co_factor rows: %d column: %d pc: %c\n", row, column, pc);
    temp_vector[row] = NULL;

    return temp_vector;
}

//http://cc.ee.ntu.edu.tw/~jhjiang/instruction/courses/fall10-lsv/lec03-2_2p.pdf
int check_tautology(char **vector)
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
    if (vector[0] == NULL) return 0;

    // check to see if cube consists of all dashes, if so then we have a
    // tautology
    for (int i=0; i<num_cubes && vector[i] != NULL; i++) {
        if (all_dash(vector[i])) {
            return 1;
        }
    }

    // pick most binate variable to check for tautology
    int binate_var = find_most_binate(vector);

    char **C0 = co_factor(vector, binate_var, '0');
    if (!check_tautology(C0)){
        return 0;
    }

    char **C1 = co_factor(vector, binate_var, '1');
    if (!check_tautology(C1)){
        return 0;
    }

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

    fscanf(fp, "%d", &num_vars);
    fscanf(fp, "%d", &num_cubes);

    printf("Found %d variables and %d cubes\n", num_vars, num_cubes);

    char **vector = (char **)malloc(num_cubes*sizeof(char*));

    for (int i=0; i<num_cubes; i++) {
        vector[i] = (char *)malloc(num_vars);
    }

    for (int i=0; i<num_cubes; i++) {
        fscanf(fp, "%s", vector[i]);
        replace_under_with_dash(vector[i]);
        if (all_dash(vector[i])) {
            printf("Function is a tautololgy\n");
            return 0;
        }
    }

    if (check_tautology(vector)) {
        printf("Function is a tautololgy\n");
    } else {
        printf("Function is NOT a tautololgy\n");
    }

    return 0;
}
