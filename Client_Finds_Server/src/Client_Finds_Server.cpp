#include <iostream>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h> // Required for name_attach and name_open
#include <string.h>
#include <unistd.h>

using namespace std;

// The globally resolvable string name for our server
#define ATTACH_POINT "my_qnx_server"

// Custom pulse code
#define MY_CUSTOM_PULSE _PULSE_CODE_MINAVAIL

// Custom message structure
typedef struct {
    uint16_t type;
    char data[256];
} my_msg_t;

// =========================================================
// Server Thread (Receiver)
// =========================================================
void* server_thread(void* arg) {
    name_attach_t *attach;
    my_msg_t msg;
    int rcvid;

    cout << "[Server] Registering global name: \"" << ATTACH_POINT << "\"" << endl;
    
    // 1. Register the name globally. 
    // This creates a channel for us automatically!
    if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
        cerr << "[Server] Failed to name_attach" << endl;
        return NULL;
    }

    cout << "[Server] Name registered successfully! Listening..." << endl;

    // 2. Loop to process messages
    while (true) {
        // We use the raw channel ID (chid) stored inside the attach struct
        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

        if (rcvid == -1) break;

        // name_attach channels also receive system pulses (like when clients connect/disconnect)
        if (rcvid == 0) {
            struct _pulse* pulse = (struct _pulse*)&msg;
            if (pulse->code == MY_CUSTOM_PULSE) {
                cout << "\n[Server] Received a custom PULSE! Value: " << pulse->value.sival_int << endl;
            } else if (pulse->code == _PULSE_CODE_DISCONNECT) {
                cout << "[Server] Received a disconnect pulse from the OS." << endl;
            } else {
                cout << "[Server] Received a system pulse. Code: " << (int)pulse->code << endl;
            }
        } else {
            // It's a standard message from a client
            cout << "\n[Server] Received message: \"" << msg.data << "\"" << endl;

            // 3. Reply to the client
            const char* reply = "I received your message via my global name!";
            MsgReply(rcvid, 0, reply, strlen(reply) + 1);

            // Exit if commanded
            if (strcmp(msg.data, "shutdown") == 0) {
                cout << "[Server] Shutdown command received. Exiting loop." << endl;
                break;
            }
        }
    }

    // Clean up
    name_detach(attach, 0);
    cout << "[Server] Exiting cleanly." << endl;
    return NULL;
}

// =========================================================
// Client Thread (Sender)
// =========================================================
void* client_thread(void* arg) {
    int coid;
    my_msg_t msg;
    char reply[256];

    // Give server time to register the name with the OS
    sleep(1);

    cout << "\n[Client] Looking for server named: \"" << ATTACH_POINT << "\"..." << endl;

    // 1. Find the server by name! No PID or CHID required!
    coid = name_open(ATTACH_POINT, 0);
    if (coid == -1) {
        cerr << "[Client] Failed to name_open. Is the server running?" << endl;
        return NULL;
    }

    cout << "[Client] Connected successfully! I got a standard Connection ID: " << coid << endl;

    // --- NEW: Send a pulse ---
    cout << "\n[Client] Sending a fast, non-blocking pulse..." << endl;
    MsgSendPulse(coid, 10, MY_CUSTOM_PULSE, 777);
    sleep(1); // Give server a moment to print the pulse before we send the next message


    // 2. Send a message
    // Now that we have a coid, we use MsgSend exactly like standard Channel IPC!
    msg.type = 1;
    strcpy(msg.data, "Hello! I found you by your string name!");
    
    cout << "[Client] Sending message..." << endl;
    MsgSend(coid, &msg, sizeof(msg), reply, sizeof(reply));
    cout << "[Client] Server replied: \"" << reply << "\"" << endl;

    sleep(1);

    // Send shutdown message
    strcpy(msg.data, "shutdown");
    cout << "\n[Client] Sending shutdown message..." << endl;
    MsgSend(coid, &msg, sizeof(msg), reply, sizeof(reply));

    // Clean up
    name_close(coid);
    cout << "[Client] Exiting cleanly." << endl;
    return NULL;
}

// =========================================================
// MAIN
// =========================================================
int main() {
    pthread_t server_tid, client_tid;

    cout << "--- QNX Name Attach/Open Example ---" << endl;

    pthread_create(&server_tid, NULL, server_thread, NULL);
    pthread_create(&client_tid, NULL, client_thread, NULL);

    pthread_join(client_tid, NULL);
    pthread_join(server_tid, NULL);

    cout << "\n--- Example Complete ---" << endl;
    return 0;
}
