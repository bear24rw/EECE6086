#ifndef __MEM_LOG__
#define __MEM_LOG__

extern char stop_log_thread;
extern long heap_size;
extern int recursion_depth;

void open_log(void);
void close_log(void);
void write_log(void);
void *mem_log();

#endif
