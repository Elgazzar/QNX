#include <iostream>
#include <pthread.h>
#include <sys/neutrino.h>
#include <string.h>
#include <unistd.h>

using namespace std;

// Global channel ID
int server_chid;

// Custom Header Structure
typedef struct {
    uint16_t msg_type;
    uint32_t payload_size;
} custom_header_t;

// =========================================================
// Server Thread (Receiver)
// =========================================================
void* server_thread(void* arg) {
    int rcvid;

    // The server will receive the message into a single flat buffer
    // It doesn't need to know that the client sent it in multiple pieces!
    char rcv_buffer[1024];

    cout << "[Server] Starting up..." << endl;

    server_chid = ChannelCreate(0);
    if (server_chid == -1) {
        cerr << "[Server] Failed to create channel!" << endl;
        return NULL;
    }

    while (true) {
        // We use a standard MsgReceive. The OS kernel will reassemble the client's 
        // fragmented IOV pieces into our flat contiguous buffer automatically!
        rcvid = MsgReceive(server_chid, rcv_buffer, sizeof(rcv_buffer), NULL);
        if (rcvid == -1) break;

        if (rcvid > 0) {
            // Re-cast the start of the buffer to read the header
            custom_header_t* header = (custom_header_t*)rcv_buffer;
            
            // The payload starts immediately after the header structure
            char* payload = rcv_buffer + sizeof(custom_header_t);

            cout << "\n[Server] Message Received (Automatically Reassembled by Kernel)!" << endl;
            cout << "  -> Header [Type]: " << header->msg_type << endl;
            cout << "  -> Header [Size]: " << header->payload_size << " bytes" << endl;
            cout << "  -> Payload String: \"" << payload << "\"" << endl;

            // Reply
            const char* reply = "Got your multi-part message!";
            MsgReply(rcvid, 0, reply, strlen(reply) + 1);

            if (header->msg_type == 99) {
                cout << "[Server] Shutdown command received. Exiting." << endl;
                break;
            }
        }
    }
    
    ChannelDestroy(server_chid);
    return NULL;
}

// =========================================================
// Client Thread (Sender)
// =========================================================
void* client_thread(void* arg) {
    int coid;
    char reply_buffer[256];

    sleep(1); // Wait for server to set up

    coid = ConnectAttach(0, 0, server_chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) return NULL;

    // --- SETUP MULTI-PART MESSAGE ---
    
    // Part 1: The Header (stored on the stack here)
    custom_header_t header;
    header.msg_type = 1;
    
    // Part 2: The Payload (stored somewhere completely different in memory)
    const char* my_payload = "This payload is in a completely different memory location than the header!";
    header.payload_size = strlen(my_payload) + 1;

    // Create an IOV array with 2 elements (Header + Payload)
    iov_t send_iov[2];

    // Use the SETIOV macro to easily configure the base pointers and lengths
    SETIOV(&send_iov[0], &header, sizeof(header));
    SETIOV(&send_iov[1], (void*)my_payload, header.payload_size);

    cout << "\n[Client] Sending a 2-part message (Header + Payload) without copying memory!" << endl;
    
    // Send it using MsgSendv! 
    // 2 = number of parts we are sending
    // We use MsgSendvs (Vector Send, String Receive) because we are sending vectors, 
    // but we just want the reply put into our flat reply_buffer.
    MsgSendvs(coid, send_iov, 2, reply_buffer, sizeof(reply_buffer));
    
    cout << "[Client] Server Replied: \"" << reply_buffer << "\"" << endl;

    sleep(1);

    // Send shutdown message
    header.msg_type = 99;
    const char* shutdown_msg = "shutdown";
    header.payload_size = strlen(shutdown_msg) + 1;
    
    SETIOV(&send_iov[0], &header, sizeof(header));
    SETIOV(&send_iov[1], (void*)shutdown_msg, header.payload_size);
    MsgSendvs(coid, send_iov, 2, reply_buffer, sizeof(reply_buffer));

    ConnectDetach(coid);
    cout << "[Client] Exiting cleanly." << endl;
    return NULL;
}

// =========================================================
// MAIN
// =========================================================
int main() {
    pthread_t server_tid, client_tid;
    
    cout << "--- QNX MsgSendv and SETIOV Example ---" << endl;

    pthread_create(&server_tid, NULL, server_thread, NULL);
    pthread_create(&client_tid, NULL, client_thread, NULL);

    pthread_join(client_tid, NULL);
    pthread_join(server_tid, NULL);

    cout << "\n--- Example Complete ---" << endl;
    return 0;
}
