#include <iostream>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h> // Added for threads

using namespace std;

// Access the global environment variables of the current process
extern char **environ;

// A simple thread function that runs in the background of our parent process
void* background_thread(void* arg) {
    for (int i = 0; i < 3; ++i) {
        cout << "[Parent's Thread] Doing some concurrent work..." << endl;
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t thread_id;
    pid_t child_pid;
    
    cout << "--- Starting a background thread in the parent process ---" << endl;
    // 1. Create a background thread inside our current C++ program
    pthread_create(&thread_id, NULL, background_thread, NULL);

    // Command to execute: 'ls -l'
    char* argv[] = { (char*)"ls", (char*)"-l", NULL };

    cout << "--- Spawning a separate child process ---" << endl;

    // 2. Create a completely separate process to run the 'ls' command
    int status = posix_spawnp(&child_pid, "ls", NULL, NULL, argv, environ);

    if (status == 0) {
        cout << "Child process spawned successfully with PID: " << child_pid << endl;
        
        // 3. The parent process waits for the external 'ls' process to finish
        int wait_status;
        if (waitpid(child_pid, &wait_status, 0) != -1) {
            if (WIFEXITED(wait_status)) {
                cout << "Child process exited normally with status: " << WEXITSTATUS(wait_status) << endl;
            } else {
                cout << "Child process exited abnormally." << endl;
            }
        }
    } else {
        cerr << "posix_spawnp failed with error code: " << status << endl;
    }

    // 4. Wait for our background thread to finish before exiting
    pthread_join(thread_id, NULL);
    cout << "Parent process finished." << endl;

    return 0;
}
