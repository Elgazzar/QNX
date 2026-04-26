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

---

## The Core APIs

### 1. `ChannelCreate` (Server Side)
**Signature:** `int ChannelCreate(unsigned flags);`
**Description:** Creates a communication channel that clients can connect to. Returns a Channel ID (`chid`).

### 2. `ConnectAttach` (Client Side)
**Signature:** `int ConnectAttach(uint32_t nd, pid_t pid, int chid, unsigned index, int flags);`
**Description:** Creates a connection (`coid`) from the client to the server's channel (`chid`). 
*Note: In complex multi-process systems, you typically use `name_attach` and `name_open` to find channels by string names instead of raw IDs.*

### 3. `MsgReceive` (Server Side)
**Signature:** `int MsgReceive(int chid, void *rmsg, int rbytes, struct _msg_info *info);`
**Description:** The server calls this to wait for a message. 
*   If no client has sent a message yet, the server goes to sleep.
*   When a message arrives, it copies the client's data into the `rmsg` buffer.
*   **Returns:** A Receive ID (`rcvid`). This ID is mathematically linked to the specific client that sent the message. The server *must* use this exact `rcvid` to reply to that specific client.

### 4. `MsgSend` (Client Side)
**Signature:** `int MsgSend(int coid, const void *smsg, int sbytes, void *rmsg, int rbytes);`
**Description:** The client calls this to send data to the server and wait for a reply.
*   `smsg`: The buffer containing the data you want to send.
*   `rmsg`: The buffer where the OS will place the server's reply data once it arrives.
*   **Blocking:** The calling thread is suspended until the server explicitly replies.

### 5. `MsgReply` (Server Side)
**Signature:** `int MsgReply(int rcvid, int status, const void *msg, int nbytes);`
**Description:** The server calls this to send the results back to the client and wake the client up.
*   `rcvid`: The specific ID returned by `MsgReceive` so the OS knows which sleeping client to wake up.
*   `msg`: The buffer containing the reply data.
