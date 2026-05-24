# Reading Fragmented Messages (`MsgRead` & `MsgReadv`)

When writing a robust QNX Server, it is extremely dangerous to assume you know how large an incoming message will be. If a client sends a 10MB message and your `MsgReceive` buffer is only 1MB, the kernel will truncate the message and you will lose data. If you blindly allocate 10MB for every `MsgReceive`, you will waste massive amounts of RAM.

## The Two-Step Receive
To solve this, QNX servers often use a "Two-Step" receive process:

### Step 1: Catch the Header (`MsgReceive`)
Every protocol should have a fixed-size header (e.g., 16 bytes). The server sets up a tiny `MsgReceive` buffer just large enough to catch this header.

```c
custom_header_t header;
// The kernel will only copy sizeof(header) bytes from the client into our memory.
rcvid = MsgReceive(chid, &header, sizeof(header), NULL);
```

### Step 2: Read the Payload (`MsgRead`)
By inspecting the `header`, the server now knows exactly how much data the client is trying to send. The server can dynamically allocate exactly the right amount of memory, and then use `MsgRead` to go back and grab the rest of the data.

**Signature:** `int MsgRead(int rcvid, void *msg, int bytes, int offset);`
*   `rcvid`: The Receive ID of the client (from `MsgReceive`).
*   `msg`: Your newly allocated buffer.
*   `bytes`: The size of the payload.
*   `offset`: Where to start reading from the client's original message. If the header was 16 bytes, the offset is 16.

```c
// We know from the header we need 1000 bytes
char* payload = malloc(1000);

// Read the remaining 1000 bytes, skipping the 16 byte header we already read
MsgRead(rcvid, payload, 1000, sizeof(header));
```

> [!TIP]
> **What is the Client doing during this?**
> Absolutely nothing! The client is still sound asleep, completely blocked on its `MsgSend`. It has no idea the server is reading its memory in pieces.

> [!NOTE]
> **What is `MsgReadv`?**
> `MsgReadv` is exactly the same concept as `MsgRead`, except instead of reading the client's payload into a single flat buffer (`void *msg`), it scatters the payload into an IOV Array on the server side!
