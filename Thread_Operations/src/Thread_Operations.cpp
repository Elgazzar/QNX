#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

// ---------------------------------------------------------
// Thread 1: Normal joinable thread
// ---------------------------------------------------------
void* thread1_func(void* arg) {
    // 1. pthread_self: Get our own thread ID
    pthread_t my_id = pthread_self();
    
    // 2. pthread_setname_np: Give this thread a human-readable name for debugging
    pthread_setname_np(my_id, "Worker-Joinable");
    
    cout << "[Thread 1] Started. My ID is: " << my_id << endl;
    cout << "[Thread 1] Doing some work for 2 seconds..." << endl;
    sleep(2);
    
    cout << "[Thread 1] Exiting gracefully." << endl;
    
    // 3. pthread_exit: Exit the thread and return a status value (e.g., 42)
    pthread_exit((void*)42); 
    
    return NULL; // This line is never reached
}

// ---------------------------------------------------------
// Thread 2: Detached thread
// ---------------------------------------------------------
void* thread2_func(void* arg) {
    pthread_setname_np(pthread_self(), "Worker-Detached");
    
    // 4. pthread_detach: Tell the OS to automatically clean up this thread's 
    // resources as soon as it finishes. No pthread_join needed!
    pthread_detach(pthread_self());
    
    cout << "[Thread 2] I am detached! I will clean up my own resources." << endl;
    sleep(1);
    cout << "[Thread 2] Finishing up." << endl;
    
    return NULL;
}

// ---------------------------------------------------------
// Thread 3: Thread to be cancelled
// ---------------------------------------------------------
void* thread3_func(void* arg) {
    pthread_setname_np(pthread_self(), "Worker-Canceled");
    
    cout << "[Thread 3] Running in an infinite loop until cancelled..." << endl;
    while(true) {
        cout << "[Thread 3] Still alive..." << endl;
        sleep(1); // sleep is a valid cancellation point
    }
    return NULL;
}

// =========================================================
// MAIN ENTRY POINT
// =========================================================
int main() {
    pthread_t t1, t2, t3;

    cout << "[Main] Creating threads..." << endl;
    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_create(&t2, NULL, thread2_func, NULL);
    pthread_create(&t3, NULL, thread3_func, NULL);

    // Give threads a moment to start up
    sleep(1); 

    // 5. pthread_kill: Sending signal '0' doesn't actually send a signal, 
    // but it allows us to check if the target thread is still alive.
    int kill_status = pthread_kill(t1, 0);
    if (kill_status == 0) {
        cout << "[Main] Checked using pthread_kill(0): Thread 1 is confirmed alive." << endl;
    }

    // 6. pthread_cancel: Forcefully request Thread 3 to terminate.
    cout << "[Main] Canceling Thread 3..." << endl;
    pthread_cancel(t3); 

    // 7. pthread_join: Wait for a thread to finish and retrieve its exit status
    void* retval;
    cout << "[Main] Waiting for Thread 1 to finish (joining)..." << endl;
    pthread_join(t1, &retval);
    cout << "[Main] Thread 1 successfully joined! Exit status was: " << (long)retval << endl;

    // NOTE: We MUST NOT join Thread 2 because we detached it! 

    // We join Thread 3 to clean up its cancellation status
    void* cancel_retval;
    pthread_join(t3, &cancel_retval);
    if (cancel_retval == PTHREAD_CANCELED) {
        cout << "[Main] Thread 3 was successfully verified as cancelled." << endl;
    }

    cout << "[Main] All operations completed. Main program exiting." << endl;
    return 0;
}
