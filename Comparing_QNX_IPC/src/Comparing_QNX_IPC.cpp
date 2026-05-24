#include <iostream>
#include <sys/neutrino.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

using namespace std;

// Custom definitions for our application
#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL
#define MSG_TYPE_HELLO  1

// Custom message structure for Native Messaging
struct MyMessage {
    uint16_t type;
    char text[128];
};

int server_chid;

// ====================================================================
// SERVER THREAD
// Demonstrates handling both Native Messages and Pulses in one loop
// ====================================================================
void* server_thread(void* arg) {
    server_chid = ChannelCreate(0);
    cout << "[Server] Started, chid = " << server_chid << endl;

    // A union is perfect here: it's either a message OR a pulse
    union {
        MyMessage msg;
        struct _pulse pulse;
    } rcv_buf;

    struct _msg_info info;

    while (true) {
        // MsgReceive blocks until a message OR a pulse arrives.
        int rcvid = MsgReceive(server_chid, &rcv_buf, sizeof(rcv_buf), &info);
        if (rcvid == -1) break;
        
        if (rcvid == 0) {
            // --------------------------------------------------------
            // QNX PULSE RECEIVED (rcvid == 0)
            // --------------------------------------------------------
            if (rcv_buf.pulse.code == MY_PULSE_CODE) {
                cout << "[Server] -> Received a Pulse! "
                     << "(Priority: " << (int)info.priority 
                     << ", Value: " << rcv_buf.pulse.value.sival_int << ")" << endl;
            } else {
                cout << "[Server] -> Received system pulse code: " << rcv_buf.pulse.code << endl;
            }
        } 
        else {
            // --------------------------------------------------------
            // QNX NATIVE MESSAGE RECEIVED (rcvid > 0)
            // --------------------------------------------------------
            // The server temporarily inherits the priority of the client here!
            if (rcv_buf.msg.type == MSG_TYPE_HELLO) {
                cout << "[Server] -> Received Message: '" << rcv_buf.msg.text 
                     << "' from client (rcvid " << rcvid << ")" << endl;
                
                // Process the data...
                sleep(1); // Simulate work
                
                // Reply to the client to unblock them and copy the reply data
                char reply_text[] = "Message received successfully!";
                MsgReply(rcvid, 0, reply_text, sizeof(reply_text));
                cout << "[Server] -> Reply sent to client." << endl;
            } else {
                MsgError(rcvid, ENOSYS);
            }
        }
    }
    return NULL;
}

// ====================================================================
// MAIN THREAD (CLIENT)
// Demonstrates sending both Native Messages and Pulses
// ====================================================================
int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, server_thread, NULL);
    sleep(1); // Give server a moment to start

    int server_coid = ConnectAttach(0, 0, server_chid, _NTO_SIDE_CHANNEL, 0);

    // ==================================================
    // 1. Demonstrate QNX Native Messaging 
    //    (Blocking, inherent sync, copies data)
    // ==================================================
    cout << "\n[Client] Sending Native Message to Server..." << endl;
    MyMessage msg;
    msg.type = MSG_TYPE_HELLO;
    strcpy(msg.text, "Hello from the Client Thread!");
    
    char reply_buf[128] = {0};
    
    // MsgSend BLOCKS the client until the server calls MsgReply.
    if (MsgSend(server_coid, &msg, sizeof(msg), reply_buf, sizeof(reply_buf)) == -1) {
        perror("MsgSend failed");
    } else {
        cout << "[Client] Server replied with: '" << reply_buf << "'" << endl;
    }

    sleep(1);

    // ==================================================
    // 2. Demonstrate QNX Pulses 
    //    (Non-blocking, tiny payload, carries priority)
    // ==================================================
    cout << "\n[Client] Sending an asynchronous Pulse to Server..." << endl;
    
    int pulse_priority = 10;
    int pulse_value = 42; // Payload (32-bit int)
    
    // MsgSendPulse DOES NOT block. The client continues immediately.
    MsgSendPulse(server_coid, pulse_priority, MY_PULSE_CODE, pulse_value);
    
    cout << "[Client] Pulse sent! I am not blocked, continuing execution..." << endl;

    sleep(1); // Wait a moment to see the server process the pulse in the terminal

    // Cleanup
    ConnectDetach(server_coid);
    ChannelDestroy(server_chid);
    
    return 0;
}
