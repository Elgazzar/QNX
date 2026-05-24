# QNX Event Delivery

In the QNX Neutrino RTOS, events (`struct sigevent`) are the primary mechanism for asynchronous notifications. They notify a thread or process that a specific condition has occurred, such as a timer expiring, an interrupt firing, or a message arriving.

## Types of Events

A `sigevent` structure can be configured to deliver different types of notifications, specified by the `sigev_notify` field:

1. **Pulse (`SIGEV_PULSE`)**: Delivers a fixed-size, non-blocking message (pulse) to a channel. This is the most common and recommended way to receive events in QNX because it integrates perfectly with the standard message-passing loop (`MsgReceive`).
2. **Signal (`SIGEV_SIGNAL` / `SIGEV_SIGNAL_CODE` / `SIGEV_SIGNAL_THREAD`)**: Sends a POSIX signal to a process or a specific thread.
3. **Thread (`SIGEV_THREAD`)**: Spawns a new thread to execute a specified notification function.
4. **Interrupt (`SIGEV_INTR`)**: Used to unblock an `InterruptWait()` call (typically used in interrupt handlers).
5. **None (`SIGEV_NONE`)**: No notification is sent.

## Initializing Events

QNX provides several handy macros in `<sys/siginfo.h>` to easily initialize the `sigevent` structure for common notification types:

### `SIGEV_PULSE_INIT(evp, coid, priority, code, value)`
Initializes the event to deliver a pulse.
* **`evp`**: Pointer to the `struct sigevent`.
* **`coid`**: The connection ID to send the pulse over (usually obtained via `ConnectAttach`).
* **`priority`**: The priority of the pulse (e.g., `SIGEV_PULSE_PRIO_INHERIT`).
* **`code`**: An application-defined pulse code (should be within `_PULSE_CODE_MINAVAIL` and `_PULSE_CODE_MAXAVAIL`).
* **`value`**: An optional integer/pointer value to send along with the pulse.

### `SIGEV_INTR_INIT(evp)`
Initializes the event for interrupt handling.
* **`evp`**: Pointer to the `struct sigevent`.
* Used specifically with `InterruptWait()` or `InterruptAttachEvent()` to unblock a thread waiting for a hardware interrupt.

### `SIGEV_SIGNAL_INIT(evp, signo)`
Initializes the event to deliver a POSIX signal.
* **`evp`**: Pointer to the `struct sigevent`.
* **`signo`**: The POSIX signal number to deliver (e.g., `SIGUSR1`).

## Example 1: Timer Pulse Delivery

A frequent use case for event delivery is configuring a timer to send a pulse event upon expiration. The typical workflow is:

1. **Create a Channel**: Use `ChannelCreate()` to create a channel to receive the event.
2. **Establish a Connection**: Use `ConnectAttach()` to connect to the channel.
3. **Initialize the Event**: Use the `SIGEV_PULSE_INIT()` macro to configure the `sigevent` to deliver a pulse over the connection.
4. **Configure the Source**: For example, create a timer with `timer_create()` passing the event, and arm it with `timer_settime()`.
5. **Wait for the Event**: Block on `MsgReceive()` to receive the pulse.

## Secure Event Delivery with `MsgRegisterEvent`

In modern QNX, it is highly recommended to use **registered events** for security in client-server communications. This ensures that only authorized servers can deliver an event to a client.

The standard client-server event workflow is:
1. **Initialize the Event**: The client initializes a `sigevent` (e.g., as a pulse).
2. **Register the Event**: The client calls `MsgRegisterEvent(&event, server_coid)`, specifying the connection ID to the server. The kernel replaces the contents of the `sigevent` structure with a secure handle.
3. **Send the Handle**: The client passes this secure handle to the server using `MsgSend()`.
4. **Deliver the Event**: The server stores the client's `rcvid` and the event handle. It replies to the client to unblock it. Later, when the condition is met, the server calls `MsgDeliverEvent(rcvid, &event)` to trigger the client's notification.

See `src/Event_Delivery.cpp` for a complete working example of a client using `MsgRegisterEvent` and `MsgSend` to register an event with a server thread.
