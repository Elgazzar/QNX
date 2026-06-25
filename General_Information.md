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

*This section has been extracted to its own document. Please see [Process_Manager.md](file:///c:/QNX/ide-8.0-workspace/Process_Manager.md).*

---

## 4. Device Files & Resource Managers

*This section has been extracted to its own document. Please see [Resources_Manger.md](file:///c:/QNX/ide-8.0-workspace/Resources_Manger.md).*

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
