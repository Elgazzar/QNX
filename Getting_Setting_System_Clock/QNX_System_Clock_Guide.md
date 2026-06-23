# Getting and Setting the System Clock in QNX

In QNX, the recommended way to interact with the system clock is by using the standard POSIX functions `clock_gettime()` and `clock_settime()`. These functions use the `CLOCK_REALTIME` clock ID to manage the wall-clock time.

## 1. POSIX Functions (Recommended)

### `clock_gettime()`
Retrieves the current time.

```c
#include <time.h>
int clock_gettime(clockid_t clock_id, struct timespec *tp);
```

- **`clock_id`**: Should be `CLOCK_REALTIME` to get the actual time of day.
- **`tp`**: A pointer to a `struct timespec` where the time will be stored. This structure contains `tv_sec` (seconds since the Epoch) and `tv_nsec` (nanoseconds).

### `clock_settime()`
Sets the current system time. 

> [!WARNING]
> You must have **superuser (root) privileges** to use `clock_settime()`. Setting the time without appropriate privileges will result in an `EPERM` error.

```c
#include <time.h>
int clock_settime(clockid_t clock_id, const struct timespec *tp);
```

- **`clock_id`**: Should be `CLOCK_REALTIME`.
- **`tp`**: A pointer to a `struct timespec` containing the new time you want to set.

## 2. QNX-Specific Kernel Call (Alternative)
QNX also provides a native kernel call `ClockTime()`, but QNX documentation advises using the POSIX standard functions instead.

```c
#include <sys/neutrino.h>
int ClockTime(clockid_t id, const uint64_t *new_time, uint64_t *old_time);
```
- Operates using a 64-bit nanosecond value.
- If `new_time` is not `NULL`, it sets the time.
- If `old_time` is not `NULL`, it retrieves the previous time.

## 3. `CLOCK_REALTIME` vs `CLOCK_MONOTONIC`

When using `clock_gettime()`, the clock ID determines the reference point:
- **`CLOCK_REALTIME` (Wall-Clock Time)**: Returns time elapsed since the POSIX Epoch (January 1, 1970). This clock represents the "real world" time. Its value can jump forwards or backwards if the system time is manually changed or synchronized via NTP.
- **`CLOCK_MONOTONIC` (Uptime / Elapsed Time)**: Returns time elapsed since an arbitrary fixed point (usually system boot). This clock strictly and continuously increases. It cannot be set (fails with `EINVAL`) and is immune to administrative changes to the system time. This makes it ideal for timeouts and measuring intervals.

## 4. Setting the System Time to the "Actual" Time

To synchronize your QNX system's `CLOCK_REALTIME` to the actual real-world time, you have three primary approaches:

### Using Network Time Protocol (NTP) - *Recommended*
If your device is networked, this is the most accurate approach:
- **`ntpd` (NTP Daemon)**: Run `ntpd -g` to continuously fetch and maintain accurate time from external servers (like `pool.ntp.org`). The `-g` flag allows an initial large jump if the clock is very inaccurate.
- **`sntp` (Simple NTP Client)**: Run `sntp -s time.nist.gov` for a one-off query to an NTP server to set the clock without leaving a daemon running.

### Using the Hardware Clock (RTC)
Most systems have a battery-backed Real-Time Clock (RTC) chip. 
- To sync the QNX OS software clock from the hardware chip (often done on boot), use: `rtc hw`
- To save the current QNX OS time back to the hardware chip, use: `rtc -s hw`

### Manually Using the `date` Command
If you lack a network connection, you can manually set the time from the command line as root:
- Command format: `date [[[[[cc]yy]mm]dd]hh]mm[.ss]`
- Example (June 11, 2026, at 20:00:30): `date 202606112000.30`

## 5. Important Considerations
- **Hardware Clock Syncing**: Modifying the OS time programmatically with `clock_settime()` only changes the time in memory. You must separately use the `rtc` utility or an equivalent API to write this time back to the battery-backed hardware clock if you want it to persist across reboots.

## 6. Smoothly Adjusting the Clock
Sometimes, you need to correct the time (e.g., from an NTP update), but abruptly jumping the clock forwards or backwards with `clock_settime()` can severely break time-sensitive applications, trigger false timeouts, or confuse utilities like `make`.

To solve this, QNX provides an API to **smoothly adjust** the clock:
- **`ClockAdjust()`** (and its thread-safe equivalent **`ClockAdjust_r()`**)

Additionally, QNX supports the standard POSIX function **`adjtime()`**, which is highly recommended if you want your code to be portable across different UNIX-like systems. Under the hood in QNX, `adjtime()` utilizes `ClockAdjust()`.

### How `ClockAdjust()` Works:
Instead of setting an absolute time instantly, you provide a `_clockadjust` structure that tells the kernel to gradually speed up or slow down the clock until the desired time is reached.

```c
#include <sys/neutrino.h>

int ClockAdjust(clockid_t id, const struct _clockadjust *new_adj, struct _clockadjust *old_adj);
```

The `_clockadjust` structure controls the adjustment:
1. **`tick_nsec_inc`**: The number of nanoseconds to add to (or subtract from) every single clock tick. A negative value safely slows down the clock without ever making time run backwards.
2. **`tick_count`**: The total number of clock ticks over which this adjustment should be applied.

This mechanism ensures the OS elegantly corrects time drift while keeping running processes stable!
