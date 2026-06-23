# Design Considerations: `delay()` vs POSIX Timers

This example demonstrates a fundamental concept in Real-Time Operating Systems (RTOS) and Adaptive AUTOSAR: why using `delay()`, `sleep()`, or `usleep()` for cyclic execution is a bad design, and why POSIX Timers are superior.

## Description & Conceptual Explanation

### The Bad Design: `delay()` and Scheduling Drift
If you have a cyclic runnable that needs to execute every 5 milliseconds, a naive approach is to put a `delay(5)` at the end of a `while(1)` loop. 
However, **threads take time to execute**. If your processing logic takes 2 milliseconds to run, and then you call `delay(5)`, your total loop time becomes **7 milliseconds**. Over hundreds of cycles, this error accumulates into massive **Scheduling Drift**, completely destroying the deterministic nature of your real-time system.

### The Good Design: POSIX Timers
POSIX Timers run independently of your thread's execution time. If you configure a timer to fire every 5ms, the kernel's timer manager will drop a pulse into your channel exactly every 5ms.
If your logic takes 2ms to process, you will only block on `MsgReceive()` for 3ms before the next pulse arrives. The total loop time remains strictly locked at **5 milliseconds**. There is zero drift.

## APIs Used

- `delay()`: Suspends the execution of the calling thread for the given number of milliseconds. Used here to simulate processing workload and demonstrate bad cyclic design.
- `timer_create` & `timer_settime`: POSIX APIs used to create an independent hardware-backed cyclic timer.
- `MsgReceive()`: QNX IPC function used to block a thread safely until the exact moment a timer pulse arrives.

## Compilation and Execution
```bash
make
./Design_Considerations
```
