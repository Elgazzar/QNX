# Death Pulse Documentation (QNX)

This document describes the mechanics and APIs used in the `Death_Pulse.cpp` example. This code demonstrates how a process can monitor the QNX Neutrino RTOS for the termination (death) of any other processes in the system.

## Overview
In QNX, the Process Manager (`procmgr`) can notify a monitoring process whenever any process terminates. This is achieved by registering for a **Process Death Event** using a communication mechanism called a **Pulse**. A pulse is a tiny, fixed-size, non-blocking message that is highly optimized for fast, asynchronous system notifications.

---

## Step-by-Step Flow & APIs Used

### 1. Creating a Communication Channel
```c
chid = ChannelCreate( _NTO_CHF_PRIVATE );
```
A channel (`chid`) is created for receiving messages and pulses. The `_NTO_CHF_PRIVATE` flag ensures that only threads within this specific process can connect to it. This adds security by preventing external rogue processes from sending fake death notifications to our application.

### 2. Attaching to the Channel
```c
coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );
```
To ask the OS to send us pulses, we must first create a connection (`coid`) to our own channel. This connection ID will be embedded in the event request we send to the OS.

### 3. Initializing the Pulse Event
```c
SIGEV_PULSE_INIT( &ev, coid, 10, 1, 0 );
SIGEV_MAKE_UPDATEABLE(&ev);
```
*   **`SIGEV_PULSE_INIT`**: Configures a `sigevent` structure to deliver a pulse. It specifies:
    *   The connection ID (`coid`) to deliver it to.
    *   The priority of the pulse (10).
    *   A custom application code (`1`), which we use to verify the pulse type later.
    *   A default value (`0`).
*   **`SIGEV_MAKE_UPDATEABLE`**: This is a critical step. By marking the event as "updateable," we grant the OS Process Manager permission to overwrite the `value` field of the pulse before delivering it. **This is how the OS tells us *which* process died**—it replaces the `0` with the Process ID (PID) of the dead process.

### 4. Registering for Notifications
```c
MsgRegisterEvent(&ev, SYSMGR_COID);
procmgr_event_notify( PROCMGR_EVENT_PROCESS_DEATH, &ev );
```
*   **`MsgRegisterEvent`**: Securely registers our event structure with the system manager connection (`SYSMGR_COID`), confirming we have permission to receive these system-level events.
*   **`procmgr_event_notify`**: This function actually places the request with the OS. It tells the Process Manager: *"Whenever the system event `PROCMGR_EVENT_PROCESS_DEATH` occurs, please trigger the pulse defined in `ev`."*

### 5. The Receiving Loop
```c
ret = MsgReceivePulse( chid, &pulse, sizeof pulse, NULL );
```
The program enters an infinite `while(1)` loop, blocking on `MsgReceivePulse`. 

**What happens behind the scenes:**
1. A random process on the system crashes or gracefully exits.
2. The QNX Process Manager detects the death.
3. It retrieves our registered pulse event.
4. It updates the pulse's value (`pulse.value.sival_int`) to be the **PID** of the newly deceased process.
5. It fires the pulse down our connection (`coid`).
6. `MsgReceivePulse` unblocks. Our code checks that the `pulse.code` is `1` (as we defined in `SIGEV_PULSE_INIT`) and prints out the dead process's PID!
