#ifndef __MAIN_H__
#define __MAIN_H__

#include <pthread.h>

extern char print_missing;
extern pthread_cond_t done_signal;
void *heur(void *filename);
void *flag(void *filename);

#endif
