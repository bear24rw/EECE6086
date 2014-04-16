#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <getopt.h>
#include "main.h"

pthread_cond_t  done_signal  = PTHREAD_COND_INITIALIZER;

char done = 0;

double mem_usage(void) {
    int tSize = 0, resident = 0, share = 0;

    FILE *fp = fopen("/proc/self/statm", "r");
    fscanf(fp, "%d %d %d", &tSize, &resident, &share);
    fclose(fp);

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    double rss = resident * page_size_kb;
    double shared_mem = share * page_size_kb;

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

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: ./main inputfile\n");
        return 1;
    }

    char do_flag = 0;
    char do_heur = 0;

    int c;
    while ((c = getopt(argc, argv, "hf")) != -1) {
        switch (c) {
            case 'f': do_flag = 1; break;
            case 'h': do_heur = 1; break;
        }
    }

    if (!do_flag && !do_heur) {
        do_flag = 1;
        do_heur = 1;
    }

    if (do_flag && do_heur) printf("Algorithms: flag heur\n");
    else if (do_flag)       printf("Algorithms: flag\n");
    else if (do_heur)       printf("Algorithms: heur\n");

    pthread_t log_thread;
    pthread_t flag_thread;
    pthread_t heur_thread;

    pthread_create(&log_thread, NULL, &mem_log, NULL);
    if (do_flag) pthread_create(&flag_thread, NULL, &flag, (void *)argv[optind]);
    if (do_heur) pthread_create(&heur_thread, NULL, &heur, (void *)argv[optind]);

    pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_wait(&done_signal, &done_mutex);

    printf("--- Waiting for threads to join ---\n");

    done = 1;

    if (do_flag) pthread_cancel(flag_thread);
    if (do_heur) pthread_cancel(heur_thread);

    if (do_flag) pthread_join(flag_thread, NULL);
    if (do_heur) pthread_join(heur_thread, NULL);
    pthread_join(log_thread, NULL);

    return 0;
}
