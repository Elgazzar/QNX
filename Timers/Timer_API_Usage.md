# QNX Timer API Usage Guide

This example demonstrates how to set up and handle a POSIX timer on QNX using fundamental system APIs. These APIs are commonly used in system-level programming and within the foundation of the Adaptive AUTOSAR stack on QNX.

## APIs Used

### `ClockPeriod`
Used to query or change the resolution (period) of a specific clock.
- **Usage here**: `ClockPeriod(CLOCK_REALTIME, NULL, &clock_res, 0)` queries the current period (tick size) of `CLOCK_REALTIME`.

### `timer_create`
Initializes a new per-process timer.
- **Parameters**: 
  - `CLOCK_MONOTONIC`: Indicates we want a timer that always increments, unaffected by system clock adjustments (like NTP syncs).
  - `&event`: A `sigevent` structure that defines how we want to be notified when the timer expires. In this example, we configure it to send a `SIGEV_PULSE`.

### `timer_settime`
Arms or disarms the timer.
- **Parameters**: 
  - We use the `itimerspec` structure to specify the `it_value` (initial expiration time, 1 second relative to now) and `it_interval` (reload value, meaning the timer will repeat every 1 second).

### `timer_delete`
Destroys the timer and frees its resources.
- Always remember to delete your timers and detach/destroy channels before exiting.

## Notification Mechanisms (`sigevent`)
When you initialize a timer using `timer_create()`, you must pass it a `sigevent` structure. This structure tells the QNX microkernel exactly *how* you want to be notified when the time elapses. The available options include:

1. **Pulse (`SIGEV_PULSE`) — *The QNX Way***: The kernel sends a tiny, non-blocking message (a "pulse") directly into a channel you created. Your application waits for it using `MsgReceive()`. This is highly recommended because it is extremely fast, avoids context-switching overhead, and integrates perfectly into standard QNX message-passing loops. This example uses `SIGEV_PULSE_INIT` to achieve this.
2. **POSIX Signal (`SIGEV_SIGNAL`)**: The kernel sends a standard UNIX-style signal (like `SIGALRM`), which interrupts your thread to run a Signal Handler. This is generally discouraged in modern QNX design due to high overhead and async-signal-safety risks.
3. **Thread Creation (`SIGEV_THREAD`)**: The kernel spins up a new thread every time the timer expires to run a callback function. This has massive overhead and is rarely used for fast periodic timers.
4. **No Notification (`SIGEV_NONE`)**: The timer expires silently, requiring manual polling via `timer_gettime()`.

### How the Pulse is Sent by the Kernel
Because we chose `SIGEV_PULSE`, the **QNX microkernel** itself handles sending the pulse. Here is the exact flow:
1. **The Setup (`SIGEV_PULSE_INIT`)**: Configures the `sigevent` to deliver a pulse with a specific code and priority to your connection ID (`coid`).
2. **Handing the Event to the Timer (`timer_create`)**: By passing the `sigevent` to `timer_create`, you register these instructions directly with the QNX kernel's timer manager.
3. **The Kernel Takes Over (`timer_settime`)**: When you arm the timer, the OS starts counting down based on the selected clock.
4. **Expiration and Delivery**: When the hardware clock interrupts and the kernel determines the timer has expired, the kernel's message-passing subsystem constructs the tiny pulse message and drops it directly into the channel (`chid`).
5. Your application, which is blocked at `MsgReceive(chid, ...)`, immediately wakes up with the pulse data.

Because the kernel handles the delivery directly into your channel, it avoids the heavy context-switching overhead of traditional POSIX signals and integrates safely into message-passing loops.

## Tickless Architecture & Clock Period
Modern QNX systems (such as QNX 7.x used in Adaptive AUTOSAR) utilize a **tickless kernel** (dynamic tick architecture).
- **Dynamic Interrupts**: The timer hardware is programmed to generate an interrupt precisely when needed. It does not generate an interrupt continuously every millisecond. The kernel dynamically programs the hardware timer to interrupt exactly when the next software timer or scheduling event is due, which drastically reduces CPU overhead and saves power.
- **Clock Period (Tick Size)**: Although you can query or set a logical "tick size" using `ClockPeriod()` (which often defaults to 1ms), modern QNX doesn't use this to constantly fire hardware interrupts. Instead, the logical tick period is used internally by the kernel for thread scheduling (time slicing/quantums) and to align or coalesce timers (grouping software timers together within a tolerance to wake the CPU up fewer times).

## Practical Use Cases (Adaptive AUTOSAR)
Because QNX is an RTOS, developers avoid `sleep()` or `delay()` for precise intervals due to thread-scheduling drift. Instead, POSIX Timers and Pulses are used to build highly deterministic systems.

1. **Deterministic Periodic Threads (Cyclic Runnables)**: The primary use case. A thread blocks on `MsgReceive()`. The timer fires a pulse (e.g., every 10ms), the thread wakes up, executes its logic (e.g., Sensor Fusion reading Radar/Lidar data), and goes back to blocking.
2. **Timeouts & Watchdogs (One-Shot Timers)**: Setting a one-shot timer as a fail-safe for asynchronous operations. For instance, waiting for a crucial `ara::com` (Adaptive AUTOSAR Communication) remote procedure call over automotive Ethernet. If the external ECU doesn't respond in time, the timer pulse wakes up the thread to log a network failure.
3. **Debouncing and Rate-Limiting**: Delaying or slowing down the processing of a flood of interrupts or messages (like a CAN bus burst) by setting a short timer and ignoring subsequent messages until the timer expires.

## Compilation and Execution
```bash
make
./Timer_API_Example
```
