#include <iostream>
#include <pthread.h>
#include <sys/neutrino.h>
#include <string.h>
#include <unistd.h>

using namespace std;

// Custom message structure sent from Client -> Server
typedef struct {
    int message_type;
    int data;
} my_msg_t;

// Custom reply structure sent from Server -> Client
typedef struct {
    int status_code;
    char response_text[64];
} my_reply_t;

// Global variable to share the Channel ID so the client can connect to it easily
int server_chid;

// =========================================================
// Server Thread (Receiver)
// =========================================================
void* server_thread(void* arg) {
    int rcvid;
    my_msg_t msg;
    my_reply_t reply;

    cout << "[Server] Starting up..." << endl;

    // 1. Create a channel for receiving messages
    server_chid = ChannelCreate(0);
    if (server_chid == -1) {
        cerr << "[Server] Failed to create channel!" << endl;
        return NULL;
    }

    cout << "[Server] Channel created. Listening for messages..." << endl;

    // 2. Infinite loop to process messages
    while (true) {
        // MsgReceive blocks until a client sends a message to this channel
        rcvid = MsgReceive(server_chid, &msg, sizeof(msg), NULL);
        
        if (rcvid == -1) {
            cerr << "[Server] Error receiving message!" << endl;
            break;
        }

        // A rcvid > 0 means we received a standard message from a client
        if (rcvid > 0) {
            cout << "[Server] Received msg from client! Type: " << msg.message_type 
                 << ", Data: " << msg.data << endl;

            // Process the message and prepare a reply
            reply.status_code = 200; // OK
            strcpy(reply.response_text, "Message processed successfully by server!");
            
            // Check for a special shutdown command
            if (msg.message_type == 99) {
                strcpy(reply.response_text, "Shutting down server...");
            }

            cout << "[Server] Sending reply back to client..." << endl;
            
            // 3. Send the reply back to unblock the client
            // We MUST use the rcvid so the OS knows which specific client to wake up
            MsgReply(rcvid, 0, &reply, sizeof(reply));
            
            // Exit loop if shutdown command received
            if (msg.message_type == 99) break;
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
    my_msg_t msg;
    my_reply_t reply;

    // Give the server a moment to start and create the channel
    sleep(1);

    cout << "\n[Client] Starting up..." << endl;

    // 1. Connect to the server's channel
    coid = ConnectAttach(0, 0, server_chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        cerr << "[Client] Failed to attach to channel!" << endl;
        return NULL;
    }

    // --- Send standard message ---
    msg.message_type = 1;
    msg.data = 42;

    cout << "[Client] Sending message to server..." << endl;

    // 2. Send the message and block until the server replies
    // MsgSend puts this thread to sleep until the server calls MsgReply
    int status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply));

    if (status == -1) {
        cerr << "[Client] MsgSend failed!" << endl;
    } else {
        cout << "[Client] Received reply from server! Status Code: " << reply.status_code 
             << " | Text: " << reply.response_text << endl;
    }
    
    sleep(1); // Pause for readability
    
    // --- Send shutdown command ---
    msg.message_type = 99;
    msg.data = 0;
    
    cout << "\n[Client] Sending shutdown command to server..." << endl;
    MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply));
    
    cout << "[Client] Server replied: " << reply.response_text << endl;

    // Clean up connection
    ConnectDetach(coid);
    cout << "[Client] Exiting cleanly." << endl;
    return NULL;
}

// =========================================================
// MAIN
// =========================================================
int main() {
    pthread_t server_tid, client_tid;

    cout << "--- QNX Message Passing Example ---" << endl;

    // Start server first so it can create the channel
    pthread_create(&server_tid, NULL, server_thread, NULL);
    
    // Start client
    pthread_create(&client_tid, NULL, client_thread, NULL);

    // Wait for both to finish
    pthread_join(client_tid, NULL);
    pthread_join(server_tid, NULL);

    cout << "\n--- Example Complete ---" << endl;
    return 0;
}
