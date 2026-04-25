#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

// Shared resources
int item_count = 0;
pthread_mutex_t my_mutex;
pthread_cond_t my_cond;

// Consumer Thread Function
void* consumer(void* arg) {
    int id = *(int*)arg;
    
    cout << "[Consumer " << id << "] Started. Trying to acquire mutex..." << endl;
    pthread_mutex_lock(&my_mutex);
    
    // CRITICAL: Always use a 'while' loop to check the condition.
    // This protects against "spurious wakeups" where the OS wakes the thread by accident.
    while (item_count == 0) {
        cout << "[Consumer " << id << "] No items available. Sleeping on condition variable..." << endl;
        
        // pthread_cond_wait does TWO things atomically:
        // 1. Unlocks 'my_mutex' so the Producer can acquire it.
        // 2. Puts this thread to sleep until someone signals 'my_cond'.
        // When it wakes up, it automatically re-locks 'my_mutex' before continuing!
        pthread_cond_wait(&my_cond, &my_mutex);
    }
    
    // If we reach here, we hold the mutex AND item_count > 0.
    item_count--;
    cout << "[Consumer " << id << "] Woke up and successfully consumed an item! Items remaining: " << item_count << endl;
    
    pthread_mutex_unlock(&my_mutex);
    return NULL;
}

// Producer Thread Function
void* producer(void* arg) {
    cout << "[Producer] Working hard to produce items (takes 2 seconds)..." << endl;
    sleep(2); 
    
    // We must acquire the mutex before modifying the shared state
    pthread_mutex_lock(&my_mutex);
    
    item_count += 2; // Produce enough items for both consumers
    cout << "[Producer] Produced 2 items! Currently available: " << item_count << endl;
    
    // Wake up ALL threads that are currently sleeping on 'my_cond'.
    // If we only wanted to wake up ONE thread, we would use pthread_cond_signal.
    cout << "[Producer] Broadcasting condition variable to wake up all consumers!" << endl;
    pthread_cond_broadcast(&my_cond);
    
    // We unlock the mutex. The awakened consumers are waiting for this 
    // mutex, so they will immediately start competing for it.
    pthread_mutex_unlock(&my_mutex);
    return NULL;
}

int main() {
    pthread_t c1, c2, p;
    int id1 = 1, id2 = 2;

    cout << "--- Condition Variable Example (Producer/Consumer) ---" << endl;

    // 1. Initialize Mutex and Condition Variable
    pthread_mutex_init(&my_mutex, NULL);
    pthread_cond_init(&my_cond, NULL);

    // 2. Start Consumer threads first (they will find no items and go to sleep)
    pthread_create(&c1, NULL, consumer, &id1);
    pthread_create(&c2, NULL, consumer, &id2);
    
    // Give consumers a split second to lock the mutex and go to sleep
    usleep(100000); 

    // 3. Start Producer thread
    pthread_create(&p, NULL, producer, NULL);

    // Wait for all threads to finish
    pthread_join(c1, NULL);
    pthread_join(c2, NULL);
    pthread_join(p, NULL);

    // 4. Destroy Mutex and Condition Variable
    pthread_cond_destroy(&my_cond);
    pthread_mutex_destroy(&my_mutex);

    cout << "--- All threads finished successfully! ---" << endl;
    return 0;
}
