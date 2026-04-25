#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

// This thread will be created INSIDE the new child process
void* child_worker_thread(void* arg) {
    for (int i = 0; i < 3; ++i) {
        cout << "  -> [Child's Thread] Running concurrently inside the child process..." << endl;
        sleep(1);
    }
    return NULL;
}

int main() {
    cout << "--- PARENT: Starting process creation using fork() ---" << endl;

    // fork() creates an exact duplicate of the current process.
    // Unlike posix_spawn, it does NOT require an external executable file!
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "fork failed!" << endl;
        return 1;
    } 
    else if (pid == 0) {
        // =========================================================
        // CHILD PROCESS CODE (pid == 0 means we are the child)
        // =========================================================
        cout << "[Child Main] Child process started." << endl;
        
        pthread_t thread_id;
        cout << "[Child Main] Spawning a thread inside the child process..." << endl;
        
        // The child process creates its own thread
        pthread_create(&thread_id, NULL, child_worker_thread, NULL);
        
        // The child's main thread also does some work
        for (int i = 0; i < 3; ++i) {
            cout << "[Child Main] Doing some work in the child's main thread..." << endl;
            sleep(1);
        }

        // Wait for the child's worker thread to finish, then exit
        pthread_join(thread_id, NULL);
        cout << "[Child Main] Child process exiting." << endl;
        
        // Important: the child process must explicitly exit so it doesn't 
        // accidentally continue running the parent's code below!
        exit(0); 
    } 
    else {
        // =========================================================
        // PARENT PROCESS CODE (pid > 0 means we are the parent)
        // =========================================================
        cout << "PARENT: Child process spawned successfully with PID: " << pid << endl;
        
        // The parent waits for the child process to finish
        int wait_status;
        waitpid(pid, &wait_status, 0);
        
        cout << "PARENT: Child process finished executing." << endl;
    }

    return 0;
}

