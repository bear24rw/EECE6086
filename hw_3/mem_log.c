#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include "mem_log.h"

char stop_log_thread = 0;

FILE *fp_mem;
FILE *fp_dep;

long heap_size = 0;
int recursion_depth = 0;

unsigned long start_time = -1;

void open_log(void)
{
    fp_mem = fopen("/tmp/mem.log", "w");
    fp_dep = fopen("/tmp/dep.log", "w");

    struct timeval tv;
    gettimeofday(&tv, NULL);
    start_time = 1000000 * tv.tv_sec + tv.tv_usec;
}

void close_log(void)
{
    fclose(fp_mem);
    fclose(fp_dep);
}

double elapsed_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long microseconds = 1000000 * tv.tv_sec + tv.tv_usec;
    return (double)(microseconds - start_time) / (double)1000000;
}

void write_log(void) {
    static long last_heap_size = -1;
    static int last_depth = -1;

    if (recursion_depth != last_depth) {
        last_depth = recursion_depth;
        fprintf(fp_dep, "%f %d\n", elapsed_time(), recursion_depth);
    }

    if (heap_size != last_heap_size) {
        last_heap_size = heap_size;
        fprintf(fp_mem, "%f %f\n", elapsed_time(), (double)heap_size/(double)1024);
    }
}

void *mem_log()
{
    open_log();

    do {
        write_log();
    } while (!stop_log_thread);

    write_log();

    close_log();

    return NULL;
}
