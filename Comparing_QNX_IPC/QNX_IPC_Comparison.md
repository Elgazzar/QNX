# QNX IPC Mechanisms Overview

This document provides a comprehensive comparison of the various Inter-Process Communication (IPC) mechanisms available in the QNX Neutrino RTOS, including both native QNX features and standard POSIX/UNIX APIs.

---

## 1. QNX Native Messaging (Message Passing)
Message passing is the fundamental, most optimized IPC mechanism in QNX. Everything in QNX (including POSIX APIs like `open()` and `read()`) translates into QNX Native Messaging under the hood.

### Key Characteristics:
*   **Client-Server (RPC) Model**: The client sends a request and waits for a response. The server receives the request, processes it, and replies.
*   **Inherent Synchronization**: The `MsgSend()` call blocks the client thread until the server receives the message, processes it, and calls `MsgReply()`. This built-in synchronization often eliminates the need for mutexes or condition variables.
*   **Copies Any Size Data**: Data of any size is directly copied from the client's virtual memory space into the server's virtual memory space (and vice-versa during the reply) by the kernel.
*   **Carries Priority Information (Priority Inheritance)**: To prevent priority inversion, the server thread temporarily inherits the priority of the highest-priority client that sent it a message. Once the reply is sent, the server's priority returns to normal.

---

## 2. QNX Pulses
Pulses are QNX's native mechanism for unblocked, asynchronous event notification.

### Key Characteristics:
*   **Non-Blocking Notification**: The sender (which can be a thread, an interrupt handler, or a timer) drops the pulse into a channel and continues executing immediately without waiting for a reply.
*   **Compatible with Native Messaging**: A server can wait for both Native Messages and Pulses in the exact same `MsgReceive()` loop, making it incredibly easy to handle both RPC requests and asynchronous hardware/timer events in a single thread.
*   **Tiny Payload**: Pulses carry a very small amount of data (an 8-bit code and a 32-bit integer or pointer value).
*   **Carries Priority Information**: Like native messages, pulses carry priority. The kernel queues pulses based on priority, ensuring high-priority pulses are handled before lower-priority messages or pulses.

---

## 3. POSIX / UNIX Standard APIs
QNX provides full support for standard POSIX IPC APIs to ensure compatibility with UNIX/Linux applications.

### 3.1 Signals
*   **Usage**: Asynchronous software interrupts used to notify a process of an event (e.g., `SIGKILL`, `SIGSEGV`).
*   **Pros**: Standardized, good for simple abort/interrupt signals.
*   **Cons**: Extremely difficult to pass complex data safely; executing inside a signal handler is restrictive and prone to race conditions.

### 3.2 Shared Memory (`shm_open`, `mmap`)
*   **Usage**: Maps physical memory into the virtual address space of multiple processes.
*   **Pros**: The absolute fastest form of IPC for moving large amounts of data (like video frames or audio buffers), as it avoids copying entirely.
*   **Cons**: Provides zero inherent synchronization. You must pair it with POSIX Mutexes or Semaphores to prevent race conditions.

### 3.3 Pipes (`pipe`, `mkfifo`)
*   **Usage**: Unidirectional, FIFO data streams between processes.
*   **Pros**: Simple to use with standard read/write operations.
*   **Cons**: Unstructured (just a byte stream, no message boundaries), cannot easily handle priority inversion.

### 3.4 POSIX Message Queues (`mq_open`, `mq_send`, `mq_receive`)
*   **Usage**: Linked-list based queues for asynchronous message passing.
*   **Pros**: POSIX standard, supports message boundaries and priorities, non-blocking nature.
*   **Cons**: Considerably slower and heavier than QNX Native Messaging because it requires an external server (mqueue) or kernel queue buffering.

### 3.5 TCP/IP Sockets
*   **Usage**: Network communication across different machines (or locally via UNIX Domain Sockets).
*   **Pros**: Universal standard for networking.
*   **Cons**: Highest processing overhead. Only use for IPC if the processes might be moved to physically separate machines in the future.
