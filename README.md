# QNX Neutrino RTOS - Learning Repository

This repository contains a collection of exercises and examples demonstrating how to build robust, multi-threaded applications on the QNX Neutrino RTOS. 

The examples are structured logically from **easiest to hardest**, building up from basic thread lifecycles to advanced QNX-proprietary microkernel features.

---

## 📚 Learning Path & Examples

### 1. Basic Thread Operations (`Thread_Operations`)
*   **Description:** The absolute basics of multi-threading in POSIX. Demonstrates how to create, join, detach, and cancel threads.
*   **Documentation:** [`Thread_Operations/Docs/thread_operations.md`](Thread_Operations/Docs/thread_operations.md)

### 2. Thread Priorities & Scheduling (`Threads` & `High_Low_Prio_Threads`)
*   **Description:** Moving beyond basic threads to explore Real-Time scheduling (`SCHED_RR`, `SCHED_FIFO`) and explicitly setting thread priorities using attribute structures.
*   **Documentation:** [`High_Low_Prio_Threads/Docs/pthread_functions.md`](High_Low_Prio_Threads/Docs/pthread_functions.md)

### 3. Mutex Synchronization (`Synchronization` & `No_Mutux_Problem`)
*   **Description:** Introduces the "Critical Section" problem. Demonstrates how race conditions corrupt shared data and how to solve them using Mutex locks.
*   **Documentation:** [`Synchronization/Docs/mutex_operations.md`](Synchronization/Docs/mutex_operations.md)

### 4. Lock-Free Atomic Operations (`Atomic_Operations`)
*   **Description:** An extremely fast alternative to Mutexes. Demonstrates QNX-specific hardware-level atomic instructions for safe math and bitwise operations across threads.
*   **Documentation:** [`Atomic_Operations/Docs/atomic_operations.md`](Atomic_Operations/Docs/atomic_operations.md)

### 5. Condition Variables (`Condvar` & `Condvar_Exercise`)
*   **Description:** Advanced thread synchronization. Demonstrates how to put threads to sleep efficiently until a specific condition is met, avoiding heavy CPU polling. Includes a complex 4-state machine example.
*   **Documentation:** [`Condvar/Docs/condvar_operations.md`](Condvar/Docs/condvar_operations.md)

### 6. Process Creation (`Process`)
*   **Description:** Explores the fundamental difference between duplicating memory with `fork()` and executing new binaries with `posix_spawn()`. Demonstrates parent/child lifecycles.
*   **Documentation:** [`Process/Docs/process_creation.md`](Process/Docs/process_creation.md)

### 7. Signal Handling (`Signals_Handling`)
*   **Description:** Explores the complex interaction between POSIX signals and multithreading. Demonstrates the best-practice pattern of blocking signals globally and catching them synchronously using a dedicated `sigwait` thread.
*   **Documentation:** [`Signals_Handling/Docs/signals_documentation.md`](Signals_Handling/Docs/signals_documentation.md)

### 8. Native IPC: Message Passing (`Message_Passing`)
*   **Description:** An introduction to QNX's core microkernel architecture. Demonstrates local Inter-Process Communication using the synchronous Client/Server model (`MsgSend`, `MsgReceive`, `MsgReply`). This is the foundation mechanism used beneath Adaptive AUTOSAR (`ara::com`).
*   **Documentation:** [`Message_Passing/Docs/message_passing.md`](Message_Passing/Docs/message_passing.md)

### 9. Asynchronous IPC: Death Pulse (`Death_Pulse`)
*   **Description:** The most advanced QNX-specific topic. Demonstrates how to securely register a private channel with the QNX Process Manager to receive asynchronous, unblocked "Pulses" notifying the app whenever any other process in the system dies.
*   **Documentation:** [`Death_Pulse/Docs/death_pulse.md`](Death_Pulse/Docs/death_pulse.md)

---

## 📝 Usage Notes
*   **C vs C++:** Because QNX system APIs are written in C, many of the `.cpp` files in this workspace utilize standard, procedural C coding styles.
*   **Portability:** While many examples use standard POSIX APIs (like `pthread_create`), several examples (Atomic Operations, Message Passing, Death Pulse) are highly proprietary to the QNX architecture and will not compile on standard Linux/Windows without abstraction layers.
