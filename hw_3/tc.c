#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <getopt.h>

unsigned int num_bits = 0;
unsigned int num_cubes = 0;
unsigned int num_flags = 0;
unsigned int num_flags_set = 0;

char *flags;

void print_binary(int number)
{
    for (int i = num_bits-1; i >= 0; i--) {
        if (number & (1 << i))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

void do_vector(char *vector)
{
    if (num_flags_set == num_flags) return;

    char *local_vector = (char *)malloc(num_bits);
    memcpy(local_vector, vector, num_bits);

    char has_dash = 0;
    for (int bit=0; bit<num_bits; bit++) {
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
        for (int bit=0; bit<num_bits; bit++)
            if (local_vector[bit] == '1')
                flag_pos |= (1 << (num_bits-bit-1));
        if (flags[flag_pos] == 0) {
            flags[flag_pos] = 1;
            num_flags_set++;
        }
    }

    free(local_vector);
}

int main(int argc,char **argv)
{
    struct timeval stop, start;
    gettimeofday(&start, NULL);

    char print_missing = getopt(argc, argv, "c") == 'c';

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Could not open file!\n");
        return 1;
    }

    fscanf(fp, "%d", &num_bits);
    fscanf(fp, "%d", &num_cubes);

    num_flags = 2<<(num_bits-1);
    flags = (char *)malloc(num_flags);
    memset(flags, 0, num_flags);

    char *all_dash = (char *)malloc(num_bits+1);
    memset(all_dash, '-', num_bits);
    all_dash[num_bits+1] = '\0';

    char *vector = (char *)malloc(num_bits+1);

    for (unsigned int cube=0; cube<num_cubes; cube++) {
        fscanf(fp, "%s", vector);
        vector[num_bits+1] = '\0';
        if (strcmp(vector, all_dash) == 0) {
            printf("All cases covered\n");
            return 0;
        }
        do_vector(vector);
    }

    int num_missing = 0;
    for (unsigned int i=0; i<num_flags; i++) {
        if (flags[i] == 0) {
            if (print_missing) print_binary(i);
            num_missing++;
        }
    }
    printf("Number of missing covers: %d\n", num_missing);

    gettimeofday(&stop, NULL);
    printf("Took %ldus\n", stop.tv_usec - start.tv_usec);

    return 0;
}
