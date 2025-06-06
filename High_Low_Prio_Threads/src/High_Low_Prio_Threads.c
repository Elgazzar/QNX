#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>

// High-priority thread function
void* high_priority(void* arg) {
    while (1) {
    	struct timespec ts;
    	clock_gettime(CLOCK_REALTIME, &ts);
    	printf("🔥 High priority thread running at %ld.%09ld\n", ts.tv_sec, ts.tv_nsec);
        usleep(50000); // 50ms
    }
    return NULL;
}

// Low-priority thread function
void* low_priority(void* arg) {
    while (1) {
    	// Inside the thread function:
    	struct timespec ts;
    	clock_gettime(CLOCK_REALTIME, &ts);
    	printf("🛠️  Low priority thread running at %ld.%09ld\n", ts.tv_sec, ts.tv_nsec);
        usleep(50000); // 50ms
    }
    return NULL;
}

int main(void) {
    pthread_t tid_high, tid_low;
    pthread_attr_t attr_high, attr_low;
    struct sched_param param_high, param_low;

    // Initialize thread attributes
    pthread_attr_init(&attr_high);
    pthread_attr_init(&attr_low);

    // Set scheduling policy to Round-Robin
    pthread_attr_setschedpolicy(&attr_high, SCHED_FIFO);
    pthread_attr_setschedpolicy(&attr_low, SCHED_FIFO);

    // Use explicit scheduling (not inherited)
    pthread_attr_setinheritsched(&attr_high, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setinheritsched(&attr_low, PTHREAD_EXPLICIT_SCHED);

    // Set priorities (range is usually 1 to 99)
    param_high.sched_priority = 80;
    param_low.sched_priority = 40;

    pthread_attr_setschedparam(&attr_high, &param_high);
    pthread_attr_setschedparam(&attr_low, &param_low);

    // Create high-priority thread
    if (pthread_create(&tid_high, &attr_high, high_priority, NULL) != 0) {
        perror("Failed to create high priority thread");
        exit(1);
    }

    // Create low-priority thread
    if (pthread_create(&tid_low, &attr_low, low_priority, NULL) != 0) {
        perror("Failed to create low priority thread");
        exit(1);
    }
    // Keep the main thread alive forever
    pause(); // Or: while(1) sleep(1);

    return 0;
}
