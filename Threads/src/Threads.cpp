#include <iostream>
using namespace std;

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syspage.h>
#include <time.h>
#include <unistd.h>

int num_cpus;

static void *worker(void *arg) {
  unsigned long last = clock_gettime_mon_ns();
  for (;;) {
    sleep(1);
    unsigned long now = clock_gettime_mon_ns();
    printf("Low Priority Task Slept for %lums\n", (now - last) / 1000000UL);
    last = now;
  }
  return NULL;
}

static void *high_priority(void *arg) {
  unsigned long last = clock_gettime_mon_ns();
  for (;;) {
    sleep(1);
    unsigned long now = clock_gettime_mon_ns();
    printf("High Priority Task Slept for %lums\n", (now - last) / 1000000UL);
    printf("Number of CPUS is  %d\n", num_cpus);
    last = now;
  }
  return NULL;
}

int main(int argc, char **argv) {

  num_cpus = _syspage_ptr->num_cpu;
  // Attribute structure for a high-priority thread.
  pthread_attr_t high_priority_attr;
  pthread_attr_t low_priority_attr;

  pthread_attr_init(&high_priority_attr);
  pthread_attr_setinheritsched(&high_priority_attr, PTHREAD_EXPLICIT_SCHED);

  pthread_attr_init(&low_priority_attr);
  pthread_attr_setinheritsched(&low_priority_attr, PTHREAD_EXPLICIT_SCHED);

  struct sched_param sched_high = {.sched_priority = 63};
  struct sched_param sched_low = {.sched_priority = 25};
  pthread_attr_setschedparam(&high_priority_attr, &sched_high);
  pthread_attr_setschedparam(&low_priority_attr, &sched_low);

  // Create the high-priority thread.
  pthread_t tids[11];
  int rc;
  rc = pthread_create(&tids[0], &high_priority_attr, high_priority, NULL);
  // rc = pthread_create(&tids[0], NULL, high_priority, NULL);
  if (rc != 0) {
    fprintf(stderr, "pthread_create: %s\n", strerror(rc));
    return EXIT_FAILURE;
  }

  // Create worker threads.
  rc = pthread_create(&tids[1], &low_priority_attr, worker, NULL);
  if (rc != 0) {
    fprintf(stderr, "pthread_create: %s\n", strerror(rc));
    return EXIT_FAILURE;
  }

  // Wait for workers to finish.
  pthread_join(tids[0], NULL);
  pthread_join(tids[1], NULL);

  return EXIT_SUCCESS;
}
