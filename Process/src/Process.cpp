#include <iostream>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

// Access the global environment variables of the current process
extern char **environ;

// =========================================================
// CHILD PROCESS LOGIC
// =========================================================

// This thread will be created INSIDE the new child process
void* child_worker_thread(void* arg) {
    for (int i = 0; i < 3; ++i) {
        cout << "  -> [Child's Thread] Running concurrently inside the child process..." << endl;
        sleep(1);
    }
    return NULL;
}

int run_as_child() {
    cout << "[Child Main] Child process started." << endl;
    
    pthread_t thread_id;
    cout << "[Child Main] Spawning a thread inside the child process..." << endl;
    // 1. The child process creates its own thread!
    pthread_create(&thread_id, NULL, child_worker_thread, NULL);
    
    // 2. The child's main thread also does some work
    for (int i = 0; i < 3; ++i) {
        cout << "[Child Main] Doing some work in the child's main thread..." << endl;
        sleep(1);
    }

    // 3. Wait for the child's worker thread to finish
    pthread_join(thread_id, NULL);
    cout << "[Child Main] Child process exiting." << endl;
    return 0;
}

// =========================================================
// PARENT PROCESS LOGIC
// =========================================================

int run_as_parent(const char* program_path) {
    pid_t child_pid;
    
    // We pass "child" as an argument so the new process knows it's the child!
    // program_path is argv[0] (the name/path of this executable)
    char* argv[] = { (char*)program_path, (char*)"child", NULL };

    cout << "--- PARENT: Spawning our own executable as a child process ---" << endl;

    // We use posix_spawn to execute THIS SAME PROGRAM, but with the "child" argument
    int status = posix_spawn(&child_pid, program_path, NULL, NULL, argv, environ);

    if (status == 0) {
        cout << "PARENT: Child process spawned successfully with PID: " << child_pid << endl;
        
        // The parent waits for the child process to finish
        int wait_status;
        if (waitpid(child_pid, &wait_status, 0) != -1) {
            cout << "PARENT: Child process finished executing." << endl;
        }
    } else {
        cerr << "posix_spawn failed with error code: " << status << endl;
    }

    return 0;
}

// =========================================================
// MAIN ENTRY POINT
// =========================================================

int main(int argc, char* argv[]) {
    // If we were passed an argument and it's "child", run the child logic
    if (argc > 1 && string(argv[1]) == "child") {
        return run_as_child();
    } 
    // Otherwise, run the parent logic (which will spawn the child)
    else {
        return run_as_parent(argv[0]);
    }
}
