#include <sys/timerfd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "repeating_task.h"

#define CLOCKID CLOCK_MONOTONIC

static void make_periodic (unsigned int period_ns, RepeatInfo* info)
{
  struct itimerspec itval;
  int fd;

  fd = timerfd_create (CLOCKID, 0);
  assert (fd > -1);
  info->overruns = 0;
  info->timer_fd = fd;

  /* Make the timer periodic */
  itval.it_interval.tv_sec = period_ns / 1000000000;
  itval.it_interval.tv_nsec = period_ns % 1000000000;
  itval.it_value.tv_sec = itval.it_interval.tv_sec;
  itval.it_value.tv_nsec = itval.it_interval.tv_nsec;
  assert(timerfd_settime (fd, 0, &itval, NULL) == 0);
}

static void wait_period (RepeatInfo* info) {
  int ret;

  /* Wait for the next timer event. If we have missed any the
     number is written to "missed" */
  ret = read (info->timer_fd, &info->overruns, sizeof (info->overruns));
  if (ret == -1) {
    perror ("read timer");
    return;
  }
  info->overruns -= 1;
}

static int repeating_task_loop(void* data) {
  RepeatInfo repeatInfo;
  TaskInfo* taskInfo = (TaskInfo*)data;
  int result;

  if (taskInfo->setup) {
    result = taskInfo->setup(taskInfo->userData);
    if (result != 0) return -1;
  }

  make_periodic (taskInfo->intervalNs, &repeatInfo);
  while ( 1 == taskInfo->running) {
    result = taskInfo->repeat(&repeatInfo, taskInfo->userData);
    if (result != 0) break;
    wait_period (&repeatInfo);
  }

  if (taskInfo->tearDown) {
    result = taskInfo->tearDown(taskInfo->userData);
    if (result != 0) return -1;
  }

  close ( repeatInfo.timer_fd );
  
  return 0;
}

int repeating_task_start(TaskInfo* taskInfo) {
  taskInfo->running = 1;
  return repeating_task_loop((void*)taskInfo);  
}

int repeating_task_stop(TaskInfo* taskInfo) {
  taskInfo->running = 0;
  return 0;
}
