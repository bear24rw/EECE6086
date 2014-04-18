#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <getopt.h>
#include <pthread.h>
#include "shared.h"
#include "flags.h"

char flags_print_missing = 0;

unsigned int num_bits = 0;
unsigned int num_cubes = 0;
unsigned int num_flags = 0;
unsigned int num_flags_set = 0;

char *flag_bits;

void set_flag(unsigned int i)
{
    flag_bits[i >> 3] |= (1 << (i & 7));
}

char get_flag(unsigned int i)
{
    return flag_bits[i >> 3] & (1 << (i & 7));
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

char all_dash(char *vector)
{
    for (unsigned int i = 0; i < num_bits; i++) {
        if (vector[i] == '1') return 0;
        if (vector[i] == '0') return 0;
    }

    return 1;
}

void do_vector(char *vector)
{
    if (num_flags_set == num_flags) return;

    int num_dashes = 0;
    for (unsigned int bit = 0; bit < num_bits; bit++) {
        if (num_dashes > 0 && vector[bit] != '-') {
            num_dashes = 0;
            break;
        }

        if (vector[bit] == '-') num_dashes++;
    }

    if (num_dashes > 0) {
        unsigned int start_flag = 0;
        unsigned int end_flag = 0;

        for (unsigned int bit = 0; bit < num_bits; bit++) {
            if (vector[bit] == '1') {
                start_flag |= (1 << (num_bits - bit - 1));
                end_flag |= (1 << (num_bits - bit - 1));
            }
        }

        start_flag &= (~0) << num_dashes;
        end_flag |= ~((~0) << num_dashes);

        for (unsigned int i = start_flag; i <= end_flag; i++) {
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
    for (unsigned int bit = 0; bit < num_bits; bit++) {
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
        for (unsigned int bit = 0; bit < num_bits; bit++)
            if (local_vector[bit] == '1')
                flag_pos |= (1 << (num_bits - bit - 1));
        if (get_flag(flag_pos) == 0) {
            set_flag(flag_pos);
            num_flags_set++;
        }
    }

    free(local_vector);
}

void *flags(void *filename)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    FILE *fp = fopen((const char *)filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file!\n");
        pthread_cond_signal(&done_signal);
        return NULL;
    }

    fscanf(fp, "%d", &num_bits);
    fscanf(fp, "%d", &num_cubes);

    num_flags = 2 << (num_bits - 1);
    flag_bits = (char *)malloc(num_flags >> 3);
    memset(flag_bits, 0, num_flags >> 3);

    char *vector = (char *)malloc(num_bits);

    for (unsigned int cube = 0; cube < num_cubes; cube++) {
        fscanf(fp, "%s", vector);
        replace_under_with_dash(vector, num_bits);
        if (all_dash(vector)) {
            pthread_mutex_lock(&print_mutex);
            is_tautology = 1;
            break;
        }
        do_vector(vector);
    }

    if (flags_print_missing) {
        pthread_mutex_lock(&print_mutex);
        fprintf(stderr, "Flags is printing complements\n");
        int num_missing = 0;
        for (unsigned int i = 0; i < num_flags; i++) {
            if (get_flag(i) == 0) {
                print_binary(i);
                num_missing++;
            }
        }
        fprintf(stderr, "Number of missing covers: %d\n", num_missing);
    }

    if (num_flags_set >= num_flags) is_tautology = 1;

    fprintf(stderr, "Flags found it\n");
    pthread_cond_signal(&done_signal);

    return NULL;
}
