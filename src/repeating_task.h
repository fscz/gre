#ifndef gre_repeating_task_h
#define gre_repeating_task_h

#include <pthread.h>

typedef struct {
  int timer_fd;
  unsigned long long overruns;
} RepeatInfo;

typedef int (*TaskSetup) (void*);
typedef int (*TaskRepeat) (RepeatInfo*, void*);
typedef int (*TaskTearDown) (void*);

typedef struct {
  pthread_t thread;
  unsigned int intervalNs;
  int running;
  TaskSetup setup;
  TaskRepeat repeat;
  TaskTearDown tearDown;
  void* userData;
} TaskInfo;

int repeating_task_start(TaskInfo* taskInfo);
int repeating_task_stop(TaskInfo* taskInfo);

#endif
