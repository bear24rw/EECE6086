#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <getopt.h>
#include <pthread.h>
#include "main.h"

unsigned int num_bits = 0;
unsigned int num_cubes = 0;
unsigned int num_flags = 0;
unsigned int num_flags_set = 0;

char *flags;

inline void set_flag(unsigned int i) {
    flags[i>>3] |= (1 << (i&7));
}

inline char get_flag(unsigned int i) {
    return flags[i>>3] & (1 << (i&7));
}

void print_binary(int number)
{
    for (int i = num_bits - 1; i >= 0; i--) {
        if (number & (1 << i))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

static void replace_under_with_dash(char *vector)
{
    for (int i=0; i<num_bits; i++)
        if (vector[i] == '_') vector[i] = '-';
}

inline char all_dash(char *vector)
{
    for (int i=0; i<num_bits; i++) {
        if (vector[i] == '1') return 0;
        if (vector[i] == '0') return 0;
    }

    return 1;
}

void do_vector(char *vector)
{
    if (num_flags_set == num_flags) return;

    int num_dashes = 0;
    for (int bit = 0; bit < num_bits; bit++) {
        if (num_dashes > 0 && vector[bit] != '-') {
            num_dashes = 0;
            break;
        }

        if (vector[bit] == '-') num_dashes++;
    }

    if (num_dashes > 0) {
        unsigned int start_flag = 0;
        unsigned int end_flag = 0;

        for (int bit = 0; bit < num_bits; bit++) {
            if (vector[bit] == '1') {
                start_flag |= (1 << (num_bits - bit - 1));
                end_flag   |= (1 << (num_bits - bit - 1));
            }
        }

        start_flag &= (~0) << num_dashes;
        end_flag   |= ~((~0) << num_dashes);

        for (int i=start_flag; i<=end_flag; i++) {
            if (get_flag(i) == 0) {
                set_flag(i);
                num_flags_set++;
                if (num_flags_set == num_flags) return;
            }
        }

        return;
    }

    char *local_vector = (char *)malloc(num_bits);
    memcpy(local_vector, vector, num_bits);

    char has_dash = 0;
    for (int bit = 0; bit < num_bits; bit++) {
        if (vector[bit] == '-') {
            local_vector[bit] = '0';
            do_vector(local_vector);
            local_vector[bit] = '1';
            do_vector(local_vector);
            has_dash = 1;
            break;
        }
    }

    if (!has_dash) {
        unsigned int flag_pos = 0;
        for (int bit = 0; bit < num_bits; bit++)
            if (local_vector[bit] == '1')
                flag_pos |= (1 << (num_bits - bit - 1));
        if (get_flag(flag_pos) == 0) {
            set_flag(flag_pos);
            num_flags_set++;
        }
    }

    free(local_vector);
}

void *flag(void *filename)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

    char print_missing = 0;

    FILE *fp = fopen((const char *)filename, "r");
    if (fp == NULL) {
        printf("Could not open file!\n");
        pthread_cond_signal(&done_signal);
        return NULL;
    }

    fscanf(fp, "%d", &num_bits);
    fscanf(fp, "%d", &num_cubes);

    num_flags = 2 << (num_bits - 1);
    flags = (char *)malloc(num_flags>>3);
    memset(flags, 0, num_flags>>3);

    char *vector = (char *)malloc(num_bits);

    for (unsigned int cube = 0; cube < num_cubes; cube++) {
        fscanf(fp, "%s", vector);
        replace_under_with_dash(vector);
        if (all_dash(vector)) {
            printf("All cases covered\n");
            pthread_cond_signal(&done_signal);
            return NULL;
        }
        do_vector(vector);
    }

    if (num_flags_set >= num_flags) {
        printf("It is a tautology\n");
    } else {
        printf("It is NOT a tautology\n");
    }

    if (print_missing) {
        int num_missing = 0;
        for (unsigned int i = 0; i < num_flags; i++) {
            if (get_flag(i) == 0) {
                print_binary(i);
                num_missing++;
            }
        }
        printf("Number of missing covers: %d\n", num_missing);
    }

    printf("Flags found it\n");
    pthread_cond_signal(&done_signal);

    return NULL;
}
