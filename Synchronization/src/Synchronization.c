#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Shared global resource
int shared_counter = 0;

// Mutex to protect our shared resource
pthread_mutex_t counter_mutex;

// Worker thread function
void *worker_thread(void *arg) {
  int thread_id = *(int *)arg;

  for (int i = 0; i < 5; ++i) {

    // --- ENTER CRITICAL SECTION ---
    // We lock the mutex. If the other thread already has it locked,
    // this thread will pause here until it gets unlocked.
    pthread_mutex_lock(&counter_mutex);

    // Now we safely modify the shared resource
    int temp = shared_counter;

    // Simulate a small delay to make race conditions extremely likely
    // if we weren't using a mutex.
    usleep(10000); // 10 milliseconds

    shared_counter = temp + 1;
    printf("[Thread %d] Incremented counter to: %d\n", thread_id,
           shared_counter);

    // --- EXIT CRITICAL SECTION ---
    // We must unlock the mutex so the other thread can have a turn!
    pthread_mutex_unlock(&counter_mutex);

    // Sleep outside the critical section to give the other thread a fair chance
    // to grab the lock
    usleep(10000);
  }

  return NULL;
}

int main(void) {
  pthread_t t1, t2;
  int id1 = 1, id2 = 2;

  printf("--- Mutex Synchronization Example ---\n");

  // 1. Initialize the mutex before starting any threads
  if (pthread_mutex_init(&counter_mutex, NULL) != 0) {
    perror("Failed to initialize mutex");
    return EXIT_FAILURE;
  }

  // 2. Start two concurrent threads
  pthread_create(&t1, NULL, worker_thread, &id1);
  pthread_create(&t2, NULL, worker_thread, &id2);

  // 3. Wait for both threads to finish their work
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  // Since we used a mutex, the final counter will always perfectly equal 10.
  // Without a mutex, they would overwrite each other and the final result would
  // be unpredictable.
  printf("\nAll threads finished. Final counter value: %d (Expected: 10)\n",
         shared_counter);

  // 4. Destroy the mutex to clean up system resources
  pthread_mutex_destroy(&counter_mutex);

  return EXIT_SUCCESS;
}
