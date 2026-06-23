# General QNX Information

This document serves as a knowledge base for fundamental QNX system concepts that don't belong to a specific code example but are crucial for understanding the operating system's architecture, particularly in the context of Adaptive AUTOSAR.

---

## 1. QNX Thread Priorities

QNX uses a strict priority-based, preemptive scheduling model. In modern QNX versions (SDP 7.0 and 8.0), thread priorities range from **0 to 255**.

### Key Priority Levels:
*   **0**: Permanently reserved for the **idle thread** (runs only when no other threads are ready).
*   **10**: The default priority for standard, normal user applications.
*   **63**: The maximum priority allowed for an **unprivileged** user application. Attempting to go higher without root privileges or the `PROCMGR_AID_PRIORITY` ability will result in an `EPERM` error.
*   **253**: The highest priority generally available to highly critical application threads (assuming privileged access).
*   **254**: Typically reserved by the system for the **Clock Interrupt Service Thread (IST)**.
*   **255**: The absolute highest priority, permanently reserved for the **Inter-Processor Interrupt (IPI)** IST, which handles critical cross-core communication.

*(Note: Values like `-1` and `65535` are mathematically invalid priority parameters).*

---

## 2. QNX Clusters

In the QNX ecosystem, the term "Cluster" can have two different meanings. As an OS and systems developer, you will primarily deal with **Processor Clusters**.

### Processor Clusters (System Architecture)
A Processor Cluster is a **logical grouping of physical CPU cores**. This is especially relevant on modern Systems-on-a-Chip (SoC) that use ARM big.LITTLE architecture, where you might have 4 low-power "Efficiency" cores and 4 high-performance "Big" cores.

Key facts about QNX Processor Clusters:
1.  **A set of related CPU cores**: Clusters group physical processors together.
2.  **Defined by startup code in the BSP**: The Board Support Package configures and names these clusters before the OS is fully booted.
3.  **Specifies where threads are allowed to run**: Using CPU Partitioning or Thread Affinity, you can lock a specific thread (or an entire process) so it only executes on a designated cluster.

### Digital Instrument Clusters (Automotive)
In the automotive industry, "Cluster" refers to the digital dashboard display behind the steering wheel (speedometer, warnings). Because this is safety-critical, developers often use the QNX OS for Safety and the QNX Hypervisor to run this cluster on the same silicon chip as the Android Infotainment system, keeping them strictly isolated.

---

## 3. The Process Manager & Process Creation

In QNX, the Microkernel and the Process Manager are two distinct logical entities that are tightly coupled into a single module (binary) called `procnto` for performance. However, they have very strict divisions of responsibility:

### Microkernel Responsibilities
The Microkernel handles all fast, deterministic real-time operations, including:
*   **Thread Scheduling** (Note: QNX only schedules *threads*, it does not schedule *processes*!)
*   **Message Passing (IPC)**
*   **Interrupt Handling**
*   **Synchronization** (Mutexes, Condvars)

### Process Manager Responsibilities
The Process Manager handles the heavy-lifting of system resources and containers, including:
*   **Process Creation/Destruction** (via `spawn`, `exec`, `fork`)
*   **Memory Management**
*   **Grouping Threads into Processes** (creating the memory space container for the threads)
*   **Providing System Information** (hosting the `/proc` filesystem and namespace mapping)

### Memory Management & Virtual Addressing
Because QNX is a highly robust and isolated OS, it doesn't give a process direct access to physical RAM. Instead, the Process Manager provides memory addresses using **Virtual Memory** and the CPU's **Memory Management Unit (MMU)**:

1.  **Virtual Address Space Creation:** When the Process Manager creates a new process, it builds a brand new, isolated "Virtual Address Space" specifically for that process. To the process, it looks like it has a massive, empty, and contiguous block of memory all to itself.
2.  **MMU Mapping (Page Tables):** The Process Manager programs the hardware MMU with a "Page Table" for that process. This table acts as a translation dictionary. When your C++ code tries to read from a pointer, the hardware MMU intercepts it, looks at the Process Manager's table, and instantly translates it to the actual physical RAM chip address (which might be heavily fragmented).
3.  **Memory Protection:** Because the Process Manager strictly controls these MMU tables, it enforces **Memory Protection**. If Process A tries to access an address belonging to Process B, that address simply won't exist in Process A's page table. The MMU immediately blocks the access and throws a hardware fault, and the Process Manager kills the offending thread with a `SIGSEGV` (Segmentation Fault) before it can corrupt the other process.
4.  **Demand Paging:** To save physical RAM, the Process Manager often maps the virtual addresses without actually allocating physical RAM immediately. Only when the process actually tries to read or write to that virtual address does the Process Manager pause the thread, quickly grab a physical page of RAM, map it into the MMU, and resume the thread.

### Process Creation Mechanics
When you want to create a new process in QNX, you have three primary POSIX options: `fork()`, `exec()`, and `posix_spawn()`. However, their behavior in a microkernel environment is unique:

### 1. `fork()`
*   **What it does:** Creates an exact duplicate of the parent process, including its memory space, file descriptors, and state.
*   **QNX Consideration (AVOID):** In monolithic kernels like Linux, `fork()` is heavily optimized using "Copy-On-Write" (COW) memory paging. However, QNX does not aggressively optimize `fork()` in this manner. Calling `fork()` in a QNX system with a large memory footprint will physically copy megabytes of memory, making it incredibly slow and memory-intensive. **Using `fork()` is heavily discouraged in modern QNX design.**

### 2. `exec()` (e.g., `execv()`, `execl()`)
*   **What it does:** Completely replaces the current process image with a new executable program.
*   **QNX Consideration:** Usually, `exec()` is called by the child process immediately after a `fork()`. Because the `fork()` just wasted massive resources copying the parent's memory, only for `exec()` to instantly throw it all away and load a new binary, the `fork()/exec()` pattern is considered an anti-pattern in QNX.

### 3. `posix_spawn()`
*   **What it does:** Directly creates a new process and loads an executable into it, effectively combining the results of `fork()` and `exec()` into a single, atomic step.
*   **QNX Consideration (BEST PRACTICE):** This is the **preferred, most efficient, and fastest** way to create a new process in QNX. It tells the Process Manager (`procnto`) to directly allocate a fresh memory space and map the target executable into it, completely bypassing the massive overhead of duplicating the parent's memory space. Always use `posix_spawn()` or `spawn()` when launching independent binaries!

---

## 4. Device Files & Resource Managers

In QNX, as with most POSIX-compliant operating systems, "Everything is a file." This means that hardware devices and drivers map themselves into the standard filesystem path space using specialized processes called **Resource Managers**.

At its core, a **Resource Manager** is simply a user-space program that extends the operating system by:
1. **Creating and managing a name** in the pathname space (e.g., `/dev/ser1`).
2. **Providing a standard POSIX interface** for clients (handling `open()`, `read()`, `write()`, etc.).
3. **Abstracting an underlying resource**, which can be **hardware** (such as a serial port or a hard disk) or purely **software** (such as a message queue `/dev/mqueue` or shared memory `/dev/shmem`).

### How the C Library (libc) Interacts with the System
If a user-space application wants to interact with these resources, it doesn't execute custom system calls. Instead, it uses standard POSIX I/O functions (like `open()`, `read()`, and `write()`) on specific file paths. 

However, because the Resource Manager and Process Manager are completely independent user-space processes:
*   You **cannot call into their code directly** (that would cause a segmentation fault).
*   The C library functions **do not trigger monolithic kernel system calls** specific to those operations (e.g., there is no `sys_read` or `sys_fork` in the QNX kernel).

Instead, the QNX C library simply acts as a wrapper that translates POSIX calls into QNX Native Messages (`MsgSend`). For example:
*   Calling `read()` will build a message payload and send it directly to the **Resource Manager** that owns that file/device.
*   Calling `fork()` or `posix_spawn()` will build a message payload and send it directly to the **Process Manager** (`procnto`) to request the creation of a new memory container.

### Example: Serial Ports
If your application needs to read and write from a serial port (UART), the serial driver (Resource Manager) will typically register itself in the device directory as `/dev/ser1` (or `/dev/ser2`, etc.).

```c
// Example: Interacting with a serial port in QNX
int fd = open("/dev/ser1", O_RDWR);
if (fd != -1) {
    write(fd, "Hello Serial Device", 19);
    close(fd);
}
```

Behind the scenes, when you call `write()` on `/dev/ser1`, the QNX microkernel translates that POSIX call into a native QNX Message Passing payload and routes it directly to the Serial Resource Manager process handling that hardware!

### Pathname Resolution & The Prefix Tree
Because QNX Resource Managers are independent processes rather than kernel modules, the **Process Manager** is responsible for figuring out which Resource Manager owns which path. It does this using a **Prefix Tree**:

1.  **Ownership on Startup:** When the OS (`procnto`) starts, its Prefix Tree is completely empty. The Process Manager effectively owns the entire root `/` pathname space.
2.  **Dynamic Registration:** When a Resource Manager starts (e.g., the network driver), it registers its path prefix (e.g., `/dev/socket`) with the Process Manager.
3.  **Path Resolution:** When your application calls `open("/dev/ser1", ...)`, your C library secretly sends a resolution message to the Process Manager first. The Process Manager checks its Prefix Tree, finds the longest matching prefix (e.g., `/dev/ser1` registered by the serial driver), and tells your application exactly which Resource Manager process to contact.
4.  **Extreme Flexibility:** Because of this architecture, Resource Managers can be dynamically started and stopped at runtime, adopting or releasing portions of the pathname space on the fly. This makes QNX an incredibly flexible and modular system for extending capabilities without touching the kernel.

*(Note: Even if a custom filesystem mounts itself at the root `/`, the Process Manager is still the one maintaining the prefix tree and handling the initial pathname resolution!)*

### How QNX Differs from Monolithic Linux/Unix
If you are coming from a traditional Linux or Unix background, this pathname resolution mechanism is fundamentally different:

*   **Linux/Unix (Monolithic):** Pathname resolution is entirely handled **inside the kernel** by the Virtual File System (VFS). When an application calls `open()`, it triggers a system call that jumps into kernel space. The kernel parses the path, interacts directly with the monolithic file system or device drivers (which are also inside the kernel), and returns a file descriptor.
*   **QNX (Microkernel):** Pathname resolution is handled largely in **user-space** via Message Passing. The microkernel itself has absolutely no concept of files, paths, or directories. It only knows how to route messages. When an application calls `open()`, the C library sends a message to the Process Manager (to resolve the prefix) and then sends a follow-up message directly to the user-space Resource Manager (driver) that owns that path. The kernel simply acts as the message courier.

---

## 5. QNX Security & Permissions

Because QNX is POSIX-compliant, it utilizes standard **Unix-style permissions** for all files, directories, and devices (Resource Managers). 

### Permission Structure
Permissions are divided into three standard categories:
*   **User (Owner)**
*   **Group**
*   **Other (World)**

Each of these categories is assigned permissions for:
*   **Read (r)**
*   **Write (w)**
*   **Execute/Search (x)** (Execute for binaries, Search for traversing directories).

### Root Execution & The Boot Image
When a QNX system boots, everything launched directly from the **Image File System (IFS)** boot script runs as `root` (User ID `0`) by default. Running an entire system as root is a massive security vulnerability.

To drop privileges and run applications as standard users, you must change this default behavior using:
1.  **Launcher Programs:** Custom utilities that launch child processes with lower privileges.
2.  **`login` Utilities:** Standard Unix-style login prompts.
3.  **C APIs:** Using `setuid()`, `seteuid()`, and related standard C library calls within the application code to drop root privileges immediately after initialization.
4.  **`setuid` Executables:** Setting the SUID bit on the file permissions so the binary drops/changes permissions upon execution.

### Fine-Grained Privileges (`procmgr_ability`)
Instead of running a process entirely as `root` just to perform a single privileged action, QNX allows you to enforce the **Principle of Least Privilege** using the `procmgr_ability()` API.

This API allows an unprivileged application (or a root launcher program configuring a child process) to request or grant specific, fine-grained kernel abilities. For example:
*   `PROCMGR_AID_PRIORITY`: Allows an unprivileged thread to raise its priority above the default unprivileged ceiling of 63.
*   `PROCMGR_AID_IO`: Allows a process to request I/O privileges (via `ThreadCtl`) to access raw hardware registers.
*   `PROCMGR_AID_SPAWN_SETUID`: Allows a process to spawn children as a different user ID.

By using `procmgr_ability()`, you can run a custom hardware driver as a completely standard, unprivileged user, while selectively granting it only the exact `PROCMGR_AID_IO` ability it needs to function safely!

### The Danger of `qconn`
During development, you will heavily rely on a daemon called **`qconn`** (the QNX target agent). `qconn` allows the QNX Momentics IDE to deploy code, debug, and profile the target system. 

> [!WARNING]
> By default, `qconn` **runs as root and launches everything as root**. While this is extremely convenient for development and debugging, it completely bypasses the OS security model. **`qconn` should NEVER be running on a released production system!**

### Security Policies & System Integration
While QNX provides powerful mechanisms like Unix permissions and `procmgr_ability`, it also utilizes comprehensive **Security Policies** (like `secpol`). 

Why are System-Level Security Policies needed?
*   **The Developer's Blindspot:** Individual component developers often do not, and cannot, know the final deployment environment or use-case of the specific binary they are writing. 
*   **The Integrator's Responsibility:** The final responsibility for the safety and security of the entire device falls squarely on the **System Integrator** (the team assembling the final OS image), not the individual component developers.
*   **Centralized Control:** Security policies solve this by shifting the control of system security away from individual components and centralizing it at the **System Integration level**. 

By using security policies, the System Integrator can forcefully restrict exactly what resources a component can access (which files it can read, which channels it can attach to, which abilities it can request) without having to modify or trust the underlying component's code.

### How Security Policies are Defined
A QNX security policy starts its life as a **host-side text file**. This text file is written in a human-readable policy language that outlines all the security rules for the system. 

During the system build process, the QNX compiler takes this text file and compiles it into a highly optimized **target-side binary policy file** that the OS can parse instantly at runtime.

At a high level, this policy file primarily defines two things:
1.  **Security Types:** Abstract labels (like `network_driver_t`, `audio_app_t`, `secure_storage_t`) that are assigned to specific processes, files, or system channels.
2.  **Privileges:** The exact rules governing what each type is allowed to do. For example, it explicitly defines whether the `audio_app_t` type is allowed to send an IPC message to the `network_driver_t` type, or if it is allowed to request a specific `procmgr_ability`.

### Security Policy Software Components
Managing these security policies involves specific utilities spread across your development host and the QNX target:

**Host-Side Tools (Development Machine):**
*   `secpolgenerate`: A powerful tool that analyzes audit logs from a running test system to automatically generate a baseline text policy.
*   `secpolcompile`: A compiler utility that takes your human-readable text policy and compiles it down into the optimized binary policy file.

**Target-Side Tools (QNX System):**
*   `secpolpush`: A utility used to push (install) the compiled binary policy directly into the running QNX microkernel.
*   `secpol`: A diagnostic utility used on the target to examine the currently active binary policy for debugging purposes.
