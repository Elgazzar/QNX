#include <iostream>
#include <sys/neutrino.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

using namespace std;

#define MY_PULSE_CODE (_PULSE_CODE_MINAVAIL + 5)
#define MSG_TYPE_REGISTER 1

// Message structure for communication between client and server
struct ClientMessage {
    uint16_t type;
    struct sigevent event;
};

int server_chid; // Global channel ID for simplicity in this example

// --- SERVER THREAD ---
void* server_thread(void* arg) {
    server_chid = ChannelCreate(0);
    if (server_chid == -1) {
        cerr << "[Server] Failed to create channel." << endl;
        return NULL;
    }
    cout << "[Server] Started, chid = " << server_chid << endl;

    ClientMessage msg;
    struct _msg_info info;

    while (true) {
        int rcvid = MsgReceive(server_chid, &msg, sizeof(msg), &info);
        if (rcvid == -1) break;
        
        if (rcvid == 0) {
            cout << "[Server] Received unexpected pulse." << endl;
            continue;
        }

        if (msg.type == MSG_TYPE_REGISTER) {
            cout << "[Server] Received event registration request." << endl;
            
            // 1. Reply to unblock the client. The client can now continue its work.
            //    We keep the rcvid and the registered event handle to use later.
            MsgReply(rcvid, 0, NULL, 0);

            // 2. Simulate some asynchronous work happening...
            cout << "[Server] Simulating work for 2 seconds..." << endl;
            sleep(2);
            
            // 3. Deliver the event to the client using the registered handle.
            //    Note: We use the saved rcvid from the client's MsgSend.
            cout << "[Server] Work complete! Delivering event to client..." << endl;
            if (MsgDeliverEvent(rcvid, &msg.event) == -1) {
                cerr << "[Server] MsgDeliverEvent failed: " << strerror(errno) << endl;
            }
            break; // End the server for this example
        } else {
            MsgError(rcvid, ENOSYS);
        }
    }
    return NULL;
}

// --- CLIENT (MAIN THREAD) ---
int main() {
    // Start the server thread
    pthread_t tid;
    pthread_create(&tid, NULL, server_thread, NULL);
    sleep(1); // Give server a moment to start and create its channel

    // 1. Create a channel for the client to receive the event (pulse)
    int client_chid = ChannelCreate(0);
    int client_coid = ConnectAttach(0, 0, client_chid, _NTO_SIDE_CHANNEL, 0);

    // 2. Connect to the server
    int server_coid = ConnectAttach(0, 0, server_chid, _NTO_SIDE_CHANNEL, 0);

    // 3. Initialize the event. We want the server to send us a pulse.
    struct sigevent event;
    SIGEV_PULSE_INIT(&event, client_coid, SIGEV_PULSE_PRIO_INHERIT, MY_PULSE_CODE, 0);

    // 4. Register the event with the kernel.
    //    We specify 'server_coid' so only THIS server is allowed to deliver the event.
    //    The kernel replaces the contents of 'event' with a secure handle.
    cout << "[Client] Registering event with the kernel..." << endl;
    if (MsgRegisterEvent(&event, server_coid) == -1) {
        cerr << "[Client] MsgRegisterEvent failed: " << strerror(errno) << endl;
        return -1;
    }

    // 5. Send the registered event handle to the server via MsgSend
    ClientMessage msg;
    msg.type = MSG_TYPE_REGISTER;
    msg.event = event;

    cout << "[Client] Sending event handle to server via MsgSend..." << endl;
    if (MsgSend(server_coid, &msg, sizeof(msg), NULL, 0) == -1) {
        cerr << "[Client] MsgSend failed: " << strerror(errno) << endl;
        return -1;
    }

    cout << "[Client] MsgSend returned. Waiting for event from server..." << endl;

    // 6. Wait for the server to deliver the pulse
    struct _pulse pulse;
    int rcvid = MsgReceive(client_chid, &pulse, sizeof(pulse), NULL);
    
    if (rcvid == 0 && pulse.code == MY_PULSE_CODE) {
        cout << "[Client] Successfully received pulse from server!" << endl;
    } else {
        cout << "[Client] Received something else." << endl;
    }

    // Cleanup
    ConnectDetach(server_coid);
    ConnectDetach(client_coid);
    ChannelDestroy(client_chid);
    pthread_join(tid, NULL);

    return 0;
}
