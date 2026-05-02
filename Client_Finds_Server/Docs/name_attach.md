# Finding Servers in QNX (`name_attach` & `name_open`)

In our previous Message Passing examples, we used `ChannelCreate()` and `ConnectAttach()`. However, `ConnectAttach()` requires the client to explicitly know the **Process ID (PID)** and **Channel ID (CHID)** of the server. 

In a dynamic operating system, PIDs change every time an application runs. How is a client supposed to find a server without hardcoding PIDs?

The answer is **Global Name Resolution**.

---

## 1. The Server (`name_attach`)
Instead of just creating a raw channel, a server can "register" a string-based name with the QNX Process Manager.
**Signature:** `name_attach_t *name_attach(dispatch_t *dpp, const char *path, unsigned flags);`

*   **What it does:** It creates a channel automatically under the hood AND registers a string name (like `"my_audio_server"`) globally across the OS.
*   **The Return Struct:** It returns a pointer to a `name_attach_t` structure. This structure contains the raw `chid` (Channel ID), which the server uses in its `MsgReceive` loop just like normal!

## 2. The Client (`name_open`)
The client no longer needs to know the PID or CHID. It just asks the OS to find the server by its string name.
**Signature:** `int name_open(const char *name, int flags);`

*   **What it does:** It searches the OS for the registered name. If found, it automatically performs the `ConnectAttach` under the hood.
*   **The Return Value:** It returns a standard Connection ID (`coid`), exactly the same as `ConnectAttach` would!

## 3. Communication (`MsgSend`)
Once `name_open` succeeds, the name resolution is completely finished. The client uses the exact same `MsgSend` API as before.
Because `name_open` returns a standard `coid`, all IPC from that point forward is identical to raw channel IPC. It is just as fast and has zero overhead compared to `ConnectAttach`.

---

## Summary of the Workflow
1.  **Server:** `name_attach(NULL, "my_cool_server", 0)` -> Gets `attach->chid`.
2.  **Server:** Loops on `MsgReceive(attach->chid, ...)`
3.  **Client:** `coid = name_open("my_cool_server", 0)`
4.  **Client:** `MsgSend(coid, ...)`
5.  **Server:** Processes data and calls `MsgReply(rcvid, ...)`
6.  **Client:** `name_close(coid)`
7.  **Server:** `name_detach(attach, 0)`
