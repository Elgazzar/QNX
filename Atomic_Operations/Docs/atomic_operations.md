# QNX Atomic Operations

This document describes the core atomic operations provided by the QNX Neutrino RTOS (via `<atomic.h>`).

## What are Atomic Operations?
Atomic operations are a way to perform basic math or bitwise operations on shared variables **safely across multiple threads without needing a mutex**.

When you write `variable++` in standard C++, the CPU actually performs three steps: Read the value, add 1, and write the value back. If two threads do this at the exact same time, they overwrite each other resulting in corrupted data (a race condition). 

Atomic operations use special, highly optimized hardware instructions built directly into the CPU architecture to guarantee that the read/modify/write cycle happens instantly as one single, uninterruptible (atomic) step. They are significantly faster than locking and unlocking a mutex, making them ideal for simple counters and flags.

> [!WARNING] 
> **QNX Specific API**
> The specific functions described here (`atomic_add`, `atomic_set`, etc.) and the `<atomic.h>` header are proprietary to the QNX C library. If you are writing cross-platform code (for Linux, Windows, macOS), you should use the standard C++ `<atomic>` library (`std::atomic<T>`) or the C11 `<stdatomic.h>` standard instead!

---

## Math Operations

### 1. `atomic_add`
**Signature:** `void atomic_add( volatile unsigned * loc, unsigned add );`
**Description:** Safely adds a value to a variable. It is the thread-safe equivalent of `*loc += add;`.

### 2. `atomic_sub`
**Signature:** `void atomic_sub( volatile unsigned * loc, unsigned sub );`
**Description:** Safely subtracts a value from a variable. It is the thread-safe equivalent of `*loc -= sub;`.

---

## Bitwise Operations
These functions are extremely useful for manipulating shared bitmasks or hardware register flags where multiple threads might be trying to flip different bits at the same time.

### 3. `atomic_set`
**Signature:** `void atomic_set( volatile unsigned * loc, unsigned bits );`
**Description:** Safely sets (turns ON) specific bits in a variable. It is the thread-safe equivalent of a Bitwise OR: `*loc |= bits;`.

### 4. `atomic_clr`
**Signature:** `void atomic_clr( volatile unsigned * loc, unsigned bits );`
**Description:** Safely clears (turns OFF) specific bits in a variable. It is the thread-safe equivalent of a Bitwise AND NOT: `*loc &= ~bits;`.

### 5. `atomic_toggle`
**Signature:** `void atomic_toggle( volatile unsigned * loc, unsigned bits );`
**Description:** Safely flips specific bits in a variable (1 becomes 0, 0 becomes 1). It is the thread-safe equivalent of a Bitwise XOR: `*loc ^= bits;`.
