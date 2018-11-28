/**
 * threadpool_test.c, copyright 2001 Steve Gribble
 *
 * Just a regression test for the threadpool code.
 */
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include "threadpool.h"
// #include "threadpool.cpp"

extern int errno;

void dispatch_to_me(void *arg) {
  // cout<<"in func"<<endl;
  int seconds = reinterpret_cast<intptr_t>(arg);
  // cout<<seconds<<endl;;
  // cout<<type(seconds)<endl;
  fprintf(stdout, "  in dispatch %d\n", seconds);

  sleep(seconds);
  fprintf(stdout, "  done dispatch %d\n", seconds);
}

int main(int argc, char **argv) {
  threadpool tp;

  tp = create_threadpool(10);
  // dispatch_to_me((void *)3);
  fprintf(stdout, "**main** dispatch 3\n");
  dispatch(tp, dispatch_to_me, (void *) 3);
  fprintf(stdout, "**main** dispatch 6\n");
  dispatch(tp, dispatch_to_me, (void *) 6);
  fprintf(stdout, "**main** dispatch 7\n");
  dispatch(tp, dispatch_to_me, (void *) 7);
fprintf(stdout, "**main** dispatch 5\n");
  dispatch(tp, dispatch_to_me, (void *) 5);
  fprintf(stdout, "**main** dispatch 8\n");
  dispatch(tp, dispatch_to_me, (void *) 8);
  fprintf(stdout, "**main** dispatch 2\n");
  dispatch(tp, dispatch_to_me, (void *) 2);
    fprintf(stdout, "**main** dispatch 10\n");
  dispatch(tp, dispatch_to_me, (void *) 10);
  fprintf(stdout, "**main** dispatch 4\n");
  dispatch(tp, dispatch_to_me, (void *) 4);
  fprintf(stdout, "**main** dispatch 9\n");
  dispatch(tp, dispatch_to_me, (void *) 9);
  fprintf(stdout, "**main** done first\n");

  sleep(50);
  fprintf(stdout, "\n\n");
  destroy_threadpool(tp);

  fprintf(stdout, "**main** dispatch 3\n");
  dispatch(tp, dispatch_to_me, (void *) 3);
  fprintf(stdout, "**main** dispatch 6\n");
  dispatch(tp, dispatch_to_me, (void *) 6);
  fprintf(stdout, "**main** dispatch 7\n");
  dispatch(tp, dispatch_to_me, (void *) 7);

  fprintf(stdout, "**main done second\n");
  sleep(20);

  // sleep(20);
  exit(-1);
}

