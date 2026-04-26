# QNX Message Passing

This document describes the core of the QNX Neutrino RTOS microkernel architecture: **Synchronous Message Passing**. 

Unlike condition variables or mutexes, message passing doesn't just synchronize threads—it allows them to safely exchange data. It is the primary way that applications communicate with drivers, the file system, and each other in QNX.

## The Client/Server Model
Message passing in QNX always follows a strict Client/Server model:
1.  **The Server** creates a channel and waits for messages.
2.  **The Client** connects to the channel and sends a message.
3.  **The Client Blocks (Sleeps):** The moment the client sends the message, it is put to sleep by the OS.
4.  **The Server Replies:** The server receives the data, processes it, and sends a reply.
5.  **The Client Wakes Up:** Only after the server replies does the client wake up and receive the response data.

> [!NOTE] 
> **Adaptive AUTOSAR (ara::com)**
> If you are running an Adaptive AUTOSAR stack on a QNX operating system, this exact message passing mechanism is heavily used under the hood! The local IPC (Inter-Process Communication) binding for `ara::com` relies on QNX `MsgSend`, `MsgReceive`, and `MsgReply` for fast communication between different Adaptive Applications (AAs) on the same ECU. However, the AUTOSAR middleware abstracts this entirely. You interact with generated C++ Proxies and Skeletons, and the middleware translates those calls into native QNX message passing payloads automatically.

---

## The Core APIs

### 1. `ChannelCreate` (Server Side)
**Signature:** `int ChannelCreate(unsigned flags);`
**Description:** Creates a communication channel that clients can connect to. Returns a Channel ID (`chid`), or `-1` on error.
**Parameters:**
*   `flags`: Controls channel behavior. Common values:
    *   `0`: Default channel, accessible by any process.
    *   `_NTO_CHF_PRIVATE`: Restricts access so only threads within the same process can connect to this channel.

### 2. `ConnectAttach` (Client Side)
**Signature:** `int ConnectAttach(uint32_t nd, pid_t pid, int chid, unsigned index, int flags);`
**Description:** Creates a connection (`coid`) from the client to the server's channel (`chid`). Returns a Connection ID, or `-1` on error.
**Parameters:**
*   `nd`: Node descriptor. Use `0` for the local machine.
*   `pid`: The Process ID of the server process.
*   `chid`: The Channel ID (`chid`) of the server's channel.
*   `index`: Use `_NTO_SIDE_CHANNEL` to create a side channel connection.
*   `flags`: Additional flags. Typically `0`.

*Note: In complex multi-process systems, you typically use `name_attach` and `name_open` to find channels by string names instead of raw IDs.*

### 3. `MsgReceive` (Server Side)
**Signature:** `int MsgReceive(int chid, void *rmsg, int rbytes, struct _msg_info *info);`
**Description:** The server calls this to wait for a message. Puts the server into **Receive Blocked** state until a client sends a message.
**Parameters:**
*   `chid`: The Channel ID created by `ChannelCreate`. The server listens on this channel.
*   `rmsg`: A pointer to the buffer where the incoming message data will be copied into.
*   `rbytes`: The size of the `rmsg` buffer in bytes. Typically `sizeof(your_msg_struct)`.
*   `info`: A pointer to a `_msg_info` struct for extra message metadata (sender's PID, etc.). Pass `NULL` if not needed.
*   **Returns:** A Receive ID (`rcvid`). This ID is mathematically linked to the specific client that sent the message. The server *must* use this exact `rcvid` to reply to that specific client.

### 4. `MsgSend` (Client Side)
**Signature:** `int MsgSend(int coid, const void *smsg, int sbytes, void *rmsg, int rbytes);`
**Description:** The client calls this to send data to the server and wait for a reply. Puts the client into **Reply Blocked** state until the server calls `MsgReply`.
**Parameters:**
*   `coid`: The Connection ID returned by `ConnectAttach`. Identifies which server to send to.
*   `smsg`: A pointer to the buffer containing the message data you want to send.
*   `sbytes`: The size of the send buffer in bytes. Typically `sizeof(your_msg_struct)`.
*   `rmsg`: A pointer to the buffer where the server's reply data will be stored once it arrives.
*   `rbytes`: The size of the reply buffer in bytes. Typically `sizeof(your_reply_struct)`.
*   **Blocking:** The calling thread is suspended until the server explicitly replies.

### 5. `MsgReply` (Server Side)
**Signature:** `int MsgReply(int rcvid, int status, const void *msg, int nbytes);`
**Description:** The server calls this to send the results back to the client and wake it up from **Reply Blocked** state.
**Parameters:**
*   `rcvid`: The Receive ID returned by `MsgReceive`. The OS uses this to identify exactly which sleeping client to wake up.
*   `status`: An integer status code returned to the client (returned as the return value of `MsgSend`). Use `EOK` (0) for success.
*   `msg`: A pointer to the buffer containing the reply data to send back to the client.
*   `nbytes`: The size of the reply buffer in bytes. Typically `sizeof(your_reply_struct)`.
