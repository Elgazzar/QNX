# Kernel Call Timeouts (`TimerTimeout`)

This example demonstrates how to use the `TimerTimeout()` API to bound the maximum amount of time a thread will block on a QNX microkernel call.

## Description & Conceptual Explanation

### The Problem
In real-time systems, blocking indefinitely on an operation is a critical risk. If a client thread calls `MsgSend()` to request data from a driver, and that driver has crashed or is deadlocked, the client thread will block forever. This can cause a cascading failure across the entire system.

### The Solution
QNX provides `TimerTimeout()`, a unique microkernel feature that arms a "timeout event" for the **next** blocking kernel call the thread makes. 

You can specify exactly which blocking state you want to time out on (e.g., `_NTO_TIMEOUT_SEND`, `_NTO_TIMEOUT_RECEIVE`, or even `_NTO_TIMEOUT_MUTEX`). If the thread enters that state and remains blocked for longer than the specified time, the kernel forcibly unblocks the thread, returning `-1` and setting `errno` to `ETIMEDOUT`.

This provides a vital fail-safe mechanism, allowing the thread to recover gracefully, log an error, or switch to a degraded operating mode.

## APIs Used

- `SIGEV_UNBLOCK_INIT()`: Initializes an event that instructs the kernel to unblock the thread when the timer expires.
- `TimerTimeout()`: Arms the actual kernel timeout for a specific blocking state.
- `MsgReceive()`: Used in this example to demonstrate an indefinite block that is successfully interrupted by the timeout.

## Compilation and Execution
```bash
make
./Kernel_Timeouts
```
