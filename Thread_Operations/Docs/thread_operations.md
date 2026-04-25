# POSIX Thread Operations Documentation

This document describes common POSIX thread (`pthread`) lifecycle and management functions. These are fundamental APIs for manipulating threads in QNX and other POSIX-compliant operating systems.

---

## 1. `pthread_self`
**Signature:** `pthread_t pthread_self(void);`
**Description:** Returns the thread ID of the calling thread. 
**Usage:** This is similar to `getpid()` for processes, but for threads. It is often used when a thread needs to pass its own ID to another function or print it for debugging.

## 2. `pthread_setname_np`
**Signature:** `int pthread_setname_np(pthread_t thread, const char *name);`
**Description:** Sets a human-readable name for a thread. The `_np` suffix stands for "non-portable" (it is a widely supported extension in Linux/QNX, but not strictly POSIX standard).
**Usage:** Extremely useful for debugging. When inspecting the process using IDE debuggers or command-line tools like `pidin` on QNX, you will see this name instead of a raw thread ID.

## 3. `pthread_exit`
**Signature:** `void pthread_exit(void *value_ptr);`
**Description:** Terminates the calling thread.
**Usage:** When a thread calls this, it immediately stops executing. The `value_ptr` allows the thread to return an exit status back to whichever thread calls `pthread_join` on it. Note: Returning from the thread's start function (e.g., `return NULL;`) implicitly calls `pthread_exit`.

## 4. `pthread_join`
**Signature:** `int pthread_join(pthread_t thread, void **value_ptr);`
**Description:** Suspends execution of the calling thread until the specified `thread` terminates.
**Usage:** This is used for synchronization and resource cleanup. If a thread is "joinable" (the default state), its resources (like its stack) are not freed by the OS until another thread joins it. The `value_ptr` will receive the status passed to `pthread_exit`.

## 5. `pthread_detach`
**Signature:** `int pthread_detach(pthread_t thread);`
**Description:** Marks a thread as "detached". 
**Usage:** When a detached thread terminates, its resources are automatically released back to the system without the need for another thread to call `pthread_join`. **Important:** Once a thread is detached, it *cannot* be joined. This is perfect for "fire-and-forget" background worker threads.

## 6. `pthread_cancel`
**Signature:** `int pthread_cancel(pthread_t thread);`
**Description:** Sends a cancellation request to a thread.
**Usage:** This is a way to forcefully stop another thread. However, the target thread doesn't die instantly. It only terminates when it reaches a "cancellation point" (usually blocking system calls like `sleep`, `read`, or `wait`). If the thread is joined later, its exit status will be `PTHREAD_CANCELED`.

## 7. `pthread_kill`
**Signature:** `int pthread_kill(pthread_t thread, int sig);`
**Description:** Sends a standard signal (like `SIGINT` or `SIGTERM`) to a specific thread within the same process.
**Usage:** While `kill()` sends a signal to a whole process, `pthread_kill()` targets a specific thread. **Trick:** If you send signal `0`, no actual signal is sent, but the OS still performs error checking. This is the standard way to check if a thread is still alive (if it returns `ESRCH`, the thread no longer exists).
