# QNX Shared Memory

Shared memory is one of the fastest and most efficient forms of Inter-Process Communication (IPC) available in QNX. It allows multiple, independent processes to map a common segment of physical memory into their respective virtual address spaces. 

Because the memory is shared directly, processes can read and write data just like normal local variables. This completely avoids the overhead of data copying that occurs with message passing or pipes.

## Common Usage

Shared memory is typically the best choice for:
*   **High-Bandwidth Data**: Transferring large volumes of data, such as uncompressed video frames, audio buffers, or high-frequency sensor readings.
*   **Global State / Blackboards**: Maintaining a common dataset that must be visible to several processes simultaneously.

**Important Note:** Because shared memory allows true concurrent access, you almost always need to implement **synchronization primitives** (like POSIX Mutexes or Semaphores initialized with the `PTHREAD_PROCESS_SHARED` attribute) inside the shared memory block to prevent race conditions.

---

## Key POSIX APIs

QNX relies on standard POSIX APIs for shared memory management. Under QNX, shared memory objects appear as files in the virtual `/dev/shmem` directory.

### 1. `shm_open()`
Creates a new shared memory object or opens an existing one.
```c
int shm_open(const char *name, int oflag, mode_t mode);
```
*   **`name`**: The name of the object. It must start with a slash (e.g., `/my_memory`).
*   **`oflag`**: Flags indicating how to open the object (e.g., `O_CREAT | O_RDWR` to create it for reading and writing).
*   **`mode`**: File permissions (e.g., `0666`), utilized only if `O_CREAT` is specified.
*   **Returns**: A file descriptor, or `-1` on error.

### 2. `ftruncate()`
Sets or adjusts the size of the shared memory object. When a shared memory object is newly created via `shm_open()`, its size is 0 bytes. You must call `ftruncate()` to allocate physical memory.
```c
int ftruncate(int fd, off_t length);
```
*   **`fd`**: The file descriptor returned by `shm_open()`.
*   **`length`**: The desired size of the shared memory region in bytes.

### 3. `mmap()`
Maps the shared memory object into the calling process's virtual address space.
```c
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off);
```
*   **`addr`**: Suggested mapping address (usually `NULL` to let the OS pick the address).
*   **`len`**: The size of the memory to map (usually matches `length` from `ftruncate`).
*   **`prot`**: Memory protection flags (e.g., `PROT_READ | PROT_WRITE`).
*   **`flags`**: Mapping visibility (almost always `MAP_SHARED` for IPC).
*   **`fd`**: The file descriptor.
*   **`off`**: Offset within the shared memory object (usually `0`).
*   **Returns**: A pointer to the mapped memory, or `MAP_FAILED` on error.

### 4. `close()`
Closes the file descriptor. 
```c
int close(int fd);
```
*   **Note**: Closing the file descriptor does **not** unmap the memory, nor does it destroy the shared memory object in `/dev/shmem`. It simply frees the file descriptor resource in your process. You should `close()` the fd as soon as `mmap()` completes successfully.

### 5. `munmap()`
Unmaps the shared memory from the process's virtual address space.
```c
int munmap(void *addr, size_t len);
```

### 6. `shm_unlink()`
Marks the shared memory object for deletion and removes its name from `/dev/shmem`. 
```c
int shm_unlink(const char *name);
```
*   **Note**: The physical memory is not actually reclaimed by the system until *all* processes have unmapped it via `munmap()`. However, once unlinked, no new processes can `shm_open()` it by name.
