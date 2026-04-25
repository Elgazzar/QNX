# POSIX Condition Variables (Condvar)

This document describes POSIX Condition Variables. A Condition Variable (`pthread_cond_t`) is a synchronization primitive that allows threads to suspend execution (go to sleep) until a certain condition or event occurs. They are **always used in conjunction with a Mutex**.

---

## 1. `pthread_cond_init`
**Signature:**
```c
int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);
```
**Description:**
Initializes a new condition variable.
*   `cond`: A pointer to the condition variable to initialize.
*   `attr`: Attributes for the condition variable (pass `NULL` for defaults).

## 2. `pthread_cond_wait`
**Signature:**
```c
int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
```
**Description:**
Causes the calling thread to block (sleep) until the condition variable is signaled or broadcast.
**CRITICAL CONCEPTS:**
1.  **The Mutex Release:** You **must** lock the `mutex` before calling `pthread_cond_wait`. When you call `pthread_cond_wait`, the OS automatically and atomically puts your thread to sleep AND unlocks the `mutex`. This allows other threads (like a producer) to acquire the mutex and change the shared data.
2.  **The Mutex Re-Acquisition:** When the thread wakes up (because it was signaled), the OS automatically re-locks the `mutex` for you before `pthread_cond_wait` returns.
3.  **Spurious Wakeups:** Threads can sometimes wake up from `pthread_cond_wait` even if no one signaled them (a quirk of OS scheduling). Because of this, you must **always** wrap `pthread_cond_wait` inside a `while` loop that checks the actual boolean condition.

## 3. `pthread_cond_signal`
**Signature:**
```c
int pthread_cond_signal(pthread_cond_t *cond);
```
**Description:**
Wakes up **at least one** thread that is currently blocked waiting on the specified condition variable. If multiple threads are waiting, the OS scheduling policy determines which one wakes up. If no threads are waiting, the signal is ignored (it is not queued for future threads).

## 4. `pthread_cond_broadcast`
**Signature:**
```c
int pthread_cond_broadcast(pthread_cond_t *cond);
```
**Description:**
Wakes up **all** threads that are currently blocked waiting on the specified condition variable. This is useful when an event occurs that changes the state in a way that multiple sleeping threads might now be able to proceed.

## 5. `pthread_cond_destroy`
**Signature:**
```c
int pthread_cond_destroy(pthread_cond_t *cond);
```
**Description:**
Destroys the condition variable, freeing any system resources. You must ensure no threads are currently blocked on the condition variable before destroying it.
