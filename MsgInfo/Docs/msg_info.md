# QNX Message Information (`MsgInfo` & `struct _msg_info`)

When a server receives a message from a client using `MsgReceive()`, it often needs to know **who** sent the message. Is the client authorized? What is their Process ID (PID) so we can look up their permissions? How many bytes did they send?

QNX provides this metadata via the `struct _msg_info` structure.

---

## 1. The `struct _msg_info`
This structure contains critical metadata about the incoming message and the client that sent it. The most commonly used fields are:

*   **`pid_t pid`**: The Process ID of the client that sent the message. (Useful for security checks and logging).
*   **`int tid`**: The Thread ID of the specific thread within the client process that called `MsgSend`.
*   **`uint32_t nd`**: The Node Descriptor. If this is `0`, the client is on the same machine as the server. If it's non-zero, the client is communicating over the network (via Qnet).
*   **`int msglen`**: The exact number of bytes the client sent in this specific message.

## 2. Two Ways to get Message Info

There are two ways a server can retrieve this information:

### Method A: Directly via `MsgReceive` (Most Common & Efficient)
The 4th argument of `MsgReceive` is a pointer to a `struct _msg_info`. If you provide it, the OS fills it out the moment the message arrives. This is highly efficient because it avoids a second OS kernel call.
```c
struct _msg_info info;
rcvid = MsgReceive(chid, &msg_buffer, sizeof(msg_buffer), &info);

printf("Message sent by PID: %d\n", info.pid);
```

### Method B: Using the `MsgInfo()` API
If you passed `NULL` as the 4th argument to `MsgReceive` because you didn't think you needed the info, but later decide you *do* need it, you can call the `MsgInfo()` function using the `rcvid`.
**Signature:** `int MsgInfo(int rcvid, struct _msg_info *info);`
```c
// We passed NULL initially
rcvid = MsgReceive(chid, &msg_buffer, sizeof(msg_buffer), NULL);

// Later in the code, we fetch the info using the rcvid
struct _msg_info info;
MsgInfo(rcvid, &info);

printf("Message sent by PID: %d\n", info.pid);
```

> [!TIP]
> **Performance Note:** It is almost always better to use **Method A**. The OS already has this information when it delivers the message. Calling `MsgInfo()` (Method B) requires a completely separate context switch into the microkernel just to fetch the data.
