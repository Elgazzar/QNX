# QNX Pulses (`MsgSendPulse`)

This document describes **Pulses** in QNX, which provide an asynchronous, non-blocking method of communication.

While `MsgSend` and `MsgReceive` provide synchronous (blocking) Client/Server communication, there are many scenarios where a client needs to notify a server without blocking (e.g., an interrupt handler telling a driver that data is ready, or a client sending a "fire and forget" event). This is where Pulses come in.

---

## What is a Pulse?
A Pulse is a tiny, non-blocking, unacknowledged message. 
*   **Non-blocking:** When you send a pulse, your thread does NOT go to sleep. It continues executing immediately.
*   **Unacknowledged:** The server does not (and cannot) reply to a pulse.
*   **Tiny Payload:** A pulse can only carry a small amount of data: an 8-bit `code` and a 32-bit `value`.
*   **System Queued:** If the server is busy, the OS queues the pulses so they aren't lost.

## Why use a Pulse instead of a Message?
You should choose a Pulse when you need **asynchronous, fire-and-forget notification** rather than synchronous data transfer:
1.  **Interrupt Service Routines (ISRs):** Hardware ISRs run at the kernel level and are strictly forbidden from blocking. You cannot use `MsgSend` in an ISR. Instead, ISRs fire pulses to instantly wake up user-space driver threads.
2.  **You cannot afford to wait:** If a high-priority thread just needs to trigger an event (like logging or shutting down), it shouldn't be paralyzed waiting for the server to call `MsgReply`.
3.  **System Events / Timers:** The OS uses pulses to notify your application of asynchronous events (e.g., POSIX timers expiring, or the "Death Pulse" when a process dies).
4.  **Low Overhead:** Sending an 8-bit code and a 32-bit integer is incredibly fast compared to setting up full message buffers and reply transactions.

## The `struct _pulse`
When a server receives a pulse, it is populated into the QNX standard `struct _pulse` (defined in `<sys/neutrino.h>`):
```c
struct _pulse {
    uint16_t type;     // _PULSE_TYPE
    uint16_t subtype;  // _PULSE_SUBTYPE
    int8_t   code;     // The 8-bit code you specify (must be >= _PULSE_CODE_MINAVAIL)
    uint8_t  zero[3];  // padding
    union sigval value; // The 32-bit data payload (can be an int or a pointer)
    int32_t  scoid;    // Server connection ID
};
```

---

## 1. Sending a Pulse (`MsgSendPulse`)
**Signature:** `int MsgSendPulse(int coid, int priority, int code, int value);`
**Description:** Sends a pulse over a connection.
**Parameters:**
*   `coid`: The Connection ID (from `ConnectAttach`).
*   `priority`: The priority at which the pulse should be delivered (often the same as the thread priority, e.g., 10).
*   `code`: An 8-bit application-specific code. To avoid conflicting with OS-reserved pulse codes, custom pulse codes should always start at `_PULSE_CODE_MINAVAIL`.
*   `value`: A 32-bit integer payload.

## 2. Receiving a Pulse
To receive a pulse, the server uses the exact same `MsgReceive` API it uses for normal messages!
**How does it tell the difference?**
When `MsgReceive` returns, you MUST check the return value (`rcvid`):
*   If `rcvid > 0`: It's a standard message from a client. You must `MsgReply`.
*   If `rcvid == 0`: **It's a PULSE!** You do not (and cannot) reply. The data in your receive buffer is a `struct _pulse`.

```c
rcvid = MsgReceive(chid, &pulse_struct, sizeof(pulse_struct), NULL);
if (rcvid == 0) {
    // We got a pulse!
    printf("Code: %d, Data: %d\n", pulse_struct.code, pulse_struct.value.sival_int);
}
```
