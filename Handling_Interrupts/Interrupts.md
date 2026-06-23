# QNX Hardware Interrupt Management

In QNX Neutrino RTOS, handling hardware interrupts often involves attaching to an interrupt vector, responding to the interrupt, and synchronizing with thread-level code. Some interrupt functions operate globally on the CPU, while others operate on specific interrupts or handlers.

**Important Privileges and Abilities:** 
Before using these APIs, your process must have the appropriate security abilities enabled (typically requested via `procmgr_ability()`):
*   **`PROCMGR_AID_INTERRUPT`**: Required to attach to any interrupt using `InterruptAttachThread()` or `InterruptAttachEvent()`.
*   **`PROCMGR_AID_IO`**: Required to request I/O privileges. Once you have this ability, you must activate the privileges for your thread by calling `ThreadCtl(_NTO_TCTL_IO, 0)`. Without activating I/O privileges, calling hardware-level functions like `InterruptDisable()` or `InterruptEnable()` will result in a segmentation fault (`SIGSEGV`).
## Key Interrupt APIs

### 1. `InterruptAttachThread(int intr, unsigned flags)`
Attaches the calling thread to a hardware interrupt, turning it into a dedicated Interrupt Service Thread (IST). 
*   **Purpose**: This is the modern, preferred method for high-performance interrupt handling in QNX. The thread calls this function and then loops on `InterruptWait()`. The OS efficiently handles waking the thread when the interrupt fires.

### 2. `InterruptAttachEvent(int intr, const struct sigevent *event, unsigned flags)`
Binds a hardware interrupt to an OS-level event (like a Pulse, Signal, or `SIGEV_INTR`).
*   **Purpose**: When the interrupt occurs, the kernel delivers the event to the designated receiver. This is excellent for integrating interrupts into standard message-passing architectures, though it might have slightly higher overhead than a dedicated IST depending on the event type.

### 3. `InterruptDetach(int id)`
Removes an interrupt handler that was previously attached using `InterruptAttachThread()`, `InterruptAttachEvent()`, or `InterruptAttach()`.
*   **`id`**: The ID returned by the attach function.
*   **Purpose**: Cleans up resources when your driver is shutting down or when you no longer want to process a specific interrupt.

### 4. `InterruptWait(int flags, const uint64_t *timeout)`
Blocks the calling thread until a hardware interrupt occurs. 
*   **Purpose**: Used by threads that have attached via `InterruptAttachThread()` or `InterruptAttachEvent()` (with `SIGEV_INTR`). The thread loops and blocks on `InterruptWait()`. When the interrupt fires, the thread wakes up and handles the hardware.

### 5. `InterruptMask(int intr, int id)`
Disables a specific hardware interrupt line at the PIC (Programmable Interrupt Controller) level.
*   **`intr`**: The interrupt vector/level to mask.
*   **`id`**: The handler ID returned by the attach function.
*   **Purpose**: Prevents the interrupt from firing again. This is typically used with level-triggered interrupts. If an IST cannot clear the source of the interrupt immediately, it must mask the interrupt and perhaps signal another thread to clear the hardware. 
*   **Note**: Calls are nested. If you mask an interrupt twice, you must unmask it twice.

### 6. `InterruptUnmask(int intr, int id)`
Re-enables a specific hardware interrupt line that was previously masked.
*   **Purpose**: Called after the thread-level code has successfully cleared the hardware condition that caused the interrupt.

### 7. `InterruptDisable()`
Disables **all** hardware interrupts globally on the current processor.
*   **Purpose**: Prevents the processor from being interrupted by hardware. 
*   **Warning**: This severely impacts system real-time performance and interrupt latency. It should be used sparingly and only for very short critical sections. Modern QNX development prefers `InterruptLock()`/`InterruptUnlock()` for protecting shared data across SMP systems.

### 8. `InterruptEnable()`
Re-enables all hardware interrupts after a previous call to `InterruptDisable()`.
*   **Purpose**: Restores normal interrupt processing after a critical section.

---

## Example Architecture
There are two primary ways to wait for an interrupt in QNX:
1.  **Dedicated Thread (Best Performance):** Use `InterruptAttachThread()`, then loop on `InterruptWait()`.
2.  **Event Driven (Best Integration):** Use `InterruptAttachEvent()` to hook up the interrupt to a `SIGEV_PULSE`, which is then delivered to a standard `MsgReceive()` loop.
