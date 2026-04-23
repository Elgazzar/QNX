# Pthread Functions Documentation

This document describes the thread attribute initialization and creation functions used in the application, specifically targeting the POSIX API used in the QNX RTOS environment.

## Headers Required
To use these functions, the following headers must be included in your C file:
```c
#include <pthread.h>
#include <sched.h> // For struct sched_param and scheduling policies
```

---

## 1. `pthread_attr_init`

**Signature:**
```c
int pthread_attr_init(pthread_attr_t *attr);
```

**Description:**
Initializes a thread attributes object (`attr`) with the system's default values. This attributes object acts as a configuration template to specify properties of a thread (such as scheduling policy, priority, detach state, and stack size) prior to its creation. It must be called before applying any other `pthread_attr_*` modification functions.

---

## 2. `pthread_attr_setschedpolicy`

**Signature:**
```c
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
```

**Description:**
Sets the scheduling policy attribute within the thread attributes object. 
In a real-time OS like QNX, common policies include:
*   `SCHED_FIFO`: First-In-First-Out scheduling. A thread runs until it blocks, voluntarily yields, or is preempted by a higher-priority thread.
*   `SCHED_RR`: Round-Robin scheduling. Similar to FIFO, but threads of the same priority share the CPU via time slicing.
*   `SCHED_SPORADIC`: A policy designed to guarantee a certain amount of execution capacity over a given replenishment period.

---

## 3. `pthread_attr_setinheritsched`

**Signature:**
```c
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
```

**Description:**
Determines how the newly created thread acquires its scheduling attributes (policy and parameters/priority). 
*   `PTHREAD_INHERIT_SCHED`: The new thread inherits the scheduling policy and parameters from the parent thread that created it. The values set in the `attr` object are ignored.
*   `PTHREAD_EXPLICIT_SCHED`: The new thread strictly uses the scheduling policy and parameters explicitly set in the attributes object. **This is required** when you want to create a thread with a specific priority different from the parent thread.

---

## 4. `pthread_attr_setschedparam`

**Signature:**
```c
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
```

**Description:**
Sets the scheduling parameters in the attributes object. The `sched_param` structure contains the `sched_priority` member, which defines the priority level. This function binds the configured priority to the attributes object so that the thread starts with the desired priority immediately upon creation.

---

## 5. `pthread_create`

**Signature:**
```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
```

**Description:**
Creates a new concurrent thread of execution within the calling process. 
*   `thread`: A pointer to a `pthread_t` variable where the system will store the ID of the newly created thread.
*   `attr`: A pointer to the initialized attributes object. If set to `NULL`, the thread is created with default system attributes.
*   `start_routine`: A function pointer to the routine the new thread will execute. The function must return a `void*` and accept a `void*` argument.
*   `arg`: The argument (if any) to be passed into the `start_routine`.

Once `pthread_create` executes successfully, the OS scheduler creates the thread, and it immediately becomes eligible to execute the provided `start_routine`.

---

## 6. `pthread_join`

**Signature:**
```c
int pthread_join(pthread_t thread, void **value_ptr);
```

**Description:**
Blocks the calling thread until the specified `thread` terminates. 
*   `thread`: The ID of the target thread you want to wait for (obtained from `pthread_create`).
*   `value_ptr`: A pointer to a location where the exit status of the target thread will be stored. If you do not care about the thread's return value (e.g., if the thread returns `NULL`), you can pass `NULL`.

**Why it's important:**
*   **Synchronization:** It ensures that the main thread (or calling thread) does not exit before the worker threads have finished their execution. If the `main()` function returns, the entire process and all of its child threads are terminated instantly.
*   **Resource Cleanup:** By default, threads are created as "joinable." When a joinable thread finishes, its resources (like the thread stack and internal OS structures) are not fully released back to the system until another thread calls `pthread_join` on it. Failing to join these threads can lead to a resource leak (often referred to as a "zombie" thread).
