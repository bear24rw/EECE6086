#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include "mem_log.h"
#include "shared.h"
#include "flags.h"
#include "tc_heur.h"

pthread_cond_t done_signal = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: ./tc [flags] inputfile\n");
        fprintf(stderr, "       -f  Only run flags algorithm\n");
        fprintf(stderr, "       -h  Only run heuristic algorithm\n");
        fprintf(stderr, "       -m  Log memory usage to /tmp/mem_log.txt\n");
        return 1;
    }

    char do_flag = 0;
    char do_heur = 0;
    char do_mem = 0;

    int c;
    while ((c = getopt(argc, argv, "hfmp")) != -1) {
        switch (c) {
            case 'f':
                do_flag = 1;
                break;
            case 'h':
                do_heur = 1;
                break;
            case 'm':
                do_mem = 1;
                break;
        }
    }

    if (!do_flag && !do_heur) {
        do_flag = 1;
        do_heur = 1;
    }

    fprintf(stderr, "Using algorithms: ");
    if (do_flag) fprintf(stderr, " flag");
    if (do_heur) fprintf(stderr, " heur");
    fprintf(stderr, "\n");

    pthread_t log_thread;
    pthread_t flag_thread;
    pthread_t heur_thread;

    if (do_mem)  pthread_create(&log_thread, NULL, &mem_log, NULL);
    if (do_flag) pthread_create(&flag_thread, NULL, &flags, (void *)argv[optind]);
    if (do_heur) pthread_create(&heur_thread, NULL, &heur, (void *)argv[optind]);

    pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_wait(&done_signal, &done_mutex);

    fprintf(stderr, "Waiting for threads to join\n");

    stop_log_thread = 1;

    if (do_mem) pthread_join(log_thread, NULL);

    if (do_flag) pthread_cancel(flag_thread);
    if (do_heur) pthread_cancel(heur_thread);

    if (do_flag) pthread_join(flag_thread, NULL);
    if (do_heur) pthread_join(heur_thread, NULL);

    if (is_tautology) {
        fprintf(stderr, "Function is a tautololgy\n");
        return 0;
    }

    fprintf(stderr, "Function is NOT a tautololgy\n");
    return 1;
}
