# Process Creation in POSIX (and QNX)

This document describes the three primary mechanisms used for process creation in a POSIX-compliant operating system like QNX Neutrino RTOS: `fork()`, the `exec()` family, and `posix_spawn()`.

## 1. `fork()`

**Signature:**
```c
pid_t fork(void);
```

**Description:**
`fork()` is the traditional UNIX way to create a new process. It creates a new process (the "child") by duplicating the calling process (the "parent").
*   The child process is an exact copy of the parent process, including its memory space, file descriptors, and CPU state, but it has a unique Process ID (PID).
*   `fork()` returns twice: once in the parent (returning the PID of the child) and once in the child (returning `0`).
The Child Process reaches the if statement, sees that pid == 0, and executes the else if block.
The Parent Process reaches the same if statement at roughly the same time, sees that pid > 0, and executes the else block.

**Drawbacks (Especially in QNX/RTOS):**
*   **Performance:** Copying the entire memory space of a large parent process is extremely expensive, even with "Copy-on-Write" optimizations.
*   **Multithreading:** Calling `fork()` in a multithreaded program is dangerous. Only the thread that called `fork()` is duplicated in the child process, which can lead to deadlocks if other threads held mutexes.

## 2. `exec()` Family

**Signatures (Examples):**
```c
int execl(const char *path, const char *arg0, ... /*, (char *)0 */);
int execv(const char *path, char *const argv[]);
// (Other variants include execle, execve, execlp, execvp)
```

**Description:**
The `exec` family of functions replaces the current process image with a new process image. 
*   It loads a new executable from the file system into the current process's memory space.
*   If `exec` is successful, it **never returns**. The newly loaded program starts executing from its `main()` function.
*   The Process ID (PID) remains the same.

**The `fork()` + `exec()` combo:**
Traditionally, to spawn a completely new program, you first `fork()` to create a child, and then immediately call `exec()` within the child to replace it with the new executable. Because of the drawbacks of `fork()`, this two-step process is highly inefficient.

## 3. `posix_spawn()`

**Signature:**
```c
int posix_spawn(pid_t *restrict pid, const char *restrict path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *restrict attrp,
                char *const argv[restrict], char *const envp[restrict]);
```

**Description:**
`posix_spawn()` is the modern, POSIX-standard replacement for the `fork()` + `exec()` combination. It creates a new process and executes a specified file directly, without needing to duplicate the parent process's memory space first.

**Why it is preferred in QNX:**
1.  **Speed:** It avoids the massive overhead of `fork()`.
2.  **Safety:** It is perfectly safe to use in multithreaded applications.
3.  **Control:** Through `file_actions` and `attrp`, you can easily redirect standard input/output, change scheduling policies, and set signal masks for the child process before it starts.
