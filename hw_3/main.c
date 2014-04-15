#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <getopt.h>
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
    int max_binate = 0;
    int binate_var = 0;
    int temp_binate = 0;

    for (int j=0; j<num_vars; j++) {
        for (int i=0; i<num_cubes; i++) {
            if (vector[i][j] == 1) true_form+=1;
            if (vector[i][j] == 0) comp_form+=1;
            printf("%c", vector[i][j]);
        }
        // check if polarity changes, if not then the variable is found to be
        // unate, continue checking if other variables are binate.
        if ((true_form >  0 && comp_form == 0) ||
            (true_form == 0 && comp_form >  0))
            continue;
        temp_binate = abs(true_form - comp_form);
        if (max_binate > temp_binate) {
            max_binate = temp_binate;
            binate_var = j;
        }
        printf("\n");
    }
    printf("Most binate variable is %d with a value of: %d\n", binate_var, max_binate);
    return binate_var;
}

//http://cc.ee.ntu.edu.tw/~jhjiang/instruction/courses/fall10-lsv/lec03-2_2p.pdf
//int check_tautology(char **vector, int binate_var, int p_flag)
//int count = 0;
//int binate_var = 0;
int check_tautology(char **vector, char **temp_minterms, int pc)
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
    //if (count)
    //if (temp_minterms == NULL) return 0;
    if (vector == NULL) return 0;

    // check to see if cube consists of all dashes, if so then we have a
    // tautology
    for (int i=0; i<num_cubes; i++) {
        //if (count) {
            //if (all_dash(temp_minterms[i])) {
            if (all_dash(vector[i])) {
                printf("Function is a tautololgy\n");
                return 1;
            }
        //}
    }

    // pick most binate variable to check for tautology
    /*
    if (count)
        binate_var = find_most_binate(temp_minterms);
    else {
        count = 1;
        binate_var = find_most_binate(vector);
    }
    */
    int binate_var = find_most_binate(vector);

    /*
    int positive_cofactor = cofactor(vector, 1);
    if (check_tautology(positive_cofactor) == False)
        return False

    int negative_cofactor = cofactor(vector, 0);
    if (check_tautology(negative_cofactor) == False)
        return False
    */

    // check for tautology on the positive cofactor of most binate variable
    if (pc) {
        for (int i=0; i<num_cubes; i++) {
            if (vector[i][binate_var] == '1') {
                printf("%c\n", vector[i][binate_var]);

                memcpy(temp_minterms[i], vector[i], num_vars*sizeof(char*));
                temp_minterms[i][binate_var] = '-';

                printf("%s\n", vector[i]);
                printf("%s\n", temp_minterms[i]);
                /*
                if (count) {
                    temp_minterms[i] = temp_minterms[i];
                    temp_minterms[i][binate_var] = '-';
                }else{
                    temp_minterms[i] = vector[i];
                    temp_minterms[i][binate_var] = '-';
                }
                */
            }
        }
        // would be nice to remove all "counts" from above and just have
        // temp_minterms overwrite the vector but i don't think i want to lose
        // 'vector' so...
        if (check_tautology(temp_minterms, temp_minterms, 1)) return 1;
        //if (check_tautology(vector, temp_minterms, 1)) return 1;
    }

    // check for tautology on the negative cofactor of most binate variable
    if (!pc) {
        for (int i=0; i<num_cubes; i++) {
            if (vector[i][binate_var] == '0') {
                printf("%c\n", vector[i][binate_var]);

                memcpy(temp_minterms[i], vector[i], num_vars*sizeof(char*));
                temp_minterms[i][binate_var] = '-';

                printf("%s\n", vector[i]);
                printf("%s\n", temp_minterms[i]);
                /*
                if (count) {
                    temp_minterms[i] = temp_minterms[i];
                    temp_minterms[i][binate_var] = '-';
                }else{
                    temp_minterms[i] = vector[i];
                    temp_minterms[i][binate_var] = '-';
                }
                */
            }
        }
        if (check_tautology(temp_minterms, temp_minterms, 0)) return 1;
    }
    /*
    */

    return 0;
}

int main(int argc, char *argv[])
{
    //start_time = std::clock();

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

    char **minterms      = (char **)malloc(num_cubes*sizeof(char*));
    char **temp_minterms = (char **)malloc(num_cubes*sizeof(char*));
    char **temp_vector   = (char **)malloc(num_cubes*sizeof(char*));

    for (int i=0; i<num_cubes; i++) {
        minterms[i]      = (char *)malloc(num_vars);
        temp_minterms[i] = (char *)malloc(num_vars);
        temp_vector[i]   = (char *)malloc(num_vars);
        //minterms[i] = (char *)malloc((num_vars+1)*sizeof(char));
    }

    for (int i=0; i<num_cubes; i++) {
        fscanf(fp, "%s", minterms[i]);
        replace_under_with_dash(minterms[i]);
        // TODO: might remove this since i'm doing it in the 'check tautology
        // func'
        if (all_dash(minterms[i])) {
            printf("Function is a tautololgy\n");
            return 0;
        }
    }

    for (int i=0; i<num_cubes; i++) {
        memcpy(temp_vector[i], minterms[i], num_vars*sizeof(char *));
    }

    //int positive_cofactor = check_tautology(minterms, temp_minterms, 1);
    int positive_cofactor = check_tautology(temp_vector, temp_minterms, 1);

    if (!positive_cofactor) {
        printf("Function is not a tautololgy\n");
        return 0;
    }

    // pretty sure by the time i pass in temp_minterms, it has already been modified....
    // i really don't want to make 100000000000 copies of the matrix, so yeah...
    int negative_cofactor = check_tautology(minterms, temp_minterms, 0);
    if (!negative_cofactor) {
        printf("Function is not a tautololgy\n");
        return 0;
    }

    printf("Function is a tautololgy\n");
    return 0;
}
