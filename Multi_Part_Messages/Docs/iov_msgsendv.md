# Multi-Part Messages (Scatter/Gather I/O)

In standard IPC using `MsgSend()`, you must send a **single, contiguous block of memory**. 

## The Problem
Imagine you are building a network protocol or a file server. You want to send a **Header** (a small `struct`) and a **Payload** (a 1MB file buffer). Because `MsgSend` only accepts one pointer, you would be forced to allocate a massive 1MB+ chunk of memory, `memcpy` the header into the front, and `memcpy` the payload into the back, just to send it.
This wastes CPU time (copying) and RAM (double allocation).

## The Solution: IOVs (I/O Vectors)
QNX supports **Scatter/Gather I/O**. Instead of giving the OS a single block of memory, you give it an array of "pointers and lengths" called **IOVs** (`iov_t`).

Under the hood, an `iov_t` structure is very simple:
```c
typedef struct {
    void   *iov_base;   // A pointer to a block of memory
    size_t  iov_len;    // The size of that block in bytes
} iov_t;
```

An **IOV Array** is just an array of these structures (e.g., `iov_t my_array[3]`). You are essentially handing the OS a "treasure map" of where your data is located.
If you have a 16-byte Header on the stack, a 1000-byte Payload on the heap, and a 4-byte Checksum somewhere else, you just fill out an IOV array with 3 elements pointing to those locations.

The OS kernel will "gather" the data directly from these separate memory locations and transmit them as one seamless, continuous stream of bytes. **Zero memory copying required by your application!**

---

## 1. Setting up an IOV (`SETIOV`)
An `iov_t` (defined in `<unistd.h>`) simply holds a memory address and a length.
QNX provides a convenient macro called `SETIOV` to configure them:

```c
// Signature: SETIOV(iov_t *iov, void *addr, size_t len);

iov_t my_iovs[2];

// Part 1: The Header
SETIOV(&my_iovs[0], &my_header_struct, sizeof(my_header_struct));

// Part 2: The Payload
SETIOV(&my_iovs[1], payload_buffer, 1024);
```

## 2. Sending Vectors (`MsgSendv`)
Instead of `MsgSend`, you use the vector-based API `MsgSendv`.

**Signature:** `int MsgSendv(int coid, const iov_t *smsg, int sparts, const iov_t *rmsg, int rparts);`

*   `smsg`: The array of IOVs you want to send.
*   `sparts`: How many parts (IOVs) are in your send array.
*   `rmsg`: The array of IOVs you want to receive the reply into.
*   `rparts`: How many parts are in your receive array.

> [!TIP]
> **What if the receiver doesn't use IOVs?**
> The receiver doesn't need to know you sent the data in pieces! The receiver can just use standard `MsgReceive` with a large, flat buffer. The QNX kernel will automatically concatenate your IOV pieces together as it writes them into the receiver's flat buffer!

## Other Variations
Because you often want to mix and match vectors and flat buffers, QNX provides convenience wrappers:
*   `MsgSendv()`: Send an IOV array, receive into an IOV array.
*   `MsgSendvs()`: Send an IOV array, receive into a flat String/buffer.
*   `MsgSendsv()`: Send a flat String/buffer, receive into an IOV array.
