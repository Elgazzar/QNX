# POSIX Mutex Operations Documentation

This document describes the core POSIX Mutex (`pthread_mutex_t`) operations. A **Mutex** (Mutual Exclusion) is a synchronization primitive used to prevent multiple threads from accessing shared resources (like a global variable, hardware peripheral, or data structure) at the exact same time, which would cause race conditions and data corruption.

---

## 1. `pthread_mutex_init`

**Signature:**
```c
int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
```

**Description:**
Initializes a newly created mutex object before it can be used.
*   `mutex`: A pointer to the `pthread_mutex_t` variable you want to initialize.
*   `attr`: A pointer to a mutex attributes object. If you pass `NULL`, the mutex is initialized with standard default attributes. (In QNX/RTOS environments, you might pass custom attributes here to enable Priority Inheritance to prevent priority inversion).

---

## 2. `pthread_mutex_lock`

**Signature:**
```c
int pthread_mutex_lock(pthread_mutex_t *mutex);
```

**Description:**
Attempts to lock the specified mutex. This is placed at the very beginning of a "Critical Section".
*   If the mutex is currently **unlocked**, the calling thread successfully locks it and continues executing immediately.
*   If the mutex is currently **locked** by another thread, the calling thread is suspended (put to sleep) until the mutex becomes unlocked. Once unlocked, the OS will wake up this thread, hand it the lock, and execution continues.

---

## 3. `pthread_mutex_unlock`

**Signature:**
```c
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

**Description:**
Releases the lock on the specified mutex. This is placed at the very end of a "Critical Section".
*   If there are other threads blocked (waiting) on this mutex, releasing the lock will wake one of them up so it can acquire the lock and proceed.
*   **Important:** You must never unlock a mutex that is not currently locked by your own thread. Doing so results in undefined behavior.

---

## 4. `pthread_mutex_destroy`

**Signature:**
```c
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

**Description:**
Destroys the specified mutex object, freeing any system resources it might be holding.
*   You must call this when you are completely finished with the mutex (usually right before the program exits or the object containing the mutex is deleted).
*   **Important:** You must never destroy a mutex that is currently locked by a thread, or one that threads are currently blocked waiting for.
