#include <iostream>
#include <pthread.h>
#include <sys/neutrino.h>
#include <string.h>
#include <unistd.h>

using namespace std;

// Global channel ID
int server_chid;

// =========================================================
// Server Thread (Receiver)
// =========================================================
void* server_thread(void* arg) {
    int rcvid;
    char msg_buffer[256];
    struct _msg_info info; // Struct to hold metadata about the message

    cout << "[Server] Starting up, my PID is: " << getpid() << endl;

    // 1. Create a channel
    server_chid = ChannelCreate(0);
    if (server_chid == -1) {
        cerr << "[Server] Failed to create channel!" << endl;
        return NULL;
    }

    // 2. Loop to process messages
    while (true) {
        // By passing &info as the 4th argument, the OS fills it with metadata about the sender!
        rcvid = MsgReceive(server_chid, msg_buffer, sizeof(msg_buffer), &info);
        
        if (rcvid == -1) {
            cerr << "[Server] Error receiving message!" << endl;
            break;
        }

        if (rcvid > 0) {
            cout << "\n[Server] Received message payload: \"" << msg_buffer << "\"" << endl;
            
            // --- Extracting information from struct _msg_info ---
            cout << "[Server] --- Message Info Metadata Extract ---" << endl;
            cout << "  -> Sender PID (Process ID): " << info.pid << endl;
            cout << "  -> Sender TID (Thread ID):  " << info.tid << endl;
            cout << "  -> Node Descriptor (ND):    " << info.nd << " (0 means local)" << endl;
            cout << "  -> Message Length:          " << info.msglen << " bytes" << endl;
            cout << "----------------------------------------------" << endl;

            /* 
             * NOTE: If we had passed NULL to MsgReceive, we could get this exact 
             * same information by calling: MsgInfo(rcvid, &info);
             */

            // Send a reply
            const char* reply = "I received your message and logged your PID!";
            MsgReply(rcvid, 0, reply, strlen(reply) + 1);
            
            // Break loop if the client sends the shutdown command
            if (strcmp(msg_buffer, "shutdown") == 0) {
                cout << "[Server] Shutdown command received. Exiting loop." << endl;
                break;
            }
        }
    }
    
    ChannelDestroy(server_chid);
    cout << "[Server] Exiting cleanly." << endl;
    return NULL;
}

// =========================================================
// Client Thread (Sender)
// =========================================================
void* client_thread(void* arg) {
    int coid;
    char reply_buffer[256];

    // Give server a moment to start
    sleep(1);

    cout << "\n[Client] Starting up, my PID is: " << getpid() 
         << " | my TID is: " << pthread_self() << endl;

    coid = ConnectAttach(0, 0, server_chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        cerr << "[Client] Failed to attach!" << endl;
        return NULL;
    }

    // Send first message
    const char* msg1 = "Hello from the client thread!";
    cout << "[Client] Sending message 1..." << endl;
    MsgSend(coid, msg1, strlen(msg1) + 1, reply_buffer, sizeof(reply_buffer));
    cout << "[Client] Server replied: " << reply_buffer << endl;

    sleep(1);

    // Send shutdown message
    const char* msg2 = "shutdown";
    cout << "\n[Client] Sending shutdown message..." << endl;
    MsgSend(coid, msg2, strlen(msg2) + 1, reply_buffer, sizeof(reply_buffer));

    ConnectDetach(coid);
    cout << "[Client] Exiting cleanly." << endl;
    return NULL;
}

// =========================================================
// MAIN
// =========================================================
int main() {
    pthread_t server_tid, client_tid;

    cout << "--- QNX MsgInfo Metadata Example ---" << endl;

    pthread_create(&server_tid, NULL, server_thread, NULL);
    pthread_create(&client_tid, NULL, client_thread, NULL);

    pthread_join(client_tid, NULL);
    pthread_join(server_tid, NULL);

    cout << "\n--- Example Complete ---" << endl;
    return 0;
}
