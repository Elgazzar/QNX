# Timer Frequency & Aliasing Issues

This example demonstrates the concept of **Timer Aliasing** (or Quantization) in RTOS design, and why requesting a timer interval that doesn't evenly divide into the system clock tick is a bad design pattern.

## Description & Conceptual Explanation

### The Problem
At its core, a standard software timer is driven by the operating system's heartbeat, known as the system tick or `ClockPeriod()`. A very common default tick size is **1 millisecond**.

If you request a periodic POSIX timer to fire every **1.5 milliseconds**, the kernel faces a dilemma. The kernel can only process timer expirations when the hardware tick interrupts the CPU (at 1ms, 2ms, 3ms...). 

Because the hardware physically cannot fire at 1.5ms, the kernel tries to be "smart" by mathematically averaging the interval over time. As a result, the kernel **alternates** the timer expirations between tick boundaries. 
Your repeating 1.5ms timer will fire at ~2.0ms, then ~1.0ms, then ~2.0ms, then ~1.0ms. Over thousands of cycles, the average is exactly 1.5ms.

### Why is this bad?
While the *average* frequency is correct, this introduces massive **Jitter**. The time between any two consecutive pulses swings wildly by 50% to 100%. In automotive systems (like Adaptive AUTOSAR) where precise CAN bus message injection rates or sensor polling rates are strictly required, this massive cycle-to-cycle jitter can lead to system faults, missed deadlines, or erratic physical behavior.

### How to Fix It (Best Practices)
To avoid jitter and guarantee strict real-time determinism, **you must always set your software timers to be an integer multiple of the system tick**. 

If you truly need a 1.5ms interval, you have three options:

1. **Query and Align (Recommended)**: Query the current system tick using `ClockPeriod()`. If the tick is 1ms, redesign your application to run at exactly 1ms or 2ms (integer multiples of the tick).
   ```c
   struct _clockperiod clock_res;
   ClockPeriod(CLOCK_REALTIME, NULL, &clock_res, 0);
   // clock_res.nsec contains the system tick size in nanoseconds (e.g., 1000000 for 1ms)
   ```
2. **Change the System Clock Period (Requires Root)**: You can change the global hardware tick. Set `ClockPeriod()` to 500us (0.5ms). Then a 1.5ms timer perfectly aligns with 3 ticks (0.5 * 3 = 1.5). Note: This impacts the whole OS and requires elevated privileges.
3. **Use a High Resolution Timer**: Use the `TIMER_TOLERANCE` flag to request a High-Resolution timer. This attempts to bypass the tick grid entirely and programs the hardware timer directly for your event.

## APIs Used

- `ClockPeriod()`: Used to query the current system clock tick resolution in nanoseconds.
- `timer_settime()`: Used to request the misaligned 1.5ms interval.

## Compilation and Execution
```bash
make
./Timer_Frequancy_Issues
```
