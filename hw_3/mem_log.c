#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include "mem_log.h"

char stop_mem_log = 0;

double mem_usage(void)
{
    int tSize = 0, resident = 0, share = 0;

    FILE *fp = fopen("/proc/self/statm", "r");
    fscanf(fp, "%d %d %d", &tSize, &resident, &share);
    fclose(fp);

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
    double rss = resident * page_size_kb;
    double shared_mem = share * page_size_kb;

    return rss - shared_mem;
}

void *mem_log()
{
    FILE *fp = fopen("/tmp/mem_log.txt", "w");
    struct timeval tv;

    gettimeofday(&tv, NULL);
    unsigned long start = 1000000 * tv.tv_sec + tv.tv_usec;

    double last_mem = 0;
    do {
        double mem = mem_usage();
        if (mem != last_mem) {
            gettimeofday(&tv, NULL);
            unsigned long microseconds = 1000000 * tv.tv_sec + tv.tv_usec;
            fprintf(fp, "%f %f\n", (double)(microseconds - start) / (double)1000000, mem);
            last_mem = mem;
        }
    } while (!stop_mem_log);

    double mem = mem_usage();
    gettimeofday(&tv, NULL);
    unsigned long microseconds = 1000000 * tv.tv_sec + tv.tv_usec;
    fprintf(fp, "%f %f\n", (double)(microseconds - start) / (double)1000000, mem);

    fclose(fp);

    return NULL;
}
