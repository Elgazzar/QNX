# High Resolution Timers (`TIMER_TOLERANCE`)

This example demonstrates how to configure a POSIX timer to act as a **High-Resolution Timer (HRT)** in QNX.

## Description

In modern QNX systems, the kernel attempts to save CPU cycles and power by "coalescing" timers. This means if multiple timers expire near each other, or near a system tick boundary (e.g., 1ms), the kernel will adjust their expiration times slightly so it only has to wake up the CPU once. 

While efficient, this introduces jitter. For highly time-sensitive tasks in Adaptive AUTOSAR (like sensor polling or strict real-time control loops), you may need a timer that fires precisely at sub-millisecond intervals. 

This is achieved using the `TIMER_TOLERANCE` flag. By specifying a tolerance of 0 (or a very small number), you tell the kernel that this timer cannot be coalesced. The kernel responds by directly programming the hardware timer to interrupt exactly at your requested time.

## Conceptual Explanation

To use `TIMER_TOLERANCE`, you must call `timer_settime` twice:
1. **Set the Tolerance**: Call it with the `TIMER_TOLERANCE` flag. You pass the desired tolerance in the `it_value` of the `itimerspec` structure.
2. **Arm the Timer**: Call it normally (without the tolerance flag) to actually start the countdown. Order matters: tolerance must be configured before the timer is armed.

*Note: Requesting a high-resolution timer requires the process to have the `PROCMGR_AID_HIGH_RESOLUTION_TIMER` ability enabled.*

## APIs Used

### `timer_settime` with `TIMER_TOLERANCE`
- **Usage**: `timer_settime(timer_id, TIMER_TOLERANCE, &tolerance_spec, NULL)`
- **Behavior**: Instead of arming the timer, this specific flag repurposes the function to configure the timer's coalesce behavior. `tolerance_spec.it_value.tv_nsec` contains the maximum acceptable jitter in nanoseconds.

### `clock_gettime`
- **Usage**: `clock_gettime(CLOCK_MONOTONIC, &ts)`
- **Behavior**: Retrieves the current time at nanosecond precision. We use this in the example to measure the exact microsecond difference between pulses and prove the timer is running accurately at a sub-millisecond rate (500us).

## Practical Use Cases (Adaptive AUTOSAR)
High-resolution timers using `TIMER_TOLERANCE` move execution from the millisecond domain into the microsecond domain, ensuring strict deterministic behavior without coalescing delays.

1. **High-Resolution Hardware Control**: Managing a high-frequency PID control loop for an electric motor where a delay of a few milliseconds could result in physical hardware damage.
2. **Precise Firing Angles**: Controlling the exact microsecond firing angle of a fuel injector.
3. **Strict Network Schedules**: Executing Time-Division Multiple Access (TDMA) network schedules where sub-millisecond precision is non-negotiable for sending Ethernet frames on the wire at exact times.

## Compilation and Execution

```bash
make
./High_Resolution_Timers
```
