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

## Notification Mechanism (Pulses)
Instead of using POSIX signals, which can interrupt code flow and involve more overhead, QNX developers usually prefer **pulses**. A pulse is a fixed-size, non-blocking message.
- `ChannelCreate`: Creates a channel for receiving messages.
- `ConnectAttach`: Creates a connection to that channel.
- `SIGEV_PULSE_INIT`: Associates the pulse with our connection ID (`coid`).
- `MsgReceive`: Blocks the thread until a message or pulse arrives on the channel.

### How the Pulse is Sent by the Kernel
The **QNX microkernel** itself handles sending the pulse. Here is the exact flow:
1. **The Setup (`SIGEV_PULSE_INIT`)**: Configures the `sigevent` to deliver a pulse with a specific code and priority to your connection ID (`coid`).
2. **Handing the Event to the Timer (`timer_create`)**: By passing the `sigevent` to `timer_create`, you register these instructions directly with the QNX kernel's timer manager.
3. **The Kernel Takes Over (`timer_settime`)**: When you arm the timer, the OS starts counting down based on the selected clock.
4. **Expiration and Delivery**: When the hardware clock interrupts and the kernel determines the timer has expired, the kernel's message-passing subsystem constructs the tiny pulse message and drops it directly into the channel (`chid`).
5. Your application, which is blocked at `MsgReceive(chid, ...)`, immediately wakes up with the pulse data.

Because the kernel handles the delivery directly into your channel, it avoids the heavy context-switching overhead of traditional POSIX signals and integrates safely into message-passing loops.

## Compilation and Execution
```bash
make
./Timer_API_Example
```
